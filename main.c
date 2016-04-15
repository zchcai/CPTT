#include <stdio.h>
#include "syntax.tab.h"
#include "tree.h"
#include "symbol.c"
extern int lex_error, syntax_error;
extern struct Node* Head;
extern SNode SHead;
void dfs(struct Node*);
void doDec(struct Node*);
void doVarDec(int, struct Node*);
void doFunDec(struct Node*);
void checkId(int, char* name);
void semantic_error(int no, int line){
	/* TODO now just put error type */
	printf("Error type %d at Line %d\n", no, line);
}
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
	//	print_node(Head, 0); 

		/* When no lexical and syntax error,
		 * we start semantic analysis. */
		dfs(Head);

	}
	return 0;
}
void dfs(struct Node* root){
	if(root == NULL)return;
	//printf("%s\n", root->name);
	if(root->type == ExtDef){
		/* four main parts in program,
		 * here to construct symbol table
		 */
		//TODO Specifier analysis!!!!!!!!!
		//now consider Specifier is TYPE, INT
		struct Node* ch1 = root->child[1];
		struct Node* v = ch1->child[0];
		switch(ch1->type){
			case ExtDecList:
				while(ch1->childno == 3){
					doVarDec(0, v);//add ID with INT if not in symbol table.
					// 0 means declare
					ch1 = ch1->child[2];
					v = ch1->child[0];
				}
				doVarDec(0, v);
				return ;
			case SEMI:
				//TODO ExtDef : Specifier SEMI
				/*
				struct Node* ch0 = root->child[0];
				if(ch0 != NULL)ch0 = ch0 -> child[0];
				if(ch0 -> type == StructSpecifier && ch0 -> childno > 2)
					defineStruct(ch0);
					*/
				return ;

			case FunDec:
				//TODO: ch0 ch1 
				//ch2:  CompSt (now : no SEMI)
				doFunDec(ch1);//add ID to ST if not declare
				dfs(root->child[2]);
				return ;
		}
	}
	else if(root->type == Def){
		//Def : Specifier DecList SEMI
		//TODO Specifier analysis!!!!!!!!!
		//now consider Specifier is TYPE, INT
		struct Node* ch = root->child[1];
 		struct Node* v  = NULL;
		if(ch != NULL)v = ch->child[0];
		else printf("Def's child error!\n");
		while(v != NULL && ch != NULL && ch->childno == 3){
			doDec(v);//add ID with INT if not in symbol table.
			ch = ch->child[2];
			v = ch->child[0];
		}
		if(v != NULL)doDec(v);
		return ;
	}
	else if(root->type == ID){
		//TODO no type check!!!
		if(root->parent->childno >= 3 && root->parent->child[1]->type == LP){
			checkId(2, root->String);
		}
		else {
			checkId(1, root->String);
		}return ;
	}
	/* don't meet above */
	int i;
	int childno = root->childno;
	for(i = 0; i< childno;i++)
		dfs(root->child[i]);	
}
void doVarDec(int control, struct Node* p){
	if(control || (p == NULL)){
		printf("VarDec error!\n");
	}

	struct Node* ch = p->child[0];
	if(ch == NULL){
		printf("VarDec's child error!\n");
		return ;
	}
	if(stFind(ch->String) != NULL){
		semantic_error(3, ch->lineno);
		return ;
	}
	SNode s = stInitNode(ch->String);
	if(s != NULL)stInsert(s);

	if(p->childno == 1){
		s->funcOrVariable = VARIABLE;
		//TODO the specific type!!
	}
	else {
		printf("Oh, no! Array not analysis!");
		return ;
	}
	
}
void doFunDec(struct Node* p){
	//TODO with VarList
	struct Node* ch = p->child[0];
	if(stFind(ch->String) != NULL){
		semantic_error(4, ch->lineno);
		return ;
	}
	SNode s = stInitNode(ch->String);
	stInsert(s);
	
	if(p->childno == 3){
		s->funcOrVariable = FUNC;
		//TODO Message!!!
	}
	else {
		printf("Oh, no! VarList not analysis!");
	}
	return ;
}
void doDec(struct Node* p){
	if(p == NULL){
		printf("error in doDec function.\n");
		return ;
	}
	doVarDec(0, p->child[0]);
	return;
}

void checkId(int error_type, char* name){
	if(stFind(name) == NULL){
		semantic_error(error_type, -1);
		return ;
	}
	return ;
}
/*
void defineStruct(struct Node* root){
	// root -> type == StructSpecifier 
	if(root == NULL)return;
	ptag = root -> child[1];
	if(ptag->childno){
		SNode p = stFind(ptag->name);
		if(p != NULL)semantic_error(3, ptag->lineno);
		p = stInitNode(ptag->name);
		if(p == NULL)printf("Symbol table error!\n");
		stInsert(p);
		p -> funcOrVariable = VARIABLE;
		p -> Variable = structInit(root->child[3]);


Type structInit(struct Node* root){
	Type p = (Type)malloc(sizeof(Type_));
	p -> kind = STRUCTURE;
	FieldList head = (FieldList)malloc(sizeof(FieldList_));
	while(root -> childno){
		struct Node* def = root->child[0];
		// check define or not 
		struct Node* strOrType = def->child[0]->child[0];
		if(strOrType->type != TYPE){
			



		root = root->child[1];
	}
	p -> structure = head;
	return p;
}
*/
