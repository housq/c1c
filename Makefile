CINC	 = -Iinclude -Isrc
INC	 = include
CONF	 = config
SRC	 = src
BIN	 = bin
UTILSRC  = $(SRC)/list.c $(SRC)/error.c $(SRC)/ast.c

CC       = gcc -g $(CINC)
LEX      = flex -i -I 
YACC     = bison -d -y

all: $(BIN)/c1c

# Generates a compiler for a simple expression language,
# the grammar is ambiguous, and uses precedence declarations
$(BIN)/c1c:   $(SRC)/c1cutil.o $(SRC)/c1.lex.o $(SRC)/c1.tab.o $(SRC)/ast.o $(SRC)/symtab.o $(SRC)/code_generator.o
	$(CC) -o $@ $^ -ll 

$(SRC)/c1.lex.o: $(SRC)/c1.lex.c $(SRC)/c1.tab.h $(INC)/util.h
	$(CC) -c -o $@ $(SRC)/c1.lex.c

$(SRC)/c1.tab.o: $(SRC)/c1.tab.c $(INC)/util.h
	$(CC) -c -o $@ $(SRC)/c1.tab.c

$(SRC)/%.lex.c: $(CONF)/%.lex
	$(LEX) -o $@ $<

$(SRC)/c1.tab.c $(SRC)/c1.tab.h: $(CONF)/c1.y
	$(YACC) -b $(SRC)/c1 $<

clean:
	rm -f *.BAK $(SRC)/*.o *.o core *~* *.a 
	rm -f $(SRC)/*.tab.h $(SRC)/*.tab.c
	rm -f $(SRC)/*.lex.c *.out
	rm -f $(BIN)/c1c
