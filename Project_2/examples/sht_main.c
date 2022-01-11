#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "bf.h"
#include "hash_file.h"
#include "sht_file.h"

#define RECORDS_NUM 1000 // you can change it if you want
#define GLOBAL_DEPT 2 // you can change it if you want

//1
#define FILE_NAME "data.db"
#define FILE_NAME_SEC "data_sec.db"

//2
#define FILE_NAME1 "data1.db"
#define FILE_NAME_SEC1 "data_sec1.db"



const char* names[] = {
  "Yannis",
  "Christofos",
  "Sofia",
  "Marianna",
  "Vagelis",
  "Maria",
  "Iosif",
  "Dionisis",
  "Konstantina",
  "Theofilos",
  "Giorgos",
  "Dimitris"
};

const char* surnames[] = {
  "Ioannidis",
  "Svingos",
  "Karvounari",
  "Rezkalla",
  "Nikolopoulos",
  "Berreta",
  "Koronis",
  "Gaitanis",
  "Oikonomou",
  "Mailis",
  "Michas",
  "Halatsis"
};

const char* cities[] = {
  "Athens",
  "San Francisco",
  "Los Angeles",
  "Amsterdam",
  "London",
  "New York",
  "Tokyo",
  "Hong Kong",
  "Munich",
  "Miami",
  "PEANUT",
  "Tokyo",
  "Delhi",
  "Shanghai", 
  "Sao Paulo",
  "Mexico City",
  "Dhaka",
  "Cairo",
  "Beijing",
  "Mumbai",
  "Osaka",
  "Karachi",
  "Chongqing",
  "Istanbul",
  "Buenos Aires",
  "Kolkata",
  "Kinshasa",
  "Lagos",
  "Manila",
  "Tianjin",
  "Guangzhou",
  "Rio De Janeiro",
  "Lahore",
  "Bangalore", 
  "Moscow"
};

#define CALL_OR_DIE(call)     \
  {                           \
    HT_ErrorCode code = call; \
    if (code != HT_OK) {      \
      printf("Error\n");      \
      exit(code);             \
    }                         \
  }

int main() {

  BF_Init(LRU);
  
  CALL_OR_DIE(HT_Init());
  CALL_OR_DIE(SHT_Init());

  int indexDesc;
  CALL_OR_DIE( HT_CreateIndex(FILE_NAME, GLOBAL_DEPT));
  
  int sindexDesc;
  char city[5] = "city";
  // CALL_OR_DIE( SHT_CreateSecondaryIndex(FILE_NAME_SEC, city, 4, GLOBAL_DEPT, FILE_NAME));


  CALL_OR_DIE( HT_OpenIndex(FILE_NAME, &indexDesc)); 
// printf("oo\n");

  // CALL_OR_DIE( SHT_OpenSecondaryIndex( FILE_NAME_SEC, &sindexDesc));

  TupleId tid;


  Record record;
  SecondaryRecord srecord;
  srand( 12569874);
  int r;
  UpdateRecordArray* updateArray;

  // for (int id = 0; id < 269; ++id) {

  //   char city1[20] = "T";
  //   char num[19];

  //   sprintf(num, "%d", rand()%(id+1));
  //   strcat(city1, num);

  //   record.id = id;
  //   r = rand() % 12;
  //   memcpy(record.name, names[r], strlen(names[r]) + 1);
  //   r = rand() % 12;
  //   memcpy(record.surname, surnames[r], strlen(surnames[r]) + 1);
  //   r = rand() % 30;
  //   memcpy(record.city, city1, strlen(city1) + 1);

  //   updateArray = calloc((BF_BLOCK_SIZE - sizeof(int))/sizeof( Record), sizeof( UpdateRecordArray)); 
    
  //   CALL_OR_DIE(HT_InsertEntry(indexDesc, record, &tid, updateArray));

  //   if( updateArray[0].city != NULL && strlen(updateArray[0].city) != 0 && updateArray[0].surname != NULL && strlen(updateArray[0].surname) != 0)
  //   {
  //     CALL_OR_DIE( SHT_SecondaryUpdateEntry( sindexDesc, updateArray));
  //   }


  //   // if(strcmp( city1, "T1") == 0)
  //   // {
  //   //   printf("id = %d\n", id);
  //   //   printf("t.b = %d kai i = %d\n", tid.block, tid.index);
  //   // }

    
    
  //   strcpy(srecord.index_key, record.city);
  //   // if( id == 103)
  //   // {
  //   //   printf("c == %s\n\n", srecord.index_key);
  //   // }
  //   srecord.tupleId.block = tid.block;
  //   srecord.tupleId.index = tid.index;
  
  //   CALL_OR_DIE(SHT_SecondaryInsertEntry( sindexDesc, srecord));

  //   free( updateArray);
  // }

  // printf("\n\n\n\n\n\n\n");

  int indexDesc1;
  CALL_OR_DIE( HT_CreateIndex(FILE_NAME1, GLOBAL_DEPT));

  
  int sindexDesc1;	
  CALL_OR_DIE( SHT_CreateSecondaryIndex(FILE_NAME_SEC1, city, 4, GLOBAL_DEPT, FILE_NAME1));


  CALL_OR_DIE( HT_OpenIndex(FILE_NAME1, &indexDesc1)); 

  CALL_OR_DIE( SHT_OpenSecondaryIndex( FILE_NAME_SEC1, &sindexDesc1));


  srand( 673897247);

  for (int id = 0; id < 50; ++id) {


    char city1[20] = "T";
    char  num[19];

    sprintf(num, "%d", rand()%(id+1));
    strcat(city1, num);

    record.id = id;
    r = rand() % 12;
    memcpy(record.name, names[r], strlen(names[r]) + 1);
    r = rand() % 12;
    memcpy(record.surname, surnames[r], strlen(surnames[r]) + 1);
    r = rand() % 30;
    memcpy(record.city, city1, strlen(city1) + 1);

    updateArray = calloc((BF_BLOCK_SIZE - sizeof(int))/sizeof( Record), sizeof( UpdateRecordArray)); 
    
    CALL_OR_DIE(HT_InsertEntry(indexDesc1, record, &tid, updateArray));


    CALL_OR_DIE( SHT_SecondaryUpdateEntry( sindexDesc1, updateArray));


    strcpy(srecord.index_key, record.city);
    srecord.tupleId.block = tid.block;
    srecord.tupleId.index = tid.index;
  
    CALL_OR_DIE(SHT_SecondaryInsertEntry( sindexDesc1, srecord));

    free( updateArray);
  }


  CALL_OR_DIE( SHT_PrintAllEntries(sindexDesc1, "T27"));


  // CALL_OR_DIE( SHT_InnerJoin( sindexDesc, sindexDesc1, "T1") );

printf("i am out\n");










  ////////////////////////
  //THEMAAAAAAAAAAAAAAAAAAAAAAAAAAAA ME COLSE SINDEXDESC
  //
  /////////////////////////


  CALL_OR_DIE(SHT_CloseSecondaryIndex(sindexDesc1));
  // CALL_OR_DIE(SHT_CloseSecondaryIndex(sindexDesc));
  CALL_OR_DIE(HT_CloseFile(indexDesc1));
  CALL_OR_DIE(HT_CloseFile(indexDesc));

}
