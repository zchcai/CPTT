#include <stdio.h>
#include "syntax.tab.h"
#include "tree.h"
extern int lex_error, syntax_error;
extern struct Node* Head;
void dfs(struct Node*);
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
		// print_node(Head, 0); 

		/* When no lexical and syntax error,
		 * we start semantic analysis. */
		dfs(Head);

	}
	return 0;
}
void dfs(struct Node* root){
	if(root == NULL)return;
	if(root->type == ExtDef){
		/* four main parts in program,
		 * here to construct symbol table
		 */
		printf("There is a ExtDef!\n");
		struct Node* ch1 = root->child[1];
		switch(ch1->type){
			case ExtDecList:
				//TODO: ch0 and ch1
				return ;
			case SEMI:
				//TODO: ch0
				struct Node* ch0 = root->child[0];

				return ;

			case FunDec:
				//TODO: ch0 ch1 
				//ch2: SEMI or CompSt
				return ;
		}
	}
	/* don't meet ExtDef */
	int i;
	int childno = root->childno;
	for(i = 0; i< childno;i++)
		dfs(root->child[i]);	
}
