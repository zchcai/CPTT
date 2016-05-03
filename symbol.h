typedef struct SymbolTableNode {
	char name[32];
	enum{ FUNC = 600, /* for function Def */
		VARIABLE = 601, /* for var Def */
		STRUCTDEF = 602, 
		DECLAR = 603,/* for func Dec */
		UNCLEAR = 604
	}visitedTag; 
	int lineno;
	union{
		struct FunctionMessageNode* func;
		struct TypeNode* var;
	}Message;
	struct SymbolTableNode* next;
	struct SymbolTableNode* fnext;
}SNode;

typedef struct FunctionMessageNode {
	struct TypeNode* returnType;
	struct{
		//TODO: now global var
		SNode* head;
		int no;
	}para;
	/*TODO: function field tag...*/
}Funcmsg;

typedef struct TypeNode {
	enum { BASIC, ARRAY, STRUCTURE } kind;
	union {
		// 基本类型
		int basic;
		// 数组类型信息包括元素类型与数组大小构成
		struct { struct TypeNode* elem; int size; } array;
		// 结构体类型信息是一个链表
		struct FieldListNode* structure;
	} u;
}Type;

typedef struct FieldListNode{
	char name[32]; // 域的名字
	struct TypeNode* type; // 域的类型
	struct FieldListNode* tail; // 下一个域
}FieldList;
SNode* stInitNode(char* name);
SNode* stFind(char* name);
void stInsert(SNode* p);
void stPrint();
Type* sfFind(Type*, char*);
