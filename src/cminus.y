/****************************************************/
/* File: tiny.y                                     */
/* The TINY Yacc/Bison specification file           */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/
%{
#define YYPARSER /* distinguishes Yacc output from other code files */

#include "globals.h"
#include "util.h"
#include "scan.h"
#include "parse.h"
#include "stdlib.h"
#include "stdio.h"

#define YYSTYPE TreeNode *
static char * savedName; /* for use in assignments */
static int savedLineNo;  /* ditto */
static TreeNode * savedTree; /* stores syntax tree for later return */
static int yylex(void);
int yyerror(char *s);
extern struct Stack name_stack;
extern struct Stack number_stack;
extern struct Stack var_or_array_stack;
extern struct Stack lineno_stack;
extern char *id_name;



%}

%token ELSE IF INT RETURN VOID WHILE 
%token ID NUM 
%token ASSIGN EQ LT LE GT GE NE PLUS MINUS TIMES OVER LPAREN RPAREN SEMI COMMA LBRACE RBRACE LBRACKET RBRACKET 
%token ERROR 

%%

program: 
          declaracao_lista {savedTree = $1;} 
;

declaracao_lista: 
                  declaracao_lista declaracao {YYSTYPE t = $1;
                                                if (t != NULL) {
                                                    while (t->sibling != NULL) t = t->sibling;
                                                    t->sibling = $2;
                                                    $$ = $1;
                                                } else {
                                                    $$ = $2;
                                                }
                                              }
|                 declaracao {$$ = $1;}
;
       
declaracao: 
            var_declaracao {$$ = $1;} 
|           fun_declaracao {$$ = $1;}
;

var_declaracao:
    tipo_especificador ID SEMI {
        $$ = newStmtNode(VarDecK);
        $$->attr.op = (TokenType)(intptr_t)$1;
        $$->child[0] = newExpNode(IdK);
        
        char *poppedStr = (char *)pop(&name_stack);
        $$->child[0]->attr.name = copyString(poppedStr);
        char *poppedLin = (char *)pop(&lineno_stack);
        $$->child[0]->lineno = atoi(copyString(poppedLin));
        free(poppedLin);

        push(&var_or_array_stack, "var"); 
        
    }
    | tipo_especificador ID LBRACKET NUM RBRACKET SEMI {
        $$ = newStmtNode(VarDecK);
        $$->attr.op = (TokenType)(intptr_t)$1;
        $$->child[0] = newExpNode(IdK);
        
        char *poppedStr = (char *)pop(&name_stack);
        $$->child[0]->attr.name = copyString(poppedStr);
        char *poppedLin = (char *)pop(&lineno_stack);
        $$->child[0]->lineno = atoi(copyString(poppedLin));
        free(poppedLin);

        push(&var_or_array_stack, "array"); 
        $$->child[0]->child[0] = newExpNode(ConstK);
        
        char *poppedInt = (char *)pop(&number_stack);
        $$->child[0]->child[0]->attr.val = atoi(poppedInt);
        free(poppedInt);
    }
;

tipo_especificador:
                    INT {$$ = (YYSTYPE) INT;}
|                   VOID {$$ = (YYSTYPE) VOID;}
;

fun_declaracao:
                tipo_especificador ID LPAREN params RPAREN composto_decl {$$ = newStmtNode(FunDecK);                                                                                                                                     
                                                                          $$->attr.op = (TokenType)(intptr_t)$1;
                                                                          $$->child[0] = newExpNode(IdK);
                                                                          char *poppedStr = (char *)pop(&name_stack);
                                                                          $$->child[0]->attr.name = copyString(poppedStr);  
                                                                          char *poppedLin = (char *)pop(&lineno_stack);
                                                                          $$->child[0]->lineno = atoi(copyString(poppedLin));
                                                                          free(poppedLin);

                                                                          $$->child[1] = $4;
                                                                          $$->child[2] = $6;
                                                                          }
;

params:
        param_lista {$$ = $1;}
|       VOID {$$ = NULL;}
;

param_lista:
            param_lista COMMA param {YYSTYPE t = $1;
                                      if (t != NULL) {
                                        while (t->sibling != NULL) t = t->sibling;
                                              t->sibling = $3;
                                              $$ = $1;
                                    } else $$ = $3;}
|           param {$$ = $1;}
;

param: 
      tipo_especificador ID {$$ = newStmtNode(VarDecK);
			                $$->attr.op = (TokenType)(intptr_t)$1;
			                $$->child[0] = newExpNode(IdK);
                      push(&var_or_array_stack, "var");
                      char *poppedStr = (char *)pop(&name_stack);
                      $$->child[0]->attr.name = copyString(poppedStr);
                      char *poppedLin = (char *)pop(&lineno_stack);
                      $$->child[0]->lineno = atoi(copyString(poppedLin));
                      free(poppedLin);
                      
			   }
|     tipo_especificador ID LBRACKET RBRACKET {$$ = newStmtNode(VarDecK);
			                $$->attr.op = (TokenType)(intptr_t)$1;
			                $$->child[0] = newExpNode(IdK);
                      push(&var_or_array_stack, "array"); 
                      char *poppedStr = (char *)pop(&name_stack);
                      $$->child[0]->attr.name = copyString(poppedStr);
                      char *poppedLin = (char *)pop(&lineno_stack);
                      $$->child[0]->lineno = atoi(copyString(poppedLin));
                      free(poppedLin);
                      
                       }
;

composto_decl:
              LBRACE local_declaracoes statement_lista RBRACE {YYSTYPE t = $2;
                if (t != NULL)
                {
                    while (t->sibling != NULL) t = t->sibling;
                    t->sibling = $3;
                    $$ = $2;
			    }
                else 
                  $$ = $3;}
;

local_declaracoes:
                   local_declaracoes var_declaracao {YYSTYPE t = $1;
                if (t != NULL) {
			        while (t->sibling != NULL) t = t->sibling;
                    t->sibling = $2;
                    $$ = $1;
			    } else $$ = $2;}
|                   /* vazio */ {$$ = NULL;}
;

statement_lista:
                  statement_lista statement {YYSTYPE t = $1;
                if (t != NULL){
			        while (t->sibling != NULL) t = t->sibling;
                    t->sibling = $2;
                    $$ = $1;
			    } else $$ = $2;}
|                  /* vazio */ {$$ = NULL;}
;

statement:
            expressao_decl  {$$ = $1;}
|           composto_decl {$$ = $1;}
|           selecao_decl {$$ = $1;}
|           iteracao_decl {$$ = $1;}
|           retorno_decl {$$ = $1;}
;

expressao_decl :
                 expressao SEMI {$$ = $1;}
|                SEMI {$$ = NULL;}
;

selecao_decl : 
               IF LPAREN expressao RPAREN statement {$$ = newStmtNode(IfK);
                                                    $$->child[0] = $3;
                                                    $$->child[1] = $5;}
|              IF LPAREN expressao RPAREN statement ELSE statement {$$ = newStmtNode(IfK);
                                                                    $$->child[0] = $3;
                                                                    $$->child[1] = $5;
                                                                    $$->child[2] = $7;}
;

iteracao_decl :
               WHILE LPAREN expressao RPAREN statement {$$ = newStmtNode(WhileK);
                                                        $$->child[0] = $3;
                                                        $$->child[1] = $5;}
;

retorno_decl : 
              RETURN SEMI {$$ = newStmtNode(ReturnK); }
|             RETURN expressao SEMI {$$ = newStmtNode(ReturnK);
			                               $$->child[0] = $2;}
;

expressao : 
              var ASSIGN expressao {$$ = newStmtNode(AssignK);
			    $$->child[0] = $1;
			    $$->child[1] = $3;}
|             simples_expressao {$$ = $1;}
;

var : 
      ID {$$ = newExpNode(IdK);
		         char *poppedStr = (char *)pop(&name_stack);
             $$->attr.name = copyString(poppedStr);
             char *poppedLin = (char *)pop(&lineno_stack);
             $$->lineno = atoi(copyString(poppedLin)); 
             free(poppedLin);

            
            	    }
|     ID LBRACKET expressao RBRACKET { $$ = newExpNode(IdK);

                char *poppedStr = (char *)pop(&name_stack);
                $$->attr.name = copyString(poppedStr);
                char *poppedLin = (char *)pop(&lineno_stack);
                $$->lineno = atoi(copyString(poppedLin));
                free(poppedLin);
            
		      $$->child[0] = $3;}
;

simples_expressao :
                    soma_expressao relacional soma_expressao {$$ = newExpNode(OpK);
                      $$->child[0] = $1;
			                $$->attr.op = (TokenType)(intptr_t)$2;
			                $$->child[1] = $3;}
|                   soma_expressao {$$ = $1; }
;

relacional : 
             LE {$$ = (YYSTYPE) LE; }
|            LT {$$ = (YYSTYPE) LT; }
|            GT {$$ = (YYSTYPE) GT; }
|            GE {$$ = (YYSTYPE) GE; }
|            EQ {$$  = (YYSTYPE) EQ; }
|            NE {$$  = (YYSTYPE) NE; }
;

soma_expressao: 
                soma_expressao soma termo {$$ = newExpNode(OpK);
			    $$->child[0] = $1;
			    $$->attr.op = (TokenType)(intptr_t) $2;
			    $$->child[2] = $3;}
|               termo {$$ = $1;}

soma : 
      PLUS {$$ = (YYSTYPE) PLUS;}
|     MINUS {$$ = (YYSTYPE) MINUS;}
;

termo : 
        termo mult fator {$$ = newExpNode(OpK);
			    $$->child[0] = $1;
			    $$->attr.op = (TokenType)(intptr_t)$2;
			    $$->child[2] = $3;}
|       fator {$$ = $1;}
;

mult : 
      TIMES {$$ = (YYSTYPE) TIMES;}
|     OVER {$$ = (YYSTYPE) OVER;}
;

fator : 
        LPAREN expressao RPAREN {$$ = $2;}
|       var {$$ = $1;}
|       ativacao {$$ = $1;}
|       NUM {$$ = newExpNode(ConstK);
             $$->attr.val = atoi(tokenString);}
;

ativacao : 
          ID LPAREN args RPAREN {
            $$ = newStmtNode(CallK);
            $$->child[0] = newExpNode(IdK);
            char *poppedStr = (char *)pop(&name_stack);
            $$->child[0]->attr.name = NULL;
            $$->attr.name = copyString(poppedStr);
            char *poppedLin = (char *)pop(&lineno_stack);
            $$->lineno = atoi(copyString(poppedLin));
            free(poppedLin);
            
            $$->child[1] = $3;
            
          }
;

args : 
      arg_lista {$$ = $1;}
|     /* vazio */ {$$ = NULL;}
;

arg_lista : 
            arg_lista COMMA expressao {YYSTYPE t = $1;
                                        if (t != NULL) {
                                        while (t->sibling != NULL) t = t->sibling;
                                              t->sibling = $3;
                                              $$ = $1;
                                        } else $$ = $3;
                                        } 
|           expressao {$$ = $1;}
;

%%



int yyerror(char * message)
{ fprintf(listing,"Syntax error at line %d: %s\n",lineno,message);
  fprintf(listing,"Current token: ");
  printToken(yychar,tokenString);
  Error = TRUE;
  return 0;
}

/* yylex calls getToken to make Yacc/Bison output
 * compatible with ealier versions of 
  TINY scanner
 */
static int yylex(void)
{ TokenType token = getToken();
  // if (token == ID)
  // { 
  //   int digits = snprintf(NULL, 0, "%d", lineno);
  //   char *str = (char *) malloc(digits + 1);
  //   snprintf(str, digits + 1, "%d", lineno);
  //   push(&lineno_stack, str); 
  // }
  return token; }

TreeNode * parse(void)
{ yyparse();
  return savedTree;
}