**⚙️ Custom 32-bit Distributed Storage OS — Multinode Realtime Sync**

A custom-built 32-bit distributed storage operating system written in no-lib C + x86 assembly, supporting multiple nodes sharing a single disk with atomic, realtime synchronization.
The OS provides multi-user isolation with data seperation and protection, a hierarchical filesystem with data persistency, a custom ATA PIO storage layer, and a full interactive CLI.

**📷 Demo**

(Add images / GIFs here)

**🎥 Video**

(Add link here)

**⭐ Features**

🟦 Shared-disk multinode execution

🟩 Atomic realtime synchronization between nodes

🗂️ Custom hierarchical filesystem

💾 Persistent ATA PIO sector I/O

👥 Multiple isolated users with data seperation and protection

📁 Path-based recursive operations

🧠 Custom stdlib with custom memory management, parsing, string ops, I/O ops. 

🖥️ 14-command interactive CLI

**🛠️ Technologies Used**

C (freestanding, no stdlib)

x86 Assembly (16-bit + 32-bit protected mode)

Custom GDT/IDT setup

Custom ATA PIO driver

QEMU for CPU + device virtualization

NBD (Network Block Device) for multinode shared-disk execution

make for project compilation and running automation

**📦 Project Structure**

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

**🧩 Architectures**

**1️⃣ 📀 Disk Sector Format (On-Disk Layout)**

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

**2️⃣ 📂 Runtime Filesystem Structure (2–3 Levels)**

ROOT ("/")
│
├── userA_root/
│   ├── docs/
│   └── media/
│
└── userB_root/
    ├── code/
    └── notes/

**3️⃣ 🧱 Entry Node Structure (DiskNode Format)**

DiskNode (stored in sectors 210–910)
-------------------------------------
name[19]      : file/dir name
type          : 1 = dir, 0 = file
user          : owner id
parent        : parent sector id
child         : first child sector
next_sib      : next sibling
prev_sib      : previous sibling
data_sector   : pointer to file data (1000–2047)
size          : file size (bytes)

**🧠 How It Works**

**1️⃣ 🧩 UML Diagram — Internal Architecture**

┌────────────────────────────────────────────────────────────────┐
│                              Kernel                            │
└────────────────────────────────────────────────────────────────┘
            ▲                          ▲                       ▲

┌──────────────────────────┐   ┌─────────────────────────┐   ┌─────────────────────────┐
│        CLI Shell         │   │   Filesystem Manager    │   │      ATA PIO Driver     │
└───────────┬──────────────┘   └─────────────┬───────────┘   └───────────┬────────────┘
            │                                │                           │
            ▼                                ▼                           ▼
                   ┌──────────────────────────────────────────┐
                   │              Node Manager                 │
                   └───────────────────┬──────────────────────┘
                                       │
                                       ▼
                           ┌────────────────────────────┐
                           │  Sync Layer (Atomic Write) │
                           └───────────────┬────────────┘
                                           │
                                           ▼
                           ┌────────────────────────────┐
                           │  Disk (IDE/ATA/NBD Virtual) │
                           └────────────────────────────┘

**2️⃣ ⭐ Distributed Execution Model**

                 ┌──────────────┐
                 │    Node 1     │
                 └───────┬──────┘
                         │
                         │ (Shared Disk)
                         ▼
        ┌──────────────┐   ┌──────────────┐   ┌──────────────┐
        │    Node 2     │   │    Node 3     │   │    Node 4     │
        └──────────────┘   └──────────────┘   └──────────────┘


All nodes view the exact same disk image → perfect global sync.

**3️⃣ ⚡ Boot Sequence**

CPU Reset
   ▼
BIOS loads MBR (sector 0)
   ▼
Bootloader executes (16-bit)
   ▼
Loads kernel from disk
   ▼
Enable Protected Mode (GDT + CR0)
   ▼
Kernel Init
   - ATA PIO Init
   - Interrupts
   - PIT
   - Video
   - Filesystem init
   ▼
Load bitmap + root DiskNode
   ▼
User authentication
   ▼
CLI shell

4️⃣ 💠 How OS Works in Virtual Mode
🟦 Step 1 — Disk Image → Virtual Block Device

The OS image becomes a virtual IDE/NBD-backed disk.
Reads/writes map to ATA PIO operations.

🟩 Step 2 — VM Boots from This Virtual Disk

BIOS loads bootloader → bootloader loads kernel.

🟧 Step 3 — Multinode Execution

Multiple VMs attach to the same virtual block device, enabling:

Instant cross-node visibility

Shared bitmap

Cross-node metadata consistency

**5️⃣ 🔗 Realtime Sync Explanation (Write Path)**

USER COMMAND
      ▼
CLI
      ▼
Filesystem Manager
      ▼
Node Manager
  - lock FS
  - allocate sectors (bitmap)
  - update metadata
      ▼
Sync Layer
  - atomic sector commits
      ▼
ATA PIO Driver
  - writes to disk


Guarantees:

Atomic metadata updates

Bitmap never corrupted

All nodes see same state

**🖥️ CLI Commands (Reference)**

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

**🚀 Build & Run Instructions**

🟦 1. Build Disk + Kernel
make set

🟩 2. Run Node Instance (example: instance 1)
make run1

🟧 3. Run Multiple Nodes

Example for k nodes:

make run1
make run2
.
.
make runk
