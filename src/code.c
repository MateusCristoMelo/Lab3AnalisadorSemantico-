/****************************************************/
/* File: code.c                                     */
/* TM Code emitting utilities                       */
/* implementation for the TINY compiler             */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "code.h"

/* TM location number for current instruction emission */
static int emitLoc = 0 ;

/* Highest TM location emitted so far
   For use in conjunction with emitSkip,
   emitBackup, and emitRestore */
static int highEmitLoc = 0;

void emitLabel( char * label) 
{   
    pc("\n%10s:\n", label);
} /* emitLabel */

void emitHalt() 
{   
    pc("          halt\n");
} /* emitLabel */

void emitReturn(int v)
{
    pc("          return t%d\n", v);
}


/* Procedure emits not constant TM instruction
 * op = the opcode
 * r = target register
 * s = 1st source register
 * t = 2nd source register
 */
/*
char * intToString(int val)
{ char valString[11];
  sprintf(valString, "%d", val);
  return copyString(valString);
}


char * getNewBranchLabel( void) {
    branchLabelNumber++;
    char * branchLabelName;
    branchLabelName = malloc(12);
    char * labelNumberStr;
    labelNumberStr = malloc(11);

    labelNumberStr = intToString(branchLabelNumber);
    
    strcpy(branchLabelName,"L");
    pc("%s", branchLabelName);
    if (strlen(branchLabelName) + strlen(labelNumberStr) < sizeof(branchLabelName)) strcat(branchLabelName, labelNumberStr);
    return branchLabelName;
}*/

char* intToString(int num) {
    char* str = (char*)malloc(12); // Ajuste o tamanho conforme necessário
    snprintf(str, 12, "%d", num);
    return str;
}

// Função principal para obter um novo rótulo de ramificação
char* getNewBranchLabel(void) {
    branchLabelNumber++;
    
    // Alocar memória para o nome da ramificação e o número da ramificação
    char* branchLabelName = (char*)malloc(12); // Ajuste o tamanho conforme necessário
    char* labelNumberStr = intToString(branchLabelNumber);

    // Construir o rótulo da ramificação
    strcpy(branchLabelName, "L");
    strcat(branchLabelName, labelNumberStr);

    // Imprimir para verificar se está correto (opcional)
    //printf("%s\n", branchLabelName);

    // Liberar a memória alocada para o número da ramificação
    free(labelNumberStr);

    // Retornar o rótulo da ramificação
    return branchLabelName;
}


void emitBranchInstruction(char* x,  char * L, int checkTrue)
{   if (strlen(x) == 0) {
        pc("          goto %s\n", L);
    } else {
        if(checkTrue)
            pc("          if_true %s goto %s\n", x, L);
        else
            pc("          if_false %s goto %s\n", x, L);
    }
} 

void emitAssignInstruction( char *op, char* r, char* s, char* t) 
{   if (strlen(op) == 0 && strlen(t) == 0) {
        pc("          %s = %s \n",r,s); 
    } else {
        pc("          %s = %s %s %s \n",r,s,op,t);
    }
} /* emitAssingInstruction */

void emitParamInstruction( char* c )
{
    pc("          param %s \n", c);
}

void emitCallInstruction( char* x, char *f, int n)
{
    pc("          %s = call %s, %d\n", x, f, n);
}
/*
void emitCallInstruction( char* x, char *f, int n)
{
    fprintf(code,"\r%*s%s = call %s, %d", SPACE, "", x, f, n);
}*/

/* Procedure emitComment prints a comment line 
 * with comment c in the code file
 */
void emitComment( char * c )
{ if (TraceCode) pc("* %s\n",c);}

/* Procedure emitRO emits a register-only
 * TM instruction
 * op = the opcode
 * r = target register
 * s = 1st source register
 * t = 2nd source register
 * c = a comment to be printed if TraceCode is TRUE
 */
void emitRO( char *op, int r, int s, int t, char *c)
{ if (TraceAssembly) pc("%3d:  %5s  %d,%d,%d ",emitLoc++,op,r,s,t);
  if (TraceCode) pc("\t%s",c) ;
  pc("\n") ;
  if (highEmitLoc < emitLoc) highEmitLoc = emitLoc ;
} /* emitRO */

/* Procedure emitRM emits a register-to-memory
 * TM instruction
 * op = the opcode
 * r = target register
 * d = the offset
 * s = the base register
 * c = a comment to be printed if TraceCode is TRUE
 */
void emitRM( char * op, int r, int d, int s, char *c)
{ if (TraceAssembly) pc("%3d:  %5s  %d,%d(%d) ",emitLoc++,op,r,d,s);
  if (TraceCode) pc("\t%s",c) ;
  pc("\n") ;
  if (highEmitLoc < emitLoc)  highEmitLoc = emitLoc ;
} /* emitRM */

/* Function emitSkip skips "howMany" code
 * locations for later backpatch. It also
 * returns the current code position
 */
int emitSkip( int howMany)
{  int i = emitLoc;
   emitLoc += howMany ;
   if (highEmitLoc < emitLoc)  highEmitLoc = emitLoc ;
   return i;
} /* emitSkip */

/* Procedure emitBackup backs up to 
 * loc = a previously skipped location
 */
void emitBackup( int loc)
{ if (loc > highEmitLoc) emitComment("BUG in emitBackup");
  emitLoc = loc ;
} /* emitBackup */

/* Procedure emitRestore restores the current 
 * code position to the highest previously
 * unemitted position
 */
void emitRestore(void)
{ emitLoc = highEmitLoc;}

/* Procedure emitRM_Abs converts an absolute reference 
 * to a pc-relative reference when emitting a
 * register-to-memory TM instruction
 * op = the opcode
 * r = target register
 * a = the absolute location in memory
 * c = a comment to be printed if TraceCode is TRUE
 */
void emitRM_Abs( char *op, int r, int a, char * c)
{ if (TraceAssembly) pc("%3d:  %5s  %d,%d(%d) ", emitLoc,op,r,a-(emitLoc+1),PC);
  ++emitLoc ;
  if (TraceCode) pc("\t%s",c) ;
  pc("\n") ;
  if (highEmitLoc < emitLoc) highEmitLoc = emitLoc ;
} /* emitRM_Abs */