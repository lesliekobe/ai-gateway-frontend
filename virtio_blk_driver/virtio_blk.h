# SPDX-License-Identifier: GPL-2.0
/*
 * virtio_blk.h - VirtIO Block Driver Public Header
 *
 * VirtIO Block device definitions per VirtIO 1.0 spec.
 * VirtIO specification: https://docs.oasis-open.org/virtio/virtio/v1.0/
 *
 * Copyright (C) 2026 OpenClaw Agent
 */

#ifndef __VIRTIO_BLK_H__
#define __VIRTIO_BLK_H__

#include <linux/types.h>
#include <linux/virtio.h>
#include <linux/virtio_config.h>
#include <linux/virtio_ring.h>

/* VirtIO Block device status codes */
#define VIRTIO_BLK_S_OK         0
#define VIRTIO_BLK_S_IOERR      1
#define VIRTIO_BLK_S_UNSUPP     2

/* VirtIO Block config space (device feature negotiated) */
struct virtio_blk_config {
    __le64 capacity;           /* 512-byte sectors */
    __le32 size_max;          /* max segment size */
    __le32 seg_max;            /* max segments per request */
    __le16 cylinders;          /* geometry */
    __le8 heads;              /* geometry */
    __le8 sectors;             /* geometry */
    __le32 blk_size;           /* block size (usually 512) */
    __le8 physical_block_exp;  /* physical block size exponent */
    __le8 alignment;           /* alignment boundary */
    __le16 min_io_size;       /* minimum I/O size */
    __le32 opt_io_size;        /* optimal I/O size */
    __le8 discard_sector;      /* discard sector alignment */
    __le32 discard_alignment;  /* discard alignment */
    __le32 discard_granularity;/* discard granularity */
    __le32 write_zeroes;       /* write zeroes support flags */
    __u8   reserved[68];       /* padding to 144 bytes */
    __le32 max_discard_sectors;/* max discard sectors */
    __le32 max_write_zeroes_sectors;/* max write zeroes sectors */
    __le32 max_secure_erase_sectors; /* max secure erase sectors */
    __le32 secure_erase_sector; /* secure erase alignment */
    __u8   unknown[80];        /* additional config */
} __attribute__((packed));

/* VirtIO Block request types */
#define VIRTIO_BLK_T_IN          0   /* Read sectors */
#define VIRTIO_BLK_T_OUT         1   /* Write sectors */
#define VIRTIO_BLK_T_FLUSH       2   /* Flush cache */
#define VIRTIO_BLK_T_DISCARD     3   /* Discard sectors */
#define VIRTIO_BLK_T_WRITE_ZEROES 4  /* Write zeroes */
#define VIRTIO_BLK_T_SECURE_ERASE 5  /* Secure erase */
#define VIRTIO_BLK_T_IOCTL       6   /* Device-specific ioctl */
#define VIRTIO_BLK_T_BLK_SIZE    7   /* Get block size (obsolete) */
#define VIRTIO_BLK_T_FLUSH_OUT   8   /* Flush out (ordered) */
#define VIRTIO_BLK_T_MAX         9   /* Maximum request type */

/* VirtIO Block feature bits */
#define VIRTIO_BLK_F_SIZE_MAX    (1 << 0)   /* Segment size limit */
#define VIRTIO_BLK_F_SEG_MAX    (1 << 1)    /* Segments per request */
#define VIRTIO_BLK_F_GEOMETRY   (1 << 2)    /* Legacy geometry */
#define VIRTIO_BLK_F_RO          (1 << 5)   /* Read-only */
#define VIRTIO_BLK_F_BLK_SIZE    (1 << 6)    /* Block size provided */
#define VIRTIO_BLK_F_FLUSH       (1 << 9)    /* Flush command */
#define VIRTIO_BLK_F_DISCARD    (1 << 13)   /* Discard support */
#define VIRTIO_BLK_F_WRITE_ZEROES (1 << 14) /* Write zeroes support */
#define VIRTIO_BLK_F_SECURE_ERASE (1 << 15) /* Secure erase support */
#define VIRTIO_BLK_F_CONFIG_WCE  (1 << 16)   /* Write-back cache config */
#define VIRTIO_BLK_F_MQ         (1 << 12)   /* Multiple queues */
#define VIRTIO_BLK_F_DISCARD_WZEROES (1 << 27) /* Discard writes zeroes */

/* VirtIO Block request header */
struct virtio_blk_req {
    __le32 type;       /* Request type (VIRTIO_BLK_T_*) */
    __le32 reserved;   /* Must be zero */
    __le64 sector;     /* Starting sector (512-byte units) */
} __attribute__((packed));

/* VirtIO Block status (always at end of descriptor chain) */
struct virtio_blk_status {
    __u8 status;       /* VIRTIO_BLK_S_* */
} __attribute__((packed));

/* VirtIO Block ioctl commands (for legacy compatibility) */
#define VIRTIO_BLK_IOCTL_SCSI_DISKZEROES   0x12
#define VIRTIO_BLK_IOCTL_SCSI_DISKPOWERCYCLE 0x16

/* VirtIO device ID for block devices */
#define VIRTIO_ID_BLOCK     2   /* from VirtIO spec */

/* MSI-X vectors (optional) */
#define VIRTIO_BLK_MSIX_CONFIG   0
#define VIRTIO_BLK_MSIX_VQ       1

/*
 * Driver feature bits we support
 * ( Negotiated features are subset of these )
 */
#define VIRTIO_BLK_F_FEATURES \
    (VIRTIO_F_RING_NOTIFY_ON_EMPTY | \
     VIRTIO_F_RING_INDIRECT_DESC | \
     VIRTIO_F_RING_EVENT_IDX | \
     VIRTIO_BLK_F_SIZE_MAX | \
     VIRTIO_BLK_F_SEG_MAX | \
     VIRTIO_BLK_F_GEOMETRY | \
     VIRTIO_BLK_F_RO | \
     VIRTIO_BLK_F_BLK_SIZE | \
     VIRTIO_BLK_F_FLUSH | \
     VIRTIO_BLK_F_DISCARD | \
     VIRTIO_BLK_F_WRITE_ZEROES | \
     VIRTIO_BLK_F_CONFIG_WCE | \
     VIRTIO_BLK_F_MQ)

/*
 * IOCTL definitions for userspace
 */
#define VIRTIO_BLKDEVNAME   "vblk"
#define VIRTBLK_IOC_MAGIC   'V'
#define VIRTBLK_IOC_FLUSH   _IO(VIRTBLK_IOC_MAGIC, 0)
#define VIRTBLK_IOC_DISCARD _IOW(VIRTBLK_IOC_MAGIC, 1, struct virtblk_discard)

struct virtblk_discard {
    __u64 sector;
    __u32 num_sectors;
    __u32 flags;
};

/*
 * Device info exposed to userspace
 */
struct virtblk_info {
    __u64 capacity;        /* Total sectors */
    __u32 blk_size;        /* Block size (typically 512) */
    __u32 max_segments;    /* Max segments per request */
    __u32 max_discard_sectors;
    __u32 max_write_zeroes_sectors;
    __u8  read_only;       /* VIRTIO_BLK_F_RO */
    __u8  write_back;      /* Cached write back mode */
    __u8  discard_enabled;
    __u8  write_zeroes_enabled;
    __u8  secure_erase_enabled;
    __u16 num_queues;      /* Number of request queues */
    __u32 reserved[3];
};

#endif /* __VIRTIO_BLK_H__ */