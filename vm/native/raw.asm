; Raw Floppy Bootloader for Retro
; --------------------------------------------------------
START equ 0x11000	        ; ORG of kernel
			        ;
use16			        ;
ORG 0x7C00		        ; Make sure we're where we
jmp 0x0000:start	        ; expect to be...
start:			        ;
      xor ax,ax 	        ; DS = ES = SS = 0
      mov ds,ax 	        ;
      mov ss,ax 	        ;
      mov es,ax 	        ;
      mov sp,7000h	        ;

      mov si, welcome	        ;
      call putstr	        ;
; --------------------------------------------------------
; Load the kernel from the disk
; --------------------------------------------------------
bootload:		        ;
	mov cx,3	        ;# retries
.retry: 		        ;
	push cx 	        ;
	mov ax,7e0h	        ;Load to es:bx -- $7e00
	mov es,ax	        ;
	xor bx,bx	        ;
	xor cx,cx	        ;ch=0 (track)
	inc cl		        ;cl=2 (sector, 1-based)
	inc cl		        ;
	xor dx,dx	        ;dl=0 (drive)  dh=0 (head)
	mov ax,240h	        ;al=35 (sectors)
	mov al,35
	int 0x13	        ;BIOS service $1302 - disk read
			        ;
	jnc .success	        ;
	pop cx		        ;
	loop .retry	        ;
.error: 		        ;
	xor al,al	        ; shut off floppy motor
	mov dx,0x3f2	        ;
	out dx,al	        ;
	mov si, failed	        ; Display 'failed' message
	call putstr	        ;
	jmp $		        ; Hang the system
.success:		        ;
	xor al,al	        ; shut off floppy motor
	mov dx,0x3f2	        ;
	out dx,al	        ;
	mov si, success         ; Display 'success' message
	call putstr
; --------------------------------------------------------
; Switch to protected mode
; --------------------------------------------------------
      cli		        ; Stop interrupts
      call enable_A20	        ; Enable A20
      lgdt [gdt]	        ; Load the GDT
      mov eax,cr0	        ;
      inc eax		        ;
      mov cr0,eax	        ; Switch to pmode
      jmp dword 8:protected     ;
use32
protected:		        ; If we get here, it worked
	mov ax,0x10	        ;
	mov ds,ax	        ; Data segment
	mov es,ax	        ; Extra segment
	mov ss,ax	        ; Stack segment
	mov esp,0x50000         ; Return stack
	cld		        ;
	mov esi,7e00h	        ; Move kernel from $7e00 to
	mov edi,START	        ; $110000
	mov edx,edi	        ;
	mov ecx,(kernel.end-kernel)/4
	cld		        ;
	rep movsd	        ;
	call edx	        ; Jump to kernel
	jmp $		        ; Problem? Just Hang!

; --------------------------------------------------------
; Data structures
; --------------------------------------------------------
gdt:	dw 0x18
	dd gdt
	dw 0
	dw 0xffff, 0, 0x9a00, 0xcf      ;code
	dw 0xffff, 0, 0x9200, 0xcf      ;data
welcome db 'Loading Retro...     ', 0
success db 'Success!'
crlf	db 10, 13, 0
failed	db 'Failed!', 0
; --------------------------------------------------------
; Code to enable A20
; --------------------------------------------------------
use16
enable_A20:
	call	a20wait
	mov	al,0xAD
	out     0x64,al

	call	a20wait
	mov	al,0xD0
	out     0x64,al

	call	a20wait2
	in	al,0x60
	push	eax

	call	a20wait
	mov	al,0xD1
	out     0x64,al

	call	a20wait
	pop	eax
	or	al,2
	out     0x60,al

	call	a20wait
	mov	al,0xAE
	out     0x64,al

	call	a20wait
	ret

a20wait:
.l0:	mov	ecx,65536
.l1:	in	al,0x64
	test	al,2
	jz	.l2
	loop	.l1
	jmp	.l0
.l2:	ret


a20wait2:
.l0:	mov	ecx,65536
.l1:	in	al,0x64
	test	al,1
	jnz	.l2
	loop	.l1
	jmp	.l0
.l2:	ret



putstr: ; si = string
	lodsb		 ; al=str[i]
	or al, al	 ; if(al==0)
	jz .ret 	 ;   return
	mov ah, 0x0E    ; put char
	mov bh, 0x00    ;
	mov bl, 14      ; bl = attribute
	int 0x10        ;
	jmp putstr	   ; repeat
  .ret:
ret

; --------------------------------------------------------
; Go to end of sector
; --------------------------------------------------------
times   510-($-$$)	    db 0

; --------------------------------------------------------
; The 'signature' of the boot sector
; Not all systems require this, but *most do*
; --------------------------------------------------------
dw 0xAA55

; --------------------------------------------------------
; Include the precompiled kernel image AFTER the 512 bytes
; for the boot sector.
; --------------------------------------------------------
kernel:
 align 4
 incbin "retro"
 align 4
.end:
