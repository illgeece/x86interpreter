#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

#define int long long  //64 bit target integer


int token; //current token
char *src, *old_src; //pointer to source code string
int poolsize; //size of stack/buffer/file
int line; //line number
//MEMORY SEGMENTS
int *text, //text segment
    *old_text, //text segment to dump
    *stack;
char *data; //data segment, char because it will only store string literals
//REGISTERS - VIRTUAL MACHINE 
int *pc, *bp, *sp, ax, cycle;
//INSTRUCTIONS - custom set of instructions to run on VM
// instructions
enum {LEA ,IMM ,JMP ,CALL,JZ  ,JNZ ,ENT ,ADJ ,LEV ,LI  ,LC  ,SI  ,SC  ,PUSH,
    OR  ,XOR ,AND ,EQ  ,NE  ,LT  ,GT  ,LE  ,GE  ,SHL ,SHR ,ADD ,SUB ,MUL ,DIV ,MOD ,
    OPEN,READ,CLOS,PRTF,MALC,MSET,MCMP,EXIT}; //instructions with args first



void next(){
  token = *src++; //dereferences old location, increments address, and returns old value (postfix op)
  return;
}

void expression(int level){
  //do nothing right now
}

void program(){
  next();
  while(token>0){
    printf("token is %c\n", token);
    next();
  }
}

int eval(){
  int op, *tmp;
  while(1){
    op= *pc++; //get next opcode
    if(op==IMM) ax= *pc++;
    else if(op==LC) {ax = *(char *)ax;}
    else if(op==LI) {ax = *(int *)ax;}
    else if(op==SC) {ax = *(char *)*sp++ = ax;}
    else if(op==SI) {*(int *)*sp++ = ax;}
    else if(op==PUSH) {*--sp = ax;}
    else if(op==JMP) {pc = (int *)*pc;} //jump to address when ax is not zero
    else if(op==JZ) {pc = ax ? pc+1 : (int *)*pc;} //jump to address when ax is zero
    else if(op==JNZ) {pc = ax ? (int *)*pc : pc+1;} //jump to address when ax is not zero

    //calling frame section start

  }


  return 0;
}

int main(int argc, char **argv){
  int i, fd;

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
    printf("read() returned %d\n", i);
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

  bp=sp= (int*)((int)stack+poolsize);
  ax=0;

  program();
  return eval();
}