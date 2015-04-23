%{
#include "util.h"
#include "symtab.h"
#include "ast.h"
#include "code_generator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
%}

%union {
        int val;
        char *name;
	ASTNode node;
	Opr op;
}
%locations

%token DIRECTIVE
%token CONST INT VOID
%token EXTERN
%token IDENT NUMBER
%token IF ELSE WHILE
%token ODDSYM
%token LBRACKET RBRACKET LPAREN RPAREN LBRACE RBRACE
%token PLUSSYM MINUSSYM MULTISYM DIVSYM MODSYM
%token GEQSYM LEQSYM NEQSYM GTRSYM LSSSYM EQLSYM
%token ASSIGN
%token COMMA SEMICOLON

%type <val> NUMBER
%type <name> IDENT DIRECTIVE
%type <op> RelOp
%type <node> Program CompUnit Decl VarDecl ConstDecl FuncDecl FuncDef Stmt Cond Exp LVal ConstDefs ConstDef Block BlockTail BlockItem BlockItems NumberList VarList Var

%left MINUSSYM PLUSSYM
%left MULTISYM DIVSYM MODSYM

%%

Program		:CompUnit{
			SYM sym;
			$$=new_parent(Program,(void *)current_symtab,$1);
			$$->symtab=current_symtab;
			root=$$;
			{debug("Program   ::=  CompUnit\n");}
/*
			sym=sym_lookup("main",$$->symtab);
			if( (sym==NULL) || (sym->type !=Function) ){
				yyerror("main function is missing");
			}
*/
		}

CompUnit	:Decl{
			$$=$1;
			{debug("CompUnit  ::=  Decl\n");}
		}
		|FuncDef{
			$$=$1;
			{debug("CompUnit  ::=  FuncDef\n");}
		}
		|Decl CompUnit{
			assign_sibling($1,$2);
			$$=$1;
			{debug("CompUnit  ::=  CompUnit Decl\n");}
		}
		|FuncDef CompUnit{
			assign_sibling($1,$2);
			$$=$1;
			{debug("CompUnit  ::=  CompUnit FuncDef\n");}
		}
		;

Decl		:ConstDecl{
			$$=$1;
			{debug("Decl      ::=  ConstDecl\n");}
		}
		|VarDecl{
			$$=$1;
			{debug("Decl      ::=  VarDecl\n");}
		}
		|FuncDecl{
			$$=$1;
			{debug("Decl      ::=  FuncDecl\n");}
		}
		;

ConstDecl	:CONST INT ConstDefs SEMICOLON{
			$$=new_parent(ConstDecl,NULL,$3);
			{debug("ConstDecl ::=  CONST INT ConstDefs SEMICOLON\n");}
		}
		;

ConstDefs	:ConstDef{
			$$=$1;
			{debug("ConstDefs ::=  ConstDef\n");}
		}
		|ConstDef COMMA ConstDefs{
			assign_sibling($1,$3);
			$$=$1;
			{debug("ConstDefs ::=  ConstDefs , ConstDef\n");}
		}
		;

ConstDef	:IDENT ASSIGN NUMBER {
			ASTNode ident,number;
			SYM	sym;
			sym = sym_lookup($1,current_symtab);
			if(sym==NULL){
				sym=sym_add($1,ConstScalar,4);
				ident=new_leaf(Ident,(void *)sym);
				number=new_leaf(Number,(void *)&($3));
				assign_sibling(ident,number);
				$$=new_parent(ConstIdent,NULL,ident);
				{debug("ConstDef  ::=  IDENT ASSIGN NUMBER\n");}
			}else{
				yyerror1("Identifier redeclaration : %s\n",$1);
			}
		}
		|IDENT LPAREN RPAREN ASSIGN LBRACE NumberList RBRACE{
			ASTNode ident,list;
			SYM	sym;
			sym = sym_lookup($1,current_symtab); 
			if(sym==NULL){
				sym=sym_add($1,ConstVector,$6->list_size*4);
				ident=new_leaf(Ident,(void *)sym);
				list=new_parent(ConstList,NULL,$6);
				list->list_size=$6->list_size;
				assign_sibling(ident,list);
				$$=new_parent(ConstArray,NULL,ident);
				{debug("ConstDef  ::=  IDENT [] = { NumberList }\n");}
			}else{
				yyerror1("Identifier redeclaration : %s\n",$1);
			}
				
		}
		|IDENT LPAREN ASSIGN LBRACE NumberList RBRACE{
			ASTNode ident,list;
			SYM	sym;
			sym = sym_lookup($1,current_symtab); 
			if(sym==NULL){
				sym=sym_add($1,ConstVector,$5->list_size*4);
				ident=new_leaf(Ident,(void *)sym);
				list=new_parent(ConstList,NULL,$5);
				list->list_size=$5->list_size;
				assign_sibling(ident,list);
				$$=new_parent(ConstArray,NULL,ident);
				{debug("ConstDef  ::=  IDENT [] = { NumberList }\n");}
			}else{
				yyerror1("Identifier redeclaration : %s\n",$1);
			}
			yyerror0("\']\' missed\n");
		}
		|IDENT LPAREN NUMBER RPAREN ASSIGN LBRACE NumberList RBRACE{
			ASTNode ident,number,list;
			SYM	sym;
			sym = sym_lookup($1,current_symtab); 
			if(sym==NULL){
				if($3<0){
					yyerror1("Array %s length must be zero or positive.\n",$1);
					$3=0;
				}else if($7->list_size>$3){
					yyerror1("Const Array %s NumberList longer than declared.\n",$1);
				}
				sym=sym_add($1,ConstVector,$3*4);
				ident=new_leaf(Ident,(void *)sym);
				number=new_leaf(Number,(void *)&($3));
				list=new_parent(ConstList,NULL,$7);
				list->list_size=$7->list_size;
				assign_sibling(number,list);
				assign_sibling(ident,number);
				$$=new_parent(ConstArray,NULL,ident);
				{debug("ConstDef  ::=  IDENT [ NUMBER ] = { NumberList }\n");}
			}else{
				yyerror1("Identifier redeclaration : %s\n",$1);
			}
		}
		|IDENT LPAREN NUMBER ASSIGN LBRACE NumberList RBRACE{
			ASTNode ident,number,list;
			SYM	sym;
			sym = sym_lookup($1,current_symtab); 
			if(sym==NULL){
				if($3<0){
					yyerror1("Array %s length must be zero or positive.\n",$1);
					$3=0;
				}else if($6->list_size){
					yyerror1("Const Array %s NumberList longer than declared.\n",$1);
				}
				sym=sym_add($1,ConstVector,$3);
				ident=new_leaf(Ident,(void *)sym);
				number=new_leaf(Number,(void *)&($3));
				list=new_parent(ConstList,NULL,$6);
				list->list_size=$6->list_size;
				assign_sibling(number,list);
				assign_sibling(ident,number);
				$$=new_parent(ConstArray,NULL,ident);
				{debug("ConstDef  ::=  IDENT [ NUMBER ] = { NumberList }\n");}
			}else{
				yyerror1("Identifier redeclaration : %s\n",$1);
			}
			yyerror0("\']\' missed\n");
		}
		|IDENT {
			ASTNode ident,number;
			SYM	sym;
			int	def=0;
			sym = sym_lookup($1,current_symtab);
			if(sym==NULL){
				sym=sym_add($1,ConstScalar,4);
				ident=new_leaf(Ident,(void *)sym);
				number=new_leaf(Number,&def);
				assign_sibling(ident,number);
				$$=new_parent(ConstIdent,NULL,ident);
				yyerror1("const %s not initialized.\n",$1);
				{debug("ConstDef  ::=  IDENT ASSIGN NUMBER\n");}
			}else{
				yyerror1("Identifier redeclaration : %s\n",$1);
			}
		}	
		;

NumberList	:NUMBER{
			{debug("NumberList::=  NUMBER\n");}
			$$=new_leaf(Number,(void *)&($1));
			$$->list_size=1;
		}
		|NUMBER COMMA NumberList{
			{debug("NumberList::=  NUMBER , NumberList \n");}
			$$=new_leaf(Number,(void *)&($1));
			$$->list_size=$3->list_size+1;
			assign_sibling($$,$3);
		}
		;

VarDecl		:INT VarList SEMICOLON{
			$$=new_parent(VarDecl,NULL,$2);
			{debug("VarDecl   ::=  INT VarList ;\n");}
		}
		;

VarList		:Var{
			$$=$1;
			{debug("VarList   ::=  Var\n");}
		}
		|Var COMMA VarList{
			assign_sibling($1,$3);
			$$=$1;
			{debug("VarList   ::=  VarList , Var\n");}
		}
		;

Var		:IDENT{
			SYM sym;
			sym = sym_lookup($1,current_symtab);
			if(sym==NULL){
				sym = sym_add($1,VarScalar,4);
				$$=new_leaf(Ident,(void *)sym);
				{debug("Var       ::=  IDENT\n");}
			}else{
				yyerror1("Identifier redeclaration : %s\n",$1);
			}
		}
		|IDENT LPAREN NUMBER RPAREN{
			SYM sym;
			ASTNode number,ident;
			sym = sym_lookup($1,current_symtab);
			if(sym==NULL){
				if($3<0){
					yyerror1("Array %s length must be zero or positive.\n",$1);
					$3=0;
				}
				sym = sym_add($1,VarVector,$3*4);
				ident=new_leaf(Ident,(void *)sym);
				number=new_leaf(Number,(void *)&($3));
				assign_sibling(ident,number);
				$$=new_parent(VarArray,NULL,ident);
				{debug("Var       ::=  IDENT [ NUMBER ]\n");}
			}else{
				yyerror1("Identifier redeclaration : %s\n",$1);
			}
		}
		|IDENT LPAREN NUMBER {
			SYM sym;
			ASTNode number,ident;
			sym = sym_lookup($1,current_symtab);
			if(sym==NULL){
				if($3<0){
					yyerror1("Array %s length must be zero or positive.\n",$1);
					$3=0;
				}
				sym = sym_add($1,VarVector,$3*4);
				ident=new_leaf(Ident,(void *)sym);
				number=new_leaf(Number,(void *)&($3));
				assign_sibling(ident,number);
				$$=new_parent(VarArray,NULL,ident);
				{debug("Var       ::=  IDENT [ NUMBER ]\n");}
			}else{
				yyerror1("Identifier redeclaration : %s\n",$1);
			}
			yyerror0("\']\' missed\n");
		}
		;

FuncDecl	:EXTERN VOID IDENT LBRACKET RBRACKET SEMICOLON{
	 		SYM sym;
			ASTNode ident;
			sym = sym_deep_lookup($3,current_symtab);
			if( sym==NULL ){
				sym = sym_add($3,Function,0);
			}else{
				if( (sym->type!=Function) && (sym->type!=ExternFunction) ){
					yyerror1("Identifier redeclaration : %s\n",$3);
				}
			}
			ident=new_leaf(Ident,(void *)sym);
			$$=new_parent(FuncDecl,NULL,ident);
			{debug("FuncDecl  ::=  EXTERN VOID IDENT ( ) ; \n");}
	 	}
		;

FuncDef		:VOID IDENT LBRACKET RBRACKET {
			SYM sym;
			sym = sym_lookup($2,current_symtab);
			if( (sym==NULL)||(sym->type==ExternFunction) ){
				if(sym==NULL){
					sym = sym_add($2,Function,0);
				}else{
					sym->type=Function;
				}
			}else{
				yyerror1("Identifier redeclaration : %s\n",$2);
			}
	 	}Block{
			SYM sym;
			ASTNode ident;
			sym = sym_lookup($2,current_symtab);
				ident=new_leaf(Ident,(void *)sym);
				assign_sibling(ident,$6);
				$$=new_parent(FuncDef,NULL,ident);
				{debug("FuncDef   ::=  VOID IDENT ( ) Block  ->  Decl\n");}
		}
		|VOID IDENT {
			yyerror0("\'()\' missing in function definition\n");
		}Block{
			SYM sym;
			ASTNode ident;
			sym = sym_lookup($2,current_symtab);
			if( (sym==NULL)||(sym->type==ExternFunction) ){
				if(sym==NULL){
					sym = sym_add($2,Function,0);
				}else{
					sym->type=Function;
				}
				sym = sym_add($2,Function,0);
				ident=new_leaf(Ident,(void *)sym);
				assign_sibling(ident,$4);
				$$=new_parent(FuncDef,NULL,ident);
				{debug("FuncDef   ::=  VOID IDENT ( ) Block  ->  Decl\n");}
			}else{
				yyerror1("Identifier redeclaration : %s\n",$2);
			}  
		}
		;


Block		:LBRACE {
			symtab_new_level();
		}BlockTail{
			$$=$3;
		}

BlockTail	:RBRACE{
			$$=new_leaf(Block,(void *)current_symtab);
			symtab_return();
			{debug("Block     ::=  { }\n");}
		}
		| BlockItems RBRACE{
			$$=new_parent(Block,(void *)current_symtab,$1);
			symtab_return();
			{debug("Block     ::=  { BlockItems } \n");}
		}
		| BlockItems {
			$$=new_parent(Block,(void *)current_symtab,$1);
			symtab_return();
			{debug("Block     ::=  { BlockItems } \n");}
			yyerror0("\'}\' missed.\n");
		}
		|{
			$$=new_leaf(Block,(void *)current_symtab);
			symtab_return();
			{debug("Block     ::=  { }\n");}
			yyerror0("\'}\' missed.\n");
		}
		
		;

BlockItems	:BlockItem{
			$$=$1;
			{debug("BlockItems::=  BlockItem\n");}
		}
		|BlockItem BlockItems{
			assign_sibling($1,$2);
			$$=$1;
			{debug("BlockItems::=  BlockItems BlockItem\n");}
		}
		;

BlockItem	:Decl{
			$$=$1;
			{debug("BlockItem ::=  Decl\n");}
		}
		|Stmt{
			$$=$1;
			{debug("BlockItem ::=  Stmt\n");}
		}
		|DIRECTIVE{
			//fprintf(stderr,"***********directive string is at %p \n",$1);
			//fprintf(stderr,"***********string content %s \n",$1);
			$$=new_leaf(CompilerDirective,$1);
			{debug("BlockItem ::=  DIRECCTIVE\n");}
		}
		;

Stmt		:LVal ASSIGN Exp SEMICOLON{
			ASTNode identifier=$1->firstchild;
			SYM sym=identifier->sym;
			if( (sym!=NULL) &&(sym->type==ConstScalar) && (sym->type !=ConstVector) ){
				yyerror1("const %s can't be assigned.\n",sym->name);
			}
			assign_sibling($1,$3);
			$$=new_parent(AssignExp,NULL,$1);
			{debug("Stmt      ::=  LVal = Exp ;\n");}
		}
		|IDENT LBRACKET RBRACKET SEMICOLON{
			SYM sym;
			ASTNode ident;
			sym = sym_deep_lookup($1,current_symtab);
			if(sym == NULL){
				yyerror1("Undeclared Identifier %s\n",$1);
				ident=new_leaf(Ident,(void *)sym);
				$$=new_parent(FunctionCall,NULL,ident);
				{debug("Stmt      ::=  IDENT ( ) ;\n");}
			}else if( (sym->type!=Function)&& (sym->type!=ExternFunction) ){
				yyerror1("Identifier %s type error ,Function type expected\n",$1);
				ident=new_leaf(Ident,(void *)sym);
				$$=new_parent(FunctionCall,NULL,ident);
				{debug("Stmt      ::=  IDENT ( ) ;\n");}
			}else{
				ident=new_leaf(Ident,(void *)sym);
				$$=new_parent(FunctionCall,NULL,ident);
				{debug("Stmt      ::=  IDENT ( ) ;\n");}
			}
		}
		|Block{
			$$=$1;
			{debug("Stmt      ::=  Block  \n");}
		}
		|IF Cond Stmt{
			assign_sibling($2,$3);
			$$=new_parent(IfClause,NULL,$2);
			{debug("Stmt      ::=  IF Cond Stmt  \n");}
		}
		|IF Cond Stmt ELSE Stmt{
			assign_sibling($3,$5);
			assign_sibling($2,$3);
			$$=new_parent(IfClause,NULL,$2);
			{debug("Stmt      ::=  IF Cond Stmt ELSE Stmt  \n");}
		}
		|WHILE Cond Stmt{
			assign_sibling($2,$3);
			$$=new_parent(WhileClause,NULL,$2);
			{debug("Stmt      ::=  WHILE Cond Stmt  \n");}
		}
		|SEMICOLON{
			$$=new_leaf(NilStmt,NULL);
			{debug("Stmt      ::=  ;  \n");}
		}
		;
LVal		:IDENT{
			SYM sym;
			ASTNode ident;
			sym = sym_deep_lookup($1,current_symtab);
			if(sym==NULL){
				yyerror1("Undeclared Identifier %s\n",$1);
				ident=new_leaf(Ident,(void *)sym);  		//assign Ident leaf to NULL symbol
				$$=new_parent(LVal,NULL,ident);
				{debug("LVal      ::=  IDENT \n");}
			}else if( (sym->type!=VarScalar) && (sym->type!=ConstScalar) ){
				yyerror1("Identifier %s type error ,Scalar type expected\n",$1);
				ident=new_leaf(Ident,(void *)sym);  		//assign Ident leaf to NULL symbol
				$$=new_parent(LVal,NULL,ident);
				{debug("LVal      ::=  IDENT \n");}
			}else{
				ident=new_leaf(Ident,(void *)sym);
				if(ident==NULL)printf("%s\n",$1);
				$$=new_parent(LVal,NULL,ident);
				{debug("LVal      ::=  IDENT \n");}
			}
		}
		|IDENT LPAREN Exp RPAREN{
			SYM sym;
			ASTNode ident;
			sym = sym_deep_lookup($1,current_symtab);
			if(sym==NULL){
				yyerror1("Undeclared Identifier %s\n",$1);
				ident=new_leaf(Ident,(void *)sym);
				assign_sibling(ident,$3);
				$$=new_parent(LVal,NULL,ident);
				{debug("LVal      ::=  IDENT [ Exp ]\n");}
			}else if( (sym->type!=VarVector) && (sym->type!=ConstVector) ){
				yyerror1("Identifier %s type error ,Vector type expected\n",$1);
				ident=new_leaf(Ident,(void *)sym);
				assign_sibling(ident,$3);
				$$=new_parent(LVal,NULL,ident);
				{debug("LVal      ::=  IDENT [ Exp ]\n");}
			}else{
				ident=new_leaf(Ident,(void *)sym);
				assign_sibling(ident,$3);
				$$=new_parent(LVal,NULL,ident);
				{debug("LVal      ::=  IDENT [ Exp ]\n");}
			}
		}
		|IDENT LPAREN Exp{
			SYM sym;
			ASTNode ident;
			sym = sym_deep_lookup($1,current_symtab);
			if(sym==NULL){
				yyerror1("Undeclared Identifier %s\n",$1);
				ident=new_leaf(Ident,(void *)sym);
				assign_sibling(ident,$3);
				$$=new_parent(LVal,NULL,ident);
				{debug("LVal      ::=  IDENT [ Exp ]\n");}
			}else if( (sym->type!=VarVector) && (sym->type!=ConstVector) ){
				yyerror1("Identifier %s type error ,Vector type expected\n",$1);
				ident=new_leaf(Ident,(void *)sym);
				assign_sibling(ident,$3);
				$$=new_parent(LVal,NULL,ident);
				{debug("LVal      ::=  IDENT [ Exp ]\n");}
			}else{
				ident=new_leaf(Ident,(void *)sym);
				assign_sibling(ident,$3);
				$$=new_parent(LVal,NULL,ident);
				{debug("LVal      ::=  IDENT [ Exp ]\n");}
			}
			yyerror0("\']\' missed\n");
		}
		;

Cond		:ODDSYM Exp{
			ASTNode op;
			op=new_leaf(OddOp,NULL);
			assign_sibling(op,$2);
			$$=new_parent(Cond,NULL,op);
			{debug("Cond      ::=  ODD Exp \n");}
		}
		|Exp RelOp Exp{
			ASTNode op;
			op=new_leaf(RelOp,(void *)&($2));
			assign_sibling(op,$3);
			assign_sibling($1,op);
			$$=new_parent(Cond,NULL,$1);
			{debug("Cond      ::=  Exp RelOp Exp \n");}
		}
		;

RelOp		:EQLSYM{
			$$=EQL;
			{debug("RelOp     ::=  !=\n");}
		}
		|LSSSYM{
			$$=LSS;
			{debug("RelOp     ::=  <\n");}
		}
		|GTRSYM{
			$$=GTR;
			{debug("RelOp     ::=  >\n");}
		}
		|LEQSYM{
			$$=LEQ;
			{debug("RelOp     ::=  <=\n");}
		}
		|GEQSYM{
			$$=GEQ;
			{debug("RelOp     ::=  >=\n");}
		}
		|NEQSYM{
			$$=NEQ;
			{debug("RelOp     ::=  >=\n");}
		}
		;

Exp		:Exp PLUSSYM Exp{
			Opr opr=PLUS;
			ASTNode op;
			op=new_leaf(BinOp,(void *)&(opr));
			assign_sibling(op,$3);
			assign_sibling($1,op);
			$$=new_parent(Exp,NULL,$1);
			{debug("Exp       ::=  Exp + Exp\n");}
		}
		|Exp MINUSSYM Exp{
			Opr opr=MINUS;
			ASTNode op;
			op=new_leaf(BinOp,(void *)&(opr));
			assign_sibling(op,$3);
			assign_sibling($1,op);
			$$=new_parent(Exp,NULL,$1);
			{debug("Exp       ::=  Exp - Exp\n");}
		}
		|Exp MULTISYM Exp{
			Opr opr=MULTI;
			ASTNode op;
			op=new_leaf(BinOp,(void *)&(opr));
			assign_sibling(op,$3);
			assign_sibling($1,op);
			$$=new_parent(Exp,NULL,$1);
			{debug("Exp       ::=  Exp * Exp\n");}
		}
		|Exp DIVSYM Exp{
			Opr opr=DIVIDE;
			ASTNode op;
			op=new_leaf(BinOp,(void *)&(opr));
			assign_sibling(op,$3);
			assign_sibling($1,op);
			$$=new_parent(Exp,NULL,$1);
			{debug("Exp       ::=  Exp / Exp\n");}
		}
		|Exp MODSYM Exp{
			Opr opr=MOD;
			ASTNode op;
			op=new_leaf(BinOp,(void *)&(opr));
			assign_sibling(op,$3);
			assign_sibling($1,op);
			$$=new_parent(Exp,NULL,$1);
			{debug("Exp       ::=  Exp % Exp\n");}
		}
		|PLUSSYM Exp %prec PLUSSYM{
			Opr opr=PLUS;
			ASTNode op;
			op=new_leaf(UnaryOp,(void *)&(opr));
			assign_sibling(op,$2);
			$$=new_parent(Exp,NULL,op);
			{debug("Exp       ::=  + Exp\n");}
		}
		|MINUSSYM Exp %prec MINUSSYM{
			Opr opr=MINUS;
			ASTNode op;
			op=new_leaf(UnaryOp,(void *)&(opr));
			assign_sibling(op,$2);
			$$=new_parent(Exp,NULL,op);
			{debug("Exp       ::=  - Exp\n");}
		}
		|LBRACKET Exp RBRACKET{
			$$=$2;
			{debug("Exp       ::=  LBRACKET Exp RBRACKET\n");}
		}
		|LVal{
			$$=$1;
			{debug("Exp       ::=  LVal\n");}
		}
		|NUMBER{
			$$=new_leaf(Number,(void *)&($1));
			{debug("Exp       ::=  NUMBER\n");}
		}
		;



%%
char infn[100];
int errored=0;

int yyerror0(const char *message){
	errored=1;
	printf("%s:%d:%d:",infn,yylloc.first_line,yylloc.first_column);
	printf(message);
	return 0;
}

int yyerror1(const char *message, char *s){
	errored=1;
	printf("%s:%d:%d:",infn,yylloc.first_line,yylloc.first_column);
	printf(message,s);
	fflush(stdout);
	return 0;
}

int yyerror(char *message){
	errored=1;
	printf("%s:%d:%d:%s\n",infn,yylloc.first_line,yylloc.first_column,message);
	exit(0);
	return 0;
}


char outfn[100];
char astfn[100];
char *optstring = "o:";
struct option long_options[] = {
	{"ast",no_argument,NULL,'a'}
};
int option_index = 0;
int main(int argc,char **argv){
	FILE *infile,*outfile,*astfile;
	int oflag=0;
	int astflag=0;
	int iflag=0;
	int opt;
	opterr=0;
	while( (opt = getopt_long_only(argc,argv,optstring,long_options, &option_index) )!=-1){
//		printf("opt=%c\n",opt);
		switch(opt){
			case 'o':
				oflag=1;
				strcpy(outfn,optarg);
				break;
			case 'a':
				astflag=1;
				break;
		}
	}
	if(optind<argc){
		iflag=1;
		strcpy(infn,argv[optind]);
	}
	if(!iflag){
		fprintf(stderr,"Input file name not specified.\n");
		return -1;
	}
	infile=fopen(infn,"rt");
	if(!oflag){
		strcpy(astfn,infn);
		strcat(astfn,".out");
	}

		strcpy(outfn,infn);
{
	int length=strlen(outfn);
	if( (length>4) && (outfn[length-3]=='.') && (outfn[length-2]=='c') && (outfn[length-1]=='1') )
		outfn[length-3]='\0';
		strcat(outfn,".s");
}
	
//	printf("option checked over\n");
	
	extern FILE *yyin;
	yyin = infile;
	symtab_init();
	yyparse();
	//printf("parse over\n");
	if(errored){
		printf("error ocured in source program %s .\n",infn);
		if(astflag){
			printf("Ast tree not dumped.\n");
		}
		exit(-1);
	}else {
		if(astflag){
			astfile=fopen(astfn,"wt");
			fprintf(stderr,"ast file opened\n");
			dumpAST(astfile,root);
			printf("Ast tree dumped.\n");
			fclose(astfile);
		}
		code_generator(root);
		outfile=fopen(outfn,"wt");
		//fprintf(stderr,"%p\n",outfile);
		code_dump(outfile,root);
	}
	
	return 0;
}
