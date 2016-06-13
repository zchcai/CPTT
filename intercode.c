#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include "translate.c"
extern FILE* ir;
extern int translate_error;
extern Node* Head;
extern SNode* SHead;
extern Type* TypeNodeInt;
Operand OperandZero= {.kind = CONSTANT, .u.var_int = 0};
Operand OperandOne= {.kind = CONSTANT, .u.var_int = 1};
Operand OperandFour= {.kind = CONSTANT, .u.var_int = 4};
Operand* OperandNodeFour = &OperandFour;
Operand* OperandNodeZero = &OperandZero;
Operand* OperandNodeOne = &OperandOne;
void print_intercodes(InterCodes*);
void print_intercode(InterCode*);
void print_operand(Operand*);
InterCodes* codesJoin(InterCodes*, InterCodes*);
InterCodes* codesInit(int k, int n, ...);
Operand* opInit(int, int);
void test_intercodes_data_structure();
void test_intercodes_data_structure(){
	/* for data structure testing */
	Operand* op1 = opInit(LABELADDRESS, 1);
	Operand* op2 = opInit(VARIABLE_3, 1);
	Operand* op4 = opInit(VARIABLE_3, 2);
	Operand* op3 = opInit(CONSTANT, 6);
	SNode* stnode = stInitNode("main");
	InterCodes* code0 = codesInit(FUNCTION_3, 1, stnode);
	InterCodes* code1 = codesInit(LABEL, 1, op1);
	InterCodes* code2 = codesInit(BLANKLINE, 0);
	InterCodes* code3 = codesInit(RETURN_3, 1, op2);
	InterCodes* code4 = codesInit(IFGOTO, 4, op2, GT, op3, op1);
	InterCodes* code5 = codesInit(DEC, 2, op4, 80);
	code0 = codesJoin(code0, code1);
	code2 = codesJoin(code2, code3);
	code4 = codesJoin(code4, code5);
	print_intercodes(codesJoin(codesJoin(code0, code2), code4));
}
void intermediate_code_generation(){
	//test_intercodes_data_structure();
	//return ;
	Node* root = Head;
	InterCodes* code = translate(root);
	if(translate_error == 0)print_intercodes(code);
}
void add_read_write(){
	SNode* pr = stInitNode("read");
	SNode* pw = stInitNode("write");
	pr -> visitedTag = FUNC;
	pw -> visitedTag = FUNC;
	stInsert(pr);
	stInsert(pw);
	Funcmsg* pfr = (Funcmsg*)malloc(sizeof(Funcmsg));
	Funcmsg* pfw = (Funcmsg*)malloc(sizeof(Funcmsg));
	pfr -> returnType = TypeNodeInt;
	pfw -> returnType = TypeNodeInt;

	pfw -> para.no = 0;
	pfr -> para.head = NULL;

	SNode* SNode4writepara = stInitNode("0000");
	SNode4writepara -> visitedTag = VARIABLE;
	SNode4writepara -> Message.var = TypeNodeInt;
	pfw -> para.head = SNode4writepara;
	pfw -> para.no = 1;

	pr -> Message.func = pfr;
	pw -> Message.func = pfw;
	return ;
}
void print_intercodes(InterCodes* head){
	if(head == NULL)return ;
	InterCodes* p = head;
	print_intercode(p -> code);
	p = p -> next;
	while(p != head && p -> code != NULL){
		print_intercode(p -> code);
		p = p -> next;
	}
}
void print_intercode(InterCode* p){
	assert(p != NULL);
	int k = p -> kind;
	/* LABEL x : */
	if(k == LABEL){
		fprintf(ir,"LABEL ");
		print_operand(p -> u.one.op);
		fprintf(ir," :");
	}
	/* FUNCTION f : */
	else if(k == FUNCTION_3){
		fprintf(ir,"FUNCTION %s :", p -> u.func.stnode -> name);
	}
	/* x := y */
	else if(k == ASSIGN_ORIGIN){
		print_operand(p -> u.assign.left);
		fprintf(ir," := ");
		print_operand(p -> u.assign.right);
	}
	/* x := y ? z */
	else if(k == ADD_3 || k == SUB_3 || k == MUL_3 || k == DIV_3){
		print_operand(p -> u.binop.result);
		fprintf(ir," := ");
		print_operand(p -> u.binop.op1);
		if(k == ADD_3){
			fprintf(ir," + ");
		}
		else if(k == SUB_3){
			fprintf(ir," - ");
		}
		else if(k == MUL_3){
			fprintf(ir," * ");
		}
		else if(k == DIV_3){
			fprintf(ir," / ");
		}
		print_operand(p -> u.binop.op2);
	}
	/* x := &y */
	else if(k == ASSIGN_ADDRESS_TO){
		print_operand(p -> u.assign.left);
		fprintf(ir," := ");
		fprintf(ir,"&");
		print_operand(p -> u.assign.right);
	}
	/* x := *y */
	else if(k == ASSIGN_VALUE_FROM){
		print_operand(p -> u.assign.left);
		fprintf(ir," := ");
		fprintf(ir,"*");
		print_operand(p -> u.assign.right);
	}
	/* *x := y */
	else if(k == ASSIGN_TO_ADDRESS){
		fprintf(ir,"*");
		print_operand(p -> u.assign.left);
		fprintf(ir," := ");
		print_operand(p -> u.assign.right);
	}
	/* GOTO x */
	else if(k == GOTO){
		fprintf(ir,"GOTO ");
		print_operand(p -> u.one.op);
	}
	/* IF x [relop] y GOTO z */
	else if(k == IFGOTO){
		fprintf(ir,"IF ");
		print_operand(p -> u.ifgoto.rel1);
		char* s = (char*)malloc(4 * sizeof(char));
		memset(s, 0, sizeof(s));
		switch(p -> u.ifgoto.reltype){
			case GT: strcpy(s, ">");break;
			case LT: strcpy(s, "<");break;
			case GE: strcpy(s, ">=");break;
			case LE: strcpy(s, "<=");break;
			case EQ: strcpy(s, "==");break;
			case NE : strcpy(s, "!=");break;
		}
		fprintf(ir," %s ", s);
		print_operand(p -> u.ifgoto.rel2);
		fprintf(ir," GOTO ");
		print_operand(p -> u.ifgoto.go);
	}
	/* RETURN x */
	else if(k == RETURN_3){
		fprintf(ir,"RETURN ");
		print_operand(p -> u.one.op);
	}
	/* DEC x [size] */
	else if(k == DEC){
		fprintf(ir,"DEC ");
		print_operand(p -> u.dec.op);
		fprintf(ir," %d", p -> u.dec.size);
	}
	/* ARG x */
	else if(k == ARG){
		fprintf(ir,"ARG ");
		print_operand(p -> u.one.op);
	}
	/* x := CALL f */
	else if(k == CALLFUNC){
		print_operand(p -> u.call.op);
		fprintf(ir," := CALL %s", p -> u.call.stnode -> name);
	}
	/* PARAM x */
	else if(k == PARAM){
		fprintf(ir,"PARAM ");
		print_operand(p -> u.one.op);
	}
	/* READ x */
	else if(k == READ){
		fprintf(ir,"READ ");
		print_operand(p -> u.one.op);
	}
	/* WRITE x */
	else if(k == WRITE){
		fprintf(ir,"WRITE ");
		print_operand(p -> u.one.op);
	}
	/* [\n] */
	else if(k == BLANKLINE){
		/* nothing */
	}
	fprintf(ir,"\n");
}	
void print_operand(Operand* p){
	assert(p != NULL);
	if(p -> kind == VARIABLE_3){
		fprintf(ir,"v%d", p -> u.var_no);
	}
	else if(p -> kind == TEMPVAR){
		fprintf(ir,"t%d", p -> u.var_no);
	}
	else if(p -> kind == CONSTANT){
		fprintf(ir,"#%d", p -> u.var_int);
	}
	else if(p -> kind == LABELADDRESS){
		fprintf(ir,"label%d", p -> u.var_no);
	}
	else if(p -> kind == ADDRESS){
		fprintf(ir,"&v%d", p -> u.var_no);
	}
	return ;
}
InterCodes* codesJoin(InterCodes* headx, InterCodes* heady){
	if(headx == NULL)return heady;
	if(heady == NULL)return headx;
	InterCodes* xtail = headx -> prev;
	InterCodes* ytail = heady -> prev;
	headx -> prev = ytail;	ytail -> next = headx;
	xtail -> next = heady;	heady -> prev = xtail;
	return headx;
}
InterCodes* codesInit(int k, int n, ...){
	int i;
	InterCodes* codes = (InterCodes*)malloc(sizeof(InterCodes));
	assert(codes != NULL);

	InterCode* p = (InterCode*)malloc(sizeof(InterCode));
	assert(p != NULL);
	p -> kind = k;
	codes -> code = p;
	codes -> prev = codes;
	codes -> next = codes;
	if(k == BLANKLINE)return codes;
	va_list vl;
	va_start(vl, n);
	/* LABEL x : */
	/* GOTO x */
	/* RETURN x */
	/* ARG x */
	/* PARAM x */
	/* READ x */
	/* WRITE x */
	if(k == LABEL || k == GOTO || k == RETURN_3 || k == ARG || k == PARAM || k == READ || k == WRITE){
		p -> u.one.op = va_arg(vl, Operand*);
	}
	/* FUNCTION f : */
	else if(k == FUNCTION_3){
		p -> u.func.stnode = va_arg(vl, SNode*);
	}
	/* x := y */
	/* x := &y */
	/* x := *y */
	/* *x := y */
	else if(k == ASSIGN_ORIGIN || k == ASSIGN_ADDRESS_TO || k == ASSIGN_VALUE_FROM || k == ASSIGN_TO_ADDRESS){
		p -> u.assign.left = va_arg(vl, Operand*);
		p -> u.assign.right = va_arg(vl, Operand*);
	}
	/* x := y ? z */
	else if(k == ADD_3 || k == SUB_3 || k == MUL_3 || k == DIV_3){
		p -> u.binop.result = va_arg(vl, Operand*);
		p -> u.binop.op1 = va_arg(vl, Operand*);
		p -> u.binop.op2 = va_arg(vl, Operand*);
	}
	/* IF x [relop] y GOTO z */
	else if(k == IFGOTO){
		p -> u.ifgoto.rel1 = va_arg(vl, Operand*);
		p -> u.ifgoto.reltype = va_arg(vl, int);
		p -> u.ifgoto.rel2 = va_arg(vl, Operand*);
		p -> u.ifgoto.go = va_arg(vl, Operand*);
	}
	/* DEC x [size] */
	else if(k == DEC){
		p -> u.dec.op = va_arg(vl, Operand*);
		int size = va_arg(vl, int);
		assert(size > 0 && size % 4 == 0);
		p -> u.dec.size = size;
	}
	/* x := CALL f */
	else if(k == CALLFUNC){
		p -> u.call.op = va_arg(vl, Operand*);
		p -> u.call.stnode = va_arg(vl, SNode*);
	}
	
	va_end(vl);

	return codes;
}
Operand* opInit(int k, int var){
	Operand* p = (Operand*)malloc(sizeof(Operand));
	assert(p != NULL);
	p -> kind = k;
	if(k == VARIABLE_3 || k == LABELADDRESS || k == TEMPVAR){
		assert(var != 0);
		p -> u.var_no = var;
	}
	else if(k == CONSTANT){
		p -> u.var_int = var;
	}
	else if(k == ADDRESS){
		assert(var != 0);
		p -> u.var_no = var;
	}
	return p;
}
