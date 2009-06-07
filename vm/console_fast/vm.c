/******************************************************
 * Ngaro
 *
 *|F|
 *|F| FILE: vm.c
 *|F|
 *
 * Threading interpreter by Matthias Schirm.
 * Released into the public domain
 ******************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <termios.h>

#include "functions.h"
#include "vm.h"


/* Variables specific to the VM */

VM_STATE vm;



/******************************************************
 *|F| void init_vm()
 *
 * This zeros out everything in the VM_STATE struct
 * to give us a known starting point.
 ******************************************************/

void init_vm()
{
   int a;
   vm.ip = vm.sp = vm.rsp = 0;
   for (a = 0; a < STACK_DEPTH; a++)
      vm.data[a] = 0;
   for (a = 0; a < ADDRESSES; a++)
      vm.address[a] = 0;
   for (a = 0; a < IMAGE_SIZE; a++)
      vm.image[a] = 0;
   for (a = 0; a < 1024; a++)
      vm.ports[a] = 0;
}


/******************************************************
 *|F| void vm_process (void)
 *
 * Token threading interpreter
 * + static data-stack caching (first two elements)
 * + static adress-stack caching (per link register)
 * + dynamic generation of new instructions (dtc)
 * + instructions for direct stack adressing
 ******************************************************/

/* bytecode to adress arrays for token and direct threading */

void **vm_process_ct_otab[255];
void **vm_process_tr_otab[255];

void vm_process (void)
{
  /* scratch variables */

  int a, b, c;

  /* register file:

	ip  = instruction pointer         (token threading)
	sp  = data stack pointer          (to third stack element)
	rsp = return adress stack pointer (to third stack element)
        IAC = integer accumulator         (cached first stack element)
        IOP = integer operand             (cached second stack element)
        LNK = return adress		  (link register for LCALL and LRETURN)

        image  = pointer to the bytecode image
        data   = start adress of the data stack
        adress = start adress of the return-adress stack

	INST  = pointer to a token threading primitive
        TINST = pointer to a direct threading stream */

  register int ip;   ip = vm.ip;
  register int sp;   sp = vm.sp;
  register int rsp; rsp = vm.rsp;

  register int *image   = vm.image;
  register int *data    = vm.data;
  register int *address = vm.address;

  register int IAC; IAC = 0;
  register int IOP; IOP = 0;
  register int LNK; LNK = 0;

  register void  *INST;
  register void **TINST = 0;

  /* direct threading streams -
     tptr_a = pointer to free space in stream array
     tptr_b = start adress of the last generated stream */

  void *tstream [MAX_STREAM * STREAM_LEN];
  int   tptr_a = 0;
  int   tptr_b = 0;

  /* initialisation of bytecode to DTC adress table */

  vm_process_tr_otab[VM_NOP]       = &&tVM_NOP;
  vm_process_tr_otab[VM_LIT]       = &&tVM_LIT;
  vm_process_tr_otab[VM_DUP]       = &&tVM_DUP;
  vm_process_tr_otab[VM_DROP]      = &&tVM_DROP;
  vm_process_tr_otab[VM_SWAP]      = &&tVM_SWAP;
  vm_process_tr_otab[VM_PUSH]      = &&tVM_PUSH;
  vm_process_tr_otab[VM_POP]       = &&tVM_POP;
  vm_process_tr_otab[VM_CALL]      = &&tVM_ILLEGAL;
  vm_process_tr_otab[VM_JUMP]      = &&tVM_ILLEGAL;
  vm_process_tr_otab[VM_RETURN]    = &&tVM_ILLEGAL;
  vm_process_tr_otab[VM_GT_JUMP]   = &&tVM_GT_JUMP;
  vm_process_tr_otab[VM_LT_JUMP]   = &&tVM_LT_JUMP;
  vm_process_tr_otab[VM_NE_JUMP]   = &&tVM_NE_JUMP;
  vm_process_tr_otab[VM_EQ_JUMP]   = &&tVM_EQ_JUMP;
  vm_process_tr_otab[VM_FETCH]     = &&tVM_FETCH;
  vm_process_tr_otab[VM_STORE]     = &&tVM_STORE;
  vm_process_tr_otab[VM_ADD]       = &&tVM_ADD;
  vm_process_tr_otab[VM_SUB]       = &&tVM_SUB;
  vm_process_tr_otab[VM_MUL]       = &&tVM_MUL;
  vm_process_tr_otab[VM_DIVMOD]    = &&tVM_DIVMOD;
  vm_process_tr_otab[VM_AND]       = &&tVM_AND;
  vm_process_tr_otab[VM_OR]        = &&tVM_OR;
  vm_process_tr_otab[VM_XOR]       = &&tVM_XOR;
  vm_process_tr_otab[VM_SHL]       = &&tVM_SHL;
  vm_process_tr_otab[VM_SHR]       = &&tVM_SHR;
  vm_process_tr_otab[VM_ZERO_EXIT] = &&tVM_ILLEGAL;
  vm_process_tr_otab[VM_INC]       = &&tVM_INC;
  vm_process_tr_otab[VM_DEC]       = &&tVM_DEC;
  vm_process_tr_otab[VM_IN]        = &&tVM_IN;
  vm_process_tr_otab[VM_OUT]       = &&tVM_OUT;
  vm_process_tr_otab[VM_WAIT]      = &&tVM_WAIT;
  vm_process_tr_otab[VM_PICK]      = &&tVM_PICK;
  vm_process_tr_otab[VM_BACK]      = &&tVM_BACK;
  vm_process_tr_otab[VM_ROT]       = &&tVM_ROT;
  vm_process_tr_otab[VM_SINST]     = &&tVM_ILLEGAL;
  vm_process_tr_otab[VM_XOP]       = &&tVM_XOP;
  vm_process_tr_otab[VM_EXE]       = &&tVM_EXE;
  vm_process_ct_otab[VM_TRESET]    = &&tVM_TRESET;
  vm_process_tr_otab[VM_LI_IAC]    = &&tVM_LI_IAC;
  vm_process_tr_otab[VM_LI_IOP]    = &&tVM_LI_IOP;
  vm_process_tr_otab[VM_PICK_IAC]  = &&tVM_PICK_IAC;
  vm_process_tr_otab[VM_PICK_IOP]  = &&tVM_PICK_IOP;
  vm_process_tr_otab[VM_RADD]      = &&tVM_RADD;
  vm_process_tr_otab[VM_RSUB]      = &&tVM_RSUB;
  vm_process_tr_otab[VM_RMUL]      = &&tVM_RMUL;
  vm_process_tr_otab[VM_RAND]      = &&tVM_RAND;
  vm_process_tr_otab[VM_ROR]       = &&tVM_ROR;
  vm_process_tr_otab[VM_RXOR]      = &&tVM_RXOR;
  vm_process_tr_otab[VM_PRINT]     = &&tVM_PRINT;
  vm_process_tr_otab[VM_DEFAULT]   = &&tVM_DEFAULT;

  /* initialisation of bytecode to TTC adress table */

  vm_process_ct_otab[VM_NOP]       = &&fVM_NOP;
  vm_process_ct_otab[VM_LIT]       = &&fVM_LIT;
  vm_process_ct_otab[VM_DUP]       = &&fVM_DUP;
  vm_process_ct_otab[VM_DROP]      = &&fVM_DROP;
  vm_process_ct_otab[VM_SWAP]      = &&fVM_SWAP;
  vm_process_ct_otab[VM_PUSH]      = &&fVM_PUSH;
  vm_process_ct_otab[VM_POP]       = &&fVM_POP;
  vm_process_ct_otab[VM_CALL]      = &&fVM_CALL;
  vm_process_ct_otab[VM_JUMP]      = &&fVM_JUMP;
  vm_process_ct_otab[VM_RETURN]    = &&fVM_RETURN;
  vm_process_ct_otab[VM_GT_JUMP]   = &&fVM_GT_JUMP;
  vm_process_ct_otab[VM_LT_JUMP]   = &&fVM_LT_JUMP;
  vm_process_ct_otab[VM_NE_JUMP]   = &&fVM_NE_JUMP;
  vm_process_ct_otab[VM_EQ_JUMP]   = &&fVM_EQ_JUMP;
  vm_process_ct_otab[VM_FETCH]     = &&fVM_FETCH;
  vm_process_ct_otab[VM_STORE]     = &&fVM_STORE;
  vm_process_ct_otab[VM_ADD]       = &&fVM_ADD;
  vm_process_ct_otab[VM_SUB]       = &&fVM_SUB;
  vm_process_ct_otab[VM_MUL]       = &&fVM_MUL;
  vm_process_ct_otab[VM_DIVMOD]    = &&fVM_DIVMOD;
  vm_process_ct_otab[VM_AND]       = &&fVM_AND;
  vm_process_ct_otab[VM_OR]        = &&fVM_OR;
  vm_process_ct_otab[VM_XOR]       = &&fVM_XOR;
  vm_process_ct_otab[VM_SHL]       = &&fVM_SHL;
  vm_process_ct_otab[VM_SHR]       = &&fVM_SHR;
  vm_process_ct_otab[VM_ZERO_EXIT] = &&fVM_ZERO_EXIT;
  vm_process_ct_otab[VM_INC]       = &&fVM_INC;
  vm_process_ct_otab[VM_DEC]       = &&fVM_DEC;
  vm_process_ct_otab[VM_IN]        = &&fVM_IN;
  vm_process_ct_otab[VM_OUT]       = &&fVM_OUT;
  vm_process_ct_otab[VM_WAIT]      = &&fVM_WAIT;
  vm_process_ct_otab[VM_PICK]      = &&fVM_PICK;
  vm_process_ct_otab[VM_PICK]      = &&fVM_BACK;
  vm_process_ct_otab[VM_ROT]       = &&fVM_ROT;
  vm_process_ct_otab[VM_SINST]     = &&fVM_SINST;
  vm_process_ct_otab[VM_XOP]       = &&fVM_XOP;
  vm_process_ct_otab[VM_EXE]       = &&fVM_EXE;
  vm_process_ct_otab[VM_TRESET]    = &&fVM_TRESET;
  vm_process_ct_otab[VM_LI_IAC]    = &&fVM_LI_IAC;
  vm_process_ct_otab[VM_LI_IOP]    = &&fVM_LI_IOP;
  vm_process_ct_otab[VM_PICK_IAC]  = &&fVM_PICK_IAC;
  vm_process_ct_otab[VM_PICK_IOP]  = &&fVM_PICK_IOP;
  vm_process_ct_otab[VM_RADD]      = &&fVM_RADD;
  vm_process_ct_otab[VM_RSUB]      = &&fVM_RSUB;
  vm_process_ct_otab[VM_RMUL]      = &&fVM_RMUL;
  vm_process_ct_otab[VM_RAND]      = &&fVM_RAND;
  vm_process_ct_otab[VM_ROR]       = &&fVM_ROR;
  vm_process_ct_otab[VM_RXOR]      = &&fVM_RXOR;
  vm_process_ct_otab[VM_PRINT]     = &&fVM_PRINT;
  vm_process_ct_otab[VM_LCALL]     = &&fVM_LCALL;
  vm_process_ct_otab[VM_LRETURN]   = &&fVM_LRETURN;
  vm_process_ct_otab[VM_DEFAULT]   = &&fVM_DEFAULT;

  /* vm exit point */

  vm.image[IMAGE_SIZE] = VM_DEFAULT;

  /* threading though the vm image */

  NEXT

  /* direct threading primitives */

  /***************************************************/
  /* NOP    Does Nothing. Used for padding           */
  /* Opcode: 0        Stack:  -       Address: -     */
  /***************************************************/

  tVM_NOP:	TNEXT

  /***************************************************/
  /* LIT    Push the value in the following cell to  */
  /*        the stack                                */
  /* Opcode: 1 n      Stack: -n       Address: -     */
  /***************************************************/

  tVM_LIT:	data[++sp] = IOP;
		IOP = IAC;
                IAC = (int) *TINST++;
         	TNEXT

  /***************************************************/
  /* DUP    Duplicate the value on the top of the    */
  /*        stack                                    */
  /* Opcode: 2        Stack: n-nn     Address: -     */
  /***************************************************/

  tVM_DUP:	data[++sp] = IOP;
         	IOP = IAC;
         	TNEXT

  /***************************************************/
  /* DROP   Drop the value on the top of the stack   */
  /* Opcode: 3        Stack: n-       Address: -     */
  /***************************************************/

  tVM_DROP:	IAC = IOP;
		IOP = data[sp--];
         	TNEXT

  /***************************************************/
  /* SWAP   Exchange the top two values on the stack */
  /* Opcode: 4        Stack: xy-yx    Address: -     */
  /***************************************************/

  tVM_SWAP:	a = IAC;
         	IAC = IOP;
         	IOP = a;
         	TNEXT

  /***************************************************/
  /* PUSH   Move the top value on the stack to the   */
  /*        address stack. Remove it from the data   */
  /*        stack.                                   */
  /* Opcode: 5        Stack: n-       Address: -n    */
  /***************************************************/

  tVM_PUSH:	address[++rsp] = IAC;
         	IAC = IOP;
         	IOP = data[sp--];
         	TNEXT

  /***************************************************/
  /* POP    Move the top value from the address      */
  /*        stack to the data stack. Remove it from  */
  /*        the address stack.                       */
  /* Opcode: 6        Stack: -n       Address: n-    */
  /***************************************************/

  tVM_POP:	data[++sp] = IOP;
		IOP = IAC;
         	IAC = address[rsp--];
         	TNEXT

  /***************************************************/
  /* >JUMP  Jump to the address in the following     */
  /*        cell if NOS > TOS.                       */
  /* Opcode: 10 a      Stack: xy-     Address: -     */
  /***************************************************/

  tVM_GT_JUMP:	a = (int) *TINST++;
  		if (IOP > IAC) TINST = tstream + a;
  		IAC = data[sp--];
  		IOP = data[sp--];
  		TNEXT

  /***************************************************/
  /* <JUMP  Jump to the address in the following     */
  /*        cell if NOS < TOS.                       */
  /* Opcode: 11 a      Stack: xy-     Address: -     */
  /***************************************************/

  tVM_LT_JUMP:	a = (int) *TINST++;
  		if (IOP < IAC) TINST = tstream + a;
  		IAC = data[sp--];
  		IOP = data[sp--];
  		TNEXT

  /***************************************************/
  /* !JUMP  Jump to the address in the following     */
  /*        cell if NOS <> TOS.                      */
  /* Opcode: 12 a      Stack: xy-     Address: -     */
  /***************************************************/

  tVM_NE_JUMP:	a = (int) *TINST++;
  		if (IOP != IAC) TINST = tstream + a;
  		IAC = data[sp--];
  		IOP = data[sp--];
  		TNEXT

  /***************************************************/
  /* =JUMP  Jump to the address in the following     */
  /*        cell if NOS = TOS.                       */
  /* Opcode: 13 a      Stack: xy-     Address: -     */
  /***************************************************/

  tVM_EQ_JUMP:	a = (int) *TINST++;
  		if (IOP == IAC) TINST = tstream + a;
  		IAC = data[sp--];
  		IOP = data[sp--];
  		TNEXT

  /***************************************************/
  /* @      Fetch a value from a memory location     */
  /* Opcode: 14        Stack: a-n     Address: -     */
  /***************************************************/

  tVM_FETCH:	IAC = image[IAC];
         	TNEXT

  /***************************************************/
  /* !      Store a value to a memory location       */
  /* Opcode: 15        Stack: na-     Address: -     */
  /***************************************************/

  tVM_STORE:	image[IAC] = IOP;
         	IAC = data[sp--];
		IOP = data[sp--];
         	TNEXT

  /***************************************************/
  /* +      Add TOS and NOS, leaving the result      */
  /* Opcode: 16        Stack: xy-z    Address: -     */
  /***************************************************/

  tVM_ADD:	IOP += IAC;
		IAC = IOP;
         	IOP = data[sp--];
         	TNEXT

  /***************************************************/
  /* -      Subtract TOS from NOS, leaving the result*/
  /* Opcode: 17        Stack: xy-z    Address: -     */
  /***************************************************/

  tVM_SUB:	IOP -= IAC;
		IAC = IOP;
         	IOP = data[sp--];
         	TNEXT

  /***************************************************/
  /* *      Multiply TOS by NOS, leaving the result  */
  /* Opcode: 18        Stack: xy-z    Address: -     */
  /***************************************************/

  tVM_MUL:	IOP *= IAC;
		IAC = IOP;
         	IOP = data[sp--];
         	TNEXT

  /***************************************************/
  /* /MOD   Divide NOS by TOS, leaving the quotient  */
  /*        and remainder.                           */
  /* Opcode: 19        Stack: xy-qr   Address: -     */
  /***************************************************/

  tVM_DIVMOD:	a = IAC;
         	b = IOP;
         	IAC = b / a;
         	IOP = b % a;
         	TNEXT

  /***************************************************/
  /* AND    Perform a bitwise and operation on TOS   */
  /*        and NOS.                                 */
  /* Opcode: 20        Stack: xy-z    Address: -     */
  /***************************************************/

  tVM_AND:	IAC = IAC & IOP;
		IOP = data[sp--];
         	TNEXT

  /***************************************************/
  /* OR     Perform a bitwise or operation on TOS    */
  /*        and NOS.                                 */
  /* Opcode: 21        Stack: xy-z    Address: -     */
  /***************************************************/

  tVM_OR:	IAC = IAC | IOP;
		IOP = data[sp--];
         	TNEXT

  /***************************************************/
  /* XOR    Perform a bitwise xor operation on TOS   */
  /*        and NOS.                                 */
  /* Opcode: 22        Stack: xy-z    Address: -     */
  /***************************************************/

  tVM_XOR:	IAC = IAC ^ IOP;
		IOP = data[sp--];
         	TNEXT

  /***************************************************/
  /* <<    Shift NOS left by TOS bits.               */
  /* Opcode: 23        Stack: xy-z    Address: -     */
  /***************************************************/

  tVM_SHL:	IAC = IOP << IAC;
		IOP = data[sp--];
         	TNEXT

  /***************************************************/
  /* >>    Shift NOS right by TOS bits.              */
  /* Opcode: 24        Stack: xy-z    Address: -     */
  /***************************************************/

  tVM_SHR:	IAC = IOP >>= IAC;
		IOP = data[sp--];
         	TNEXT

  /***************************************************/
  /* 1+    Increase TOS by 1                         */
  /* Opcode: 26        Stack: x-y     Address: -     */
  /***************************************************/

  tVM_INC:	IAC += 1;
         	TNEXT

  /***************************************************/
  /* 1-    Decrease TOS by 1                         */
  /* Opcode: 27        Stack: x-y     Address: -     */
  /***************************************************/

  tVM_DEC:	IAC -= 1;
         	TNEXT

  /***************************************************/
  /* IN    Read a value from an I/O port             */
  /* Opcode: 28        Stack: p-n     Address: -     */
  /***************************************************/

  tVM_IN:	a = IAC;
         	IAC = vm.ports[a];
         	vm.ports[a] = 0;
         	TNEXT

  /***************************************************/
  /* OUT   Send a value to an I/O port               */
  /* Opcode: 29        Stack: np-     Address: -     */
  /***************************************************/

  tVM_OUT:	vm.ports[0] = 0;
         	vm.ports[IAC] = IOP;
         	IAC = data[sp--];
		IOP = data[sp--];
         	TNEXT

  /***************************************************/
  /* WAIT  Wait for an I/O event to occur.           */
  /* Opcode: 30        Stack: -       Address: -     */
  /***************************************************/

  tVM_WAIT:     if (vm.ports[0] == 0 && vm.ports[1] == 1)
                {
                  vm.ports[1] = dev_getch();
                  vm.ports[0] = 1;
                }

                /* Output (character generator) */
                if (vm.ports[2] == 1)
                {
    		  c = IAC;
		  IAC = IOP;
		  IOP = data[sp--];
    		  dev_putch(c);
    		  vm.ports[2] = 0;
    		  vm.ports[0] = 1;
                }

                /* Save Image */
                if (vm.ports[4] == 1)
                {
                  vm_save_image(vm.filename);
                  vm.ports[4] = 0;
                  vm.ports[0] = 1;
                }

                /* Capabilities */
                if (vm.ports[5] == -1)
                {
                  vm.ports[5] = IMAGE_SIZE;
                  vm.ports[0] = 1;
                }

                /* The framebuffer related bits aren't supported, so return 0
                   for them. */
                if (vm.ports[5] == -2 || vm.ports[5] == -3 || vm.ports[5] == -4)
                {
                  vm.ports[5] = 0;
                  vm.ports[0] = 1;
                }

                /* Data & Return Stack Depth */
                if (vm.ports[5] == -5)
                {
                  vm.ports[5] = vm.sp;
                  vm.ports[0] = 1;
                }
                if (vm.ports[5] == -6)
                {
                  vm.ports[5] = vm.rsp;
                  vm.ports[0] = 1;
                }
                dev_refresh();
         	TNEXT

  /***************************************************/
  /* PICK  put 3-n'th stack element on top of stack  */
  /* Opcode: 31 n      Stack: -n      Address: -     */
  /***************************************************/

  tVM_PICK:	a = (int) *TINST++;
		data[++sp] = IOP;
         	IOP = IAC;
		IAC = data[a];
		TNEXT
  /***************************************************/
  /* BACK  store tos in 3-n'th stack element         */
  /* Opcode: 32 n      Stack: -      Address: -      */
  /***************************************************/

  tVM_BACK:	a = (int) *TINST++;
		data[a] = IAC;
		TNEXT;

  /***************************************************/
  /* ROT  exchange tos and 3'th stack element        */
  /* Opcode: 33        Stack: -      Address: -      */
  /***************************************************/

  tVM_ROT:      a = IAC;
		IAC = data[sp];
		data[sp] = a;
		TNEXT

  /***************************************************/
  /* XOP  opcode prefix for dynamic instructions     */
  /* Opcode: 34 opcode Stack: -      Address: -      */
  /***************************************************/

  tVM_XOP:	TINST = tstream + image[ip++];
		TNEXT

  /***************************************************/
  /* EXE  opcode prefix for dynamic instructions     */
  /* Opcode: 35        Stack: opcode- Address: -     */
  /***************************************************/

  tVM_EXE:	TINST = tstream + IAC;
		IAC = IOP;
		IOP = data[sp--];
		TNEXT

  /***************************************************/
  /* TRESET  release newest instruction stream       */
  /* Opcode: 36        Stack: -      Address: -      */
  /***************************************************/

  tVM_TRESET:	tptr_a = tptr_b;
		TNEXT

  /***************************************************/
  /* LIAC   Immediate load of first stack element    */
  /*        without stack update                     */
  /* Opcode: 38 n      Stack: -n _    Address: -     */
  /***************************************************/

  tVM_LI_IAC:	IAC = (int) *TINST++;
		TNEXT

  /***************************************************/
  /* LIOP  immediate load of second stack element    */
  /*       without stack update                      */
  /* Opcode: 39 n      Stack: - _ n    Address: -    */
  /***************************************************/

  tVM_LI_IOP:	IOP = (int) *TINST++;
		TNEXT

  /***************************************************/
  /* PCKIAC  pick 3+num'th stack element on top      */
  /*         without stack update                    */
  /* Opcode: 40 num      Stack: - n    Address: -    */
  /***************************************************/

  tVM_PICK_IAC: a = (int) *TINST++;
		IAC = data[a];
		TNEXT

  /***************************************************/
  /* PCKIOP  load 3+num'th stack element in second   */
  /*         stack elment without stack update       */
  /* Opcode: 41 num      Stack: - n    Address: -    */
  /***************************************************/

  tVM_PICK_IOP: a = (int) *TINST++;
		IOP = data[a];
		TNEXT

  /***************************************************/
  /* RADD    add TOS and NOS (no stack update)       */
  /* Opcode: 42          Stack: x y - z  Address: -  */
  /***************************************************/

  tVM_RADD:	IAC = IAC + IOP;
		TNEXT

  /***************************************************/
  /* RSUB    subtract TOS and NOS (no stack update)  */
  /* Opcode: 43          Stack: x y - z  Address: -  */
  /***************************************************/

  tVM_RSUB:	IAC = IOP - IAC;
		TNEXT

  /***************************************************/
  /* RMUL    multiply TOS and NOS (no stack update)  */
  /* Opcode: 44          Stack: x y - z  Address: -  */
  /***************************************************/

  tVM_RMUL:	IAC = IAC * IOP;
		TNEXT

  /***************************************************/
  /* RAND   Perform a bitwise and operation on TOS   */
  /*        and NOS without stack update.            */
  /* Opcode: 45          Stack: x y - z  Address: -  */
  /***************************************************/

  tVM_RAND:	IAC = IAC & (int) *TINST++;
		TNEXT

  /***************************************************/
  /* ROR    Perform a bitwise or operation on TOS    */
  /*        and NOS without stack update.            */
  /* Opcode: 46          Stack: x y - z  Address: -  */
  /***************************************************/

  tVM_ROR:	IAC = IAC | (int) *TINST++;
		TNEXT

  /***************************************************/
  /* RXOR   Perform a bitwise xor operation on TOS   */
  /*        and NOS without stack update.            */
  /* Opcode: 47          Stack: x y - z  Address: -  */
  /***************************************************/

  tVM_RXOR:	IAC = IAC ^ (int) *TINST++;
		TNEXT

  /***************************************************/
  /* PRINT  for debugging                            */
  /* Opcode: -           Stack: -  Address: -        */
  /***************************************************/

  tVM_PRINT:	printf ("%i\n",IAC);
		TNEXT

 /***************************************************/
  /* ILLEGAL  illegal instruction handler, fall back */
  /* for all branches which can't be executed        */
  /* directly in a stream because there adress       */
  /* doesn't relate to a correct stream offset       */
  /* Opcode: -         Stack: -      Address: -      */
  /***************************************************/

  tVM_ILLEGAL:	printf ("[XOP,EXE] Branch opcode in DTC stream detected !\n");
		exit (-3);

  tVM_EXIT:	NEXT

  /***************************************************/
  /* DEFAULT  immediate interpreter exit             */
  /* Opcode: -         Stack: -      Address: -      */
  /***************************************************/

  tVM_DEFAULT:	goto fVM_DEFAULT;


  /* token-threading bytecode primitives */

  fVM_NOP:	NEXT
  fVM_LIT:	data[++sp] = IOP;
		IOP = IAC;
                IAC = image[ip++];
         	NEXT
  fVM_DUP:	data[++sp] = IOP;
         	IOP = IAC;
         	NEXT
  fVM_DROP:	IAC = IOP;
		IOP = data[sp--];
         	NEXT
  fVM_SWAP:	a = IAC;
         	IAC = IOP;
         	IOP = a;
         	NEXT
  fVM_PUSH:	address[++rsp] = IAC;
         	IAC = IOP;
         	IOP = data[sp--];
         	NEXT
  fVM_POP:	data[++sp] = IOP;
		IOP = IAC;
         	IAC = address[rsp--];
         	NEXT

  /***************************************************/
  /* CALL   Call a subroutine whose address is given */
  /*        in the following cell. Push the address  */
  /*        following this instruction to the address*/
  /*         stack.                                  */
  /* Opcode: 7 a       Stack: -      Address: -a     */
  /***************************************************/

  fVM_CALL:	address[++rsp] = ip++;
         	ip = image[ip-1];
         	NEXT

  /***************************************************/
  /* JUMP   Unconditional jump to the address given  */
  /*        in the following cell.                   */
  /* Opcode: 8 a       Stack: -       Address: -     */
  /***************************************************/

  fVM_JUMP:	ip = image[ip];
         	NEXT

  /***************************************************/
  /* ;      Return from a subroutine. Control is     */
  /*        passed to the address on the top of the  */
  /*        address stack.                           */
  /* Opcode: 9         Stack: -       Address: a-    */
  /***************************************************/

  fVM_RETURN:	ip = address[rsp--]+1;
         	NEXT
  fVM_GT_JUMP:	if(IOP > IAC)
		{
           	  ip = image[ip];
         	  IAC = data[sp--];
		  IOP = data[sp--];
		  NEXT
		}
         	ip++;
		IAC = data[sp--];
		IOP = data[sp--];
		NEXT
  fVM_LT_JUMP:	if(IOP < IAC)
		{
           	  ip = image[ip];
         	  IAC = data[sp--];
		  IOP = data[sp--];
         	  NEXT
		}
		ip++;
		IAC = data[sp--];
		IOP = data[sp--];
		NEXT
  fVM_NE_JUMP:	if(IAC != IOP)
		{
           	  ip = image[ip];
         	  IAC = data[sp--];
		  IOP = data[sp--];
         	  NEXT
		}
		ip++;
		IAC = data[sp--];
		IOP = data[sp--];
		NEXT
  fVM_EQ_JUMP:	if(IAC == IOP)
		{
           	  ip = image[ip];
         	  IAC = data[sp--];
		  IOP = data[sp--];
         	  NEXT
		}
		ip++;
		IAC = data[sp--];
		IOP = data[sp--];
		NEXT
  fVM_FETCH:	IAC = image[IAC];
         	NEXT
  fVM_STORE:	image[IAC] = IOP;
         	IAC = data[sp--];
		IOP = data[sp--];
         	NEXT
  fVM_ADD:	IOP += IAC;
		IAC = IOP;
         	IOP = data[sp--];
         	NEXT
  fVM_SUB:	IOP -= IAC;
		IAC = IOP;
         	IOP = data[sp--];
         	NEXT
  fVM_MUL:	IOP *= IAC;
		IAC = IOP;
         	IOP = data[sp--];
         	NEXT
  fVM_DIVMOD:	a = IAC;
         	b = IOP;
         	IAC = b / a;
         	IOP = b % a;
         	NEXT
  fVM_AND:	IAC = IAC & IOP;
		IOP = data[sp--];
         	NEXT
  fVM_OR:	IAC = IAC | IOP;
		IOP = data[sp--];
         	NEXT
  fVM_XOR:	IAC = IAC ^ IOP;
		IOP = data[sp--];
         	NEXT
  fVM_SHL:	IAC = IOP << IAC;
		IOP = data[sp--];
         	NEXT
  fVM_SHR:	IAC = IOP >>= IAC;
		IOP = data[sp--];
         	NEXT

  /***************************************************/
  /* 0;    Return from a subroutine if TOS = 0.      */
  /*       If TOS = 0, DROP TOS.                     */
  /*       If TOS <> 0, do nothing                   */
  /* Opcode: 25         Stack: n-     Address: a-    */
  /*                    Stack: n-n    Address: -     */
  /***************************************************/

  fVM_ZERO_EXIT:if (IAC == 0)
         	{
           	  IAC = IOP;
		  IOP = data[sp--];
           	  ip = address[rsp--]+1;
         	}
         	NEXT
  fVM_INC:	IAC += 1;
         	NEXT
  fVM_DEC:	IAC -= 1;
         	NEXT
  fVM_IN:	a = IAC;
         	IAC = vm.ports[a];
         	vm.ports[a] = 0;
         	NEXT
  fVM_OUT:	vm.ports[0] = 0;
         	vm.ports[IAC] = IOP;
         	IAC = data[sp--];
		IOP = data[sp--];
         	NEXT
  fVM_WAIT:     if (vm.ports[0] == 0 && vm.ports[1] == 1)
                {
                  vm.ports[1] = dev_getch();
                  vm.ports[0] = 1;
                }

                /* Output (character generator) */
                if (vm.ports[2] == 1)
                {
    		  c = IAC;
		  IAC = IOP;
		  IOP = data[sp--];
    		  dev_putch(c);
    		  vm.ports[2] = 0;
    		  vm.ports[0] = 1;
                }

                /* Save Image */
                if (vm.ports[4] == 1)
                {
                  vm_save_image(vm.filename);
                  vm.ports[4] = 0;
                  vm.ports[0] = 1;
                }

                /* Capabilities */
                if (vm.ports[5] == -1)
                {
                  vm.ports[5] = IMAGE_SIZE;
                  vm.ports[0] = 1;
                }

                /* The framebuffer related bits aren't supported, so return 0
                   for them. */
                if (vm.ports[5] == -2 || vm.ports[5] == -3 || vm.ports[5] == -4)
                {
                  vm.ports[5] = 0;
                  vm.ports[0] = 1;
                }

                /* Data & Return Stack Depth */
                if (vm.ports[5] == -5)
                {
                  vm.ports[5] = vm.sp;
                  vm.ports[0] = 1;
                }
                if (vm.ports[5] == -6)
                {
                  vm.ports[5] = vm.rsp;
                  vm.ports[0] = 1;
                }
                dev_refresh();
         	NEXT
  fVM_PICK:	data[++sp] = IOP;
         	IOP = IAC;
		IAC = data[image[ip++]];
		NEXT
  fVM_BACK:	data[image[ip++]] = IAC;
		NEXT
  fVM_ROT:      a = IAC;
		IAC = data[sp];
		data[sp] = a;
		NEXT

  /***************************************************/
  /* SINST   generate new instruction stream and     */
  /*         return opcode in tos                    */
  /* Opcode: 37 number-of-ins, ins-1 .. ins-n        */
  /*                    Stack: -opcode Address: -    */
  /***************************************************/

  fVM_SINST:	a = image[ip++];
		c = tptr_a;
		tptr_b = tptr_a;
		for (b = 0; b < a; b++)
		{
		  if (tptr_a < (MAX_STREAM * STREAM_LEN))
		  {
                    if (image[ip] <= NUM_OPS)
                    {
		      if ((image[ip] == VM_LIT)      ||
			  (image[ip] == VM_GT_JUMP)  ||
			  (image[ip] == VM_LT_JUMP)  ||
			  (image[ip] == VM_NE_JUMP)  ||
			  (image[ip] == VM_EQ_JUMP)  ||
			  (image[ip] == VM_PICK)     ||
			  (image[ip] == VM_BACK)     ||
                          (image[ip] == VM_LI_IAC)   ||
                          (image[ip] == VM_LI_IOP)   ||
                          (image[ip] == VM_PICK_IAC) ||
			  (image[ip] == VM_PICK_IOP) ||
			  (image[ip] == VM_RAND)     ||
			  (image[ip] == VM_ROR)      ||
			  (image[ip] == VM_RXOR))
		      {
			tstream[tptr_a++] = vm_process_tr_otab[image[ip++]];
			tstream[tptr_a++] = (void*) image[ip++];
                      }
		      else tstream[tptr_a++] = vm_process_tr_otab[image[ip++]];
                    }
		    else
		    {
		      printf ("[SINST] Illegal opcode detected !\n");
		      exit (-1);
		    }
		  }
		  else
		  {
		    printf ("[SINST] No space for new DTC streams left !\n");
		    exit (-2);
		  }
		}
                tstream[tptr_a++] = &&tVM_EXIT;
		data[++sp] = IOP;
         	IOP = IAC;
		IAC = c;
		NEXT
  fVM_XOP:	TINST = tstream + image[ip++];
		TNEXT
  fVM_EXE:	TINST = tstream + IAC;
		IAC = IOP;
		IOP = data[sp--];
		TNEXT
  fVM_TRESET:	tptr_a = tptr_b;
		NEXT
  fVM_LI_IAC:	IAC = image[ip++];
		NEXT
  fVM_LI_IOP:	IOP = image[ip++];
		NEXT
  fVM_PICK_IAC: IAC = data[image[ip++]];
		NEXT
  fVM_PICK_IOP: IOP = data[image[ip++]];
		NEXT
  fVM_RADD:	IAC = IAC + IOP;
		NEXT
  fVM_RSUB:	IAC = IOP - IAC;
		NEXT
  fVM_RMUL:	IAC = IAC * IOP;
		NEXT
  fVM_RAND:	IAC = IAC & image[ip++];
		NEXT
  fVM_ROR:	IAC = IAC | image[ip++];
		NEXT
  fVM_RXOR:	IAC = IAC ^ image[ip++];
		NEXT

  /***************************************************/
  /* LCALL  load link register with return adress    */
  /*        and branch to a                          */
  /* Opcode: 48 a       Stack: -       Address: -    */
  /***************************************************/

  fVM_LCALL:	LNK = ip++;
         	ip = image[ip-1];
         	NEXT

  /***************************************************/
  /* LRETURN  load program counter with adress from  */
  /*          link register                          */
  /* Opcode: 49         Stack: -       Address: -    */
  /***************************************************/

  fVM_LRETURN:	ip = LNK;
		NEXT;

  fVM_PRINT:	printf ("%i\n",IAC);
		NEXT
  fVM_DEFAULT:	vm.ip  = ip;
		vm.sp  = sp;
		vm.rsp = rsp;
		printf ("[ngaro]   IAC: %i|",IAC);
		printf ("IOP: %i|",IOP);
		printf ("LNK: %i|",LNK);
		printf ("SP: %i|",sp);
		printf ("RSP: %i\n",rsp);
}
