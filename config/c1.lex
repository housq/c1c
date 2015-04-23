/*
 * pascal.lex: An example PASCAL scanner
 *
 */

%{
#include "util.h"
#include "symtab.h"
#include "ast.h"
#include "c1.tab.h"
#include <stdio.h>
#include <string.h>


int yycolumn =1;
#define YY_USER_ACTION yylloc.first_line = yylloc.last_line = yylineno; \
	yylloc.first_column = yycolumn; yylloc.last_column = yycolumn + yyleng -1;\
	yycolumn +=yyleng;


%}

%option yylineno

%x COMMENT

white_space       [ \t]*
eol		  \n
digit             [0-9]
alpha             [A-Za-z_]
alpha_num         ({alpha}|{digit})
hex_digit         [0-9A-Fa-f]
identifier        {alpha}{alpha_num}*
oct_integer       0{digit}+
unsigned_integer  {digit}+
hex_integer       0(x|X){hex_digit}{hex_digit}*
pragma		  #pragma{white_space}.*\n

%%


"/*"                BEGIN(COMMENT);
<COMMENT>[\n]	    yycolumn=1;
<COMMENT>[^/*]+
<COMMENT>"*/"       BEGIN(INITIAL);

 /* note that FILE and BEGIN are already 
  * defined in FLEX or C so they can't  
  * be used. This can be overcome in                               
  * a cleaner way by defining all the
  * tokens to start with TOK_ or some
  * other prefix.
  */


int 		     return(INT);
const                return(CONST);
if                   return(IF);
else		     return(ELSE);
odd		     return(ODDSYM);
void                 return(VOID);
while                return(WHILE);
extern		     return(EXTERN);

"\n"		     yycolumn=1;
"<="                 return(LEQSYM);
">="                 return(GEQSYM);
"!="                 return(NEQSYM);
"=="                 return(EQLSYM);
"="		     return(ASSIGN);
"<"	             return(LSSSYM);
">"		     return(GTRSYM);

"+"		     return(PLUSSYM);
"-"		     return(MINUSSYM);
"/"		     return(DIVSYM);
"*"		     return(MULTISYM);
"%"		     return(MODSYM);
"("		     return(LBRACKET);
")"		     return(RBRACKET);
"["		     return(LPAREN);
"]"		     return(RPAREN);
"{"		     return(LBRACE);
"}"		     return(RBRACE);

";"		     return(SEMICOLON);
","		     return(COMMA);

{pragma}		{
				yylval.name=(char *)malloc(yyleng+1);
				strcpy(yylval.name,yytext);
//				fprintf(stderr,"******************yylval.name is at %p\n",yylval.name);
//				fprintf(stderr,"******************string content: %s\n",yylval.name);
				yycolumn=1;
				return(DIRECTIVE);
			}

{oct_integer}		{
				yylval.val=0;
				int i;
				for(i=1;i<=yyleng-1;i++){
					if( (yytext[i]=='8') | (yytext[i]=='9') ) {
						yyerror("number 8&9 are not allowed in oct integer");
						return(NUMBER);
					}
					yylval.val=yylval.val*8+yytext[i]-'0';
				}
				return(NUMBER);
			}
{hex_integer}        	{
				yylval.val=0;
				int i;
				for(i=2;i<=yyleng-1;i++)
					yylval.val=yylval.val*16+hextoint(yytext[i]);
				return(NUMBER);
			}
{unsigned_integer}   	{ 
				yylval.val=0;
				int i;
				for(i=0;i<=yyleng-1;i++)
					yylval.val=yylval.val*10+yytext[i]-'0';
				return(NUMBER);
			}
				
{identifier}         	{
				yylval.name=malloc(yyleng+1);
				strcpy(yylval.name,yytext);
				return(IDENT);
			}
 
 

{white_space}        /* do nothing */
.                    {	
				yyerror("Illegal input");
		    		return(0);
		     }

%%



