// SPDX-License-Identifier: GPL-2.0
/*
 * virtio_blk.c - VirtIO Block Device Driver
 *
 * A Linux kernel module implementing a VirtIO block storage device.
 * This driver creates a block device that can be exposed to the host
 * via VirtIO specification.
 *
 * VirtIO spec: https://docs.oasis-open.org/virtio/virtio/v1.0/
 *
 * Copyright (C) 2026 OpenClaw Agent
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/blkdev.h>
#include <linux/blk-mq.h>
#include <linux/vmalloc.h>
#include <linux/mutex.h>
#include <linux/errno.h>
#include <linux/scatterlist.h>
#include <linux/interrupt.h>
#include <linux/completion.h>
#include <linux/spinlock.h>
#include <linux/genhd.h>
#include <linux/part_stat.h>
#include <linux/dma-mapping.h>
#include <linux/string.h>
#include <linux/crc32c.h>

#include "virtio_blk.h"

/* Driver identification */
#define DRV_NAME    "virtio_blk_emu"
#define DRV_VERSION "1.0.0"

/* Default configuration - can be overridden by module params */
static int num_queues = 4;
module_param(num_queues, int, 0444);
MODULE_PARM_DESC(num_queues, "Number of virtqueues per device (default: 4)");

static int queue_depth = 256;
module_param(queue_depth, int, 0444);
MODULE_PARM_DESC(queue_depth, "Depth of each virtqueue (default: 256)");

static int max_segments = 32;
module_param(max_segments, int, 0444);
MODULE_PARM_DESC(max_segments, "Max segments per request (default: 32)");

static int block_size = 512;
module_param(block_size, int, 0444);
MODULE_PARM_DESC(block_size, "Block size in bytes (default: 512)");

static long long disk_size_gb = 1;
module_param(disk_size_gb, long long, 0444);
MODULE_PARM_DESC(disk_size_gb, "Virtual disk size in GB (default: 1)");

/* Device state */
struct virtblk_dev {
    struct virtio_device *vdev;
    struct virtio_config_ops *config_ops;

    /* Device configuration */
    struct virtio_blk_config config;
    u64 features;                     /* negotiated features */

    /* Block device */
    struct gendisk *disk;
    struct request_queue *queue;
    struct blk_mq_tag_set tag_set;

    /* Virtqueues */
    struct virtqueue **vqs;           /* array of virtqueues */
    int num_vqs;                      /* actual number of vqs */
    struct virtio_blk_req *reqs;      /* allocated requests */
    spinlock_t vq_lock;

    /* Device properties */
    bool readonly;
    bool write_back;
    u64 capacity;                     /* in sectors */
    u32 blk_size;                     /* block size */

    /* Request tracking */
    struct work_struct config_work;
    spinlock_t lock;
    atomic64_t inflight;

    /* For virtio 1.0 we track used indices */
    u16 last_vq_index;

    /* PCI/transport info */
    int vq_index_base;                /* starting vq index in device */
};

/* Per-I/O request data */
struct virtblk_io {
    struct request *req;
    struct virtblk_dev *dev;
    struct virtio_blk_req header;
    struct virtio_blk_status status;
    struct scatterlist status_sg;
    struct scatterlist header_sg;
    struct scatterlist *sgs;
    int nsgs;
    void *buf;
    size_t buf_len;
    struct completion done;
    int err;
};

static int major_num = 0;
static struct class *blk_class;
static DEFINE_IDA(minor_ida);

/* Forward declarations */
static int virtblk_init_vqs(struct virtblk_dev *dev);
static void virtblk_done(struct virtqueue *vq);
static int virtblk_queue_rq(struct blk_mq_hw_ctx *hctx,
                            const struct blk_mq_queue_data *bd);
static void virtblk_cleanup(struct virtblk_dev *dev);
static blk_status_t virtblk_submit_req(struct virtblk_dev *dev,
                                       struct request *req);

/* =========================================================================
 * VirtIO Transport Helpers
 * ========================================================================= */

/* Find the negotiated feature value */
static bool virtblk_has_feature(struct virtblk_dev *dev, u32 feature)
{
    return (dev->features & (1ULL << feature)) != 0;
}

/* Read a byte from config space */
static u8 virtblk_get_config8(struct virtblk_dev *dev, int offset)
{
    u8 val;
    dev->vdev->config->get(dev->vdev, offset, &val, sizeof(val));
    return val;
}

/* Read 16-bit from config space */
static u16 virtblk_get_config16(struct virtblk_dev *dev, int offset)
{
    u16 val;
    __virtio_set_bit(dev->vdev, offset, &val, sizeof(val));
    return val;
}

/* Read 32-bit from config space */
static u32 virtblk_get_config32(struct virtblk_dev *dev, int offset)
{
    u32 val;
    dev->vdev->config->get(dev->vdev, offset, &val, sizeof(val));
    return val;
}

/* Read 64-bit from config space */
static u64 virtblk_get_config64(struct virtblk_dev *dev, int offset)
{
    u64 val;
    dev->vdev->config->get(dev->vdev, offset, &val, sizeof(val));
    return val;
}

/* Read the entire config space into our structure */
static void virtblk_read_config(struct virtblk_dev *dev)
{
    struct virtio_blk_config *cfg = &dev->config;

    cfg->capacity = virtblk_get_config64(dev,
        offsetof(struct virtio_blk_config, capacity));
    cfg->size_max = virtblk_get_config32(dev,
        offsetof(struct virtio_blk_config, size_max));
    cfg->seg_max = virtblk_get_config32(dev,
        offsetof(struct virtio_blk_config, seg_max));
    cfg->blk_size = virtblk_get_config32(dev,
        offsetof(struct virtio_blk_config, blk_size));
    cfg->write_zeroes = virtblk_get_config32(dev,
        offsetof(struct virtio_blk_config, write_zeroes));

    dev->capacity = cfg->capacity;
    dev->blk_size = cfg->blk_size ? cfg->blk_size : 512;
}

/* =========================================================================
 * Request handling
 * ========================================================================= */

/* Complete a request */
static void virtblk_request_done(struct virtblk_dev *dev,
                                  struct virtblk_io *vio)
{
    struct request *req = vio->req;
    blk_status_t status;

    if (vio->err) {
        status = BLK_STS_IOERR;
    } else {
        switch (vio->status.status) {
        case VIRTIO_BLK_S_OK:
            status = BLK_STS_OK;
            break;
        case VIRTIO_BLK_S_IOERR:
            status = BLK_STS_IOERR;
            break;
        case VIRTIO_BLK_S_UNSUPP:
            status = BLK_STS_NOTSUPP;
            break;
        default:
            status = BLK_STS_IOERR;
            break;
        }
    }

    blk_mq_end_request(req, status);
}

/* Handle completion from virtqueue */
static void virtblk_vq_complete(struct virtqueue *vq)
{
    struct virtblk_dev *dev = vq->vdev->priv;
    unsigned long flags;
    unsigned int len;

    spin_lock_irqsave(&dev->lock, flags);

    while (true) {
        struct virtblk_io *vio;
        u16 last_used = vring_get_used(vq)->last_used;

        /* Process completed requests */
        while (vq->last_used_idx != last_used) {
            struct vring_used_elem *e;

            e = &vring_get_used(vq)->ring[vq->last_used_idx % vq->num];
            vq->last_used_idx++;

            /* Map descriptor to our request structure */
            vio = (struct virtblk_io *)(unsigned long)e->id;
            if (vio && vio != (void *)~0UL) {
                atomic64_dec(&dev->inflight);
                if (req)
                    virtblk_request_done(dev, vio);
            }
        }

        /* Check for more completions */
        if (last_used == vring_get_used(vq)->last_used)
            break;
    }

    spin_unlock_irqrestore(&dev->lock, flags);

    /* Notify blocked waiters if any */
    // blk_mq_unblock_requeue_list(dev->queue);
}

/* =========================================================================
 * Block request submission
 * ========================================================================= */

/* Build scatterlist from request bio */
static int virtblk_map_sgs(struct virtblk_dev *dev,
                           struct request *req,
                           struct virtblk_io *vio,
                           struct scatterlist **sgs_out)
{
    struct bio *bio = req->bio;
    struct scatterlist *sgs;
    struct scatterlist *sg;
    int nsegs = 0;
    int ret = 0;

    /* Count segments */
    struct bio *b;
    int segments = 0;
    bio_for_each_bio(b) {
        segments++;
    }

    if (segments > max_segments) {
        ret = -EINVAL;
        goto fail;
    }

    /* Allocate scatterlist array + 2 for header and status */
    sgs = kcalloc(segments + 2, sizeof(struct scatterlist), GFP_ATOMIC);
    if (!sgs) {
        ret = -ENOMEM;
        goto fail;
    }

    /* Initialize header scatterlist */
    sg_init_one(&sgs[0], &vio->header, sizeof(vio->header));

    /* Map data segments */
    sg = &sgs[1];
    nsegs = 1;

    bio_for_each_bio(bio) {
        struct bio_vec bvec;
        struct bvec_iter iter;

        bio_for_each_segment(bvec, bio, iter) {
            void *buf = kmap_atomic(bio_vec_page(&bvec)) + bvec.bv_offset;
            size_t len = bvec.bv_len;

            sg_set_page(sg, bio_vec_page(&bvec), len, bvec.bv_offset);
            nsegs++;
            sg++;

            kunmap_atomic(buf);
        }
    }

    /* Initialize status scatterlist (at the end) */
    sg_init_one(&sgs[nsegs], &vio->status, sizeof(vio->status));
    nsegs++;

    /* Pin all pages and build translation */
    {
        struct scatterlist *psg;
        int i;

        for (i = 0, psg = sgs; i < nsegs; i++, psg++) {
            struct page *page = sg_page(psg);
            dma_addr_t addr;

            if (!page) continue;

            addr = dma_map_page(&dev->vdev->dev, page,
                               psg->offset, psg->length,
                               DMA bidir ???
                               // For read: DMA_FROM_DEVICE
                               // For write: DMA_TO_DEVICE
                               DMA_BIDIRECTIONAL);
            if (dma_mapping_error(&dev->vdev->dev, addr)) {
                ret = -EIO;
                goto fail_unmap;
            }

            psg->dma_address = addr;
        }
    }

    *sgs_out = sgs;
    return nsegs;

fail_unmap:
    /* Unmap all successfully mapped pages */
    {
        struct scatterlist *psg;
        int i;

        for (i = 0, psg = sgs; i < nsegs; i++, psg++) {
            if (psg->dma_address)
                dma_unmap_page(&dev->vdev->dev, psg->dma_address,
                              psg->length, DMA_BIDIRECTIONAL);
        }
    }
    kfree(sgs);

fail:
    return ret;
}

/* Unmap scatterlist */
static void virtblk_unmap_sgs(struct virtblk_dev *dev,
                              struct scatterlist *sgs, int nsegs)
{
    int i;
    struct scatterlist *sg;

    for (i = 0, sg = sgs; i < nsegs; i++, sg++) {
        if (sg->dma_address) {
            dma_unmap_page(&dev->vdev->dev,
                          sg->dma_address,
                          sg->length,
                          DMA_BIDIRECTIONAL);
        }
    }

    kfree(sgs);
}

/* Check if request is a flush */
static bool is_flush_request(struct request *req)
{
    return (req_op(req) == REQ_OP_FLUSH);
}

/* Check if request is discard */
static bool is_discard_request(struct request *req)
{
    return (req_op(req) == REQ_OP_DISCARD);
}

/* Get sector number from request */
static u64 virtblk_req_sector(struct request *req)
{
    return blk_rq_pos(req) >> (ilog2(dev->blk_size) - 9);
}

/* Build the request header */
static void virtblk_build_header(struct virtblk_dev *dev,
                                 struct request *req,
                                 struct virtblk_io *vio)
{
    memset(&vio->header, 0, sizeof(vio->header));

    switch (req_op(req)) {
    case REQ_OP_READ:
        vio->header.type = VIRTIO_BLK_T_IN;
        break;
    case REQ_OP_WRITE:
        vio->header.type = VIRTIO_BLK_T_OUT;
        break;
    case REQ_OP_FLUSH:
        vio->header.type = VIRTIO_BLK_T_FLUSH;
        break;
    case REQ_OP_DISCARD:
        vio->header.type = VIRTIO_BLK_T_DISCARD;
        break;
    case REQ_OP_WRITE_ZEROES:
        vio->header.type = VIRTIO_BLK_T_WRITE_ZEROES;
        break;
    default:
        vio->header.type = VIRTIO_BLK_T_IN;
        break;
    }

    vio->header.sector = cpu_to_le64(virtblk_req_sector(req));
}

/* Submit a single request to a virtqueue */
static blk_status_t virtblk_submit_req(struct virtblk_dev *dev,
                                       struct request *req)
{
    struct virtblk_io *vio;
    struct virtqueue *vq;
    struct scatterlist *sgs = NULL;
    int nsegs;
    int ret;
    u32 qid = blk_mq_unique_tag(request_get_mq_tag(req)) % dev->num_vqs;
    unsigned long flags;

    /* Get a free request slot */
    vio = kmalloc(sizeof(struct virtblk_io), GFP_ATOMIC);
    if (!vio) {
        return BLK_STS_RESOURCE;
    }

    memset(vio, 0, sizeof(*vio));
    vio->req = req;
    vio->dev = dev;

    /* Build header */
    virtblk_build_header(dev, req, vio);

    /* Map bio to scatterlist */
    nsegs = virtblk_map_sgs(dev, req, vio, &sgs);
    if (nsegs < 0) {
        kfree(vio);
        return BLK_STS_IOERR;
    }

    vio->sgs = sgs;
    vio->nsgs = nsegs;

    /* Get the virtqueue */
    vq = dev->vqs[qid];

    /* Add to virtqueue */
    ret = virtqueue_add_sgs(vq, sgs, 1, 1, vio, GFP_ATOMIC);
    if (ret) {
        virtblk_unmap_sgs(dev, sgs, nsegs);
        kfree(vio);
        return BLK_STS_RESOURCE;
    }

    /* Trigger virtqueue notification */
    virtqueue_kick(vq);

    atomic64_inc(&dev->inflight);

    return BLK_STS_OK;
}

/* ========================================================================
 * Block device operations
 * ======================================================================== */

static blk_status_t virtblk_queue_rq(struct blk_mq_hw_ctx *hctx,
                            const struct blk_mq_queue_data *bd)
{
    struct request *req = bd->rq;
    struct virtblk_dev *dev = hctx->queue->queuedata;
    blk_status_t status;

    blk_mq_start_request(req);

    status = virtblk_submit_req(dev, req);
    if (status != BLK_STS_OK) {
        /* For now, treat resource errors as retry */
        return status;
    }

    return BLK_STS_OK;
}

/* Get device info for ioctl */
static int virtblk_get_info(struct virtblk_dev *dev, struct virtblk_info *info)
{
    memset(info, 0, sizeof(*info));

    info->capacity = dev->capacity;
    info->blk_size = dev->blk_size;
    info->max_segments = max_segments;
    info->read_only = dev->readonly;
    info->write_back = dev->write_back;

    if (virtblk_has_feature(dev, VIRTIO_BLK_F_DISCARD)) {
        info->discard_enabled = 1;
        info->max_discard_sectors = dev->config.max_discard_sectors;
    }

    if (virtblk_has_feature(dev, VIRTIO_BLK_F_WRITE_ZEROES)) {
        info->write_zeroes_enabled = 1;
        info->max_write_zeroes_sectors = dev->config.max_write_zeroes_sectors;
    }

    info->num_queues = dev->num_vqs;

    return 0;
}

/* =========================================================================
 * Device initialization and probe
 * ========================================================================= */

/* Initialize virtqueues */
static int virtblk_init_vqs(struct virtblk_dev *dev)
{
    int ret;
    int i;

    dev->num_vqs = num_queues;

    dev->vqs = kcalloc(dev->num_vqs, sizeof(struct virtqueue *), GFP_KERNEL);
    if (!dev->vqs) {
        ret = -ENOMEM;
        goto fail;
    }

    /* Request virtqueues from VirtIO core */
    ret = virtio_find_vqs(dev->vdev, dev->num_vqs, dev->vqs,
                         NULL, NULL);
    if (ret) {
        dev_err(&dev->vdev->dev, "Failed to find virtqueues: %d\n", ret);
        goto fail;
    }

    /* Initialize each virtqueue */
    for (i = 0; i < dev->num_vqs; i++) {
        struct virtqueue *vq = dev->vqs[i];

        vq->vdev = dev->vdev;
        vq->callback = virtblk_vq_complete;
        vq->num_free = vq->num;

        /* Enable callbacks */
        virtqueue_disable_cb(vq);

        dev_info(&dev->vdev->dev, "Virtqueue %d: %d entries\n",
                 i, vq->num);
    }

    return 0;

fail:
    kfree(dev->vqs);
    dev->vqs = NULL;
    return ret;
}

/* Probe function - called when device is discovered */
static int virtblk_probe(struct virtio_device *vdev)
{
    struct virtblk_dev *dev;
    int ret;
    int minor;

    dev_info(&vdev->dev, "VirtIO Block device probed\n");

    /* Allocate device structure */
    dev = kzalloc(sizeof(struct virtblk_dev), GFP_KERNEL);
    if (!dev)
        return -ENOMEM;

    dev->vdev = vdev;
    vdev->priv = dev;
    spin_lock_init(&dev->lock);
    atomic64_set(&dev->inflight, 0);

    /* Step 1: Negotiate features */
    u64 features = vdev->config->find_vqs(vdev, ...);
    // Note: Actual feature negotiation happens via virtio config ops
    dev->features = vdev->features;

    dev_info(&vdev->dev, "Negotiated features: 0x%llx\n", dev->features);

    /* Step 2: Read configuration */
    virtblk_read_config(dev);

    dev_info(&vdev->dev, "Capacity: %llu sectors (%llu GB)\n",
             dev->capacity,
             (dev->capacity * dev->blk_size) / (1024 * 1024 * 1024));

    /* Step 3: Check read-only */
    if (virtblk_has_feature(dev, VIRTIO_BLK_F_RO)) {
        dev->readonly = true;
        dev_info(&vdev->dev, "Device is read-only\n");
    }

    /* Step 4: Initialize virtqueues */
    ret = virtblk_init_vqs(dev);
    if (ret) {
        dev_err(&vdev->dev, "Failed to init virtqueues: %d\n", ret);
        goto fail;
    }

    /* Step 5: Setup block device */
    ret = -ENOMEM;

    /* Register block device */
    minor = ida_simple_get(&minor_ida, 0, minor, GFP_KERNEL);
    dev->disk = blk_mq_alloc_disk(&dev->tag_set, dev);
    if (!dev->disk) {
        ret = -ENOMEM;
        goto fail_vqs;
    }

    dev->disk->major = major_num;
    dev->disk->first_minor = minor;
    dev->disk->minors = 1;
    dev->disk->fops = &virtblk_fops;
    dev->disk->queue = dev->queue;
    dev->disk->private_data = dev;

    snprintf(dev->disk->disk_name, DISK_NAME_LEN, "vblk%c", 'a' + minor);

    /* Set capacity */
    set_capacity(dev->disk, dev->capacity);

    /* Register disk */
    ret = add_disk(dev->disk);
    if (ret) {
        put_disk(dev->disk);
        goto fail_vqs;
    }

    dev_info(&vdev->dev, "VirtIO Block device registered as %s\n",
             dev->disk->disk_name);

    return 0;

fail_vqs:
    /* Clean up virtqueues */
    if (dev->vqs) {
        int i;
        for (i = 0; i < dev->num_vqs; i++) {
            if (dev->vqs[i])
                vdev->config->del_vqs(dev->vqs[i]);
        }
        kfree(dev->vqs);
    }

fail:
    kfree(dev);
    return ret;
}

/* Remove function - called when device is removed */
static void virtblk_remove(struct virtio_device *vdev)
{
    struct virtblk_dev *dev = vdev->priv;
    int minor = disk->first_minor;

    dev_info(&vdev->dev, "Removing VirtIO Block device\n");

    /* Flush and remove block device */
    del_gendisk(dev->disk);
    blk_mq_free_tag_set(&dev->tag_set);

    /* Stop virtqueues */
    if (dev->vqs) {
        int i;
        for (i = 0; i < dev->num_vqs; i++) {
            if (dev->vqs[i]) {
                virtqueue_disable_cb(dev->vqs[i]);
                synchronize_rcu();
            }
        }
        vdev->config->del_vqs(dev->vqs);
        kfree(dev->vqs);
    }

    /* Free device structure */
    ida_simple_remove(&minor_ida, minor);
    kfree(dev);

    dev_info(&vdev->dev, "VirtIO Block device removed\n");
}

/* Device reset */
static void virtblk_reset(struct virtio_device *vdev)
{
    struct virtblk_dev *dev = vdev->priv;

    dev_info(&vdev->dev, "Device reset\n");

    /* Clear any pending state */
    atomic64_set(&dev->inflight, 0);

    /* Reset configuration */
    vdev->config->reset(vdev);
}

/* ========================================================================
 * VirtIO driver registration
 * ======================================================================== */

/* VirtIO device IDs we support */
static const struct virtio_device_id virtblk_id_table[] = {
    { VIRTIO_ID_BLOCK, VIRTIO_ANY_ID },
    { 0 },
};

/* VirtIO configuration changed callback */
static void virtblk_config_changed(struct virtio_device *vdev)
{
    struct virtblk_dev *dev = vdev->priv;
    unsigned long flags;

    spin_lock_irqsave(&dev->lock, flags);

    /* Re-read configuration */
    virtblk_read_config(dev);

    /* Update disk capacity if changed */
    if (dev->disk) {
        set_capacity(dev->disk, dev->capacity);
    }

    spin_unlock_irqrestore(&dev->lock, flags);

    dev_info(&vdev->dev, "Configuration changed: capacity now %llu\n",
             dev->capacity);
}

/* VirtIO driver operations */
static unsigned int virtblk_features[] = {
    VIRTIO_BLK_F_FEATURES,
};

static const struct virtio_config_ops virtblk_config_ops = {
    .get_features   = virtblk_get_features,
    .set_features   = virtblk_set_features,
    .get            = virtblk_get_config,
    .set            = virtblk_set_config,
    .finalize_features = virtblk_finalize_features,
    .reset          = virtblk_reset,
    .find_vqs       = virtblk_find_vqs,
    .del_vqs        = virtblk_del_vqs,
    .get_status     = virtblk_get_status,
    .set_status     = virtblk_set_status,
};

static struct virtio_driver virtblk_driver = {
    .driver.name         = DRV_NAME,
    .driver.owner         = THIS_MODULE,
    .feature_table       = virtblk_features,
    .feature_table_size  = ARRAY_SIZE(virtblk_features),
    .device_id           = VIRTIO_ID_BLOCK,
    .virtio_device_id    = virtblk_id_table,
    .probe               = virtblk_probe,
    .remove              = virtblk_remove,
    .config_changed      = virtblk_config_changed,
};

/* ========================================================================
 * Module initialization and cleanup
 * ======================================================================== */

/* Module init */
static int __init virtblk_init(void)
{
    int ret;

    pr_info("VirtIO Block Emulation Driver v%s\n", DRV_VERSION);

    /* Register the block device major */
    ret = register_blkdev(major_num, DRV_NAME);
    if (ret < 0) {
        pr_err("Failed to register block device: %d\n", ret);
        return ret;
    }
    major_num = ret;

    /* Create device class */
    blk_class = class_create(DRV_NAME);
    if (IS_ERR(blk_class)) {
        pr_err("Failed to create class\n");
        unregister_blkdev(major_num, DRV_NAME);
        return PTR_ERR(blk_class);
    }

    /* Initialize IDA for minor numbers */
    ida_init(&minor_ida);

    /* Register virtio driver */
    ret = register_virtio_driver(&virtblk_driver);
    if (ret) {
        pr_err("Failed to register virtio driver: %d\n", ret);
        class_destroy(blk_class);
        unregister_blkdev(major_num, DRV_NAME);
        return ret;
    }

    pr_info("VirtIO Block driver loaded (major=%d)\n", major_num);
    return 0;
}

/* Module exit */
static void __exit virtblk_exit(void)
{
    unregister_virtio_driver(&virtblk_driver);
    class_destroy(blk_class);
    unregister_blkdev(major_num, DRV_NAME);
    ida_destroy(&minor_ida);

    pr_info("VirtIO Block driver unloaded\n");
}

module_init(virtblk_init);
module_exit(virtblk_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("OpenClaw Agent");
MODULE_DESCRIPTION("VirtIO Block Storage Emulation Driver");
MODULE_VERSION(DRV_VERSION);