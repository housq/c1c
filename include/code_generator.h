#ifndef CODE_GENERATOR_H
#define	CODE_GENERATOR_H

#include <stdio.h>
#include "symtab.h"
#include "ast.h"
#include "code_generator_struct.h"


extern int stack_allocate(SYMTAB,int);
extern int code_function(ASTNode);
extern int code_program(ASTNode);
extern int code_generator(ASTNode);
extern int code_dump(FILE *, ASTNode);
CODE86 *newcode(OPR opr, DESSRC *src, DESSRC *des, void *descriptor);
//DESSRC *newdessrc(DESSRCTYPE,int,SYM,int,int factor);




#endif
