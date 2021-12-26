// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>

// #include "bf.h"

// #define CALL_OR_DIE(call)     \
//   {                           \
//     BF_ErrorCode code = call; \
//     if (code != BF_OK) {      \
//       BF_PrintError(code);    \
//       exit(code);             \
//     }                         \
//   }

// int main() {
//   int fd1;
//   BF_Block *block;
//   BF_Block_Init(&block);

//   CALL_OR_DIE(BF_Init(LRU));
//   CALL_OR_DIE(BF_CreateFile("data1.db"))
//   CALL_OR_DIE(BF_OpenFile("data1.db", &fd1));


//   char* data;
//   for (int i = 0; i < 1000; ++i) {
//     CALL_OR_DIE(BF_AllocateBlock(fd1, block));
//     data = BF_Block_GetData(block);
//     memset(data, i % 127, BF_BUFFER_SIZE);
//     BF_Block_SetDirty(block);
//     CALL_OR_DIE(BF_UnpinBlock(block));
//   }
//   for (int i = 0; i < 1000; ++i) {
//     CALL_OR_DIE(BF_GetBlock(fd1, i, block));
//     data = BF_Block_GetData(block);
//     printf("block = %d and data = %d\n", i, data[0]);
//     CALL_OR_DIE(BF_UnpinBlock(block));
//   }

//   CALL_OR_DIE(BF_CloseFile(fd1));
//   CALL_OR_DIE(BF_Close());

//   CALL_OR_DIE(BF_Init(LRU));
//   CALL_OR_DIE(BF_OpenFile("data1.db", &fd1));
//   int blocks_num;
//   CALL_OR_DIE(BF_GetBlockCounter(fd1, &blocks_num));

//   for (int i = 0; i < blocks_num; ++i) {
//     CALL_OR_DIE(BF_GetBlock(fd1, i, block));
//     data = BF_Block_GetData(block);
//     printf("block = %d and data = %d\n", i, data[i % BF_BUFFER_SIZE]);
//     CALL_OR_DIE(BF_UnpinBlock(block));
//   }

//   BF_Block_Destroy(&block);
//   CALL_OR_DIE(BF_CloseFile(fd1));
//   CALL_OR_DIE(BF_Close());
// }






/////////////////////////////////
/////tsalaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaassssssssssssssssssssssssssssssssssss
/////////////////////////////
///////////////////////////
////////////////////////////


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/bf.h"

#define CALL_OR_DIE(call)     \
  {                           \
    BF_ErrorCode code = call; \
    if (code != BF_OK) {      \
      BF_PrintError(code);    \
      exit(code);             \
    }                         \
  }

typedef struct Record {
    int id;
    char name[15];
    char surname[20];
    char city[22];
} Record;
int main() {

    printf("sizeof(record):%lu\n",sizeof(Record));

    int fd1;
    BF_Block *block;
    BF_Block_Init(&block);// it allocates the suitable space in memory

    CALL_OR_DIE(BF_Init(LRU));//it uses the LRU method to clear the unneeded blocks from the buffer
    // CALL_OR_DIE(BF_CreateFile("data11.db"))
    CALL_OR_DIE(BF_OpenFile("data11.db", &fd1));// it gives the data1.db file a specific ID(lets say ID=11)


    char* data;
    for (int i = 0; i < 10; ++i) {
        //In data1.dp we allocate a new block at the end of the file
        CALL_OR_DIE(BF_AllocateBlock(fd1, block));
        data = BF_Block_GetData(block);// we take info from the block we just allocated to data1.db
        memcpy(data, &i, sizeof(int));// we change the data
        BF_Block_SetDirty(block);
        CALL_OR_DIE(BF_UnpinBlock(block));
    }
    char str[10];
    for (int i = 0; i < 10; ++i) {
        CALL_OR_DIE(BF_GetBlock(fd1, i, block));//it finds the block_file with ID=fd1(11)
        // and it search for block with block_num==i and returns this block to var "block"
        data = BF_Block_GetData(block); // we take the info from the block we just searched for!
        int result;
        memcpy(&result,data,sizeof(int));
        printf("block = %d and data = %d\n", i, result);
        CALL_OR_DIE(BF_UnpinBlock(block));// we dodnt need this block anymore so we unpinned it from the buffer
    }

    CALL_OR_DIE(BF_CloseFile(fd1));// we close the specific buffer
    CALL_OR_DIE(BF_Close());// we close the BLOCK_LEVEL and we write all the blocks from the buffer back to the disk;

    CALL_OR_DIE(BF_Init(LRU));
    CALL_OR_DIE(BF_OpenFile("data11.db", &fd1));
    int blocks_num;
    CALL_OR_DIE(BF_GetBlockCounter(fd1, &blocks_num));//
    printf("DATA11.DB NUMBER OF BLOCKS:%d\n",blocks_num);

    for (int i = 0; i < blocks_num; ++i) {
        CALL_OR_DIE(BF_GetBlock(fd1, i, block));
        data = BF_Block_GetData(block);
        int result;
        memcpy(&result,data, sizeof(int));
        printf("block = %d and data = %d\n", i, result);
        CALL_OR_DIE(BF_UnpinBlock(block));
    }

    BF_Block_Destroy(&block);
    CALL_OR_DIE(BF_CloseFile(fd1));
    CALL_OR_DIE(BF_Close());
}
