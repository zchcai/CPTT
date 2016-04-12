#include <stdio.h>
extern int lex_error, syntax_error;
extern struct Node* Head;
int main(int argc, char** argv) {
	if (argc <= 1) return 1;
	FILE* f = fopen(argv[1], "r");
	if (!f) {
		perror(argv[1]);
		return 1;
	}
	yyrestart(f);
	yyparse();
	if((lex_error == 0) && (syntax_error == 0)){
		/* Only Lab1 use this */
		print_node(Head, 0);
	}
	return 0;
}
