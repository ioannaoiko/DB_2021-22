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

  //φτιαχνω ενα μπλοκ και βαζω μεσα το βαθος
  BF_Block *block;
  BF_Block_Init(&block);

  CALL_BF( BF_AllocateBlock( file_desc, block));
  char* data;

  data = BF_Block_GetData(block);
  memcpy(data, &depth, sizeof(int));
  BF_Block_SetDirty(block);

  data = data + sizeof(int);
  int hash_block_index = 1;
  memcpy(data, &hash_block_index, sizeof(int));
  BF_Block_SetDirty(block);


  //-1 is the final price in the first block
  data = data + sizeof(int);
  int end = -1;
  memcpy(data, &end, sizeof(int));
  BF_Block_SetDirty(block);

  // CALL_BF( BF_UnpinBlock( block));
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

  CALL_BF( BF_AllocateBlock( file_desc, block));
 
  //Πρεπει να το ξαναδω αν σε ενοχλεί βάλτο σε σχόλια μην το σβήσεις.
  int i = 0;
  int num_of_ints = 0;
  data = BF_Block_GetData(block); //pairno to data

  //FOR HASH-TABLE
  while (num_of_ints < a)        //ελεγχουμε αν χωραει στο ιδιο μπλοκ το αρχικο μας hash-table
  {
    if(data+i*sizeof(int) > data+BF_BLOCK_SIZE-1)
    { 
      // CALL_BF( BF_AllocateBlock( file_desc, block));
      CALL_BF( BF_AllocateBlock( file_desc, block));
      data = BF_Block_GetData( block);
      i = 0;
    }
    
    int bucket_block = num_of_ints + 2;
    memcpy( data + i*sizeof(int), &bucket_block, sizeof( int));
    BF_Block_SetDirty(block);

    num_of_ints++;
    i++;
  }


  // print all elements
  // PRINTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
  data = BF_Block_GetData( block);
  for( int i = 0; i < 4; i++)
  {
      int* d = data + i*sizeof(int);
      printf("EDO EIMAI   %d data:  %d\n",d[0], d);

  }
  // CALL_BF( BF_AllocateBlock( file_desc, block));
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
  // int blocks_num;
  // CALL_BF( BF_GetBlockCounter( file_desc, &blocks_num));
  

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
  // CALL_BF( BF_GetBlockCounter( file_desc, &blocks_num));

  // for( int i = 0; i < blocks_num; i++)
  // {
  //   CALL_BF( BF_GetBlock(file_desc, i, block));
  //   CALL_BF( BF_UnpinBlock( block));
  // }

  printf("STIN CLOSE %d\n", file_desc);
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
  int bl_d = 0;
  for( int i = 0; i < depth; i++)
  {
    int a = 1;
    for(int j = 0; j< depth -i -1; j++)
    {
      a = a*2;
    }
    // printf("depth: %d pow is %d\n", depth, a);  
    bl_d = bl_d + (id%2)*a;
    id = id/2;
  }
  // id = record.id;
  // int bl_d1 = 0;
  // for( int i = 0; i < depth-1; i++)
  // {
  //   int a = 1;
  //   for(int j = 0; j< depth-1 -i -1; j++)
  //   {
  //     a = a*2;
  //   }
  //   // printf("depth: %d pow is %d\n", depth, a);  
  //   bl_d1 = bl_d1 + (id%2)*a;
  //   id = id/2;
  // }
  // printf("id= %d, bl_d = %d\n", record.id, bl_d);

  return bl_d;

}



//MHTSOOOOOOOOOOOOOOOO
//EDO FTIAKSE
//-1 KAI -2/KAI CREATE
//EXOUME BUG
HT_ErrorCode CreateNewBucket( int filedesc, Record record, int bucket){
  BF_Block* block;
  BF_Block_Init(&block);

  CALL_BF(BF_GetBlock(filedesc, 0, block));
  char* data = BF_Block_GetData( block);
  int global_depth = data[0];
  
  BF_AllocateBlock(filedesc, block);

  int blocks_num;
  BF_GetBlockCounter( filedesc, &blocks_num);
  int dest = blocks_num - 1;

  
  int previous_block = -1;
  int i = 1;
  CALL_BF(BF_GetBlock(filedesc, i, block));
  data = BF_Block_GetData(block);
  int f = 0;
  int* d = data + f*sizeof(int);
  int count = 0;
  
  printf("%d kai b %d\n",d[0], bucket);
  
  while(d[0] <= bucket){
    
    if(d[0] == bucket)
    {
      count++;
    }
        

    f++;
    d =  data+ f*sizeof(int);

    if(d > data + BF_BLOCK_SIZE-1)
    {
      
      CALL_BF(BF_GetBlock(filedesc, 0, block));
      char* data1 = BF_Block_GetData(block);
      int k = 1;
      char* d1;
      d1 = data1 + sizeof(int); 
      
      while(d1[0] <= i)
      {
        k++;
        d1 = data1 + k*sizeof(int);
      }

      previous_block = i;
      i = d1[0];
      CALL_BF(BF_GetBlock(filedesc, data1[0], block));
  
      //data einai i thieythisi toy protoy stoixeio toy teleytaioy block eksereunisis
      data = BF_Block_GetData(block);
      f = 0;
      d = data + f*sizeof(int);
    }
  printf("%d kai b %d\n",d[0], bucket);
    
  }

  
  //count einai poses theseis toy hash table deixnoyn sto bucket-problem
  int num;
  if( f == 0)
  {
    CALL_BF(BF_GetBlock(filedesc, previous_block, block));
    data = BF_Block_GetData(block);
    i = previous_block;
    num = 1;
  }
  else
  {  
    num = (BF_BLOCK_SIZE/sizeof(int))- f +1;
  }
  


  //me vlepeis;;;;
  //ok
  //0.....6
  //1o block -- plir
  //2o block -- hash
  //bucket 2
  //bucket 3
  //bucket 4
  //bucket 5
  //bucket 6
  //..
  //..
  //..
  //8o block -- hash
  for(int k = 0; k < count/2; k++){
    // printf("exo to num = %d KAI TO bucket: %d\n", num, bucket);
    

    // tha xoyme apla ena hash table me stoixeia 2^3
    // gt na pame apo to telos;;;;;;;
    d = data + (BF_BLOCK_SIZE - num*sizeof(int));

    //if num = 128 -- bf_block_size -num*sizeof(int) == 0 // oti eimaste sti proti thesi pano pano
    //if num = 129 -- bf_block_size -num*sizeof(int) == -4 //tha thelei to proiu

    if( BF_BLOCK_SIZE < num*sizeof(int)){
      // printf("hello un for count/2 \n");
      CALL_BF(BF_GetBlock(filedesc, 0, block));


      char* data1 = BF_Block_GetData(block);
      int k = 1;
      char* d1;
      d1 = data1 + sizeof(int); 

      //thes na vreis to proigoymeno hash blcok
      int find_previous = -1;
      printf("ela baino\n");
      while(d1[0] != i){
        printf("f_p:  %d %d\n", find_previous, i);
        find_previous = d1[0];
        k++;
        d1 = data1 + k*sizeof(int);
      }

      CALL_BF(BF_GetBlock(filedesc, find_previous, block));
      data = BF_Block_GetData(block);
      d = data + BF_BLOCK_SIZE - sizeof(int);
      num = 1;
    }
    num++;
    // printf("grafp stp : %d\n", d);
    memcpy(d, &dest, sizeof(int));
    // printf(" dest:  %d\n", dest);
    BF_Block_SetDirty(block);
  }
  // printf("all is good\n");

  CALL_BF(BF_GetBlock(filedesc, 1, block));
  data = BF_Block_GetData( block);
  // for( int i = 0; i < 8; i++)
  // {
  //     int* d = data + i*sizeof(int);
  //     printf("EDO EIMAI   %d data:  %d\n",d[0], d);

  // }


  ///////////////////////////////
  /////////////////////////////
  //////////////////////////
  ///////////////////////

  //TORA prepei na hasharo ksana tis times tou paliou kai neou block ksana
  //ara exo dyo block
  int old_bucket = bucket;
  int new_bucket = dest;

  // ena bucket xoraei to poli 8 eggrafes eipame
  // diavazp tis times aytes kai tis apothikeyo se ena pinaka pao record
  
  // strcpy(city, record.city);
  CALL_BF( BF_GetBlock(filedesc, old_bucket, block));


  //auksano to topiko vathos gia to palio block
  data = BF_Block_GetData( block);
  int topic_depth = data[0] + 1;

  memcpy( data, &topic_depth, sizeof(int));
  BF_Block_SetDirty( block);

  //grafo to topiko vahtos gia to neo block
  //pou einai oso to topic_depth
  //tou old_bucket
  CALL_BF( BF_GetBlock(filedesc, new_bucket, block));
  data = BF_Block_GetData( block);

  memcpy( data, &topic_depth, sizeof(int));
  BF_Block_SetDirty( block);


  //ksana sto old_bucket
  CALL_BF( BF_GetBlock(filedesc, old_bucket, block));
  data = BF_Block_GetData( block) + sizeof(int);
  int* d1;
  int k = 0;

  //ta palia kai to kainoyrio record
  Record rec[ (BF_BLOCK_SIZE -sizeof(int))/sizeof(record) +1];
  
  int id;
  char name[15];
  char surname[20];
  char city[20];
  //read all record and save in the table rec
  while( k < (BF_BLOCK_SIZE-sizeof(int))/sizeof(record))
  {   

      d1 = data + k*sizeof(record);
      rec[k].id = d1[0];

      char* d2 = data + k*sizeof(record) + sizeof(int);
      strcpy( rec[k].name, d2);

      // memcpy( data, &, sizeof(record));

      char* d3 = data + k*sizeof(record) + sizeof(int) + sizeof(name);
      strcpy( rec[k].surname, d3);

      char* d4 = data + k*sizeof(record) + sizeof(int) + sizeof(name) + sizeof(surname);
      strcpy( rec[k].city, d4);
      
      if( rec[k].name == NULL || strlen( rec[k].name) == 0)
      {
        break;
      }

      // printf("to record mas me stoixeia :%d, %s, %s, %s\n", rec[k].id, rec[k].name, rec[k].surname, rec[k].city);
      k++;
      
  }
  //tora tha valo kai tin teleytaia eggrafi poy einai i nea mas eggrafi
  int final = (BF_BLOCK_SIZE -sizeof(int))/sizeof(record);
  rec[final].id = record.id;
  strcpy( rec[final].name, record.name);
  strcpy( rec[final].surname, record.surname);
  strcpy( rec[final].city, record.city); 

  //after i will delete all record from this block
  data = BF_Block_GetData( block) + sizeof(int); //because the fisth 4 bytes is the topic depth
  int delete = NULL;
  int j = 0;
  while( data + j*sizeof(int) < data + BF_BLOCK_SIZE -1)
  { 
    memcpy( data + j*sizeof(int), &delete, sizeof(int));
    BF_Block_SetDirty( block);
    j++;  
  }

  //gia tis palies eggrafes
  Record record_old;

  //tha ksanahasaro tis times
  int num_old = 0;
  int num_new = 0;

  for( int i = 0; i < (BF_BLOCK_SIZE -sizeof(int))/sizeof(record) + 1; i++)
  {
    record_old.id = rec[i].id;
    strcpy( record_old.name, rec[i].name);
    strcpy( record_old.surname, rec[i].surname);
    strcpy( record_old.city, rec[i].city);

    int HashNum = HashFunction( record_old, global_depth);

    int j = 1;
    CALL_BF(BF_GetBlock(filedesc, j, block));
    data = BF_Block_GetData( block);
  
    while( data+ HashNum*sizeof(int) > data+BF_BLOCK_SIZE-1 )
    {
      j++;
      CALL_BF(BF_GetBlock(filedesc, j, block));
      data = BF_Block_GetData( block);

      HashNum-=128;
    }

    int* d = data + HashNum*sizeof(int);

    int bucket_from_hash = d[0];

    BF_GetBlock( filedesc, bucket_from_hash, block);
    data = BF_Block_GetData( block) + sizeof(int);
    d = data;

    //vazo tis times
    id = record_old.id;
    strcpy( name, record_old.name);
    strcpy( surname, record_old.surname);
    strcpy( city, record_old.city);

    
    if( bucket_from_hash == old_bucket)
    {
      //id
      memcpy( data + num_old*sizeof(record) , &id, sizeof(int));
      BF_Block_SetDirty(block);

      //name
      memcpy( data + num_old*sizeof(record) + sizeof(int) , name, sizeof(name));
      BF_Block_SetDirty(block);

      //surname
      memcpy( data + num_old*sizeof(record) + sizeof(int) + sizeof(name) , surname, sizeof(surname));
      BF_Block_SetDirty(block);

      //surname
      memcpy( data + num_old*sizeof(record) + sizeof(int) + sizeof(name) + sizeof(surname), city, sizeof(city));
      BF_Block_SetDirty(block);

      num_old++;
    }
    else
    {
      //id
      memcpy( data + num_new*sizeof(record) , &id, sizeof(int));
      BF_Block_SetDirty(block);

      //name
      memcpy( data + num_new*sizeof(record) + sizeof(int) , name, sizeof(name));
      BF_Block_SetDirty(block);

      //surname
      memcpy( data + num_new*sizeof(record) + sizeof(int) + sizeof(name) , surname, sizeof(surname));
      BF_Block_SetDirty(block);

      //surname
      memcpy( data + num_new*sizeof(record) + sizeof(int) + sizeof(name) + sizeof(surname), city, sizeof(city));
      BF_Block_SetDirty(block);

      num_new++;
    }
  }
  printf("new %d old %d\n", num_old, num_new);
  
  //PRINT OLDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD
  k = 0;
  printf("ALL FOR OLD: %d\n", old_bucket);
  BF_GetBlock(filedesc, old_bucket, block);
  data = BF_Block_GetData( block) + sizeof(int);
  while( k < (BF_BLOCK_SIZE-sizeof(int))/sizeof(record))
  {   

      d1 = data + k*sizeof(record);
      int id = d1[0];


      char* d2 = data + k*sizeof(record) + sizeof(int);
      // char* nam = d2[0];
      // char nam[15];
      strcpy(name, d2);
      // printf(" elaa %s\n", d2[0]);

      char* d3 = data + k*sizeof(record) + sizeof(int) + sizeof(name);
      // char* snam = d1[0];
      strcpy(surname, d3);

      char* d4 = data + k*sizeof(record) + sizeof(int) + sizeof(name) + sizeof(surname);
      // char* cit = d1[0];
      strcpy(city, d4);

      if( name == NULL || strlen( name) == 0)
      {
        printf("ta pameeeeeeeeeeeeeeeeeeeeeee\n");
        break;
      }
      // printf("to record mas me stoixeia :%d, %s, %s, %s\n", id, name, surname, city);

      k++;
      
  }



    //PRINT NEWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW
  k = 0;
  // printf("ALL FOR NEW: %d\n", new_bucket);
  BF_GetBlock(filedesc, new_bucket, block);
  data = BF_Block_GetData( block) + sizeof(int);
  while( k < (BF_BLOCK_SIZE-sizeof(int))/sizeof(record))
  {   

      d1 = data + k*sizeof(record);
      int id = d1[0];


      char* d2 = data + k*sizeof(record) + sizeof(int);
      // char* nam = d2[0];
      // char nam[15];
      strcpy(name, d2);
      // printf(" elaa %s\n", d2[0]);

      char* d3 = data + k*sizeof(record) + sizeof(int) + sizeof(name);
      // char* snam = d1[0];
      strcpy(surname, d3);

      char* d4 = data + k*sizeof(record) + sizeof(int) + sizeof(name) + sizeof(surname);
      // char* cit = d1[0];
      strcpy(city, d4);

      if( name == NULL || strlen( name) == 0)
      {
        printf("ta pameeeeeeeeeeeeeeeeeeeeeee\n");
        break;
      }
      // printf("to record mas me stoixeia :%d, %s, %s, %s\n", id, name, surname, city);

      k++;
      
  }


  BF_Block_Destroy( &block);
  // printf("o[a\n");
  return HT_OK; 
}


HT_ErrorCode CreateNewHashTable( int filedesc, Record record, int bucket_b)
{ 
  BF_Block* block;
  BF_Block_Init( &block);

  int blocks_num;
  BF_GetBlockCounter( filedesc, &blocks_num);
  if(blocks_num <  3)  { return HT_ERROR;}
  
  CALL_BF( BF_GetBlock(filedesc, 0, block));

  char* data;
  data = BF_Block_GetData(block);

  int depth = data[0];
  // depth = 6;
  printf("EXO dep: %d\n", depth);
  int a = 1;
  for( int i = 0; i < depth; i++)
  {
    a = a*2;
  }
  //prepei na vro posa block eixa gia to hash table
  //1o block ------- 127 theseis gia hash
  //2o block ------- 128 theseis gia hash
  //3o block ------- 128 theseis gia hash
  //...
  //...
  //...
  //N-osto block --- 128 theseis gia hash
  int num_block_hash_before;

  //PREVIOUS
  int num_of_ints = BF_BLOCK_SIZE/sizeof(int);
  num_block_hash_before = a/num_of_ints;
  if(a%num_of_ints > 0)
    num_block_hash_before++;
  // printf("other:  %d\n", num_block_hash_before);

   
  //FOR NEW HASh
  depth++;
  int previous_a = a;
  int previous_depth = depth - 1;
  a = a*2;
  int num_block_hash_new;
  
  //AFTERRR
  num_block_hash_new = a/num_of_ints;
  if(a%num_of_ints > 0)
    num_block_hash_new++;

  // printf("other:  %d\n", num_block_hash_new);

  int different = num_block_hash_new - num_block_hash_before;
  int i = 0;
  //edo tha isxyei mono oso to kainourio depth < 6 ti shmaieni ayto;
  // ayto shmainei to 2^6 = 64 < 127 ---- ayto shmainei oti eimaste
  //2^5 = 32 ---- posa block = 1
  //2^6 = 64 ---- posa block = 1
  //2^7 = 128 --- posa block = 2
  //2^8 = 256 ---- posa block = 3
  //2^9 = 512 ---- posa block = 5
  if(different == 0)
  {

    int bucket = blocks_num - num_block_hash_new;
    //γραφω στην αρχη του 1ου μου μπλοκ
    //το νεο μας βαθος
    


    //diavazoume to hashtable
    //apothikeoume se statiko pinaka se pio kado deixnei kath thesi
    //unpin block
    //sto idio block
    //grafoume vathos proto
    //kai meta to kainourio hash table kai hasharoume se pio apo ta yparkta bucket deixnei
    //char* data = BF_Block_GetData( block) + sizeof(int);
    CALL_BF( BF_GetBlock(filedesc, 1, block));
    data = BF_Block_GetData(block); 

    int prices[ previous_a];
    for( int i = 0; i < previous_a; i++)
    {
      int* d = data + i*sizeof(int);
      // printf("%d\n",d[0]);
      prices[i] = d[0];
    }      
    // CALL_BF( BF_UnpinBlock( block));
    


    // int new_depth = depth;
    // memcpy(data, &new_depth, sizeof(int));
    // BF_Block_SetDirty( block); 

    //0 1 2 3
    // d = 2
    //00 01 10 11

    //d =3
    ///000 001 010 011 
    ///100 101 110 111

    //d = 4
    //0000 0001 0010 0011 
    //0100 0101 0101 0111
    //1000 1001 1010 1011
    //1100 1101 1110 1111


    //0000 = 000
    //0001 = 000
    //0100 = 010

    //previous depth= 3
    //new depth = 4
    
    //2^2 = thseis = 4
    //2^3 = theseis = 2^2 + 4
    //2^4 = theseis = 2^3 + 8
    //2^5 = theseis = 2^4 + 16 = 16+16

    // int nums = previous_a/2; // = 2

    //010; // theloume na ginei 01 = 1
            //100; //theloume to 10 = 2
            //011; //thloume 01  = 1       
    //0....7 na paroun timi 0..3
    // i >> ;
    //i = 0
    //i = 1
    //i = 2 -1 = 1
    //i = 3 
    //τωρα πρεπει να επεκτεινουμε το hashtable

    CALL_BF( BF_GetBlock(filedesc, 0, block));
    data = BF_Block_GetData(block);
    int new_depth = depth;
    memcpy(data, &new_depth, sizeof(int));
    BF_Block_SetDirty(block);

    CALL_BF( BF_GetBlock(filedesc, 1, block));
    data = BF_Block_GetData(block);
    num_of_ints = 0;
    
    while (num_of_ints < a)       
    {
      int value = num_of_ints >> 1;
      int v = prices[value];
      printf("num_of_int = %d   value = %d\n",num_of_ints, v);
      memcpy(data, &v, sizeof(int));
      BF_Block_SetDirty(block);
      num_of_ints++;
      data += sizeof(int);
    } 

  }
  else
  {
    //eimaste sti periptosi poy depth >= 7
    //2^7 = 128 --- posa block = 2
    //2^8 = 256 ---- posa block = 3
    //2^9 = 512 ---- posa block = 5
    //2^10 = 1024 -- posa block =  8


    //2^ 7 + 1 block hash
    //na epanalavoume to pano gia to 1o 
    //kai meta na grapsoyme sto 2o block to hash_table_ upoloipo

  }

  CreateNewBucket( filedesc, record, bucket_b);
  printf("trolllaro\n");
  BF_Block_Destroy( &block);
  return HT_OK;
}



HT_ErrorCode HT_InsertEntry(int indexDesc, Record record) {
  //insert code here

  //παιρνω τον αριθμο αρχειου ωστε να το βρω
  int filedesc = filetable->table[indexDesc]->file_desc;
  BF_Block* block;
  BF_Block_Init( &block);

  //ποσα μπλοκ εχει το υπαρχον αρχειο
  int blocks_num;
  CALL_BF( BF_GetBlockCounter( filedesc, &blocks_num));
  
  //με την προυποθεση οτι το βαθος θα ειναι μεγαλυτερο ή ισο με 1
  if( blocks_num < 3)
  {
    return HT_ERROR;
  }


  int i = 0;

  //GetBlock with data
  //το πρωτο μπλοκ εχει τις πληροφοριες
  CALL_BF(BF_GetBlock(filedesc, i, block));
  char* data;
  data = BF_Block_GetData( block);

  
  //global depth 
  int depth = data[0];
  
  //Hashing
  int HashNum = HashFunction( record, depth);

  int* d;
  int num_block_hash = 3;

  //το 2ο μπλοκ του αρχειου ειναι το 1ο μπλοκ του ευρετηριου
  i = 1;
  int num_info = 1;
  int block_info = 0;
  CALL_BF(BF_GetBlock(filedesc, i, block));
  data = BF_Block_GetData( block);
printf("hasharei %d - insert\n", HashNum);
  //Αν η θεση του πινακα που χασαρουμε δεν ειναι στο 1ο μπλοκ ευρετηριου τοτε παμε στο
  //επομενο και ουτε καθεξης
  while( data+ HashNum*sizeof(int) > data+BF_BLOCK_SIZE-1 )
  { 
    //αρα διαβαζουμε απο το μπλοκ πληροφοριες ποιο ειναι το επομενο
    CALL_BF( BF_GetBlock( filedesc, block_info, block));
    //παιρνουμε τον αριθμο του επομενου μπλοκ ευρετηριου
    char* data1 = BF_Block_GetData( block) + sizeof(int) + num_info*sizeof(int);
    int* d1 = data1;
    //αν ειναι -1 ειναι το τελος και εχουμε ερρορ
    if ( d1[0] == -1)
    {
      return HT_ERROR;
    }
    else if( d1[0] == -2)
    {
      //αν ειναι ισο με -2 σημαινει οτι ειναι το πρωτελευταιο στοιχειο του
      //μπλοκ πληροφοριες και οτι το επομενο στοιχειο ειναι το μπλοκ που συνεχιζεται το μπλοκ
      //πληροφοριων

      d1 = data1 + sizeof(int);
      block_info = d1[0];
      CALL_BF( BF_GetBlock( filedesc, block_info, block));
      data1 = BF_Block_GetData( block) + sizeof(int);
      num_info = 0;
      d1 = data1;
    }
    num_info++;
    i = d1[0];

    CALL_BF(BF_GetBlock(filedesc, i, block));
    data = BF_Block_GetData( block);
    HashNum = HashNum - 128;

  }

  //παιρνουμε σε ποιο μπλοκ χασαρει τελικα
  d = data + HashNum*sizeof(int);
  
  printf("ecooo %d - insert\n", d[0]);
  
  int bucket = d[0];

  //παιρνουμε τον αριθμο των μπλοκς
  int num_blocks;
  BF_GetBlockCounter( filedesc, &num_blocks);
  
  BF_GetBlock( filedesc, bucket, block);
  data = BF_Block_GetData( block) + sizeof(int);
  int k = 0;
  
  //μετραω ποσα στοιχεια εχει μεσα το μπλοκ μου
  while( k < (BF_BLOCK_SIZE - sizeof(int))/sizeof(record))
  {
  

      char name[15];
      char* d1 = data + k*sizeof(record) + sizeof(int);
      strcpy( name, d1);
      if(name == NULL || strlen( name) == 0)
      { 
        break;
      }
      k++;
      
  }

  //ξεχωριζω τα πεδια του record
  int id = record.id;
  char name[15];
  strcpy(name, record.name);

  char surname[20];
  strcpy(surname, record.surname);
  
  char city[20];
  strcpy(city, record.city);


  data = BF_Block_GetData( block) + sizeof(int);
  //Εχουμε δυο περιπτωσεις η μια να χωραει στο μπλοκ μας
  //και η αλλη να μην χωραει
// printf("\n\n\n k einai %d\n\n\nSS",k);
  if( k == (BF_BLOCK_SIZE-sizeof(int))/sizeof(record))
  { 
    //η περιπτωση που το μπλοκ μας ειναι γεματο
    printf("Eimai mesa sto if oste na kano epektasi hashtable - InsertEntry\n");
    data = BF_Block_GetData(block);
    int depth_bucket = data[0];

    printf("Global_d = %d kai Topic_d = %d - InsertEntry\n", depth, depth_bucket);
    //εφαρμοζουμε τον αλγοριθμο αναλογα το τοπικο και ολικο βαθος
    if( depth == depth_bucket)
    { 
      CreateNewHashTable( filedesc, record, bucket);
    }
    else
    {
      CreateNewBucket( filedesc, record, bucket);
    }
  
  }
  //αν χωραει στο μπλοκ η εγγραφη τοτε την χασαρουμε
  else
  {
    //id
    memcpy( data + k*sizeof(record) , &id, sizeof(int));
    BF_Block_SetDirty(block);

    //name
    memcpy( data + k*sizeof(record) + sizeof(int) , name, sizeof(name));
    BF_Block_SetDirty(block);

    //surname
    memcpy( data + k*sizeof(record) + sizeof(int) + sizeof(name) , surname, sizeof(surname));
    BF_Block_SetDirty(block);

    //surname
    memcpy( data + k*sizeof(record) + sizeof(int) + sizeof(name) + sizeof(surname), city, sizeof(city));
    BF_Block_SetDirty(block);
    // printf("to record mas me stoixeia :%d, %s, %s, %s\n", id, name, surname, city);


  }

  BF_Block_Destroy( &block);
  printf("all is good with insert - end - InsertEntry\n");
  return HT_OK;
}

HT_ErrorCode HT_PrintAllEntries(int indexDesc, int *id) {
  //insert code here
  return HT_OK;
}