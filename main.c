#include <assert.h>
#include <stdio.h>
#include "syntax.tab.h"
#include "tree.h"
#include "symbol.c"
extern int lex_error, syntax_error;
extern Node* Head;
extern SNode* SHead;
extern SNode* FunDeclarHead;
extern Type* TypeNodeInt;
extern Type* TypeNodeFloat;
Type* currentFuncReturnType = NULL;
void dfs(Node*);
void doExtDecList(Node* p, Type* type);
void doFunDecInDeclar(Node* p, Type* pt);
void doFunDec(Node*, Type*);
int check(Type*, Type*);
FieldList* doParamDecInDeclar(Node* p);
SNode* doParamDec(Node* p);
Type* doSpecifier(Node* p);
Type* doExp(Node* p);
void doDefInCompSt(Node* p);
void doDecInCompSt(Node* p, Type* type);
Type* doDefListInStruct(Node* p);
FieldList* doDecInStruct(Node*, Type*);
int doArgs(Node* p, Funcmsg* fm);
void DeclarAndDefine();
Type* doVarDec(Node*, Type*);
void semantic_print_error(int no, int line, char* msg);
void semantic_error(int no, int line);
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

		/*
		 * When no lexical and syntax error,
		 * we start semantic analysis. 
		 */
		dfs(Head);
		DeclarAndDefine();
		/* for test in lab2 */
		// stPrint();
	}
	return 0;
}
void DeclarAndDefine(){
	SNode* p = FunDeclarHead;
	while(p != NULL){
		SNode* temp = stFind(p -> name);
		if(temp == NULL || temp -> visitedTag != FUNC){
			semantic_print_error(18, p -> lineno, p -> name);
		}
		else {
			if(check(p -> Message.declar -> returnType, temp -> Message.func -> returnType) != 0){
				semantic_print_error(19, p -> lineno, p -> name);
			}
			else {
				FieldList* af = p -> Message.declar -> head;
				SNode* bs = temp -> Message.func -> para.head;
				while((af != NULL) || (bs != NULL)){
					if(((bs == NULL) && (af !=NULL)) || ((af == NULL) && (bs !=NULL))){
						semantic_print_error(19, p -> lineno, p -> name);
						break;
					}
					if(check(af -> type, bs -> Message.var) != 0){
						semantic_print_error(19, p -> lineno, p -> name);
						break;
					}
					af = af -> tail;
					bs = bs -> fnext;
				}
			}
		}
		p = p -> next;
	}
}
void dfs(Node* root){
	if(root == NULL)return;
	if(root -> type == ExtDef){
		/* four main parts in program,
		 * here to construct symbol table
		 */
		Type* type_result = doSpecifier(root -> child[0]);
		if(type_result == NULL){
			/* Hypothesis:
			 * This is a stupid error */
			type_result = TypeNodeInt;
		}
		Node* ch1 = root->child[1];
		switch(ch1->type){
			case ExtDecList:
				doExtDecList(ch1, type_result);
				return ;
			case SEMI:
				//ExtDef : Specifier SEMI
				return ;
			case FunDec:
				//ch2:  CompSt 
				if(root -> child[2] -> type == CompSt){
					doFunDec(ch1, type_result);
					currentFuncReturnType = type_result;
					dfs(root->child[2]);//TODO
					currentFuncReturnType = NULL;
					return ;
				}
				/* ch2: SEMI */
				else {
					doFunDecInDeclar(ch1, type_result);
					return ;
				}
		}
	}
	else if(root -> type == CompSt){
		/* TODO multi fields function */
		assert(currentFuncReturnType != NULL);
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
	}
	else if(root -> type == Stmt){
		int childno = root -> childno;
		// for return type
		if(childno == 3){
			if(check(doExp(root -> child[1]), currentFuncReturnType) != 0){
				semantic_error(8, root -> lineno);
			}
		}
		// exp type check in while and if
		else if(childno == 5 || childno == 7){
			if(!isLogic(doExp(root -> child[2]))){
				semantic_error(7, root -> lineno);
			}
		}
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
		semantic_print_error(3, tp -> lineno, tp -> String);
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
void doFunDecInDeclar(Node* p, Type* pt){
	assert(p != NULL);
	Node* ch = p->child[0];
	SNode* s = stInitNode(ch->String);
	s -> visitedTag = DECLAR;
	s -> lineno = p -> lineno;
	Declarmsg* pfunc = (Declarmsg*)malloc(sizeof(Declarmsg));
	pfunc -> returnType = pt;
	pfunc -> head = NULL;
	
	assert((p -> childno == 3) || (p -> childno == 4));
	pfunc -> no = 0;
	if(p -> childno == 4){
		p = p -> child[2];
		while(p -> childno == 3){
			FieldList* ppara = doParamDecInDeclar(p -> child[0]);
			p = p -> child[2];
			if(ppara != NULL){
				FieldList* temp = pfunc -> head;
				pfunc -> head = ppara;
				ppara -> tail = temp;
				pfunc -> no ++;
			}
		}
		FieldList* ppara = doParamDecInDeclar(p -> child[0]);
		if(ppara != NULL){
			FieldList* temp = pfunc -> head;
			pfunc -> head = ppara;
			ppara -> tail = temp;
			pfunc -> no ++;
		}
	}
	s -> Message.declar = pfunc;
	/* check whether conflict with existed declar */
	SNode* cur = FunDeclarHead;
	while(cur != NULL){
		if(strcmp(ch->String, cur->name) == 0){
			Declarmsg* a = cur -> Message.declar;
			if(check(a -> returnType, pfunc -> returnType) != 0){
				semantic_print_error(19, p -> lineno, cur -> name);
				free(pfunc); free(s);
				return ;
			}
			FieldList* af = a -> head;
			FieldList* bf = pfunc -> head;
			while((af != NULL) || (bf != NULL)){
				if(((bf == NULL) && (af !=NULL)) || ((af == NULL) && (bf !=NULL))){
					semantic_print_error(19, p -> lineno, cur -> name);
					free(pfunc); free(s);
					return ;
				}
				if(check(af -> type, bf -> type) != 0){
					semantic_print_error(19, p -> lineno, cur -> name);
					free(pfunc); free(s);
					return ;
				}
				af = af -> tail;
				bf = bf -> tail;
			}
			break;
		}
		cur = cur -> next;
	}
	SNode* temp = FunDeclarHead;
	FunDeclarHead = s;
	s -> next = temp;
	return ;
}
void doFunDec(Node* p, Type* pt){
	assert(p != NULL);
	Node* ch = p->child[0];
	if(stFind(ch->String) != NULL){
		/* a bit different with PDF:
		 * conflict with any other symbol */
		semantic_print_error(4, ch -> lineno, ch -> String);
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
FieldList* doParamDecInDeclar(Node* p){
	/* Hypothesis:
	 * no matter ID, type counts */
	Type* type = doSpecifier(p -> child[0]);
	type = doVarDec(p -> child[1], type);
	p = p -> child[1];
	while(p -> childno != 1){
		assert(p != NULL);
		p = p -> child[0];
	}
	p = p -> child[0];
	
	FieldList* pff = (FieldList*)malloc(sizeof(FieldList));
	strcpy(pff -> name, p -> String);
	pff -> type = type;
	return pff;
}
SNode* doParamDec(Node* p){
	/* Hypothesis:
	 * can def struct in func def */
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
		semantic_print_error(3, p -> lineno, p -> String);
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
				semantic_print_error(17, p -> lineno, p -> String);
				return NULL;
			}
			else {
				if(stnode -> visitedTag != STRUCTDEF){
					/* conflict with exist name */
					semantic_print_error(16, p -> lineno, p -> String);
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
					semantic_print_error(16, p -> lineno, p -> String);
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
	if(type == NULL)return ;
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
		semantic_print_error(3, pvar -> lineno, pvar -> String);
		return ;
	}
	else {
		s = stInitNode(pvar -> String);
		s -> visitedTag = VARIABLE;
		s -> lineno = pvar -> lineno;
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
	/* Hypothesis:
	 * for NULL means no handle, we set INT default 
	 */
	Type* finaltype = TypeNodeInt;
	if(childno == 1){
		Node* ch = p -> child[0];
		if(ch -> type == ID){
			SNode* s = stFind(ch -> String);
			if(s == NULL){
				semantic_print_error(1, ch -> lineno, ch -> String);
			}
			else{
				if(s -> visitedTag != VARIABLE){
					semantic_print_error(1, ch -> lineno, ch -> String);
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
		Node* ch0 = p -> child[0];
		Node* ch1 = p -> child[1];
		Node* ch2 = p -> child[2];
		if(ch1 -> type == DOT){
			Type* t = doExp(ch0);
			if(t -> kind != STRUCTURE){
				semantic_error(13, p -> lineno);
				finaltype = TypeNodeInt;
			}
			else {
				Type* ft = sfFind(t, ch2 -> String);
				if(ft == NULL){
					semantic_print_error(14, p -> lineno, ch2 -> String);
					finaltype = TypeNodeInt;
				}
				else {
					finaltype = ft;
				}
			}
		}
		else if(ch1 -> type == LP){
			//ID LP RP which is a function
			SNode* s = stFind(ch0 -> String);
			if(s == NULL){
				/* no def when use func 
				 * check declar list*/
				SNode* temp = FunDeclarHead;
				while(temp != NULL){
					if(strcmp(temp -> name, ch0 -> String) == 0)break;
					temp = temp -> next;
				}
				if(temp != NULL){
					if(doArgsWithDeclar(ch2, temp -> Message.declar) != 0){
						semantic_print_error(9, p -> lineno, ch0 -> String);
					}
					finaltype = temp -> Message.declar -> returnType;
				}
				else {
					semantic_print_error(2, p -> lineno, ch0 -> String);
					finaltype = TypeNodeInt;
				}
			}
			else if(s -> visitedTag != FUNC){
				semantic_print_error(11, p -> lineno, ch0 -> String);
				finaltype = TypeNodeInt;
			}
			else {
				// check fieldlist 
				Funcmsg* fm = s -> Message.func;
				if(fm -> para.no != 0){
					semantic_print_error(9, p -> lineno, ch0 -> String);
				}
				finaltype = fm -> returnType;
			}
		}
		else if(ch1 -> type == Exp){
			finaltype = doExp(ch1);
		}
		else {
			Type* t0 = doExp(ch0);
			Type* t2 = doExp(ch2);
			if(ch1 -> type == ASSIGNOP){
				if(check(t0, t2) != 0){
					semantic_error(5, p -> lineno);
				}
				finaltype = t2;
				// deal with error type 6 			
				if(ch0 -> childno == 1){
					if(ch0 -> child[0] -> type != ID){
						semantic_error(6, p -> lineno);
					}
				}
				else if(ch0 -> childno == 3){
					if(ch0 -> child[1] -> type != DOT){
						semantic_error(6, p -> lineno);
					}
				}
				else if(ch0 -> childno == 4){
					if(ch0 -> child[1] -> type != LB){
						semantic_error(6, p -> lineno);
					}
				}
			}
			else if((ch1 -> type == AND) || (ch1 -> type == OR)){
				if((!isLogic(t0)) || (!isLogic(t2))){
					semantic_error(7, p -> lineno);
				}
				finaltype = TypeNodeInt;
			}
			else { // relop + - * / 5 types
				if((!isArith(t0)) || (!isArith(t2))){
					semantic_error(7, p -> lineno);
				}
				else if(check(t0, t2) != 0){
					semantic_error(7, p -> lineno);
				}
				finaltype = t0;
			}
		}
	}
	else if(childno == 4){
		Node* ch0 = p -> child[0];
		Node* ch2 = p -> child[2];
		if(ch0 -> type == ID){
			SNode* s = stFind(ch0 -> String);
			if(s == NULL){
				/* no def when use func 
				 * check declar list*/
				SNode* temp = FunDeclarHead;
				while(temp != NULL){
					if(strcmp(temp -> name, ch0 -> String) == 0)break;
					temp = temp -> next;
				}
				if(temp != NULL){
					if(doArgsWithDeclar(ch2, temp -> Message.declar) != 0){
						semantic_print_error(9, p -> lineno, ch0 -> String);
					}
					finaltype = temp -> Message.declar -> returnType;
				}
				else {
					semantic_print_error(2, p -> lineno, ch0 -> String);
					finaltype = TypeNodeInt;
				}
			}
			else if(s -> visitedTag != FUNC){
				semantic_print_error(11, p -> lineno, ch0 -> String);
				finaltype = TypeNodeInt;
			}
			else {
				// check fieldlist 
				Funcmsg* fm = s -> Message.func;
				if(doArgs(ch2, fm) != 0){
					semantic_print_error(9, p -> lineno, ch0 -> String);
				}
				finaltype = fm -> returnType;
			}
		}
		else if(ch0 -> type == Exp){
			Type* arraytype = doExp(ch0);
			if(arraytype -> kind != ARRAY){
				semantic_error(10, p -> lineno);
				finaltype = arraytype;
			}
			else {
				finaltype = arraytype -> u.array.elem;
			}
			Type* numtype = doExp(ch2);
			if(!isLogic(numtype)){
				semantic_error(12, p -> lineno);
			}
		}
	}
	p -> exptype = finaltype;
	return finaltype;
}
int doArgsWithDeclar(Node* p, Declarmsg* fm){
	/* 1. num of args
	 * 2. type of them
	 */
	int count = 1;
	Node* temp = p;
	while(temp -> childno == 3){
		count ++;
		temp = temp -> child[2];
	}
	if(count != fm -> no){
		return -1;
	}
	FieldList* head = NULL;
	while(p -> childno == 3){
		FieldList* q = (FieldList*)malloc(sizeof(FieldList));
		q -> type = doExp(p -> child[0]);
		FieldList* tmp = head;
		head = q;
		q -> tail = tmp;
		p = p -> child[2];
	}
	FieldList* q = (FieldList*)malloc(sizeof(FieldList));
	q -> type = doExp(p -> child[0]);
	FieldList* tmp = head;
	head = q;
	q -> tail = tmp;

	FieldList* origin = fm -> head;
	while(count --){
		if(check(origin -> type, head -> type) != 0){
			return -1;
		}
	}
	return 0;
}
int doArgs(Node* p, Funcmsg* fm){
	/* 1. num of args
	 * 2. type of them
	 */
	int count = 1;
	Node* temp = p;
	while(temp -> childno == 3){
		count ++;
		temp = temp -> child[2];
	}
	if(count != fm -> para.no){
		return -1;
	}
	FieldList* head = NULL;
	while(p -> childno == 3){
		FieldList* q = (FieldList*)malloc(sizeof(FieldList));
		q -> type = doExp(p -> child[0]);
		FieldList* tmp = head;
		head = q;
		q -> tail = tmp;
		p = p -> child[2];
	}
	FieldList* q = (FieldList*)malloc(sizeof(FieldList));
	q -> type = doExp(p -> child[0]);
	FieldList* tmp = head;
	head = q;
	q -> tail = tmp;

	SNode* origin = fm -> para.head;
	while(count --){
		if(check(origin -> Message.var, head -> type) != 0){
			return -1;
		}
	}
	return 0;
}
int isArith(Type* t){
	return ((check(t, TypeNodeInt) == 0) || (check(t, TypeNodeFloat) == 0));
}
int isLogic(Type* t){
	return (check(t, TypeNodeInt) == 0);
}
int check(Type* a, Type* b){
	if(a == b)return 0;//no difference
	if(a -> kind != b -> kind)return -1;
	if(a -> kind == BASIC){
		if(a -> u.basic == b -> u.basic)return 0;
		else return -1;
	}
	else if(a -> kind == ARRAY){
		return check(a -> u.array.elem, b -> u.array.elem);
	}
	else if(a -> kind == STRUCTURE){
		FieldList* af = a -> u.structure;
		FieldList* bf = b -> u.structure;
		while((af != NULL) || (bf != NULL)){
			if(((bf == NULL) && (af !=NULL)) || ((af == NULL) && (bf !=NULL))){
				return -1;
			}
			if(check(af -> type, bf -> type) != 0){
				return -1;
			}
			af = af -> tail;
			bf = bf -> tail;
		}
		return 0;
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
		if(valtype == NULL){
			p = p -> child[1];
			continue;
		}
		Node* pdeclist = pdef -> child[1];
		while(pdeclist -> childno == 3){
			FieldList* pf = doDecInStruct(pdeclist -> child[0], valtype);
			if(sfFind(theType, pf -> name) == NULL){
				FieldList* temp = theType -> u.structure;
				theType -> u.structure = pf;
				pf -> tail = temp;
			}
			else {
				semantic_print_error(15, p -> lineno, pf -> name);
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
			semantic_print_error(15, p -> lineno, pf -> name);
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
	while(p -> type != ID){
		p = p -> child[0];
	}
	strcpy(pf -> name, p -> String);
	return pf;
}
void semantic_print_error(int no, int line, char* msg){
	printf("Error type %d at Line %d: ", no, line);
	switch(no){
		case 1:
			printf("Undefined variable \"%s\".", msg);
			break;
		case 2:
			printf("Undefined function \"%s\".", msg);
			break;
		case 3:
			printf("Redefined variable \"%s\".", msg);
			break;
		case 4:
			printf("Redefined function \"%s\".", msg);
			break;
		case 9:
			printf("Function \"%s\" is not applicable for arguments.", msg);
			break;
		case 11:
			printf("\"%s\" is not a function.", msg);
			break;
		case 14:
			printf("Non-existent field \"%s\".", msg);
			break;
		case 15:
			printf("Redefined field \"%s\".", msg);
			break;
		case 16:
			printf("Duplicated name \"%s\".", msg);
			break;
		case 17:
			printf("Undefined structure \"%s\".", msg);
			break;
		case 18:
			printf("Undefined function \"%s\".", msg);
			break;
		case 19:
			printf("Inconsistent declaration of function \"%s\".", msg);
			break;
	}
	printf("\n");
}
void semantic_error(int no, int line){
	printf("Error type %d at Line %d: ", no, line);
	switch(no){
		case 5:
			printf("Type mismatched for assignment.");
			break;
		case 6:
			printf("The left-hand side of an assignment must be a variable.");
			break;
		case 7:
			printf("Type mismatched for oprands.");
			break;
		case 8:
			printf("Type mismatched for return.");
			break;
		case 10:
			printf("This is not an array.");
			break;
		case 12:
			printf("Illegal use of array expression.");
			break;
		case 13:
			printf("Illegal use of \".\".");
			break;
		case 15:
			printf("Assignment in structure definition.");
			break;
	}
	printf("\n");
}
