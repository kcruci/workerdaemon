#ifndef _HASH_TABLE_H_INCLUDED_
#define _HASH_TABLE_H_INCLUDED_

#include <stdlib.h>

#define HASH_MAX_ROW 100

typedef struct {
	void *pTable;
	size_t uNodeSize;
	size_t uRowNum;
	size_t auNodeNums[HASH_MAX_ROW];
	size_t auMods[HASH_MAX_ROW];

	int (*Compare)(const void *pKey, const void *pNode);
} HashTable;

size_t HashTableEvalTableSize(size_t uNodeSize, size_t uRowNum, size_t auNodeNums[]);

int HashTableInit(HashTable *pstHashTable,void *pTable, size_t uTableSize, size_t uNodeSize, size_t uRowNum, size_t auNodeNums[], size_t auMods[], int (*Compare)(const void *pKey, const void *pNode));

void *HashTableSearch(HashTable *pstHashTable,const void *pKey, size_t uShortKey);
void *HashTableSearchEx(HashTable *pstHashTable,const void *pSearchKey, const void *pEmptyKey, size_t uShortKey, int *piExist);

int HashTableSearchAll(HashTable *pstHashTable,size_t uShortKey, int * piCount, void * apAllNodes[]);
int HashTableSearchEmptyN(HashTable *pstHashTable,const void *pEmptyKey, size_t uShortKey, int * piCount, void * apEmptyNodes[]);
int HashTableSearchN(HashTable *pstHashTable,const void *pSearchKey, size_t uShortKey, int * piCount, void * apNodes[]);
#endif
