#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "bf.h"
#include "hash_file.h"
#define MAX_OPEN_FILES 20

#define CALL_BF(call)       \
{                           \
  BF_ErrorCode code = call; \
  if (code != BF_OK) {         \
    BF_PrintError(code);    \
    return HP_ERROR;        \
  }                         \
}



//hash table for everything file
struct hash_table{

};

typedef struct hash_table* HashTable;

//one file have one table_file with filename,indexdesx and one hash table
struct file{
  char* filename;
  int* indexdesc;
  HashTable hash_table;
};

typedef struct file* File;

struct file_info{
  char* filename;
  int indexdesc;
  FILE* fptr;
};

typedef struct file_info* File_info;


//ενας struct που αποθηκευουμε δομες μας
struct table_file {
	File_info table[20];    
  int size_table;    //size for table
};

typedef struct table_file* FileTable;



//καθολικη μεταβλητη - για τις δομες μας
FileTable filetable;





/*
 * Η συνάρτηση HT_Init χρησιμοποιείται για την αρχικοποίηση κάποιον δομών που μπορεί να χρειαστείτε. 
 * Σε περίπτωση που εκτελεστεί επιτυχώς, επιστρέφεται HT_OK, ενώ σε διαφορετική περίπτωση κωδικός λάθους.
 */
HT_ErrorCode HT_Init() 
{
  //insert code here
  filetable = malloc(sizeof(struct table_file));

  //αρχικοποιουμε τις δομες μας
  //το πολυ 20 αρχεια μπορουν να υπαρχουν ανοικτα
  // for( int i = 0; i < 20; i++)
  // {
  //   filetable->table[i] = malloc(sizeof(struct file_info));
  //   if( filetable->table[i] == NULL) { return HT_ERROR;}
  // }
  
  filetable->size_table = 0;
  return HT_OK;
}







/*
 * Η συνάρτηση HT_CreateIndex χρησιμοποιείται για τη δημιουργία και κατάλληλη αρχικοποίηση ενός άδειου αρχείου κατακερματισμού με όνομα fileName. 
 * Στην περίπτωση που το αρχείο υπάρχει ήδη, τότε επιστρέφεται ένας κωδικός λάθους. 
 * Σε περίπτωση που εκτελεστεί επιτυχώς επιστρέφεται HΤ_OK, ενώ σε διαφορετική περίπτωση κωδικός λάθους.
 */
HT_ErrorCode HT_CreateIndex(const char *filename, int depth) {
  //insert code here
  
  
  //μια μεταβλητη bool αν υπαρχει ή οχι
  
  bool find = false;

  //τσεκαρουμε αν το filename υπαρχει στο πινακα μας
  for(int i = 0; i < filetable->size_table; i++)
  {
    char* file = filetable->table[ i]->filename;
    if(strcmp( filename, file) == 0)
    {
      find = true;
      break;
    }
  }
  
  //αν δεν υπαρχει στο πινακα το εισχωρουμε
  //και αυξανουμε το size
  if(find == false)
  { 
    if( filetable->size_table == 20){ return HT_ERROR;}

    //akou edo to kano create
    FILE *file;
    file = fopen(filename, "mode");

    //edo to apothikeyo
    filetable->table[ filetable->size_table]->filename = file;
    filetable->table[ filetable->size_table]->indexdesc = NULL;
    filetable->table[ filetable->size_table]->hash_table = malloc(sizeof( struct hash_table));
    filetable->size_table = filetable->size_table + 1;

  }
  //αν υπαρχει τοτε μυνημα σφαλματος
  else
  {
    return HT_ERROR;
  }



  return HT_OK;
}






/*
 * Η ρουτίνα αυτή ανοίγει το αρχείο με όνομα fileName. 
 * Εάν το αρχείο ανοιχτεί κανονικά, η ρουτίνα επιστρέφει HT_OK, ενώ σε διαφορετική περίπτωση κωδικός λάθους.
 */

HT_ErrorCode HT_OpenIndex(const char *fileName, int *indexDesc){
  //insert code here
  
  if(filetable->size_table == 20){
    return HT_ERROR;
  }
  FILE* fptr = fopen(fileName, 'r+');

  for(int i = 0; i < filetable->size_table; i++){
    if(filetable->table[i] == NULL){
      *indexDesc = i;
      break;
    }
  }

  filetable->table[*indexDesc] = malloc(sizeof(struct file_info));
  filetable->table[*indexDesc]->fptr = fptr;
  filetable->table[*indexDesc]->indexdesc = *indexDesc;
  strcpy(filetable->table[*indexDesc]->filename, fileName);

  filetable->size_table++;

  return HT_OK;
}

HT_ErrorCode HT_CloseFile(int indexDesc) {
  //insert code here
  if(indexDesc >= 20){
    return HT_ERROR;
  }

  if(filetable->table[indexDesc] == NULL){
    return HT_ERROR;
  }

  char* ptr = filetable->table[indexDesc]->fptr;
  fclose(ptr);

  free(filetable->table[indexDesc]);

  filetable->size_table--;
  
  return HT_OK;
}

HT_ErrorCode HT_InsertEntry(int indexDesc, Record record) {
  //insert code here
  return HT_OK;
}

HT_ErrorCode HT_PrintAllEntries(int indexDesc, int *id) {
  //insert code here
  return HT_OK;
}

