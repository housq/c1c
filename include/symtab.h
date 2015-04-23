#ifndef C1SYMTAB
#define	C1SYMTAB

typedef enum symtype{
	ExternFunction,
	Function,
	ConstScalar,
	VarScalar,
	ConstVector,
	VarVector
} SYMTYPE;

typedef enum memory_type{
	None,
	Stack,
	Heap
}MEMTYPE;

	

typedef struct sym{
	char	*name;
	SYMTYPE	type;
	int	size;
	struct sym 	*next;
	MEMTYPE	memtype;
	int	offset;
} *SYM;

typedef struct symtable{
	SYM	symlist;
	SYM	tail;
	struct symtable *parent;
	struct symtable *nextsibling;
	struct symtable *firstchild;
	int	size_local;
	int	size_total;
	int	size_max;
} *SYMTAB;


extern SYMTAB current_symtab;	

extern void symtab_init();
extern void symtab_new_level();
extern void symtab_return();

extern SYM sym_lookup(char *,SYMTAB);
extern SYM sym_deep_lookup(char *,SYMTAB);
extern SYM sym_tab_add(char *,SYMTYPE,SYMTAB,int);
extern SYM sym_add(char *,SYMTYPE,int);




#endif
