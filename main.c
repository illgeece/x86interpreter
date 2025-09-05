#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

typedef long long vm_int;  //64 bit target integer for VM



vm_int token; //current token
char *src, *old_src; //pointer to source code string
int poolsize; //size of stack/buffer/file
int line; //line number
//MEMORY SEGMENTS
vm_int *text, //text segment
    *old_text, //text segment to dump
    *stack;
char *data; //data segment, char because it will only store string literals
//REGISTERS - VIRTUAL MACHINE 
vm_int *pc, *bp, *sp, ax, cycle;
//INSTRUCTIONS - custom set of instructions to run on VM
// instructions
enum {LEA ,IMM ,JMP ,CALL,JZ  ,JNZ ,ENT ,ADJ ,LEV ,LI  ,LC  ,SI  ,SC  ,PUSH,
    OR  ,XOR ,AND ,EQ  ,NE  ,LT  ,GT  ,LE  ,GE  ,SHL ,SHR ,ADD ,SUB ,MUL ,DIV ,MOD ,
    OPEN,READ,CLOS,PRTF,MALC,MSET,MCMP,EXIT}; //instructions with args first



void next(){
  token = *src++; //dereferences old location, increments address, and returns old value (postfix op)
  return;
}

void expression(vm_int level){
  //do nothing right now
}

void program(){
  next();
  while(token>0){
    printf("token is %c\n", (char)token);
    next();
  }
}

vm_int eval(){
  vm_int op, *tmp;
  while(1){
    op= *pc++; //get next opcode
    //ordered by precedence
    if(op==IMM) ax= *pc++;
    else if(op==LC) {ax = *(char *)ax;}
    else if(op==LI) {ax = *(int *)ax;}
    else if(op==SC) {ax = *(char *)*sp++ = ax;}
    else if(op==SI) {*(int *)*sp++ = ax;}
    else if(op==PUSH) {*--sp = ax;} 
    //PUSH - unique operation for pushing that takes no args, does NOT support function calls due to VM architecture
    else if(op==JMP) {pc = (int *)*pc;} //jump to address
    else if(op==JZ) {pc = ax ? pc+1 : (int *)*pc;} //jump to address when ax is zero
    else if(op==JNZ) {pc = ax ? (int *)*pc : pc+1;} //jump to address when ax is not zero

    //calling frame section start
    else if(op==CALL) {*--sp = (int)(pc+1); pc = (int *)*pc;}
    //special instructions to mitigate lack of support for function calls, easy to add in VM compared to hardware
    else if(op==ENT) {*--sp = (int)bp; bp = sp; sp = sp-*pc++;} //allocate stack frame for function calls, stores PC val in stack and saves space for local vars
    else if(op==ADJ) {sp = sp+*pc++;} //adjust stack pointer by value of PC, special ADD instruction
    else if(op==LEV) {sp=bp; bp=(int *)*sp++; pc=(int *)*sp++;} //combines POP, MOV, RET instructions
    //LEA - custom load effective address, essentially an add instruction to return addresses based off the base pointer
    else if(op==LEA) {ax=(int)(bp+*pc++);}

    //mathematical operations, first arg stored atop the stack, second stored in ax
    else if (op == OR)  ax = *sp++ | ax;
    else if (op == XOR) ax = *sp++ ^ ax;
    else if (op == AND) ax = *sp++ & ax;
    else if (op == EQ)  ax = *sp++ == ax;
    else if (op == NE)  ax = *sp++ != ax;
    else if (op == LT)  ax = *sp++ < ax;
    else if (op == LE)  ax = *sp++ <= ax;
    else if (op == GT)  ax = *sp++ >  ax;
    else if (op == GE)  ax = *sp++ >= ax;
    else if (op == SHL) ax = *sp++ << ax;
    else if (op == SHR) ax = *sp++ >> ax;
    else if (op == ADD) ax = *sp++ + ax;
    else if (op == SUB) ax = *sp++ - ax;
    else if (op == MUL) ax = *sp++ * ax;
    else if (op == DIV) ax = *sp++ / ax;
    else if (op == MOD) ax = *sp++ % ax;
    
    //bridge operations betweeen interpeter and interpreted programs
    else if (op == EXIT) { printf("exit(%lld)", *sp); return *sp;}
    else if (op == OPEN) { ax = open((char *)sp[1], sp[0]); }
    else if (op == CLOS) { ax = close(*sp);}
    else if (op == READ) { ax = read(sp[2], (char *)sp[1], *sp); }
    else if (op == PRTF) { tmp = sp + pc[1]; ax = printf((char *)tmp[-1], tmp[-2], tmp[-3], tmp[-4], tmp[-5], tmp[-6]); }
    else if (op == MALC) { ax = (vm_int)malloc(*sp);}
    else if (op == MSET) { ax = (vm_int)memset((char *)sp[2], sp[1], *sp);}
    else if (op == MCMP) { ax = memcmp((char *)sp[2], (char *)sp[1], *sp);}
    
    //error handling
    else {
      printf("unknown instruction: %lld\n", op);
      return -1;
    }
  }
  return 0;
}


int main(int argc, char **argv){
  vm_int i, fd;

  argc--;
  argv++;

  poolsize = 256 * 1024; //arbitrary size
  line = 1;
  
  if((fd=open(*argv, 0))<0){
    printf("cound not open file %s\n", *argv);
    return -1;
  }
  
  if(!(src=old_src=malloc(poolsize))){
    printf("could not malloc (%d) bytes of memory\n", poolsize);
    return -1;
  }

  //read source file
  if((i=read(fd, src, poolsize-1)) <= 0){ //reads from file descriptor to buffer pointed to by src
    printf("read() returned %lld\n", i);
    return -1;
  }

  src[i]=0; //using just zero for EOF
  close(fd);


  //allocate memory for virtual machine
  if(!(text=old_text=malloc(poolsize))){
    printf("could not malloc (%d) bytes of memory for text area\n", poolsize);
    return -1;
  }

  if(!(data=malloc(poolsize))){
    printf("could not malloc (%d) bytes of memory for data area\n", poolsize);
    return -1;
  }

  if(!(stack=malloc(poolsize))){
    printf("could not malloc (%d) bytes of memory for stack area\n", poolsize);
    return -1;
  }

  memset(text, 0, poolsize);
  memset(data, 0, poolsize);
  memset(stack, 0, poolsize);

  bp=sp= (vm_int*)((vm_int)stack+poolsize);
  ax=0;

  program();
  return eval();
}