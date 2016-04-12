struct Node{
		int type, lineno, childno;
		char* name;
		union {
			float Float;
			int Int;
		}value;
		char String[32];
		struct Node* parent;
		struct Node* child[8];
	};
/* declared non-terminals in some type */
	enum nonTer{Program = 300, ExtDefList = 301, ExtDef, ExtDecList, Specifier, FunDec, CompSt, VarDec, StructSpecifier, OptTag, DefList, Tag, VarList, ParamDec, StmtList, Stmt, Exp, Def, DecList, Dec, Args};
	enum relopType{GT = 400, LT = 401, GE, LE, EQ, NE};
