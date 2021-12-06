#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>

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

//dimioytgo prota ena filename ok;
//ara ftiaxno ena adeio hash table
//episis ftiaxno kai ena 


//ενας struct που αποθηκευουμε δομες μας
struct table_file {
	File_open table[20];
  int size_table;    //size for table
};

typedef struct table_file* FileTable;


//καθολικη μεταβλητη - για τις δομες μας
FileTable filetable;



struct FileInMem{
  Pointer** table;
};



/* 
 * Η συνάρτηση HT_Init χρησιμοποιείται για την αρχικοποίηση κάποιον δομών που μπορεί να χρειαστείτε. 
 * Σε περίπτωση που εκτελεστεί επιτυχώς, επιστρέφεται HT_OK, ενώ σε διαφορετική περίπτωση κωδικός λάθους.
 */
HT_ErrorCode HT_Init() 
{
  //insert code here
  filetable = malloc(sizeof(struct table_file));
  // for( int i = 0 ; i<20;i++)
  // {
  //   filetable->table[i] = malloc(sizeof(struct file_open));
  //   filetable-
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
    
  int file_desc;

  CALL_BF( BF_CreateFile(filename));
  CALL_BF( BF_OpenFile( filename, &file_desc));

  // printf("elaa create %s -- %d\n", filename, file_desc);
  //number of pointers in a block
  int num = BF_BLOCK_SIZE/sizeof(int);
  //num of blocks required for hash table
  int a = 1;
  for( int i = 0; i < depth; i++)
  {
    a = a*2;
  }
  int num_of_blocks = a/num;
  if(a % num >0){
    num_of_blocks++;
  }

  //φτιαχνω ενα μπλοκ και βαζω μεσα το βαθος
  BF_Block *block;
  BF_Block_Init(&block);

  CALL_BF( BF_AllocateBlock( file_desc, block));
  char* data;

  data = BF_Block_GetData(block);
  memcpy(data, &depth, sizeof(int));
  BF_Block_SetDirty(block);
 
  //Πρεπει να το ξαναδω αν σε ενοχλεί βάλτο σε σχόλια μην το σβήσεις.
  int i = 0;
  int num_of_ints = 0;
  data = BF_Block_GetData(block) + sizeof(int); //pairno to data

  //FOR HASH-TABLE
  while (num_of_ints < a)        //ελεγχουμε αν χωραει στο ιδιο μπλοκ το αρχικο μας hash-table
  {
    if(data+i*sizeof(int) > data+BF_BLOCK_SIZE-1)
    {
      CALL_BF( BF_AllocateBlock( file_desc, block));
      data = BF_Block_GetData( block);
      i = 0;
    }
    
  
    memcpy( data + i*sizeof(int), &num_of_ints, sizeof( int));
    BF_Block_SetDirty(block);

    num_of_ints++;
    i++;
  }


  //print all elements
  //PRINTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
  // data = BF_Block_GetData( block);
  // for( int i = 0; i < 5; i++)
  // {
  //     int* d = data + i*sizeof(int);
  //     printf("EDO EIMAI   %d data:  %d\n",d[0], d);

  // }

  //FOR BUCKET IN HASHTABLE 
  //1BUCKET = 1BLOCK
  int depth_bucket = depth;
  for( int i = 0; i < a; i++)
  {
    CALL_BF(BF_AllocateBlock(file_desc, block));

    data = BF_Block_GetData(block);
    memcpy( data, &depth_bucket, sizeof(int));
    BF_Block_SetDirty(block);
  }



  //Unpin the blocks
  int blocks_num;
  CALL_BF( BF_GetBlockCounter( file_desc, &blocks_num));
  

  //PRINTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
  // for( int i = 1; i < blocks_num; i++)
  // {   
  //     BF_GetBlock( file_desc, i, block);
  //     int* data = BF_Block_GetData(block);
  //     printf("DEPTH BUCKET   %d \n",data[0]);

  // }

  //PRINTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
  // printf("eco blocks: %d\n", blocks_num);
  // for( int i = 0; i < blocks_num; i++)
  // {
  //   CALL_BF( BF_GetBlock(file_desc, i, block));
  //   CALL_BF( BF_UnpinBlock( block));
  // }

  // CALL_BF(BF_CloseFile(file_desc));
  BF_Block_Destroy( &block);

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
  int file_desc;
  CALL_BF( BF_OpenFile(fileName, &file_desc));

  // printf("elaaaaaa  %s --- %d\n",fileName,file_desc);

  for(int i = 0; i < filetable->size_table; i++){
    if(filetable->table[i] == NULL){
      *indexDesc = i;
      break;
    }
  }

  // printf(" ind: %d\n",filetable->table[*indexDesc]->filename);  
  filetable->table[*indexDesc] = malloc(sizeof(struct file_open));
  filetable->table[*indexDesc]->file_desc = file_desc;
  filetable->table[*indexDesc]->indexdesc = *indexDesc;
  filetable->table[*indexDesc]->filename = malloc((strlen(fileName) +1)*sizeof(char));
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
 
  BF_Block* block;
  BF_Block_Init( &block);
  int blocks_num;
  CALL_BF( BF_GetBlockCounter( file_desc, &blocks_num));

  for( int i = 0; i < blocks_num; i++)
  {
    CALL_BF( BF_GetBlock(file_desc, i, block));
    CALL_BF( BF_UnpinBlock( block));
  }

  BF_CloseFile(file_desc);
  BF_Block_Destroy( &block);

  free(filetable->table[indexDesc]->filename);
  free(filetable->table[indexDesc]);

  filetable->size_table--;
  
  return HT_OK;
}

int HashFunction( Record record, int depth)
{

  int id = record.id;
  id = id >> sizeof(int) - depth;
  printf("id == %d", id);

}

HT_ErrorCode HT_InsertEntry(int indexDesc, Record record) {
  //insert code here


  int filedesc = filetable->table[indexDesc]->file_desc;
  BF_Block* block;
  BF_Block_Init( &block);

  int blocks_num;
  CALL_BF( BF_GetBlockCounter( filedesc, &blocks_num));
  
  if( blocks_num < 5)
  {
    return HT_ERROR;
  }
  printf("num:    %d\n", filedesc);


  int i = 0;

  // for( int i = 0; i < blocks_num; i++)
  // {
    // CALL_BF( BF_GetBlock(file_desc, i, block));
    // CALL_BF( BF_UnpinBlock( block));
  // }
  CALL_BF(BF_GetBlock(filedesc, i, block));
  // int* data = BF_Block_GetData(block); 
  char* data;
  data = BF_Block_GetData( block);
  for( int i = 0; i < 5; i++)
  {
      int* d = data + i*sizeof(int);
      printf("EDO EIMAI   %d data:  %d\n",d[0], d);

  }
  
  printf("dep:  %d\n", data[0]);
  // HashFunction( record, depth);

  BF_Block_Destroy( &block);
  return HT_OK;
}

HT_ErrorCode HT_PrintAllEntries(int indexDesc, int *id) {
  //insert code here
  return HT_OK;
}

