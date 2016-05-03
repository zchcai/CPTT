#include <stdlib.h>
#include <string.h>
#include "symbol.h"
/* SHead is the head of symbol table
 * TODO new structure */
SNode* SHead = NULL;
/* executable code cannot occur outside of a function */
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
	p->fnext = NULL;
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
	printf("Symbol table:\n");
	while(p != NULL){
		printf("\tNo %d: %s\t%d\n", i, p->name, p->visitedTag);
		if(p -> visitedTag == VARIABLE){
			printf("\t\tkind no: %d\n", p -> Message.var -> kind);
		}
		else if(p -> visitedTag == FUNC){
			printf("\t\treturnType kind no: %d\n", p ->Message.func -> returnType -> kind);
		}
		p = p->next;
		++ i;
	}
}
Type* sfFind(Type* p, char* name){
	FieldList* head = p -> u.structure;
	while(head != NULL){
		if(strcmp(name, head -> name) == 0){
			return head -> type;
		}
		head = head -> tail;
	}
	return NULL;
}
