CMMFILES	= $(shell find test/ -name "*.cmm")
FLEX	= lexical.l
SYN	= syntax.y
FLEX_C	= lex.yy.c
SYMBOL_SOURCE = symbol.c symbol.h
parser	: $(FLEX_C) $(SYMBOL_SOURCE) syntax.tab.c main.c
	gcc main.c syntax.tab.c -ll -lfl -ly -o parser 

$(FLEX_C) : $(FLEX)
	flex $(FLEX)

syntax.tab.h syntax.tab.c : $(SYN)
	bison -d $(SYN)

test	: parser
	for cmm in $(CMMFILES);\
		do\
			echo "$$cmm";\
			./parser $$cmm;\
		done
clean	:
	rm parser
