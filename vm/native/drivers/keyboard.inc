;---------------------------------------------------------------
; ( KEY, a generic keyboard driver )
key:
	  xor eax,eax	        ;  clear eax
key_a:	 in al,64h		 ;  Is any data waiting?
	  test al,1		        ;  Is character = ASCII 0?
	  jz key_a 	        ;  Yes? Try again
	in al,60h	        ;  Otherwise, read scancode
	xor edx,edx	        ;  edx: 0=make, 1=break
	test al,80h	        ;  Is character = HEX 80h?
	jz key_b		        ;  Skip the next line
	inc edx 	        ;  Update edx
key_b:	 and al,7Fh		 ;  Filters to handle
	cmp al,39h	        ;  the ignored keys
	  ja key_a 	        ;  We just try another key
	mov ecx,[board]         ;  Load the keymap
	mov al,[ecx+eax]        ;  Get the key ASCII char
	  or al,al		        ;  Is is = 0?
	js key_shift		        ;  No, use CAPITALS
	jz key_a		        ;  Ignore 0's
	or dl,dl		        ;  Filter for break code
	jnz key_a		        ;  Ignore break code
	  ret
key_shift:  mov ecx,[edx*4 + shifts]	 ;  Load the CAPITAL keymap
	mov [board],ecx 	        ;  Store into BOARD pointer
	jmp key_a			  ;  And try again
shifts dd shift,alpha
board dd alpha
alpha:
  db 0,27,"1234567890-=",8	        ;00-0E
  db 9,"qwertyuiop[]",10	        ;0F-1C
  db 0,"asdfghjkl;'`"		        ;1D-29
  db -1,"\zxcvbnm,./",-1,"*",0,32,-2    ;2A-3A
shift:
  db 0,27,"!@#$%^&*()_+",8	        ;00-0E
  db 9,"QWERTYUIOP{}",10	        ;0F-1C
  db 0,'ASDFGHJKL:"~'		        ;1D-29
  db -1,"|ZXCVBNM<>?",-1,"*",0,32,-2    ;2A-3A
;---------------------------------------------------------------
