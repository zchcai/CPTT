#include <assert.h>
#include <stdio.h>
#include "syntax.tab.h"
#include "tree.h"
#include "symbol.c"
extern int lex_error, syntax_error;
extern Node* Head;
extern SNode* SHead;
extern Type* TypeNodeInt;
extern Type* TypeNodeFloat;
void dfs(Node*);
void doDec(Node*);
void doVarDec(int, Node*);
void doFunDec(Node*);
void checkId(int, char* name);
Type* doSpecifier(Node* p);
Type* doDefListInStruct(Node* p);
FieldList* doDecInStruct(Node*, Type*);
Type* doVarDecInStruct(Node*, Type*);
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
		/* for test in lab2 */
		stPrint();
	}
	return 0;
}
void dfs(Node* root){
	if(root == NULL)return;
	if(root->type == ExtDef){
		/* four main parts in program,
		 * here to construct symbol table
		 */
		//TODO Specifier analysis!!!!!!!!!
		Type* type_result = doSpecifier(root -> child[0]);
		Node* ch1 = root->child[1];
		Node* v = ch1->child[0];
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
		Node* ch = root->child[1];
 		Node* v  = NULL;
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
void doVarDec(int control, Node* p){
	if(control || (p == NULL)){
		printf("VarDec error!\n");
	}

	Node* ch = p->child[0];
	assert(ch != NULL);
	if(stFind(ch->String) != NULL){
		semantic_error(3, ch->lineno);
		return ;
	}
	SNode* s = stInitNode(ch->String);
	if(s != NULL)stInsert(s);

	if(p->childno == 1){
		s->visitedTag = VARIABLE;
		//TODO the specific type!!
	}
	else {
		printf("Oh, no! Array not analysis!");
		return ;
	}
	
}
void doFunDec(Node* p){
	//TODO with VarList
	Node* ch = p->child[0];
	if(stFind(ch->String) != NULL){
		semantic_error(4, ch->lineno);
		return ;
	}
	SNode* s = stInitNode(ch->String);
	stInsert(s);
	
	if(p->childno == 3){
		s->visitedTag = FUNC;
		//TODO Message!!!
	}
	else {
		printf("Oh, no! VarList not analysis!");
	}
	return ;
}
void doDec(Node* p){
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
Type* doSpecifier(Node* p){
	printf("aaaaaa\n");
	assert((p != NULL) && (p -> childno));
	p = p -> child[0];
	assert(p != NULL);
	int t = p -> type;
	if(t == TYPE){
		if(strcmp(p -> String, "int") == 0){
			return TypeNodeInt;
		}
		else if(strcmp(p -> String, "float") == 0){
			return TypeNodeFloat;
		}
	}
	else if(t == StructSpecifier){
		assert((p -> childno == 2) || (p -> childno == 5));
		if(p -> childno == 2){
			/* def var */
			p = p -> child[1];
			assert((p != NULL) && (p -> childno));
			p = p -> child[0];
			SNode* stnode = stFind(p -> String);
			if(stnode == NULL){
				/* struct id but id not in symbol table */
				semantic_error(17, p -> lineno);
				return NULL;
			}
			else {
				if(stnode -> visitedTag != STRUCTDEF){
					/* conflict with exist name */
					semantic_error(16, p -> lineno);
				}
				else {
					return stnode -> Message.var;
				}
			}
		}
		else if(p -> childno == 5){
			/* def structure */
			Type* for_return = doDefListInStruct(p -> child[3]);
			if(p -> child[1] -> childno){
				p = p -> child[1] -> child[0];
				assert(p!=NULL);
				SNode* stnode = stFind(p -> String);
				if(stnode == NULL){
					SNode* newsym = stInitNode(p -> String);
					newsym -> visitedTag = STRUCTDEF;
					newsym -> lineno = p -> lineno;
					newsym -> Message.var = for_return;
					stInsert(newsym);
				}
				else {
					/* conflict with existed symbol */
					semantic_error(16, p -> lineno);
				}
			}
			return for_return;
		}
	}
	else {
		printf("error in doSpecifier function\n");
	}
	return NULL;
}
Type* doDefListInStruct(Node* p){
	Type* theType = (Type*)malloc(sizeof(Type));
	theType -> kind = STRUCTURE;
	theType -> u.structure = NULL;
	while(p -> childno == 2){
		Node* pdef = p -> child[0];
		assert(pdef != NULL);
		Type* valtype = doSpecifier(pdef -> child[0]);
		Node* pdeclist = pdef -> child[1];
		while(pdeclist -> childno == 3){
			FieldList* pf = doDecInStruct(pdeclist -> child[0], valtype);
			FieldList* temp = theType -> u.structure;
			theType -> u.structure = pf;
			pf -> tail = temp;

			pdeclist = pdeclist -> child[2];
		}
		FieldList* pf = doDecInStruct(pdeclist -> child[0], valtype);
		FieldList* temp = theType -> u.structure;
		theType -> u.structure = pf;
		pf -> tail = temp;
	
		p = p -> child[1];
	}
	return theType;
}
FieldList* doDecInStruct(Node* p, Type* valtype){
	FieldList* pf = (FieldList*)malloc(sizeof(FieldList));
	pf -> tail = NULL;
	if(p -> childno == 3){
		/* init in struct definition */
		semantic_error(15, p -> lineno);
	}
	p = p -> child[0];
	pf -> type = doVarDecInStruct(p, valtype);
	while(p -> childno == 4){
		p = p -> child[0];
	}
	strcpy(pf -> name, p -> String);
	return pf;
}
Type* doVarDecInStruct(Node* p, Type* valtype){
	if(p -> childno == 4){
		Type* pvar = (Type*)malloc(sizeof(Type));
		pvar -> kind = ARRAY;
		pvar -> u.array.elem = doVarDecInStruct(p -> child[0], valtype);
		pvar -> u.array.size = p -> child[2] -> value.Int;
		return pvar;
	}
	else {
		return valtype;
	}
}
