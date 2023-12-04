/****************************************************/
/* File: symtab.c                                   */
/* Symbol table implementation for the TINY compiler*/
/* (allows only one symbol table)                   */
/* Symbol table is implemented as a chained         */
/* hash table                                       */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"

/* SIZE is the size of the hash table */
#define SIZE 211

/* SHIFT is the power of two used as multiplier
   in hash function  */
#define SHIFT 4


// int stringsum(char * comp_string)
// { int i = 0;
// int sum = 0;
//   if(comp_string == NULL)
//     return 0;
    
//     while (comp_string[i] != '\0') {
//         sum += comp_string[i] - '0';  // Convertendo o caractere para valor inteiro
//         i++;
//     }
    
//   return sum;
// }

/* the hash function */
static int hash ( char * key )
{ int temp = 0;
  int i = 0;
  // int s = stringsum(scope);
  while (key[i] != '\0')
  { 
    temp = ((temp << SHIFT) + key[i]) % SIZE;
    // temp = ((temp << SHIFT) + key[i] + s) % SIZE;
    //pc("\nKEY i = %c", key[i]);
    //pc("\nADD because 1 digit\n");
    ++i;
  }
  return temp;
}

/* the list of line numbers of the source 
 * code in which a variable is referenced
 */
typedef struct LineListRec
   { int lineno;
     struct LineListRec * next;
   } * LineList;

/* The record in the bucket lists for
 * each variable, including name, 
 * assigned memory location, and
 * the list of line numbers in which
 * it appears in the source code
 */

typedef struct BucketListRec
   { char * name;
     char * scope;
     char * id_type;
     char * data_type;
     LineList lines;
     int memloc ; /* memory location for variable */
     struct BucketListRec * next;
   } * BucketList;

/* the hash table */
static BucketList hashTable[SIZE];

/* Procedure st_insert inserts line numbers and
 * memory locations into the symbol table
 * loc = memory location is inserted only the
 * first time, otherwise ignored
 */
void st_insert( char * name, int lineno, int loc, char * scope, char * id_type, char * data_type)
{ 
  //pc("\n\nHASH OF INSERT -----------------------------------------------------------\n\n");
  if (name == NULL)
    return;
  int h = hash(name);
  // pc("\n\nHASH: %d\n", h);
  // pc("NAME: %s\n", name);
  // pc("SCOPE: %s\n", scope);
  BucketList l = hashTable[h];
  while (l != NULL)
  {
    if((strcmp(name,l->name) == 0) && (strcmp(scope,l->scope) == 0))
    {
      break;
    }
    l = l->next;
    // pc("STORING IN NEXT BUCKET\n");
  }
  if (l == NULL) /* variable not yet in table */
  { 
    l = (BucketList) malloc(sizeof(struct BucketListRec));
    l->name = name;
    l->lines = (LineList) malloc(sizeof(struct LineListRec));
    l->lines->lineno = lineno;
    l->memloc = loc;

    l->scope=scope;
    l->id_type=id_type;
    l->data_type=data_type;

    l->lines->next = NULL;
    l->next = hashTable[h];
    hashTable[h] = l; }
  else /* found in table, so just add line number */
  { 
    LineList t = l->lines;
    if(t->lineno == lineno) return;
    while (t->next != NULL) {
      if(t->next->lineno == lineno) return;
      t = t->next;
      }
    t->next = (LineList) malloc(sizeof(struct LineListRec));
    t->next->lineno = lineno;
    t->next->next = NULL;
  }
} /* st_insert */

/* Function st_lookup returns the memory 
 * location of a variable or -1 if not found
 */
int st_lookup ( char * name, char * scope)
{ 
  //pc("\n\nSTRING: %s --------------------------------------------------------------\n\n", name);
  if (name == NULL) return -1;
  int h = hash(name);
  //pc("\n\nHASH: %d\n", h);
  BucketList l =  hashTable[h];
  while (l != NULL)
  {
    if((strcmp(name,l->name) == 0) && (strcmp(scope,l->scope) == 0))
    {
      break;
    }
    l = l->next;
    // pc("STORING IN NEXT BUCKET\n");
  }
  if (l == NULL) return -1;
  else return l->memloc;
}

/* Procedure printSymTab prints a formatted 
 * list of the symbol table contents 
 */
void printSymTab()//tem q alterar
{ int i;
  pc("Variable Name  Scope     ID Type  Data Type  Line Numbers             \n");
  pc("-------------  --------  -------  ---------  -------------------------\n");
  for (i=0;i<SIZE;++i)
  { 
    if (hashTable[i] != NULL)
    { 
      BucketList l = hashTable[i];
      while (l != NULL)
      { 
        pc("%-14s ",l->name);
        pc("%-8s  ",l->scope);
        pc("%-7s  ",l->id_type);
        pc("%-9s  ",l->data_type);

        //pc("%-8d  ",l->memloc);
        LineList t = l->lines;
        while (t != NULL)
        {
          if(t->lineno != 0)
          {
            pc("%2d ",t->lineno);
          }
          t = t->next;
        }
        pc("\n");

        l = l->next;
      }
    }
  }
} /* printSymTab */