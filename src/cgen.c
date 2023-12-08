/****************************************************/
/* File: cgen.c                                     */
/* The code generator implementation                */
/* for the TINY compiler                            */
/* (generates code for the TM machine)              */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "symtab.h"
#include "code.h"
#include "cgen.h"

/* tmpOffset is the memory offset for temps
   It is decremented each time a temp is
   stored, and incremeted when loaded again
*/
static int tmpOffset = 0;
static char * ScopeNow = "";

/* prototype for internal recursive code generator */
static void cGen (TreeNode * tree);
static void genExp( TreeNode * tree);

/* Procedure genStmt generates code at a statement node */
static void genStmt( TreeNode * tree)
{ 
   TreeNode * p1, * p2, * p3;
            char * l1 = NULL;
         char * l2 = NULL;
  int savedLoc1,savedLoc2,currentLoc;
  int loc;
  switch (tree->kind.stmt) {

      case VarDecK:
      //emitLabel(tree->child[0]->attr.data.name);
         genExp(tree->child[0]); //leva pro node idk
         break;

      case FunDecK ://Tem que arurmar aqui para ele levar para o CALL ao inves do Dec
      if (TraceCode) {emitComment("-> FunDec") ;emitLabel(tree->child[0]->attr.data.name);}
      if(tree->child[0]->attr.data.name)
         ScopeNow = tree->child[0]->attr.data.name;
         loc = st_lookup(tree->child[0]->attr.data.name, "");
         emitRM("ST",ac,loc,gp,"assign: store return adress");

//tree->attr.data.name
                  //p1 = tree->child[0]->child[0];
                  p2 = tree->child[0]->child[1];
                  /* do nothing for p1 */
                  //cGen(p1); //Sao os argumentos da funcao
               cGen(p2);
         if (TraceCode) emitComment("<- FunDec") ;
                  
               
               break; /* decl_k */

      case IfK ://Creio OK
         if (TraceCode) emitComment("-> if") ;
         p1 = tree->child[0] ;
         p2 = tree->child[1] ;
         p3 = tree->child[2] ;

         /* generate code for test expression */
         genExp(p1);
         savedLoc1 = emitSkip(1) ;
         emitComment("if: jump to else belongs here");
         /* recurse on then part */
         l1 = getNewBranchLabel();
         l2 = getNewBranchLabel();
         cGen(p2);
         savedLoc2 = emitSkip(1) ;
         emitComment("if: jump to end belongs here");
         currentLoc = emitSkip(0) ;
         emitBackup(savedLoc1) ;
         
         emitRestore() ;

         
         /* recurse on else part */
         //emitBranchInstruction("name1", l1, TRUE);
         if (p3 != NULL) {
            int currentLoc = emitSkip(0);
            emitBackup(savedLoc1);
            emitRM_Abs("JEQ", ac, currentLoc, "If: jump to else");
            emitRestore();
            cGen(p3);
         }
         //emitBranchInstruction("", l2, FALSE);
         
         if (TraceCode)  emitLabel(l1);
         
         if (TraceCode)  emitLabel(l2);
         currentLoc = emitSkip(0) ;
         emitBackup(savedLoc2) ;
         emitRM_Abs("JEQ",PC,currentLoc,"if: jmp to end");
         emitRestore() ;
         if (TraceCode)  emitComment("<- if") ;
         break; /* if_k */

      case WhileK:// Creio que OK
         if (TraceCode) emitComment("-> while") ;

         p1 = tree->child[0] ;
         p2 = tree->child[1] ;
         savedLoc1 = emitSkip(0);
         emitComment("repeat: jump after body comes back here");
         l1 = getNewBranchLabel();
         l2 = getNewBranchLabel();
         /* generate code for body */
         if (TraceCode)  emitLabel(l1);
         genExp(p1);
         /* generate code for test */
         //emitBranchInstruction("reg1", l2, FALSE);
         cGen(p2);
         //emitBranchInstruction("", l1, FALSE);
         /* next instruction block */
         if (TraceCode)  emitLabel(l2);
         emitRM_Abs("JEQ",ac,savedLoc1,"repeat: jmp back to body");
         if (TraceCode)  emitComment("<- repeat") ;
         break; /* repeat */

      case AssignK: //Esse acho que nao OK. Para a arvore de p2, tenho que salvar o valor em algum lugar
         if (TraceCode) emitComment("-> assign") ;
         p1 = tree->child[0] ;
         p2 = tree->child[1] ;
         /* generate code for rhs */
         genExp(p1);
         genExp(p2);
         /* now store value */
         // loc = st_lookup(tree->attr.data.name, ScopeNow);
         // emitRM("ST",ac,loc,gp,"assign: store value");

         // cGen(p1);
         if(strcmp(tree->child[0]->attr.data.type, "array")) {
            loc = st_lookup(tree->child[0]->attr.data.name, ScopeNow);
            // arranjar um jeito de puxar o id
         } else {
            loc = st_lookup(tree->child[0]->attr.data.name, ScopeNow);
         }

         emitRM("ST",ac,loc,gp,"assign: store value");

         //emitAssignInstruction("", "reg1", "reg2", "");
         if (TraceCode)  emitComment("<- assign") ;
         break; /* assign_k */

      case ReturnK :
      if (TraceCode)  emitComment("-> Return") ;
        
         p1 = tree->child[0];
         if (p1 != NULL) {
            
            genExp(p1);
            //if (TraceCode)  emitReturn(gp);
            //emitRM("Return",ac,loc,gp,"return: store value");
            //emitReturnInstruction(t1);
         }
            emitRM("LDA", mp, -1, mp, "adjust fp");
            emitRM("LD", ac1, 0, mp, "load return address to ac1");
            // Pulando de volta para o chamador
            emitRM("LD", pc, 0, ac1, "return to caller");

         if (TraceCode)  emitComment("<- Return") ;
         break; /* decl_k */
         
/*
Acredito que o problema esta no return, o meu:
* -> Return
 19:    LDA  6,-1(6) 	adjust fp
 20:     LD  1,0(6) 	load return address to ac1
 21:     LD  856998529,0(1) 	return to caller
* <- Return
o que era para dar:
-> return
* -> Function Call (gdc)
 20:     ST  2,-4(2) 	Guard fp
* -> Id
 21:     LD  0,-3(2) 	load id value
* <- Id
 22:     ST  0,-6(2) 	Store value of func argument
* -> Op
* -> Id
 23:     LD  0,-2(2) 	load id value
* <- Id
 24:     ST  0,-7(2) 	op: push left
* -> Op
* -> Op
* -> Id
 25:     LD  0,-2(2) 	load id value
* <- Id
 26:     ST  0,-8(2) 	op: push left
* -> Id
 27:     LD  0,-3(2) 	load id value
* <- Id
 28:     LD  1,-8(2) 	op: load left
 29:    DIV  0,1,0 	op /
* <- Op
 30:     ST  0,-8(2) 	op: push left
* -> Id
 31:     LD  0,-3(2) 	load id value
* <- Id
 32:     LD  1,-8(2) 	op: load left
 33:    MUL  0,1,0 	op *
* <- Op
 34:     LD  1,-7(2) 	op: load left
 35:    SUB  0,1,0 	op -
* <- Op
 36:     ST  0,-7(2) 	Store value of func argument
 37:    LDA  2,-4(2) 	change fp
 38:    LDC  0,40(0) 	Guard return adress
 39:    LDA  7,-36(7) 	jump to function
* <- Function Call
 40:    LDA  1,0(2) 	save current fp into ac1
 41:     LD  2,0(2) 	make fp = ofp
 42:     LD  7,-1(1) 	return to caller
* <- return
*/

      case CallK: 
      if (TraceCode)  emitComment("-> FCall") ;
         p1 = tree->child[1];
         int countParams = 0;
         while( p1 != NULL ){
               genExp(p1);
               emitRM("ST", ac, tmpOffset--, mp, "Call: push argument");
               countParams++;
               //cGen(p1);
               //emitParamInstruction("reg/var");
               p1 = p1->sibling;
         }
         // Emita a instrução de chamada de função
         // Ajuste tmpOffset para acomodar o endereço de retorno
         tmpOffset -= 1;
         emitRM("LDA", mp, tmpOffset + 1, mp, "Call: store return address");

         // Ajuste tmpOffset para acomodar os parâmetros e o endereço de retorno
         tmpOffset -= countParams;

         // Emita uma instrução de salto para o endereço da função
         emitRM("LDA", pc, st_lookup(tree->child[0]->attr.data.name, ""), PC, "Call: jump to function");

         // Recupere o valor de retorno após a chamada da função
         tmpOffset += countParams;
         emitRM("LD", ac, tmpOffset + 1, mp, "Call: retrieve return value");

      if (TraceCode)  emitComment("<- FCALL") ;
      default:
         break;   
    }



} /* genStmt */



/* Procedure genExp generates code at an expression node */
static void genExp( TreeNode * tree)
{ 
   int loc;
   TreeNode * p1, * p2;
   char * name1, * name2, * name3;
   //Acho que uma fila/pilha aqui pra pegar os nomes talvez
   name1 = "reg1"; name2 = "reg2"; name3 = "reg3";
   switch (tree->kind.exp) {

      case ConstK ://OK
         //emitLabelInt(tree->attr.val);
         if (TraceCode) emitComment("-> Const") ;
         // pc("\n\nCONST\n\n");
         /* gen code to load integer constant using LDC */
         emitRM("LDC",ac,tree->attr.val,0,"load const");
         if (TraceCode)  emitComment("<- Const") ;
         break; /* ConstK */
      
      case IdK : //OK
         if (TraceCode) emitComment("-> Id") ;
         loc = st_lookup(tree->attr.data.name, ScopeNow);
         if(loc == -1) {
            loc = st_lookup(tree->attr.data.name, "");
         }
         // pc("\n\nNome variavel %s LOCALIZADA no LOC %d\n\n", tree->attr.data.name, loc);
         // if(!strcmp(tree->attr.data.type, "array")){
         //    if (TraceCode)  emitAssignInstruction("IdK", "reg", "exp", "4");
         // }
         emitRM("LD",ac,loc,gp,"load id value");
         if (TraceCode)  emitComment("<- Id") ;
         break; /* IdK */

      case OpK : //OK
            if (TraceCode) emitComment("-> Op") ;
            p1 = tree->child[0];
            p2 = tree->child[2];

            /* gen code for ac = left arg */
            if (TraceCode) emitComment("-> left") ;
            genExp(p1);
            if (TraceCode) emitComment("<- left") ;
            /* gen code to push left operand */
            emitRM("ST",ac,tmpOffset--,mp,"op: push left");
            /* gen code for ac = right operand */
            if (TraceCode) emitComment("-> right") ;
            genExp(p2);
            if (TraceCode) emitComment("<- right") ;
            /* now load left operand */
            emitRM("LD",ac1,++tmpOffset,mp,"op: load left");
            switch (tree->attr.op) {        
               case PLUS :  
                  emitRO("ADD",ac,ac1,ac,"op +"); 
                  if (TraceCode)  emitAssignInstruction("+",  name1,name2,name3);
                  break;
               case MINUS : 
                  emitRO("SUB",ac,ac1,ac,"op -"); 
                  if (TraceCode)  emitAssignInstruction("-",  name1,name2,name3);
                  break;
               case TIMES : 
                  emitRO("MUL",ac,ac1,ac,"op *"); 
                  if (TraceCode)  emitAssignInstruction("*",  name1,name2,name3);
                  break;
               case OVER :  
                  emitRO("DIV",ac,ac1,ac,"op /"); 
                  if (TraceCode)  emitAssignInstruction("/",  name1,name2,name3);
                  break;
               case EQ : 
                  emitRO("SUB",ac,ac1,ac,"op ==") ;
                  emitRM("JEQ",ac,2,PC,"br if true");
                  emitRM("LDC",ac,0,ac,"false case") ;
                  emitRM("LDA",PC,1,PC,"unconditional jmp") ;
                  emitRM("LDC",ac,1,ac,"true case") ;
                  if (TraceCode)  emitAssignInstruction("==",  name1,name2,name3);
                  break;
               case LT : 
                  emitRO("SUB",ac,ac1,ac,"op <") ;
                  emitRM("JLT",ac,2,PC,"br if true") ;
                  emitRM("LDC",ac,0,ac,"false case") ;
                  emitRM("LDA",PC,1,PC,"unconditional jmp") ;
                  emitRM("LDC",ac,1,ac,"true case") ;
                  if (TraceCode)  emitAssignInstruction("<",  name1,name2,name3); 
                  break;
               case LE : 
                  emitRO("SUB",ac,ac1,ac,"op <=") ;
                  emitRM("JLE",ac,2,PC,"br if true") ;
                  emitRM("LDC",ac,0,ac,"false case") ;
                  emitRM("LDA",PC,1,PC,"unconditional jmp") ;
                  emitRM("LDC",ac,1,ac,"true case") ;
                  if (TraceCode)  emitAssignInstruction("<=",  name1,name2,name3); 
                  break;
               case GT : 
                  emitRO("SUB",ac,ac1,ac,"op >") ;
                  emitRM("JGT",ac,2,PC,"br if true") ;
                  emitRM("LDC",ac,0,ac,"false case") ;
                  emitRM("LDA",PC,1,PC,"unconditional jmp") ;
                  emitRM("LDC",ac,1,ac,"true case") ;
                  if (TraceCode)  emitAssignInstruction(">",  name1,name2,name3);
                  break;
               case GE : 
                  emitRO("SUB",ac,ac1,ac,"op >") ;
                  emitRM("JGE",ac,2,PC,"br if true") ;
                  emitRM("LDC",ac,0,ac,"false case") ;
                  emitRM("LDA",PC,1,PC,"unconditional jmp") ;
                  emitRM("LDC",ac,1,ac,"true case") ;
                  if (TraceCode)  emitAssignInstruction(">=",  name1,name2,name3);
                  break;
               case NE : 
                  emitRO("SUB",ac,ac1,ac,"op >") ;
                  emitRM("JNE",ac,2,PC,"br if true") ;
                  emitRM("LDC",ac,0,ac,"false case") ;
                  emitRM("LDA",PC,1,PC,"unconditional jmp") ;
                  emitRM("LDC",ac,1,ac,"true case") ;
                  if (TraceCode)  emitAssignInstruction("!=",  name1,name2,name3); 
                  break;
               default:
                  emitComment("BUG: Unknown operator");
                  break;
               
            } /* case op */
            if (TraceCode)  emitComment("<- Op") ;
            break; /* OpK */
      default:
         break;
  }
} /* genExp */

/* Procedure cGen recursively generates code by
 * tree traversal
 */
static void cGen( TreeNode * tree)
{ 
   if (tree != NULL)
   { switch (tree->nodekind) {
         case StmtK:
         genStmt(tree);
         break;
         case ExpK:
         genExp(tree);
         break;
         default:
         break;
      }
   cGen(tree->sibling);
   }
}

/**********************************************/
/* the primary function of the code generator */
/**********************************************/
/* Procedure codeGen generates code to a code
 * file by traversal of the syntax tree. The
 * second parameter (codefile) is the file name
 * of the code file, and is used to print the
 * file name as a comment in the code file
 */


void codeGen(TreeNode * syntaxTree, char * codefile)
{  char * s = malloc(strlen(codefile)+7);
   strcpy(s,"File: ");
   strcat(s,codefile);
   emitComment("C- Compilation to 3 Address Intermediate Code");
   //emitComment(s);
   /* generate standard prelude */
   //emitComment("Standard prelude:");
   emitRM("LD",mp,0,ac,"load maxaddress from location 0");
   emitRM("LD",mp-4,0,ac,"load maxaddress from location 0");
   emitRM("ST",ac,0,ac,"clear location 0");
   //emitComment("End of standard prelude.");
   /* generate code for TINY program */
   cGen(syntaxTree);
   /* finish */
   emitComment("End of execution.");
   if (TraceCode)  emitHalt() ;
   emitRO("HALT",0,0,0,"");
}