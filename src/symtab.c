#include "util.h"
#include "symtab.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

SYMTAB current_symtab;

void symtab_init(){
	current_symtab = (SYMTAB)malloc(sizeof(*current_symtab));
	current_symtab->symlist = \
	current_symtab->tail = NULL;
	current_symtab->parent = \
	current_symtab->nextsibling = \
	current_symtab->firstchild = NULL;
	current_symtab->size_local = \
	current_symtab->size_total = \
	current_symtab->size_max = 0;
}

void symtab_new_level(){
	SYMTAB newtab;
	newtab = (SYMTAB)malloc(sizeof(*newtab));
	newtab->symlist = \
	newtab->tail = NULL;
	newtab->parent = current_symtab;
	newtab->firstchild = NULL;
	newtab->size_local = \
	newtab->size_total = \
	newtab->size_max = 0;
	newtab->nextsibling = current_symtab->firstchild;
	current_symtab->firstchild = newtab;
	current_symtab = newtab;
}

void symtab_return(){
	SYMTAB p;
	p = current_symtab->parent;
	current_symtab->size_total+=current_symtab->size_local;
	p->size_total+=current_symtab->size_total;
	p->size_max+=p->size_local;
	if(current_symtab->size_max > p->size_max)
		p->size_max=current_symtab->size_max;
	//printf("symtab return : total local max :%d %d %d \n",current_symtab->size_total,current_symtab->size_local,current_symtab->size_max);
	current_symtab=p;
}
	

SYM sym_lookup(char *name , SYMTAB table){
	SYM	find=table->symlist;
	while (find ){
		if(strcmp(name,find->name)==0) return find;
		find=find->next;
	}
	return NULL;
}

SYM sym_deep_lookup(char *name , SYMTAB table){
	SYMTAB	looktab=table;
	SYM	tmp;
	while (looktab){
		if(tmp=sym_lookup(name,looktab) ) return tmp;
		looktab=looktab->parent;
	}
	return NULL;
}

SYM sym_table_add(char *name, SYMTYPE type, SYMTAB table,int size){
	SYM	sym;
	sym = (SYM)malloc(sizeof(*sym));
	sym->name = (char *)malloc(strlen(name)+1);
	strcpy(sym->name,name);
	sym->type=type;
	sym->next=NULL;
	sym->size=size;
	if(table->tail){
		table->tail->next=sym;
	}else{
		table->symlist=sym;
	}
	table->tail=sym;
	table->size_local+=size;
	return sym;
}
	

SYM sym_add(char *name, SYMTYPE type,int size){
	return sym_table_add(name,type,current_symtab,size);
}
