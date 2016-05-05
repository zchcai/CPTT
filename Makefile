CMMFILES_1	= $(shell find test/1/ -name "*.cmm")
CMMFILES_2	= $(shell find test/2/ -name "*.cmm")
CMMFILES_3	= $(shell find test/3/ -name "*.cmm")
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

test1	: parser
	for cmm in $(CMMFILES_1);\
		do\
			echo "$$cmm";\
			./parser $$cmm;\
		done

test2	: parser
	for cmm in $(CMMFILES_2);\
		do\
			echo "$$cmm";\
			./parser $$cmm;\
		done

test3	: parser
	for cmm in $(CMMFILES_3);\
		do\
			echo "$$cmm";\
			./parser $$cmm;\
		done
clean	:
	rm parser
