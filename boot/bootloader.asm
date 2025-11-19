[BITS 16]
[ORG 0x7C00]

start:
    cli
    xor ax,ax
    mov ds,ax
    mov es,ax
    mov ss,ax
    mov sp,0x7C00

    mov si,0
    call load_kernel

    call enable_protected_mode

    jmp 0x08:0x1000

load_kernel:
    mov ah, 0x02
    mov al,50
    mov ch,0
    mov cl, 2
    mov dh, 0
    mov dl, 0x80
    mov bx,0x1000
    int 0x13
    ret

enable_protected_mode:
    cli
    lgdt [gdt_descriptor]

    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp 0x08:protected_mode_entry

;Doing the setup for 32 bit mode

[BITS 32]
protected_mode_entry:
    mov ax,0x10
    mov ds,ax
    mov es,ax
    mov ss,ax
    mov esp,0x90000

    jmp 0x08:0x1000

gdt_start:
    dq 0x0000000000000000
    dq 0x00CF9A000000FFFF
    dq 0x00CF92000000FFFF

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

gdt_end:

times 510 - ($ - $$) db 0
dw 0xAA55
