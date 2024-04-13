
#include "hashtable.h"
#include <stdlib.h>
#include <stdio.h>

HashTable *createHashTable(int size, unsigned int (*hashFunction)(void *),
                           int (*equalFunction)(void *, void *)) {
  int i = 0;
  HashTable *newTable = malloc(sizeof(HashTable));
  newTable->size = size;
  newTable->used = 0;
  newTable->data = malloc(sizeof(struct HashBucket *) * size);
  for (i = 0; i < size; ++i) {
    newTable->data[i] = NULL;
  }
  newTable->hashFunction = hashFunction;
  newTable->equalFunction = equalFunction;
  return newTable;
}

void freeTable(HashTable *table){
  int i = 0;
  for(i = 0; i < table->size; ++i){
    struct HashBucket *at = table->data[i];
    struct HashBucket *old = NULL;
    while(at != NULL){
      old = at;
      at = at->next;
      free(old);
    }
  }
  free(table->data);
  free(table);
}

void insertData(HashTable *table, void *key, void *data) {
  unsigned int location  = 0;
  struct HashBucket *newBucket =
      (struct HashBucket *)malloc(sizeof(struct HashBucket));
  /*
   * At this point we need to resize the table as the occupancy is too
   * high.
   */
  if(table->used > table->size) {
    int oldSize = table->size;
    struct HashBucket **oldData = table->data;
    int i = 0;
    table->size = table->size * 2;
    table->used = 0;
    table->data = malloc(sizeof(struct HashBucket *) * table->size);
    for(i = 0; i < table->size; ++i){
      table->data[i] = NULL;
    }
    for(i = 0; i < oldSize; ++i){
      struct HashBucket *at = oldData[i];
      struct HashBucket *old = NULL;
      while(at != NULL) {
	insertData(table, at->key, at->data);
	old = at;
	at = at->next;
	free(old);
      }
    }
    free(oldData);

  }
  location  = ((table->hashFunction)(key)) % table->size;
  newBucket->next = table->data[location];
  newBucket->data = data;
  newBucket->key = key;
  table->data[location] = newBucket;
  table->used += 1;
}

void *findData(HashTable *table, void *key) {
  unsigned int location = ((table->hashFunction)(key)) % table->size;
  struct HashBucket *lookAt = table->data[location];
  while (lookAt != NULL) {
    if ((table->equalFunction)(key, lookAt->key) != 0) {
      return lookAt->data;
    }
    lookAt = lookAt->next;
  }
  return NULL;
}
