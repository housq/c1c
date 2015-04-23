#ifndef	C1AST
#define C1AST

#include "symtab.h"
#include "code_generator_struct.h"

typedef enum{
	ODD=0,
	EQL=1,
	NEQ=2,
	LSS=3,
	GTR=4,
	LEQ=5,
	GEQ=6,
	PLUS=7,
	MINUS=8,
	MULTI=9,
	DIVIDE=10,
	MOD=11,
	POSITIVE=12,
	NEGATIVE=13
} Opr;

typedef struct location {
	int first_line;
	int first_column;
	int last_line;
	int last_column;
} Loc;

typedef enum{
/** structure **/
	Program,
	VarDecl,
	ConstDecl,
	FuncDecl,
	FuncDef,
/** statement **/
	IfClause,
	WhileClause,
	Block,
	AssignExp,
	FunctionCall,
	NilStmt,
/** expression **/
	Cond,
	Exp,
	LVal,
	VarArray,
	ConstIdent,
	ConstArray,
	ConstList,
/** leaf **/
	Ident,
	Number,
	RelOp,
	OddOp,
	BinOp,
	UnaryOp,
/** directive **/
	CompilerDirective
} Ast_type;

typedef struct astnode{
	Ast_type	type;
	union {
		SYM	sym;		//Ident
		Opr	opr;		//RelOp BinOp UnaryOp
		int	val;		//Number
		SYMTAB	symtab;		//Block CompUnit
	};
	char	*str;		//CompilerDirective
	int	list_size;	//ConstList
	Loc		loc;
	struct astnode 	*parent;
	struct astnode 	*firstchild;
	struct astnode	*nextsibling;
	struct astnode	*lastsibling;
	CODELIST	codes;
} *ASTNode;	

typedef ASTNode	ASTTree;	

extern ASTNode new_leaf(Ast_type type,void *value);
extern ASTNode new_parent(Ast_type type,void *value, ASTNode first_child);
extern void assign_sibling(ASTNode left,ASTNode right);

extern ASTNode root;


#endif
