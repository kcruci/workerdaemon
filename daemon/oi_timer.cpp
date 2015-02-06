#include "oi_timer.h"
#include "oi_list.h"
//#include "oi_macro.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <sys/time.h>
#include <unistd.h>
#include "hash_table.h"

typedef struct
{
	unsigned uiSeq;
	unsigned uiExpire;
	ListHead list;
	ExpireFunc OnExpire;
	unsigned uiDataLen;
	unsigned uiPad;
	char sData[0];
}TimerNode;

#define TVN_BITS 6
#define TVR_BITS 8
#define TVN_SIZE (1 << TVN_BITS)
#define TVR_SIZE (1 << TVR_BITS)
#define TVN_MASK (TVN_SIZE - 1)
#define TVR_MASK (TVR_SIZE - 1)

typedef struct
{
	int index;
	ListHead vec[TVR_SIZE];
}TimerVecRoot;

typedef struct
{
	int index;
	ListHead vec[TVN_SIZE];
}TimerVec;

static TimerVec tv5;
static TimerVec tv4;
static TimerVec tv3;
static TimerVec tv2;
static TimerVecRoot tv1;
static TimerVec * const tvecs[] = {
	(TimerVec *)&tv1, &tv2, &tv3, &tv4, &tv5
};
#define NOOF_TVECS (sizeof(tvecs) / sizeof(tvecs[0]))

static HashTable stTimerHash;

static struct timeval stBaseTV;
static unsigned uiCheckTime;
static unsigned uiCheckInvlUs = 0;
static unsigned uiSeq = 0;
static unsigned uiCount = 0;
static unsigned uiMaxCount = 0;
static unsigned uiMaxLen = 0;
static unsigned uiTimerType = 1;

unsigned GetCurTimerNode(void)
{
	return uiCount;
}

unsigned GetMaxTimerNode(void)
{
	return uiMaxCount;
}

unsigned GetMaxDataLen(void)
{
	return uiMaxLen;
}

int CmpTimerHashNode(const void * pKey, const void * pNode)
{
	TimerNode * pstKey = (TimerNode*)pKey;
	TimerNode * pstNode = (TimerNode*)pNode;
	//return (signed)(pstKey->uiSeq - pstNode->uiSeq);
	if(pstKey->uiSeq == pstNode->uiSeq) return 0;
	if(pstKey->uiSeq < pstNode->uiSeq) return -1;
	return 1;
}

/* new must be newer than old */
#define USDIFF(new, old) (1000000 * (unsigned long long)((new).tv_sec - (old).tv_sec) \
		+ (new).tv_usec - (old).tv_usec)

int InitTimer(unsigned uMaxUserDataSize, unsigned uRowNum, size_t auNodeNum[], unsigned uiCheckIntervalUs, unsigned uiType)
{
	{
		//Init tvs
		int i;
		for(i = 0; i < TVN_SIZE; i++) {
			INIT_LIST_HEAD(tv5.vec + i);
			INIT_LIST_HEAD(tv4.vec + i);
			INIT_LIST_HEAD(tv3.vec + i);
			INIT_LIST_HEAD(tv2.vec + i);
		}
		tv5.index = 0;
		tv4.index = 0;
		tv3.index = 0;
		tv2.index = 0;
		for(i = 0; i < TVR_SIZE; i++) {
			INIT_LIST_HEAD(tv1.vec + i);
		}
		tv1.index = 0;
	}
	{
		//Init hash
		int iRet;
		unsigned uTableSize;
		void *pTable;
		memset(&stTimerHash, 0, sizeof(HashTable));
		uTableSize = HashTableEvalTableSize(sizeof(TimerNode) + uMaxUserDataSize, uRowNum, auNodeNum);
		/*
		printf("hash table size %u, node size %u\n", uTableSize, sizeof(TimerNode)+uMaxUserDataSize);
		if (GetShm2(&pTable, SESSION_KEY, uTableSize, 0660|IPC_CREAT) < 0) 
		{
			ERROR_LOG("get shm mem failed! No enough memory.");
			return -10;
		}
		*/

		pTable = malloc(uTableSize);
		if(NULL == pTable) {
			printf("malloc mem failed! No enough memory.");
			return -10;
		}
		memset(pTable, 0, uTableSize);
		iRet = HashTableInit(&stTimerHash, pTable, uTableSize, 
				sizeof(TimerNode) + uMaxUserDataSize, uRowNum, auNodeNum, auNodeNum, CmpTimerHashNode);
		if(0 > iRet) {
			printf("HashTableInit error return: %d", iRet);
			return iRet;
		}
	}
	{
		//Init time
		struct timeval stTV;
		uiCheckInvlUs = uiCheckIntervalUs;
		if((uiType != CONST_DATA_LEN_TIMER) && (uiType != NON_CONST_DATA_LEN_TIMER)) {
			uiTimerType = CONST_DATA_LEN_TIMER;			//默认采用定长
		} else {
			uiTimerType = uiType;
		}
		if(0 >= uiCheckInvlUs) {
			printf("BUG: uiCheckInvlUs can NOT set to be zero!");
			return -2;
		}
		gettimeofday(&stTV, NULL);
		stBaseTV = stTV;
		uiCheckTime = 0;
	}
	{
		//uiSeq
		//srand(time(NULL));
		//uiSeq = (unsigned)rand();
	}
	return 0;
}

static inline void InternalAddTimer(TimerNode * pstNode)
{
	unsigned expires = pstNode->uiExpire;
	unsigned idx = expires - uiCheckTime;
	ListHead * vec;
	if (idx < TVR_SIZE) {
		int i = (int)(expires & TVR_MASK);
		vec = tv1.vec + i;
	} else if (idx < 1 << (TVR_BITS + TVN_BITS)) {
		int i = (int)((expires >> TVR_BITS) & TVN_MASK);
		vec = tv2.vec + i;
	} else if (idx < 1 << (TVR_BITS + 2 * TVN_BITS)) {
		int i = (int)((expires >> (TVR_BITS + TVN_BITS)) & TVN_MASK);
		vec = tv3.vec + i;
	} else if (idx < 1 << (TVR_BITS + 3 * TVN_BITS)) {
		int i = (int)((expires >> (TVR_BITS + 2 * TVN_BITS)) & TVN_MASK);
		vec = tv4.vec + i;
	} else if ((signed) idx < 0) {
		/* can happen if you add a timer with expires == jiffies,
		 * or you set a timer to go off in the past
		 */
		vec = tv1.vec + tv1.index;
	} else {
		int i = (int)((expires >> (TVR_BITS + 3 * TVN_BITS)) & TVN_MASK);
		vec = tv5.vec + i;
	}
	/*
	 * Timers are FIFO!
	 * same as:
	 * LIST_ADD_TAIL(&(pstNode->list), vec);
	 */
	LIST_ADD(&(pstNode->list), vec->pstPrev);
}


/*
 * SetTimerHashData
 * 设置Timerhash 数据
 *
 * return: 
 * 0成功
 * <0失败错误
 * >0失败需继续
 */
int SetTimerHashData(TimerNode ** ppstNode, unsigned uiSeq, unsigned uiDataLen, char * sData)
{
	int i = 0;
	TimerNode stEmptyNode;
	TimerNode * apEmptyNodes[HASH_MAX_ROW];
	int iEmptyTotal = (int)(DIM(apEmptyNodes));
	unsigned uiNeedCount = uiDataLen / (stTimerHash.uNodeSize - sizeof(TimerNode)) + 
							(0 == (uiDataLen % (stTimerHash.uNodeSize - sizeof(TimerNode))) ? 0 : 1);
	int iEmptyCount;
	stEmptyNode.uiSeq = 0;
	iEmptyCount = HashTableSearchEmptyN(&stTimerHash, (const void*)&stEmptyNode, uiSeq, &iEmptyTotal, (void **)apEmptyNodes);
	if(0 > iEmptyCount) return -1;
	if((unsigned)iEmptyCount < uiNeedCount) return 1;
	for(i = 0; i + 1 < uiNeedCount; i++) {
		apEmptyNodes[i]->uiSeq = uiSeq;
		memcpy(apEmptyNodes[i]->sData, sData + (stTimerHash.uNodeSize - sizeof(TimerNode)) * i, stTimerHash.uNodeSize - sizeof(TimerNode));
	}
	apEmptyNodes[i]->uiSeq = uiSeq;
	memcpy(apEmptyNodes[i]->sData, sData + (stTimerHash.uNodeSize - sizeof(TimerNode)) * i, uiDataLen - (stTimerHash.uNodeSize - sizeof(TimerNode)) * i);
	if(ppstNode)*ppstNode = apEmptyNodes[0];
	return 0;
}

/*
 * GetTimerHashData
 * 取TimerHash数据
 * return
 * 0成功
 * <0失败
 */
int GetTimerHashData(unsigned uiSeq, unsigned * puiDataLen, char ** psData)
{
	static char sData[65536];
	TimerNode stNode;
	int i = 0;
	TimerNode * apNodes[HASH_MAX_ROW];
	int iNodeTotal = (int)(DIM(apNodes));
	int iNodeCount;
	stNode.uiSeq = uiSeq;
	iNodeCount = HashTableSearchN(&stTimerHash, (const void*)&stNode, uiSeq, &iNodeTotal, (void **)apNodes);
	if(0 >= iNodeCount) return -1;
	if(sizeof(TimerNode) * (unsigned)iNodeCount + apNodes[0]->uiDataLen > stTimerHash.uNodeSize * (unsigned)iNodeCount) {
		return -2;
	}
	for(i = 0; i + 1 < iNodeCount; i++) {
		memcpy(sData + (stTimerHash.uNodeSize - sizeof(TimerNode)) * i, apNodes[i]->sData, stTimerHash.uNodeSize - sizeof(TimerNode));
	}
	memcpy(sData + (stTimerHash.uNodeSize - sizeof(TimerNode)) * i, apNodes[i]->sData, apNodes[0]->uiDataLen - (stTimerHash.uNodeSize - sizeof(TimerNode)) * i);

	*puiDataLen = apNodes[0]->uiDataLen;
	*psData = sData;
	return 0;
}

/*
 * ClrTimerHashData
 * 清TimerHash数据
 * return 
 * 0成功
 * >0成功，但数据可能有些问题，如不存在或者长度不符合
 * <0失败
 */
int ClrTimerHashData(TimerNode ** ppstNode, unsigned uiSeq)
{
	//int iRet = 0;
	TimerNode stNode;
	int i = 0;
	TimerNode * apNodes[HASH_MAX_ROW];
	int iNodeTotal = (int)(DIM(apNodes));
	int iNodeCount;
	stNode.uiSeq = uiSeq;
	iNodeCount = HashTableSearchN(&stTimerHash, (const void*)&stNode, uiSeq, &iNodeTotal, (void **)apNodes);
	if(0 >= iNodeCount) return 1;
	/*if(sizeof(TimerNode) * (unsigned)iNodeCount + apNodes[0]->uiDataLen > stTimerHash.uNodeSize * (unsigned)iNodeCount) {
		iRet = 2;
	}*/
	for(i = iNodeCount - 1; i > 0; i--) {
		memset(apNodes[i], 0, sizeof(TimerNode));
	}
	//return iRet;
	if(ppstNode)*ppstNode = apNodes[0];
	return 0;
}

/*
 * GetClrTimerHashData
 * 取出并清空TimerHash数据
 * return:
 * 0成功
 * <0失败
 */
int GetClrTimerHashData(TimerNode ** ppstNode, unsigned uiSeq, unsigned * puiDataLen, char ** psData)
{
	static char sData[65536];
	TimerNode stNode;
	int i = 0;
	TimerNode * apNodes[HASH_MAX_ROW];
	int iNodeTotal = (int)(DIM(apNodes));
	int iNodeCount;
	stNode.uiSeq = uiSeq;
	iNodeCount = HashTableSearchN(&stTimerHash, (const void*)&stNode, uiSeq, &iNodeTotal, (void **)apNodes);
	if(0 >= iNodeCount) return -1;
	if(sizeof(TimerNode) * (unsigned)iNodeCount + apNodes[0]->uiDataLen > stTimerHash.uNodeSize * (unsigned)iNodeCount) {
		apNodes[0]->uiDataLen = (stTimerHash.uNodeSize - sizeof(TimerNode)) * (unsigned)iNodeCount;
	}
	for(i = 0; i + 1 < iNodeCount; i++) {
		memcpy(sData + (stTimerHash.uNodeSize - sizeof(TimerNode)) * i, apNodes[i]->sData, stTimerHash.uNodeSize - sizeof(TimerNode));
	}
	memcpy(sData + (stTimerHash.uNodeSize - sizeof(TimerNode)) * i, apNodes[i]->sData, apNodes[0]->uiDataLen - (stTimerHash.uNodeSize - sizeof(TimerNode)) * i);

	*puiDataLen = apNodes[0]->uiDataLen;
	*psData = sData;
	for(i = iNodeCount - 1; i > 0; i--) {
		memset(apNodes[i], 0, sizeof(TimerNode));
	}
	if(ppstNode)*ppstNode = apNodes[0];
	return 0;
}

int AddTimer(unsigned *puiSeq, unsigned uiTimeOut, 
		ExpireFunc OnExpire, unsigned uiDataLen, char * sData)
{
	int iRet = 0;
	TimerNode * pstNode;
	TimerNode stNode;
	TimerNode stEmptyNode;
	int iExist = 1;
	struct timeval stTV;

	unsigned uiLastSeq = uiSeq;

	if(0 >= uiCheckInvlUs) {
		//not initialized!
		//We can _NOT_ log yet...
		////XXX:ERROR_LOG("BUG: the Timer has not been initialized!");
		return -1;
	}
	
	if(uiTimerType != NON_CONST_DATA_LEN_TIMER)
	{
		if(uiDataLen + sizeof(TimerNode) > stTimerHash.uNodeSize) {
			printf("BUG: datalen [%u] is too long, must not longer than [%u]", 
					uiDataLen, stTimerHash.uNodeSize - sizeof(TimerNode));
			return -3;
		}
	} else {
		if(uiDataLen + sizeof(TimerNode) * stTimerHash.uRowNum > stTimerHash.uNodeSize * stTimerHash.uRowNum) {
			printf("BUG: datalen [%u] is too long, must not longer than [%u]", 
					uiDataLen, (stTimerHash.uNodeSize - sizeof(TimerNode)) * stTimerHash.uRowNum);
			return -3;
		}
	}

	stEmptyNode.uiSeq = 0;
	while(iExist) {
		uiSeq++;
		if(!uiSeq) uiSeq++;
		////noneed -- XXX:hash table can not hold MAX_UNSIGNED timers
		if(uiLastSeq == uiSeq) {
			printf("Error: too many timers, reaching the timer limit!");
			return -1;
		}
		stNode.uiSeq = uiSeq;
		if(uiTimerType != NON_CONST_DATA_LEN_TIMER) {
			if(NULL == (pstNode = HashTableSearchEx(&stTimerHash, 
							(const void*)&stNode, (const void*)&stEmptyNode, 
							uiSeq, &iExist))) {
				//Need Warnning here?
				//Attr_API(, 1);
					printf("Error: too many timers. the timer hash is almost used up!");
				return -2;
			}
		} else {
			/*{
			  int i = 0;
			  TimerNode * apEmptyNodes[HASH_MAX_ROW];
			  int iEmptyTotal = (int)(DIM(apEmptyNodes));
			  unsigned uiNeedCount = uiDataLen / (stTimerHash.uNodeSize - sizeof(TimerNode)) + 
			  0 == (uiDataLen % (stTimerHash.uNodeSize - sizeof(TimerNode))) ? 0 : 1;
			  int iEmptyCount = HashTableSearchEmptyN(&stTimerHash, (const void*)&stEmptyNode, uiSeq, &iEmptyTotal, (void * [])apEmptyNodes);
			  if(0 > iEmptyCount) return -1;
			  if((unsigned)iEmptyCount < uiNeedCount) continue;
			  for(i = 0; i + 1 < uiNeedCount; i++) {
			  apEmptyNodes[i]->uiSeq = uiSeq;
			  memcpy(apEmptyNodes[i]->sData, sData + (stTimerHash.uNodeSize - sizeof(TimerNode)) * i, stTimerHash.uNodeSize - sizeof(TimerNode));
			  }
			  apEmptyNodes[i]->uiSeq = uiSeq;
			  memcpy(apEmptyNodes[i]->sData, sData + (stTimerHash.uNodeSize - sizeof(TimerNode)) * i, uiDataLen - (stTimerHash.uNodeSize - sizeof(TimerNode)) * i);
			  pstNode = apEmptyNodes[0];
			  break;
			  }*/
			iRet = SetTimerHashData(&pstNode, uiSeq, uiDataLen, sData);
			if(0 > iRet) return iRet;
			if(0 < iRet) continue;
			break;
		}
	}
	pstNode->uiSeq = uiSeq;
	gettimeofday(&stTV, NULL);
	pstNode->uiExpire = (unsigned)(USDIFF(stTV, stBaseTV) / uiCheckInvlUs + uiTimeOut);
	pstNode->OnExpire = OnExpire;
	pstNode->uiDataLen = uiDataLen;
	
	if(uiTimerType != NON_CONST_DATA_LEN_TIMER) {
		memcpy(pstNode->sData, sData, uiDataLen);
	}

	InternalAddTimer(pstNode);
	uiCount++;
	if(uiCount > uiMaxCount) uiMaxCount = uiCount;
	if(uiDataLen > uiMaxLen) uiMaxLen = uiDataLen;
	if(puiSeq)*puiSeq = uiSeq;
	return 0;
}

int GetTimer(unsigned uiSeq, unsigned * puiDataLen, char ** psData)
{
	//static char sData[65536];
	TimerNode * pstNode;
	TimerNode stNode;

	if(0 >= uiCheckInvlUs) {
		//not initialized, may not need timer, so return 1 as ok
		//We can _NOT_ log yet...
		////XXX:ERROR_LOG("BUG: the Timer has not been initialized!");
		//return -10;
		return -1;
	}

	if(!puiDataLen || !psData) return -2;

	if (uiSeq == 0) return -3;

	stNode.uiSeq = uiSeq;
	if(uiTimerType != NON_CONST_DATA_LEN_TIMER) {
		if(NULL == (pstNode = HashTableSearch(&stTimerHash, (const void*)&stNode, uiSeq))) {
			//not found. be deleted yet or just NOT added at all?
			return -2;
		}

		*puiDataLen = pstNode->uiDataLen;
		*psData = pstNode->sData;
	} else {
		/*{
		  int i = 0;
		  TimerNode * apNodes[HASH_MAX_ROW];
		  int iNodeTotal = (int)(DIM(apNodes));
		  int iNodeCount = HashTableSearchN(&stTimerHash, (const void*)&stNode, uiSeq, &iNodeTotal, (void*[])apNodes);
		  if(0 > iNodeCount) return -1;
		  for(i = 0; i + 1 < iNodeCount; i++) {
		  memcpy(sData + (stTimerHash.uNodeSize - sizeof(TimerNode)) * i, apNodes[i]->sData, stTimerHash.uNodeSize - sizeof(TimerNode));
		  }
		  memcpy(sData + (stTimerHash.uNodeSize - sizeof(TimerNode)) * i, apNodes[i]->sData, apNodes[0]->uiDataLen - (stTimerHash.uNodeSize - sizeof(TimerNode)) * i);

		  pstNode = apNodes[0];
		 *puiDataLen = pstNode->uiDataLen;
		 *psData = sData;
		 }*/
		return GetTimerHashData(uiSeq, puiDataLen, psData);
	}
	
	return 0;
}

int ModTimer(unsigned uiSeq, unsigned uiTimeOut)
{
	TimerNode * pstNode;
	TimerNode stNode;
	struct timeval stTV;

	if(0 >= uiCheckInvlUs) {
		//not initialized, may not need timer, so return 1 as ok
		//We can _NOT_ log yet...
		////XXX:ERROR_LOG("BUG: the Timer has not been initialized!");
		//return -10;
		return -1;
	}

	if (uiSeq == 0)	return -2;
	
	stNode.uiSeq = uiSeq;
	//这里由于第一个就是timernode，因此 NON_CONST_DATA_LEN_TIMER 也不需要修改
	if(NULL == (pstNode = HashTableSearch(&stTimerHash, (const void*)&stNode, uiSeq))) {
		//not found. be deleted yet or just NOT added at all?
		return -2;
	}
	LIST_DEL(&(pstNode->list));
	//pstNode->uiExpire = uiCheckTime + uiTimeOut;
	gettimeofday(&stTV, NULL);
	pstNode->uiExpire = (unsigned)(USDIFF(stTV, stBaseTV) / uiCheckInvlUs + uiTimeOut);
	InternalAddTimer(pstNode);
	return 0;
}

int DelTimer(unsigned uiSeq)
{
	TimerNode * pstNode;
	TimerNode stNode;
	int iRet = -1;
	if(0 >= uiCheckInvlUs) {
		//not initialized, may not need timer, so return 1 as ok
		//We can _NOT_ log yet...
		////XXX:ERROR_LOG("BUG: the Timer has not been initialized!");
		//return -10;
		return -1;
	}

	if (uiSeq == 0)
	{
		return -2;
	}
	
	stNode.uiSeq = uiSeq;
	if(uiTimerType != NON_CONST_DATA_LEN_TIMER) {
		if(NULL == (pstNode = HashTableSearch(&stTimerHash, (const void*)&stNode, uiSeq))) {
			//not found. be deleted yet or just NOT added at all?
			return 1;
		}
	} else {
		/*{
		  int i = 0;
		  TimerNode * apNodes[HASH_MAX_ROW];
		  int iNodeTotal = (int)(DIM(apNodes));
		  int iNodeCount = HashTableSearchN(&stTimerHash, (const void*)&stNode, uiSeq, &iNodeTotal, (void*[])apNodes);
		  if(0 > iNodeCount) return 1;
		  for(i = iNodeCount - 1; i > 0; i--) {
		  memset(apNodes[i], 0, sizeof(TimerNode));
		  }
		  pstNode = apNodes[i];
		  }*/
		iRet = ClrTimerHashData(&pstNode, uiSeq);
		if(0 < iRet) return iRet;
	}
	LIST_DEL(&(pstNode->list));
	uiCount--;
	memset(pstNode, 0, sizeof(TimerNode));
	return 0;
}

/**
 * CascadeTimers and RunTimerList is same as timer.c in linux kernel 2.4.xx
 */
static inline void CascadeTimers(TimerVec * tv)
{
	/* cascade all the timers from tv up one level */
	ListHead *pstHead;
	ListHead *pstCurr;
	ListHead *pstNext;
	pstHead = tv->vec + tv->index;
	pstCurr = pstHead->pstNext;
	/*
	 * We are removing _all_ timers from the list, so we don't have to
	 * detach them individually, just clear the list afterwards.
	 */
	while (pstCurr != pstHead) {
		TimerNode * pstCurNode = LIST_ENTRY(pstCurr, TimerNode, list);
		pstNext = pstCurr->pstNext;
		//LIST_DEL(pstCurr);//no needed
		InternalAddTimer(pstCurNode);
		pstCurr = pstNext;
	}
	INIT_LIST_HEAD(pstHead);
	tv->index = (tv->index + 1) & TVN_MASK;
}

static inline void RunTimerList(unsigned uiCurTime)
{
	//static char sData[65536];
	while ((signed)(uiCurTime - uiCheckTime) >= 0) {
		ListHead *pstHead;
		ListHead *pstCurr;
		if (!tv1.index) {
			int n = 1;
			do {
				CascadeTimers(tvecs[n]);
			} while (tvecs[n]->index == 1 && ++n < NOOF_TVECS);
		}
repeat:
		pstHead = tv1.vec + tv1.index;
		pstCurr = pstHead->pstNext;
		if (pstCurr != pstHead) {
			TimerNode * pstCurNode = LIST_ENTRY(pstCurr, TimerNode, list);
			ExpireFunc OnExpire = pstCurNode->OnExpire;
			unsigned uiDataLen = pstCurNode->uiDataLen;
			char * sData;
			if(uiTimerType != NON_CONST_DATA_LEN_TIMER) {
				sData = pstCurNode->sData;
			} else {
				/*{
				  int i = 0;
				  TimerNode * apNodes[HASH_MAX_ROW];
				  int iNodeTotal = (int)(DIM(apNodes));
				  int iNodeCount = HashTableSearchN(&stTimerHash, (const void*)&stNode, uiSeq, &iNodeTotal, (void*[])apNodes);
				  if(0 > iNodeCount) {
				  uiDataLen = 0;
				  OnExpire = NULL;
				  } else {
				  if(sizeof(TimerNode) * (unsigned)iNodeCount + apNodes[0]->uiDataLen > stTimerHash.uNodeSize * (unsigned)iNodeCount) {
				  apNodes[0]->uiDataLen = (stTimerHash.uNodeSize - sizeof(TimerNode)) * (unsigned)iNodeCount;
				  }
				  for(i = 0; i + 1 < iNodeCount; i++) {
				  memcpy(sData + (stTimerHash.uNodeSize - sizeof(TimerNode)) * i, apNodes[i]->sData, stTimerHash.uNodeSize - sizeof(TimerNode));
				  }
				  memcpy(sData + (stTimerHash.uNodeSize - sizeof(TimerNode)) * i, apNodes[i]->sData, apNodes[0]->uiDataLen - (stTimerHash.uNodeSize - sizeof(TimerNode)) * i);

				 *puiDataLen = apNodes[0]->uiDataLen;
				 *psData = sData;
				 for(i = iNodeCount - 1; i > 0; i--) {
				 memset(apNodes[i], 0, sizeof(TimerNode));
				 }
				 }
				 }*/
				int iRet = -1;
				sData = NULL;
				iRet = GetClrTimerHashData(NULL, pstCurNode->uiSeq, &uiDataLen, &sData);
				if(0 > iRet) {
					uiDataLen = 0;
					OnExpire = NULL;
					printf("GetClrTimerHashData Error:%d", iRet);
				}
			}
			
			LIST_DEL(pstCurr);
			uiCount--;
			if(OnExpire)OnExpire(uiDataLen, sData);
			memset(pstCurNode, 0, sizeof(TimerNode));
			goto repeat;
		}
		++uiCheckTime;
		tv1.index = (tv1.index + 1) & TVR_MASK;
	}
}

void CheckTimer(void)
{
	unsigned uiCurTime;
	struct timeval stTV;
	if(0 >= uiCheckInvlUs) return;// there is no timer...
	//if uiCheckInvlUs large than 1000000, that is sec
	//need we use time call instead?
	gettimeofday(&stTV, NULL);
	uiCurTime = (unsigned)(USDIFF(stTV, stBaseTV) / uiCheckInvlUs);
	//just skip it
	/*if(uiCurTime < uiCheckTime) {
		//too fast?
		//cool down
		usleep(1);
		gettimeofday(&stTV, NULL);
		uiCurTime = (unsigned)USDIFF(stTV, stBaseTV) / uiCheckInvlUs;
	}*/
	if((signed)(uiCurTime - uiCheckTime) >= 0) {
		RunTimerList(uiCurTime);
	}
}
