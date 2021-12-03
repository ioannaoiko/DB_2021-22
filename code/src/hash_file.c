#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#include "bf.h"
#include "hash_file.h"
#define MAX_OPEN_FILES 20

#define CALL_BF(call)       \
{                           \
  BF_ErrorCode code = call; \
  if (code != BF_OK) {         \
    BF_PrintError(code);    \
    return HT_ERROR;        \
  }                         \
}



// //hash table for everything file
// struct hash_table{

// };

// typedef struct hash_table* HashTable;

// //one file have one table_file with filename,indexdesx and one hash table
// struct file{
//   char* filename;
//   int* indexdesc;
//   HashTable hash_table;
// };

// typedef struct file* File;

struct file_open{
  char* filename;
  int indexdesc;
  int file_desc;
};

typedef struct file_open* File_open;

struct all_files
{
  
};

typedef struct all_files* AllFiles;
//dimioytgo prota ena filename ok;
//ara ftiaxno ena adeio hash table
//episis ftiaxno kai ena 


//ενας struct που αποθηκευουμε δομες μας
struct table_file {
	File_open table[20];
  int size_table;    //size for table
};

typedef struct table_file* FileTable;


struct FileInMem{
  Pointer** table;
};



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
  
  int file_desc;

  CALL_BF(BF_Init(LRU));
  CALL_BF( BF_CreateFile(filename));
  CALL_BF( BF_OpenFile( filename, &file_desc));

  //number of pointers in a block
  int num = BF_BLOCK_SIZE/sizeof(int);
  //num of blocks required for hash table
  int a = pow(2,depth);
  int num_of_blocks = a/num;
  if(a % num >0){
    num_of_blocks++;
  }

  // BF_Block*** table;
  // table = malloc(depth*sizeof(BF_Block***));
  // for( int i = 0; i < depth; i++)
  // {
  //   table[i] = malloc(sizeof(BF_Block**));
    
  // }
  // printf("Bl size: %d\n", sizeof(table));
  BF_Block *block;
  BF_Block_Init(&block);

  printf("eimai do\n");
  CALL_BF( BF_AllocateBlock( file_desc, block));
  char* data;
  printf("eimai do\n");
  data = BF_Block_GetData(block);
  printf("eimai don %d\n", depth);
  memset(data, depth, BF_BLOCK_SIZE);
  BF_Block_SetDirty(block);
  data = BF_Block_GetData(block);
  printf("edwsa data %d\n", data[513]);
  CALL_BF(BF_UnpinBlock(block));

  CALL_BF( BF_AllocateBlock( file_desc, block));

  //Πρεπει να το ξαναδω αν σε ενοχλεί βάλτο σε σχόλια μην το σβήσεις.
  int i = 0;
  int num_of_ints = 0;
  while (num_of_ints < a){
    if(block+i*sizeof(int) > block+BF_BLOCK_SIZE-1){
      CALL_BF( BF_AllocateBlock( file_desc, block));
      i = 0;
    }
    memcpy(block+i*sizeof(int), num_of_ints, sizeof(int));
    num_of_ints++;
    i++;
  }
  
  //se ena block Plirofories
  //bathos - filename


  //se ena deytero to pointer tou table

  //alla block gia egrafes gia arxi 4


  CALL_BF( BF_CloseFile(file_desc));


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
  int* file_desc;
  BF_OpenFile(fileName, file_desc);

  for(int i = 0; i < filetable->size_table; i++){
    if(filetable->table[i] == NULL){
      *indexDesc = i;
      break;
    }
  }

  filetable->table[*indexDesc] = malloc(sizeof(struct file_open));
  filetable->table[*indexDesc]->file_desc = *file_desc;
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

  int file_desc = filetable->table[indexDesc]->file_desc;
  BF_CloseFile(file_desc);

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

