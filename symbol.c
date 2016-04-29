#include <stdlib.h>
#include <string.h>
#include "symbol.h"
SNode* SHead = NULL;
SNode* stInitNode(char* name){
	SNode* p = (SNode*)malloc(sizeof(SNode));
	if(p == NULL)return NULL;
	strcpy(p->name, name);
	p->visitedTag = UNCLEAR;
	p->lineno = 0;
	p->next = NULL;
	return p;
}

/* consider only shown once */

/* this data structure has to updated. */
SNode* stFind(char* name){
	SNode* p = SHead;
	while(p != NULL){
		if(strcmp(name, p->name) == 0)break;
		p = p->next;
	}
	return p;
}
/* according to stFind's return */
void stInsert(SNode* p){
	if(SHead == NULL){
		SHead = p;
	}
	else {
		SNode* temp = SHead;
		SHead = p;
		p->next = temp;
	}
}
/* no use of stDelete currently */
int stDelete(char* name){
	SNode* p = SHead;
	if(p != NULL){
		if(strcmp(name, p->name) == 1){
			SHead = SHead->next;
			free(p);
			return 0;
		}
	}
	else return -1;
	SNode* q = p->next;
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
