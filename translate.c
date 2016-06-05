#include "intercode.h"
SNode* fnode = NULL;
int temp_var_no = 0, address_no = 0;
//TODO
InterCodes* translate(Node* p){
	/* Hypothesis:
	 * There are no global variables or function declarations.
	 * */
	if(p == NULL || p -> childno == 0)return NULL;
	int childno = p -> childno;
	Node* child[8];
	int i;
	for(i = 0; i < childno; i++){
		child[i] = p -> child[i];
	}
	switch(p -> type){
		case Program: 
			return translate(child[0]);

		case ExtDefList: 
			return codesJoin(translate(child[0]), translate(child[1]));

		case ExtDef: 
			if(childno == 3 && child[2] -> type == CompSt){
				/* function */
				SNode* stnode = stFind(child[1] -> child[0] -> name);
				assert(stnode == NULL);
				InterCodes* code1 = codesInit(FUNCTION_3, 1, stnode);
				fnode = stnode;
				InterCodes* code2 = translate(child[2]);
				InterCodes* code3 = codesInit(BLANKLINE, 0);
				return codesJoin(code1, code2);
			}
			else{
			   	return NULL;
			}
			break

		case CompSt:
			return codesJoin(translate(child[1]), translate(child[2]));

		case DefList:
			//TODO
			//
		case StmtList: 
			return codesJoin(translate(child[0]), translate(child[1]));

		case Stmt: 
			if(childno == 1)return translate(child[0]);
			else if(childno == 2)return translate_Exp(child[0], NULL);
			else if(childno == 3){
				Operand* t1 = new_temp();
				InterCodes* code1 = translate_Exp(child[1], t1);
				InterCodes* code2 = codesInit(RETURN_3, 1, t1);
				return codesJoin(code1, code2);
			}
			else if(childno == 5 && //TODO
			break;
	}
}
