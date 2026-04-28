<div align="center">

# Multinode-OS

```
    A custom 32-bit baremetal OS written in freestanding C and x86 Assembly.
    No standard library. Boots from scratch. Runs on QEMU.
    Multiple VM instances share one virtual disk with atomic synchronized writes.
```

---

[![Demo Video](https://img.shields.io/badge/Demo-Google%20Drive-blue?logo=googledrive)](https://drive.google.com/file/d/1973q_ohmqSTnc7vITdf72fYgvUf7jBHP/view?usp=sharing)
![Language](https://img.shields.io/github/languages/top/Tanish-S-K/Multinode-OS)
![License](https://img.shields.io/github/license/Tanish-S-K/Multinode-OS)

</div>

## What This Actually Is

```
    NOT a networked distributed OS.
    NOT using Raft, Paxos, or any consensus protocol.

    It is a shared-disk multinode execution model.
    Multiple QEMU VMs attach to the same disk image via NBD.
    The OS ensures writes are atomic so all nodes stay consistent.
    The hypervisor handles disk sharing. The OS handles write safety.
```

---

## Demo

    Demo Video: https://drive.google.com/file/d/1973q_ohmqSTnc7vITdf72fYgvUf7jBHP/view?usp=sharing

## Project Structure

```
    Multinode-OS/
    |
    +-- boot/
    |   +-- bootloader.asm
    |
    +-- kernel/
    |   +-- asm/
    |   |   +-- entry.asm
    |   |
    |   +-- kernel.c
    |   +-- file_system.c
    |   +-- cli.c
    |   +-- auth.c
    |   +-- mystdlib.c
    |
    +-- header/
    |   +-- file_system.h
    |   +-- cli.h
    |   +-- auth.h
    |   +-- mystdlib.h
    |
    +-- makefile
    +-- README.md
```

---
    
## Boot Sequence

```
    CPU Reset
        |
        v
    BIOS loads Sector 0
        |
        v
    Bootloader                  [ 16-bit real mode | Assembly ]
        |
        +-- Loads kernel into memory
        |
        v
    Protected Mode Entry        [ 32-bit | GDT / IDT / IRQ setup ]
        |
        v
    Kernel Init
        |
        +-- ATA PIO driver
        +-- PIT timer
        +-- Video output
        |
        v
    Filesystem Init
        |
        +-- Load sector bitmap
        +-- Load root DiskNode
        |
        v
    User Authentication
        |
        v
    CLI Shell                   [ 14 commands | path-based ]
```

---

## Disk Layout

```
    Sector 0
    +---------------------------+
    |  Bootloader               |   BIOS entry point
    +---------------------------+
    Sectors 1 - 9
    +---------------------------+
    |  Kernel Code              |   OS loaded here at boot
    +---------------------------+
    Sectors 10 - 209
    +---------------------------+
    |  Auth Data                |   Persistent user credentials
    +---------------------------+
    Sectors 210 - 909
    +---------------------------+
    |  DiskNodes                |   File and directory metadata
    +---------------------------+
    Sectors 910 - 999
    +---------------------------+
    |  Bitmap                   |   Tracks free/used data sectors
    +---------------------------+
    Sectors 1000 - 2047
    +---------------------------+
    |  File Data                |   Actual file content
    +---------------------------+

    Note: zones were explicitly separated after a bug where
    kernel sectors were being overwritten by file data during writes.
```

---

## DiskNode Format

```
    Every file and directory is stored as a DiskNode on disk.
    One DiskNode occupies one sector.

    DiskNode
    +---------------+------------------------------------------+
    | name[19]      |  file or directory name                  |
    | type          |  0 = file,  1 = directory                |
    | user          |  owner user ID                           |
    | parent        |  sector address of parent directory      |
    | child         |  sector address of first child entry     |
    | next_sib      |  next sibling (linked list forward)      |
    | prev_sib      |  previous sibling (linked list backward) |
    | data_sector   |  sector address of file content          |
    | size          |  file size in bytes                      |
    +---------------+------------------------------------------+

    Tree on disk:

    root/
    |
    +-- user1/
    |   |
    |   +-- docs/
    |   |   |
    |   |   +-- notes.txt ---------> [data sector 1024]
    |   |
    |   +-- code.c -----------------> [data sector 1031]
    |
    +-- user2/
        |
        +-- config.txt ------------> [data sector 1056]

    Directory traversal uses subtree search, not flat sequential scan.
```

---

## Write Path

```
    User types command
            |
            v
        CLI Shell
            |
            v
        Filesystem Manager
            |
            +-- lock filesystem
            +-- check bitmap for free sector
            +-- allocate sector
            +-- update DiskNode metadata
            |
            v
        ATA PIO Driver
            |
            +-- write metadata to disk
            +-- write file data to disk
            +-- update bitmap on disk
            |
            v
        Unlock filesystem

    Result: no partial writes, no torn metadata, bitmap always consistent.
    All nodes attached to the shared disk see the same state after every write.
```

---

## Multinode Execution

```
    +------------+     +------------+     +------------+
    |   Node 1   |     |   Node 2   |     |   Node 3   |
    |   (QEMU)   |     |   (QEMU)   |     |   (QEMU)   |
    +-----+------+     +-----+------+     +-----+------+
          |                  |                  |
          +------------------+------------------+
                             |
                    +--------v--------+
                    |   Shared Disk   |
                    |  (NBD / .img)   |
                    +-----------------+

    Each node runs the same OS image independently.
    All nodes read and write the same underlying disk.
    The OS atomic write path prevents nodes from corrupting each other.
```

---

## Multi-User System

```
    Each user has an isolated subtree in the filesystem.
    Login is required at boot.
    User A cannot access User B's files or directories.
    Credentials and user metadata are stored persistently on disk.
```

---

## Built From Scratch

```
    Component           Description
    ----------------    -------------------------------------------------
    mystdlib.c          memcpy, memset, string ops, int-to-string, etc.
    mem allocator       custom mem() function, equivalent of malloc
    ATA PIO driver      raw sector read/write via x86 I/O ports
    GDT / IDT           descriptor tables and interrupt handler setup
    Filesystem          tree structure, bitmap, path-based resolution
    Auth system         persistent multi-user login
    CLI                 14 commands, full path-based navigation
```

---

## CLI Commands

```
    File Operations
    ---------------
    nfile <path>    create file
    wfile <path>    write to file
    rfile <path>    read file
    dfile <path>    delete file
    cpy   <path>    copy file
    paste <path>    paste file

    Directory Operations
    --------------------
    ndir  <path>    create directory
    goto  <path>    change current directory
    list            list entries in current directory
    curpos          print current directory path

    System
    ------
    nuser           create new user
    dformat         format entire disk
    shutdown        power off
    restart         reboot
```

---

## Technologies Used

```
    Language        Freestanding C (no stdlib), x86 Assembly
    Emulator        QEMU
    Disk sharing    NBD (Network Block Device)
    Build tools     GNU Make, NASM, GCC cross-compiler
```

---

## Build and Run

```
    1. Build disk image and kernel
       make set

    2. Run a single node
       make run1

    3. Run multiple nodes against shared disk
       make run1     (terminal 1)
       make run2     (terminal 2)
       make run3     (terminal 3)
```

---

## Known Limitations

```
    - Multinode sync relies on QEMU + NBD, not a custom network protocol
    - No networking stack
    - Single-threaded kernel, no preemptive scheduling
    - No virtual memory or paging
```

---
