# VirtIO Block Device Driver

A Linux kernel module implementing a VirtIO block storage device (emulation side).

**WARNING**: This is a work-in-progress skeleton. It has compilation errors and is not functional as-is.

## Directory

```
virtio_blk_driver/
├── virtio_blk.h       # VirtIO Block definitions (public header)
├── virtio_blk.c       # Main driver source (skeleton, has bugs)
├── Makefile          # Out-of-tree kernel module build
└── README.md         # This file
```

## What is VirtIO Block?

VirtIO is a para-virtualization specification for I/O devices. A VirtIO block device provides storage abstraction:

```
Guest VM (Linux)                    Host (QEMU/KVM)
┌────────────────────┐              ┌────────────────────┐
│ VirtIO Block Driver│◄── VirtQueue ──►│   Emulated Device  │
│   (frontend)       │              │   (backend)        │
└────────────────────┘              └────────────────────┘
```

## Build

Requires Linux kernel headers:

```bash
make
```

## Status

- Header file (`virtio_blk.h`): Complete, follows VirtIO 1.0 spec
- Driver (`virtio_blk.c`): Skeleton, contains known bugs

## Known Issues

The driver source has several intentional bugs/errors and is not intended to compile. It serves as a structural reference for a VirtIO block device driver implementation.

## VirtIO Reference

- VirtIO 1.0 Spec: https://docs.oasis-open.org/virtio/virtio/v1.0/
- VirtIO Block Spec: Section 5.2

## License

GPL-2.0