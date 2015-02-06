#ifndef __OI_TIMER_H__
#define __OI_TIMER_H__

#include <sys/types.h>
#define CONST_DATA_LEN_TIMER	1
#define NON_CONST_DATA_LEN_TIMER	2

#define SESSION_KEY 	0x90765

/*
 * get some stat data for operation supporting
 */
/*
 * get some stat data for operation supporting
 */
//获得当前timer个数，该值随添加、删除或时间到动态更新，为切片数据
unsigned GetCurTimerNode(void);
//获得运行过程中最大timer个数，用于合理设置hash阶和模。对于一般的异步网络服务，该值与请求量有关
unsigned GetMaxTimerNode(void);
//获得运行过程中最大的session数据大小，用于合理设置MaxUserData值。
unsigned GetMaxDataLen(void);
//hash的阶和模，以及MaxUserData的值，直接影响到timer所需的内存大小

/*
 * the expire callback function type
 */

/*
 * the expire callback function type
 */
typedef void(*ExpireFunc)(unsigned uiDataLen, char * sData);

/*
 * uMaxUserDataSize: max data size for timer
 * uRowNum: number of hash rows
 * auNodeNum: hash base
 * uiCheckIntervalUs: units of uiTimeOut, in usec
 *
 * iLogLevel:
 * 0	silent
 * 1	ERROR_LOG
 * 2	WARN_LOG
 * 3	DEBUG_LOG
 * 4	INFO_LOG
 * 5	TRACE_LOG
 *
 * -NOTE-: iLogLevel is ignored when pstLog is NULL, that is silent
 */
//初始化Timer函数。该函数会动态申请内存
//返回值
//0 -- 成功；
//小于0 -- 失败： 
//-10 内存不够
//-1 uRowNum和auNodeNum 参数非法（uRowNum不能为0）
//-2 uiCheckIntervalUs 参数非法（不能为0）

int InitTimer(unsigned uMaxUserDataSize, unsigned uRowNum, size_t auNodeNum[], 
		unsigned uiCheckIntervalUs, unsigned uiType);

/*
 * set a timer
 * it will expire after uiCheckIntervalUs * uiTimeOut usecs, if it's not be deleted
 * and it can be modifer the expire time
 *
 * on success, that is ,0 returned, a unique seq output by puiSeq if it's present
 */
//添加Timer函数。
//返回值
//0 -- 成功
//-1 -- Timer未初始化（没调用InitTimer）
//-2 -- Timer满。 -NOTE-: 需要上报一个异常ID
//-3 -- session数据太长：最好也上报一个异常ID
int AddTimer(unsigned *puiSeq, unsigned uiTimeOut, 
		ExpireFunc OnExpire, unsigned uiDataLen, char * sData);

/*
 * change the expire time
 */
//更改Timer超时时间函数
//返回值
//0 -- 成功
//-1 -- Timer未初始化（没调用InitTimer）
//-2 -- 未找到Timer（没加入或已经超时掉）
int ModTimer(unsigned uiSeq, unsigned uiTimeOut);

/*
 * get the timer data
 */
//从Timer获得session数据。-NOTE-：该函数不会删除Timer
//返回值
//0 -- 成功
//-1 -- Timer未初始化（没调用InitTimer）
//-2 -- 未找到Timer（没加入或已经超时掉）
int GetTimer(unsigned uiSeq, unsigned * puiDataLen, char ** psData);

/*
 * unset a timer
 */
//删除Timer
//返回值
//0 -- 成功
//-1 -- Timer未初始化（没调用InitTimer）
int DelTimer(unsigned uiSeq);

/*
 * check timer routine, just called in your main loop
 */
//Timer触发函数。在主循环中调用即可
void CheckTimer(void);
#endif/*__OI_TIMER_H__*/
