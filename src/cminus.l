/****************************************************/
/* File: cminus.l                                   */
/* Lex specification for C-                         */
/* Project for CES41: Compiladores                  */
/****************************************************/

%{
#include "globals.h"
#include "util.h"
#include "scan.h"
/* lexeme of identifier or reserved word */
char tokenString[MAXTOKENLEN+1];
static int yylex(void);
extern struct Stack name_stack;
extern struct Stack number_stack;
char *id_name;
char *number_value;
%}

digit       [0-9]
number      {digit}+
letter      [a-zA-Z]
identifier  {letter}+
newline     \r?\n|\r
whitespace  [ \t]+

%option noyywrap

%%

"else"          {return ELSE;}
"if"            {return IF;}
"int"           {return INT;}
"return"        {return RETURN;}
"void"          {return VOID;}
"while"         {return WHILE;}
"="             {return ASSIGN;}
"=="            {return EQ;}
"<"             {return LT;}
"<="            {return LE;}
">"             {return GT;}
">="            {return GE;}
"!="            {return NE;}
"+"             {return PLUS;}
"-"             {return MINUS;}
"*"             {return TIMES;}
"/"             {return OVER;}
"("             {return LPAREN;}
")"             {return RPAREN;}
";"             {return SEMI;}
","             {return COMMA;}
"{"             {return LBRACE;}
"}"             {return RBRACE;}
"["             {return LBRACKET;}
"]"             {return RBRACKET;}
{number}        {number_value = copyString(yytext);
                 push(&number_stack, number_value);
                 return NUM;}
{identifier}    {id_name = copyString(yytext);
                 push(&name_stack, id_name);
                 return ID;}
{newline}       {
                if(linhas[lineno] == NULL)
                  fprintf(listing,"%d: \n",lineno + 1);
                else
                  fprintf(listing,"%d: %s",lineno + 1, linhas[lineno]);
                lineno++;
                }

{whitespace}    {/* skip whitespace */}
"/*" { char c;
       char d;
       //printf("%d: /*", lineno);
       do {
        c = input();
        //printf("%c", c);
        if (c == EOF) break;
        if (c == '\n') 
        {
          lineno++;
          //printf("%d: ", lineno);
          fprintf(listing,"%d: %s",lineno, linhas[lineno - 1]);
        }
        if (c == '*') /* to close the comment, '/' must follow '*' */ 
        {
          d = input();
          if (d == '/') break;
        }
       } while (c);
       //printf("/\n");
      }
.               {return ERROR;}

%%

TokenType getToken(void)
{ static int firstTime = TRUE;
  TokenType currentToken;
  if (firstTime)
  { firstTime = FALSE;
    if(linhas[lineno] == NULL)
      fprintf(listing,"%d: \n",lineno + 1);
    else
      fprintf(listing,"%d: %s",lineno + 1, linhas[lineno]);
    lineno++;
    yyin = source;
    yyout = listing;
  }
  currentToken = yylex();
  strncpy(tokenString,yytext,MAXTOKENLEN);
  if (TraceScan) {
    fprintf(listing,"\t%d: ",lineno);
    printToken(currentToken,tokenString);
  }
  return currentToken;
}

