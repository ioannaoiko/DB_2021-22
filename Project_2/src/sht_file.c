#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "sht_file.h"
#define MAX_OPEN_FILES 20

#define CALL_BF(call)       \
{                           \
  BF_ErrorCode code = call; \
  if (code != BF_OK) {         \
    BF_PrintError(code);    \
    return HP_ERROR;        \
  }                         \
}

// #ifndef HASH_FILE_H
// #define HASH_FILE_H


typedef enum HT_ErrorCode {
  HT_OK,
  HT_ERROR
} HT_ErrorCode;

typedef struct Record {
	int id;
	char name[15];
	char surname[20];
	char city[20];
} Record;

typedef struct{
  char index_key[20];
  TupleId tupleId;  /*Ακέραιος που προσδιορίζει το block και τη θέση μέσα στο block στην οποία     έγινε η εισαγωγή της εγγραφής στο πρωτεύον ευρετήριο.*/ 
} SecondaryRecord;





//////
struct file_open{
  char* filename;
  int indexdesc;
  int file_desc;
};

typedef struct file_open* File_open;

//ενας struct που αποθηκευουμε τα ανοιχτά αρχεία.
struct table_file {
	File_open table[20];
  int size_table;    //size for table
};

typedef struct table_file* FileTable;


//καθολικη μεταβλητη - για τις δομες μας
FileTable filetable;





HT_ErrorCode SHT_Init() {
  //insert code here
  filetable = malloc(sizeof(struct table_file));
  filetable->size_table = 0;
  return HT_OK;
}

// const char *sfileName, /* όνομα αρχείου */
// char *attrName, /* όνομα πεδίου-κλειδιού */ //px city
// int attrLength, /* μήκος πεδίου-κλειδιού */ //20
// int depth, /* το ολικό βάθος ευρετηρίου επεκτατού κατακερματισμού */
// char *fileName /* όνομα αρχείου πρωτεύοντος ευρετηρίου*/)


// Η συνάρτηση SHT_CreateSecondaryIndex χρησιμοποιείται για τη δημιουργία και κατάλληλη
// αρχικοποίηση ενός αρχείου δευτερεύοντος κατακερματισμού με όνομα sfileName για το
// αρχείο πρωτεύοντος κατακερματισμού fileName.
// Σε περίπτωση που εκτελεστεί επιτυχώς, επιστρέφεται HT_OK, ενώ σε διαφορετική
// περίπτωση κωδικός λάθους.

HT_ErrorCode SHT_CreateSecondaryIndex(const char *sfileName, char *attrName, int attrLength, int depth,char *fileName ) {
  //insert code here

  int file_desc;

  CALL_BF( BF_CreateFile( sfileName));
  CALL_BF( BF_OpenFile( sfileName, &file_desc));

  //φτιαχνω ενα μπλοκ και βαζω μεσα το βαθος
  BF_Block *block;
  BF_Block_Init(&block);

  CALL_BF( BF_AllocateBlock( file_desc, block));
  char* data;
  data = BF_Block_GetData( file_desc, block);
  
  //save filename
  memcpy( data, fileName, sizeof( fileName));
  BF_Block_SetDirty( block);

  //save attr name
  memcpy(data + sizeof(fileName), attrName, sizeof(attrName) );
  BF_Block_SetDirty( block);

  //save attr lengh
  memcpy( data + sizeof(fileName) + sizeof(attrName), attrLength, sizeof(int));
  BF_Block_SetDirty( block);

  CALL_BF( BF_UnpinBlock( block));


  CALL_BF( BF_AllocateBlock( file_desc, block));
  char* data;

  data = BF_Block_GetData(block);
  memcpy(data, &depth, sizeof(int));
  BF_Block_SetDirty(block);

  int hash_block_index = 2;
  memcpy(data + sizeof(int), &hash_block_index, sizeof(int));
  BF_Block_SetDirty(block);


  // -1 is the final prihe first ce in tblock
  int end = -1;
  memcpy(data + 2*sizeof(int), &end, sizeof(int));
  BF_Block_SetDirty(block);

  //number of pointers in a block
  int num = BF_BLOCK_SIZE/sizeof(int);
  // num of blocks required for hash table
  int a = 1;
  for( int i = 0; i < depth; i++)
  {
    a = a*2;
  }
  int num_of_blocks = a/num;
  if(a % num >0){
    num_of_blocks++;
  }
  
  CALL_BF( BF_UnpinBlock( block));

  CALL_BF( BF_AllocateBlock( file_desc, block));
 
  int i = 0;
  int num_of_ints = 0;
  data = BF_Block_GetData(block); //pairno to data
  int hash_block = 1;
  //FOR HASH-TABLE
  while (num_of_ints < a)        //ελεγχουμε αν χωραει στο ιδιο μπλοκ το αρχικο μας hash-table
  {
    if(data + i*sizeof(int) > data + BF_BLOCK_SIZE-1)
    { 
      CALL_BF( BF_UnpinBlock( block));
      CALL_BF( BF_AllocateBlock( file_desc, block));
      data = BF_Block_GetData( block);
      i = 0;
    }
    
    int bucket_block = num_of_ints + 3;
    memcpy( data + i*sizeof(int), &bucket_block, sizeof( int));
    BF_Block_SetDirty(block);

    num_of_ints++;
    i++;
  }


  //FOR BUCKET IN HASHTABLE 
  //1BUCKET = 1BLOCK
  int depth_bucket = depth;
  for( int i = 0; i < a; i++)
  {
    CALL_BF( BF_UnpinBlock( block));
    CALL_BF(BF_AllocateBlock(file_desc, block));

    data = BF_Block_GetData(block);
    memcpy( data, &depth_bucket, sizeof(int));
    BF_Block_SetDirty(block);
  }


  CALL_BF( BF_UnpinBlock( block));

  // BF_Block_Destroy( &block);
  return HT_OK;
}

HT_ErrorCode SHT_OpenSecondaryIndex(const char *sfileName, int *indexDesc  ) {
  //insert code here
   //insert code here
  
  if(filetable->size_table == 20){
    return HT_ERROR;
  }

  int filedesc;
  CALL_BF( BF_OpenFile( sfileName, &filedesc));

  for(int i = 0; i < MAX_OPEN_FILES; i++){
    if(filetable->table[i] == NULL){
      *indexDesc = i;
      break;
    }
  }

  filetable->table[*indexDesc] = malloc(sizeof(struct file_open));
  filetable->table[*indexDesc]->file_desc = filedesc;
  filetable->table[*indexDesc]->indexdesc = *indexDesc;
  filetable->table[*indexDesc]->filename = malloc((strlen( sfileName) +1)*sizeof(char));
  strcpy(filetable->table[*indexDesc]->filename, sfileName);

  filetable->size_table++;

  return HT_OK;
}

HT_ErrorCode SHT_CloseSecondaryIndex(int indexDesc) {
  //insert code here

  if(indexDesc >= 20){
    return HT_ERROR;
  }

  if(filetable->table[indexDesc] == NULL){
    return HT_ERROR;
  }

  int file_desc = filetable->table[indexDesc]->file_desc;
 
  BF_Block* block;
  BF_Block_Init( &block);
  int blocks_num;
  CALL_BF( BF_GetBlockCounter( file_desc, &blocks_num));

  BF_CloseFile(file_desc);
  BF_Block_Destroy( &block);

  free(filetable->table[indexDesc]->filename);
  free(filetable->table[indexDesc]);

  filetable->size_table--;
  return HT_OK;
}

HT_ErrorCode SHT_SecondaryInsertEntry (int indexDesc, SecondaryRecord record  ) {
  //insert code here
  return HT_OK;
}

HT_ErrorCode SHT_SecondaryUpdateEntry (int indexDesc, UpdateRecordArray *updateArray ) {
  //insert code here
  return HT_OK;
}

HT_ErrorCode SHT_PrintAllEntries(int sindexDesc, char *index_key ) {
  //insert code here
  return HT_OK;
}

HT_ErrorCode SHT_HashStatistics(char *filename ) {
  //insert code here
  return HT_OK;
}

HT_ErrorCode SHT_InnerJoin(int sindexDesc1, int sindexDesc2,  char *index_key ) {
  //insert code here
  return HT_OK;
}


// #endif // HASH_FILE_H