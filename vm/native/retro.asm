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

process:
  xor eax, eax
  mov ebx, 4
  mov eax, dword [vm_ip]
  mul ebx
  mov ecx, dword image
  add ecx, eax
  mov ecx, dword [ecx]

  cmp ecx, 0
  jz near op_nop

  cmp ecx, 1
  jz near op_lit

  cmp ecx, 2
  jz near op_dup

  cmp ecx, 3
  jz near op_drop

  cmp ecx, 4
  jz near op_swap

  cmp ecx, 5
  jz near op_push

  cmp ecx, 6
  jz near op_pop

  cmp ecx, 7
  jz near op_call

  cmp ecx, 8
  jz near op_jump

  cmp ecx, 9
  jz near op_ret

  cmp ecx, 10
  jz near op_gt

  cmp ecx, 11
  jz near op_lt

  cmp ecx, 12
  jz near op_ne

  cmp ecx, 13
  jz near op_eq

  cmp ecx, 14
  jz near op_fetch

  cmp ecx, 15
  jz near op_store

  cmp ecx, 16
  jz near op_add

  cmp ecx, 17
  jz near op_sub

  cmp ecx, 18
  jz near op_mul

  cmp ecx, 19
  jz near op_divmod

  cmp ecx, 20
  jz near op_and

  cmp ecx, 21
  jz near op_or

  cmp ecx, 22
  jz near op_xor

  cmp ecx, 23
  jz near op_shl

  cmp ecx, 24
  jz near op_shr

  cmp ecx, 25
  jz near op_0ret

  cmp ecx, 26
  jz near op_inc

  cmp ecx, 27
  jz near op_dec

  cmp ecx, 28
  jz near op_in

  cmp ecx, 29
  jz near op_out

  cmp ecx, 30
  jz near op_wait

;mov eax, dword [vm_ip]
;  mov ebx, 4
;mul ebx
;add eax, 48
;call emit
jmp nextop

hang:
  call key
  call emit
  jmp  hang

nextop:
  inc dword [vm_ip]
  cmp dword [vm_ip], 2000
  jl near process
  jmp hang
; -------------------------------------------------------------
op_nop:
  mov eax, 'a'
  call emit
  jmp nextop
op_lit:
  mov eax, 'b'
  call emit
  inc dword [vm_ip]
  jmp nextop
op_dup:
  mov eax, 'c'
  call emit
  jmp nextop
op_drop:
  mov eax, 'd'
  call emit
  jmp nextop
op_swap:
  mov eax, 'e'
  call emit
  jmp nextop
op_push:
  mov eax, 'f'
  call emit
  jmp nextop
op_pop:
  mov eax, 'g'
  call emit
  jmp nextop
op_call:
  mov eax, 'h'
  call emit
  inc dword [vm_ip]
  inc dword [vm_ip]
  xor eax, eax
  mov ebx, 4
  mov eax, dword [vm_ip]
  mul ebx
  mov ecx, dword image
  add ecx, eax
  mov ecx, dword [ecx]
  mov dword [vm_ip], ecx
  jmp nextop
op_jump:
  mov eax, 'i'
  call emit
  inc dword [vm_ip]
  xor eax, eax
  mov ebx, 4
  mov eax, dword [vm_ip]
  mul ebx
  mov ecx, dword image
  add ecx, eax
  mov ecx, dword [ecx]
  mov dword [vm_ip], ecx
  jmp nextop
op_ret
  mov eax, 'j'
  call emit
  jmp nextop
op_gt:
  mov eax, 'k'
  call emit
  inc dword [vm_ip]
  jmp nextop
op_lt:
  mov eax, 'l'
  call emit
  inc dword [vm_ip]
  jmp nextop
op_ne:
  mov eax, 'm'
  call emit
  inc dword [vm_ip]
  jmp nextop
op_eq:
  mov eax, 'n'
  call emit
  inc dword [vm_ip]
  jmp nextop
op_fetch:
  mov eax, 'o'
  call emit
  jmp nextop
op_store:
  mov eax, 'p'
  call emit
  jmp nextop
op_add:
  mov eax, 'q'
  call emit
  jmp nextop
op_sub:
  mov eax, 'r'
  call emit
  jmp nextop
op_mul:
  mov eax, 's'
  call emit
  jmp nextop
op_divmod:
  mov eax, 't'
  call emit
  jmp nextop
op_and:
  mov eax, 'u'
  call emit
  jmp nextop
op_or:
  mov eax, 'v'
  call emit
  jmp nextop
op_xor:
  mov eax, 'w'
  call emit
  jmp nextop
op_shl:
  mov eax, 'x'
  call emit
  jmp nextop
op_shr:
  mov eax, 'y'
  call emit
  jmp nextop
op_0ret:
  mov eax, 'z'
  call emit
  jmp nextop
op_inc:
  mov eax, '1'
  call emit
  jmp nextop
op_dec:
  mov eax, '2'
  call emit
  jmp nextop
op_in:
  mov eax, '3'
  call emit
  jmp nextop
op_out:
  mov eax, '4'
  call emit
  jmp nextop
op_wait:
  mov eax, '5'
  call emit
  jmp nextop
; -------------------------------------------------------------
%include 'drivers/video.inc'
%include 'drivers/keyboard.inc'
; -------------------------------------------------------------
vm_ip   dd 0
vm_sp   dd 0
vm_rsp  dd 0
vm_data times 1024 dd 0
vm_addr times 1024 dd 0
; -------------------------------------------------------------
image incbin '../../bin/retroImage'
