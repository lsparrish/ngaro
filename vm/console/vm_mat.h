#ifndef NGARO_HEADER
#define NGARO_HEADER

enum vm_opcode {VM_NOP, VM_LIT, VM_DUP, VM_DROP, VM_SWAP, VM_PUSH, VM_POP,
                VM_CALL, VM_JUMP, VM_RETURN, VM_GT_JUMP, VM_LT_JUMP,
                VM_NE_JUMP, VM_EQ_JUMP, VM_FETCH, VM_STORE, VM_ADD,
                VM_SUB, VM_MUL, VM_DIVMOD, VM_AND, VM_OR, VM_XOR, VM_SHL,
                VM_SHR, VM_ZERO_EXIT, VM_INC, VM_DEC, VM_IN, VM_OUT,
                VM_WAIT, VM_PICK, VM_BACK, VM_ROT, VM_XOP, VM_EXE, VM_TRESET,
                VM_SINST, VM_LI_IAC, VM_LI_IOP, VM_PICK_IAC, VM_PICK_IOP,
		VM_RADD, VM_RSUB, VM_RMUL, VM_RAND, VM_ROR, VM_RXOR, VM_LCALL,
                VM_LRETURN, VM_PRINT, VM_DEFAULT};

#define NUM_OPS      	  53

#define MAX_STREAM       255
#define STREAM_LEN      1024
#define IMAGE_SIZE   5000000
#define STACK_DEPTH      100
#define ADDRESSES       1024

#define CELL int

#define NEXT  goto *INST = vm_process_ct_otab[vm.image[ip++]];
#define TNEXT goto **TINST++;

#define INPUT  1
#define OUTPUT 2

typedef struct {
  int sp;
  int rsp;
  int ip;
  int data[STACK_DEPTH];
  int address[ADDRESSES];
  int ports[1024];
  int image[IMAGE_SIZE];
  int padding;
  char filename[2048];
} VM_STATE;

#endif
