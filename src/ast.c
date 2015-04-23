#include "symtab.h"
#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ASTNode root;

char oprstring[14][10]={
	" odd ",
	"==",
	"!=",
	"<",
	">",
	"<=",
	">=",
	"+",
	"-",
	"*",
	"/",
	"%",
	"+",
	"-"
};
	

ASTNode	new_leaf(Ast_type type,void *value){
	ASTNode node;
	node = (ASTNode)malloc(sizeof(*node));
	node->type = type;
	switch(type){
		case Ident:
			node->sym=(SYM)value;
			break;
		case Number:
			node->val=*(int *)value;
			break;
		case BinOp:
		case UnaryOp:
		case RelOp:
			node->opr=*(Opr *)value;
			break;
		case Block:
		case OddOp:
		case NilStmt:
			break;
		case CompilerDirective:
			node->str=(char  *)value;
		//	fprintf(stderr,"directive string is at %p\n",node->str);
		//	fprintf(stderr,"directive string is  %s\n",node->str);
			break;
		default:
			fprintf(stderr,"leaf node error\n");
	}
	//node->loc = loc;
	node->parent = node->firstchild = node->nextsibling = node->lastsibling = NULL;
	return node;
}

ASTNode new_parent(Ast_type type,void *value,ASTNode firstchild){
	ASTNode node;
	node = (ASTNode)malloc(sizeof(*node));
	node->type = type;
	switch(type){
		case Program:
		case Block:
			node->symtab=(SYMTAB)value;
			break;
	}

	firstchild->parent=node;
	node->firstchild=firstchild;
//	if(node->firstchild==NULL)printf("*****************");
	return node;
}

void dumpAST(FILE *fp,ASTNode node){
	int flag,i;
	ASTNode child=node->firstchild;
	switch(node->type){
		case Program:
			fprintf(fp,"AST dump start\n");
			while(child){
				dumpAST(fp,child);
				child=child->nextsibling;
			}
			break;
		case VarDecl:
			fprintf(fp,"int ");
			while(child){
				dumpAST(fp,child);
				child=child->nextsibling;
				if(child) fprintf(fp,",");
			}
			fprintf(fp,";\n");
			break;
		case ConstDecl:
			fprintf(fp,"const int ");
			while(child){
				dumpAST(fp,child);
				child=child->nextsibling;
				if(child) fprintf(fp,",");
			}
			fprintf(fp,";\n");
			break;
		case FuncDecl:
			fprintf(fp,"extern void ");
			dumpAST(fp,child);
			fprintf(fp,"();");
			break;
		case FuncDef:
			fprintf(fp,"void ");
			dumpAST(fp,child);
			child=child->nextsibling;
			fprintf(fp,"()\n");
			dumpAST(fp,child);
			break;
		case IfClause:
			fprintf(fp,"if ");
			dumpAST(fp,child);
			child=child->nextsibling;
			dumpAST(fp,child);
			child=child->nextsibling;
			if(child){
				fprintf(fp,"else \n");
				dumpAST(fp,child);
			}
			break;
		case WhileClause:
			fprintf(fp,"while ");
			dumpAST(fp,child);
			fprintf(fp,"\n");
			child=child->nextsibling;
			dumpAST(fp,child);
			break;
		case Block:
			fprintf(fp,"{\n");
			while(child){
				dumpAST(fp,child);
				child=child->nextsibling;
//				if(child) fprintf(fp,"\n");
			}
			fprintf(fp,"}\n");
			break;
		case AssignExp:
			dumpAST(fp,child);
			fprintf(fp," = ");
			child=child->nextsibling;
			dumpAST(fp,child);
			fprintf(fp,";\n");
			break;
		case FunctionCall:
			dumpAST(fp,child);
			fprintf(fp,"();\n");
			break;
		case NilStmt:
			fprintf(fp,";\n");
			break;
		case Cond:
			while(child){
				dumpAST(fp,child);
				fprintf(fp," ");
				child=child->nextsibling;
			}
			break;
		case Exp:
			flag=(child->nextsibling) && (child->nextsibling->type == BinOp ) && ( (child->nextsibling->opr == PLUS) || (child->nextsibling->opr == MINUS) );
			if(flag) fprintf(fp,"(");
			while(child){
				dumpAST(fp,child);
				child=child->nextsibling;
			}
			if(flag) fprintf(fp,")");
			break;
		case LVal:
			dumpAST(fp,child);
			child=child->nextsibling;
			if(child){
				fprintf(fp,"[");
				dumpAST(fp,child);
				fprintf(fp,"]");
				child=child->nextsibling;
			}
			break;
		case VarArray:
			dumpAST(fp,child);
			child=child->nextsibling;
			fprintf(fp,"[");
			dumpAST(fp,child);
			fprintf(fp,"]");
			break;
		case ConstIdent:
			dumpAST(fp,child);
			fprintf(fp,"=");
			child=child->nextsibling;
			dumpAST(fp,child);
			break;
		case ConstArray:
			dumpAST(fp,child);
			fprintf(fp,"[");
			child=child->nextsibling;
			if(child->type==Number){
				dumpAST(fp,child);
				child=child->nextsibling;
			}
			fprintf(fp,"]={");
			dumpAST(fp,child);
			fprintf(fp,"}");
			break;
		case ConstList:
			while(child){
				dumpAST(fp,child);
				child=child->nextsibling;
				if(child)fprintf(fp,",");
			}
			break;
		case Ident:
			fprintf(fp,"%s",node->sym->name);
			break;
		case Number:
			fprintf(fp,"%d",node->val);
			break;
		case RelOp:
		case OddOp:
		case BinOp:
		case UnaryOp:
			fprintf(fp,"%s",oprstring[node->opr]);
			break;
		case CompilerDirective:
			//fprintf(stderr,"directive string is at %p\n",node->str);
			//fprintf(stderr,"directive string is  %s\n",node->str);
			fprintf(fp,"%s",node->str);
			
			
			
			
	}
}

void assign_sibling(ASTNode left,ASTNode right){
	left->nextsibling = right;
	right->lastsibling = left;
}
	
			
