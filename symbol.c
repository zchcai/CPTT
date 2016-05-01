#include <stdlib.h>
#include <string.h>
#include "symbol.h"
/* SHead is the head of symbol table
 * TODO new structure */
SNode* SHead = NULL;
Type TypeInt = {.kind = BASIC, .u.basic = INT};
Type TypeFloat = {.kind = BASIC, .u.basic = FLOAT};
Type* TypeNodeInt = &TypeInt;
Type* TypeNodeFloat = &TypeFloat;

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
void stPrint(){
	SNode* p = SHead;
	int i = 0;
	while(p != NULL){
		printf("No %d: %s\t%d\n", i, p->name, p->visitedTag);
		p = p->next;
	}
}
