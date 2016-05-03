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
void doExtDecList(Node* p, Type* type);
void doFunDec(Node*, Type*);
int check(Type*, Type*);
SNode* doParamDec(Node* p);
Type* doSpecifier(Node* p);
Type* doExp(Node* p);
void doDefInCompSt(Node* p);
void doDecInCompSt(Node* p, Type* type);
Type* doDefListInStruct(Node* p);
FieldList* doDecInStruct(Node*, Type*);
Type* doVarDec(Node*, Type*);
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
		// print_node(Head, 0); 
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
	if(root -> type == ExtDef){
		/* four main parts in program,
		 * here to construct symbol table
		 */
		Type* type_result = doSpecifier(root -> child[0]);
		Node* ch1 = root->child[1];
		switch(ch1->type){
			case ExtDecList:
				doExtDecList(ch1, type_result);
				return ;
			case SEMI:
				//ExtDef : Specifier SEMI
				return ;
			case FunDec:
				//ch2:  CompSt (now : no SEMI)
				doFunDec(ch1, type_result);
				dfs(root->child[2]);//TODO
				return ;
		}
	}
	else if(root -> type == CompSt){
		/* TODO multi fields function */
	}
	else if(root -> type == DefList){
		if(root -> childno){
			doDefInCompSt(root -> child[0]);
			dfs(root -> child[1]);
			return ;
		}
	}
	/* don't meet above */
	int i;
	int childno = root->childno;
	for(i = 0; i< childno;i++)
		dfs(root->child[i]);	
	/* L */
	if(root -> type == Exp){
		Type* drop = doExp(root);
		//TODO type check
	}
	else if(root -> type == Stmt){
		//TODO exp type check in while and if
		//also for return type
	}
}
void doExtDecList(Node* p, Type* type){
	assert((p != NULL) && p -> childno);
	Type* finaltype = doVarDec(p -> child[0], type);
	Node* tp = p -> child[0];
	while(tp -> childno){
		tp = tp -> child[0];
	}
	SNode* s = stFind(tp -> String);
	if(s != NULL){
		semantic_error(3, tp -> lineno);
	}
	else {
		s = stInitNode(tp -> String);
		s -> visitedTag = VARIABLE;
		s -> lineno = tp -> lineno;
		s -> Message.var = finaltype;
		stInsert(s);
	}
	if(p -> childno == 3)doExtDecList(p -> child[2], type);
}
Type* doVarDec(Node* p, Type* type){
	Type* finaltype = type;
	if(p -> childno == 4){
		Type* pvar = (Type*)malloc(sizeof(Type));
		pvar -> kind = ARRAY;
		pvar -> u.array.elem = doVarDec(p -> child[0], type);
		pvar -> u.array.size = p -> child[2] -> value.Int;
		finaltype = pvar;
	}
	return finaltype;
}
void doFunDec(Node* p, Type* pt){
	assert(p != NULL);
	Node* ch = p->child[0];
	if(stFind(ch->String) != NULL){
		/* a bit different with PDF:
		 * conflict with any other symbol */
		semantic_error(4, ch->lineno);
		return ;
	}
	SNode* s = stInitNode(ch->String);
	stInsert(s);
	s -> visitedTag = FUNC;
	s -> lineno = p -> lineno;
	Funcmsg* pfunc = (Funcmsg*)malloc(sizeof(Funcmsg));
	pfunc -> returnType = pt;
	pfunc -> para.head = NULL;
	
	assert((p -> childno == 3) || (p -> childno == 4));
	pfunc -> para.no = 0;
	if(p -> childno == 4){
		p = p -> child[2];
		while(p -> childno == 3){
			SNode* ppara = doParamDec(p -> child[0]);
			p = p -> child[2];
			if(ppara != NULL){
				SNode* temp = pfunc -> para.head;
				pfunc -> para.head = ppara;
				ppara -> fnext = temp;
				pfunc -> para.no ++;
			}
		}
		SNode* ppara = doParamDec(p -> child[0]);
		if(ppara != NULL){
			SNode* temp = pfunc -> para.head;
			pfunc -> para.head = ppara;
			ppara -> fnext = temp;
			pfunc -> para.no ++;
		}
	}
	s -> Message.func = pfunc;
	return ;
}
SNode* doParamDec(Node* p){
	/* simplify: can def struct in func def */
	// TODO
	Type* type = doSpecifier(p -> child[0]);
	type = doVarDec(p -> child[1], type);
	p = p -> child[1];
	while(p -> childno != 1){
		assert(p != NULL);
		p = p -> child[0];
	}
	p = p -> child[0];
	SNode* ps = stFind(p -> String);
	if(ps != NULL){
		/* conflict with existed sym */
		semantic_error(3, p -> lineno);
		ps = NULL;
	}
	else{
		ps = stInitNode(p -> String);
		ps -> visitedTag = VARIABLE;
		ps -> lineno = p -> lineno;
		ps -> Message.var = type;
		stInsert(ps);
	}
	return ps;
}
Type* doSpecifier(Node* p){
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
void doDefInCompSt(Node* p){
	Type* type = doSpecifier(p -> child[0]);
	p = p -> child[1];
	while(p -> childno == 3){
		doDecInCompSt(p -> child[0], type);
		p = p -> child[2];
	}
	doDecInCompSt(p -> child[0], type);
}
void doDecInCompSt(Node* p, Type* type){
	Node* pvar = p -> child[0];
	Type* valtype = doVarDec(pvar, type);
	while(pvar -> type != ID){
		assert(pvar != NULL);
		pvar = pvar -> child[0];
	}
	SNode* s = stFind(pvar -> String);
	if(s != NULL){
		semantic_error(3, pvar -> lineno);
		return ;
	}
	else {
		s = stInitNode(pvar -> String);
		s -> visitedTag = VARIABLE;
		s -> Message.var = valtype;
		stInsert(s);
	}
	if(p -> childno == 3){
		Type* asstype = doExp(p -> child[2]);
		if(check(valtype, asstype) != 0){
			semantic_error(5, p -> child[2] -> lineno);
		}
	}
}
Type* doExp(Node* p){
	if(p -> exptype != NULL){
		return p -> exptype;
	}
	int childno = p -> childno;
	Type* finaltype = NULL;
	if(childno == 1){
		Node* ch = p -> child[0];
		if(ch -> type == ID){
			SNode* s = stFind(ch -> String);
			if(s == NULL){
				semantic_error(1, ch -> lineno);
			}
			else{
				if(s -> visitedTag != VARIABLE){
					semantic_error(1, ch -> lineno);
				}
				else {
					finaltype = s -> Message.var;
				}
			}
		}
		else if(ch -> type == INT){
			finaltype = TypeNodeInt;
		}
		else if(ch -> type == FLOAT){
			finaltype = TypeNodeFloat;
		}
	}
	else if(childno == 2){
		Node* ch = p -> child[1];
		Type* temp = doExp(ch);
		if(ch -> type == MINUS){
			if(isArith(temp)){
				finaltype = temp;
			}
			else{
				semantic_error(7, ch -> lineno);
				finaltype = TypeNodeInt;
			}
		}
		else if(ch -> type == NOT){
			if(!isLogic(temp)){
				semantic_error(7, ch -> lineno);
			}
			finaltype = TypeNodeInt;
		}
	}
	else if(childno == 3){
		Node* ch1 = p -> child[1];
		if(ch1 -> type == DOT){
			Type* t = doExp(p -> child[0]);
			if(t -> kind != STRUCTURE){
				semantic_error(13, p -> lineno);
				finaltype = TypeNodeInt;
			}
			else {
				Type* ft = sfFind(t, p -> child[2] -> String);
				if(ft == NULL){
					semantic_error(14, p -> lineno);
					finaltype = TypeNodeInt;
				}
				else {
					finaltype = ft;
				}
			}
		}
		else if(ch1 -> type == LP){
			//TODO
		}
		else if(ch1 -> type == Exp){
			finaltype = doExp(ch1);
		}
		else if(ch1 -> type == ASSIGNOP){
			//TODO
		}
		else if(ch1 -> type == RELOP){
			//TODO
		}
		else if((ch1 -> type == AND) || (ch1 -> type == OR)){
			//TODO
		}
		else {
			//TODO
		}
	}
	else if(childno == 4){
		//TODO
	}
	p -> exptype = finaltype;
	return finaltype;
}
int isArith(Type* t){
	return ((check(t, TypeNodeInt) == 0) || (check(t, TypeNodeFloat) == 0));
}
int isLogic(Type* t){
	return (check(t, TypeNodeInt) == 0);
}
int check(Type* a, Type* b){
	if(a == b)return 0;//no difference
	if(a -> type != b -> type)return -1;
	if(a -> type == BASIC){
		if(a -> u.basic == b -> u.basic)return 0;
		else return -1;
	}
	else if(a -> type == ARRAY){
		return check(a -> u.elem, b -> u.elem);
	}
	else if(a -> type == STRUCTURE){
	//TODO
	}
	return -1;
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
			if(sfFind(theType, pf -> name) == NULL){
				FieldList* temp = theType -> u.structure;
				theType -> u.structure = pf;
				pf -> tail = temp;
			}
			else {
				semantic_error(15, p -> lineno);
			}
			pdeclist = pdeclist -> child[2];
		}
		FieldList* pf = doDecInStruct(pdeclist -> child[0], valtype);
		if(sfFind(theType, pf -> name) == NULL){
			FieldList* temp = theType -> u.structure;
			theType -> u.structure = pf;
			pf -> tail = temp;
		}
		else {
			semantic_error(15, p -> lineno);
		}
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
	pf -> type = doVarDec(p, valtype);
	while(p -> childno == 4){
		p = p -> child[0];
	}
	strcpy(pf -> name, p -> String);
	return pf;
}
