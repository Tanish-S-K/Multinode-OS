⚙️ Custom 32-bit Distributed Storage OS — Multinode Realtime Sync

A custom-built 32-bit distributed storage operating system written in no-lib C + x86 assembly, supporting multiple nodes sharing a single disk with atomic realtime synchronization.
Provides multi-user isolation, a hierarchical persistent filesystem, a custom ATA PIO storage stack, and a complete interactive CLI.

📷 Demo

(Add images / GIFs here)

🎥 Video

(Add link here)

⭐ Features

Shared-disk multinode execution

Atomic realtime synchronization between nodes

Custom hierarchical filesystem

Persistent ATA PIO sector I/O

Multiple isolated users with data separation & protection

Path-based recursive operations

Custom stdlib (memory management, parsing, I/O, string ops)

14-command interactive CLI

🛠️ Technologies Used

C (freestanding, no stdlib)

x86 Assembly: 16-bit Real Mode + 32-bit Protected Mode

Custom GDT/IDT setup

Custom ATA PIO driver

QEMU (CPU + device virtualization)

NBD (Network Block Device) for multinode shared-disk execution

Makefile-based build + run automation

📦 Project Structure
OS/
├── boot/
│   └── bootloader.asm
│
├── kernel/
│   ├── asm/
│   │   └── entry.asm
│   ├── kernel.c
│   ├── file_system.c
│   ├── cli.c
│   ├── auth.c
│   └── mystdlib.c
│
├── header/
│   ├── file_system.h
│   ├── cli.h
│   ├── auth.h
│   └── mystdlib.h
│
├── Makefile
└── README.md

🧩 Architectures
1️⃣ 📀 Disk Sector Format (On-Disk Layout)
┌───────────────────────────────────────────────┐
│ 0–199     : Bootloader + Kernel + Drivers     │
├───────────────────────────────────────────────┤
│ 200–209   : Bitmap (allocation table)         │
├───────────────────────────────────────────────┤
│ 210–910   : DiskNode Metadata Table           │
├───────────────────────────────────────────────┤
│ 911–999   : User Credentials                  │
├───────────────────────────────────────────────┤
│ 1000–2047 : File Data (raw sectors)           │
└───────────────────────────────────────────────┘

2️⃣ 📂 Runtime Filesystem Structure (2–3 Levels)
ROOT ("/")
│
├── userA_root/
│   ├── docs/
│   └── media/
│
└── userB_root/
    ├── code/
    └── notes/

3️⃣ 🧱 Entry Node Structure (DiskNode Format)
DiskNode (sector 210–910)
-------------------------------------
name[19]      : filename / dirname
type          : 1 = directory, 0 = file
user          : owner id
parent        : parent sector id
child         : first child sector
next_sib      : next sibling
prev_sib      : previous sibling
data_sector   : file data pointer (1000–2047)
size          : file size in bytes

🧠 How It Works
1️⃣ 🧩 Internal Architecture (UML Overview)
┌──────────────────────────────────────────────────────────────────┐
│                              Kernel                              │
└──────────────────────────────────────────────────────────────────┘
          ▲                        ▲                        ▲
          │                        │                        │

┌──────────────────────────┐   ┌─────────────────────────┐  ┌─────────────────────────┐
│        CLI Shell         │   │   Filesystem Manager    │  │     ATA PIO Driver      │
└───────────┬──────────────┘   └─────────────┬───────────┘  └───────────┬────────────┘
            │                                │                          │
            ▼                                ▼                          ▼
                       ┌──────────────────────────────────────────┐
                       │              Node Manager                 │
                       └──────────────────┬───────────────────────┘
                                          │
                                          ▼
                               ┌────────────────────────────┐
                               │   Sync Layer (Atomic I/O)  │
                               └───────────────┬────────────┘
                                               │
                                               ▼
                               ┌────────────────────────────┐
                               │   Disk (IDE / ATA / NBD)   │
                               └────────────────────────────┘

2️⃣ ⭐ Distributed Execution Model
                   ┌──────────────┐
                   │    Node 1     │
                   └───────┬──────┘
                           │
                           │  (Shared Virtual Disk)
                           ▼
           ┌──────────────┐   ┌──────────────┐   ┌──────────────┐
           │    Node 2     │   │    Node 3     │   │    Node 4     │
           └──────────────┘   └──────────────┘   └──────────────┘


All nodes read/write the same disk → realtime, global sync.

3️⃣ ⚡ Boot Sequence
CPU Reset
   ▼
BIOS loads MBR (sector 0)
   ▼
Bootloader executes (16-bit)
   ▼
Bootloader loads kernel from disk
   ▼
Enable 32-bit Protected Mode (GDT + CR0)
   ▼
Kernel Initialization
   - ATA PIO driver
   - Interrupts + IDT
   - PIT
   - Terminal screen
   - Memory management
   - Filesystem init
   ▼
Load bitmap + root node
   ▼
User authentication
   ▼
CLI shell

4️⃣ 💠 How OS Works in Virtual Mode
Step 1 — Disk Image → Virtual Block Device

The OS image is mapped as a virtual IDE/NBD-backed disk device.
Reads/writes translate into ATA PIO sector operations.

Step 2 — VM Boots From This Virtual Disk

BIOS → Bootloader → Kernel → Filesystem init → CLI.

Step 3 — Multinode Execution

Multiple QEMU instances attach to the same disk, enabling:

Instant visibility of changes

Shared bitmap

Globally consistent metadata

Atomic write commits

5️⃣ 🔗 Realtime Sync — Write Path
USER COMMAND
     ▼
CLI Shell
     ▼
Filesystem Manager
     ▼
Node Manager
  - lock FS
  - allocate sectors (bitmap)
  - update metadata
     ▼
Sync Layer
  - atomic sector writes
     ▼
ATA PIO Driver
  - physical disk write

Guarantees

Atomic filesystem metadata

Bitmap never corrupted

All nodes see the same state

🖥️ CLI Commands (Reference)
ndir <path>      → create directory  
nfile <path>     → create file  
wfile <path>     → write to file  
rfile <path>     → read file  
dfile <path>     → delete file  

goto <path>      → change directory  
list             → list entries  
curpos           → show current path  

cpy <path>       → copy file  
paste <path>     → paste copied file  

dformat          → format disk  
nuser            → create user  

shutdown         → shutdown OS  
restart          → reboot OS  

🚀 Build & Run Instructions
1️⃣ Build disk + kernel
make set

2️⃣ Run a node (example: node 1)
make run1

3️⃣ Run multiple nodes (example: k = 3)
make run1
make run2
make run3
