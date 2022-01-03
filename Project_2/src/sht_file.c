#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "bf.h"
#include "sht_file.h"

#define MAX_OPEN_FILES 20




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
  data = BF_Block_GetData( block);
  
  //save filename
  memcpy( data, fileName, 20);
  BF_Block_SetDirty( block);
  // printf("%s\n", fileName);


  //save attr name
  memcpy(data + 20, attrName, sizeof(attrName) );
  BF_Block_SetDirty( block);

  //save attr lengh
  memcpy( data + 20 + sizeof(attrName), &attrLength, sizeof(int));
  BF_Block_SetDirty( block);

  CALL_BF( BF_UnpinBlock( block));


  CALL_BF( BF_AllocateBlock( file_desc, block));

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

//Η συναρτηση που χασαρει εγγραφες αναλογα με το βαθος μας
int SHT_HashFunction( SecondaryRecord srecord, int depth)
{
  int id = 0;
  for(int i = 0; i < strlen(srecord.index_key); i++){
    id += (int)srecord.index_key[i];
  }

  int bl_d = 0;
  for( int i = 0; i < depth; i++)
  {
    int a = 1;
    for(int j = 0; j< depth -i -1; j++)
    {
      a = a*2;
    }
    bl_d = bl_d + (id%2)*a;

    id = id/2;
  }

  return bl_d;

}

HT_ErrorCode SHT_CreateNewBucket( int filedesc, SecondaryRecord record, int bucket)
{


  //init for block
  BF_Block* block;
  BF_Block_Init(&block);

  //to eyretirio mas arxizei apo to block = 0
  int block_info = 1;
  CALL_BF(BF_GetBlock(filedesc, block_info, block));
  char* data = BF_Block_GetData( block);
  
  int global_depth = data[0];   //global depth
  CALL_BF( BF_UnpinBlock( block));


  //take all blocks_num
  int blocks_num;
  CALL_BF( BF_GetBlockCounter( filedesc, &blocks_num));

  //take num for the last block where is the new block
  int dest = blocks_num;

  
  int previous_block = -1;
  
  //to eyretirio mas hashblock arxizei apo to 3o block toy arxeio me i = 2
  int i = 2;
  CALL_BF(BF_GetBlock(filedesc, i, block));
  data = BF_Block_GetData(block);
  
  int f = 0;
  //pairno to proto stoixeio toy pinaka kai to perieomeno toy diladi se poio block deixnei
  int* d = data + f*sizeof(int);
  int count = 0;                //to count metraei poses theseis tou pinaka deixnoyn sto bucket provlima
  int num_info = 1;            //eimai sto block info poy deixnei se kapoio eyretirio


  //apothikeyo tis plirofories gia to proto block
  int* first_data = NULL;
  int first_hashtable_num = 2;
  int first_information_block = 1;
  bool first_find = false;
  int first_f = NULL;
  int first_num_info = num_info;

  while(1){

    //ta bucket mas dn einai arithmimena pros kapoia kateythinsi mesa sto eyretirio
    //ara psaxno otan vro to proto toe kano first_find = true
    //oi epomenes x(agnostes) theseis deixnoun profanos sto idio bucket mexri to proto poy tha diaferei
    //ara otan vroume ayto to diaforetiko( kai to first_find = true) tote spame tin epanalipsi
    if(d[0] == bucket)
    {
      if( first_find == false)
      {

        first_data = d;
        first_f = f;
        first_find = true;
        first_hashtable_num = i;
        first_num_info = num_info;
        first_information_block = block_info; 
      }
      count++;
    }
    else
    {
      if(first_find == true)
      {
        break;
      }
    }
        

    f++;
    d =  data+ f*sizeof(int);
    
    //an vgoume ektos block
    //tote prepei na vroume to epomeno hashblock poy synexizei to eyretirio mas
    if (d > data + BF_BLOCK_SIZE - 1)
    {
      //pao sto block info poy vriskomai
      CALL_BF( BF_UnpinBlock( block));
      CALL_BF(BF_GetBlock(filedesc, block_info, block));
      int k = 1;

      char *data1;
      //παιρνουμε τον αριθμο του επομενου μπλοκ ευρετηριου
      if( block_info == 1)
      {
        data1 = BF_Block_GetData(block) + sizeof(int) + num_info*sizeof(int);
      }
      else
      {
        //epeidi to proto stoixeio einai kapoio hash block kai oxi to vathos
        data1 = BF_Block_GetData(block)  + num_info*sizeof(int);
      }
      int *d1 = data1;

      //αν ειναι -1 ειναι το τελος και εχουμε ερρορ
      if (d1[0] == -1)
      {
        break;
      }
      else if (d1[0] == -2)
      {
        //αν ειναι ισο με -2 σημαινει οτι ειναι το πρωτελευταιο στοιχειο του
        //μπλοκ πληροφοριες και οτι το επομενο στοιχειο ειναι το μπλοκ που συνεχιζεται το μπλοκ
        //πληροφοριων

        d1 = data1 + sizeof(int);
        block_info = d1[0];

        CALL_BF( BF_UnpinBlock( block));
        CALL_BF(BF_GetBlock(filedesc, block_info, block));
        data1 = BF_Block_GetData(block);
        num_info = 0;
        d1 = data1;
      }
      num_info++;
      previous_block = i;
      i = d1[0];

      CALL_BF( BF_UnpinBlock( block));
      CALL_BF(BF_GetBlock(filedesc, i, block));

      //data einai i thieythisi toy protoy stoixeio toy teleytaioy block eksereunisis
      data = BF_Block_GetData(block);
      f = 0;
      d = data + f*sizeof(int);
    }
    
  }

  //error
  if(count == 0)
  {
    BF_Block_Destroy( &block);
    return HT_ERROR;
  }
  
  //count einai poses theseis toy hash table deixnoyn sto bucket-problem
  int num;

  num = first_f;    

  CALL_BF( BF_UnpinBlock( block));
  
  CALL_BF( BF_GetBlock(filedesc, first_hashtable_num, block));  //pairno to block poy ksekina na deixnei sto bucket provlima
  data = BF_Block_GetData( block);                //pairno tin dieythisni tis protis thesis tou eyretirioy atri
  block_info = first_information_block;
  num_info = first_num_info;

  //kano ta teleytaia apo to telos count/2 block anti na deixnoun
  //sto bucket_provlima na deixnoun sto kainourio dhladh ------------> dest
  for(int k = 0; k < count/2; k++){
    
    //vrisko tin dieythinsi pou vrisketai h proti mas thesi me busket_problem
    char* data11 = data + num*sizeof(int);

    //an omos ftaso sto telos tou block_eyretiriou kai vgo ektos orioy
    //prepei na pao sto epomeno  
    if( data11 > data + BF_BLOCK_SIZE -1){
      
      //pairno apo to block_info pou einai to hashtable poy vriskomai kai pao ena brosta hashblock-eyretirio
      CALL_BF( BF_UnpinBlock( block));
      CALL_BF(BF_GetBlock(filedesc, block_info, block));



      char *data1;
      //παιρνουμε τον αριθμο του επομενου μπλοκ ευρετηριου
      if( block_info == 1)
      {
        data1 = BF_Block_GetData(block) + sizeof(int) + num_info*sizeof(int);
      }
      else
      {
        //epeidi to proto stoixeio einai kapoio hash block kai oxi to vathos
        data1 = BF_Block_GetData(block)  + num_info*sizeof(int);
      }

      int *d1 = data1;

      //αν ειναι -1 ειναι το τελος και εχουμε ερρορ
      if (d1[0] == -1)
      {
        break;
      }
      else if (d1[0] == -2)
      {
        //αν ειναι ισο με -2 σημαινει οτι ειναι το πρωτελευταιο στοιχειο του
        //μπλοκ πληροφοριες και οτι το επομενο στοιχειο ειναι το μπλοκ που συνεχιζεται το μπλοκ
        //πληροφοριων

        d1 = data1 + sizeof(int);
        block_info = d1[0];

        CALL_BF( BF_UnpinBlock( block));
        CALL_BF(BF_GetBlock(filedesc, block_info, block));
        data1 = BF_Block_GetData(block);
        num_info = 0;
        d1 = data1;
      }
      num_info++;
      i = d1[0];

      CALL_BF( BF_UnpinBlock( block));
      CALL_BF(BF_GetBlock(filedesc, i, block));
      data = BF_Block_GetData(block);

      num = 0;
      data11 = data + num*sizeof(int);
    }

    if( dest < 0)
    {
      return HT_ERROR;
    } 

    memcpy(data11, &dest, sizeof(int));
    BF_Block_SetDirty(block);
    num++;
  }
  CALL_BF( BF_UnpinBlock( block));
 
  //TORA prepei na hasharo ksana tis times tou paliou kai neou block ksana
  //ara exo dyo block

  int old_bucket = bucket;
  int new_bucket = dest;
  // ena bucket xoraei to poli 8 eggrafes eipame
  // diavazp tis times aytes kai tis apothikeyo se ena pinaka pao record
  
  CALL_BF( BF_GetBlock(filedesc, old_bucket, block));


  //auksano to topiko vathos gia to palio block
  data = BF_Block_GetData( block);
  int* num_depth = data;
  int topic_depth;
  topic_depth = num_depth[0] + 1;

  memcpy( data, &topic_depth, sizeof(int));
  BF_Block_SetDirty( block);
  CALL_BF( BF_UnpinBlock( block));
  
  //grafo to topiko vahtos gia to neo block
  //pou einai oso to topic_depth
  //tou old_bucket 
  
  //////////////////////////////////////////////////////////////////////////////////////////
  CALL_BF(BF_AllocateBlock( filedesc, block));
  /////////////////////////////////////////////////////////////////////////////////////////
  
  CALL_BF( BF_GetBlockCounter( filedesc, &blocks_num));
  data = BF_Block_GetData( block);

  memcpy( data, &topic_depth, sizeof(int));
  BF_Block_SetDirty( block);
  CALL_BF( BF_UnpinBlock( block));


  //ksana sto old_bucket
  CALL_BF( BF_GetBlock(filedesc, old_bucket, block));
  data = BF_Block_GetData( block) + sizeof(int);
  int* d1;
  int k = 0;

  //ta palia kai to kainoyrio record
  SecondaryRecord rec[ (BF_BLOCK_SIZE -sizeof(int))/sizeof(SecondaryRecord) +1];
  
  char key[20];
  TupleId tid;
  int bucket_block;
  int index_block;
  


  //read all record and save in the table rec
  while( k < (BF_BLOCK_SIZE -sizeof(int))/sizeof(SecondaryRecord))
  {   

      d1 = data + k*sizeof(SecondaryRecord);
      strcpy( rec[k].index_key, d1);

      char* d2 = data + k*sizeof(SecondaryRecord) + sizeof(key);
      rec[k].tupleId.block = d2[0];

      char* d3 = data + k*sizeof(SecondaryRecord) + sizeof(key) + sizeof(int);
      rec[k].tupleId.index = d3[0];
      
      
      if( rec[k].index_key == NULL || strlen( rec[k].index_key) == 0)
      {
        break;
      }

      k++;
      
  }

  //tora tha valo kai tin teleytaia eggrafi poy einai i nea mas eggrafi
  int final = (BF_BLOCK_SIZE -sizeof(int))/sizeof(SecondaryRecord);
  
  strcpy( rec[final].index_key, record.index_key);
  rec[final].tupleId.block = record.tupleId.block;
  rec[final].tupleId.index = record.tupleId.index;
  
  
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
  SecondaryRecord record_old;

  //tha ksanahasaro tis times
  int num_old = 0;
  int num_new = 0;


  for( int i = 0; i < (BF_BLOCK_SIZE -sizeof(int))/sizeof(record) + 1; i++)
  {
    record_old.tupleId.block = rec[i].tupleId.block;
    record_old.tupleId.index = rec[i].tupleId.index;
    strcpy( record_old.index_key, rec[i].index_key);

    int HashNum = SHT_HashFunction( record_old, global_depth);

    block_info = 1;
    int block_hash = 2;
    int num_from_info_to_hash = 2;
    CALL_BF( BF_UnpinBlock( block));    // to old bucket
    CALL_BF(BF_GetBlock(filedesc, block_hash, block)); 
    data = BF_Block_GetData( block);

    int* d1;
  
    while( data+ HashNum*sizeof(int) > data+BF_BLOCK_SIZE-1 )
    {
      CALL_BF( BF_UnpinBlock( block));
      CALL_BF( BF_GetBlock( filedesc, block_info, block));
      char* data1;
    
      data1 = BF_Block_GetData( block) + num_from_info_to_hash*sizeof(int);

      d1 = data1;

      //an einai -1 tote -----> end   
      if (d1[0] == -1)
      {
        BF_Block_Destroy( &block);
        return HT_ERROR;
      }
      //an einai -2 tote ------> vrisko NextInformationBlock
      else if (d1[0] == -2)
      {
        //αν ειναι ισο με -2 σημαινει οτι ειναι το πρωτελευταιο στοιχειο του
        //μπλοκ πληροφοριες και οτι το επομενο στοιχειο ειναι το μπλοκ που συνεχιζεται το μπλοκ
        //πληροφοριων
        num_from_info_to_hash++;
        d1 = BF_Block_GetData( block) + num_from_info_to_hash*sizeof(int);

        block_info = d1[0];
        CALL_BF( BF_UnpinBlock( block));
        CALL_BF(BF_GetBlock( filedesc, block_info, block));
        
        data = BF_Block_GetData( block);
        num_from_info_to_hash = 0;
        d1 = data;
      }

      num_from_info_to_hash++;
      block_hash = d1[0];
      
      CALL_BF( BF_UnpinBlock( block));
      CALL_BF(BF_GetBlock(filedesc, block_hash, block));
      data = BF_Block_GetData( block);

      HashNum-=128;
    }


    int* d = data + HashNum*sizeof(int);
    int bucket_from_hash = d[0];

    CALL_BF( BF_UnpinBlock( block));
    CALL_BF( BF_GetBlock( filedesc, bucket_from_hash, block));


    data = BF_Block_GetData( block) + sizeof(int);
    d = data;

    //vazo tis times
    bucket_block = record_old.tupleId.block;
    index_block = record_old.tupleId.index;
    strcpy( key, record_old.index_key);

    if( bucket_from_hash == old_bucket)
    {

      if( num_old == (BF_BLOCK_SIZE - sizeof(int))/sizeof(SecondaryRecord))
      {
        fprintf( stderr,"Memory-Block is full\n");
        exit(EXIT_FAILURE);

      }
      //index-key
      memcpy( data + num_old*sizeof(SecondaryRecord) , key, sizeof(key));
      BF_Block_SetDirty(block);

      //block
      memcpy( data + num_old*sizeof(SecondaryRecord) + sizeof(key) , &bucket_block, sizeof(int));
      BF_Block_SetDirty(block);

      //index
      memcpy( data + num_old*sizeof(SecondaryRecord) + sizeof(key) + sizeof(int), &index_block, sizeof(int));
      BF_Block_SetDirty(block);

      num_old++;
    }
    else
    { 

      if( num_new == (BF_BLOCK_SIZE - sizeof(int))/sizeof(SecondaryRecord))
      {
        fprintf( stderr,"Memory-Block is full\n");
        exit(EXIT_FAILURE);
      }

      //index-key
      memcpy( data + num_new*sizeof(SecondaryRecord) , key, sizeof(key));
      BF_Block_SetDirty(block);

      //block
      memcpy( data + num_new*sizeof(SecondaryRecord) + sizeof(key) , &bucket_block, sizeof(int));
      BF_Block_SetDirty(block);

      //index
      memcpy( data + num_new*sizeof(SecondaryRecord) + sizeof(key) + sizeof(int), &index_block, sizeof(int));
      BF_Block_SetDirty(block);


      num_new++;
    }

  }  

  CALL_BF( BF_UnpinBlock( block));

  BF_Block_Destroy( &block);
  return HT_OK; 
}


HT_ErrorCode SHT_CreateNewHashTable( int filedesc, SecondaryRecord record, int bucket_b)
{ 

  BF_Block* block;
  BF_Block_Init( &block);

  int blocks_num;
  BF_GetBlockCounter( filedesc, &blocks_num);
  if(blocks_num <  3)  
  { 
    BF_Block_Destroy( &block);
    return HT_ERROR;
  }
  
  //παιρνω απο το μπλοκ - πληροφοριων
  CALL_BF( BF_GetBlock(filedesc, 1, block));

  char* data;
  data = BF_Block_GetData(block);

  //παιρνω το global_depth
  int depth = data[0];

  //υπολογιζω το 2^depth
  //που ειναι το μεγεθος του hashtable
  int a = 1;
  for( int i = 0; i < depth; i++)
  {
    a = a*2;
  }


  int num_block_hash_before;

  //PREVIOUS
  int num_of_ints = BF_BLOCK_SIZE/sizeof(int);
  num_block_hash_before = a/num_of_ints;
  if(a%num_of_ints > 0)
    num_block_hash_before++;

   
  //FOR NEW HASh
  depth++; //new depth
  int previous_a = a; //previous 2^depth
  int previous_depth = depth - 1; //previous depth
  a = a*2;  //new 2^depth
  int num_block_hash_new;
  
  //AFTERRR
  num_block_hash_new = a/num_of_ints;
  if(a%num_of_ints > 0)
    num_block_hash_new++;


  int different = num_block_hash_new - num_block_hash_before;
  int block_info = 1;
  int block_hash = 2;

  //if different == 0 than i have only one block( block_num = 1) for hash table
  //this is when new_depth <= 7...2^depth = 128
  if(different == 0)
  {

    //γραφω στην αρχη του 1ου μου μπλοκ
    //το νεο μας βαθος
    
    CALL_BF( BF_UnpinBlock( block));
    CALL_BF( BF_GetBlock(filedesc, block_hash, block));
    data = BF_Block_GetData(block); 

    //διαβαζω τις προηγουμενες τιμες του hash table και
    //τις αποθηκευω σε ενα πινακα
    int prices[ previous_a];
    for( int i = 0; i < previous_a; i++)
    {
      int* d = data + i*sizeof(int);
      prices[i] = d[0];
    }      
 
    //τωρα πρεπει να επεκτεινουμε το hashtable
    CALL_BF( BF_UnpinBlock( block));
    CALL_BF( BF_GetBlock(filedesc, block_info, block));
    data = BF_Block_GetData(block);

    //new depth write in the first block information in the start
    int new_depth = depth;
    memcpy(data, &new_depth, sizeof(int));
    BF_Block_SetDirty(block);

    //take first hash block

    CALL_BF( BF_UnpinBlock( block));
    CALL_BF( BF_GetBlock(filedesc, block_hash, block));
    data = BF_Block_GetData(block);
    num_of_ints = 0;
    
    while (num_of_ints < a)       
    {
      int value = num_of_ints >> 1;
      int v = prices[value];
      memcpy(data, &v, sizeof(int));

      BF_Block_SetDirty(block);
      num_of_ints++;
      data += sizeof(int);
    } 

  }
  //gia depth >= 8
  else
  {
    //prepei na epekteino to eyretirio mou
    //για να δημιουργησω αλλο ή αλλα hashblock

    //2^8 + 1
    //2^9 + 2
    //2^10 + 4

    //dhmioyrgo kapoia block gia arxi

    //αρχικα διαβαζω τα στοιχεια του ή των προηγουμενων μπλοκ
    block_hash = 2;
    block_info = 1;
    CALL_BF( BF_UnpinBlock( block));
    CALL_BF( BF_GetBlock(filedesc, block_hash, block));
    data = BF_Block_GetData(block); 

    //διαβαζω τις προηγουμενες τιμες του hash table και
    //τις αποθηκευω σε ενα πινακα

    int* prices = malloc(previous_a*sizeof(int));


    int elements = 0;
    int num_from_info_to_hash = 1;
    int Max_at_block = BF_BLOCK_SIZE/sizeof(int);

    int num_all_b;
    CALL_BF( BF_GetBlockCounter( filedesc, &num_all_b));
    
    for( int i = 0; i < previous_a; i++)
    { 
      if(Max_at_block == elements)
      {

        //παιρνω το μπλοκ-πληροφοριων

        CALL_BF( BF_UnpinBlock( block));
        CALL_BF( BF_GetBlock( filedesc, block_info, block));
        char* data1;
        if(block_info == 1)
        {
          data1 = BF_Block_GetData( block) + sizeof(int)+ num_from_info_to_hash*sizeof(int);
        }
        else
        {
          data1 = BF_Block_GetData( block) + num_from_info_to_hash*sizeof(int);
        }

        int* d1 = data1;

        //an einai -1 tote -----> end   
        if (d1[0] == -1)
        {
          BF_Block_Destroy( &block);
          return HT_ERROR;
        }
        //an einai -2 tote ------> vrisko NextInformationBlock
        else if (d1[0] == -2)
        {
          //αν ειναι ισο με -2 σημαινει οτι ειναι το πρωτελευταιο στοιχειο του
          //μπλοκ πληροφοριες και οτι το επομενο στοιχειο ειναι το μπλοκ που συνεχιζεται το μπλοκ
          //πληροφοριων

          d1 = data1 + sizeof(int);
          block_info = d1[0];
          CALL_BF( BF_UnpinBlock( block));
          CALL_BF(BF_GetBlock( filedesc, block_info, block));
          
          data = BF_Block_GetData( block);
          num_from_info_to_hash = 0;
          d1 = data;
        }

        num_from_info_to_hash++;
        block_hash = d1[0];

        CALL_BF( BF_UnpinBlock( block));
        CALL_BF( BF_GetBlock( filedesc, block_hash, block));
        data = BF_Block_GetData( block);
        elements = 0; //apo tin arxi tou hash
        
      }
      int* d = data + elements*sizeof(int);

      prices[i] = d[0];
      elements++;
    }      

    int new_blocks_num;
    new_blocks_num = (a - previous_a)/( BF_BLOCK_SIZE/sizeof(int)); 

    //δεσμευω τα καινουριο new_blocks_num που χρειαζονται
    //επισης τα αποθηκευω στο τελος του μπλοκ με της πληροφοριες
    block_info = 1;

    CALL_BF( BF_UnpinBlock( block));
    CALL_BF( BF_GetBlock( filedesc, block_info, block));
    data = BF_Block_GetData( block) + sizeof( int); //because is the depth the first element
    
    int num = 0;
    while(1)
    {
      int* d1 = data + num*sizeof(int);
     
      if( d1[0] == -1)
      {
        break;
      }
      else if( d1[0] == -2)
      {
        d1 = data + num*sizeof(int) + sizeof(int);
        block_info = d1[0];
        CALL_BF( BF_UnpinBlock( block));
        CALL_BF(BF_GetBlock( filedesc, block_info, block));
          
        data = BF_Block_GetData( block);
        num = 0;
        d1 = data;
      }
      num++;
    }

    //pairno ton arithmo twn yparxon blocks
    int num_blocks;
    CALL_BF( BF_GetBlockCounter( filedesc, &num_blocks));
    
    int The_new_HashBlock[ new_blocks_num];
    //kanoume allocate ta kainoyria block poy xreiazomaste gia to eyretiorio
    for( int j = 0; j < new_blocks_num; j++)
    { 
      CALL_BF( BF_UnpinBlock( block));
      CALL_BF( BF_AllocateBlock( filedesc, block));
      int num_block;
      CALL_BF( BF_GetBlockCounter( filedesc, &num_block));
      num_block--; //because start from zero(0)

      The_new_HashBlock[j] = num_block;
    }

    //if this block is the first( zero)    
    if( block_info == 1)
    {
      num++; //epeidi stin arxi to data mas einai data = Bf_getdata + sizeof(int) --- > giati exei to global_depth
    }

    int new_info = -2;
    int end_info = -1;
    CALL_BF( BF_UnpinBlock( block));  //itan slash
    CALL_BF( BF_GetBlock( filedesc, block_info, block));

    data = BF_Block_GetData( block);
    //num einai apo pano apo kei poy xei price = -1
    //block info apo pano

    //tora apothikeyo tou arithmous aytwn twn blocks sto block-informations
    for( int j = 0; j < new_blocks_num; j++)
    {

      //edo gia na apothikeyso ton arithmo toy kainoyrioy block prepei na desmeyso neo block_info
      //an den xoraei opote ta ftiaxno ola opos prepei gia na leitoyrgoyn sosta
      //kai na gnorizo pos na metakinitho sto neo block_info
      //kapoioi symvolismoi einai oi parakato
      //
      // Me -1 ------> den yparxei allo hash_block kai oyte synexeia block_info einai to terma
      //
      // Me -2 ------> an yparxei to -2 einai stin thesi [BF_BLOCK_SIZE/sizeof(int) - 2]  kai ayto
      //shmainei oti to block_info exei kai epomeno block_info opote sti teleytaia thesi tou torinou block
      //meta to -2 yparxei h timi toy epomenoy block_info
      if( num == BF_BLOCK_SIZE/sizeof(int) -1)
      { 
        
        int* d_1 = data + (num-1)*sizeof(int);
        int new_hash_to_transfer = d_1[0];

        memcpy( data + (num -1)*sizeof(int), &new_info, sizeof(int));
        BF_Block_SetDirty( block);
        

        //δεσμευω το καινουριο μπλοκ πληροφοριων και το αποθηκευω σαν τελευταιο αριθμο στο μπλοκ που βρισκομαι ωστε να γννωριζει
        //ποιο ειναι το επομενο
        CALL_BF( BF_UnpinBlock( block));  //itan shash
        CALL_BF( BF_AllocateBlock( filedesc, block));
        int new_block_info;
        CALL_BF( BF_GetBlockCounter( filedesc, &new_block_info));
        new_block_info = new_block_info - 1; //start from zero(0)

        //ksanapairno to block poy vriskomai
        CALL_BF( BF_UnpinBlock( block));  //itan slash
        CALL_BF( BF_GetBlock( filedesc, block_info, block));
        data = BF_Block_GetData( block);

        //kai apothikeyo poio einai to kainoyrio block pliroforiwn
        memcpy( data + num*sizeof(int), &new_block_info, sizeof(int));
        BF_Block_SetDirty( block);

        //tora prepei na grapso sto kainourio block pliroforiwn to previous_hash_to_transfer poy evgala oste na to apothikeyso ekei
        num = 0;
        CALL_BF( BF_UnpinBlock( block));
        CALL_BF( BF_GetBlock( filedesc, new_block_info, block));
        data = BF_Block_GetData( block);

        memcpy( data + num*sizeof(int), &new_hash_to_transfer, sizeof(int));
        BF_Block_SetDirty( block);

        num++; //here is the end -------> the second element in the new block info
        memcpy( data + num*sizeof(int), &end_info, sizeof(int));
        BF_Block_SetDirty( block);

        block_info = new_block_info;
      }
      

      //eisagogi sto block_info to neo mas hash_block
      //apo ton pinaka apothikeysis kata tin dhmioyrgia toys
      int new_hash_block = The_new_HashBlock[j];
      int* d11 = data + num*sizeof(int);

      memcpy( data + num*sizeof(int), &new_hash_block, sizeof(int)); //the new hashblock in the end( price = -1)
      
      BF_Block_SetDirty( block);

      num++;
      memcpy( data + num*sizeof(int), &end_info, sizeof(int)); //the end in the block( price = -1)
      BF_Block_SetDirty( block);

    }


    //tora exo desmeyesei ta kainouria block poy xreiazontai gia to eyretirio
    //exo enhmerosei ta/to block pliroforiwn kai an xreiazetai exo dhmioyrgisei kai nea block_info
    //exo apothikeysei poia einai ta kainouria block se enan pinaka
    //exo diavasei apo ta blockHash - block eyretirion oles tis times kai tis exo apothikeysei se enan pinaka
    //tora ayto poy menei einai na grapso to kainoyrio mou eyretirio me to neo mou vathos


    //διαβαζω τις προηγουμενες τιμες του hash table και
    //τις αποθηκευω σε ενα πινακα
    //τωρα πρεπει να επεκτεινουμε το hashtable
    block_info = 1;
    int num_to_block_info = 2; //epeidi to proto int einai vathos kai to deytero einai to block = 2 h arxi eyretyrioy poy to kseroyme
    CALL_BF( BF_UnpinBlock( block));
    CALL_BF( BF_GetBlock(filedesc, block_info, block));
    data = BF_Block_GetData(block);

    //new depth write in the first block information in the start
    int new_depth = depth;
    memcpy(data, &new_depth, sizeof(int));
    BF_Block_SetDirty(block);

    block_hash = 2;
    int num_for_hash_block = 0;

    //take first hash block
    CALL_BF( BF_UnpinBlock( block));
    CALL_BF( BF_GetBlock(filedesc, block_hash, block));
    data = BF_Block_GetData(block);
    num_of_ints = 0;
    
    while (num_of_ints < a)       
    {
      int value = num_of_ints >> 1;
      int v = prices[value];

      int* d1;
      if( data + num_for_hash_block*sizeof(int) > data + BF_BLOCK_SIZE -1)
      {
        //TOTE prepei na allakso to num_block_hash
        //ara diavazo apo to block_info
        CALL_BF( BF_UnpinBlock( block));
        CALL_BF( BF_GetBlock( filedesc, block_info, block));
        char* d_1 = BF_Block_GetData( block);
        d1 = d_1 + num_to_block_info*sizeof(int);
    
        if( d1[0] == -1)
        {
          BF_Block_Destroy( &block);
          return HT_ERROR;
        }
        else if( d1[0] == -2)
        {
          d1 = d_1 + num_to_block_info*sizeof(int) + sizeof(int);
          block_info = d1[0];
          CALL_BF( BF_UnpinBlock( block));
          CALL_BF(BF_GetBlock( filedesc, block_info, block));
          
          d_1 = BF_Block_GetData( block);
          d1 = d_1;
          num_to_block_info = 0;
        }

        //pairno ta data tou kainoyrio block hash poy tha diavazo
        block_hash = d1[0];
        CALL_BF( BF_UnpinBlock( block));
        CALL_BF( BF_GetBlock( filedesc, block_hash, block));
        data = BF_Block_GetData( block);

        num_for_hash_block = 0;
        num_to_block_info++;
      }

      memcpy( data + num_for_hash_block*sizeof(int), &v, sizeof(int));
      BF_Block_SetDirty(block);
      num_of_ints++;
      num_for_hash_block++;
    } 
    free( prices);
  }
  CALL_BF( BF_UnpinBlock( block));

  SHT_CreateNewBucket( filedesc, record, bucket_b);
  BF_Block_Destroy( &block);

  return HT_OK;
}

//other_inffo = 0
//block info = 1
//hash_block = 2
HT_ErrorCode SHT_SecondaryInsertEntry (int indexDesc, SecondaryRecord record ) {
  //insert code here

  //παιρνω τον αριθμο αρχειου ωστε να το βρω
  int filedesc = filetable->table[indexDesc]->file_desc;
  BF_Block* block;
  BF_Block_Init( &block);

  //ποσα μπλοκ εχει το υπαρχον αρχειο
  int blocks_num;
  CALL_BF( BF_GetBlockCounter( filedesc, &blocks_num));

  int i = 1;

  //GetBlock with data
  //το πρωτο μπλοκ εχει τις πληροφοριες
  CALL_BF(BF_GetBlock(filedesc, i, block));
  char* data;
  data = BF_Block_GetData( block);

  
  //global depth 
  int depth = data[0];
  // printf("secondary depth = %d", depth);
  CALL_BF( BF_UnpinBlock( block)); 
  //Hashing
  int HashNum = SHT_HashFunction( record, depth);

  int* d;

  //το 3ο μπλοκ του αρχειου ειναι το 2ο μπλοκ του ευρετηριου
  i = 2;
  int num_info = 1;
  int block_info = 1;
  CALL_BF(BF_GetBlock(filedesc, i, block));
  data = BF_Block_GetData( block);
  
  
  //Αν η θεση του πινακα που χασαρουμε δεν ειναι στο 1ο μπλοκ ευρετηριου τοτε παμε στο
  //επομενο και ουτε καθεξης
  while( data+ HashNum*sizeof(int) > data+BF_BLOCK_SIZE-1 )
  { 
    CALL_BF( BF_UnpinBlock( block));

    //αρα διαβαζουμε απο το μπλοκ πληροφοριες ποιο ειναι το επομενο
    CALL_BF( BF_GetBlock( filedesc, block_info, block));
    
    char *data1;
    //παιρνουμε τον αριθμο του επομενου μπλοκ ευρετηριου
    if( block_info == 1)
    {
      data1 = BF_Block_GetData(block) + sizeof(int) + num_info*sizeof(int);
    }
    else
    {
      //epeidi to proto stoixeio einai kapoio hash block kai oxi to vathos
      data1 = BF_Block_GetData(block)  + num_info*sizeof(int);
    }
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

      CALL_BF( BF_UnpinBlock( block));
      CALL_BF( BF_GetBlock( filedesc, block_info, block));

      data1 = BF_Block_GetData( block);
      num_info = 0;
      d1 = data1;
    }
    num_info++;
    i = d1[0];

    CALL_BF( BF_UnpinBlock( block));
    CALL_BF(BF_GetBlock(filedesc, i, block));
    data = BF_Block_GetData( block);
    HashNum = HashNum - 128;

  }

  //παιρνουμε σε ποιο μπλοκ χασαρει τελικα
  d = data + HashNum*sizeof(int);
  
  
  int bucket = d[0];

  //παιρνουμε τον αριθμο των μπλοκς
  int num_blocks;
  BF_GetBlockCounter( filedesc, &num_blocks);
  
  CALL_BF( BF_UnpinBlock( block));
  CALL_BF(BF_GetBlock( filedesc, bucket, block));
  data = BF_Block_GetData( block) + sizeof(int);
  int k = 0;
  
  //μετραω ποσα στοιχεια εχει μεσα το μπλοκ μου
  while( k < (BF_BLOCK_SIZE - sizeof(int))/sizeof(SecondaryRecord))
  {
      char key[20];
      char* d1 = data + k*sizeof(SecondaryRecord);
      strcpy( key, d1);
      if(key == NULL || strlen( key) == 0)
      { 
        break;
      }
      k++;
  }

  //ξεχωριζω τα πεδια του record
  char key[20];
  strcpy(key, record.index_key);

  TupleId tid = record.tupleId;

  int bucket_block = tid.block;
  
  int index_block = tid.index;


  data = BF_Block_GetData( block) + sizeof(int);
  //Εχουμε δυο περιπτωσεις η μια να χωραει στο μπλοκ μας
  //και η αλλη να μην χωραει
  
  if( k == (BF_BLOCK_SIZE-sizeof(int))/sizeof(SecondaryRecord))
  { 
    //η περιπτωση που το μπλοκ μας ειναι γεματο

    data = BF_Block_GetData(block);
    int depth_bucket = data[0];
    CALL_BF( BF_UnpinBlock( block)); 

    if( depth < depth_bucket)
    {
      printf("Global_d = %d kai Topic_d = %d - InsertEntry\n", depth, depth_bucket);
      BF_Block_Destroy( &block);
      return HT_ERROR;
    }
    if( depth < 0)
    {
      printf("Global_d = %d kai Topic_d = %d - InsertEntry\n", depth, depth_bucket);
      BF_Block_Destroy( &block);
      return HT_ERROR;
    }
    if( depth_bucket <0)
    {
      printf("Global_d = %d kai Topic_d = %d - InsertEntry\n", depth, depth_bucket);
      BF_Block_Destroy( &block);
      return HT_ERROR;
    }
 

    if( depth == depth_bucket)
    { 
      SHT_CreateNewHashTable( filedesc, record, bucket);
    }
    else
    {
      SHT_CreateNewBucket( filedesc, record, bucket);
    }
  }
  //αν χωραει στο μπλοκ η εγγραφη τοτε την χασαρουμε
  else
  {
    //index_key
    memcpy( data + k*sizeof(SecondaryRecord) , key, sizeof( key));
    BF_Block_SetDirty(block);

    //bucket
    memcpy( data + k*sizeof(SecondaryRecord) + sizeof(key) , &bucket_block, sizeof(int));
    BF_Block_SetDirty(block);

    //index
    memcpy( data + k*sizeof(SecondaryRecord) + sizeof(key) + sizeof(int) , &index_block, sizeof(int));
    BF_Block_SetDirty(block);

    CALL_BF( BF_UnpinBlock( block));

  }

  int bl_num;
  BF_GetBlockCounter( filedesc, &bl_num);

  BF_Block_Destroy( &block);
  return HT_OK;
}


HT_ErrorCode SHT_SecondaryUpdateEntry (int indexDesc, UpdateRecordArray *updateArray ) {
  // insert code here
  BF_Block *block;
  BF_Block_Init(&block);

  int Num = 0;
  char city[20];
  char surname[20];
  TupleId tid_old;
  TupleId tid_new;

  CALL_BF(BF_GetBlock(indexDesc, 0, block));

  char *data;
  data = BF_Block_GetData(block) + 20;
  char key[20];
  strcpy(key, data);

  while (1)
  {
    if (updateArray[Num].city == NULL || strlen(updateArray[Num].city) == 0 || updateArray[Num].surname == NULL || strlen(updateArray[Num].surname) == 0)
    {
      break;
    }

    strcpy(city, updateArray[Num].city);
    strcpy(surname, updateArray[Num].surname);
    tid_old.block = updateArray[Num].oldTupleId.block;
    tid_old.index = updateArray[Num].oldTupleId.index;

    tid_new.block = updateArray[Num].newTupleId.block;
    tid_new.index = updateArray[Num].newTupleId.index;

    char key_find[20];
    SecondaryRecord srecord;

    if (strcmp(key, "city") == 0)
    { 
      strcpy(srecord.index_key, city);
      strcpy( key_find, city);
      srecord.tupleId.block = tid_old.block;
      srecord.tupleId.index = tid_old.index;
    }
    else
    {
      strcpy(srecord.index_key, surname);
      strcpy( key_find, surname);

      srecord.tupleId.block = tid_old.block;
      srecord.tupleId.index = tid_old.index;
    }
    int filedesc = indexDesc;

    CALL_BF( BF_UnpinBlock( block));
    CALL_BF(BF_GetBlock(filedesc, 1, block));
    data = BF_Block_GetData(block);
    int depth = data[0];
    int HashNum = SHT_HashFunction( srecord, depth);

    int i = 2;
    int num_info = 1;
    int block_info = 1;

    CALL_BF(BF_UnpinBlock(block));
    CALL_BF(BF_GetBlock(filedesc, i, block));
    data = BF_Block_GetData(block);

    //Αν η θεση του πινακα που χασαρουμε δεν ειναι στο 2ο μπλοκ ευρετηριου τοτε παμε στο
    //επομενο και ουτε καθεξης

    while (data + HashNum * sizeof(int) > data + BF_BLOCK_SIZE - 1)
    {
      CALL_BF(BF_UnpinBlock(block));
      //αρα διαβαζουμε απο το μπλοκ πληροφοριες ποιο ειναι το επομενο
      CALL_BF(BF_GetBlock(filedesc, block_info, block));

      char *data1;
      //παιρνουμε τον αριθμο του επομενου μπλοκ ευρετηριου
      if (block_info == 1)
      {
        data1 = BF_Block_GetData(block) + sizeof(int) + num_info * sizeof(int);
      }
      else
      {
        // epeidi to proto stoixeio einai kapoio hash block kai oxi to vathos
        data1 = BF_Block_GetData(block) + num_info * sizeof(int);
      }
      int *d1 = data1;

      //αν ειναι -1 ειναι το τελος και εχουμε ερρορ
      if (d1[0] == -1)
      {
        BF_Block_Destroy(&block);
        return HT_ERROR;
      }
      else if (d1[0] == -2)
      {
        //αν ειναι ισο με -2 σημαινει οτι ειναι το πρωτελευταιο στοιχειο του
        //μπλοκ πληροφοριες και οτι το επομενο στοιχειο ειναι το μπλοκ που συνεχιζεται το μπλοκ
        //πληροφοριων

        d1 = data1 + sizeof(int);
        block_info = d1[0];

        CALL_BF(BF_UnpinBlock(block));
        CALL_BF(BF_GetBlock(filedesc, block_info, block));

        data1 = BF_Block_GetData(block);
        num_info = 0;
        d1 = data1;
      }
      num_info++;
      i = d1[0];

      CALL_BF(BF_UnpinBlock(block));
      CALL_BF(BF_GetBlock(filedesc, i, block));
      data = BF_Block_GetData(block);
      HashNum = HashNum - 128;
    }
    //παιρνουμε σε ποιο μπλοκ χασαρει τελικα
    char *d = data + HashNum * sizeof(int);
    int *d1 = d;

    //εχουμε βρει το καδο μας που χασαρουν τα index-key
    int bucket = d1[0];
    
    CALL_BF(BF_UnpinBlock(block));
    CALL_BF( BF_GetBlock( filedesc, bucket, block));
    data = BF_Block_GetData( block);
    d = data + sizeof(int);
    while( d < data + BF_BLOCK_SIZE - 1)
    { 
      char key_i[20];
      strcpy( key_i, d);
      
      int* d1 = d + sizeof(key_i);
      int block_bucket = d1[0];

      d1 = d + sizeof(key_i) + sizeof(int);
      int index_block = d1[0];

      if( strcmp( key_i, key_find) == 0 && block_bucket == tid_old.block && index_block == tid_old.index)
      {
        memcpy( d + sizeof(key_i), &(tid_new.block), sizeof(int));
        BF_Block_SetDirty( block); 
       
        memcpy(d + sizeof(key_i) + sizeof(int), &(tid_new.index), sizeof(int));
        BF_Block_SetDirty(block);

        break;
      }
      d = d + sizeof(SecondaryRecord);
    }
    Num++;
  }

  CALL_BF(BF_UnpinBlock(block));
  return HT_OK;
}

HT_ErrorCode SHT_PrintAllEntries(int sindexDesc, char *index_key ) {
  //insert code here

  BF_Block* block;
  BF_Block_Init( &block);

  int filedesc = filetable->table[sindexDesc]->file_desc;

  CALL_BF(BF_GetBlock(filedesc, 1, block));
  char *data1 = BF_Block_GetData(block);
  int depth = data1[0];

  SecondaryRecord record;
  strcpy(record.index_key, index_key);

  int HashNum = SHT_HashFunction(record, depth);

  int i = 2;
  int num_info = 1;
  int block_info = 1;
  
  CALL_BF(BF_UnpinBlock(block));
  CALL_BF(BF_GetBlock(filedesc, i, block));
  char *data = BF_Block_GetData(block);
  
  //Αν η θεση του πινακα που χασαρουμε δεν ειναι στο 2ο μπλοκ ευρετηριου τοτε παμε στο
  //επομενο και ουτε καθεξης

  while (data + HashNum * sizeof(int) > data + BF_BLOCK_SIZE - 1)
  {
    CALL_BF(BF_UnpinBlock(block));
    //αρα διαβαζουμε απο το μπλοκ πληροφοριες ποιο ειναι το επομενο
    CALL_BF(BF_GetBlock(filedesc, block_info, block));

    char *data1;
    //παιρνουμε τον αριθμο του επομενου μπλοκ ευρετηριου
    if (block_info == 1)
    {
      data1 = BF_Block_GetData(block) + sizeof(int) + num_info * sizeof(int);
    }
    else
    {
      // epeidi to proto stoixeio einai kapoio hash block kai oxi to vathos
      data1 = BF_Block_GetData(block) + num_info * sizeof(int);
    }
    int *d1 = data1;

    //αν ειναι -1 ειναι το τελος και εχουμε ερρορ
    if (d1[0] == -1)
    {
      BF_Block_Destroy(&block);
      return HT_ERROR;
    }
    else if (d1[0] == -2)
    {
      //αν ειναι ισο με -2 σημαινει οτι ειναι το πρωτελευταιο στοιχειο του
      //μπλοκ πληροφοριες και οτι το επομενο στοιχειο ειναι το μπλοκ που συνεχιζεται το μπλοκ
      //πληροφοριων

      d1 = data1 + sizeof(int);
      block_info = d1[0];

      CALL_BF(BF_UnpinBlock(block));
      CALL_BF(BF_GetBlock(filedesc, block_info, block));

      data1 = BF_Block_GetData(block);
      num_info = 0;
      d1 = data1;
    }
    num_info++;
    i = d1[0];

    CALL_BF(BF_UnpinBlock(block));
    CALL_BF(BF_GetBlock(filedesc, i, block));
    data = BF_Block_GetData(block);
    HashNum = HashNum - 128;
  }
  //παιρνουμε σε ποιο μπλοκ χασαρει τελικα
  char *d = data + HashNum * sizeof(int);
  int *d1 = d;

  //εχουμε βρει το καδο μας που χασαρουν τα index-key
  int bucket = d1[0];

  CALL_BF(BF_UnpinBlock(block));
  CALL_BF(BF_GetBlock(filedesc, bucket, block));
  data = BF_Block_GetData(block);
  d = data + sizeof(int);
  char key[20];
  strcpy( key, index_key);

  //tha diavasouyme to filename
  CALL_BF(BF_UnpinBlock(block));
  CALL_BF(BF_GetBlock(filedesc, 0, block));
  
  char* data11;
  data11 = BF_Block_GetData(block);
  
  char fileName[20];
  strcpy( fileName, data11);

  int indexDesc;
  CALL_BF( BF_OpenFile( fileName, &indexDesc));
  while (d < data + BF_BLOCK_SIZE - 1)
  {
    if ( strcmp( key, d) == 0)
    {


      int* d1 = d + sizeof(key);
      int bucket_block = d1[0];
      
      int* d2 = d + sizeof(key) + sizeof(int);
      int index_block = d2[0];
      printf("\n\nkey = %s kai block = %d kai ind = %d\n", d, bucket_block, index_block);

      
      CALL_BF(BF_UnpinBlock(block));
      CALL_BF( BF_GetBlock( indexDesc, bucket_block, block));
      data11 = BF_Block_GetData( block);

      char* d4 = data11 + index_block*sizeof(Record) + sizeof(int);
      int* d_id = data11 + index_block*sizeof(Record) + sizeof(int);
      printf("Id : %d\n", d_id[0]);
      char *data1 = d4 + sizeof(int);

      char name[15];
      char city[20];
      char surname[20];
      strcpy(name, data1);
      printf("Name: %s\n", name);

      data1 = d4 + sizeof(int) + sizeof(name);

      strcpy(surname, data1);
      printf("SurName: %s\n", surname);

      data1 = d4 + sizeof(int) + sizeof(name) + sizeof(surname);

      strcpy(city, data1);
      printf("City: %s\n", city);
    }
    d = d + sizeof(SecondaryRecord);
  }

  CALL_BF(BF_UnpinBlock(block));
  CALL_BF( BF_CloseFile( indexDesc));

  return HT_OK;
}

HT_ErrorCode SHT_HashStatistics(char *filename ) {
  //insert code here
  int filedesc;
  CALL_BF(BF_OpenFile(filename, &filedesc));

  int num_of_blocks;

  BF_GetBlockCounter(filedesc, &num_of_blocks);

  printf("Secondary Hash File named %s has %d blocks\n", filename,num_of_blocks);

  BF_Block* block;
  BF_Block_Init(&block);

  int i = 2;
  BF_GetBlock(filedesc, i, block);
  char* data = BF_Block_GetData(block);

  CALL_BF( BF_UnpinBlock( block));

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  BF_GetBlock(filedesc, 1, block);
  data = BF_Block_GetData(block);
  int* d__1 = data;
  int depth = d__1[0];
  int a = 1;
  for (int i = 0; i < depth; i++)
  {
    a = a * 2;
  }

  int sum_all_records = 0;
  int sum_all_buckets = 0;
  int max = -1;
  int min = BF_BLOCK_SIZE/sizeof(Record) + 1;

  int block_info = 1;
  int hash_block = 2;
  int num_info = 2;
  int num_hash = 0;

  CALL_BF(BF_UnpinBlock(block));
  CALL_BF(BF_GetBlock(filedesc, hash_block, block));

  int previous_bucket = -1;
  char name[15];
  bool end = false;
  int num_of_ints = 0;

  data = BF_Block_GetData(block);
  while (num_of_ints < a)
  {

    int *d_1 = data + num_hash * sizeof(int);
    int bucket = d_1[0];

    if (bucket != previous_bucket)
    {
      CALL_BF(BF_UnpinBlock(block));
      CALL_BF(BF_GetBlock(filedesc, bucket, block));
      char *d1 = BF_Block_GetData(block) + sizeof(int);
      
      int num_in_bucket = 0;
      int sum_bucket = 0;
      while (num_in_bucket < BF_BLOCK_SIZE / sizeof(Record))
      {
        char *d2 = d1 + num_in_bucket * sizeof(Record) + sizeof(int);
        strcpy(name, d2);

        num_in_bucket++;
        sum_bucket++;
        sum_all_records++;
        if (name == NULL || strlen(name) == 0)
        {
          break;
        }
      }
      sum_all_buckets++;

      if(sum_bucket > max)
      {
        max = sum_bucket;
      }

      if( sum_bucket < min )
      {
        min = sum_bucket;
      }

      CALL_BF(BF_UnpinBlock(block));
      CALL_BF(BF_GetBlock(filedesc, hash_block, block))
      data = BF_Block_GetData(block);
      previous_bucket = bucket;
    }
    num_hash++;
    num_of_ints++;

    // elengxoume an einai ektos to hash block-eyretirio
    // kai pairno to epomeno
    // an to epomeno einai -1 tote kano break

    if (data + num_hash * sizeof(int) > data + BF_BLOCK_SIZE - 1)
    {

      CALL_BF(BF_UnpinBlock(block));
      //αρα διαβαζουμε απο το μπλοκ πληροφοριες ποιο ειναι το επομενο
      CALL_BF(BF_GetBlock(filedesc, block_info, block));

      char *data1;
      //παιρνουμε τον αριθμο του επομενου μπλοκ ευρετηριου
      data1 = BF_Block_GetData(block) + num_info * sizeof(int);
      int *d1 = data1;

      //αν ειναι -1 ειναι το τελος και εχουμε ερρορ
      if (d1[0] == -1)
      {
        end = true;
        break;
      }
      else if (d1[0] == -2)
      {
        //αν ειναι ισο με -2 σημαινει οτι ειναι το προτελευταιο στοιχειο του
        //μπλοκ πληροφοριες και οτι το επομενο στοιχειο ειναι το μπλοκ που συνεχιζεται το μπλοκ
        //πληροφοριων
        d1 = data1 + sizeof(int);
        block_info = d1[0];

        CALL_BF(BF_UnpinBlock(block));
        CALL_BF(BF_GetBlock(filedesc, block_info, block));

        data1 = BF_Block_GetData(block);
        num_info = 0;
        d1 = data1;
      }

      num_info++;
      hash_block = d1[0];
      num_hash = 0;

      CALL_BF(BF_UnpinBlock(block));
      CALL_BF(BF_GetBlock(filedesc, hash_block, block));
      data = BF_Block_GetData(block);
      int *d_11 = data;
    }
  }


  printf("Average number of records in a bucket is %d\n", sum_all_records/sum_all_buckets);
  printf("Minimum number of records in a bucket is %d\n", min);
  printf("Maximum number of records in a bucket is %d\n", max);
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  BF_Block_Destroy( &block);
  return HT_OK;
}

HT_ErrorCode SHT_InnerJoin(int sindexDesc1, int sindexDesc2,  char *index_key ) 
{
  //insert code here
  // int filedesc;
  // CALL_BF(BF_OpenFile(filename, &filedesc));


  BF_Block* block;
  BF_Block_Init(&block);



  char Father_file_1[20];
  char Father_file_2[20];


  //FOR THE FIRST FILE
  CALL_BF( BF_GetBlock( sindexDesc1, 0, block));
  char* data_1 = BF_Block_GetData(block);
  strcpy( Father_file_1, data_1);
  CALL_BF( BF_UnpinBlock( block));
  int indexDesc1;
  BF_OpenFile( Father_file_1, &indexDesc1);

  CALL_BF(BF_GetBlock( sindexDesc1, 1, block));
  int* data_11 = BF_Block_GetData( block);
  int depth_1 = data_11[0];
  CALL_BF( BF_UnpinBlock( block))

  
  
  int a_1 = 1;
  for (int i = 0; i < depth_1; i++)
  {
    a_1 = a_1 * 2;
  }
  ////////////////////////////////////////
  
  
  //FOR THE SECOND FILE
  BF_GetBlock(sindexDesc2, 0, block);
  char* data_2 = BF_Block_GetData(block);
  strcpy( Father_file_2, data_2);
  CALL_BF(BF_UnpinBlock(block));
  int indexDesc2;
  BF_OpenFile(Father_file_2, &indexDesc2);
  
  BF_GetBlock(sindexDesc2, 1, block);
  int* data_22 = BF_Block_GetData(block);
  int depth_2 = data_22[0];  
  CALL_BF(BF_UnpinBlock(block));
  
  int a_2 = 1;
  for (int i = 0; i < depth_2; i++)
  {
    a_2 = a_2 * 2;
  }
  ////////////////////////////////////////


  // int num_of_blocks;

  

  int block_info_1 = 1;
  int hash_block_1 = 2;
  int num_info_1 = 2;
  int num_hash_1 = 0;

  CALL_BF(BF_GetBlock( sindexDesc1, hash_block_1, block));

  // char *data;
  int previous_bucket_1 = -1;

  char name[15];
  char surname[20];
  char city[20];

  bool end = false;
  int num_of_ints_1 = 0;

  data_1 = BF_Block_GetData(block);
  while (num_of_ints_1 < a_1)
  {

    int *d_1 = data_1 + num_hash_1 * sizeof(int);
    int bucket_1 = d_1[0];

    if (bucket_1 != previous_bucket_1)
    {
      CALL_BF(BF_UnpinBlock(block));
      CALL_BF(BF_GetBlock( sindexDesc1, bucket_1, block));
      char *d_1_block = BF_Block_GetData(block) + sizeof(int);
      int num_in_bucket_1 = 0;


      char key[20];
      while (num_in_bucket_1 < BF_BLOCK_SIZE / sizeof(SecondaryRecord))
      {
        // printf("hel\n");
        d_1_block = d_1_block +sizeof(SecondaryRecord);

        char index_key_1[20];
        strcpy( index_key_1, d_1_block);
        // printf("ela %d\n", strlen( d_1_block));

        if( strcmp( index_key_1, "NULL") == 0|| index_key_1 == 0||index_key_1 == "NULL" || strlen( index_key_1) == 0)
        {
          // printf("i am here\n");
          break;
        }
        // printf("ela %s\n", index_key_1);

        ////
        int *d1 = d_1_block + sizeof(key);
        int bucket_block = d1[0];

        int *d2 = d_1_block + sizeof(key) + sizeof(int);
        int index_block = d2[0];
        // printf("ela %d\n", bucket_block);

        CALL_BF(BF_UnpinBlock(block));
        CALL_BF(BF_GetBlock( indexDesc1, bucket_block, block));
        // printf("ela\n");

        char* data11 = BF_Block_GetData(block);

        char *d4 = data11 + index_block * sizeof(Record) + sizeof(int);
        int *d_id = data11 + index_block * sizeof(Record) + sizeof(int);
        int id_1 = d_id[0];
        

        char name_1[15];
        char city_1[20];
        char surname_1[20];

        char *data1 = d4 + sizeof(int);
        strcpy(name_1, data1);

        data1 = d4 + sizeof(int) + sizeof(name_1);
        strcpy(surname_1, data1);

        data1 = d4 + sizeof(int) + sizeof(name_1) + sizeof(surname_1);
        strcpy(city_1, data1);

        // printf("eco na = %s kai su = %s\n", name_1, surname_1);
        num_in_bucket_1++;



        //open second file

        int block_info_2 = 1;
        int hash_block_2 = 2;
        int num_info_2 = 2;
        int num_hash_2 = 0;

        CALL_BF(BF_GetBlock( sindexDesc2, hash_block_2, block));
        data_2 = BF_Block_GetData(block);


        // char *data;
        int previous_bucket_2 = -1;

        char name_2[15];
        char surname_2[20];
        char city_2[20];

        int num_of_ints_2 = 0;
        while (num_of_ints_2 < a_2)
        {

          int *d_2 = data_2 + num_hash_2 * sizeof(int);
          int bucket_2 = d_2[0];

          if (bucket_2 != previous_bucket_2)
          {
            CALL_BF(BF_UnpinBlock(block));
            CALL_BF(BF_GetBlock( sindexDesc2, bucket_2, block));
            char *d_2_block = BF_Block_GetData(block) + sizeof(int);
            int num_in_bucket_2 = 0;

            while (num_in_bucket_2 < BF_BLOCK_SIZE / sizeof( SecondaryRecord))
            {
              
              d_2_block = d_2_block + sizeof( SecondaryRecord);

              if( strcmp( d_2_block, "NULL") == 0 || strlen( d_2_block) == 0)
              {
                break;
              }

              char index_key_2[20];
              strcpy( index_key_2, d_2_block);
              ////
              int *d1 = d_2_block + sizeof(key);
              int bucket_block = d1[0];

              int *d2 = d_2_block + sizeof(key) + sizeof(int);
              int index_block = d2[0];


              CALL_BF(BF_UnpinBlock(block));


              CALL_BF(BF_GetBlock( indexDesc2, bucket_block, block));

              char* data22 = BF_Block_GetData(block);

              char *d4 = data22 + index_block * sizeof(Record) + sizeof(int);
              int *d_id = data22 + index_block * sizeof(Record) + sizeof(int);
              int id_2 = d_id[0];
              

    
              char *data1 = d4 + sizeof(int);
              strcpy(name_2, data1);

              data1 = d4 + sizeof(int) + sizeof(name_2);
              strcpy(surname_2, data1);

              data1 = d4 + sizeof(int) + sizeof(name_2) + sizeof(surname_2);
              strcpy(city_2, data1);


              if( strcmp( index_key, "NULL") == 0)
              {
                printf("ids:  %d - %d\n", id_1, id_2);                
                printf("names:  %s - %s\n", name_1, name_2);
                printf("surnames:  %s - %s\n", surname_1, surname_2);
                printf("cities:  %s - %s\n", city_1, city_2);
              
              }
              else if( strcmp( index_key_1, index_key_2) == 0 && strcmp( index_key_1, index_key) == 0)
              {
                printf("ids:  %d - %d\n", id_1, id_2);                
                printf("names:  %s - %s\n", name_1, name_2);
                printf("surnames:  %s - %s\n", surname_1, surname_2);
                printf("cities:  %s - %s\n", city_1, city_2);
              }
              // printf("elaaaaaaaaaaaaaaaaa\n");
              num_in_bucket_2++;
            }

            CALL_BF(BF_UnpinBlock(block));
            CALL_BF(BF_GetBlock( sindexDesc2, hash_block_2, block))
            data_2 = BF_Block_GetData(block);
            previous_bucket_2 = bucket_2;
          }
          num_hash_2++;
          num_of_ints_2++;

          // elengxoume an einai ektos to hash block-eyretirio
          // kai pairno to epomeno
          // an to epomeno einai -1 tote kano break

          if (data_2 + num_hash_2 * sizeof(int) > data_2 + BF_BLOCK_SIZE - 1)
          {

            CALL_BF(BF_UnpinBlock(block));
            //αρα διαβαζουμε απο το μπλοκ πληροφοριες ποιο ειναι το επομενο
            CALL_BF(BF_GetBlock( sindexDesc2, block_info_2, block));

            char *data2;
            //παιρνουμε τον αριθμο του επομενου μπλοκ ευρετηριου
            data2 = BF_Block_GetData(block) + num_info_2 * sizeof(int);
            int *d1 = data2;

            //αν ειναι -1 ειναι το τελος και εχουμε ερρορ
            if (d1[0] == -1)
            {
              end = true;
              break;
            }
            else if (d1[0] == -2)
            {
              //αν ειναι ισο με -2 σημαινει οτι ειναι το πρωτελευταιο στοιχειο του
              //μπλοκ πληροφοριες και οτι το επομενο στοιχειο ειναι το μπλοκ που συνεχιζεται το μπλοκ
              //πληροφοριων
              d1 = data2 + sizeof(int);
              block_info_2 = d1[0];

              CALL_BF(BF_UnpinBlock(block));
              CALL_BF(BF_GetBlock( sindexDesc2, block_info_2, block));

              data2 = BF_Block_GetData(block);
              num_info_2 = 0;
              d1 = data2;
            }

            num_info_2++;
            hash_block_2 = d1[0];
            num_hash_2 = 0;

            CALL_BF(BF_UnpinBlock(block));
            CALL_BF(BF_GetBlock( sindexDesc2, hash_block_2, block));
            data_2 = BF_Block_GetData(block);
          }

        }

        num_in_bucket_1++;

      }

      CALL_BF(BF_UnpinBlock(block));
      CALL_BF(BF_GetBlock( sindexDesc1, hash_block_1, block))
      data_1 = BF_Block_GetData(block);
      previous_bucket_1 = bucket_1;
    }
    num_hash_1++;
    num_of_ints_1++;
    

    // elengxoume an einai ektos to hash block-eyretirio
    // kai pairno to epomeno
    // an to epomeno einai -1 tote kano break

    if (data_1 + num_hash_1 * sizeof(int) > data_1 + BF_BLOCK_SIZE - 1)
    {

      CALL_BF(BF_UnpinBlock(block));
      //αρα διαβαζουμε απο το μπλοκ πληροφοριες ποιο ειναι το επομενο
      CALL_BF(BF_GetBlock( sindexDesc1, block_info_1, block));

      char *data1;
      //παιρνουμε τον αριθμο του επομενου μπλοκ ευρετηριου
      data1 = BF_Block_GetData(block) + num_info_1 * sizeof(int);
      int *d1 = data1;

      //αν ειναι -1 ειναι το τελος και εχουμε ερρορ
      if (d1[0] == -1)
      {
        end = true;
        break;
      }
      else if (d1[0] == -2)
      {
        //αν ειναι ισο με -2 σημαινει οτι ειναι το πρωτελευταιο στοιχειο του
        //μπλοκ πληροφοριες και οτι το επομενο στοιχειο ειναι το μπλοκ που συνεχιζεται το μπλοκ
        //πληροφοριων
        d1 = data1 + sizeof(int);
        block_info_1 = d1[0];

        CALL_BF(BF_UnpinBlock(block));
        CALL_BF(BF_GetBlock(sindexDesc1, block_info_1, block));

        data1 = BF_Block_GetData(block);
        num_info_1 = 0;
        d1 = data1;
      }

      num_info_1++;
      hash_block_1 = d1[0];
      num_hash_1 = 0;

      CALL_BF(BF_UnpinBlock(block));
      CALL_BF(BF_GetBlock(sindexDesc1, hash_block_1, block));
      data_1 = BF_Block_GetData(block);
    }
  }
  

  return HT_OK;
}


// #endif // HASH_FILE_H