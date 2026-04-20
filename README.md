<div align="center">

# Multinode-OS

**A custom 32-bit operating system built from scratch in freestanding C and x86 Assembly.**

Multiple independent QEMU instances boot from and operate on the same shared virtual disk — giving each node a live, consistent view of the filesystem without any network protocol or replication layer.

[![Demo Video](https://img.shields.io/badge/Demo-Google%20Drive-blue?logo=googledrive)](https://drive.google.com/file/d/1973q_ohmqSTnc7vITdf72fYgvUf7jBHP/view?usp=sharing)
![Language](https://img.shields.io/github/languages/top/Tanish-S-K/Multinode-OS)
![License](https://img.shields.io/github/license/Tanish-S-K/Multinode-OS)

</div>

---

## Overview

Multinode-OS is a bare-metal operating system written without any standard library or OS support. It boots via a hand-written 16-bit bootloader, transitions into 32-bit protected mode, and exposes a persistent hierarchical filesystem over a raw ATA PIO block device.

The multinode aspect is not distributed computing — there is no network stack, no message passing, and no replication protocol. Instead, multiple QEMU VMs are each connected to the **same raw disk image** via Linux's NBD (Network Block Device) interface. Because every node reads and writes the same physical storage, they inherently share state. Consistency is achieved through **atomic sector-level writes** on the OS side, ensuring no node ever observes a partially-written filesystem.

Think of it as multiple CPUs sharing one RAM chip — except the shared medium is a block device.

---

## Features

| Category | Details |
|---|---|
| **Shared-disk multinode** | Multiple QEMU VMs mount and operate on the same NBD-backed disk image simultaneously |
| **Atomic sector writes** | Bitmap allocation, metadata updates, and data writes are sequenced to prevent torn or partial state |
| **Hierarchical filesystem** | Doubly-linked sibling tree stored directly on disk — supports nested directories and full path navigation |
| **ATA PIO driver** | Raw sector reads/writes via port-mapped I/O, no DMA, no abstraction layer |
| **Multi-user isolation** | Per-node user accounts with ownership tracked on every filesystem node |
| **Custom stdlib** | Hand-rolled memory management, string ops, integer parsing, and ATA wrappers — zero external dependencies |
| **Interactive CLI** | 14-command shell covering file ops, directory traversal, copy/paste, user management, and disk formatting |

---

## Architecture

### Boot Sequence

```
CPU Reset
    │
    ▼
BIOS loads sector 0
    │
    ▼
Bootloader  (16-bit real mode — bootloader.asm)
    │  loads kernel image into memory
    ▼
Protected Mode switch  (entry.asm — GDT load, CR0 set)
    │
    ▼
Kernel Init
    ├── GDT / IDT setup
    ├── IRQ remapping (PIC)
    ├── PIT (system timer)
    ├── VGA text-mode driver
    └── ATA PIO driver init
    │
    ▼
Filesystem Init
    ├── Load sector bitmap  (sectors 0–209)
    └── Load root DiskNode  (sector 210)
    │
    ▼
User Authentication
    │
    ▼
CLI Shell
```

### How Multinode Sharing Works

Each node is a fully independent QEMU VM. They do not communicate with each other. What they share is the **underlying block device** — a single `os-image.img` file exposed to each VM via `qemu-nbd` as a separate `/dev/nbdN` device.

```
┌──────────────┐    ┌──────────────┐    ┌──────────────┐
│   QEMU VM 1  │    │   QEMU VM 2  │    │   QEMU VM N  │
│  /dev/nbd0   │    │  /dev/nbd1   │    │  /dev/nbdN   │
│              │    │              │    │              │
│  boots its   │    │  boots its   │    │  boots its   │
│  own kernel  │    │  own kernel  │    │  own kernel  │
└──────┬───────┘    └──────┬───────┘    └──────┬───────┘
       │  ATA PIO write    │                   │
       └───────────────────┼───────────────────┘
                           │  all point to the same file
                    ┌──────▼───────┐
                    │  os-image.img │
                    │  (shared raw  │
                    │   disk image) │
                    └──────────────┘
```

A sector written by VM 1 is immediately present in the raw file — so when VM 2 next reads that sector via its own ATA driver, it gets the updated data. There is no sync protocol; the shared medium *is* the synchronization mechanism.

### Write Path & Atomicity

The OS ensures that a write from any node leaves the disk in a valid state at every step:

```
User Command
    │
    ▼
CLI Shell
    │
    ▼
Filesystem Manager
    │
    ▼
Node Manager
    ├── 1. Lock filesystem state
    ├── 2. Allocate sector in bitmap  ← bitmap written first
    └── 3. Update DiskNode metadata   ← metadata written second
    │
    ▼
ATA PIO Driver  ──▶  os-image.img  ──▶  visible to all VMs
```

**Ordering guarantees:**
- The bitmap sector is committed before any data is written — no dangling pointers on crash
- Each DiskNode is written as a single 512-byte sector — no partial metadata state
- Any node reading after a write completes will see the fully updated filesystem

### DiskNode Structure

Every file and directory is one `DiskNode` record, stored in the metadata region (sectors 210–909). File contents live in the data region (sectors 1000–2047).

```
DiskNode  (one per 512-byte sector)
────────────────────────────────────
name[19]      file or directory name
type          1 = directory,  0 = file
user          owner user ID
parent        sector index of parent node
child         sector index of first child node
next_sib      sector index of next sibling
prev_sib      sector index of previous sibling
data_sector   sector index of file content (1000–2047)
size          file size in bytes
```

Siblings at each directory level are linked in a doubly-linked list, allowing directory listing and recursive operations without a separate index structure.

---

## Project Structure

```
Multinode-OS/
├── boot/
│   └── bootloader.asm       # 16-bit real-mode bootloader; loads kernel & enters protected mode
│
├── kernel/
│   ├── asm/
│   │   └── entry.asm        # GDT descriptor, protected-mode jump, calls kernel_main()
│   ├── kernel.c             # Kernel entry point, hardware init sequence
│   ├── file_system.c        # DiskNode CRUD, sector bitmap allocator, path resolver
│   ├── cli.c                # Interactive shell, command parser and dispatch
│   ├── auth.c               # User creation and boot-time authentication
│   └── mystdlib.c           # Memory, string, and ATA utility functions (no libc)
│
├── header/
│   ├── file_system.h
│   ├── cli.h
│   ├── auth.h
│   └── mystdlib.h
│
├── makefile
└── README.md
```

---

## CLI Reference

### File Operations
| Command | Description |
|---|---|
| `nfile <path>` | Create a new file |
| `wfile <path>` | Write content to a file |
| `rfile <path>` | Read and print a file |
| `dfile <path>` | Delete a file |
| `cpy <path>` | Copy a file to the internal clipboard |
| `paste <path>` | Paste clipboard contents to a destination |

### Directory Operations
| Command | Description |
|---|---|
| `ndir <path>` | Create a new directory |
| `goto <path>` | Change working directory |
| `list` | List entries in the current directory |
| `curpos` | Print the current directory path |

### System Commands
| Command | Description |
|---|---|
| `nuser` | Create a new user account |
| `dformat` | Format the disk (erases all data) |
| `shutdown` | Halt the OS |
| `restart` | Reboot the OS |

---

## Prerequisites

A Linux host with the following packages:

```bash
sudo apt install nasm gcc gcc-multilib binutils qemu-system-x86 qemu-utils make
```

| Tool | Purpose |
|---|---|
| `nasm` | Assembles the bootloader and kernel entry |
| `gcc` (multilib) | Compiles the kernel in 32-bit freestanding mode |
| `ld` (binutils, i386) | Links kernel objects into a flat binary |
| `qemu-system-i386` | Runs each VM node |
| `qemu-nbd` | Exposes the disk image as an NBD block device |

---

## Build & Run

### Step 1 — Load the NBD kernel module
Run once per host boot session:
```bash
make set
# sudo modprobe nbd max_part=16
```

### Step 2 — Build the OS image
```bash
make
```
This compiles the bootloader and kernel, links them, and produces `os-image.img`.

### Step 3 — Boot a single node
```bash
make run1
```

### Step 4 — Run multiple nodes on the same disk
Open a separate terminal for each node and launch concurrently:
```bash
# Terminal 1        # Terminal 2        # Terminal 3
make run1           make run2           make run3
```

Any file created or modified from one node will be readable from any other node immediately after the write completes.

### Clean up
```bash
make clean
```

---

## License

[MIT](LICENSE)

---

<div align="center">
No libc. No Linux. No abstractions. Just the hardware.
</div>
