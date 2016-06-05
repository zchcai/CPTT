typedef struct OperandStruct{
	enum{ VARIABLE_3, CONSTANT, ADDRESS}kind;
	union{
		int var_no;
		int var_int;
		/* In Lab3, float won't appear */
	}u;
}Operand;

typedef struct InterMediateCode{
	enum{LABEL, FUNCTION_3, ASSIGN_ORIGIN, ADD_3, SUB_3, MUL_3, DIV_3, ASSIGN_ADDRESS_TO, ASSIGN_VALUE_FROM, ASSIGN_TO_ADDRESS, GOTO, IFGOTO, RETURN_3, DEC, ARG, CALLFUNC, PARAM, READ, WRITE, BLANKLINE}kind;
	union{
		struct{struct OperandStruct *left, *right;}assign;
		struct{struct OperandStruct *result, *op1, *op2;}binop;
		struct{struct OperandStruct* op;}one;
		struct{
			struct OperandStruct* op;
			int size;
		}dec;
		struct{struct OperandStruct *op1, *op2;}two;
		struct{
			struct OperandStruct *rel1, *rel2, *go;
			int reltype;
			/* enum relopType{GT = 401, LT = 401, GE = 402, LE = 403, EQ = 404, NE = 405}; */
		}ifgoto;
		struct{ SNode* stnode; }func;
		struct{ 
			SNode* stnode;
			struct OperandStruct* op;
		}call;
	}u;
}InterCode;

typedef struct InterCodeListNode{
	struct InterMediateCode* code;
	struct InterCodeListNode *prev, *next;
}InterCodes;

void print_intercodes(InterCodes*);
void print_intercode(InterCode*);
void print_operand(Operand*);
InterCodes* codesJoin(InterCodes*, InterCodes*);
InterCodes* codesInit(int k, int n, ...);
void test_intercodes_data_structure();
Operand* opInit(int, int);
