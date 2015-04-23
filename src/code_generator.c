#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"
#include "util.h"
#include "ast.h"
#include "code_generator.h"

extern char infn[];

#define MAXLABELS 1000000

typedef enum labelt{
	LABEL_SYM,
	LABEL_LFB,
	LABEL_LFE,
	LABEL_JMP,
}LABELTYPE;

static	char	STR_RODATA[]=".rodata";
static	char	STR_OBJECT[]="@object";
static	char	STR_FUNCTION[]="@function";
static	char	STR_GOMPP[]="GOMP_parallel";

static	int	omp_flag=0;
static	int	omp_count=0;
CODELIST	omp_functions;
SYM	func_sym;

static void code_error(char *s){
	fprintf(stderr,"%s\n",s);
	exit(0);
}


PRAGMA *check_pragma(char *p){
	PRAGMA	*pra=(PRAGMA *)malloc(sizeof(PRAGMA));
	static char	pragmatmp[100];
	p=p+8;//skip #pragma{white_space}
	sscanf(p,"%s",pragmatmp);
	if(strcmp(pragmatmp,"omp")==0){
		pra->type=OMP;
		p=strstr(p,pragmatmp)+strlen(pragmatmp)+1;
		//fprintf(stderr,"***********%s\n",p);
	}else{
		pra->type=INVALIDPRAGMA;
		return pra;
	}
	sscanf(p,"%s",pragmatmp);
	if(strcmp(pragmatmp,"parallel")==0){
		pra->parallel_en=1;
		p=strstr(p,pragmatmp)+strlen(pragmatmp);
	}else{
		pra->parallel_en=0;
		pra->type=INVALIDPRAGMA;
		return pra;
	}
	sscanf(p,"%s",pragmatmp);
	if(strcmp(pragmatmp,"for")==0){
		pra->for_en=1;
		p=strstr(p,pragmatmp)+strlen(pragmatmp);
	}else{
		pra->for_en=0;
		return pra;
	}
	return pra;
}

char 	*new_label(LABELTYPE type,void *descriptor,int ord){
	SYM	sym;
	char	*str;
	static	char tmp[10];
	static	int	lfbcount=0;
	static	int	lfecount=0;
	static	int	jmpcount=0;
	str=(char *)malloc(100);
	switch(type){
		case LABEL_SYM:
			sym=(SYM)descriptor;
			strcpy(str,sym->name);
			if(ord){
				sprintf(tmp,"._omp_fn.%d",ord);
				strcat(str,tmp);
			}
			break;	
		case LABEL_LFB:
			sprintf(str,".LFB%d",lfbcount);
			lfbcount++;
			break;
		case LABEL_LFE:
			sprintf(str,".LFE%d",lfecount);
			lfecount++;
			break;
		case LABEL_JMP:
			sprintf(str,".L%d",jmpcount);
			jmpcount++;
			break;
		default:
			fprintf(stderr,"unknown label type\n");
	}
	return str;
}

CODE86 *newcode(OPR opr, DESSRC *src, DESSRC *des, void *descriptor){
	CODE86 *code;
	code=(CODE86 *)malloc(sizeof(CODE86));
	code->opr=opr;
	code->src=src;
	code->des=des;
	code->descriptor=descriptor;
	code->next=NULL;
	return code;
}

DESSRC *newdessrc(DESSRCTYPE type,int imm,SYM sym,int reg,int offset,int factor){
	DESSRC *res;
	res=(DESSRC *)malloc(sizeof(DESSRC));
	res->type=type;
	res->imm=imm;
	res->sym=sym;
	res->reg=reg;
	res->offset=offset;
	res->factor=factor;
	return res;
}

int code_link(CODELIST *a,CODELIST *b){
	if(a->head==NULL){
		a->head=b->head;
		a->tail=b->tail;
	}else if(b->head){
		a->tail->next=b->head;
		a->tail=b->tail;
	}else{
	}
	return 0;
}

int code_link_one(CODELIST *a,CODE86 *b){
	if(a->head==NULL){
		a->head=a->tail=b;
	}else if(b){
		a->tail->next=b;
		a->tail=b;
	}
	return 0;
}


int code_globalconst(ASTNode root){
	ASTNode constlist=root->firstchild;
	ASTNode p=constlist;
	ASTNode	pp;
	CODE86 *code;
	SYM	sym;
	int	size,align;
	int	num;
	char	*label;
	root->codes.head=root->codes.tail=NULL;
	while(p){
		p->codes.head=p->codes.head=NULL;
		if(p->type==ConstIdent){
			sym=p->firstchild->sym;
			size=sym->size;
			if(size<16)
				align=4;
			else if(size<64)
				align=16;
			else
				align=64;
	/* .globl */
			code=newcode(PS_GLOBL,NULL,NULL,(void *)sym);
			code_link_one(&p->codes,code);
	/* .section rodata */
			code=newcode(PS_SECTION,NULL,NULL,(void *)STR_RODATA);
			code_link_one(&p->codes,code);
	/* .align */
			code=newcode(PS_ALIGN,newdessrc(IMM,align,NULL,NONE,0,0),NULL,NULL);
			code_link_one(&p->codes,code);
	/* .type */
			code=newcode(PS_TYPE,NULL,NULL,(void *)sym);
			code_link_one(&p->codes,code);
	/* .size */
			code=newcode(PS_SIZE,newdessrc(IMM,size,NULL,NONE,0,0),newdessrc(IMM,0,NULL,NONE,0,0),(void *)sym);
			code_link_one(&p->codes,code);
	/* symbol label */
			label=new_label(LABEL_SYM,(void *)sym,0);
			code=newcode(PS_LABEL,NULL,NULL,(void *)label);
			code_link_one(&p->codes,code);
	/* .long */
			num=p->firstchild->nextsibling->val;
			code=newcode(PS_LONG,newdessrc(IMM,num,NULL,NONE,0,0),NULL,NULL);
			code_link_one(&p->codes,code);
		}else{
			sym=p->firstchild->sym;
			size=sym->size;
			if(size<16)
				align=4;
			else if(size<64)
				align=16;
			else
				align=64;
	/* .globl */
			code=newcode(PS_GLOBL,NULL,NULL,(void *)sym);
			code_link_one(&p->codes,code);
	/* .section rodata */
			code=newcode(PS_SECTION,NULL,NULL,(void *)STR_RODATA);
			code_link_one(&p->codes,code);
	/* .align */
			code=newcode(PS_ALIGN,newdessrc(IMM,align,NULL,NONE,0,0),NULL,NULL);
			code_link_one(&p->codes,code);
	/* .type */
			code=newcode(PS_TYPE,NULL,NULL,(void *)sym);
			code_link_one(&p->codes,code);
	/* .size */
			code=newcode(PS_SIZE,newdessrc(IMM,size,NULL,NONE,0,0),newdessrc(IMM,0,NULL,NONE,0,0),(void *)sym);
			code_link_one(&p->codes,code);
	/* symbol label */
			label=new_label(LABEL_SYM,(void *)sym,0);
			code=newcode(PS_LABEL,NULL,NULL,(void *)label);
			code_link_one(&p->codes,code);
	/* .long */
			pp=p->firstchild;
			while(pp->type!=ConstList){
				/*if(pp->type==Number)fprintf(stderr,"*************Number\n");
				else if(pp->type==Ident)fprintf(stderr,"*************Ident\n");
				else fprintf(stderr,"*************??\n");
				*/
				pp=pp->nextsibling;
			}
			pp=pp->firstchild;
			while(pp){
				num=pp->val;
				code=newcode(PS_LONG,newdessrc(IMM,num,NULL,NONE,0,0),NULL,NULL);
				code_link_one(&p->codes,code);
				size-=4;
				pp=pp->nextsibling;
			}
	/* .zero */
			if(size>0){
				code=newcode(PS_ZERO,newdessrc(IMM,size,NULL,NONE,0,0),NULL,NULL);
				code_link_one(&p->codes,code);
			}
		}
		code_link(&root->codes,&p->codes);
		p=p->nextsibling;
	}
	return 0;
}

int code_globalvar(ASTNode root){
	ASTNode varlist=root->firstchild;
	ASTNode p=varlist;
	CODE86 *code;
	int size,align;
	SYM sym;
	root->codes.head=root->codes.tail=NULL;
	while(p){
				if(p->type==Ident)
					sym=p->sym;
				else
					sym=p->firstchild->sym;
				if(sym==NULL)fprintf(stderr,"************var sym not found\n");
				size=sym->size;
				if(size<16)
					align=4;
				else if(size<64)
					align=16;
				else
					align=64;
				code=newcode(PS_COMM,newdessrc(IMM,size,NULL,NONE,0,0),newdessrc(IMM,align,NULL,NONE,0,0),(void *)sym->name);
				p->codes.head=p->codes.tail=code;
				code_link(&root->codes,&p->codes);
		p=p->nextsibling;
	}
	return 0;
}

int stack_allocate(SYMTAB symtab,int base){
	int total_size=symtab->size_max+symtab->size_local;
	SYM p=symtab->symlist;
	SYMTAB	child=symtab->firstchild;
//	printf("enter stack_allocate\n");
	while(p){
		p->memtype=Stack;
		base+=p->size;
		p->offset=-base;
//		printf("%s allocated in stack ,offset %d\n",p->name,p->offset);
		p=p->next;
	}
	while(child){
		stack_allocate(child,base);
		child=child->nextsibling;
	}
	return 0;
}

int code_blockconst(ASTNode constdecl,int BASEREG){
	ASTNode p=constdecl->firstchild;
	ASTNode pp;
	SYM	sym;
	int	val,offset,size,listsize;
	CODE86 *code;
	int pushflag=0;
	CODELIST	pushlist,poplist,initlist;
	constdecl->codes.head=constdecl->codes.tail=NULL;
	pushlist.head=pushlist.tail=NULL;
	poplist.head=poplist.tail=NULL;
	initlist.head=initlist.tail=NULL;
	while(p){
		switch(p->type){
			case ConstIdent:
				pp=p->firstchild;
				sym=pp->sym;
				pp=pp->nextsibling;
				val=pp->val;
				offset=sym->offset;
				code=newcode(OP_MOVL,newdessrc(IMM,val,NULL,NONE,0,0),newdessrc(REG_OFFSET,0,NULL,BASEREG,offset,0),NULL);
				p->codes.head=p->codes.tail=code;
				break;
			case ConstArray:
				p->codes.head=p->codes.tail=NULL;
				pp=p->firstchild;
				sym=pp->sym;
				offset=sym->offset;
				size=sym->size;
				if(size==0)break;	//empty array such as const int a[0]
				pp=pp->nextsibling;
				if(pp->type!=ConstList)
					pp=pp->nextsibling;
				listsize=pp->list_size;
				if(listsize*4<size){
					pushflag=1;
				/* leal offset(%ebp/%esi),%edx*/
					code=newcode(OP_LEAL,newdessrc(REG_OFFSET,0,NULL,BASEREG,offset,0),newdessrc(REG,0,NULL,EDX,0,0),NULL);
					code_link_one(&initlist,code);
				/* movl $0,%eax */
					code=newcode(OP_MOVL,newdessrc(IMM,0,NULL,NONE,0,0),newdessrc(REG,0,NULL,EAX,0,0),NULL);
					code_link_one(&initlist,code);
				/* movl size/4 ,%ecx */
					code=newcode(OP_MOVL,newdessrc(IMM,size/4,NULL,NONE,0,0),newdessrc(REG,0,NULL,ECX,0,0),NULL);
					code_link_one(&initlist,code);
				/* movl %edx, %edi */
					code=newcode(OP_MOVL,newdessrc(REG,size/4,NULL,EDX,0,0),newdessrc(REG,0,NULL,EDI,0,0),NULL);
					code_link_one(&initlist,code);
				/* rep stosl */
					code=newcode(OP_REP_STOSL,NULL,NULL,NULL);
					code_link_one(&initlist,code);
				}
				pp=pp->firstchild;
				while(pp){
				/* movl val,offset(%ebp/%esi) */
					code=newcode(OP_MOVL,newdessrc(IMM,pp->val,NULL,NONE,0,0),newdessrc(REG_OFFSET,0,NULL,BASEREG,offset,0),NULL);
					code_link_one(&initlist,code);
					offset+=4;
					pp=pp->nextsibling;
				}
				break;
		}
		code_link(&initlist,&p->codes);
		p=p->nextsibling;
	}
	if(pushflag){
/*pushl %edi*/
		code=newcode(OP_PUSHL,newdessrc(REG,0,NULL,EDI,0,0),NULL,NULL);;
		code_link_one(&pushlist,code);
/* popl %edi */
		code=newcode(OP_POPL,newdessrc(REG,0,NULL,EDI,0,0),NULL,NULL);;
		code_link_one(&poplist,code);
	}
	code_link(&constdecl->codes,&pushlist);
	code_link(&constdecl->codes,&initlist);
	code_link(&constdecl->codes,&poplist);
	return 0;
}
int code_stmt(ASTNode stmt,int BASEREG);


/* code_exptest 
 * return type
 * 0 :reg
 * 1 :memory direct
 * 2 :imm number
 */
int code_exptest(ASTNode exp,int BASEREG){
	if( (exp->type==LVal) && ( exp->firstchild->nextsibling==NULL) ){
		return 1;
	}
	if(exp->type==Number)
		return 2;
	return 0;
}

DESSRC *code_exp_ds(ASTNode exp,int BASEREG){
	SYM sym;
	int offset;
	if( (exp->type==LVal) && ( exp->firstchild->nextsibling==NULL) ){
		sym=exp->firstchild->sym;
		if(sym->memtype==Stack){
			return newdessrc(REG_OFFSET,0,NULL,BASEREG,sym->offset,0);
		}else
			return newdessrc(STATIC,0,sym,NONE,0,0);
	}
	if(exp->type==Number)
		return newdessrc(IMM,exp->val,NULL,NONE,0,0);
	if( (exp->type==LVal) && (code_exptest(exp->firstchild->nextsibling,BASEREG)==2) ){
		sym=exp->firstchild->sym;
		offset=exp->firstchild->nextsibling->val;
		if(sym->memtype==Stack){
			return newdessrc(REG_OFFSET,0,NULL,BASEREG,sym->offset+offset*4,0);
		}else {
			return newdessrc(STATIC_OFFSET,0,sym,NONE,offset*4,0);
		}
	}
	fprintf(stderr,"******************unsupported auto ds\n");
}


int code_exp(ASTNode exp,int BASEREG){
	CODE86 *code;
	ASTNode exp1,exp2;
	SYM	sym;
	exp->codes.head=exp->codes.tail=NULL;
	if(exp->type==Number){
			code=newcode(OP_MOVL,newdessrc(IMM,exp->val,NULL,NONE,0,0),newdessrc(REG,0,NULL,EAX,0,0),NULL);
			code_link_one(&exp->codes,code);
		return 0;
	}else if(exp->type==LVal){
		if(exp->firstchild->nextsibling==NULL){
			/* exp is a scalar */
			code=newcode(OP_MOVL,code_exp_ds(exp,BASEREG),newdessrc(REG,0,NULL,EAX,0,0),NULL);
			code_link_one(&exp->codes,code);
			return 0;
		}else{
			sym=exp->firstchild->sym;
			exp1=exp->firstchild->nextsibling;
			if(code_exptest(exp1,BASEREG)==2){
				int index=exp1->val;
				if(sym->memtype==Stack){
					code=newcode(OP_MOVL,newdessrc(REG_OFFSET,0,NULL,BASEREG,index*4+sym->offset,0),newdessrc(REG,0,NULL,EAX,0,0),NULL);
					code_link_one(&exp->codes,code);
				}else{
					code=newcode(OP_MOVL,newdessrc(STATIC_OFFSET,0,sym,NONE,index*4,0),newdessrc(REG,0,NULL,EAX,0,0),NULL);
					code_link_one(&exp->codes,code);
				}
				return 0;
			}else{
				code_exp(exp1,BASEREG);
				code_link(&exp->codes,&exp1->codes);
			/* index is in eax */
				if(sym->memtype==Stack){
					code=newcode(OP_MOVL,newdessrc(REG_OFFSET_INDEX,EAX,NULL,BASEREG,sym->offset,4),newdessrc(REG,0,NULL,EAX,0,0),NULL);
					code_link_one(&exp->codes,code);
				}else{
					code=newcode(OP_MOVL,newdessrc(STATIC_INDEX,EAX,sym,NONE,0,4),newdessrc(REG,0,NULL,EAX,0,0),NULL);
					code_link_one(&exp->codes,code);
				}
				return 0;
			}
		}
	}else if(exp->firstchild->type==UnaryOp){
		Opr opr=exp->firstchild->opr;
		exp1=exp->firstchild->nextsibling;
		code_exp(exp1,BASEREG);
		code_link(&exp->codes,&exp1->codes);
		if(opr==MINUS){
			code=newcode(OP_NEGL,newdessrc(REG,0,NULL,EAX,0,0),NULL,NULL);
			code_link_one(&exp->codes,code);
		}
	}else {
		Opr opr=exp->firstchild->nextsibling->opr;
		exp1=exp->firstchild;
		exp2=exp->firstchild->nextsibling->nextsibling;
		code_exp(exp2,BASEREG);
		code_link(&exp->codes,&exp2->codes);
		/* pushl %eax */
		code=newcode(OP_PUSHL,newdessrc(REG,0,NULL,EAX,0,0),NULL,NULL);
		code_link_one(&exp->codes,code);
		code_exp(exp1,BASEREG);
		code_link(&exp->codes,&exp1->codes);
		/* popl %ecx */
		code=newcode(OP_POPL,newdessrc(REG,0,NULL,ECX,0,0),NULL,NULL);
		code_link_one(&exp->codes,code);

		switch(opr){
			case PLUS:
				code=newcode(OP_ADDL,newdessrc(REG,0,NULL,ECX,0,0),newdessrc(REG,0,NULL,EAX,0,0),NULL);
				code_link_one(&exp->codes,code);
				break;
			case MINUS:
				code=newcode(OP_SUBL,newdessrc(REG,0,NULL,ECX,0,0),newdessrc(REG,0,NULL,EAX,0,0),NULL);
				code_link_one(&exp->codes,code);
				break;
			case MULTI:
				code=newcode(OP_IMULL,newdessrc(REG,0,NULL,ECX,0,0),newdessrc(REG,0,NULL,EAX,0,0),NULL);
				code_link_one(&exp->codes,code);
				break;
			case DIVIDE:
				code=newcode(OP_CLTD,NULL,NULL,NULL);
				code_link_one(&exp->codes,code);
				code=newcode(OP_IDIVL,newdessrc(REG,0,NULL,ECX,0,0),NULL,NULL);
				code_link_one(&exp->codes,code);
				break;
			case MOD:
				code=newcode(OP_CLTD,NULL,NULL,NULL);
				code_link_one(&exp->codes,code);
				code=newcode(OP_IDIVL,newdessrc(REG,0,NULL,ECX,0,0),NULL,NULL);
				code_link_one(&exp->codes,code);
				/* mov %edx, %eax */
				code=newcode(OP_MOVL,newdessrc(REG,0,NULL,EDX,0,0),newdessrc(REG,0,NULL,EAX,0,0),NULL);
				code_link_one(&exp->codes,code);
				break;
		}
	}
	return 0;
}


int code_cond(ASTNode cond,int BASEREG){
	CODE86 *code;
	int opr;
	int unaryflag;
	int test1,test2;
	ASTNode	p,exp1,exp2;
	p=cond->firstchild;
	cond->codes.head=cond->codes.tail=NULL;
	if(p->type==OddOp){
		opr=p->opr;
		unaryflag=1;
	}else{
		opr=p->nextsibling->opr;
		unaryflag=0;
	}
	if(unaryflag){
		exp1=cond->firstchild->nextsibling;
		code_exp(exp1,BASEREG);
		code_link(&cond->codes,&exp1->codes);
		code=newcode(OP_TESTL,newdessrc(IMM,1,NULL,NONE,0,0),newdessrc(REG,0,NULL,EAX,0,0),NULL);
		code_link_one(&cond->codes,code);
	}else{
		exp1=cond->firstchild;
		exp2=exp1->nextsibling->nextsibling;
		test1=code_exptest(exp1,BASEREG);
		test2=code_exptest(exp2,BASEREG);
		if(test2==0){
			if(test1==1){
				code_exp(exp2,BASEREG);
				code_link(&cond->codes,&exp2->codes);
			/* exp2 result is in %eax now */
				code=newcode(OP_CMPL,newdessrc(REG,0,NULL,EAX,0,0),code_exp_ds(exp1,BASEREG),NULL);
				code_link_one(&cond->codes,code);
			}else{
				code_exp(exp2,BASEREG);
				code_link(&cond->codes,&exp2->codes);
			/* exp2 result is in %eax now */
			/* pushl %eax */
				code=newcode(OP_PUSHL,newdessrc(REG,0,NULL,EAX,0,0),NULL,NULL);
				code_link_one(&cond->codes,code);

				code_exp(exp1,BASEREG);
				code_link(&cond->codes,&exp1->codes);
			/* exp1 result is in %eax now */
			/* popl %ecx */
				code=newcode(OP_POPL,newdessrc(REG,0,NULL,ECX,0,0),NULL,NULL);
				code_link_one(&cond->codes,code);
			/* cmpl %ecx,%eax */
				code=newcode(OP_CMPL,newdessrc(REG,0,NULL,ECX,0,0),newdessrc(REG,0,NULL,EAX,0,0),NULL);
				code_link_one(&cond->codes,code);
			}
		}else {
			code_exp(exp1,BASEREG);
			code_link(&cond->codes,&exp1->codes);
		/* exp1 result is in %eax now */
			code=newcode(OP_CMPL,code_exp_ds(exp2,BASEREG),newdessrc(REG,0,NULL,EAX,0,0),NULL);
			code_link_one(&cond->codes,code);
		}
	}
	return opr;
}

int code_whileclause(ASTNode whileclause,int BASEREG){
	CODE86	*code;
	ASTNode cond,stmt;
	char	*label[2];
	int	opr;
	int	i;
	OPR	oprand;
	cond=whileclause->firstchild;
	stmt=cond->nextsibling;
	whileclause->codes.head=whileclause->codes.tail=NULL;
	for(i=0;i<2;i++)
		label[i]=new_label(LABEL_JMP,NULL,0);
/* .L0 */
	code=newcode(PS_LABEL,NULL,NULL,label[0]);
	code_link_one(&whileclause->codes,code);
/* cond codes */
	opr=code_cond(cond,BASEREG);
		switch(opr){
			case ODD:
				oprand=OP_JP;
				break;
			case NEQ:
				oprand=OP_JE;
				break;
			case EQL:
				oprand=OP_JNE;
				break;
			case LSS:
				oprand=OP_JGE;
				break;
			case GTR:
				oprand=OP_JLE;
				break;
			case LEQ:
				oprand=OP_JG;
				break;
			case GEQ:
				oprand=OP_JL;
				break;
		}
	code_link(&whileclause->codes,&cond->codes);
/* if false goto .L1 */
	code=newcode(oprand,NULL,NULL,label[1]);
	code_link_one(&whileclause->codes,code);
/* stmt */
	code_stmt(stmt,BASEREG);
	code_link(&whileclause->codes,&stmt->codes);
/* jmp .L0 */
	code=newcode(OP_JMP,NULL,NULL,label[0]);
	code_link_one(&whileclause->codes,code);
/* .L1 */
	code=newcode(PS_LABEL,NULL,NULL,label[1]);
	code_link_one(&whileclause->codes,code);
	return 0;
}

int code_ifclause(ASTNode ifclause,int BASEREG){
	ASTNode cond,stmt1,stmt2;
	CODE86	*code;
	int	elseflag;
	char	*label[2];
	int	i;
	int	opr;
	OPR	oprand;
	
	cond=ifclause->firstchild;
	stmt1=cond->nextsibling;
	stmt2=stmt1->nextsibling;
	ifclause->codes.head=ifclause->codes.tail=NULL;
	if(stmt2){
		elseflag=1;
	}else{
		elseflag=0;
	}
	if(elseflag){
		for(i=0;i<2;i++)
			label[i]=new_label(LABEL_JMP,NULL,0);
/* opr.code */
		opr=code_cond(cond,BASEREG);
		code_link(&ifclause->codes,&cond->codes);
		switch(opr){
			case ODD:
				oprand=OP_JP;
				break;
			case NEQ:
				oprand=OP_JE;
				break;
			case EQL:
				oprand=OP_JNE;
				break;
			case LSS:
				oprand=OP_JGE;
				break;
			case GTR:
				oprand=OP_JLE;
				break;
			case LEQ:
				oprand=OP_JG;
				break;
			case GEQ:
				oprand=OP_JL;
				break;
		}
/* if false goto .L0 */
		code=newcode(oprand,NULL,NULL,label[0]);
		code_link_one(&ifclause->codes,code);
/* true:then */
		code_stmt(stmt1,BASEREG);
		code_link(&ifclause->codes,&stmt1->codes);
/* jmp .L1 */
		code=newcode(OP_JMP,NULL,NULL,label[1]);
		code_link_one(&ifclause->codes,code);
/* .L0 */
		code=newcode(PS_LABEL,NULL,NULL,label[0]);
		code_link_one(&ifclause->codes,code);
/* false:else */
		code_stmt(stmt2,BASEREG);
		code_link(&ifclause->codes,&stmt2->codes);
/* .L1 out */
		code=newcode(PS_LABEL,NULL,NULL,label[1]);
		code_link_one(&ifclause->codes,code);
	}else{
		for(i=0;i<1;i++)
			label[i]=new_label(LABEL_JMP,NULL,0);
		opr=code_cond(cond,BASEREG);
		code_link(&ifclause->codes,&cond->codes);
		switch(opr){
			case ODD:
				oprand=OP_JP;
				break;
			case NEQ:
				oprand=OP_JE;
				break;
			case EQL:
				oprand=OP_JNE;
				break;
			case LSS:
				oprand=OP_JGE;
				break;
			case GTR:
				oprand=OP_JLE;
				break;
			case LEQ:
				oprand=OP_JG;
				break;
			case GEQ:
				oprand=OP_JL;
				break;
		}
/* if false goto .L0 */
		code=newcode(oprand,NULL,NULL,label[0]);
		code_link_one(&ifclause->codes,code);
/* true:then */
		code_stmt(stmt1,BASEREG);
		code_link(&ifclause->codes,&stmt1->codes);
/* .L0 out */
		code=newcode(PS_LABEL,NULL,NULL,label[0]);
		code_link_one(&ifclause->codes,code);
	}
	return 0;
}

char *code_omp_stmt(ASTNode stmt,int BASEREG){
	CODE86	*code;
	CODELIST	omplist;
	char	*lfe,*lfb,*ompf;
	omp_count+=1;
	ompf=new_label(LABEL_SYM,(void *)func_sym,omp_count);
	lfe=new_label(LABEL_LFE,NULL,0);
	lfb=new_label(LABEL_LFB,NULL,0);
	omplist.head=omplist.tail=NULL;
	code=newcode(PS_TYPE,newdessrc(IMM,1,NULL,NONE,0,0),NULL,(void *)ompf);
	code_link_one(&omplist,code);
	code=newcode(PS_LABEL,NULL,NULL,(void *)ompf);
	code_link_one(&omplist,code);
	code=newcode(PS_LABEL,NULL,NULL,(void *)lfb);
	code_link_one(&omplist,code);
/* pushl %ebp */
	code=newcode(OP_PUSHL,newdessrc(REG,0,NULL,EBP,0,0),NULL,NULL);;
	code_link_one(&omplist,code);
/* movl %esp, %ebp */
	code=newcode(OP_MOVL,newdessrc(REG,0,NULL,ESP,0,0),newdessrc(REG,0,NULL,EBP,0,0),NULL);
	code_link_one(&omplist,code);
/* pushl %esi */
	code=newcode(OP_PUSHL,newdessrc(REG,0,NULL,ESI,0,0),NULL,NULL);;
	code_link_one(&omplist,code);
/* movl 8(%ebp),%esi */
	code=newcode(OP_MOVL,newdessrc(REG_OFFSET,0,NULL,EBP,8,0),newdessrc(REG,0,NULL,ESI,0,0),NULL);
	code_link_one(&omplist,code);
/* generate function code */
	code_stmt(stmt,ESI);
	code_link(&omplist,&stmt->codes);
/* popl %esi */
	code=newcode(OP_POPL,newdessrc(REG,0,NULL,ESI,0,0),NULL,NULL);;
	code_link_one(&omplist,code);
/* leave */
	code=newcode(OP_LEAVE,NULL,NULL,NULL);;
	code_link_one(&omplist,code);
/* ret */
	code=newcode(OP_RET,NULL,NULL,NULL);
	code_link_one(&omplist,code);
/* .lfb */
	code=newcode(PS_LABEL,NULL,NULL,(void *)lfe);
	code_link_one(&omplist,code);
/* .size */
	code=newcode(PS_SIZE,NULL,newdessrc(IMM,2,NULL,NONE,0,0),ompf);
	code_link_one(&omplist,code);
	code_link(&omp_functions,&omplist);

	return ompf;
}

int code_block(ASTNode block,int BASEREG){
	CODE86	*code;
	PRAGMA	*prag;
	char	*labelmp;
	int	skiplink=0;
	ASTNode p=block->firstchild;
	ASTNode pp;
	//fprintf(stderr,"enter a block\n");
	while(p){
		switch(p->type){
			case VarDecl:
			/* no need to generate code for var decl */
				p->codes.head=p->codes.tail=NULL;
				break;
			case ConstDecl:
				p->codes.head=p->codes.tail=NULL;
				code_blockconst(p,BASEREG);
				break;
			case FuncDecl:
			/* no need to generate code for funcdecl */
				p->codes.head=p->codes.tail=NULL;
				break;
			case CompilerDirective:
				//fprintf(stderr,"***************get one directive *\n");
				prag=check_pragma(p->str);
				p->codes.head=p->codes.tail=NULL;
				if(prag->type==OMP){
					pp=p->nextsibling;
					if( omp_flag){
						code_error("not suppored omp in another omp directive\n");
					}

					else if( (pp==NULL)||(pp->type==VarDecl) || (pp->type==ConstDecl) ||(pp->type==FuncDecl) ){
						code_error("pragma omp must be followed by a stmt\n");
						break;
					}else{
						omp_flag=1;
						labelmp=code_omp_stmt(pp,BASEREG);
						/* pushl 0 *2 */
						code=newcode(OP_PUSHL,newdessrc(IMM,0,NULL,NONE,0,0),NULL,NULL);
						code_link_one(&p->codes,code);
						code=newcode(OP_PUSHL,newdessrc(IMM,0,NULL,NONE,0,0),NULL,NULL);
						code_link_one(&p->codes,code);
						/* pushl %ebp */
						code=newcode(OP_PUSHL,newdessrc(REG,0,NULL,EBP,0,0),NULL,NULL);
						code_link_one(&p->codes,code);
						/* pushl ompfunc */
						code=newcode(OP_PUSHL,newdessrc(IMM,0,NULL,NONE,0,0),NULL,(void *)labelmp);
						code_link_one(&p->codes,code);
						/* call GOMP_parallel */
						code=newcode(OP_CALL,NULL,NULL,(void *)STR_GOMPP);
						code_link_one(&p->codes,code);
						/* addl $16,%esp */
						code=newcode(OP_ADDL,newdessrc(IMM,16,NULL,NONE,0,0),newdessrc(REG,0,NULL,ESP,0,0),NULL);
						code_link_one(&p->codes,code);
						omp_flag=0;
						skiplink=1;
					}
				}else{
					fprintf(stderr,"unsupported pragma \n");
				}
				break;
			default:
				p->codes.head=p->codes.tail=NULL;
				//fprintf(stderr,"checked one statements item\n");
				code_stmt(p,BASEREG);
		}	
		code_link(&block->codes,&p->codes);
		if(skiplink){
			skiplink=0;
			p=pp->nextsibling;
		}else{
			p=p->nextsibling;
		}
	}
	return 0;
}

int code_assign(ASTNode stmt,int BASEREG){
	CODE86	*code;
	ASTNode	lval=stmt->firstchild;
	SYM sym=lval->firstchild->sym;
	ASTNode	expi=lval->firstchild->nextsibling;
	ASTNode exp=lval->nextsibling;
	stmt->codes.head=stmt->codes.tail=0;
	if(expi==NULL){
		if(code_exptest(exp,BASEREG)==2){
			code=newcode(OP_MOVL,code_exp_ds(exp,BASEREG),code_exp_ds(lval,BASEREG),NULL);
			code_link_one(&stmt->codes,code);
		}else{
			code_exp(exp,BASEREG);
			code_link(&stmt->codes,&exp->codes);
			code=newcode(OP_MOVL,newdessrc(REG,0,NULL,EAX,0,0),code_exp_ds(lval,BASEREG),NULL);
			code_link_one(&stmt->codes,code);
		}
	}else {
		if(code_exptest(expi,BASEREG)==2){
			if(code_exptest(exp,BASEREG)==2){
				code=newcode(OP_MOVL,code_exp_ds(exp,BASEREG),code_exp_ds(lval,BASEREG),NULL);
				code_link_one(&stmt->codes,code);
			}else{
				code_exp(exp,BASEREG);
				code_link(&stmt->codes,&exp->codes);
				code=newcode(OP_MOVL,newdessrc(REG,0,NULL,EAX,0,0),code_exp_ds(lval,BASEREG),NULL);
				code_link_one(&stmt->codes,code);
			}
		}else{
			code_exp(exp,BASEREG);
			code_link(&stmt->codes,&exp->codes);
			code=newcode(OP_PUSHL,newdessrc(REG,0,NULL,EAX,0,0),NULL,NULL);
			code_link_one(&stmt->codes,code);
			code_exp(expi,BASEREG);
			code_link(&stmt->codes,&expi->codes);
			code=newcode(OP_POPL,newdessrc(REG,0,NULL,ECX,0,0),NULL,NULL);
			code_link_one(&stmt->codes,code);
			/* index is in eax */
				if(sym->memtype==Stack){
					code=newcode(OP_MOVL,newdessrc(REG,0,NULL,ECX,0,0),newdessrc(REG_OFFSET_INDEX,EAX,NULL,BASEREG,sym->offset,4),NULL);
					code_link_one(&stmt->codes,code);
				}else{
					code=newcode(OP_MOVL,newdessrc(REG,0,NULL,ECX,0,0),newdessrc(STATIC_INDEX,EAX,sym,NONE,0,4),NULL);
					code_link_one(&stmt->codes,code);
				}
		}
	}
	return 0;
}

int code_call(ASTNode call,int BASEREG){
	CODE86	*code;
	ASTNode ident;
	SYM	sym;
	call->codes.head=call->codes.tail=NULL;
	ident=call->firstchild;
	sym=ident->sym;
	code=newcode(OP_CALL,NULL,NULL,sym->name);
	code_link_one(&call->codes,code);
	return 0;
}

int code_stmt(ASTNode stmt,int BASEREG){
	switch(stmt->type){
		case Block:
			code_block(stmt,BASEREG);
			break;
		case IfClause:
			code_ifclause(stmt,BASEREG);
			break;
		case AssignExp:
			code_assign(stmt,BASEREG);
			break;
		case NilStmt:
			stmt->codes.head=stmt->codes.tail=NULL;
			break;
		case WhileClause:
			code_whileclause(stmt,BASEREG);
			break;
		case FunctionCall:
			code_call(stmt,BASEREG);
			break;
		default:
			fprintf(stderr,"unsupported stmt\n");
	}
	return 0;
}


int code_function(ASTNode function){
	CODELIST *list;
	CODELIST envstore,envrestore,stackalloc,stackrestore,statements;
	CODE86	*code;
	ASTNode	func_block=function->firstchild->nextsibling;
	ASTNode p=function;
	SYMTAB	symtab=func_block->symtab;
	SYM	sym=function->firstchild->sym;
	int	maxsize;
	char	*symlabel;
	char	*lfblabel;
	char	*lfelabel;
/*omp env init */
	omp_flag=0;
	omp_count=0;
	func_sym=sym;
	omp_functions.head=omp_functions.tail=NULL;


//	printf("enter code_function\n");
	stack_allocate(symtab,0);
	maxsize=symtab->size_max;
	function->codes.head=function->codes.tail=NULL;
/* .text */
	code=newcode(PS_TEXT,NULL,NULL,NULL);
	code_link_one(&p->codes,code);
/* .globl */
	code=newcode(PS_GLOBL,NULL,NULL,(void *)sym);
	code_link_one(&p->codes,code);
/* .type */
	code=newcode(PS_TYPE,NULL,NULL,(void *)sym);
	code_link_one(&p->codes,code);
/* .label */
	symlabel=new_label(LABEL_SYM,(void *)sym,0);
	code=newcode(PS_LABEL,NULL,NULL,(void *)symlabel);
	code_link_one(&p->codes,code);
/* .LFB */
	lfblabel=new_label(LABEL_LFB,NULL,0);
	lfelabel=new_label(LABEL_LFE,NULL,0);
	code=newcode(PS_LABEL,NULL,NULL,(void *)lfblabel);
	code_link_one(&p->codes,code);
/* pushl %ebp */
	code=newcode(OP_PUSHL,newdessrc(REG,0,NULL,EBP,0,0),NULL,NULL);;
	code_link_one(&p->codes,code);
/* movl %esp, %ebp */
	code=newcode(OP_MOVL,newdessrc(REG,0,NULL,ESP,0,0),newdessrc(REG,0,NULL,EBP,0,0),NULL);
	code_link_one(&p->codes,code);
/* stackalloc */
	stackalloc.head=stackalloc.tail=NULL;
	/* subl maxsize, %esp */
		code=newcode(OP_SUBL,newdessrc(IMM,maxsize,NULL,NONE,0,0),newdessrc(REG,0,NULL,ESP,0,0),NULL);
		code_link_one(&stackalloc,code);
/* envstore */
	envstore.head=envstore.tail=NULL;

/* block */
	statements.head=statements.tail=NULL;
	code_block(func_block,EBP);
	code_link(&statements,&func_block->codes);

/* evnrestore */
	envrestore.head=envrestore.tail=NULL;

/* stackrestore */
	stackrestore.head=stackrestore.tail=NULL;
	/* addl maxsize, %esp */
		code=newcode(OP_ADDL,newdessrc(IMM,maxsize,NULL,NONE,0,0),newdessrc(REG,0,NULL,ESP,0,0),NULL);
		code_link_one(&stackrestore,code);

/******************* link codes ************/
	code_link(&p->codes,&stackalloc);
	code_link(&p->codes,&envstore);
	code_link(&p->codes,&statements);
	code_link(&p->codes,&envrestore);
	code_link(&p->codes,&stackrestore);
	
	
/* leave */
	code=newcode(OP_LEAVE,NULL,NULL,NULL);;
	code_link_one(&p->codes,code);
/* ret */
	code=newcode(OP_RET,NULL,NULL,NULL);
	code_link_one(&p->codes,code);
/* .LFE */
	code=newcode(PS_LABEL,NULL,NULL,(void *)lfelabel);
	code_link_one(&p->codes,code);
/* .size */
	code=newcode(PS_SIZE,NULL,newdessrc(IMM,1,NULL,NONE,0,0),(void *)sym);
	code_link_one(&p->codes,code);

code_link(&p->codes,&omp_functions);

	return 0;
}

int code_program(ASTNode root){
	SYMTAB	symtab=root->symtab;
	SYM	sym;
	ASTNode	p;
	CODE86	*code;
	CODELIST *list;
	root->codes.head=root->codes.tail=NULL;
/*generate file psuedo code*/
	code=newcode(PS_FILE,NULL,NULL,infn);
	root->codes.head=code;
	root->codes.tail=code;
/*generate bss data segment*/
	
	
	
	p=root->firstchild;
//	printf("enter code_program\n");
	while(p){
		switch(p->type){
			case FuncDef:
				code_function(p);
				break;
			case ConstDecl:
				code_globalconst(p);
				break;
			case VarDecl:
				code_globalvar(p);
				break;
			case FuncDecl:
				p->codes.head=p->codes.tail=NULL;
				break;
			default:
				fprintf(stderr,"unknown compunit");
				p->codes.head=p->codes.tail=NULL;
		}
		p=p->nextsibling;
		//fprintf(stderr,"compunit checked\n");
	}
	p=root->firstchild;
	while(p){
		//fprintf(stderr,"compunit link\n");
		code_link(&root->codes,&p->codes);
		p=p->nextsibling;
	}
	return 0;
}

int code_generator(ASTNode root){
	switch(root->type){
		case Program:
			code_program(root);
			break;
	}
	return 0;
}

int dumpdessrc(FILE *of,DESSRC *dessrc){
	switch(dessrc->type){
		case	IMM:
			fprintf(of,"$%d",dessrc->imm);
			break;
		case	REG:
			fprintf(of,"%s",regname[dessrc->reg]);
			break;
		case	REG_OFFSET:
			fprintf(of,"%d(%s)",dessrc->offset,regname[dessrc->reg]);
			break;
		case	REG_OFFSET_INDEX:
			fprintf(of,"%d(%s,%s,%d)",dessrc->offset,regname[dessrc->reg],regname[dessrc->imm],dessrc->factor);
			break;
		case	STATIC:
			fprintf(of,"%s",dessrc->sym->name);
			break;
		case	STATIC_OFFSET:
			fprintf(of,"%s",dessrc->sym->name);
			if(dessrc->offset>=0)
				fprintf(of,"+");
			fprintf(of,"%d",dessrc->offset);
			break;
		case	STATIC_INDEX:
			fprintf(of,"%s( ,%s,%d)",dessrc->sym->name,regname[dessrc->imm],dessrc->factor);
			break;
		default:
			fprintf(of,"unsupported dessrc");
	}
}

int code_dump_one(FILE *of,CODE86 *code){
	SYM sym=(SYM)(code->descriptor);
	char *str;
	switch(code->opr){
		case PS_FILE:
			fprintf(of,"\t.file\t\"%s\"",infn);
			break;
		case PS_COMM:
			fprintf(of,"\t.comm\t%s,%d,%d",(char *)code->descriptor,code->src->imm,code->des->imm);
			break;
		case PS_DATA:
			fprintf(of,"\t.data");
			break;
		case PS_ALIGN:
			fprintf(of,"\t.align\t%d",code->src->imm);
			break;
		case PS_TYPE:
			if(code->src!=NULL){
				fprintf(of,"\t.type\t%s, %s",(char *)code->descriptor,STR_FUNCTION);
				break;
			}	
			if(sym->type==Function){
				str=STR_FUNCTION;
			}else{
				str=STR_OBJECT;
			}
			fprintf(of,"\t.type\t%s, %s",sym->name,str);
			break;
		case PS_LONG:
			fprintf(of,"\t.long\t%d",code->src->imm);
			break;
		case PS_SIZE:
			if(code->des->imm==0){
				fprintf(of,"\t.size\t%s, %d",sym->name,code->src->imm);
			}else if(code->des->imm==1){
				fprintf(of,"\t.size\t%s, .-%s",sym->name,sym->name);
			}else if(code->des->imm==2){
				fprintf(of,"\t.size\t%s, .-%s",(char *)code->descriptor,(char *)code->descriptor);
			}
			break;
		case PS_ZERO:
			fprintf(of,"\t.zero\t%d",code->src->imm);
			break;
		case PS_SECTION:
			fprintf(of,"\t.section\t%s",(char *)code->descriptor);
			break;
		case PS_GLOBL:
			fprintf(of,"\t.globl\t%s",sym->name);
			break;
		case PS_LABEL:
			fprintf(of,"%s:",(char *)code->descriptor);
			break;
		case PS_TEXT:
			fprintf(of,"\t.text");
			break;
		case OP_RET:
			fprintf(of,"\tret");
			break;
		case OP_PUSHL:
			fprintf(of,"\tpushl\t");
			if(code->descriptor){
				fprintf(of,"$%s",(char *)code->descriptor);
			}else{
				dumpdessrc(of,code->src);
			}
			break;
		case OP_POPL:
			fprintf(of,"\tpopl\t");
			dumpdessrc(of,code->src);
			break;
		case OP_MOVL:
			fprintf(of,"\tmovl\t");
			dumpdessrc(of,code->src);
			fprintf(of,", ");
			dumpdessrc(of,code->des);
			break;
		case OP_ADDL:
			fprintf(of,"\taddl\t");
			dumpdessrc(of,code->src);
			fprintf(of,", ");
			dumpdessrc(of,code->des);
			break;
		case OP_TESTL:
			fprintf(of,"\ttestl\t");
			dumpdessrc(of,code->src);
			fprintf(of,", ");
			dumpdessrc(of,code->des);
			break;
		case OP_CMPL:
			fprintf(of,"\tcmpl\t");
			dumpdessrc(of,code->src);
			fprintf(of,", ");
			dumpdessrc(of,code->des);
			break;
		case OP_SUBL:
			fprintf(of,"\tsubl\t");
			dumpdessrc(of,code->src);
			fprintf(of,", ");
			dumpdessrc(of,code->des);
			break;
		case OP_NEGL:
			fprintf(of,"\tnegl\t");
			dumpdessrc(of,code->src);
			break;
		case OP_LEAL:
			fprintf(of,"\tleal\t");
			dumpdessrc(of,code->src);
			fprintf(of,", ");
			dumpdessrc(of,code->des);
			break;
		case OP_IMULL:
			fprintf(of,"\timull\t");
			dumpdessrc(of,code->src);
			break;
		case OP_IDIVL:
			fprintf(of,"\tidivl\t");
			dumpdessrc(of,code->src);
			break;
		case OP_CLTD:
			fprintf(of,"\tcltd\t");
			break;
		case OP_REP_STOSL:
			fprintf(of,"\trep stosl");
			break;
		case OP_LEAVE:
			fprintf(of,"\tleave");
			break;
		case OP_JMP:
			fprintf(of,"\tjmp\t%s",(char *)code->descriptor);
			break;
		case OP_JLE:
			fprintf(of,"\tjle\t%s",(char *)code->descriptor);
			break;
		case OP_JGE:
			fprintf(of,"\tjge\t%s",(char *)code->descriptor);
			break;
		case OP_JL:
			fprintf(of,"\tjl\t%s",(char *)code->descriptor);
			break;
		case OP_JG:
			fprintf(of,"\tjg\t%s",(char *)code->descriptor);
			break;
		case OP_JE:
			fprintf(of,"\tje\t%s",(char *)code->descriptor);
			break;
		case OP_JNE:
			fprintf(of,"\tjne\t%s",(char *)code->descriptor);
			break;
		case OP_JP:
			fprintf(of,"\tjp\t%s",(char *)code->descriptor);
			break;
		case OP_JNP:
			fprintf(of,"\tjnp\t%s",(char *)code->descriptor);
			break;
		case OP_CALL:
			fprintf(of,"\tcall\t%s",(char *)code->descriptor);
			break;
		default:
			fprintf(of,";unknown oprand");
	}
	fprintf(of,"\n");
	return 0;
}

int code_dump(FILE *of,ASTNode root){
	CODE86 *code=root->codes.head;
	while(code){
		code_dump_one(of,code);
		code=code->next;
	}
	return 0;
	
}
