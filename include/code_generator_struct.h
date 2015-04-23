#ifndef CODE_GENERATOR_STRUCT_H
#define	CODE_GENERATOR_STRUCT_H

#include "symtab.h"
#include "ast.h"

typedef enum opr{
/* true oprands*/
	/*TRANSFER*/
	OP_LEAL,
	OP_MOVL,
	OP_PUSHL,
	OP_POPL,
	/*algorithm &logic*/
	OP_ADDL,
	OP_SUBL,
	OP_IMULL,
	OP_IDIVL,
	OP_NEGL,
	OP_CLTD,
	OP_TESTL,
	OP_CMPL,
	/*jump &call*/
	OP_JMP,
	OP_JE,
	OP_JNE,
	OP_JGE,
	OP_JLE,
	OP_JG,
	OP_JL,
	OP_JP,
	OP_JNP,
	OP_CALL,
	OP_LEAVE,
	OP_RET,
	/*string loop*/
	OP_REP_STOSL,
/* pseudo oprands*/
	PS_LABEL,
	PS_FILE,
	PS_COMM,
	PS_SECTION,
	PS_STRING,
	PS_TEXT,
	PS_GLOBL,
	PS_DATA,
	PS_ALIGN,
	PS_TYPE,
	PS_SIZE,
	PS_IDENT,
	PS_LONG,
	PS_ZERO
}OPR;

enum REGS{
	NONE=0,
	EAX=1,
	EBX=2,
	ECX=3,
	EDX=4,
	ESI=5,
	EDI=6,
	EBP=7,
	ESP=8
};

static char regname[][10]={
	"none",
	"%eax",
	"%ebx",
	"%ecx",
	"%edx",
	"%esi",
	"%edi",
	"%ebp",
	"%esp"
};

enum{
	INVALIDPRAGMA,
	OMP
};

typedef struct Directives{
	int	type;
	int	parallel_en;
	int	for_en;
}PRAGMA;

typedef enum dessrctype86{
	REG,
	IMM,
	STATIC,
	REG_OFFSET,
	REG_OFFSET_INDEX,
	STATIC_OFFSET,
	STATIC_INDEX,
	STATIC_OFFSET_INDEX,
}DESSRCTYPE;


typedef struct DESSRC86{
	DESSRCTYPE type;
	int	imm;	//also used for index reg
	SYM	sym;
	int	reg;
	int	offset;
	int	factor;
}DESSRC;


typedef struct code{
	OPR opr;
	DESSRC *src;
	DESSRC *des;
	void *descriptor;
	void *next;
}CODE86;

typedef struct codelist{
	CODE86	*head,*tail;
}CODELIST;





#endif
