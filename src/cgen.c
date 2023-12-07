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
static char * ScopeNow;

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
         cGen(tree->child[0]); //leva pro node idk
         break;

      case FunDecK :
         ScopeNow = tree->child[0]->attr.data.name;
         emitLabel(tree->child[0]->attr.data.name); //tree->attr.data.name
                  //p1 = tree->child[0]->child[0];
                  p2 = tree->child[0]->child[1];
                  /* do nothing for p1 */
                  //cGen(p1); //Sao os argumentos da funcao
                  cGen(p2);
                  
               
               break; /* decl_k */

      case IfK :
         if (TraceCode) emitComment("-> if") ;
         p1 = tree->child[0] ;
         p2 = tree->child[1] ;
         p3 = tree->child[2] ;

         /* generate code for test expression */
         cGen(p1);
         savedLoc1 = emitSkip(1) ;
         emitComment("if: jump to else belongs here");
         /* recurse on then part */
         l1 = getNewBranchLabel();
         l2 = getNewBranchLabel();
         //cGen(p2);
         savedLoc2 = emitSkip(1) ;
         emitComment("if: jump to end belongs here");
         currentLoc = emitSkip(0) ;
         emitBackup(savedLoc1) ;
         //emitRM_Abs("JEQ",ac,currentLoc,"if: jmp to else");
         emitRestore() ;
         /* recurse on else part */
         emitBranchInstruction("name1", l1, TRUE);
         if (p3 != NULL) cGen(p3);
         emitBranchInstruction("", l2, FALSE);
         emitLabel(l1);
         cGen(p2);
         emitLabel(l2);
         currentLoc = emitSkip(0) ;
         emitBackup(savedLoc2) ;
         //emitRM_Abs("LDA",PC,currentLoc,"jmp to end") ;
         emitRestore() ;
         if (TraceCode)  emitComment("<- if") ;
         break; /* if_k */

      case WhileK:
         if (TraceCode) emitComment("-> while") ;

         p1 = tree->child[0] ;
         p2 = tree->child[1] ;
         savedLoc1 = emitSkip(0);
         emitComment("repeat: jump after body comes back here");
         l1 = getNewBranchLabel();
         l2 = getNewBranchLabel();
         /* generate code for body */
         emitLabel(l1);
         cGen(p1);
         /* generate code for test */
         emitBranchInstruction("reg1", l2, FALSE);
         cGen(p2);
         emitBranchInstruction("", l1, FALSE);
         /* next instruction block */
         emitLabel(l2);
         //emitRM_Abs("JEQ",ac,savedLoc1,"repeat: jmp back to body");
         if (TraceCode)  emitComment("<- repeat") ;
         break; /* repeat */

      case AssignK:
         if (TraceCode) emitComment("-> assign") ;
         p1 = tree->child[0] ;
         p2 = tree->child[1] ;
         /* generate code for rhs */
         cGen(p1);
         /* now store value */
         loc = st_lookup(tree->attr.data.name, ScopeNow);
         //emitRM("ST",ac,loc,gp,"assign: store value");
         cGen(p2);
         emitAssignInstruction("", "reg1", "reg2", "");
         if (TraceCode)  emitComment("<- assign") ;
         break; /* assign_k */

      case ReturnK :
         p1 = tree->child[0];
         if (p1 != NULL) {
            
            genExp(p1);
            emitReturn(gp);
            //emitRM("Return",ac,loc,gp,"return: store value");
            //emitReturnInstruction(t1);
         }
         break; /* decl_k */
         
      case CallK: 
         p1 = tree->child[1];
         int countParams = 0;
         while( p1 != NULL ){
               countParams++;
               cGen(p1);
               emitParamInstruction("reg/var");
               p1 = p1->sibling;
         }
         emitCallInstruction("nomeReg", tree->attr.data.name, countParams);
         //t1 = getNewVariable();

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

    case ConstK :
      //emitLabelInt(tree->attr.val);
      if (TraceCode) emitComment("-> Const") ;
      /* gen code to load integer constant using LDC */
      //emitRM("LDC",ac,tree->attr.val,0,"load const");
      if (TraceCode)  emitComment("<- Const") ;
      break; /* ConstK */
    
    case IdK :
      if (TraceCode) emitComment("-> Id") ;
      loc = st_lookup(tree->attr.data.name, ScopeNow);
      //pc("Nome variavel %c", tree->attr.data.name);
      p1 = tree->child[0];
      (p1);
      //emitRM("LD",ac,loc,gp,"load id value");
      if (TraceCode)  emitComment("<- Id") ;
      break; /* IdK */

    case OpK :
          if (TraceCode) emitComment("-> Op") ;
         p1 = tree->child[0];
         p2 = tree->child[1];
         /* gen code for ac = left arg */
         
         /* gen code to push left operand */
         //emitRM("ST",ac,tmpOffset--,mp,"op: push left");
         /* gen code for ac = right operand */
         cGen(p1);
         cGen(p2);
         /* now load left operand */
         //emitRM("LD",ac1,++tmpOffset,mp,"op: load left");
         switch (tree->attr.op) {        
            case PLUS : emitAssignInstruction("+", name1,name2,name3); break;
            case MINUS : emitAssignInstruction("-",  name1,name2,name3); break;
            case TIMES : emitAssignInstruction("*",  name1,name2,name3); break;
            case OVER :emitAssignInstruction("/",  name1,name2,name3); break;
            case EQ : emitAssignInstruction("==",  name1,name2,name3); break;
            case LT : emitAssignInstruction("<",  name1,name2,name3); break;
            case LE : emitAssignInstruction("<=",  name1,name2,name3); break;
            case GT : emitAssignInstruction(">",  name1,name2,name3); break;
            case GE : emitAssignInstruction(">=",  name1,name2,name3); break;
            case NE : emitAssignInstruction("!=",  name1,name2,name3); break;
            


            /* emitRO("SUB",ac,ac1,ac,"op <") ;
               emitRM("JLT",ac,2,PC,"br if true") ;
               emitRM("LDC",ac,0,ac,"false case") ;
               emitRM("LDA",PC,1,PC,"unconditional jmp") ;
               emitRM("LDC",ac,1,ac,"true case") ;
               //break;
            case EQ :
           emitRO("SUB",ac,ac1,ac,"op ==") ;
               emitRM("JEQ",ac,2,PC,"br if true");
               emitRM("LDC",ac,0,ac,"false case") ;
               emitRM("LDA",PC,1,PC,"unconditional jmp") ;
               emitRM("LDC",ac,1,ac,"true case") ;
               break;*/
            default:
               emitComment("BUG: Unknown operator");
               break;
            
         } /* case op */
         if (TraceCode)  emitComment("<- Op") ;
         break; /* OpK */

         
            //emitCallInstruction(t1, tree->attr.id.name, countParams);

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
   emitRM("ST",ac,0,ac,"clear location 0");
   //emitComment("End of standard prelude.");
   /* generate code for TINY program */
   cGen(syntaxTree);
   /* finish */
   emitComment("End of execution.");
   emitHalt() ;
   emitRO("HALT",0,0,0,"");
}