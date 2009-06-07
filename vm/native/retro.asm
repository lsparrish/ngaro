; -------------------------------------------------------------
; Virtual Machine for Retro
;
; This is a native, x86-based implementation designed to be
; used without a host operating system.
; -------------------------------------------------------------
use32
ORG 0x11000
MAIN:
  call init_screen
hang:
  call key
  call emit
  jmp  hang
; -------------------------------------------------------------
%include 'drivers/video.inc'
%include 'drivers/keyboard.inc'
; -------------------------------------------------------------
vm_call_table
  dd op_nop      ;  0
  dd op_lit      ;  1
  dd op_dup      ;  2
  dd op_drop     ;  3
  dd op_swap     ;  4
  dd op_push     ;  5
  dd op_pop      ;  6
  dd op_call     ;  7
  dd op_jump     ;  8
  dd op_ret      ;  9
  dd op_gt       ; 10
  dd op_lt       ; 11
  dd op_ne       ; 12
  dd op_eq       ; 13
  dd op_fetch    ; 14
  dd op_store    ; 15
  dd op_add      ; 16
  dd op_sub      ; 17
  dd op_mul      ; 18
  dd op_divmod   ; 19
  dd op_and      ; 20
  dd op_or       ; 21
  dd op_xor      ; 22
  dd op_shl      ; 23
  dd op_shr      ; 24
  dd op_0ret     ; 25
  dd op_inc      ; 26
  dd op_dec      ; 27
  dd op_in       ; 28
  dd op_out      ; 29
  dd op_wait     ; 30
; -------------------------------------------------------------
op_nop:
  ret
op_lit:
  ret
op_dup:
  ret
op_drop:
  ret
op_swap:
  ret
op_push:
  ret
op_pop:
  ret
op_call:
  ret
op_jump:
  ret
op_ret:
  ret
op_gt:
  ret
op_lt:
  ret
op_ne:
  ret
op_eq:
  ret
op_fetch:
  ret
op_store:
  ret
op_add:
  ret
op_sub:
  ret
op_mul:
  ret
op_divmod:
  ret
op_and:
  ret
op_or:
  ret
op_xor:
  ret
op_shl:
  ret
op_shr:
  ret
op_0ret:
  ret
op_inc:
  ret
op_dec:
  ret
op_in:
  ret
op_out:
  ret
op_wait:
  ret

; -------------------------------------------------------------
vm_ip   dd 0
vm_sp   dd 0
vm_rsp  dd 0
vm_data times 1024 dd 0
vm_addr times 1024 dd 0
; -------------------------------------------------------------
image incbin '../../bin/retroImage'
