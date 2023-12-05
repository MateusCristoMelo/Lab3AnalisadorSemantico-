/****************************************************/
/* File: main.c                                     */
/* Main program for TINY compiler                   */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"

/* set NO_PARSE to TRUE to get a scanner-only compiler */
#define NO_PARSE FALSE
/* set NO_ANALYZE to TRUE to get a parser-only compiler */
#define NO_ANALYZE FALSE

/* set NO_CODE to TRUE to get a compiler that does not
 * generate code
 */
#define NO_CODE TRUE

#include "util.h"
#if NO_PARSE
#include "scan.h"
#else
#include "parse.h"
#if !NO_ANALYZE
#include "analyze.h"
#if !NO_CODE
#include "cgen.h"
#endif
#endif
#endif

/* allocate global variables */
int lineno = 0;
FILE * source;
FILE * listing;
FILE * code;

char **linhas = NULL;
struct Stack name_stack;
struct Stack number_stack; 
struct Stack _Scope; 
struct Queue var_or_array_stack;
struct Stack lineno_stack; 

/* allocate and set tracing flags */
int EchoSource = FALSE;
int TraceScan = TRUE;
int TraceParse = TRUE;
int TraceAnalyze = TRUE;
int TraceCode = TRUE;

int Error = FALSE;

int main( int argc, char * argv[] )
{ TreeNode * syntaxTree;

  char *linha = NULL;
  size_t tamanho = 0;
  size_t lidos;
  int numero_de_linhas = 0;
  int numero_linhas_arquivo = 0;
  FILE * linhas_codigo;
  FILE * linhas_iteracao;
  initStack(&name_stack);
  initStack(&number_stack);
  initStack(&_Scope);
  initQueue(&var_or_array_stack);
  initStack(&lineno_stack);

    char pgm[120]; /* source code file name */
    if ((argc < 2) || (argc > 3))
    { fprintf(stderr,"usage: %s <filename> [<detailpath>]\n",argv[0]);
      exit(1);
    }
    strcpy(pgm,argv[1]);
    if (strchr (pgm, '.') == NULL)
        strcat(pgm,".cm");
    source = fopen(pgm,"r");

    linhas_codigo = fopen(pgm,"r");

    //redundant_source = fopen(pgm, "r");
    if (source==NULL)
    { fprintf(stderr,"File %s not found\n",pgm);
        exit(1);
    }

    while ((lidos = getline(&linha, &tamanho, linhas_codigo)) != -1) {
      linhas = realloc(linhas, (numero_de_linhas + 2) * sizeof(char *));
      linhas[numero_de_linhas] = malloc((size_t)(lidos + 1));
      strcpy(linhas[numero_de_linhas], linha);
      numero_de_linhas++;
    }


    
    char detailpath[200];
    if (3 == argc) {
        strcpy(detailpath,argv[2]);
    } else strcpy(detailpath,"/tmp/");
    
    listing = stdout; /* send listing to screen */
    initializePrinter(detailpath, pgm, LOGALL);


    
  fprintf(listing,"\nC- COMPILATION: %s\n",pgm);
#if NO_PARSE
  while (getToken()!=ENDFILE);
#else
  fprintf(listing,"\nStart parse():\n");
  syntaxTree = parse();
  doneLEXstartSYN();
  if (TraceParse) {

    fprintf(listing,"\nSyntax tree:\n");
    printTree(syntaxTree);
  }
#if !NO_ANALYZE
  doneSYNstartTAB();
  if (! Error)
  { if (TraceAnalyze) fprintf(listing,"\nBuilding Symbol Table...\n");
    buildSymtab(syntaxTree);
    if (TraceAnalyze) fprintf(listing,"\nChecking types...\n");
    typeCheck(syntaxTree);
    if (TraceAnalyze) fprintf(listing,"\nSemantic analysis finished\n");
  }
#if !NO_CODE
  doneTABstartGEN();
  if (! Error)
  { 
    codeGen(syntaxTree);
  }
#endif
#endif
#endif
  fclose(source);
  closePrinter();
  return 0;
}

