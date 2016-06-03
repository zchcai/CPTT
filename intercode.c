#include <stdlib.h>
extern SNode* SHead;
extern Type* TypeNodeInt;
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
