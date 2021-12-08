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
  char* data;
 
  //Πρεπει να το ξαναδω αν σε ενοχλεί βάλτο σε σχόλια μην το σβήσεις.
  int i = 0;
  int num_of_ints = 0;
  data = BF_Block_GetData(block); //pairno to data

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


  // print all elements
  // PRINTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
  data = BF_Block_GetData( block);
  for( int i = 0; i < 5; i++)
  {
      int* d = data + i*sizeof(int);
      printf("EDO EIMAI   %d data:  %d\n",d[0], d);

  }

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
  return id;

}

HT_ErrorCode CreateNewBucket( int filedesc, Record record)
{
  BF_Block* block;
  BF_Block_Init(&block);

  CALL_BF(BF_GetBlock(filedesc, 0, block));
  BF_AllocateBlock(filedesc, block);

  int blocks_num;
  BF_GetBlockCounter( filedesc, &blocks_num);

  return HT_OK; 
}

HT_ErrorCode CreateNewHashTable( int filedesc, Record record)
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
  depth = 5;
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
  num_block_hash_before = (a+1)/num_of_ints;
  if((a+1)%num_of_ints > 0)
    num_block_hash_before++;
  // printf("other:  %d\n", num_block_hash_before);

   
  //FOR NEW HASh
  depth++;
  int previous_a = a;
  int previous_depth = depth - 1;
  a = a*2;
  int num_block_hash_new;
  
  //AFTERRR
  num_block_hash_new = (a+1)/num_of_ints;
  if((a+1)%num_of_ints > 0)
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
    data+=sizeof(int);
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

    data = BF_Block_GetData(block);
    int new_depth = depth;
    memcpy(data, &new_depth, sizeof(int));
    BF_Block_SetDirty(block);
    data += sizeof(int);
    num_of_ints = 0;
    
    while (num_of_ints < a)       
    {
      int value = num_of_ints >> 1;
      int v = prices[value];
      // printf("num_of_int = %d   value = %d\n",num_of_ints, v);
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

  CreateNewBucket( filedesc, record);
  return HT_OK;
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


  int i = 0;

  //GetBlock with data
  CALL_BF(BF_GetBlock(filedesc, i, block));
  char* data;
  data = BF_Block_GetData( block);

  
  //depth 
  int depth = data[0];
  
  //Hashing
  int HashNum = HashFunction( record, depth);

  int a = 1;
  for( int i = 0; i < depth; i++)
  {
    a = a*2;
  }

  //paikse me bathosssssssssssssssssssssssssssss 7...............
  int* d;
  int num_block_hash = 2;
  HashNum++; //128
  while( data+ HashNum*sizeof(int) > data+BF_BLOCK_SIZE-1 )
  {
    i++;
    CALL_BF(BF_GetBlock(filedesc, i, block));
    data = BF_Block_GetData( block);
    if( i == 1)
    {
      HashNum = HashNum - 127;
    }
    else
    {
      HashNum = HashNum - 128;
    }
    num_block_hash++;

  }

  
  d = data + HashNum*sizeof(int);
  int bucket = d[0];
  int num_blocks;
  BF_GetBlockCounter( filedesc, &num_blocks);
  BF_GetBlock( filedesc, num_block_hash + bucket -1 , block);
  data = BF_Block_GetData( block);
  d = data + sizeof(int);
  int k = 0;

  while( k < (BF_BLOCK_SIZE-sizeof(int))/sizeof(record))
  {
      d = data + k*sizeof(record) + sizeof(int);
      char* nam1 = d[0];
      if( nam1 == NULL)
      {
        break;
      }
      k++;
      
  }

  int id = record.id;
  char name[15];
  strcpy(name, record.name);

  char surname[20];
  strcpy(surname, record.surname);
  
  char city[20];
  strcpy(city, record.city);


  data = BF_Block_GetData( block) + sizeof(int);
  if( k == (BF_BLOCK_SIZE-sizeof(int))/sizeof(record))
  {
    //tote edo exoume thema me to block
    //eksetazoume periptoseis'
    printf("kalinictaaaa\n");
    data = BF_Block_GetData(block);
    int depth_bucket = data[0];
    printf("edo to d:  %d\n", depth_bucket);


    //ola mas ta bucket exoun bathos ---- 2
    //to oliko bathos poso einai;; ------ 2
    //paei na grapsei se bucket kai den exei xoro ti kanei;; --- New hash
    //
    //to kainourio hash ti bathos exei;; ---- 3
    //meta ti kanei;;; ------------------- split to bucket poy einai na bei h eggrafi
    //ara meta afoy hasharo exo oliko vathos < tobiko vathos toy kadoy provlima
    //
    // oliko vathos = 3 kai topiko vathos = 2
    //ANAGOMASTE META STO CREATENEWBUCKET
    //ginetai to split to kadou provlima
    //diaxortismos ton deiktwn sto hash table
    //
    //epeaypologismos eggrafon gia oles tis palies kai thn nea
    //gia kado provlima
    //
    //topiko vathos = topiko vathos + 1 gia ta 2 ayta bucket
    //
    //to kathe bucket pos vathos exei;; ------ apo ayta ta 2 -- 3
    //
    //
    //
    //
    //
    //
    //
    //
    //
    if( depth == depth_bucket)
    {
      CreateNewHashTable( filedesc, record);
    }
    else
    {
      CreateNewBucket( filedesc, record);
    }
    return HT_ERROR;
  
  }
  //AN DN EINAI gemato to block mas poy ginetai to hashing tote vazoume to record
  //kai den exoyme kanena thema
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
  }


  // // //PRINTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
  // data = BF_Block_GetData( block) + sizeof(int);
  // int* d1;
  // k = 0;
  // while( k < (BF_BLOCK_SIZE-sizeof(int))/sizeof(record))
  // {   

  //     d1 = data + k*sizeof(record);
  //     int id = d1[0];


  //     char* d2 = data + k*sizeof(record) + sizeof(int);
  //     // char* nam = d2[0];
  //     // char nam[15];
  //     strcpy(name, d2);
  //     // printf(" elaa %s\n", d2[0]);

  //     // d1 = data + k*sizeof(record) + sizeof(int) + sizeof(name);
  //     // char* snam = d1[0];


  //     // d1 = data + k*sizeof(record) + sizeof(int) + sizeof(name) + sizeof(surname);
  //     // char* cit = d1[0];
      
  //     if( name == NULL || strlen( name) == 0)
  //     {
  //       break;
  //     }
  //     printf("to record mas me stoixeia id: %d kai name %s\n", id, name);
  //     k++;
      
  // }

  

  // printf("num bllll:  gioxann:    %d --- tsosmi: %d\n", num_of_hashblocks, num_block_hash);
  BF_Block_Destroy( &block);
  return HT_OK;
}

HT_ErrorCode HT_PrintAllEntries(int indexDesc, int *id) {
  //insert code here
  return HT_OK;
}
      // // CALL_BF( BF_AllocateBlock( file_desc, block));
      //   // data = BF_Block_GetData( block);
      //   // i = 0;
  
      // memcpy( data + i*sizeof(int), &bucket_point, sizeof( int));
      // BF_Block_SetDirty(block);

      // num_of_ints++;
      // i++;