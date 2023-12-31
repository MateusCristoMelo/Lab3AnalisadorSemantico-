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
static int memorySize = 0;

static int locFunction = 0;
static char comment[128];

/* prototype for internal recursive code generator */
static void cGen (TreeNode * tree);
static void genExp( TreeNode * tree);

/* Procedure genStmt generates code at a statement node */
static void genStmt( TreeNode * tree)
{ 
   TreeNode * p1, * p2, * p3;
   char * l1 = NULL;
   char * l2 = NULL;
   int savedLoc1, savedLoc2, currentLoc;
   int loc;
   if (tree == NULL)
         return;
   switch (tree->kind.stmt) {

      case VarDecK:
         /* 
          *  Todo VarDecK é seguido de IdK
          *  Nao eh preciso fazer nada porque ja existe espaco no gp para TODA DECLARACAO
          *  Basta imprimir o loc no symtab, cada var_dec_k ja tem um gp reservado, assim como cada fun_dec_k
          */

         sprintf(comment, "-> VarDec %s ", tree->child[0]->attr.data.name);
         if(TraceCode) emitComment(comment);
         
         sprintf(comment, "<- VarDec %s ", tree->child[0]->attr.data.name);
         if(TraceCode) emitComment(comment);
         break; /* var_dec_k */

         // int tmpSize = 1;
         // NEED TO SUM THE SIZE OF ARRAY TO ALLOCATE PROPERLY
         // if(!strcmp(tree->child[0]->attr.data.type, "array"))
         // {
         //    --tmpSize;
         //    tmpSize += p1->child[0]->child[0]->attr.val;
         // }
         // emitRM("LDC", ac1, tmpSize, 0, "ac1 = sum of size of local variables");         
         // emitRO("SUB", mp, mp, ac1, "allocate local variables");
         // emitRM("LDC", ac1, tmpSize, 0, "ac1 = sum of size of local variables");
         // emitRO("ADD", mp, mp, ac1, "free local variable");
         // genExp(tree->child[0]); //leva pro node idk

      case FunDecK:
         /* 
          *  FunDeck guarda IdK em child[0]
          *  Parameters em child[0]->child[0]
          *  Statements em child[0]->child[1]
          *  Averiguar espaço na memória para params, mas não fazer nada ainda
          */
         
         sprintf(comment, "-> FunDec %s ---------------------------", tree->child[0]->attr.data.name);
         if(TraceCode) emitComment(comment);

         savedLoc1 = emitSkip(0);
         if(strcmp(tree->child[0]->attr.data.name, "input") && strcmp(tree->child[0]->attr.data.name, "output") )
         {
            ScopeNow = tree->child[0]->attr.data.name;
         }
         emitBackup(locFunction);

         // confere se esta na main, caso positivo ent insere comando inicial de jump main
         if(!strcmp(tree->child[0]->attr.data.name, "main")) 
         {
            emitRM("LDC", PC, savedLoc1, 0, "jump to main"); // salto incondicional
         }
         else {
            // HANDLE FUNCTION POINTER IN THE GP TABLE
            loc = st_lookup(tree->child[0]->attr.data.name, "");
            sprintf(comment, "function %s is at %d", tree->child[0]->attr.data.name, loc);
            
            if (TraceCode) emitComment(comment);
            sprintf(comment, "load function location (%d)", savedLoc1);
            
            emitRM("LDC", ac1, savedLoc1, 0, comment); // carrega o acumulador com o loc de instrucao
            emitRM("ST", ac1, loc, gp, "add into memory"); // salva em qual instrucao pular quando aparecer um call no gp referente a funcao
         }

         locFunction += 2;
         emitRestore();

         // COPY RETURN ADDRESS TO THE STACK
         if(strcmp(tree->child[0]->attr.data.name, "main")) 
         {
            // salva Old Frame Pointer na MEMORY
            emitRM("LDA",mp,-1,mp,"mp = mp - 1");
            emitRM("ST", fp, 0, mp, "save old fp to mp"); // emitRM("ST", fp, --memorySize, mp, "save old fp to mp");
            // atualiza o Frame Pointer
            emitRM("LDA", fp, 0, mp, "update fp with mp of old fp"); // emitRM("LDA", fp, memorySize, mp, "update fp with mp of old fp");
            // da um store do return address presente no acumulador
            emitRM("LDA",mp,-1,mp,"mp = mp - 1");
            emitRM("ST", ac, 0, mp, "store return address from ac"); // emitRM("ST", ac, --memorySize, mp, "store return address from ac");
         }

         // HANDLE PARAMETERS
         // talvez seja uma boa contar os parametros para dar shift no espaço da memory
         p1 = tree->child[0]->child[0];
         int count_params = 0;
         while( p1 != NULL )
         {
            if (TraceCode) emitComment("-> params") ;
            
            count_params++;
            
            // if(!strcmp(tree->child[0]->attr.data.type, "array"))
            // {
            //    emitRM("LD", ac1, (loc+count_params), gp, "get variable from global");
            // }
            // else
            // {
               emitRM("LD", ac1, (loc+count_params), gp, "get variable from global");
            // }

            sprintf(comment, "variable %s pushed to stack", p1->child[0]->attr.data.name);
            emitRM("LDA",mp,-1,mp,"mp = mp - 1");
            emitRM("ST", ac1, 0, mp, comment); // emitRM("ST", ac1, --memorySize, mp, comment);
            
            p1 = p1->sibling;
            
            if (TraceCode) emitComment("<- params") ;
         }

         // HANDLE STATEMENTS
         p2 = tree->child[0]->child[1];
         cGen(p2);
         
         sprintf(comment, "<- FunDec %s ---------------------------", tree->child[0]->attr.data.name);
         if(TraceCode) emitComment(comment);
         break; /* fun_dec_k */

         // HANDLE RETURN ADDRESS, IF ITS MAIN JUST LET IT FINISHES
         // if(strcmp(tree->child[0]->attr.data.name, "main")) {
         //    // emitRM("LDA", mp, 0, fp, "copy fp to mp");
         //    // emitRM("LD", fp, 0, mp, "pop fp");
         //    // emitRM("LDC", ac1, 1, 0, "ac1 = 1");
         //    // emitRO("ADD", mp, mp, ac1, "mp = mp + ac1 = mp + 1");
         //    // emitRM("LD", PC, -2, mp, "jump to return address");
         // }

         // // HANDLE INPUT/OUTPUT
         // if(!strcmp(tree->child[0]->attr.data.name, "input")) 
         // {
         //    emitRO("IN",ac,0,0,"read integer value");
         // }
         // else
         // {
         //    // // HANDLE PARAMETERS
         //    // // talvez seja uma boa contar os parametros para dar shift no espaço da memory
         //    // p1 = tree->child[0]->child[0];
         //    // int count_params = 0;
         //    // while( p1 != NULL )
         //    // {
         //    //    if (TraceCode) emitComment("-> params") ;
         //    //    // PQ VC TA DIMINUINDO O OFFSET DEPOIS?
         //    //    // emitRM("ST", ac, tmpOffset--, mp, "Call: push argument");
         //    //    count_params++;
         //    //    p1 = p1->sibling;
         //    //    if (TraceCode) emitComment("<- params") ;
         //    // }
            
         //    if(!strcmp(tree->child[0]->attr.data.name, "output"))
         //    { 
         //       // // IMPORTANTE LEMBRAR DE CARREGAR OS ARGUMENTOS NO ACUMULADOR
         //       // emitRM("LD", ac, 1, fp, "load first argument");
         //       /* now output it */
         //       emitRO("OUT",ac,0,0,"write ac");
         //    }
         //    else
         //    {
         //       // HANDLE STATEMENTS
         //       p2 = tree->child[0]->child[1];
         //       cGen(p2);
         //    }
         // }
         
         // if(tree->child[0]->attr.data.name)
         //    ScopeNow = tree->child[0]->attr.data.name;
         //    loc = st_lookup(tree->child[0]->attr.data.name, "");
         //    emitRM("ST",ac,loc,gp,"assign: store return adress");
         //             //tree->attr.data.name
         //             //p1 = tree->child[0]->child[0];
         //             p2 = tree->child[0]->child[1];
         //             /* do nothing for p1 */
         //             //cGen(p1); //Sao os argumentos da funcao
         //          cGen(p2);
         // if (TraceCode) emitComment("<- FunDec") ;

      case IfK ://Creio OK
         /* 
          *  savedLoc1: se o valor de p1 for zero (ele ta passando oq ta no acumulador), ou seja, FALSE, ent jump para ++savedLoc2
          *  (instrucoes do que acontece se p1 for 1, ou seja, TRUE)
          *  savedLoc2: se chegou aqui ent PC pula direto para depois das instrucoes do ELSE
          *  (instrucoes do que acontece se p1 for 0, ou seja, FALSE)
          *  currentLoc (final): PC deveria acabar todo IF stmt aqui
          */

         if (TraceCode) emitComment("-> if") ;
         p1 = tree->child[0] ;
         p2 = tree->child[1] ;
         p3 = tree->child[2] ;

         /* generate code for test expression */
         genExp(p1); // calcula o que vai gerar daqui e armazena em algum lugar
         savedLoc1 = emitSkip(1); // pula uma instrução do assembly e salva uma antes do inicio do que acontece se IF for TRUE
         emitComment("if: jump to else belongs here");
         
         l1 = getNewBranchLabel();
         l2 = getNewBranchLabel();
         
         /* recurse on then part */
         cGen(p2); // gera as instruções do do caso IF
         savedLoc2 = emitSkip(1); // pula uma instrução do assembly e salva a numeracao da ultima instrucao de caso IF for TRUE
         emitComment("if: jump to end belongs here");
         
         currentLoc = emitSkip(0); // salva exatamente o pós statement se o IF for TRUE (primeira branch)
         emitBackup(savedLoc1); // volta para a expressao que decide o IF
         // VERIFICAR ESSE JEQ
         emitRM_Abs("JEQ",ac,currentLoc,"if: jmp to else"); // se o aumulador (que deveria ser o resultado de genExp(p1)) for 0, ou seja, FALSE, ent pule o IF (só que ta escrevendo isso na instrucao savedLoc1)
         emitRestore() ; // essencialmente volta para savedLoc2

         if (TraceCode)  emitLabel(l1);
         cGen(p3); // gera o que aconteceria no caso ELSE
         currentLoc = emitSkip(0); // grava o final do ELSE
         emitBackup(savedLoc2); //volta para o savedLoc2 (se tudo isso aqui estivesse dentro do IF tu n poderia ter dado skip(1) quando salva o savedloc2)
         emitRM_Abs("LDA",PC,currentLoc,"if: jmp to end");
         emitRestore() ;

         if (TraceCode)  emitLabel(l2);
         
         if (TraceCode)  emitComment("<- if") ;
         break; /* if_k */

         /* recurse on else part */
         //emitBranchInstruction("name1", l1, TRUE);
         // if (p3 != NULL) {
         //    genStmt(p3);
         //    int currentLoc = emitSkip(0);

         //    emitBackup(savedLoc1);
         //    emitRM_Abs("JEQ", ac, currentLoc, "If: jump to else");
         //    emitRestore();
            
         // }
         //emitBranchInstruction("", l2, FALSE);

      case WhileK:// Creio que OK
         /*
          *  savedLoc1: n teve skip, ent essencialmente é a primeira instrução do cálculo de p1
          *  (calculo de p1)
          *  savedLoc2: se o valor que ta no acumulador for zero, ou seja, FALSE, pule para currentLoc
          *  (instrucoes do que acontece se o while for TRUE)
          *  currentLoc: final de tudo
          */

         if (TraceCode) emitComment("-> while") ;

         p1 = tree->child[0] ;
         p2 = tree->child[1] ;

         savedLoc1 = emitSkip(0);
         emitComment("repeat: jump after body comes back here");
         
         l1 = getNewBranchLabel();
         l2 = getNewBranchLabel();
         /* generate code for body */
         if (TraceCode)  emitLabel(l1);
         if (TraceCode) emitComment("while : test expression start");
         genExp(p1);
         if (TraceCode) emitComment("while : test expression end");
         /* generate code for test */
         //emitBranchInstruction("reg1", l2, FALSE);
         
         savedLoc2 = emitSkip(1);
         if (TraceCode) emitComment("while : body start");
         cGen(p2);
         if (TraceCode) emitComment("while : body end");
         //emitBranchInstruction("", l1, FALSE);
         /* next instruction block */
         if (TraceCode)  emitLabel(l2);
         
         // salto incondicional, professor fala para usar LDA, ent seria bom trocar...
         // aqui depois de executar p2, ele precisa recalcular p1 pra entender se houve alguma mudança na condicao, por isso ele volta para savedLoc1
         emitRM("LDC", PC, savedLoc1, 0, "unconditional jump");
         // emitRM_Abs("LDA",PC,savedLoc1,"unconditional jump"); // uma ideia é usar alguma operacao para puxar o currentLoc, pega pelo skip(0)

         // Volta para savedLoc2 e assinala instrucao de branch
         currentLoc = emitSkip(0);
         emitBackup(savedLoc2);
         emitRM_Abs("JEQ",ac,currentLoc,"while : false");
         // emitRM_Abs("JEQ",ac,savedLoc1,"repeat: jmp back to body");
         emitRestore();
         
         if (TraceCode)  emitComment("<- repeat") ;
         
         break; /* while_k */

      case AssignK: // PROBLEMA É QUE PRECISA USAR fp
         /* 
          *  AssignK soh aparece na arvore no formato var ASSIGN expresao
          *  Ou seja, P1 é necessariamente um IdK, NAO PRECISA CHAMAR P1, é só armazenar na memória onde ele tá
          */

         if (TraceCode) emitComment("-> assign") ;
         p1 = tree->child[0] ;
         p2 = tree->child[1] ;
         
         /* generate code for rhs */
         // genExp(p1);
         if (TraceCode) emitComment("-> generate code for rhs") ;
         cGen(p2);
         if (TraceCode) emitComment("<- generate code for rhs end") ;
         
         int scope_loc = 0;
         if(strcmp(ScopeNow, "") && strcmp(ScopeNow, "main"))
         {
            scope_loc = st_lookup(ScopeNow, "");
            loc = st_lookup(tree->child[0]->attr.data.name, ScopeNow);
            int offset = -1+(scope_loc-loc);
            
            if(!strcmp(tree->child[0]->attr.data.type, "array"))
            {
               if (TraceCode) emitComment("-> Array");
               
               emitRM("LDA",mp,-1,mp,"mp = mp - 1");
               emitRM("ST",ac,0,mp,"assign: push result val"); 
               
               // abre expressao dentro do indice e puxa no acumulador
               genExp(tree->child[0]->child[0]);
               emitRM("LD",ac1,offset,fp,"load id value of parameter stack");
               emitRO("ADD",ac,ac1,ac,"load id array value");

               emitRM("LD",ac1,0,mp,"assign: load result val");
               emitRM("LDA",mp,1,mp,"mp = mp + 1");

               emitRM("ST",ac1,0,ac,"assign: store array value");
               if (TraceCode) emitComment("<- Array");
            }
            else
            {
               emitRM("ST",ac,offset,fp,"assign stack: store value");
            }
         }
         else
         {
            loc = st_lookup(tree->child[0]->attr.data.name, ScopeNow);
            // if (loc == -1) {
            //    loc = st_lookup(tree->child[0]->attr.data.name, "");
            // }
            
            if(!strcmp(tree->child[0]->attr.data.type, "array"))
            {
               if (TraceCode) emitComment("-> Array");
               
               emitRM("LDA",mp,-1,mp,"mp = mp - 1");
               emitRM("ST",ac,0,mp,"assign: push result val"); 
               
               // emitRM("LDA",ac1,0,ac,"save ac value");
               // abre expressao dentro do indice e puxa no acumulador
               genExp(tree->child[0]->child[0]);
               emitRM("LDA",ac,loc,ac,"find id array address");
               emitRO("ADD",ac,ac,gp,"load array address");

               emitRM("LD",ac1,0,mp,"assign: load result val");
               emitRM("LDA",mp,1,mp,"mp = mp + 1");

               emitRM("ST",ac1,0,ac,"assign: store array value");
               if (TraceCode) emitComment("<- Array");
            }
            else
            {
               sprintf(comment, "Assign %s to loc %d in scope %s", tree->child[0]->attr.data.name, loc, ScopeNow);
               if(TraceCode) emitComment(comment);
               emitRM("ST",ac,loc,gp,"assign: store value");
            }
         }
         
         if (TraceCode)  emitComment("<- assign") ;
         break; /* assign_k */

         // pc("\n\nNome variavel %s LOCALIZADA no LOC %d\n\n", tree->attr.data.name, loc);
         // if(!strcmp(tree->attr.data.type, "array")){
         //    if (TraceCode)  emitAssignInstruction("IdK", "reg", "exp", "4");
         // }

         /* now store value */
         // loc = st_lookup(tree->attr.data.name, ScopeNow);
         // emitRM("ST",ac,loc,gp,"assign: store value");

         // if(!strcmp(tree->child[0]->attr.data.type, "array")) {
         //    loc = st_lookup(tree->child[0]->attr.data.name, ScopeNow);
         //    // arranjar um jeito de puxar o id
         // }
         // loc = st_lookup(tree->child[0]->attr.data.name, ScopeNow);

         // PROBLEMA AQUI É O OFFSET DA PILHA, NAO TA SENDO CONSIDERADO o caso de VAR PARAMETER
         // emitRM("ST",ac,loc,gp,"assign: store value");

         //emitAssignInstruction("", "reg1", "reg2", "");

      case ReturnK :
         /* 
          *  Somente pula de Node, pode ser IdK ou CallK, por isso cgen
          */
         
         if (TraceCode)  emitComment("-> Return") ;
      
         p1 = tree->child[0];
         if (p1 != NULL) {
            // Da um load no ACUMULADOR ac que guarda qual eh o valor de retorno
            cGen(p1);
         }

         // HANDLE RETURN ADDRESS
         emitRM("LDA", ac1, 0, fp, "save current fp to ac1");
         emitRM("LD", fp, 0, fp, "adjust fp");
         emitRM("LD", PC, -1, ac1, "jump to return address");

         if (TraceCode)  emitComment("<- Return") ;
         break; /* return_k */

         //if (TraceCode)  emitReturn(gp);
            //emitRM("Return",ac,loc,gp,"return: store value");
            //emitReturnInstruction(t1);

         // emitRM("LDA", mp, 0, fp, "copy fp to mp");
         // emitRM("LD", fp, 0, mp, "pop fp");
         // emitRM("LDC", ac1, 1, 0, "ac1 = 1");
         // emitRO("ADD", mp, mp, ac1, "mp = mp + ac1 = mp + 1");
         
         // emitRM("LDA", mp, -1, mp, "adjust fp");
         // emitRM("LD", ac1, 0, mp, "load return address to ac1");
         // // Pulando de volta para o chamador
         // emitRM("LD", pc, 0, ac1, "return to caller");

      case CallK:
         /* 
          *  Todo CallK tem como child[0] o ID da funcao
          *  Tem como child[1] a lista de args
          *  A cada chamada de um args, basta puxar a arvore do cara que o valor dele ficou guardado no acumulador
          */

         sprintf(comment, "-> FCall %s", tree->child[0]->attr.data.name);
         if(TraceCode) emitComment(comment);

         p1 = tree->child[1];
         
         // HANDLE INPUT/OUTPUT
         if(!strcmp(tree->child[0]->attr.data.name, "input")) 
         {
            emitRO("IN",ac,0,0,"read integer value");
         } 
         else 
         {
            // HANDLE LOAD ARGS
            // toda vez que puxa um argumento, so dar store do acumulador no gp respectivo
            loc = st_lookup(tree->child[0]->attr.data.name, "");
            
            int count_args = 0;
            while( p1 != NULL )
            {
               count_args++;
               // Percorre o valor de Id e da LOAD no acumulador
               switch (p1->nodekind) {
                  case StmtK:
                     genStmt(p1);
                     break;
                  case ExpK:
                     // switch (p1->kind.exp) {
                     //    case IdK:
                     //       pc("\n\nid --------------\n\n");
                     //       pc("\n\nname %s --------------\n\n", p1->attr.data.name);
                     //       break;
                     //    case OpK:
                     //       pc("\n\nop --------------\n\n");
                     //       break;
                     //    case ConstK:
                     //       pc("\n\nconst --------------\n\n");
                     //       break;
                     //    default:
                     //       pc("\n\ndefaut --------------\n\n");
                     //       break;
                     // }
                     // pc("\n\nname %s --------------\n\n", p1->attr.data.name);
                     if(p1->kind.exp == IdK)
                     {   
                        if(strcmp(ScopeNow, "") && strcmp(ScopeNow, "main"))
                        {
                           if(!id_lookup(p1->attr.data.name, ScopeNow, "array"))
                           {
                              if (TraceCode) emitComment("-> call array 1") ;
                              int loc_array = st_lookup(p1->attr.data.name, ScopeNow);
                              // emitRM("LDA",ac,loc_array,gp,"load array address value");
                              emitRM("LD",ac,-1-count_args,fp,"load id value of parameter stack");
                              sprintf(comment, "argument %d saved at gp dMem %d", count_args, (loc+count_args));
                              emitRM("ST", ac, (loc+count_args), gp, comment);
                              if (TraceCode) emitComment("<- call array 1") ;
                           }
                           else if(!id_lookup(p1->attr.data.name, "", "array"))
                           {
                              if (TraceCode) emitComment("-> call array 2") ;
                              int loc_array = st_lookup(p1->attr.data.name, "");
                              // emitRM("LDA",ac,loc_array,gp,"load array address value");
                              emitRM("LD",ac,-1-count_args,fp,"load id value of parameter stack");
                              sprintf(comment, "argument %d saved at gp dMem %d", count_args, (loc+count_args));
                              emitRM("ST", ac, (loc+count_args), gp, comment);
                              if (TraceCode) emitComment("<- call array 2") ;
                           }
                           else
                           {
                              if (TraceCode) emitComment("-> call array 3") ;
                              // pc("\nvar %d\n",id_lookup(p1->attr.data.name, "", "array"));
                              genExp(p1);
                              sprintf(comment, "argument %d saved at gp dMem %d", count_args, (loc+count_args));
                              // pega o que esta no acumulador e passa para loc do argumento certo
                              emitRM("ST", ac, (loc+count_args), gp, comment);
                              if (TraceCode) emitComment("-> call array 3") ;
                           }
                        }
                        else
                        {
                           if(!id_lookup(p1->attr.data.name, ScopeNow, "array"))
                           {
                              if (TraceCode) emitComment("-> call array 1") ;
                              int loc_array = st_lookup(p1->attr.data.name, ScopeNow);
                              emitRM("LDA",ac,loc_array,gp,"load array address value");
                              sprintf(comment, "argument %d saved at gp dMem %d", count_args, (loc+count_args));
                              emitRM("ST", ac, (loc+count_args), gp, comment);
                              if (TraceCode) emitComment("<- call array 1") ;
                           }
                           else if(!id_lookup(p1->attr.data.name, "", "array"))
                           {
                              if (TraceCode) emitComment("-> call array 2") ;
                              int loc_array = st_lookup(p1->attr.data.name, "");
                              emitRM("LDA",ac,loc_array,gp,"load array address value");
                              sprintf(comment, "argument %d saved at gp dMem %d", count_args, (loc+count_args));
                              emitRM("ST", ac, (loc+count_args), gp, comment);
                              if (TraceCode) emitComment("<- call array 2") ;
                           }
                           else
                           {
                              if (TraceCode) emitComment("-> call array 3") ;
                              // pc("\nvar %d\n",id_lookup(p1->attr.data.name, "", "array"));
                              genExp(p1);
                              sprintf(comment, "argument %d saved at gp dMem %d", count_args, (loc+count_args));
                              // pega o que esta no acumulador e passa para loc do argumento certo
                              emitRM("ST", ac, (loc+count_args), gp, comment);
                              if (TraceCode) emitComment("-> call array 3") ;
                           }
                        }
                     }
                     else
                     {
                        if (TraceCode) emitComment("-> call id") ;
                        genExp(p1);
                        sprintf(comment, "argument %d saved at gp dMem %d", count_args, (loc+count_args));
                        // pega o que esta no acumulador e passa para loc do argumento certo
                        emitRM("ST", ac, (loc+count_args), gp, comment);
                        if (TraceCode) emitComment("-> call id") ;
                     }
                     break;
                  default:
                     break;
               }
               p1 = p1->sibling;
            }

            if(!strcmp(tree->child[0]->attr.data.name, "output"))
            { 
               /* now output it */
               emitRO("OUT",ac,0,0,"write ac");
            }
            else
            {
               emitRM("LDA", ac, 1, PC, "accumulate address call");

               sprintf(comment, "jump to function at %d", loc);
               if (TraceCode) emitComment(comment);
               emitRM("LD", PC, loc, gp, comment);
            }
         }

         sprintf(comment, "<- FCall %s", tree->child[0]->attr.data.name);
         if(TraceCode) emitComment(comment);
         break; /* call_k */

         // // DECRESCE mp para DAR ESPAÇO para colocar FUNCTION POINTER REGISTER
         // emitRM("LDC", ac1, 1, 0, "ac1 = 1");
         // emitRO("SUB", mp, mp, ac1, "mp = mp - ac1");
         // emitRM("ST", fp, 0, mp, "push fp");
         // // COLOCA fp NA MEMORIA
         // emitRM("LDA", fp, 0, mp, "copy sp to fp");
         // emitRO("SUB", mp, mp, ac1, "mp = mp - ac1");
         // CALCULATE RETURN ADDRESS AND PUSH INTO THE STACK
         // emitRM("LDC", ac1, 2, 0, "ac1 = 2");
         // emitRO("ADD", ac1, ac1, PC, "calculate return address");
         // emitRM("ST", ac1, 0, mp, "push return address");
         // emitRM("LDC", ac, 2, PC, "accumulate return address");
         // CALL FUNCTION at the correct global pointer address

         //  HANDLE LOAD ARGS
         // // toda vez que puxa um argumento, so dar store do acumulador no gp respectivo
         // int count_args = 0;
         // while( p1 != NULL )
         // {
         //       genExp(p1);
         //       // PQ VC TA DIMINUINDO O OFFSET DEPOIS?
         //       // emitRM("ST", ac, tmpOffset--, mp, "Call: push argument");
         //       // emitRM("ST", ac, --tmpOffset, mp, "op: push argument(reverse order)");
         //       count_args++;
         //       //cGen(p1);
         //       //emitParamInstruction("reg/var");
         //       p1 = p1->sibling;
         // }
         // emitRM("LDA", mp, -count_args, mp, "stack growth after push arguments");
         // sprintf(comment, "%d arguments are pushed", count_args);
         // if(TraceCode) emitComment(comment);

         // emitRM("LDC", ac1, 1, 0, "ac1 = 1");
         // emitRO("SUB", mp, mp, ac1, "mp = mp - ac1");
         // emitRM("ST", fp, 0, mp, "push fp");
         // emitRM("LDA", fp, 0, mp, "copy sp to fp");
         // emitRO("SUB", mp, mp, ac1, "mp = mp - ac1");
         // emitRM("LDC", ac1, 2, 0, "ac1 = 2");
         // emitRO("ADD", ac1, ac1, PC, "calculate return address");
         // emitRM("ST", ac1, 0, mp, "push return address");

         // // HANDLE CLEAR THE STACK (MEMORY POINTER)
         //    if (count_args > 0)
         //    {
         //       emitRM("LDC", ac1, count_args, 0, "ac1 = numberOfArguments");
         //       emitRO("ADD", mp, mp, ac1, "pop arguments");
         //    }

         // // Emita a instrução de chamada de função
         // // Ajuste tmpOffset para acomodar o endereço de retorno
         // tmpOffset -= 1;
         // emitRM("LDA", mp, tmpOffset + 1, mp, "Call: store return address");

         // // Ajuste tmpOffset para acomodar os parâmetros e o endereço de retorno
         // tmpOffset -= countParams;

         // // Emita uma instrução de salto para o endereço da função
         // emitRM("LDA", pc, st_lookup(tree->child[0]->attr.data.name, ""), PC, "Call: jump to function");

         // // Recupere o valor de retorno após a chamada da função
         // tmpOffset += countParams;
         // emitRM("LD", ac, tmpOffset + 1, mp, "Call: retrieve return value");

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
   if (tree == NULL)
      return;
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
         
         int scope_loc = 0;
         
         if(strcmp(ScopeNow, "") && strcmp(ScopeNow, "main"))
         {
            scope_loc = st_lookup(ScopeNow, "");
            loc = st_lookup(tree->attr.data.name, ScopeNow);
            int offset = -1+(scope_loc-loc);

            if((!strcmp(tree->attr.data.type, "array")) && (tree->child[0] != NULL))
            {
               if (TraceCode) emitComment("-> Array");
               // abre expressao dentro do indice e puxa no acumulador
               // if(tree->child[0] != NULL)
               // {
                  genExp(tree->child[0]);
                  emitRM("LD",ac1,offset,fp,"load id value of parameter stack");
                  emitRO("ADD",ac,ac1,ac,"load id array value");

                  emitRM("LD",ac,0,ac,"load id array value");
               // }
               if (TraceCode) emitComment("<- Array");
            }
            else
            {
               emitRM("LD",ac,offset,fp,"load id value of parameter stack");
            }
         }
         else
         {
            loc = st_lookup(tree->attr.data.name, ScopeNow);
            if(loc == -1)
            {
               loc = st_lookup(tree->attr.data.name, "");
            }
            // pc("\n\nNome variavel %s LOCALIZADA no LOC %d\n\n", tree->attr.data.name, loc);
            if((!strcmp(tree->attr.data.type, "array")) && (tree->child[0] != NULL))
            {
               if (TraceCode) emitComment("-> Array");
               // abre expressao dentro do indice e puxa no acumulador
               // if(tree->child[0] != NULL)
               // {
                  genExp(tree->child[0]);
                  emitRM("LDA",ac,loc,ac,"find id array address");
                  emitRO("ADD",ac,ac,gp,"load array address");
                  
                  emitRM("LD",ac,0,ac,"load id array value");
               // }
               if (TraceCode) emitComment("<- Array");
            }
            else
            {
               emitRM("LD",ac,loc,gp,"load id value");
            }
         }
         
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
         // tmpOffset--;
         emitRM("LDA",mp,-1,mp,"mp = mp - 1");
         emitRM("ST",ac,0,mp,"op: push left"); // emitRM("ST",ac,--memorySize,mp,"op: push left");
         /* gen code for ac = right operand */
         if (TraceCode) emitComment("-> right") ;
         genExp(p2);
         if (TraceCode) emitComment("<- right") ;
         /* now load left operand */
         emitRM("LD",ac1,0,mp,"op: load left"); // emitRM("LD",ac1,memorySize++,mp,"op: load left");
         emitRM("LDA",mp,1,mp,"mp = mp + 1");
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

static int getSizeOfGlobal(TreeNode * syntaxTree)
{
   int result = 0;
   TreeNode *tree = syntaxTree;
   while(tree != NULL)
   {
      if(tree->kind.stmt == FunDecK)
         result++;
      // if(tree->kind.stmt == VarDecK && (!strcmp(tree->child[0]->attr.data.type, "array")))
      //    // result += tree->child[0]->child[0]->attr.val;
      // else
      //    result++;
      tree = tree->sibling;
   }
   return result;
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
   emitRM("LD",fp,0,ac,"load maxaddress from location 0");
   emitRM("ST",ac,0,ac,"clear location 0");
   //emitComment("End of standard prelude.");
   /* generate code for TINY program */
   locFunction = emitSkip(getSizeOfGlobal(syntaxTree)*2 - 1);
   // pc("* Size of global is %d\n", getSizeOfGlobal(syntaxTree));
   cGen(syntaxTree);
   /* finish */
   emitComment("End of execution.");
   if (TraceCode)  emitHalt() ;
   emitRO("HALT",0,0,0,"");
}