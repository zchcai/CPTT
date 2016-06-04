#include <stdarg.h>
#include <stdlib.h>
#include "intercode.h"
extern Node* Head;
extern SNode* SHead;
extern Type* TypeNodeInt;
void print_intercodes(InterCodes*);
void print_intercode(InterCode*);
void print_operand(Operand*);
InterCodes* codesInit(InterCode*);
InterCodes* codesJoin(InterCodes*, InterCodes*);
InterCode* codeInit(int k, int n, ...);
Operand* opInit(int, int);
void test(){
	Operand* op1 = opInit(ADDRESS, 1);
	Operand* op2 = opInit(VARIABLE_3, 1);
	Operand* op4 = opInit(VARIABLE_3, 2);
	Operand* op3 = opInit(CONSTANT, 6);
	SNode* stnode = stInitNode("main");
	InterCodes* code0 = codesInit(codeInit(FUNCTION_3, 1, stnode));
	InterCodes* code1 = codesInit(codeInit(LABEL, 1, op1));
	InterCodes* code2 = codesInit(codeInit(READ, 1, op2));
	InterCodes* code3 = codesInit(codeInit(RETURN_3, 1, op2));
	InterCodes* code4 = codesInit(codeInit(IFGOTO, 4, op2, GT, op3, op1));
	InterCodes* code5 = codesInit(codeInit(DEC, 2, op4, 80));
	code0 = codesJoin(code0, code1);
	code2 = codesJoin(code2, code3);
	code4 = codesJoin(code4, code5);
	print_intercodes(codesJoin(codesJoin(code0, code2), code4));
}
void intermediate_code_generation(){
	test();
	//TODO
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
		printf("LABEL ");
		print_operand(p -> u.one.op);
		printf(" :");
	}
	/* FUNCTION f : */
	else if(k == FUNCTION_3){
		printf("FUNCTION %s :", p -> u.func.stnode -> name);
	}
	/* x := y */
	else if(k == ASSIGN_ORIGIN){
		print_operand(p -> u.assign.left);
		printf(" := ");
		print_operand(p -> u.assign.right);
	}
	/* x := y ? z */
	else if(k == ADD_3 || k == SUB_3 || k == MUL_3 || k == DIV_3){
		print_operand(p -> u.binop.result);
		printf(" := ");
		print_operand(p -> u.binop.op1);
		if(k == ADD_3){
			printf(" + ");
		}
		else if(k == SUB_3){
			printf(" - ");
		}
		else if(k == MUL_3){
			printf(" * ");
		}
		else if(k == DIV_3){
			printf(" / ");
		}
		print_operand(p -> u.binop.op2);
	}
	/* x := &y */
	else if(k == ASSIGN_ADDRESS_TO){
		print_operand(p -> u.assign.left);
		printf(" := ");
		printf("&");
		print_operand(p -> u.assign.right);
	}
	/* x := *y */
	else if(k == ASSIGN_VALUE_FROM){
		print_operand(p -> u.assign.left);
		printf(" := ");
		printf("*");
		print_operand(p -> u.assign.right);
	}
	/* *x := y */
	else if(k == ASSIGN_TO_ADDRESS){
		printf("*");
		print_operand(p -> u.assign.left);
		printf(" := ");
		print_operand(p -> u.assign.right);
	}
	/* GOTO x */
	else if(k == GOTO){
		printf("GOTO ");
		print_operand(p -> u.one.op);
	}
	/* IF x [relop] y GOTO z */
	else if(k == IFGOTO){
		printf("IF ");
		print_operand(p -> u.ifgoto.rel1);
		printf(" %s ", type2name(p -> u.ifgoto.reltype));
		print_operand(p -> u.ifgoto.rel2);
		printf(" GOTO ");
		print_operand(p -> u.ifgoto.go);
	}
	/* RETURN x */
	else if(k == RETURN_3){
		printf("RETURN ");
		print_operand(p -> u.one.op);
	}
	/* DEC x [size] */
	else if(k == DEC){
		printf("DEC ");
		print_operand(p -> u.dec.op);
		printf(" %d", p -> u.dec.size);
	}
	/* ARG x */
	else if(k == ARG){
		printf("ARG ");
		print_operand(p -> u.one.op);
	}
	/* x := CALL f */
	else if(k == CALLFUNC){
		print_operand(p -> u.call.op);
		printf(" := CALL %s", p -> u.call.stnode -> name);
	}
	/* PARAM x */
	else if(k == PARAM){
		printf("PARAM ");
		print_operand(p -> u.one.op);
	}
	/* READ x */
	else if(k == READ){
		printf("READ ");
		print_operand(p -> u.one.op);
	}
	/* WRITE x */
	else if(k == WRITE){
		printf("WRITE ");
		print_operand(p -> u.one.op);
	}
	printf("\n");
}	
void print_operand(Operand* p){
	assert(p != NULL);
	if(p -> kind == VARIABLE_3){
		printf("v%d", p -> u.var_no);
	}
	else if(p -> kind == CONSTANT){
		printf("#%d", p -> u.var_int);
	}
	else {
		printf("label%d", p -> u.var_no);
	}
	return ;
}
InterCodes* codesInit(InterCode* code){
	InterCodes* p = (InterCodes*)malloc(sizeof(InterCodes));
	if(p == NULL)return NULL;
	p -> code = code;
	p -> prev = p;
	p -> next = p;
	return p;
}
InterCodes* codesJoin(InterCodes* headx, InterCodes* heady){
	InterCodes* xtail = headx -> prev;
	InterCodes* ytail = heady -> prev;
	headx -> prev = ytail;	ytail -> next = headx;
	xtail -> next = heady;	heady -> prev = xtail;
	return headx;
}
InterCode* codeInit(int k, int n, ...){
	int i;
	InterCode* p = (InterCode*)malloc(sizeof(InterCode));
	if(p == NULL)return NULL;
	p -> kind = k;
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

	return p;
}
Operand* opInit(int k, int var){
	Operand* p = (Operand*)malloc(sizeof(Operand));
	if(p == NULL)return NULL;
	p -> kind = k;
	if(k == VARIABLE_3 || k == ADDRESS){
		p -> u.var_no = var;
	}
	else if(k == CONSTANT){
		p -> u.var_int = var;
	}
	return p;
}
