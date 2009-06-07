emit:
	cmp	al,8
	 je emit_backspace
	cmp	al,9
	 je emit_tab
	cmp	al,10
	 je emit_cr

	push	edi	        ; printable character
	mov	ah,byte [attr]
	movzx	edi, word [xy]
	add	edi, [vfb]
	stosw
	add	word [xy], 2
	call	scroll
	call	update_cursor
	pop	edi
	drop
	ret
emit_backspace
	push edi
	sub word [xy], 2
	movzx edi, word [xy]
	add edi, [vfb]
	xor al, al
	stosb
	call update_cursor
	pop edi
	ret
emit_tab
	and dword [xy], 0x0F            ;; round [xy] up to the nearest 16
	add dword [xy], 0x10
	call update_cursor
	ret
emit_cr
	call cr
ret

init_screen:
;
; Get cursor position from BIOS data area
;
	movzx	eax,word [0x44A]        ;columns
	mov     [vw],eax
	movzx	eax,byte [0x484]        ;rows-1
	inc	al
	mov     [vh],eax
	movzx	eax,byte [0x451]        ;cursor row
	mul	dword [vw]
	movzx	edx,byte [0x450]        ;cursor column
	add	eax,edx
	shl	eax,1
	mov     [xy],eax
	ret
;---------------------------------------------------------------
vfb	dd      0xB8000 	        ;video frame buffer (text/gfx)
vw	dd      80
vh	dd      25
xy	dd      0
attr	db      15

;---------------------------------------------------------------
update_cursor:
;
; Physically move the cursor to [x,y], using port I/O
;
; Index port: 0x3d4
; Data port:  0x3d5
;
	push eax
        push edx
	mov	al, 0x0f        ;Index reg.: cursor pos (low byte)
	mov	dx, 0x3d4
	out	dx, al
	mov	ax, word [xy]	     ;Write low byte
	shr	ax, 1
	inc	edx	        ;0x3d5
	out	dx, al
	mov	al, 0x0e        ;Index reg.: cursor pos (high byte)
	dec	edx	        ;0x3d4
	out	dx, al
	mov	al, ah	        ;Write high byte
	inc	edx	        ;0x3d5
	out	dx, al
	pop edx
	pop eax
	ret
;---------------------------------------------------------------
cr:
	push ecx
	push edx
	mov	ax,word [xy]
	xor	dx,dx
	mov	cx,word [vw]
	shl	cx,1
	div	cx	        ;dx = remainder
	mov	ax,word [vw]
	shl	ax,1
	sub	ax,dx
	add	word [xy],ax
	call	scroll
	call	update_cursor
	pop edx
	pop ecx
	ret
;---------------------------------------------------------------
scroll:
;
; If the cursor is past the bottom of the screen, scroll down 1 line.
;
	mov	eax,[vw]
	imul	eax,[vh]
	shl	eax,1
	cmp	word [xy],ax		     ;need to scroll?
	jb	.end

	pusha
	mov	edi,[vfb]	        ;Move rows up 1
	mov	esi,[vw]
	shl	esi,1
	add	esi,edi
	shr	eax,1
	mov	ecx,eax
	sub	ecx,[vw]
	cld
	rep	movsw

	mov	ah,[attr]	        ;Clear bottom row
	xor	al,al
	mov	ecx,[vw]
	rep	stosw

	mov	eax,[vw]		        ;move cursor
	shl	eax,1
	sub     [xy],eax
	popa
.end:	ret
