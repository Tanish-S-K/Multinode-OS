<div align="center">

# Multinode-OS

**A custom 32-bit distributed storage operating system built from scratch in freestanding C and x86 Assembly.**

Multiple QEMU nodes share a single virtual block device via NBD, with atomic sector-level synchronization ensuring every node sees a consistent filesystem in real time.

[![Demo Video](https://img.shields.io/badge/Demo-Google%20Drive-blue?logo=googledrive)](https://drive.google.com/file/d/1973q_ohmqSTnc7vITdf72fYgvUf7jBHP/view?usp=sharing)
![Language](https://img.shields.io/github/languages/top/Tanish-S-K/Multinode-OS)
![License](https://img.shields.io/github/license/Tanish-S-K/Multinode-OS)

</div>

---

## Overview

Multinode-OS is a bare-metal operating system designed to explore distributed storage at the lowest possible level — no Linux, no libc, no abstractions. The kernel boots via a custom 16-bit bootloader, transitions to 32-bit protected mode, and exposes a persistent hierarchical filesystem over a raw ATA PIO block device. Multiple VM instances attach to the same disk image through Linux's Network Block Device (NBD), achieving real-time synchronization purely through atomic sector writes and a shared bitmap — no network protocol stack required.

---

## Features

| Category | Details |
|---|---|
| **Multinode Sync** | Multiple QEMU VMs share one NBD-backed virtual disk; writes on any node are immediately visible to all others |
| **Atomic I/O** | Metadata updates, bitmap allocation, and sector writes are sequenced to prevent torn writes or partial state |
| **Hierarchical Filesystem** | Doubly-linked sibling tree stored directly on disk sectors; supports nested directories and path-based navigation |
| **ATA PIO Driver** | Raw sector reads/writes via port-mapped I/O — no DMA, no OS abstraction |
| **Multi-user Isolation** | Per-user ownership on every file/directory node; authentication at boot |
| **Custom Standard Library** | Hand-rolled memory management, string ops, integer parsing, and ATA wrappers — zero external dependencies |
| **Interactive CLI** | 14-command shell supporting file ops, directory traversal, copy/paste, user management, and disk formatting |

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
Bootloader  (16-bit real mode, bootloader.asm)
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

### DiskNode Layout

Every file and directory is stored as a `DiskNode` record occupying one 512-byte sector in the metadata region (sectors 210–909). File data lives in the data region (sectors 1000–2047).

```
DiskNode
────────────────────────────────
name[19]      file or directory name
type          1 = directory, 0 = file
user          owner user ID
parent        sector index of parent node
child         sector index of first child
next_sib      sector index of next sibling
prev_sib      sector index of previous sibling
data_sector   sector index of file data (1000–2047)
size          file size in bytes
```

The sibling pointers form a doubly-linked list per directory level, enabling O(n) directory traversal without a separate allocation table.

### Write Path & Sync Guarantee

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
    ├── lock filesystem state
    ├── allocate sector via bitmap
    └── update DiskNode metadata
    │
    ▼
Sync Layer  (atomic sector writes)
    │
    ▼
ATA PIO Driver  ──▶  Shared NBD disk  ──▶  all attached nodes
```

**Guarantees:**
- Bitmap is updated before data is written — no leaked sectors on crash
- Metadata sector is written in a single PIO command — no partial node state
- All nodes read from the same underlying block device — no cache divergence

### Multinode Execution Model

```
┌──────────────┐    ┌──────────────┐    ┌──────────────┐
│   QEMU VM 1  │    │   QEMU VM 2  │    │   QEMU VM N  │
│  /dev/nbd0   │    │  /dev/nbd1   │    │  /dev/nbdN   │
└──────┬───────┘    └──────┬───────┘    └──────┬───────┘
       │                   │                   │
       └───────────────────┼───────────────────┘
                           │
                    ┌──────▼───────┐
                    │  os-image.img │
                    │ (raw disk img)│
                    └──────────────┘
```

Each VM connects to the same image file via `qemu-nbd`. A write committed by VM 1 is immediately visible as raw sector data to VM 2 and VM 3 on their next read.

---

## Project Structure

```
Multinode-OS/
├── boot/
│   └── bootloader.asm       # 16-bit real-mode bootloader; loads kernel & switches to protected mode
│
├── kernel/
│   ├── asm/
│   │   └── entry.asm        # GDT load, protected-mode entry point, jumps to kernel_main()
│   ├── kernel.c             # Kernel entry, hardware init sequence
│   ├── file_system.c        # DiskNode CRUD, bitmap allocator, path resolver
│   ├── cli.c                # Interactive shell, command dispatch
│   ├── auth.c               # User creation & boot-time authentication
│   └── mystdlib.c           # Custom memory, string, and ATA utility functions
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
| `nfile <path>` | Create a new file at the given path |
| `wfile <path>` | Write content to a file |
| `rfile <path>` | Read and print file content |
| `dfile <path>` | Delete a file |
| `cpy <path>` | Copy a file to the clipboard buffer |
| `paste <path>` | Paste the clipboard buffer to a destination path |

### Directory Operations
| Command | Description |
|---|---|
| `ndir <path>` | Create a new directory |
| `goto <path>` | Change the current working directory |
| `list` | List all entries in the current directory |
| `curpos` | Print the current directory path |

### System Commands
| Command | Description |
|---|---|
| `nuser` | Create a new user account |
| `dformat` | Format the entire disk (wipes all data) |
| `shutdown` | Halt the OS |
| `restart` | Reboot the OS |

---

## Prerequisites

The following tools must be installed on a Linux host:

- `nasm` — assembler for the bootloader and kernel entry
- `gcc` with multilib (`gcc-multilib`) — 32-bit freestanding compilation
- `ld` (GNU binutils, i386 target) — binary linking
- `qemu-system-i386` — x86 VM emulation
- `qemu-nbd` — NBD server for shared-disk multinode mode
- `make`

On Debian/Ubuntu:
```bash
sudo apt install nasm gcc gcc-multilib binutils qemu-system-x86 qemu-utils make
```

---

## Build & Run

### 1. Load the NBD kernel module *(once per host boot)*
```bash
make set
# runs: sudo modprobe nbd max_part=16
```

### 2. Build the OS image
```bash
make
# compiles bootloader + kernel, links, and writes os-image.img
```

### 3. Boot a single node
```bash
make run1
```

### 4. Boot multiple nodes (shared-disk multinode)
Open separate terminals and run each node concurrently:
```bash
# Terminal 1
make run1

# Terminal 2
make run2

# Terminal 3
make run3
```
Any file written from one node will appear immediately when read from another.

### 5. Clean build artifacts
```bash
make clean
```

---

## How NBD Sync Works

Linux's `qemu-nbd` exposes the raw `os-image.img` file as a block device (e.g. `/dev/nbd0`). Each QEMU instance is launched with that block device as its IDE drive. Because all instances ultimately read and write the same file on the host filesystem, there is no replication layer — consistency is a direct consequence of shared physical storage. The OS's atomic write ordering ensures that readers on any node never observe an intermediate state.

---

## License

This project is licensed under the [MIT License](LICENSE).

---

<div align="center">
Built from scratch — no libc, no Linux, no shortcuts.
</div>
