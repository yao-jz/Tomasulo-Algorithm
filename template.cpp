#include <stdio.h>
#include <string.h>

#define MAXLINELENGTH 1000  /* 机器指令的最大长度 */
#define MEMSIZE 10000 		/* 内存的最大容量     */
#define NUMREGS 32			/* 寄存器数量         */

/*
 * 操作码和功能码定义
 */

#define regRegALU 0  /* 寄存器-寄存器的ALU运算的操作码为0 */
#define LW 35
#define SW 43
#define ADDI 8
#define ANDI 12
#define BEQZ 4
#define J 2
#define HALT 1
#define NOOP 3
#define addFunc 32	/* ALU运算的功能码 */
#define subFunc 34
#define andFunc 36

#define NOOPINSTRUCTION 0x0c000000;

/*
 * 执行单元
 */	
#define LOAD1	1
#define LOAD2	2
#define STORE1	3
#define STORE2	4
#define INT1	5
#define INT2	6

#define NUMUNITS 6	/* 执行单元数量 */
char *unitname[NUMUNITS]={"LOAD1","LOAD2","STORE1","STORE2","INT1","INT2"};  /* 执行单元的名称 */

/*
 * 不同操作所需要的周期数
 */
#define BRANCHEXEC	3	/* 分支操作 */
#define LDEXEC		2	/* Load     */
#define STEXEC		2	/* Store    */
#define INTEXEC		1	/* 整数运算 */

/*
 * 指令状态
 */
#define ISSUING			0	/* 发射   */
#define EXECUTING		1	/* 执行   */
#define WRITINGRESULT	2	/* 写结果 */
#define COMMITTING		3	/* 提交   */
char *statename[4]={"ISSUING","EXECUTING","WRITINGRESULT","COMMTITTING"};  /*  状态名称 */

#define RBSIZE	16	/* ROB有16个单元 */
#define BTBSIZE	8	/* 分支预测缓冲栈有8个单元 */

/*
 * 2 bit 分支预测状态
 */
#define STRONGNOT	0
#define WEAKTAKEN	1
#define WEAKNOT		2
#define STRONGTAKEN	3

/*
 * 分支跳转结果
 */
#define NOTTAKEN        0
#define TAKEN           1

typedef struct _resStation {	/* 保留栈的数据结构 */
  int instr;	/*    指令    */
  int busy;		/* 空闲标志位 */
  int Vj;		/* Vj, Vk 存放操作数 */
  int Vk;
  int Qj;		/* Qj, Qk 存放将会生成结果的执行单元编号 */
  int Qk;		/* 为零则表示对应的V有效 */
  int exTimeLeft;	/* 指令执行的剩余时间 */
  int reorderNum;   /* 该指令对应的ROB项编号 */
}resStation;

typedef struct _reorderEntry {	/* ROB项的数据结构 */
  int busy;     	/* 空闲标志位 */
  int instr;		/* 指令 */
  int execUnit;		/* 执行单元编号 */
  int instrStatus;  /* 指令的当前状态 */
  int valid;		/* 表明结果是否有效的标志位 */
  int result;		/* 在提交之前临时存放结果 */
  int storeAddress; /* store指令的内存地址 */
}reorderEntry;

typedef struct _regResultEntry {	/* 寄存器状态的数据结构 */
  int valid;		/* 1表示寄存器值有效, 否则0 */
  int reorderNum;	/* 如果值无效, 记录ROB中哪个项目会提交结果 */
}regResultEntry;

typedef struct _btbEntry{	/* 分支预测缓冲栈的数据结构 */
  int valid;		/* 有效位 */
  int branchPC;		/* 分支指令的PC值 */
  int branchTarget; /* when predict taken, update PC with target */
  int branchPred;	/* 预测：2-bit分支历史 */
}btbEntry;

typedef struct _machineState{	/* 虚拟机状态的数据结构 */
  int pc;		/* PC */
  int cycles;	/* 已经过的周期数 */
  resStation reservation[NUMUNITS];		/* 保留栈 */
  reorderEntry	reorderBuf[RBSIZE];		/* ROB */
  regResultEntry regResult[NUMREGS];	/* 寄存器状态 */
  btbEntry	btBuf[BTBSIZE];				/* 分支预测缓冲栈 */
  int memory[MEMSIZE];		/* 内存   */
  int regFile[NUMREGS];		/* 寄存器 */
}machineState;

int field0(int);
int field1(int);
int field2(int);
int opcode(int);

void printInstruction(int);

void printState(machineState *statePtr,int memorySize)
{
	int i;
	
	printf("Cycles: %d\n", statePtr->cycles);
	
	printf("\t pc=%d\n",statePtr->pc);
	
	printf("\t Reservation stations:\n");
	for (i=0; i<NUMUNITS; i++){
		if (statePtr->reservation[i].busy == 1){
			printf("\t \t Reservation station %d: ",i);
			if (statePtr->reservation[i].Qj==0)
			{printf("Vj = %d ", statePtr->reservation[i].Vj);} 
			else 
			{printf("Qj = '%s' ", unitname[statePtr->reservation[i].Qj-1]);}
			if (statePtr->reservation[i].Qk==0)
			{printf("Vk = %d ", statePtr->reservation[i].Vk);} 
			else 
			{printf("Qk = '%s' ", unitname[statePtr->reservation[i].Qk-1]);}
			printf(" ExTimeLeft = %d  RBNum = %d\n", 
				statePtr->reservation[i].exTimeLeft,
				statePtr->reservation[i].reorderNum);
		}
	}
	
	printf("\t Reorder buffers:\n");
	for (i=0; i<RBSIZE; i++){
		if (statePtr->reorderBuf[i].busy == 1){
			printf("\t \t Reorder buffer %d: ",i);
			printf("instr %d  executionUnit '%s'  state %s  valid %d  result %d storeAddress %d\n",
				statePtr->reorderBuf[i].instr,
				unitname[statePtr->reorderBuf[i].instr, statePtr->reorderBuf[i].execUnit-1],
				statename[statePtr->reorderBuf[i].instrStatus], 
				statePtr->reorderBuf[i].valid, statePtr->reorderBuf[i].result,
				statePtr->reorderBuf[i].storeAddress); 
		}
	}
    
	printf("\t Register result status:\n");
	for (i=1; i<NUMREGS; i++){
		if (!statePtr->regResult[i].valid){
			printf("\t \t Register %d: ",i);
			printf("waiting for reorder buffer number %d\n",
				statePtr->regResult[i].reorderNum);
		}
	}
	
	
	/*
	* [TODO]如果你实现了动态分支预测, 将这里的注释取消
	*/
	
	 /*printf("\t Branch target buffer:\n");
	 for (i=0; i<BTBSIZE; i++){
		 if (statePtr->btBuf[i].valid){
			 printf("\t \t Entry %d: PC=%d, Target=%d, Pred=%d\n",
				i, statePtr->btBuf[i].branchPC, statePtr->btBuf[i].branchTarget,
				statePtr->btBuf[i].branchPred);
		}
	 }*/
	 
	printf("\t Memory:\n");
	for (i=0; i<memorySize; i++){
		printf("\t \t memory[%d] = %d\n", i, statePtr->memory[i]);
	}
	
	printf("\t Registers:\n");
	for (i=0; i<NUMREGS; i++){
		printf("\t \t regFile[%d] = %d\n", i, statePtr->regFile[i]);
	}
}

/*
 *[TODO]
 *这里对指令进行解码，转换成程序可以识别的格式，需要根据指令格式来进行。
 *可以考虑使用高级语言中的位和逻辑运算
 */
int field0(int instruction)
{
/*
 *[TODO]
 *返回指令的第一个寄存器RS1
 */
    return 
}

int field1(int instruction)
{
/*
 *[TODO]
 *返回指令的第二个寄存器，RS2或者Rd
 */
    return
}

int field2(int instruction)
{
/*
 *[TODO]
 *返回指令的第三个寄存器，Rd
 */
    return(
}

int immediate(int instruction)
{
/*
 *[TODO]
 *返回I型指令的立即数部分
 */
    return
}

int jumpAddr(int instruction)
{
/*
 *[TODO]
 *返回J型指令的跳转地址
 */
    return
}

int opcode(int instruction)
{
/*
 *[TODO]
 *返回指令的操作码
 */
    return
}

int func( int instruction)
{
/*
 *[TODO]
 *返回R型指令的功能域
 */
  return
}

void printInstruction(int instr)
{
    char opcodeString[10];
    char funcString[11];
    int funcCode;
    int op;

    if (opcode(instr) == regRegALU) {
	funcCode = func(instr);
	if (funcCode == addFunc){
	  strcpy(opcodeString, "add");
	} else if (funcCode == subFunc){
	  strcpy(opcodeString, "sub");
	} else if (funcCode == andFunc){
	  strcpy(opcodeString, "and");
	} else {
	  strcpy(opcodeString, "alu");
	}
	printf("%s %d %d %d \n", opcodeString, field0(instr), field1(instr),
	       field2(instr));
    } else if (opcode(instr) == LW) {
	strcpy(opcodeString, "lw");
	printf("%s %d %d %d\n", opcodeString, field0(instr), field1(instr),
	       immediate(instr));
    } else if (opcode(instr) == SW) {
	strcpy(opcodeString, "sw");
	printf("%s %d %d %d\n", opcodeString, field0(instr), field1(instr),
	       immediate(instr));
    } else if (opcode(instr) == ADDI) {
	strcpy(opcodeString, "addi");
	printf("%s %d %d %d\n", opcodeString, field0(instr), field1(instr),
	       immediate(instr));
    } else if (opcode(instr) == ANDI) {
	strcpy(opcodeString, "andi");
	printf("%s %d %d %d\n", opcodeString, field0(instr), field1(instr),
	       immediate(instr));
    } else if (opcode(instr) == BEQZ) {
	strcpy(opcodeString, "beqz");
	printf("%s %d %d %d\n", opcodeString, field0(instr), field1(instr),
	       immediate(instr));
    } else if (opcode(instr) == J) {
	strcpy(opcodeString, "j");
	printf("%s %d\n", opcodeString, jumpAddr(instr));
    } else if (opcode(instr) == HALT) {
	strcpy(opcodeString, "halt");
	printf("%s\n", opcodeString);
    } else if (opcode(instr) == NOOP) {
	strcpy(opcodeString, "noop");
	printf("%s\n", opcodeString);
    } else {
	strcpy(opcodeString, "data");
	printf("%s %d\n", opcodeString, instr);
    }
}

int convertNum16(int num)
{
  /* convert an 16 bit number into a 32-bit or 64-bit number */
  if (num & 0x8000) {
    num -= 65536;
  }
  return(num);
}

int convertNum26(int num)
{
  /* convert an 26 bit number into a 32-bit or 64-bit number */
  if (num & 0x200000) {
    num -= 67108864;
  }
  return(num);
}

void updateRes(int unit, machineState *statePtr, int value)
{
  /*
   *[TODO]
   * 更新保留栈:
   * 将位于公共数据总线上的数据
   * 复制到正在等待它的其他保留栈中去
   */

}

void issueInstr(int instr, int unit, machineState *statePtr, int reorderNum)
{
  
  /*
  * [TODO]
  * 发射指令:
  * 填写保留栈和ROB项的内容.
  * 注意, 要在所有的字段中写入正确的值.
  * 检查寄存器状态, 相应的在Vj,Vk和Qj,Qk字段中设置正确的值:
  * 对于I类型指令, 设置Qk=0,Vk=0;
  * 对于sw指令, 如果寄存器有效, 将寄存器中的内存基地址保存在Vj中;
  * 对于beqz和j指令, 将当前PC+1的值保存在Vk字段中.
  * 如果指令在提交时会修改寄存器的值, 还需要在这里更新寄存器状态数据结构.
  */

}

int checkReorder(machineState *statePtr, int *headRB, int *tailRB)
{
  /*
   * [TODO]
   * 在ROB的队尾检查是否有空闲的空间, 如果有, 返回空闲项目的编号.
   * ROB是一个循环队列, 它可以容纳RBSIZE个项目.
   * 新的指令被添加到队列的末尾, 指令提交则是从队首进行的.
   * 当队列的首指针或尾指针到达数组中的最后一项时, 它应滚动到数组的第一项.
   */

}

int getResult(resStation rStation, machineState *statePtr)
{
  int op, immed, function, address;

  /* 
   * [TODO]
   * 这个函数负责计算有输出的指令的结果.
   * 你需要完成下面的case语句....
   */

  op = opcode(rStation.instr);
  immed = immediate(rStation.instr);

  switch (op){
  case ANDI:
    return(rStation.Vj & immed);
    break;
  /* case .... */
  default:
    break;
  }
}

/* 选作内容 */
int getPrediction(machineState *statePtr)
{
  /*
   * [TODO]
   * 对给定的PC, 检查分支预测缓冲栈中是否有历史记录
   * 如果有, 返回根据历史信息进行的预测, 否则返回-1
   */
}

/* 选作内容 */
void updateBTB(machineState *statePtr, int branchPC, int targetPC, int outcome)
{
    /*
     * [TODO]
     * 更新分支预测缓冲栈: 检查是否与缓冲栈中的项目匹配.
     * 如果是, 对2-bit的历史记录进行更新;
     * 如果不是, 将当前的分支语句添加到缓冲栈中去. 
     * 如果缓冲栈已满，你需要选择一种替换算法将旧的记录替换出去.
     * 如果当前跳转成功, 将初始的历史状态设置为STRONGTAKEN;
     * 如果不成功, 将历史设置为STRONGNOT
     */
}

/* 选作内容 */
int getTarget(machineState *statePtr, int reorderNum)
{
    /* 
     * [TODO]
     * 检查分支指令是否已保存在分支预测缓冲栈中:
     * 如果不是, 返回当前pc+1, 这意味着我们预测分支跳转不会成功;
     * 如果在, 并且历史信息为STRONGTAKEN或WEAKTAKEN, 返回跳转的目标地址,
     * 如果历史信息为STRONGNOT或WEAKNOT, 返回当前pc+1.
     */
}


main(int argc, char *argv[])
{
    FILE *filePtr;
    int pc, done, instr, i;
    char line[MAXLINELENGTH];
    machineState *statePtr;
    int memorySize;
    int success, newBuf, op, halt, unit;
    int headRB, tailRB;
    int regA, regB, immed, address;
    int flush;
    int rbnum;
    

    if (argc != 2) {
      printf("error: usage: %s <machine-code file>\n", argv[0]);
      exit(1);
    }

    filePtr = fopen(argv[1], "r");
    if (filePtr == NULL) {
      printf("error: can't open file %s", argv[1]);
      perror("fopen");
      exit(1);
    }

    /*
     * 初始化, 读输入文件等
     *
     */

    /*
     * 分配数据结构空间
     */

    statePtr = (machineState *) malloc(sizeof(machineState));

    /* 
     * 将机器指令读入到内存中
     */

    for (i=0; i<MEMSIZE; i++){
      statePtr->memory[i] = 0;
    }
    pc = 16;
    done = 0;
    while (!done){
      if (fgets(line, MAXLINELENGTH, filePtr) == NULL){
	done = 1;
      } else {
	if (sscanf(line, "%d", &instr) != 1) {
	    printf("error in reading address %d\n", pc);
	    exit(1);
	} 
	
	statePtr->memory[pc] = instr;
	printf("memory[%d]=%d\n", pc, statePtr->memory[pc]);
	pc = pc + 1;
      }
    }

    memorySize = pc;
    halt = 0;

    /*
     * 状态初始化
     */

    statePtr->pc = 16;
    statePtr->cycles = 0;
    for (i=0; i<NUMREGS; i++){
      statePtr->regFile[i] = 0;
    }
    for (i=0; i<NUMUNITS; i++){
      statePtr->reservation[i].busy = 0;
    }
    for (i=0; i<RBSIZE; i++){
      statePtr->reorderBuf[i].busy = 0;
    }

    headRB = -1;
    tailRB = -1;

    for (i=0; i<NUMREGS; i++){
      statePtr->regResult[i].valid = 1;
    }
    for (i=0; i<BTBSIZE; i++){
      statePtr->btBuf[i].valid = 0;
    }

    /*
     * 处理指令
     */

    while (1) {  /* 执行循环:你应该在执行halt指令时跳出这个循环 */

	printState(statePtr, memorySize);

	/*
   * [TODO]
	 * 基本要求:
	 * 首先, 确定是否需要清空流水线或提交位于ROB的队首的指令.
	 * 我们处理分支跳转的缺省方法是假设跳转不成功, 如果我们的预测是错误的,
	 * 就需要清空流水线(ROB/保留栈/寄存器状态), 设置新的pc = 跳转目标.
	 * 如果不需要清空, 并且队首指令能够提交, 在这里更新状态:
	 *     对寄存器访问, 修改寄存器;
	 *     对内存写操作, 修改内存.
	 * 在完成清空或提交操作后, 不要忘了释放保留栈并更新队列的首指针.
	 */
	 
	/*
   * [TODO]
	 * 选作内容:
	 * 在提交的时候, 我们知道跳转指令的最终结果.
	 * 有三种可能的情况: 预测跳转成功, 预测跳转不成功, 不能预测(因为分支预测缓冲栈中没有对应的项目).
	 * 如果我们预测跳转成功:
	 *     如果我们的预测是正确的, 只需要继续执行就可以了;
	 *     如果我们的预测是错误的, 即实际没有发生跳转, 就必须重新设置正确的PC值, 并清空流水线.
	 * 如果我们预测跳转不成功:
	 *     如果预测是正确的, 继续执行;
	 *     如果预测是错误的, 即实际上发生了跳转, 就必须将PC设置为跳转目标, 并清空流水线.
	 * 如果我们不能预测跳转是否成功:
	 *     如果跳转成功, 仍然需要清空流水线, 将PC修改为跳转目标.
	 * 在遇到分支时, 需要更新分支预测缓冲站的内容.
     */


	/*
   * [TODO]
	 * 提交完成.
	 * 检查所有保留栈中的指令, 对下列状态, 分别完成所需的操作:
	 */

	/* 
   * [TODO]
	 * 对Writing Result状态:
	 * 将结果复制到正在等待该结果的其他保留栈中去;
	 * 还需要将结果保存在ROB中的临时存储区中.
	 * 释放指令占用的保留栈, 将指令状态修改为Committing
	 */

	/*
   * [TODO]
	 * 对Executing状态:
	 * 执行剩余时间递减;
	 * 在执行完成时, 将指令状态修改为Writing Result
	 */

	/*
   * [TODO]
	 * 对Issuing状态:
	 * 检查两个操作数是否都已经准备好, 如果是, 将指令状态修改为Executing
	 */

	/*
   * [TODO]
	 * 最后, 当我们处理完了保留栈中的所有指令后, 检查是否能够发射一条新的指令.
	 * 首先检查是否有空闲的保留栈, 如果有, 再检查ROB中是否有空闲的空间,
	 * 如果也能找到空闲空间, 发射指令.
	 */
	 
	/*
   * [TODO]
	 * 选作内容:
	 * 在发射跳转指令时, 将PC修改为正确的目标: 是pc = pc+1, 还是pc = 跳转目标?
	 * 在发射其他的指令时, 只需要设置pc = pc+1.
	 */

	    
	/*
   * [TODO]
	 * 周期计数加1
	 */

    }   /* while (1) */
	printf("halting machine\n");
}