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
  printf("elaaaaaa  %s --- %d\n",filename,file_desc);

  // printf("fd: %d\n", file_desc);
  //φτιαχνω ενα μπλοκ και βαζω μεσα το βαθος
  BF_Block *block;
  BF_Block_Init(&block);

  CALL_BF( BF_AllocateBlock( file_desc, block));
  char* data;

  data = BF_Block_GetData(block);
  memcpy(data, &depth, sizeof(int));
  BF_Block_SetDirty(block);

  int hash_block_index = 1;
  memcpy(data + sizeof(int), &hash_block_index, sizeof(int));
  BF_Block_SetDirty(block);


  //-1 is the final price in the first block
  int end = -1;
  memcpy(data + 2*sizeof(int), &end, sizeof(int));
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
  
  CALL_BF( BF_UnpinBlock( block));

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
      CALL_BF( BF_UnpinBlock( block));
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

  int filedesc;
  CALL_BF( BF_OpenFile(fileName, &filedesc));

  // printf("elaaaaaa  %s --- %d\n",fileName,filedesc);
  // return HT_ERROR;
  for(int i = 0; i < filetable->size_table; i++){
    if(filetable->table[i] == NULL){
      *indexDesc = i;
      break;
    }
  }

  // printf(" ind: %d\n",filetable->table[*indexDesc]->filename);  
  filetable->table[*indexDesc] = malloc(sizeof(struct file_open));
  filetable->table[*indexDesc]->file_desc = filedesc;
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

  return bl_d;

}



//MHTSOOOOOOOOOOOOOOOO
//EDO FTIAKSE
//-1 KAI -2/KAI CREATE
//EXOUME BUG
HT_ErrorCode CreateNewBucket( int filedesc, Record record, int bucket){

  printf("\neimai mesa me bucket = %d - CreateNewBucket\n", bucket);


  // int blocks_num;
  // CALL_BF( BF_GetBlockCounter( filedesc, &blocks_num));
  
  //init for block
  BF_Block* block;
  BF_Block_Init(&block);
  printf("\n\nektypono  %d\n\n",block);

  //block info
  //to eyretirio mas arxizei apo to block = 0
  int block_info = 0;
  CALL_BF(BF_GetBlock(filedesc, block_info, block));
  char* data = BF_Block_GetData( block);
  int global_depth = data[0];   //global depth
  
  printf("global_ d ====== %d kaii filedec == %d\n", global_depth, filedesc);
  

  //create one new block == BUCKET

  // CALL_BF( BF_UnpinBlock( block));
  // // CALL_BF( BF_AllocateBlock( filedesc, block));
  // BF_PrintError(BF_AllocateBlock( filedesc, block));
  int bl_ddd;
  CALL_BF( BF_GetBlockCounter( filedesc, &bl_ddd));
  printf(" all_blocks = %d for depth = %d\n\n\n",bl_ddd, global_depth);

  // //take all blocks_num
  int blocks_num;
  CALL_BF( BF_GetBlockCounter( filedesc, &blocks_num));
  
  //take num for the last block where is the new block
  int dest = blocks_num;

  
  int previous_block = -1;
  
  //to eyretirio mas hashblock arxizei apo to 2o block toy arxeio me i = 1
  int i = 1;
  CALL_BF( BF_UnpinBlock( block));
  CALL_BF(BF_GetBlock(filedesc, i, block));
  data = BF_Block_GetData(block);
  
  int f = 0;
  //pairno to proto stoixeio toy pinaka kai to perieomeno toy diladi se poio block deixnei
  int* d = data + f*sizeof(int);
  int count = 0;                //to count metraei poses theseis tou pinaka deixnoyn sto bucket provlima
  int num_info = 1;            //eimai sto block info poy deixnei se kapoio eyretirio


  //apothikeyo tis plirofories gia to proto block
  int* first_data = NULL;
  int first_hashtable_num = 1;
  int first_information_block = 0;
  int first_find = false;
  int first_f = NULL;
  int first_num_info = num_info;

  while(1){
    // printf("LOIPON MALAKA : %d\n\n", d[0]);

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
      if( block_info == 0)
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
  // printf("%d kai b %d\n",d[0], bucket);
    
  }

///////////////////////
////////////////////SVISEEE EDOOOOOOOOOOOOOOOOOOOOO

  //error
  if(count == 0)
  {
    printf("sou gamietai to spiti dhmitraki\n");
    BF_Block_Destroy( &block);
    return HT_ERROR;
  }

  // printf("\nexo vrei to count = %d - CreateNewBucket\n", count);
  
  //count einai poses theseis toy hash table deixnoyn sto bucket-problem
  int num;

  num = first_f;
    
  // printf("3ol %d\n", first_hashtable_num);

  CALL_BF( BF_UnpinBlock( block));
  CALL_BF( BF_GetBlock(filedesc, first_hashtable_num, block));  //pairno to block poy ksekina na deixnei sto bucket provlima
  data = BF_Block_GetData( block);                //pairno tin dieythisni tis protis thesis tou eyretirioy atri
  block_info = first_information_block;
  num_info = first_num_info;

  //kano ta teleytaia apo to telos count/2 block anti na deixnoun
  //sto bucket_provlima na deixnoun sto kainourio dhladh ------------> dest
  for(int k = 0; k < count/2; k++){
    
    //vrisko tin dieythinsi pou vrisketai h proti mas thesi me busket_problem
    d = data + num*sizeof(int);

    //an omos ftaso sto telos tou block_eyretiriou kai vgo ektos orioy
    //prepei na pao sto epomeno  
    if( d > data + BF_BLOCK_SIZE -1){
      
      //pairno apo to block_info pou einai to hashtable poy vriskomai kai pao ena brosta hashblock-eyretirio
      CALL_BF( BF_UnpinBlock( block));
      CALL_BF(BF_GetBlock(filedesc, block_info, block));



      char *data1;
      //παιρνουμε τον αριθμο του επομενου μπλοκ ευρετηριου
      if( block_info == 0)
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
      d = data + num*sizeof(int);
      // num = 1;
    }

    num++;
    memcpy(d, &dest, sizeof(int));
    BF_Block_SetDirty(block);
  }
 
  //TORA prepei na hasharo ksana tis times tou paliou kai neou block ksana
  //ara exo dyo block
  int old_bucket = bucket;
  int new_bucket = dest;

  // ena bucket xoraei to poli 8 eggrafes eipame
  // diavazp tis times aytes kai tis apothikeyo se ena pinaka pao record
  
  // strcpy(city, record.city);
  CALL_BF( BF_UnpinBlock( block));                                                              //mmmmm
  CALL_BF( BF_GetBlock(filedesc, old_bucket, block));


  //auksano to topiko vathos gia to palio block
  data = BF_Block_GetData( block);
  int topic_depth;
  topic_depth = data[0] + 1;
  printf("next t_d = %d && prev = %d\n\n", topic_depth, data[0]);
  memcpy( data, &topic_depth, sizeof(int));
  BF_Block_SetDirty( block);

  //grafo to topiko vahtos gia to neo block
  //pou einai oso to topic_depth
  //tou old_bucket 
  
  //////////////////////////////////////////////////////////////////////////////////////////
  CALL_BF( BF_UnpinBlock( block));
  BF_PrintError(BF_AllocateBlock( filedesc, block));
  /////////////////////////////////////////////////////////////////////////////////////////

  CALL_BF( BF_UnpinBlock( block));
  CALL_BF( BF_GetBlock(filedesc, new_bucket, block));
  data = BF_Block_GetData( block);

  memcpy( data, &topic_depth, sizeof(int));
  BF_Block_SetDirty( block);


  //ksana sto old_bucket
  CALL_BF( BF_UnpinBlock( block));
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
    // printf("Hashum : %d\n", HashNum);
    

    block_info = 0;
    int block_hash = 1;
    int num_from_info_to_hash = 0;
    CALL_BF( BF_UnpinBlock( block));    // to old bucket
    CALL_BF(BF_GetBlock(filedesc, block_hash, block)); 
    data = BF_Block_GetData( block);



  
  
    while( data+ HashNum*sizeof(int) > data+BF_BLOCK_SIZE-1 )
    {




      /////////////
      //////////////////
      // printf("ellaa\n");
      CALL_BF( BF_UnpinBlock( block));
      CALL_BF( BF_GetBlock( filedesc, block_info, block));
      char* data1;
      if(block_info == 0)
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

        /////////////////////////////////////////////////////////////////////FFFFFFFFFFFFFFFFFFFFFFFFFFE____DESSS TOOOOOOOOOOO
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
        d1 = data[0];
      }

      num_from_info_to_hash++;
      block_hash = d1[0];
      ///////////////////
      
      CALL_BF( BF_UnpinBlock( block));
      CALL_BF(BF_GetBlock(filedesc, block_hash, block));
      data = BF_Block_GetData( block);

      HashNum-=128;
    }

    int* d = data + HashNum*sizeof(int);
    int bucket_from_hash = d[0];
    // printf("ookkkk %d\n", bucket_from_hash);

    CALL_BF( BF_UnpinBlock( block));
    CALL_BF( BF_GetBlock( filedesc, bucket_from_hash, block));

    // int bl_co;
    // CALL_BF( BF_GetBlockCounter( filedesc, &bl_co));
    // printf("bl_co = %d\n", bl_co);
    // printf("bl = %d\n", bucket_from_hash);
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
      // printf("num new == %d\n", num_new);
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
  
  CALL_BF( BF_UnpinBlock( block));
  BF_Block_Destroy( &block);
  // printf("o[a\n");
  return HT_OK; 
}


HT_ErrorCode CreateNewHashTable( int filedesc, Record record, int bucket_b)
{ 
  printf("\neimai mesa me bucket = %d - CreateNewHashTable\n", bucket_b);

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
  CALL_BF( BF_GetBlock(filedesc, 0, block));

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

  // printf("other:  %d\n", num_block_hash_new);

  int different = num_block_hash_new - num_block_hash_before;
  int block_info = 0;
  int block_hash = 1;

  //if different == 0 than i have only one block( block_num = 1) for hash table
  //this is when new_depth <= 7...2^depth = 128
  if(different == 0)
  {

    // int bucket = blocks_num - num_block_hash_new;
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
      printf("Pos = %d   bucket = %d - CreateNewHashTable\n",num_of_ints, v);
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
    block_hash = 1;
    block_info = 0;
    CALL_BF( BF_UnpinBlock( block));
    CALL_BF( BF_GetBlock(filedesc, block_hash, block));
    data = BF_Block_GetData(block); 

    //διαβαζω τις προηγουμενες τιμες του hash table και
    //τις αποθηκευω σε ενα πινακα
    int prices[ previous_a];
    int elements = 0;
    int num_from_info_to_hash = 1;
    int Max_at_block = BF_BLOCK_SIZE/sizeof(int);

    int num_all_b;
    CALL_BF( BF_GetBlockCounter( filedesc, &num_all_b));
    printf("num all = %d\n",num_all_b);
    
    for( int i = 0; i < previous_a; i++)
    { 
      if(Max_at_block == elements)
      {

        //παιρνω το μπλοκ-πληροφοριων

        CALL_BF( BF_UnpinBlock( block));
        CALL_BF( BF_GetBlock( filedesc, block_info, block));
        char* data1;
        if(block_info == 0)
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
          d1 = data[0];
        }

        num_from_info_to_hash++;
        block_hash = d1[0];

        CALL_BF( BF_UnpinBlock( block));
        CALL_BF( BF_GetBlock( filedesc, block_hash, block));
        data = BF_Block_GetData( block);
        elements = 0; //apo tin arxi tou hash
        
      }
      int* d = data + elements*sizeof(int);
      printf("Prices prev %d - CreateNewHashTable + newBlocks\n",d[0]);

      prices[i] = d[0];
      elements++;
    }      
    printf("\n\n\n\n");


    int new_blocks_num;
    new_blocks_num = (a - previous_a)/( BF_BLOCK_SIZE/sizeof(int)); 
    printf("new_blocks = %d - CreateNewHashTable\n", new_blocks_num);
    //δεσμευω τα καινουριο new_blocks_num που χρειαζονται
    //επισης τα αποθηκευω στο τελος του μπλοκ με της πληροφοριες
    block_info = 0;

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
        d1 = data[0];
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
      // printf("PINAKAS TOY HASSH GIA NEA HASH %d\n\n", num_block);
    }

    //if this block is the first( zero)    
    if( block_info == 0)
    {
      num++; //epeidi stin arxi to data mas einai data = Bf_getdata + sizeof(int) --- > giati exei to global_depth
    }

    int new_info = -2;
    int end_info = -1;
    CALL_BF( BF_UnpinBlock( block));  //itan slash
    // printf("blcok_info =  %d kia num = %d\n\n", block_info, num);
    CALL_BF( BF_GetBlock( filedesc, block_info, block));
    // printf("blcok_info =  %d kia num = %d\n\n", block_info, num);

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
      // printf("OPAAAAAAAAAAAAAAAAAAAAA %d\n\n", new_hash_block);
      int* d11 = data + num*sizeof(int);
      // printf("d11 = %d\n\n",d11[0]);




      // CALL_BF(BF_GetBlock(filedesc, block_info, block));
      // data = BF_Block_GetData( block);

      // d11 = data + num*sizeof(int);
      // printf("d11 = %d\n\n",d11[0]);
      memcpy( data + num*sizeof(int), &new_hash_block, sizeof(int)); //the new hashblock in the end( price = -1)
      
      
      
      
      
      // printf("allgof\n");
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

    // depth++; //new depth
    // int previous_a = a; //previous 2^depth
    // int previous_depth = depth - 1; //previous depth
    // a = a*2;  //new 2^depth


    // //διαβαζω τις προηγουμενες τιμες του hash table και
    // //τις αποθηκευω σε ενα πινακα
    // int prices[ previous_a];
    // int elements = 0;
    // int num_from_info_to_hash = 1;
    // int Max_at_block = BF_BLOCK_SIZE/sizeof(int);
    //τωρα πρεπει να επεκτεινουμε το hashtable
    block_info = 0;
    int num_to_block_info = 2; //epeidi to proto int einai vathos kai to deytero einai to block =1 h arxi eyretyrioy poy to kseroyme
    CALL_BF( BF_UnpinBlock( block));
    CALL_BF( BF_GetBlock(filedesc, block_info, block));
    data = BF_Block_GetData(block);

    //new depth write in the first block information in the start
    int new_depth = depth;
    memcpy(data, &new_depth, sizeof(int));
    BF_Block_SetDirty(block);

    block_hash = 1;
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
      printf("Pos = %d   bucket = %d - CreateNewHashTable + newBlocks\n",num_of_ints, v);

      int* d1;
      if( data + num_for_hash_block*sizeof(int) > data + BF_BLOCK_SIZE -1)
      {
        //TOTE prepei na allakso to num_block_hash
        //ara diavazo apo to block_info
        CALL_BF( BF_UnpinBlock( block));
        CALL_BF( BF_GetBlock( filedesc, block_info, block));
        char* d_1 = BF_Block_GetData( block);
        d1 = d_1 + num_to_block_info*sizeof(int);
    
        /////
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

  }
  CALL_BF( BF_UnpinBlock( block));

  CreateNewBucket( filedesc, record, bucket_b);
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
  
  // //με την προυποθεση οτι το βαθος θα ειναι μεγαλυτερο ή ισο με 1
  // if( blocks_num < 3)
  // { 
  //   BF_Block_Destroy( &block);
  //   return HT_ERROR;
  // }


  int i = 0;

  //GetBlock with data
  //το πρωτο μπλοκ εχει τις πληροφοριες
  CALL_BF(BF_GetBlock(filedesc, i, block));
  char* data;
  data = BF_Block_GetData( block);

  
  //global depth 
  int depth = data[0];
  CALL_BF( BF_UnpinBlock( block));                                            //PALIA ITAN
  //Hashing
  int HashNum = HashFunction( record, depth);

  int* d;
  // int num_block_hash = 3;

  //το 2ο μπλοκ του αρχειου ειναι το 1ο μπλοκ του ευρετηριου
  i = 1;
  int num_info = 1;
  int block_info = 0;                                                           //itan paliiia
  CALL_BF(BF_GetBlock(filedesc, i, block));
  data = BF_Block_GetData( block);


  printf("hasharei %d - insert\n", HashNum);
  
  
  //Αν η θεση του πινακα που χασαρουμε δεν ειναι στο 1ο μπλοκ ευρετηριου τοτε παμε στο
  //επομενο και ουτε καθεξης
  while( data+ HashNum*sizeof(int) > data+BF_BLOCK_SIZE-1 )
  { 
    // printf("elllaaaa %d\n",i);
    CALL_BF( BF_UnpinBlock( block));
    //αρα διαβαζουμε απο το μπλοκ πληροφοριες ποιο ειναι το επομενο
    CALL_BF( BF_GetBlock( filedesc, block_info, block));
    
    char *data1;
    //παιρνουμε τον αριθμο του επομενου μπλοκ ευρετηριου
    if( block_info == 0)
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
      //////////////////////////////////////////FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF DEITTTTTTEEEE TOOOO
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
      data1 = BF_Block_GetData( block); //+ sizeof(int);
      num_info = 0;
      d1 = data1;
    }
    num_info++;
    i = d1[0];

    CALL_BF( BF_UnpinBlock( block));
    CALL_BF(BF_GetBlock(filedesc, i, block));
    printf("I = %d\n",i);
    data = BF_Block_GetData( block);
    HashNum = HashNum - 128;

  }

  //παιρνουμε σε ποιο μπλοκ χασαρει τελικα
  d = data + HashNum*sizeof(int);
  
  // printf("\n\necooo gia bucket %d - insert\n", d[0]);
  
  int bucket = d[0];
  printf(" buck = %d kai d[0] = %d\n", bucket, d[0]);

  //παιρνουμε τον αριθμο των μπλοκς
  int num_blocks;
  BF_GetBlockCounter( filedesc, &num_blocks);
  
  CALL_BF( BF_UnpinBlock( block));
  CALL_BF(BF_GetBlock( filedesc, bucket, block));
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
  if( k == (BF_BLOCK_SIZE-sizeof(int))/sizeof(record))
  { 
    //η περιπτωση που το μπλοκ μας ειναι γεματο

    data = BF_Block_GetData(block);
    int depth_bucket = data[0];
    CALL_BF( BF_UnpinBlock( block)); 

    ///checkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk
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
    printf("Global_d = %d kai Topic_d = %d - InsertEntry\n", depth, depth_bucket);
 

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

    CALL_BF( BF_UnpinBlock( block));

  }
  BF_Block_Destroy( &block);
  return HT_OK;
}

HT_ErrorCode HT_PrintAllEntries(int indexDesc, int *id) {
  //insert code here
  return HT_OK;
}

HT_ErrorCode HT_HashStatistics(char* filename){
  int* indexDesc;
  HT_OpenIndex(filename, indexDesc);

  int* num_of_blocks;

  int fileDesc = filetable->table[*indexDesc]->file_desc;
  int count = BF_GetBlockCounter(fileDesc, num_of_blocks);

  printf("Hash File named %s has %d blocks\n", filename, count);

  BF_Block* block;
  BF_Block_Init(&block);

  int i = 1;
  BF_GetBlock(fileDesc, i, block);
  char* data = BF_Block_GetData(block);

  CALL_BF( BF_UnpinBlock( block));

  return HT_OK;
}


void other()
{
  // CALL_BF( BF_UnpinBlock( block));

  
  // CALL_BF( BF_GetBlock( filedesc, 1, block));
  // data = BF_Block_GetData( block);
  // int a = 1;
  // for(int k = 0; k < depth; k++)
  // {
  //   a = a*2;
  // }
  // for( int i = 0; i < a; i++)
  // {
  //     int* d = data + i*sizeof(int);
  //     printf("to table m mitso   %d data:  %d\n",d[0], d);

  // }




// printf("new %d old %d\n", num_old, num_new);
  
  //PRINT OLDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD
  // k = 0;
  // // printf("ALL FOR OLD: %d\n", old_bucket);
  // CALL_BF( BF_UnpinBlock( block));
  // BF_GetBlock(filedesc, old_bucket, block);
  // data = BF_Block_GetData( block) + sizeof(int);
  // while( k < (BF_BLOCK_SIZE-sizeof(int))/sizeof(record))
  // {   

  //     d1 = data + k*sizeof(record);
  //     int id = d1[0];


  //     char* d2 = data + k*sizeof(record) + sizeof(int);
  //     // char* nam = d2[0];
  //     // char nam[15];
  //     strcpy(name, d2);
  //     // printf(" elaa %s\n", d2[0]);

  //     char* d3 = data + k*sizeof(record) + sizeof(int) + sizeof(name);
  //     // char* snam = d1[0];
  //     strcpy(surname, d3);

  //     char* d4 = data + k*sizeof(record) + sizeof(int) + sizeof(name) + sizeof(surname);
  //     // char* cit = d1[0];
  //     strcpy(city, d4);

  //     if( name == NULL || strlen( name) == 0)
  //     {
  //       // printf("ta pameeeeeeeeeeeeeeeeeeeeeee\n");
  //       break;
  //     }
  //     // printf("to record mas me stoixeia :%d, %s, %s, %s\n", id, name, surname, city);

  //     k++;
      
  // }



  //   //PRINT NEWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW
  // k = 0;
  // // printf("ALL FOR NEW: %d\n", new_bucket);
  // // CALL_BF( BF_UnpinBlock( block));
  // BF_GetBlock(filedesc, new_bucket, block);
  // data = BF_Block_GetData( block) + sizeof(int);
  // while( k < (BF_BLOCK_SIZE-sizeof(int))/sizeof(record))
  // {   

  //     d1 = data + k*sizeof(record);
  //     int id = d1[0];


  //     char* d2 = data + k*sizeof(record) + sizeof(int);
  //     strcpy(name, d2);

  //     char* d3 = data + k*sizeof(record) + sizeof(int) + sizeof(name);
  //     strcpy(surname, d3);

  //     char* d4 = data + k*sizeof(record) + sizeof(int) + sizeof(name) + sizeof(surname);
  //     strcpy(city, d4);

  //     if( name == NULL || strlen( name) == 0)
  //     {
  //       // printf("ta pameeeeeeeeeeeeeeeeeeeeeee\n");
  //       break;
  //     }
  //     // printf("to record mas me stoixeia :%d, %s, %s, %s\n", id, name, surname, city);

  //     k++;
      
  // }
  // CALL_BF( BF_GetBlock( filedesc, 1, block));
  // data = BF_Block_GetData( block);
  // int a = 1;
  // for(int k = 0; k < global_depth; k++)
  // {
  //   a = a*2;
  // }
  // for( int i = 0; i < a; i++)
  // {
  //     int* d = data + i*sizeof(int);
  //     printf("to table m mitso   %d data:  %d\n",d[0], d);

  // }



   //Unpin the blocks
  // // int blocks_num;
  // CALL_BF( BF_GetBlockCounter( file_desc, &blocks_num));
  

  // // PRINTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
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



  // print all elements
  // PRINTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
  // data = BF_Block_GetData( block);
  // for( int i = 0; i < 4; i++)
  // {
  //     int* d = data + i*sizeof(int);
  //     printf("EDO EIMAI   %d data:  %d\n",d[0], d);

  // }
  // CALL_BF( BF_AllocateBlock( file_desc, block));
}