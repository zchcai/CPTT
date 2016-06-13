Type* doExp(Node*);
#include "intercode.h"
int calcTypeSize(Type* vartype);
InterCodes* translate_Args(Node* p);
InterCodes* translate_Exp(Node*, Operand*);
InterCodes* translate_Cond(Node*, Operand*, Operand*);
extern Type* TypeNodeInt;
extern Type* TypeNodeFloat;
extern Operand* OperandNodeZero;
extern Operand* OperandNodeOne;
extern Operand* OperandNodeFour;
extern int translate_error;
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
int calcTypeSize(Type* vartype){
	int  size = 0;
	if(vartype -> kind == BASIC){
		size = 4;
	}
	else if(vartype -> kind == ARRAY){
		size = vartype -> u.array.size * calcTypeSize(vartype -> u.array.elem);
	}
	else if(vartype -> kind == STRUCTURE){
		FieldList* field = vartype -> u.structure;
		while(field != NULL){
			size = size + calcTypeSize(field -> type);
			field = field -> tail;
		}
	}
	return size;
}
InterCodes* translate_Args(Node* p){
	/* The first child must be Exp */
	assert(p != NULL);
	int childno = p -> childno;
	Node* child[4];
	int i;
	for(i = 0; i < childno; i++){
		child[i] = p -> child[i];
	}
	InterCodes* code1 = NULL;
	InterCodes* code2 = NULL;

	Operand* var = new_temp();
	code1 = translate_Exp(child[0], var);
	code2 = codesInit(ARG, 1, var);

	if(childno == 1){
		return codesJoin(code1, code2);
	}
	else {
	/* Exp COMMA Args */
		InterCodes* code3 = translate_Args(child[2]);
		code1 = codesJoin(code1, code3);
		return codesJoin(code1, code2);
	}
}
InterCodes* translate_Exp(Node* p, Operand* place){
	/* if Exp is BASIC, return value;
	 * else if ARRAY or STRUCTURE, return ADDRESS. */
	assert(p != NULL);
	int childno = p -> childno;
	Node* child[4];
	int i;
	for(i = 0; i < childno; i++){
		child[i] = p -> child[i];
	}
	/* LP Exp RP */
	if(child[0] -> type == LP){
		Operand* t1 = new_temp();
		InterCodes* code1 = translate_Exp(child[1], t1);
		InterCodes* code2 = NULL;
		if(place != NULL)code2 = codesInit(ASSIGN_ORIGIN, 2, place, t1);
		return codesJoin(code1, code2);
	}
	/* INT */
	else if(child[0] -> type == INT){
		if(place == NULL)return NULL;
		Operand* v = opInit(CONSTANT, child[0] -> value.Int);
		return codesInit(ASSIGN_ORIGIN, 2, place, v);
	}
	/* ID */
	else if(childno == 1 && child[0] -> type == ID){
		if(place == NULL)return NULL;
		SNode* vnode = stFind(child[0] -> String);
		assert(vnode -> visitedTag == VARIABLE);
		Type* vartype = vnode -> Message.var;
		Operand* v = opInit(VARIABLE_3, vnode -> op_var_no);
		if(vartype -> kind == BASIC){
			return codesInit(ASSIGN_ORIGIN, 2, place, v);
		}
		else {
			if(vnode -> isParam != 0){
				return codesInit(ASSIGN_ORIGIN, 2, place, v);
			}
			else return codesInit(ASSIGN_ADDRESS_TO, 2, place, v);
		}
	}
	/* FLOAT */
	else if(child[0] -> type == FLOAT){
		if(place == NULL)return NULL;
	/* Hypothesis:
	 * No FLOAT
	 * */
		translate_error ++;
		printf("Cannot translate: Code contains variables or parameters of Float.\n");
	}
	/* MINUS Exp */
	else if(child[0] -> type == MINUS){
		Operand* t1 = new_temp();
		InterCodes* code1 = translate_Exp(child[1], t1);
		InterCodes* code2 = NULL;
		if(place != NULL)code2 = codesInit(SUB_3, 3, place, OperandNodeZero, t1);
		return codesJoin(code1, code2);
	}
	/* NOT Exp */
	/* Exp RELOP Exp */
	/* Exp AND Exp */
	/* Exp OR Exp */
	else if(child[0] -> type == NOT || (childno == 3 && (child[1] -> type == RELOP || child[1] -> type == AND || child[1] -> type == OR))){
		assert(place != NULL);
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
		if(place != NULL){
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
		}
		code1 = codesJoin(code1, code2);
		return codesJoin(code1, code3);
	}
	/* Exp ASSIGNOP Exp */
	else if(childno == 3 && child[1] -> type == ASSIGNOP){
		Type* lefttype = doExp(child[0]);
		/* Exp DOT ID 
		 * ID 
		 * Exp LB Exp RB */
		int leftchildno = child[0] -> childno;
		Operand* right = new_temp();
		InterCodes*	calc_right = translate_Exp(child[2], right);
		Operand* left = NULL; // value or address
		InterCodes* assign = NULL;
		if(leftchildno == 1){ // ID 
			SNode* vnode = stFind(child[0] -> child[0] -> String);
			left = opInit(VARIABLE_3, vnode -> op_var_no);
			assign = codesInit(ASSIGN_ORIGIN, 2, left, right);
		}
		else if(leftchildno == 4){ // Exp LB Exp RB
			InterCodes* code[5];
			Operand* num = new_temp();
			code[0] = translate_Exp(child[0] -> child[2], num);
			Operand* base = new_temp();
			code[1] = translate_Exp(child[0] -> child[0], base);
			int typesize = calcTypeSize(lefttype);
			Operand* size = opInit(CONSTANT, typesize);

			code[2] = codesInit(MUL_3, 3, num, num, size);
			left = new_temp();
			code[3] = codesInit(ADD_3, 3, left, base, num);
			code[4] = codesInit(ASSIGN_TO_ADDRESS, 2, left, right);
			int i;
			for(i = 0; i < 5; i++){
				assign = codesJoin(assign, code[i]);
			}
		}
		else if(leftchildno == 3){ // Exp DOT ID
			InterCodes* code[3];
			Operand* base = new_temp();
			code[0] = translate_Exp(child[0] -> child[0], base);
			Type* stype = doExp(child[0] -> child[0]);
			int size = calcTypeSize(stype);
			FieldList* f = stype -> u.structure;
			while(f != NULL){
				size -= calcTypeSize(f -> type);
				if(strcmp(child[0] -> child[2] -> String, f -> name) == 0)break;
				f = f -> tail;
			}
			Operand* rel = opInit(CONSTANT, size);
			left = new_temp();
			code[1] = codesInit(ADD_3, 3, left, base, rel);
			code[2] = codesInit(ASSIGN_TO_ADDRESS, 2, left, right);
			int i;
			for(i = 0; i < 3; i++){
				assign = codesJoin(assign, code[i]);
			}
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
			//TODO
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
		InterCodes* code[6];
		Type* ptype = doExp(p);

		Operand* base = new_temp();
		code[0] = translate_Exp(child[0], base);
		Operand* num = new_temp();
		code[1] = translate_Exp(child[2], num);

		int typesize = calcTypeSize(ptype);
		Operand* size = opInit(CONSTANT, typesize);
		code[2] = codesInit(MUL_3, 3, num, size, num);

		code[3] = codesInit(ADD_3, 3, base, base, num);
		Operand* v = new_temp();
		if(ptype -> kind != BASIC){
			code[4] = codesInit(ASSIGN_ORIGIN, 2, v, base);
		}
		else {
			code[4] = codesInit(ASSIGN_VALUE_FROM, 2, v, base);
		}
		code[5] = NULL;
		if(place != NULL){
			code[5] = codesInit(ASSIGN_ORIGIN, 2, place, v);
		}
		int i;
		for(i = 1; i < 6; i++){
			code[0] = codesJoin(code[0], code[i]);
		}
		return code[0];
	}
	/* Exp DOT ID */
	else if(childno == 3 && child[1] -> type == DOT){
		Type* stype = doExp(child[0]);
		assert(stype -> kind == STRUCTURE);
		Operand* v = new_temp();
		Operand* base = new_temp();
		InterCodes* code[4];
		code[0] = translate_Exp(child[0], base);
		/* base must be an adddress */
		int size = calcTypeSize(stype);
		FieldList* f = stype -> u.structure;
		while(f != NULL){
			size -= calcTypeSize(f -> type);
			if(strcmp(child[2] -> String, f -> name) == 0){
				Operand* rel = opInit(CONSTANT, size);
				code[1] = codesInit(ADD_3, 3, base, base, rel);
				if(f -> type -> kind != BASIC){
					code[2] = codesInit(ASSIGN_ORIGIN, 2, v, base);
				}
				else {
					code[2] = codesInit(ASSIGN_VALUE_FROM, 2, v, base);
				}
				break;
			}
			f = f -> tail;
		}
		code[3] = NULL;
		if(place != NULL){
			code[3] = codesInit(ASSIGN_ORIGIN, 2, place, v);
		}
		int i;
		for(i = 1; i < 4; i++){
			code[0] = codesJoin(code[0], code[i]);
		}
		return code[0];
	}
}
InterCodes* translate_Cond(Node* p, Operand* label_true, Operand* label_false){
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
	switch(p -> type){
		case Program: 
			/* ExtDefList */
			return translate(child[0]);

		case ExtDefList: 
			if(childno == 2){
			/* ExtDef ExtDefList */
				InterCodes* code1 = translate(child[0]);
				InterCodes* code2 = translate(child[1]);
				return codesJoin(code1, code2);
			}
			/*  */
			else return NULL;

		case ExtDef: 
			if(childno == 3 && child[2] -> type == CompSt){
			/* Specifier FunDec CompSt */
				SNode* stnode = stFind(child[1] -> child[0] -> String);
				assert(stnode != NULL);
				InterCodes* code1 = codesInit(FUNCTION_3, 1, stnode);
				InterCodes* code2 = translate(child[1]);
				/* set currently handled function */
				fnode = stnode;

				InterCodes* code3 = translate(child[2]);
				InterCodes* code4 = codesInit(BLANKLINE, 0);
				code1 = codesJoin(code1, code2);
				code3 = codesJoin(code3, code4);
				return codesJoin(code1, code3);
			}
			else if(childno == 2){
			/* Specifier SEMI */
				return NULL;
			}
			else{
			/* Specifier ExtDecList SEMI 
			 * Because of the Hypothesis 4:
			 * No global variants */
			   	return NULL;
			}
			break;

		case FunDec:
			if(childno == 3){ /* ID LP RP */
				return NULL;
			}
			else { /* ID LP VarList RP */
				SNode* stnode = stFind(child[0] -> String);
				Funcmsg* pfunc = stnode -> Message.func;
				int no = pfunc -> para.no;
				assert(no != 0);
				SNode* paranode = pfunc -> para.head;
				InterCodes* code = NULL;
				while(no--){
					assert(paranode != NULL);
					/* set a new no for each parameter */
					paranode -> op_var_no = new_var();

					/* Generate 'PARAM x's */
					Operand* v = opInit(VARIABLE_3, paranode -> op_var_no);

					paranode -> isParam = 1;
					/* v represents the value when parameter is BASIC,
					 * and the address when ARRAY or STRUCTURE */
					code = codesJoin(codesInit(PARAM, 1, v), code);
					paranode = paranode -> fnext;
				}
				return code;
			}

		case CompSt: {
			/* LC DefList StmtList RC */
				InterCodes* code1 = translate(child[1]);
				InterCodes* code2 = translate(child[2]);
				return codesJoin(code1, code2);
			}

		case DefList:
			if(childno == 2){
			/* Def DefList */
				InterCodes* code1 = translate(child[0]);
				InterCodes* code2 = translate(child[1]);
				return codesJoin(code1, code2);
			}
			else return NULL;

		case Def:
			/* Specifier DecList SEMI */
			return translate(child[1]);

		case DecList: {
			InterCodes* code1 = translate(child[0]);
			if(childno == 1){
			/* Dec */
				return code1;
			}
			else if(childno == 3){
			/* Dec COMMA DecList */
				return codesJoin(code1, translate(child[2]));
			}
		}

		case Dec:{
			/* 1. Find id
			 * 2. New a place for it if ARRAY or STRUCTURE
			 * 3. (Optional)Assign value for BASIC
			 * Hypothesis: 
			 * no direct assignment for non-basic variants */
			Node* varp = child[0];
			while(varp != NULL && varp -> type != ID){
				varp = varp -> child[0];
			}
			assert(varp != NULL);
			SNode* vnode = stFind(varp -> String);
			assert(vnode != NULL);

			/* set a new no for each parameter */
			vnode -> op_var_no = new_var();

			Type* vartype = vnode -> Message.var;
			if(vartype -> kind == ARRAY || vartype -> kind == STRUCTURE){
				/* apply for memory space */
				int size = calcTypeSize(vartype);
				Operand* v = opInit(VARIABLE_3, vnode -> op_var_no);
				InterCodes* code = codesInit(DEC, 2, v, size);
				return code;
			}

			/* if basic type has initialization */
			else if(vartype -> kind == BASIC && childno == 3){
				/* VarDec ASSIGNOP Exp */
				Operand* t1 = new_temp();
				Type* type = doExp(child[2]);
				InterCodes* code1 = translate_Exp(child[2], t1);
				Operand* v1 = opInit(VARIABLE_3, vnode -> op_var_no);
				InterCodes* code2 = NULL;
				if(type -> kind == BASIC){
					code2 = codesInit(ASSIGN_ORIGIN, 2, v1, t1);
				}
				else {
					assert(0);
					//code2 = codesInit(ASSIGN_VALUE_FROM, 2, v1, t1);
					//TODO
				}
				return codesJoin(code1, code2);
			}
		}

		case StmtList: 
			if(childno == 2){
			/* Stmt StmtList */
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
			else if(childno == 3){
			/* RETURN Exp SEMI 
			 * Hypothesis 5:
			 * Only return BASIC type */
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
