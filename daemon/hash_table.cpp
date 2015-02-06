#include "hash_table.h"

size_t HashTableEvalTableSize(size_t uNodeSize, size_t uRowNum, size_t auNodeNums[])
{
	int i;
	size_t uNodeCnt;

	if (uRowNum >= HASH_MAX_ROW) { return 0; }
	for (i=0, uNodeCnt=0; i<uRowNum; i++) {
		uNodeCnt += auNodeNums[i];
	}
	return (uNodeSize * uNodeCnt);
}

int HashTableInit(HashTable *pstHashTable, void *pTable, size_t uTableSize, size_t uNodeSize, size_t uRowNum, size_t auNodeNums[], size_t auMods[], int (*Compare)(const void *pKey, const void *pNode))
{
	int i;
	size_t uNodeCnt;
	if (!pstHashTable) { return -1; }


#define CHECK_AND_SET(x) ({ if (!(pstHashTable->x = x)) { return -1; } })
	CHECK_AND_SET(Compare);
	CHECK_AND_SET(pTable);
	CHECK_AND_SET(uNodeSize);
	CHECK_AND_SET(uRowNum);
	if (uRowNum >= HASH_MAX_ROW) { return -1; }
#undef CHECK_AND_SET

#define CHECK_AND_SET(x) ({ if (!(pstHashTable->x[i] = x[i])) { return -1; } })
	for (i=0, uNodeCnt=0; i<uRowNum; i++) {
		CHECK_AND_SET(auNodeNums);
		CHECK_AND_SET(auMods);
		uNodeCnt += auNodeNums[i];
	}
#undef CHECK_AND_SET
	if (uTableSize != uNodeSize * uNodeCnt) { return -1; }

	return 0;
}

void *HashTableSearch(HashTable *pstHashTable, const void *pKey, size_t uShortKey)
{
	int i;
	void *pRow, *pNode;
	size_t uNodeSize = pstHashTable->uNodeSize;

	for (i = 0, pRow = pstHashTable->pTable;
			i < pstHashTable->uRowNum;
			pRow += uNodeSize * pstHashTable->auNodeNums[i], i++) {
		pNode = pRow + uNodeSize * (uShortKey % pstHashTable->auMods[i]);
		if (pstHashTable->Compare(pKey, pNode) == 0) { return pNode; }
	}

	return NULL;
}

void *HashTableSearchEx(HashTable *pstHashTable, const void *pSearchKey, const void *pEmptyKey, size_t uShortKey, int *piExist)
{
	int i;
	void *pRow, *pNode, *pEmptyNode = NULL, *pFoundNode = NULL;
	size_t uNodeSize = pstHashTable->uNodeSize;

	for (i = 0, pRow = pstHashTable->pTable;
			i < pstHashTable->uRowNum;
			pRow += uNodeSize * pstHashTable->auNodeNums[i], i++) {
		pNode = pRow + uNodeSize * (uShortKey % pstHashTable->auMods[i]);
		if (pstHashTable->Compare(pSearchKey, pNode) == 0) {
			pFoundNode = pNode; break;
		}
		if (pEmptyNode == NULL && pstHashTable->Compare(pEmptyKey, pNode) == 0) {
			pEmptyNode = pNode;
		}
	}

	if (piExist) { *piExist = (pFoundNode != NULL); }
	return (pFoundNode ? pFoundNode : pEmptyNode);
}

int HashTableSearchAll(HashTable *pstHashTable, size_t uShortKey, int * piCount, void * apAllNodes[])
{
	int i;
	void *pRow, *pNode;
	size_t uNodeSize = pstHashTable->uNodeSize;

	int iCount = 0;
	int iTotal = *piCount;

	for (i = 0, pRow = pstHashTable->pTable;
			i < pstHashTable->uRowNum;
			pRow += uNodeSize * pstHashTable->auNodeNums[i], i++) {
		pNode = pRow + uNodeSize * (uShortKey % pstHashTable->auMods[i]);
		if(iCount >= iTotal) return -2;
		apAllNodes[iCount++] = pNode;
	}

	*piCount = iCount;
	return iCount;
}

int HashTableSearchEmptyN(HashTable *pstHashTable, const void *pEmptyKey, size_t uShortKey, int * piCount, void * apEmptyNodes[])
{
	int i;
	void *pRow, *pNode;
	size_t uNodeSize = pstHashTable->uNodeSize;

	int iCount = 0;
	int iTotal = *piCount;

	for (i = 0, pRow = pstHashTable->pTable;
			i < pstHashTable->uRowNum;
			pRow += uNodeSize * pstHashTable->auNodeNums[i], i++) {
		pNode = pRow + uNodeSize * (uShortKey % pstHashTable->auMods[i]);
		if (pstHashTable->Compare(pEmptyKey, pNode) == 0) {
			if(iCount >= iTotal) return -2;
			apEmptyNodes[iCount++] = pNode;
		}
	}

	*piCount = iCount;
	return iCount;
}

int HashTableSearchN(HashTable *pstHashTable, const void *pSearchKey, size_t uShortKey, int * piCount, void * apNodes[])
{
	int i;
	void *pRow, *pNode;
	size_t uNodeSize = pstHashTable->uNodeSize;

	int iCount = 0;
	int iTotal = *piCount;

	for (i = 0, pRow = pstHashTable->pTable;
			i < pstHashTable->uRowNum;
			pRow += uNodeSize * pstHashTable->auNodeNums[i], i++) {
		pNode = pRow + uNodeSize * (uShortKey % pstHashTable->auMods[i]);
		if (pstHashTable->Compare(pSearchKey, pNode) == 0) {
			if(iCount >= iTotal) return -2;
			apNodes[iCount++] = pNode;
		}
	}

	*piCount = iCount;
	return iCount;
}
