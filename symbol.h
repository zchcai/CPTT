typedef struct SNode_* SNode;
typedef struct Type_* Type;
typedef struct FieldList_* FieldList;

struct SNode_ {
	char name[32];
	enum{FUNC = 600, VARIABLE = 601, Definition, Declaration}funcOrVariable;
	int  visitedTag, lineNumber;
	union{
		struct FunctionMessage* Function;
		Type Variable;
	}Message;
	SNode next;
};

struct FunctionMessage {
	Type returnType;
	struct{
		Type p, next;
	}parameter;
	/*TODO: function field tag...*/
};

struct Type_ {
	enum { BASIC, ARRAY, STRUCTURE } kind;
	union {
		// 基本类型
		int basic;
		// 数组类型信息包括元素类型与数组大小构成
		struct { Type elem; int size; } array;
		// 结构体类型信息是一个链表
		FieldList structure;
	} u;
};
struct FieldList_ {
	char name[32]; // 域的名字
	Type type; // 域的类型
	FieldList tail; // 下一个域
};
SNode stInitNode(char* name);
SNode stFind(char* name);
void stInsert(SNode p);
int stDelete(char* name);
