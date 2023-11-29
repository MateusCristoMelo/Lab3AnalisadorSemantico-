/****************************************************/
/* File: util.h                                     */
/* Utility functions for the TINY compiler          */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#ifndef _UTIL_H_
#define _UTIL_H_

/* Procedure printToken prints a token 
 * and its lexeme 
 */
void printToken( TokenType, const char* );

/* Function newStmtNode creates a new statement
 * node for syntax tree construction
 */
TreeNode * newStmtNode(StmtKind);

/* Function newExpNode creates a new expression 
 * node for syntax tree construction
 */
TreeNode * newExpNode(ExpKind);

/* Function copyString allocates and makes a new
 * copy of an existing string
 */
char * copyString( char * );

/* procedure printTree prints a syntax tree 
 * using indentation to indicate subtrees
 */
void printTree( TreeNode * );


#define MAX_SIZE 100

// Definição da estrutura da pilha
struct Stack {
    void *items[MAX_SIZE];
    int top;
};

// Função para inicializar a pilha
void initStack(struct Stack *stack);

// Função para verificar se a pilha está vazia
int isEmpty(struct Stack *stack);

// Função para verificar se a pilha está cheia
int isFull(struct Stack *stack);

// Função para adicionar um elemento à pilha (push)
void push(struct Stack *stack, void *item);

// Função para retirar um elemento da pilha (pop)
void *pop(struct Stack *stack);

#endif
