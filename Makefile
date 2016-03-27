CMMFILES	= $(shell find test/ -name "*.cmm")
OBJS	= ex1.l ex1.y
parser	: $(OBJS)
	flex ex1.l
	bison -d ex1.y
	gcc -o parser ex1.tab.c -ll -lfl -ly
debug	: $(OBJS)
	flex ex1.l
	bison -d -t ex1.y
	gcc -o parser ex1.tab.c -ll -lfl -ly
test	: parser
	for cmm in $(CMMFILES);\
		do\
			echo "$$cmm";\
			./parser $$cmm;\
		done
clean	:
	rm parser
