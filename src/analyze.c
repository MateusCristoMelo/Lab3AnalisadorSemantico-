/****************************************************/
/* File: analyze.c                                  */
/* Semantic analyzer implementation                 */
/* for the TINY compiler                            */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "symtab.h"
#include "analyze.h"
#include "util.h"

extern struct Queue var_or_array_stack;
extern struct Stack _Scope;

/* counter for variable memory locations */
static int location = 0;

/* Procedure traverse is a generic recursive 
 * syntax tree traversal routine:
 * it applies preProc in preorder and postProc 
 * in postorder to tree pointed to by t
 */
static void traverse( TreeNode * t, void (* preProc) (TreeNode *), void (* postProc) (TreeNode *) )
{ if (t != NULL)
  { 
    preProc(t);
    { int i;
      for (i=0; i < MAXCHILDREN; i++)
        traverse(t->child[i],preProc,postProc);
    }
    postProc(t);
    traverse(t->sibling,preProc,postProc);
  }
}

/* nullProc is a do-nothing procedure to 
 * generate preorder-only or postorder-only
 * traversals from traverse
 */
static void nullProc(TreeNode * t)
{ if (t==NULL) return;
  else return;
}

static void typeError(TreeNode * t, char * message)
{ 
  if(!strcmp(message, "undefined reference to 'main'")) 
  {
    TraceAnalyze = FALSE;
    pce("Semantic error: %s\n",message);
  }
  else if(!strcmp(message, "was not declared in this scope") || !strcmp(message, "was already declared as a variable") || !strcmp(message, "was already declared as a function")) 
  {
    TraceAnalyze = FALSE;
    pce("Semantic error at line %d: '%s' %s\n",t->lineno, t->attr.name, message);
  }
  else
  {
    pce("Semantic error at line %d: %s\n",t->lineno,message);
  }
  Error = TRUE;
}

char* Token2Char(TokenType token)
{ switch (token)
  { 
    case VOID: return "void";
    case INT: return "int"; 
    default: /* should never happen */ pc("Unknown token: %d\n",token);
  }
}

char* Val2Char(int value)
{ if(value > 0)
  return "int";
  else
  return "void";
}

char *Scope = NULL;
char *var_type= NULL;

/* Procedure insertNode inserts 
 * identifiers stored in t into 
 * the symbol table 
 */
static void insertNode(TreeNode * t) //alterar essa
{
  // pc("\n\nINSERT NODE\n\n");
  switch (t->nodekind)
  { case StmtK:
      switch (t->kind.stmt)
      { 
        case VarDecK: 
          // pc("\n\n%s VarDecK xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n\n", copyString(t->child[0]->attr.name));
          // pc("\n\n%s (0) CHAMA INSERT\n\n", t->child[0]->attr.name);
          
          if (st_lookup(t->child[0]->attr.name, Scope) == -1){  
            if (st_lookup(t->child[0]->attr.name, "") == -1){
              st_insert(t->child[0]->attr.name, t->child[0]->lineno, location++, Scope, (char*)dequeue(&var_or_array_stack), Token2Char(t->attr.op));
            } else {
              if (!strcmp(id_lookup(t->child[0]->attr.name, ""), "var")) {
                typeError(t->child[0], "was already declared as a variable");
              } else {
                typeError(t->child[0], "was already declared as a function");
              }
              t->child[0]->attr.name = NULL;
            }
            
          } else {            
            if (!strcmp(id_lookup(t->child[0]->attr.name, Scope), "var")) {
                typeError(t->child[0], "was already declared as a variable");
              } else {
                typeError(t->child[0], "was already declared as a function");
              }
            t->child[0]->attr.name = NULL;
          }
          break; //tratado em idk

        case FunDecK:
          // pc("\n\n%s FunDecK xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n\n", copyString(t->attr.name));
          if (st_lookup(t->child[0]->attr.name, "") == -1){
            /* not yet in table, so treat as new definition */
            Scope = t->child[0]->attr.name;
            push(&_Scope, Scope);
            // if(!strcmp(t->child[0]->attr.name, "main")) {
            //   pc("\n\nFLAG\n\nLINE PARENT: %d\nLINE CHILD: %d\n\n", t->child[0]->lineno,t->lineno);
            // }
            // pc("\n\n%s (1) CHAMA INSERT\n\n", t->child[0]->attr.name);
            st_insert(t->child[0]->attr.name, t->child[0]->lineno, location++, "" , "fun", Token2Char(t->attr.op));
          } else {
            /* already in table, so ignore location, 
            add line number of use only */ 
            // pc("\n\n%s (2) CHAMA INSERT\n\n", t->child[0]->attr.name);
            // st_insert(t->child[0]->attr.name, t->child[0]->lineno, 0, "" , "", "");
            typeError(t->child[0], "was already declared as a function");
            t->child[0]->attr.name = NULL;
          }
          break;

        case CallK:
          // pc("\n\n%s CallK xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n\n", copyString(t->attr.name));
          if (st_lookup(t->attr.name, "") == -1) {
            /* not yet in table, so treat as new definition */
              //Scope = t->attr.name;
              // pc("OI");
              // pc("\n\n%s (3) CHAMA INSERT NA LINHA %d\n\n", t->attr.name, t->lineno);
              typeError(t,"was not declared in this scope");
              // t->attr.name = NULL;
              // st_insert(t->attr.name, t->lineno, location++, "" , "fun", "int");
            } else {
              /* already in table, so ignore location, 
              add line number of use only */
              // pc("\n\n%s (4) CHAMA INSERT\n\n", t->attr.name);
              st_insert(t->attr.name, t->lineno, 0, "" , "", "");
            }
          break;

        default:
          break;
      }
      break;
    case ExpK:
      // pc("\n\n%s ExpK xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n\n", copyString(t->attr.name));
      switch (t->kind.exp)
      { 
        case IdK:
          // pc("My ID is %s\n", t->attr.name);
          if(Scope == t->attr.name || t->attr.name == NULL) {
            break;
          }
          if ((st_lookup(t->attr.name, Scope) == -1)) {
            /* not yet in table, so treat as new definition */
            // pc("\n\n%s (5) CHAMA INSERT\n\n", t->attr.name);
            typeError(t,"was not declared in this scope");
            // st_insert(t->attr.name, t->lineno, location++, Scope, (char*)dequeue(&var_or_array_stack), var_type);
          } else {
            /* already in table, so ignore location, 
              add line number of use only */
            // pc("\n\n%s (6) CHAMA INSERT NA LINHA %d\n\n", t->attr.name, t->lineno);
            st_insert(t->attr.name, t->lineno, 0, Scope , "", "");
          }
          break;

        case ConstK:
          break; //o valor da variavel nao entra na tabela

        case OpK:
          break; //operador nao entra na tabela

        default:
          break;
      }
      break;
    default:
      break;
  }
}

// static char * currentScope;

/* Function buildSymtab constructs the symbol 
 * table by preorder traversal of the syntax tree
 */
void buildSymtab(TreeNode * syntaxTree)
{ 
  // insertBuiltinFunctions();
  st_insert("input", 0, 0, "" ,"fun", "int");
  st_insert("output", 0, 0, "" ,"fun", "void");
  // pc("\n\nSTACK\n\n");
  // while (!isEmpty(&var_or_array_stack)) {
  //   pc("\n\n%-7s\n\n",pop(&var_or_array_stack));
  // }
  /* Traversing the sintax tree */
  traverse(syntaxTree,insertNode,nullProc);
  pc("\nSymbol table:\n\n");
  printSymTab();
  pc("\nChecking for main...\n");
  if (st_lookup("main", "") == -1)
  {
    typeError(syntaxTree, "undefined reference to 'main'");
  }
}

// static void print_node(TreeNode * tree) {
//   if (tree->nodekind==StmtK)
//   { switch (tree->kind.stmt) {
//       case IfK: pc("If\n"); break;
//       case WhileK: pc("While\n"); break;
//       case AssignK: pc("Assign:\n"); break;
//       case ReturnK: pc("Return\n"); break;
//       case CallK: pc("Activation: %s\n", tree->attr.name); /*printTokenSyn(tree->attr.name,"\0"); */break;
//       case VarDecK: pc("Type: ");printTokenSyn(tree->attr.op,"\0"); break;
//       case FunDecK: pc("Type: ");printTokenSyn(tree->attr.op,"\0"); break;
//       default: pc("Unknown ExpNode kind\n"); break;
//     }
//   }
//   else if (tree->nodekind==ExpK)
//   { switch (tree->kind.exp) {
//       case OpK: pc("Op: "); printTokenSyn(tree->attr.op,"\0"); break;
//       case ConstK: pc("Const: %d\n",tree->attr.val); break;
//       case IdK: if(tree->attr.name != NULL) pc("Id: %s\n",tree->attr.name); break;
//       default: pc("Unknown ExpNode kind\n"); break;
//     }
//   }
//   else pc("Unknown node kind\n");
// }

/* Procedure checkNode performs
 * type checking at a single tree node
 */
static void checkNode(TreeNode * t) //alterar essa
{
  switch (t->nodekind)
  { case ExpK:
      switch (t->kind.exp)
      { case OpK:
          if ((t->child[0]->type != Integer) || (t->child[2]->type != Integer))
            // typeError(t,"Op applied to non-integer");
          if ((t->attr.op == EQ) || (t->attr.op == LT))
            t->type = Boolean;
          else
            t->type = Integer;
          break;
        case ConstK:
        case IdK:
          // if (var_type != Token2Char(INT))
          //   // typeError(t,"variable declared void");
          break;
        default:
          break;
      }
      break;
    case StmtK:
      switch (t->kind.stmt)
      { case IfK:
          // if (t->child[0]->type == Integer)
            // typeError(t->child[0],"if test is not Boolean");
          break;
        case AssignK:
          if (t->child[1]->kind.stmt == CallK) {
            // pc("\n\nFIRST: %s \n\n", t->child[0]->attr.name);
            // pc("\n\nSECOND: %s \n\n", t->child[1]->attr.name);
            // pc("\n\nSECOND: %s \n\n", t->child[1]->attr.op);
            // pc("\n\nCALL NO SCOPE: %s \n\n", Scope);
            // pc("\n\n%s %s\n\n", t->child[1]->attr.name, Scope);
            if (!strcmp(type_lookup(t->child[1]->attr.name, ""), "void")) {
              typeError(t, "invalid use of void expression");
            }
          }
            // typeError(t->child[0],"assignment of non-integer value");
          break;
        case WhileK:
          // if (t->child[0]->type == Integer)
            // typeError(t->child[0],"while has no right parameters");
          break;
        case ReturnK:
          // if (t->child[0]->type == Integer)
          //  typeError(t->child[0],"return of function isnt correct");
        case CallK:
          break;
        case VarDecK:
          // pc("\n\nVARIAVEL DECLARADA NO ESCOPO %s NA LINHA %d\n\n", Scope, t->lineno);
          // print_node(t);
          if((t->kind.stmt != WhileK) && (t->kind.stmt != ReturnK)) {
            if(t->attr.op != INT) {
              typeError(t->child[0], "variable declared void");
            }
          }
          // if (!strcmp(id_lookup(t->child[0]->attr.name, ""), "var")) {
          //   pc("\n\nVARIAVEL %s DECLARADA NO ESCOPO %s\n\n", t->child[0], Scope);
          // }
            // pc("\n\nVARIAVEL %s DECLARADA NO ESCOPO %s\n\n", t->child[0], Scope);
          // if (!strcmp(type_lookup(t->child[0]->attr.name, Scope), "void")) {
          //   typeError(t->child[0], "variable declared void");
          // }
          // if (t->attr.op == Void) {
          //   typeError(t->child[0], "variable declared void");
          // }
          break;
        case FunDecK:
          // pc("\n\nMUDANCA DE SCOPE, saindo do atual: %s\n", Scope);
          if(_Scope.top == -1) {
            Scope = NULL;  
          } else {
            Scope = _Scope.items[_Scope.top];
            pop(&_Scope);
          }
          // pc("Novo SCOPE: %s\n\n", Scope);
          // if (t->child[0]->attr.name != NULL) {
          //   if (t->child[0]->attr.name == "main") {
          //       hasMain = 1;  // Marcar que a função main foi encontrada
          //   }
          // }
          break;
        default:
          break;
      }
      break;
    default:
      break;

  }
}


/* Procedure typeCheck performs type checking 
 * by a postorder syntax tree traversal
 */
void typeCheck(TreeNode * syntaxTree)
{ 
  traverse(syntaxTree,nullProc,checkNode);
}