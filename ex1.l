%option yylineno
delim	[ \t\n]
ws		{delim}+
letter	[A-Za-z]
letter_	[_A-Za-z]
digit	[0-9]
id		{letter_}({letter_}|{digit}){0,31}
octal	0[0-7]+
decimal 0|([1-9][0-9]*)
hex		0[xX][0-9a-fA-F]+
float	{decimal}\.{digit}+
exp		(({digit}+\.{digit}*)|(\.{digit}+))[eE][+-]?{digit}+
type	(int)|(float)
%%
{ws}	{/* no action and no return */}
while	{printf("%s\n", "WHILE");}
if	{printf("%s\n", "IF");}
else	{printf("%s\n", "ELSE");}
return	{printf("%s\n", "RETURN");}
struct	{printf("%s\n", "STRUCT");}
{type}	{printf("%s\n", "TYPE");}
{id}	{printf("%s\n", "ID");}
{octal}|{decimal}|{hex}	{printf("%s\n", "INT");}
{float}|{exp}	{printf("%s\n", "FLOAT");}
"!"	{printf("%s\n", "NOT");}
"="	{printf("%s\n", "ASSIGNOP");}
">"	{printf("%s\t%s\n", "RELOP", yytext);}
"<"	{printf("%s\t%s\n", "RELOP", yytext);}
">="	{printf("%s\t%s\n", "RELOP", yytext);}
"<="	{printf("%s\t%s\n", "RELOP", yytext);}
"=="	{printf("%s\t%s\n", "RELOP", yytext);}
"!="	{printf("%s\t%s\n", "RELOP", yytext);}
"("	{printf("%s\n", "LP");}
")"	{printf("%s\n", "RP");}
"["	{printf("%s\n", "LB");}
"]"	{printf("%s\n", "RB");}
"{"	{printf("%s\n", "LC");}
"}"	{printf("%s\n", "RC");}
"+"	{printf("%s\n", "PLUS");}
"-"	{printf("%s\n", "MINUS");}
"*"	{printf("%s\n", "STAR");}
"/"	{printf("%s\n", "DIV");}
"."	{printf("%s\n", "DOT");}
";"	{printf("%s\n", "SEMI");}
","	{printf("%s\n", "COMMA");}
"||"	{printf("%s\n", "OR");}
"&&"	{printf("%s\n", "AND");}
.	{printf("Error type A at Line %d: Mysterious characters \'%s\'\n", yylineno, yytext);}
"/*"	{printf("%s\n", "LN");}
"*/"	{printf("%s\n", "RN");}
"//"	{
	char c = input();
	while(c != '\n') c = input();
}
%%
int main(int argc, char** argv){
	if(argc > 1){
		if(!(yyin = fopen(argv[1], "r"))){
			perror(argv[1]);
			return 1;
		}
	}
	while (yylex() != 0);
	return 0;
}
