#include "intercode.h"
InterCodes* translate_Args(Node* p);
InterCodes* translate_Exp(Node*, Operand*);
InterCodes* translate_Cond(Node*, Operand*, Operand*);
extern Type* TypeNodeInt;
extern Operand* OperandNodeZero;
extern Operand* OperandNodeOne;
extern Operand* OperandNodeFour;
SNode* fnode = NULL;
int temp_var_no = 0, label_address_no = 0, st_var_no = 0;
int new_var(){
	st_var_no ++ ;
	return st_var_no;
}
Operand* new_temp(){
	temp_var_no ++;
	return opInit(TEMPVAR, temp_var_no);
}
Operand* new_label(){
	label_address_no ++;
	return opInit(LABELADDRESS, label_address_no);
}
InterCodes* translate_Args(Node* p){
	assert(p != NULL);
	int childno = p -> childno;
	Node* child[4];
	int i;
	for(i = 0; i < childno; i++){
		child[i] = p -> child[i];
	}
	Operand* t1 = new_temp();
	InterCodes* code1 = translate_Exp(child[0], t1);
	//TODO 3.2.a 3.1.b
	InterCodes* code2 = codesInit(ARG, 1, t1);
	if(childno == 1){
		return codesJoin(code1, code2);
	}
	else {
		InterCodes* code3 = translate_Args(child[2]);
		code1 = codesJoin(code1, code3);
		return codesJoin(code1, code2);
	}
}
InterCodes* translate_Exp(Node* p, Operand* place){
	//printf("enter into translate_Exp:\n");
	assert(p != NULL);
	int childno = p -> childno;
	Node* child[4];
	int i;
	for(i = 0; i < childno; i++){
		child[i] = p -> child[i];
	}
	assert((place != NULL) || (childno > 2 && child[1] -> type == ASSIGNOP) || (childno > 2 && child[0] -> type == ID));
	/* Hypothesis:
	 * No FLOAT
	 * */
	/* LP Exp RP */
	if(child[0] -> type == LP){
		Operand* t1 = new_temp();
		InterCodes* code1 = translate_Exp(child[1], t1);
		InterCodes* code2 = codesInit(ASSIGN_ORIGIN, 2, place, t1);
		return codesJoin(code1, code2);
	}
	/* INT */
	else if(child[0] -> type == INT){
		Operand* value = opInit(CONSTANT, child[0] -> value.Int);
		return codesInit(ASSIGN_ORIGIN, 2, place, value);
	}
	/* ID */
	else if(childno == 1 && child[0] -> type == ID){
		SNode* vnode = stFind(child[0] -> String);
		//printf("%s\t%d\n", vnode -> name, vnode -> op_var_no);
		//printf("a\n");
		Operand* v1 = opInit(VARIABLE_3, vnode -> op_var_no);
		return codesInit(ASSIGN_ORIGIN, 2, place, v1);
	}
	/* FLOAT */
	else if(child[0] -> type == FLOAT){
		printf("Cannot translate: Code contains variables or parameters of Float.\n");
	}
	/* MINUS Exp */
	else if(child[0] -> type == MINUS){
		Operand* t1 = new_temp();
		InterCodes* code1 = translate_Exp(child[1], t1);
		InterCodes* code2 = codesInit(SUB_3, 3, place, OperandNodeZero, t1);
		return codesJoin(code1, code2);
	}
	/* NOT Exp */
	/* Exp RELOP Exp */
	/* Exp AND Exp */
	/* Exp OR */
	else if(child[0] -> type == NOT || (childno == 3 && (child[1] -> type == RELOP || child[1] -> type == AND || child[1] -> type == OR))){
		Operand* label1 = new_label();
		Operand* label2 = new_label();
		InterCodes* code0 = codesInit(ASSIGN_ORIGIN, 2, place, OperandNodeZero);
		InterCodes* code1 = translate_Cond(p, label1, label2);
		InterCodes* code2 = codesJoin(codesInit(LABEL, 1, label1), codesInit(ASSIGN_ORIGIN, 2, place, OperandNodeOne));
		code0 = codesJoin(code0 ,code1);
		code2 = codesJoin(code2, codesInit(LABEL, 1, label2));
		return codesJoin(code0 ,code2);
	}
	/* Exp PLUS Exp */
	/* Exp MINUS Exp */
	/* Exp STAR Exp */
	/* Exp DIV Exp */
	else if(childno == 3 && (child[1] -> type == PLUS || child[1] -> type == MINUS || child[1] -> type == STAR || child[1] -> type == DIV)){
		Operand* t1 = new_temp();
		Operand* t2 = new_temp();
		InterCodes* code1 = translate_Exp(child[0], t1);
		InterCodes* code2 = translate_Exp(child[2], t2);
		InterCodes* code3 = NULL;
		if(child[1] -> type == PLUS){
			code3 = codesInit(ADD_3, 3, place, t1, t2);
		}
		else if(child[1] -> type == MINUS){
			code3 = codesInit(SUB_3, 3, place, t1, t2);
		}
		else if(child[1] -> type == STAR){
			code3 = codesInit(MUL_3, 3, place, t1, t2);
		}
		else if(child[1] -> type == DIV){
			code3 = codesInit(DIV_3, 3, place, t1, t2);
		}
		code1 = codesJoin(code1, code2);
		return codesJoin(code1, code3);
	}
	else if(childno == 3 && child[1] -> type == ASSIGNOP){
		Node* ch = child[0] -> child[0];
		int leftchildno = child[0] -> childno;
		Operand* right = new_temp();
		InterCodes*	calc_right = translate_Exp(child[2], right);
		Operand* left = NULL;
		InterCodes* assign = NULL;
		if(ch -> type == ID){
			SNode* vnode = stFind(ch -> String);
			left = opInit(VARIABLE_3, vnode -> op_var_no);
			assign = codesInit(ASSIGN_ORIGIN, 2, left, right);
		}
		else if(leftchildno == 4 && ch -> type == Exp){
			Node* chch = ch -> child[0];

			assert(chch -> type == ID);//TODO 3.2.b

			SNode* stnode = stFind(chch -> String);
			left = opInit(VARIABLE_3, stnode -> op_var_no);
			Type* vtype = stnode -> Message.var;

			assert(vtype -> u.array.elem == TypeNodeInt);

			Operand* t1 = new_temp();
			Operand* t2 = new_temp();

			InterCodes* code[5];
			code[0] = translate_Exp(child[0] -> child[2], t1);
			code[1] = codesInit(MUL_3, 3, t1, t1, OperandNodeFour);
			code[2] = codesInit(ASSIGN_ADDRESS_TO, 2, t2, left);
			code[3] = codesInit(ADD_3, 3, t2, t2, t1);
			code[4] = codesInit(ASSIGN_TO_ADDRESS, 2, t2, right);
			int i;
			for(i = 0; i < 5; i++){
				assign = codesJoin(assign, code[i]);
			}
		}
		else{
			//TODO 3.1.a 3.2.b
			//now one-dim var
			
			printf("error\n");
			assert(0);
		}
		if(place != NULL)assign = codesJoin(assign, codesInit(ASSIGN_ORIGIN, 2, place, right));
		return codesJoin(calc_right, assign);
	}
	/* ID LP Args RP */
	else if(childno == 4 && child[0] -> type == ID){
		SNode* stnode = stFind(child[0] -> String);
		if(strcmp(stnode -> name, "write") == 0){
			Operand* t1 = new_temp();
			InterCodes* code1 = translate_Exp(child[2] -> child[0], t1);
			return codesJoin(code1, codesInit(WRITE, 1 ,t1));
		}
		else {
			InterCodes* code1 = translate_Args(child[2]);
			return codesJoin(code1, codesInit(CALLFUNC, 2, place, stnode));
		}
	}

	/* ID LP RP */
	else if(childno == 3 && child[0] -> type == ID){
		SNode* fnode = stFind(child[0] -> String);
		if(strcmp(fnode -> name, "read") == 0){
			return codesInit(READ, 1, place);
		}
		else {
			return codesInit(CALLFUNC, 2, place, fnode);
		}
	}
	/* Exp LB Exp RB */
	else if(childno == 4 && child[1] -> type == LB){
		Node* ch = child[0] -> child[0];
		if(ch -> type == ID){
			SNode* stnode = stFind(ch -> String);
			int no = stnode -> op_var_no;
			Type* vtype = stnode -> Message.var;

			assert(vtype -> u.array.elem == TypeNodeInt);

			Operand* t1 = new_temp();
			Operand* t2 = new_temp();
			Operand* v1 = opInit(VARIABLE_3, no);
			InterCodes* code[5];
			code[0] = translate_Exp(child[2], t1);
			code[1] = codesInit(ASSIGN_ADDRESS_TO, 2, t2, v1);
			code[2] = codesInit(MUL_3, 3, t1, t1, OperandNodeFour);
			code[3] = codesInit(ADD_3, 3, t2, t2, t1);
			code[4] = codesInit(ASSIGN_VALUE_FROM, 2, place, t2);
			int i;
			for(i = 1; i < 5; i++){
				code[0] = codesJoin(code[0], code[i]);
			}
			return code[0];
		}
		else {
			//TODO 3.2
			printf("Cannot translate: Code contains variables of multi-dimensional array type.\n");
			assert(0);
		}
	}
	else {
	/* Exp DOT ID */
		printf("chlidno = %d\n", childno);
		//TODO 3.1
		printf("error\n");
		assert(0);
	}
	
}
InterCodes* translate_Cond(Node* p, Operand* label_true, Operand* label_false){
	//printf("enter into translate_Cond:\n");
	/* p's type is Exp */
	assert(p != NULL);
	int childno = p -> childno;
	Node* child[4];
	int i;
	for(i = 0; i < childno; i++){
		child[i] = p -> child[i];
	}
	/* Exp RELOP Exp */
	if(child[1] -> type == RELOP){
		Operand* t1 = new_temp();
		Operand* t2 = new_temp();
		InterCodes* code1 = translate_Exp(child[0], t1);
		InterCodes* code2 = translate_Exp(child[2], t2);
		InterCodes* code3 = codesInit(IFGOTO, 4, t1, child[1] -> value.Int, t2, label_true);
		code1 = codesJoin(code1, code2);
		code3 = codesJoin(code3, codesInit(GOTO, 1, label_false));
		return codesJoin(code1, code3);
	}
	/* NOT Exp */
	else if(child[0] -> type == NOT){
		return translate_Cond(child[1], label_false, label_true);
	}
	/* Exp AND Exp */
	else if(child[1] -> type == AND){
		Operand* label1 = new_label();
		InterCodes* code1 = translate_Cond(child[0], label1, label_false);
		InterCodes* code2 = translate_Cond(child[2], label_true, label_false);
		code1 = codesJoin(code1, codesInit(LABEL, 1, label1));
		return codesJoin(code1, code2);
	}
	/* Exp OR Exp */
	else if(child[1] -> type == OR){
		Operand* label1 = new_label();
		InterCodes* code1 = translate_Cond(child[0], label_true, label1);
		InterCodes* code2 = translate_Cond(child[2], label_true, label_false);
		code1 = codesJoin(code1, codesInit(LABEL, 1, label1));
		return codesJoin(code1, code2);
	}
	else {
		Operand* t1 = new_temp();
		InterCodes* code1 = translate_Exp(p, t1);
		InterCodes* code2 = codesInit(IFGOTO, 4, t1, NE, OperandNodeZero, label_true);
		code2 = codesJoin(code2, codesInit(GOTO, 1, label_false));
		return codesJoin(code1, code2);
	}
}
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
	//printf("%s\n", p -> name);
	switch(p -> type){
		case Program: 
			return translate(child[0]);

		case ExtDefList: 
			if(childno == 2){
				InterCodes* code1 = translate(child[0]);
				InterCodes* code2 = translate(child[1]);
				return codesJoin(code1, code2);
			}
			else return NULL;

		case ExtDef: 
			if(childno == 3 && child[2] -> type == CompSt){
				/* function */
				SNode* stnode = stFind(child[1] -> child[0] -> String);
				assert(stnode != NULL);
				InterCodes* code1 = codesInit(FUNCTION_3, 1, stnode);
				InterCodes* code2 = translate(child[1]);
				fnode = stnode;
				InterCodes* code3 = translate(child[2]);
				InterCodes* code4 = codesInit(BLANKLINE, 0);
				code1 = codesJoin(code1, code2);
				code3 = codesJoin(code3, code4);
				return codesJoin(code1, code3);
			}
			else{
			   	return NULL;
			}
			break;

		case FunDec:
			/* ID LP RP */
			if(childno == 3)return NULL;
			/* ID LP VarList RP */
			else{
				SNode* stnode = stFind(child[0] -> String);
				Funcmsg* pfunc = stnode -> Message.func;
				int no = pfunc -> para.no;
				assert(no != 0);
				SNode* paranode = pfunc -> para.head;
				InterCodes* code = NULL;
				while(no--){
					assert(paranode != NULL);
					paranode -> op_var_no = new_var();
					//printf("%d\n", paranode -> op_var_no);
					Operand* v = opInit(VARIABLE_3, paranode -> op_var_no);
					code = codesJoin(codesInit(PARAM, 1, v), code);
					paranode = paranode -> fnext;
				}
				return code;
			}
			break;

		case CompSt:
			if(childno == 4){
				InterCodes* code1 = translate(child[1]);
				InterCodes* code2 = translate(child[2]);
				return codesJoin(code1, code2);
			}
			else return NULL;
			break;

		case DefList:
			if(childno == 2){
				InterCodes* code1 = translate(child[0]);
				InterCodes* code2 = translate(child[1]);
				return codesJoin(code1, code2);
			}
			else return NULL;
		case Def:
			return translate(child[1]);
		case DecList: {
			InterCodes* code1 = translate(child[0]);
			if(childno == 1)return code1;
			else if(childno == 3){
				return codesJoin(code1, translate(child[2]));
			}
		}
		case Dec:{
			/* common part */
			Node* varp = child[0];
			while(varp != NULL && varp -> type != ID){
				varp = varp -> child[0];
			}
			assert(varp != NULL);
			SNode* vnode = stFind(varp -> String);
			assert(vnode != NULL);
			vnode -> op_var_no = new_var();
			Type* vartype = vnode -> Message.var;
			if(vartype -> kind == ARRAY){
				//TODO 3.2.b
				//now: use size = 4;
				int size = 4 * vartype -> u.array.size;
				Operand* v1 = opInit(VARIABLE_3, vnode -> op_var_no);
				InterCodes* code1 = codesInit(DEC, 2, v1, size);
				return code1;
			}
			else if(vartype -> kind == STRUCTURE){
				//TODO 3.1
				printf("error\n");
				assert(0);
			}

			/* if basic type has initialization */
			else if(vartype -> kind == BASIC && childno == 3){
				//TODO: without consideration of structure or array
				Operand* t1 = new_temp();
				InterCodes* code1 = translate_Exp(child[2], t1);
				//printf("%s\t%d\n", vnode -> name, vnode -> op_var_no);
				//printf("c\n");
				Operand* v1 = opInit(VARIABLE_3, vnode -> op_var_no);
				InterCodes* code2 = codesInit(ASSIGN_ORIGIN, 2, v1, t1);
				return codesJoin(code1, code2);
			}
			return NULL;
		}

		case StmtList: 
			if(childno == 2){
				InterCodes* code1 = translate(child[0]);
				InterCodes* code2 = translate(child[1]);
				return codesJoin(code1, code2);
			}
			else return NULL;

		case Stmt:
			/* CompSt */
			if(childno == 1)return translate(child[0]);
			/* Exp SEMI */
			else if(childno == 2)return translate_Exp(child[0], NULL);
			/* RETURN Exp SEMI */
			else if(childno == 3){
				Operand* t1 = new_temp();
				InterCodes* code1 = translate_Exp(child[1], t1);
				InterCodes* code2 = codesInit(RETURN_3, 1, t1);
				return codesJoin(code1, code2);
			}
			/* IF LP Exp RP Stmt ELSE Stmt */
			else if(childno == 7){
				Operand* label1 = new_label();
				Operand* label2 = new_label();
				Operand* label3 = new_label();
				InterCodes* code1 = translate_Cond(child[2], label1, label2);
				InterCodes* code2 = translate(child[4]);
				InterCodes* code3 = translate(child[6]);
				code1 = codesJoin(code1, codesInit(LABEL, 1, label1));
				code2 = codesJoin(code2, codesInit(GOTO, 1, label3));
				code2 = codesJoin(code2, codesInit(LABEL, 1, label2));
				code3 = codesJoin(code3, codesInit(LABEL, 1, label3));
				code1 = codesJoin(code1, code2);
				return codesJoin(code1, code3);
			}
			/* IF LP Exp RP Stmt */
			else if(child[0] -> type == IF){
				Operand* label1 = new_label();
				Operand* label2 = new_label();
				InterCodes* code1 = translate_Cond(child[2], label1, label2);
				InterCodes* code2 = translate(child[4]);
				code1 = codesJoin(code1, codesInit(LABEL, 1, label1));
				code2 = codesJoin(code2, codesInit(LABEL, 1, label2));
				return codesJoin(code1, code2);
			}
			/* WHILE LP Exp RP Stmt */
			else if(child[0] -> type == WHILE){
				Operand* label1 = new_label();
				Operand* label2 = new_label();
				Operand* label3 = new_label();
				InterCodes* code[6];
				code[0] = codesInit(LABEL, 1, label1);
				code[1] = translate_Cond(child[2], label2, label3);
				code[2] = codesInit(LABEL, 1, label2);
				code[3] = translate(child[4]);
				code[4] = codesInit(GOTO, 1, label1);
				code[5] = codesInit(LABEL, 1, label3);
				int i;
				for(i = 1; i < 6; i++){
					code[0] = codesJoin(code[0], code[i]);
				}
				return code[0];
			}
			break;
	}
}
