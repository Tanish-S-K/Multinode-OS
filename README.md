**⚙️ Custom 32-bit Distributed Storage OS — Multinode Realtime Sync**

        A custom-built 32-bit distributed storage operating system written in no-lib C + x86 assembly, 
        supporting multiple nodes sharing a single virtual disk with atomic realtime synchronization.
        The OS includes Multinode sync using shared block device execution providing realtime data sync,
        Multi-user isolation,Persistent hierarchical filesystem,Custom ATA PIO storage driver,Full 
        interactive CLI.

## Demo

🎥 Demo Video: https://drive.google.com/file/d/1973q_ohmqSTnc7vITdf72fYgvUf7jBHP/view?usp=sharing


**⭐ Features**

        🟦 Shared-disk multinode execution
        
        🟩 Atomic realtime synchronization (metadata + sector writes)
        
        🗂️ Custom hierarchical filesystem
        
        💾 Persistent ATA PIO disk I/O
        
        👥 Isolated multi-user support
        
        📁 Path-based recursive operations
        
        🧠 Custom stdlib (memory mgmt, parsing, string ops, ATA wrappers)
        
        💻 14-command interactive CLI

**🛠️ Technologies Used**

        Freestanding C (no stdlib)
        
        x86 Assembly (16-bit bootloader + 32-bit protected-mode kernel)
        
        Custom GDT / IDT / IRQ handling
        
        ATA PIO driver for raw sector I/O
        
        QEMU for CPU + device virtualization
        
        NBD (Network Block Device) for multinode shared-disk execution
        
        make for build automation

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

**High level file system overview**
  
        pass
        

** 🧱 Entry Node Structure (DiskNode Format)**

        DiskNode (210–910)
        ------------------
        name[19]     → file/dir name
        type         → 1 = dir, 0 = file
        user         → owner ID
        parent       → parent sector
        child        → first child sector
        next_sib     → next sibling
        prev_sib     → previous sibling
        data_sector  → pointer to file data (1000–2047)
        size         → file size (bytes)

**⚡ Boot Sequence**

        CPU Reset
           ↓
        BIOS loads sector 0
           ↓
        Bootloader (16-bit real mode)
           ↓
        Load kernel → enable protected mode
           ↓
        Kernel Init:
           - GDT / IDT
           - IRQs
           - PIT
           - Video
           - ATA PIO driver
           ↓
        Filesystem Init:
           - Load bitmap
           - Load root DiskNode
           ↓
        User Authentication
           ↓
        CLI Shell

**💠 How OS Works in Virtual Mode**

        🔹 Step 1 — Virtual Disk Creation
        
        The OS image is mapped into a virtual IDE/NBD disk.
        All sector reads/writes go through the ATA PIO driver.
        
        🔹 Step 2 — VM Boots from Virtual Disk
        
        BIOS → Bootloader → Kernel → Filesystem.
        
        🔹 Step 3 — Multinode Execution
        
        Multiple VMs attach to the same virtual block device →
        Writes from one node instantly appear in all nodes.

**🔗 Realtime Sync Explanation (Write Path)**

        USER COMMAND
             ↓
        CLI Shell
             ↓
        Filesystem Manager
             ↓
        Node Manager
           - lock FS
           - allocate sectors (bitmap)
           - update metadata
             ↓
        Sync Layer (atomic sector writes)
             ↓
        ATA PIO Driver (physical write)


        Guarantees:
        
        Atomic metadata updates
        
        Bitmap always consistent
        
        No partial or torn writes
        
        All nodes see the same disk state

**🖥️ CLI Commands (Reference)**

        ndir <path>      → create directory
        nfile <path>     → create file
        wfile <path>     → write to file
        rfile <path>     → read file
        dfile <path>     → delete file
        
        goto <path>      → change directory
        list             → list entries
        curpos           → show current directory
        
        cpy <path>       → copy file
        paste <path>     → paste file
        
        dformat          → format disk
        nuser            → create user
        shutdown         → shutdown OS
        restart          → reboot OS

**🚀 Build & Run Instructions**

        1️⃣ Build Disk + Kernel
        make set
        
        2️⃣ Run a Node Instance (example: node 1)
        make run1
        
        3️⃣ Run Multiple Nodes (shared-disk multinode)
        
        make run1
        make run2
        make run3
        ...
        make runN
