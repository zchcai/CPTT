#include "symbol.h"
#include <string.h>
SNode SHead = NULL;
SNode stInitNode(){
	SNode p = (SNode)malloc(sizeof(SNode_));
	if(p == NULL)return NULL;
	p->name = NULL;
	p->funcOrVariable = 0;
	p->visitedTag = 0;
	p->lineNumber = 0;
	p->next = NULL;
	return p;
}

/* consider only shown once */

SNode stFind(char* name){
	SNode p = SHead;
	while(p != NULL){
		if(strcmp(name, p->name) == 0)return p;
		p = p->next;
	}
	return NULL;
}
/* according to stFind's return */
void stInsert(SNode p){
	if(SHead == NULL){
		SHead = p;
	}
	else {
		SNode temp = SHead;
		SHead = p;
		p->next = SHead;
	}
}
int stDelete(char* name){
	SNode p = SHead;
	if(p != NULL){
		if(strcmp(name, p->name) == 0){
			SHead = SHead->next;
			free(p);
			return 0;
		}
	}
	else return -1;
	SNode q = p->next;
	while(q != NULL){
		if(strcmp(name, q->name) == 0){
			p->next = q->next;
			free(q);
			return 0;
		}
		p = q;
		q = q->next;
	}
	return -1;
}



