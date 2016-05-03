%{
	#include <stdio.h>
	#include <stdlib.h>
	#include <stdarg.h>
	#include "tree.h"
	Node* init_node(int);
	Node* create_node(int, int, ...);
	char* type2name(int);
	Node* Head = NULL;
	int lex_error = 0, syntax_error = 0;
 
/* declared non-terminals in some type 
	now transfer to tree.h*/
	#define YYSTYPE Node*
%}
%error-verbose
/* declared tokens */
%token INT FLOAT ID TYPE
%nonassoc LOWER_THAN_SEMI
%token SEMI COMMA
%token LC RC
%nonassoc LOWER_THAN_ELSE
%nonassoc STRUCT RETURN IF ELSE WHILE
/* 8 */
%right ASSIGNOP
/* 7 */
%left OR
/* 6 */
%left AND
/* 5 */
%left RELOP
/* 4 */
%left PLUS MINUS
/* 3 */
%left STAR DIV
/* 2 */
%right NOT UMINUS /* OK if UMINUS is left or token */
/* 1 */
%left DOT LP RP LB RB
%%
Program	: ExtDefList {$$ = create_node(Program, 1, $1);Head = $$;}
	;
ExtDefList	: ExtDef ExtDefList {$$ = create_node(ExtDefList, 2, $1, $2);}
	| {$$ = create_node(ExtDefList, 0);}
	;
ExtDef	: Specifier ExtDecList SEMI {$$ = create_node(ExtDef, 3, $1, $2, $3);}
	| Specifier ExtDecList %prec LOWER_THAN_SEMI {yyerror("Missing \";\"");}
	| Specifier SEMI {$$ = create_node(ExtDef, 2, $1, $2);}
	| Specifier %prec LOWER_THAN_SEMI {yyerror("Missing \";\"");}
	| Specifier FunDec CompSt {$$ = create_node(ExtDef, 3, $1, $2, $3);}
	| error SEMI
	;
ExtDecList	: VarDec {$$ = create_node(ExtDecList, 1, $1);}
	| VarDec COMMA ExtDecList {$$ = create_node(ExtDecList, 3, $1, $2, $3);}
	;
Specifier	: TYPE {$$ = create_node(Specifier, 1, $1);}
	| StructSpecifier {$$ = create_node(Specifier, 1, $1);}
StructSpecifier	: STRUCT OptTag LC DefList RC {$$ = create_node(StructSpecifier, 5, $1, $2, $3, $4, $5);}
	| STRUCT Tag {$$ = create_node(StructSpecifier, 2, $1, $2);}
	;
OptTag	: ID {$$ = create_node(OptTag, 1, $1);}
	| {$$ = create_node(OptTag, 0);}
	;
Tag	: ID {$$ = create_node(Tag, 1, $1);}
	;

VarDec	: ID {$$ = create_node(VarDec, 1, $1);}
	| VarDec LB INT RB {$$ = create_node(VarDec, 4, $1, $2, $3, $4);}
	;
FunDec	: ID LP VarList RP {$$ = create_node(FunDec, 4, $1, $2, $3, $4);}
	| ID LP RP {$$ = create_node(FunDec, 3, $1, $2, $3);}
	| ID LP %prec LOWER_THAN_SEMI {yyerror("Missing \")\" or Specifier");}
	;
VarList	: ParamDec COMMA VarList {$$ = create_node(VarList, 3, $1, $2, $3);}
	| ParamDec {$$ = create_node(VarList, 1, $1);}
	;
ParamDec	: Specifier VarDec {$$ = create_node(ParamDec, 2, $1, $2);}
	;
CompSt	: LC DefList StmtList RC {$$ = create_node(CompSt, 4, $1, $2, $3, $4);}
	| error RC
	;
StmtList	: Stmt StmtList {$$ = create_node(StmtList, 2, $1, $2);}
	| {$$ = create_node(StmtList, 0);}
	;
Stmt	: Exp SEMI {$$ = create_node(Stmt, 2, $1, $2);}
	| CompSt {$$ = create_node(Stmt, 1, $1);}
	| RETURN Exp SEMI {$$ = create_node(Stmt, 3, $1, $2, $3);}
	| IF LP Exp RP Stmt %prec LOWER_THAN_ELSE {$$ = create_node(Stmt, 5, $1, $2, $3, $4, $5);}
	| IF LP Exp RP Stmt ELSE Stmt {$$ = create_node(Stmt, 6, $1, $2, $3, $4, $5, $6);}
	| WHILE LP Exp RP Stmt {$$ = create_node(Stmt, 5, $1, $2, $3, $4, $5);}
	| error SEMI
	;

DefList	: Def DefList {$$ = create_node(DefList, 2, $1, $2);}
	| {$$ = create_node(DefList, 0);}
	;
Def	: Specifier DecList SEMI {$$ = create_node(Def, 3, $1, $2, $3);}
	| Specifier DecList Specifier DecList Specifier DecList SEMI %prec LOWER_THAN_SEMI {yyerror("Previous local definition expecting \";\"");}
	| Specifier DecList Specifier DecList SEMI %prec LOWER_THAN_SEMI {yyerror("Previous local definition expecting \";\"");}
	;
DecList	: Dec {$$ = create_node(DecList, 1, $1);}
	| Dec COMMA DecList {$$ = create_node(DecList, 3, $1, $2, $3);}
	;
Dec	: VarDec {$$ = create_node(Dec, 1, $1);}
	| VarDec ASSIGNOP Exp {$$ = create_node(Dec, 3, $1, $2, $3);}
	;

Exp	: Exp ASSIGNOP Exp {$$ = create_node(Exp, 3, $1, $2, $3);}
	| Exp AND Exp {$$ = create_node(Exp, 3, $1, $2, $3);}
	| Exp OR Exp {$$ = create_node(Exp, 3, $1, $2, $3);}
	| Exp RELOP Exp {$$ = create_node(Exp, 3, $1, $2, $3);}
	| Exp PLUS Exp {$$ = create_node(Exp, 3, $1, $2, $3);}
	| Exp MINUS Exp {$$ = create_node(Exp, 3, $1, $2, $3);}
	| Exp STAR Exp {$$ = create_node(Exp, 3, $1, $2, $3);}
	| Exp DIV Exp {$$ = create_node(Exp, 3, $1, $2, $3);}
	| LP Exp RP {$$ = create_node(Exp, 3, $1, $2, $3);}
	| MINUS Exp %prec UMINUS {$$ = create_node(Exp, 2, $1, $2);}
	| NOT Exp {$$ = create_node(Exp, 2, $1, $2);}
	| ID LP Args RP {$$ = create_node(Exp, 4, $1, $2, $3, $4);}
	| ID LP %prec LOWER_THAN_SEMI {yyerror("Missing \")\"");}
	| ID LP RP {$$ = create_node(Exp, 3, $1, $2, $3);}
	| Exp LB Exp RB {$$ = create_node(Exp, 4, $1, $2, $3, $4);}
	| Exp DOT ID {$$ = create_node(Exp, 3, $1, $2, $3);}
	| ID {$$ = create_node(Exp, 1, $1);}
	| INT {$$ = create_node(Exp, 1, $1);}
	| FLOAT {$$ = create_node(Exp, 1, $1);}
	| error RP
	| error RB
	| error INT
	| error FLOAT
	;
Args	: Exp COMMA Args {$$ = create_node(Args, 3, $1, $2, $3);}
	| Exp {$$ = create_node(Args, 1, $1);}
	;
%%
#include "lex.yy.c"
Node* init_node(int T){
	int i;
	Node* p;
	if((p = (Node*)malloc(sizeof(Node))) == NULL)
		return NULL;
	p->type = T;
	p->parent = NULL;
	p->name = type2name(T);
	p->childno = 0;
	for(i = 0; i < 8; i++)p->child[i] = NULL;
	p->exptype = NULL;
	return p;
}
Node* create_node(int type, int n, ...){
	int i;	
	Node* p = init_node(type);
	p->childno = n;
	if(!n)return p;
	Node* val;
	va_list vl;
	va_start(vl, n);
	for(i = 0; i < n; i++){
		val = va_arg(vl, Node*);
		switch(val -> type){
			default: break;
		}
		val -> parent = p;
		p -> child[i] = val;
	}
	va_end(vl);
	p->lineno = p->child[0]->lineno;
	return p;
}
/* print_node is for lab1 and no use any more */
void print_node(Node* root, int nLayer){
	if(root == NULL){
		printf("PRINT ERROR!!!!\n");
		return;
	}
	int childno = root->childno;
	int type = root->type;
	if(!childno && type >=300)return ;
	int i;	
	for(i = 0;i < nLayer; i++)
		printf("  ");
	printf("%s", root->name);
	if(type >= 300)printf(" (%d)", root->lineno);
	else{
		if((type == ID) || (type == TYPE)){printf(": %s",root->String);}
		else if(type == INT){printf(": %d", root->value.Int);}
		else if(type == FLOAT){printf(": %f", root->value.Float);}
	}
	printf("\n");
	for(i = 0; i< childno;i++)
		print_node(root->child[i], nLayer+1);
}

yyerror(char* msg) {
	syntax_error ++;
	if(msg[0] == 's')msg[0] = 'S';
	fprintf(stdout, "Error type B at Line %d: %s.\n",yylineno, msg);
}
char* type2name(int type){
switch(type){
	case Program: return "Program";
	case ExtDefList: return "ExtDefList";
	case ExtDef: return "ExtDef";
	case ExtDecList: return "ExtDecList";
	case Specifier: return "Specifier";
	case FunDec: return "FunDec";
	case CompSt: return "CompSt";
	case VarDec: return "VarDec";
	case StructSpecifier: return "StructSpecifier";
	case OptTag: return "OptTag";
	case DefList: return "DefList";
	case Tag: return "Tag";
	case VarList: return "VarList";
	case ParamDec: return "ParamDec";
	case StmtList: return "StmtList";
	case Stmt: return "Stmt";
	case Exp: return "Exp";
	case Def: return "Def";
	case DecList: return "DecList";
	case Dec: return "Dec";
	case Args: return "Args";
	case INT: return "INT";
	case FLOAT: return "FLOAT";
	case ID: return "ID";
	case TYPE: return "TYPE";
	case SEMI: return "SEMI";
	case COMMA: return "COMMA";
	case GT: return "GT";
	case LT: return "LT";
	case GE: return "GE";
	case LE: return "LE";
	case EQ: return "EQ";
	case NE : return "NE";
	case LC: return "LC";
	case RC: return "RC";
	case STRUCT: return "STRUCT";
	case RETURN: return "RETURN";
	case IF: return "IF";
	case ELSE: return "ELSE";
	case WHILE: return "WHILE";
	case ASSIGNOP: return "ASSIGNOP";
	case OR: return "OR";
	case AND: return "AND";
	case RELOP: return "RELOP";
	case PLUS: return "PLUS";
	case MINUS: return "MINUS";
	case STAR: return "STAR";
	case DIV: return "DIV";
	case NOT: return "NOT";
	case DOT: return "DOT";
	case LP: return "LP";
	case RP: return "RP";
	case LB: return "LB";
	case RB: return "RB";
	}
}
