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








//ενας struct που αποθηκευουμε δομες μας
typedef struct HT {
	char** table_file;    //πινακας συμβολοσειρων με ονοματα αρχειων(filename)
  int size_table_file;    //αρχεια
} HT_tables;


//καθολικη μεταβλητη - για τις δομες μας
HT_tables* tables;





/*
 * Η συνάρτηση HT_Init χρησιμοποιείται για την αρχικοποίηση κάποιον δομών που μπορεί να χρειαστείτε. 
 * Σε περίπτωση που εκτελεστεί επιτυχώς, επιστρέφεται HT_OK, ενώ σε διαφορετική περίπτωση κωδικός λάθους.
 */
HT_ErrorCode HT_Init() 
{
  //insert code here
  tables = malloc(sizeof(struct HT));

  //αρχικοποιουμε τις δομες μας
  //το πολυ 20 αρχεια μπορουν να υπαρχουν ανοικτα
  tables->table_file = malloc(20*sizeof(char*));
  if(tables->table_file == NULL)
  {
    return HT_ERROR;
  }
  for(int i = 0; i < 20; i++)
  {
    tables->table_file[i] = malloc(100*sizeof(char));
    if( tables->table_file[i] == NULL)
    {
      return HT_ERROR;
    }
  }

  tables->size_table_file = 0;
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
  int size_table = tables->size_table_file;
  bool find = false;

  //τσεκαρουμε αν το filename υπαρχει στο πινακα μας
  for(int i = 0; i < size_table; i++)
  {
    char* file = tables->table_file[i];
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
    if( tables->size_table_file == 20)
    {
      return HT_ERROR;
    }
    tables->table_file[(tables->size_table_file)++] = filename;
  }
  //αν υπαρχει τοτε μυνημα σφαλματος
  else
  {
    return HT_ERROR;
  }



  return HT_OK;
}




















HT_ErrorCode HT_OpenIndex(const char *fileName, int *indexDesc){
  //insert code here
  return HT_OK;
}

HT_ErrorCode HT_CloseFile(int indexDesc) {
  //insert code here
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

