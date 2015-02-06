// Copyright (C) 2014 - isky Inc.
// Author: davidluan
// CreateTime: 2014-02-13

#ifndef _REDIS_WRAPPER_H
#define _REDIS_WRAPPER_H

#include <string>
#include <map>
#include <vector>
#include <list>
#include <set>
#include "hiredis.h"
#include "log/logger.h"
#include "basic/trans.h"
#include "redis_conf.h"

#define MAXKEY 50
#define MAXVAL 200
#define LISTDELFLAG "This is must be delete"
#define REDIS_NO_DATA_VAL "-9999999"

enum REDIS_INFO{
    REDIS_NO_DATA  = 50000,
};
enum REDIC_ERR{
    NO_CONTEXT = -30000,
    REPLY_NULL = -30001,
    REPLY_ERROR = -30002,
    REPLY_TYPE_VALID = -30003,
    REPLY_STATUS_NOTOK = -30004,
    REPLY_ARRAY_NOTDOUBLE = -30005,
    REQUEST_PARA_ERROR = -30006,
    REDISCONF_ERROR = -30007, //读取redis的配置文件失败
    REDIS_VEC_ERROR = -30008, //批量读取时vec为0
};

#define SRETURN(reply, ret)     \
    do{                         \
        if (reply != NULL) {freeReplyObject(reply);reply = NULL;}\
        if (ret != 0) LOG_ERROR("safe ret :%d", ret);\
        return ret;              \
    }while(0);


#define CHECK_STATU_OK(_pReply)\
    do{                         \
        if ( _pReply->type != REDIS_REPLY_STATUS || strcmp(_pReply->str,"OK")!=0 ){\
            snprintf(_errmsg,1024,"replay  not success [%d] msg:[%s]",_pReply->type,_pReply->str); \
            LOG_ERROR(_errmsg);\
            if (_pReply != NULL) {freeReplyObject(_pReply);_pReply = NULL;}\
            return REPLY_STATUS_NOTOK;\
        }\
    }while(0);

#define CHECK_INT_OK(_pReply)\
    do{                         \
        if ( _pReply->type != REDIS_REPLY_INTEGER){\
            snprintf(_errmsg,1024,"replay  type valied[%d] msg:[%s]",_pReply->type,_pReply->str); \
            LOG_ERROR(_errmsg);\
            if (_pReply != NULL) {freeReplyObject(_pReply);_pReply = NULL;}\
            return REPLY_TYPE_VALID; \
        }\
    }while(0);
//获取redis上下文
#define CHECK_CONTEXT(context)\
    do{                         \
        if ( _DBContext == NULL||\
            _connected == false )\
        { \
            LOG_ERROR("No context Error.ret:");\
            return NO_CONTEXT; \
        }\
    }while(0);

//检查redis relpy
#define CHECK_REPLY(reply) \
    do{                         \
        if ( reply == NULL){ \
            snprintf(_errmsg,1024,"reply return NULL.msg:%s",_DBContext->errstr);\
            LOG_ERROR(_errmsg);\
            Close();\
            return REPLY_NULL; \
        } \
        if ( reply->type == REDIS_REPLY_ERROR ) {\
            snprintf(_errmsg,1024,"reply error.msg:%s",reply->str);\
            LOG_ERROR(_errmsg);\
            FREEREPLYOBJ(reply)\
            return REPLY_ERROR;\
        }\
    }while(0);
#define CHECK_NIL(reply)\
    do{                         \
        if ( reply->type == REDIS_REPLY_NIL ){ \
            FREEREPLYOBJ(reply)\
            return REDIS_NO_DATA;}\
    }while(0);
#define CHECK_STRING(reply)\
    do{                         \
        if ( reply->type != REDIS_REPLY_STRING ){\
            LOG_ERROR("need:%d, actual:%d", REDIS_REPLY_STRING, reply->type);\
            FREEREPLYOBJ(reply)\
            return REPLY_TYPE_VALID;}\
    }while(0);

#define CHECK_MULTI(reply)\
    do{                         \
        if ( reply->type != REDIS_REPLY_ARRAY ){\
            LOG_ERROR("need:%d, actual:%d", REDIS_REPLY_ARRAY, reply->type);\
            FREEREPLYOBJ(reply)\
            return REPLY_TYPE_VALID;} \
    }while(0);

#define CHECK_INT(reply)\
    do{                         \
        if (reply->type != REDIS_REPLY_INTEGER){ \
            LOG_ERROR("need:%d, actual:%d", REDIS_REPLY_INTEGER, reply->type);\
            FREEREPLYOBJ(reply)\
            return REPLY_TYPE_VALID;}  \
    }while(0);
#define FREEREPLYOBJ(reply)\
    do{                         \
        if (reply != NULL){     \
            freeReplyObject(reply);\
            reply = NULL;} \
    }while(0);

#define CHECK_ARRAY_DOUBLE(reply)\
    do{                          \
        if (reply->elements%2 != 0){\
            FREEREPLYOBJ(reply)     \
            return REPLY_ARRAY_NOTDOUBLE;}\
    }while(0);

/*!
 * \class CRedis
 * \brief 封装了一些常用的Redis C API
 */
class CRedis
{

public:

    /*!\brief 构造函数,初始化Redis的IP,port*/
    /*!
      \param[in] 无
      \Return: 无
      */
    CRedis();

    /*!\brief 构造函数,初始化DB的IP,用户名,密码*/
    /*!
      \param[in] sHost   DB的IP
      \param[in] iPort  端口
      \param[in] sPass   密码
      \Return: 无
      */

    CRedis(const std::string& sHost, int iPort, int iTimeout, const std::string& passwd = "");
    /*!\brief 析构函数，关闭连接,释放结果集 */
    /*!
      \param[in] 无
      \Return: 无
      */
    ~CRedis();

    int Connect();
    /*!\brief ping server*/
    /*!
      \Return: int
      0  --succ
      !0 --fail
      */
    int Auth();
    int Close();
    int Ping(std::string& sRes);
    int DEL(const std::string& skey);

    int EVAL(const std::string& key, const std::string& value, const std::string& cmdString, std::vector<std::string>& vctRsp);
    int EVAL(const std::vector<std::string>& vctKey, const std::vector<std::string>& vctValue, const std::string& cmdString, std::vector<std::string>& vctRsp);

    //list的操作
    int LIST_RPUSH(const std::string& skey,const std::string& sVal,int& iLen);//从队列右边入队
    int LIST_LPUSH(const std::string& skey,const std::string& sVal,int& iLen);//从队列左边入队
    int LIST_LPUSH(const std::string& skey,const void* Value, const int iLength,int& iLen);
    int LIST_DEL(const std::string& skey,int iIndex);//删除下标为Index的
    int LIST_DEL(const std::string& skey, const std::string& sVal);//删除值为sVal的
    int LIST_RANGE(const std::string& skey,int iStart,int iStop,std::vector<std::string>& vecBack);
    int LIST_LEN(const std::string& skey,int& iLen);
    int LIST_INDEX(const std::string& skey, int iIndex, std::string& reStr);
    int LIST_LTRIM(const std::string& skey, int iStart, int iStop);
    int LIST_SET(const std::string& skey,int iIndex,const std::string& sVal);


    //有序列表
    int ZUNIONSTORE(const std::string& from,const std::string& to);
    int ZINCRBY(const std::string& skey,const std::string& sValue,int iIncrVal);
    int ZCARD(const std::string& skey,int& iCount);
    int ZSCORE(const std::string& skey,const std::string& sValue,int& iScore);
    int ZADD(const std::string& skey,int iScore,const std::string& sValue);
    int ZADD(const std::string& skey,const std::vector<std::string>& vecVal,const std::vector<int>& vecScore);
    int ZCOUNT(const std::string& skey,int iMin, int iMax,int& iCount);//
    int ZRANGE(const std::string& skey,int iStart, int iStop,std::vector<std::string>& vecVal);//
    int ZRANGEWITHSCORE(const std::string& skey,int iStart,int iStop,std::vector<std::string>& vecVal, std::vector<std::string>& vecScore);
    int ZRANK(const std::string& skey,const std::string& sValue,int& iReIndex);
    int ZREVRANK(const std::string& skey,const std::string& sValue,int& iReIndex);
    int ZREM(const std::string& skey, const std::string& sValue);
    //type 为1时是反序，否则正序，就是从低到高
    int ZRANGEBYSCORE(const std::string& skey,int iLeftScore, int iRightScore,int iOffset, int iCount,  std::vector<std::string>& vecVal,bool reverse = false,bool allclose=false);
    int ZRANGEALL(const std::string& skey,int iOffset,int iCount,std::vector<std::string>& vecVal);
    int ZRANGEALLWITHSCORE(const std::string& skey,int iOffset, int iCount,std::vector<std::string>& vecVal,std::vector<std::string>& vecScore,bool mainseq = false);
    int ZREMRANGEBYSCORE(const std::string& skey,int min,int max);
    int ZREMRANGEBYRANK(const std::string& skey,int iStart,int iStop);
    int ZSET(const std::string& skey, int iscore,const std::string& sValue);//这里只限定score唯一的情况
    /*!\brief ping server*/
    /*!
      \Return: int
      0  --succ
      !0 --fail
      */
    int SAddKey(const std::string& skey,const std::string& value,int& iCount);
    int IsInSet(const std::string& skey,const std::string& value,bool& bIn);
    int SREM(const std::string& skey,const std::string& value,int& iDelNum);
    int SRANDMEMBER(const std::string& skey,std::string& value);

    //设置iValue，旧址由iLastVal返回，成功返回0，原来的值不存在则返回50000
    int GetSetKey(const std::string& skey,int iValue, int& iLastVal);
    int SetKeyNX(const std::string& skey, long long iValue, int& iRes);
    int SetKeyNX(const std::string& skey, long long iValue);

    int SetKey(const std::string& skey, const int iValue);

    int SetKey(const std::string& skey, const float fValue);

    int SetKey(const std::string& skey, const double dValue);

    int SetKey(const std::string& skey, long long lValue);

    int SetKey(const std::string& skey, const std::string sValue);

    int SetKey(const std::string& skey, const void* Value, const int iLength);

    int SetKey(const std::string& skey, const std::list<std::string> listValue);

    int IncKey(const std::string& skey, long long& iCurVal,int iValue = 0);

    int DecrKey(const std::string& skey, long long& iCurVal);

    int IncKeyBy(const std::string& skey, long long& iCurVal,int iValue = 1);

    int DecrKeyBy(const std::string& skey, long long& iCurVal, int iValue = 1);

    int GetKey(const std::string skey, int& iValue);

    int GetKey(const std::string skey, long long& lValue);

    int GetKey(const std::string skey, float& fValue);

    int GetKey(const std::string skey, double& dValue);

    int GetKey(const std::string skey, std::string& sValue);

    int GetKey(const std::string skey, char* chrValue);

    int GetKey(const std::string skey, std::list<std::string>& listValue);

    int DelKey(const std::string& skey);

    int MGetKey(const std::vector<std::string> vecKey, std::vector<std::string>& vecVal);

	int KEYS(const std::string &skey, std::vector<std::string> &vecVal);

    /*
       int Multi();

       int Watch(std::vector<std::string> vstKeys);

       int Watch(std::string sKey);
       int Exec();
       */
    inline std::string GenerateKey(const std::string& skey)
    {
        return skey;
    }
    //去掉字符串中的非字符
    std::string ConvertKey(const std::string& skey)
    {
        std::string strTmp;
        for ( int i = 0; i < (int)skey.size(); i++)
        {
            if (skey[i]>='0' && skey[i]<='9')
            {
                strTmp.append(1,skey[i]);
            }
        }
        return strTmp;
    }
    int GetIpPort(const std::string& skey,std::string& ip,int& port)
    {
        //QUERYCONF();
        return 0;
    }
    std::string GetError()
    {
        return std::string(_errmsg);
    }

    int GetOneIntInArray(long long& reInt,redisReply *reply)
    {
        CHECK_REPLY(reply);
        CHECK_NIL(reply);
        CHECK_INT(reply);
        reInt = reply->integer;
        return 0;
    }
    int GetOneStrInArray(std::string& reStr,redisReply *reply)
    {
    	//ԭ����CHECK_REPLY
        if ( reply == NULL)
		{ 
            snprintf(_errmsg,1024,"reply return NULL.msg:%s",_DBContext->errstr);
            LOG_ERROR(_errmsg);
            return REPLY_NULL; 
        }
        if ( reply->type == REDIS_REPLY_ERROR )
		{
            snprintf(_errmsg,1024,"reply error.msg:%s",reply->str);
            LOG_ERROR(_errmsg);
            return REPLY_ERROR;
        }

		//ԭ����CHECK_NIL
        if ( reply->type == REDIS_REPLY_NIL )
		{ 
            return REDIS_NO_DATA;
		}

		//CHECK_STRING
        if ( reply->type != REDIS_REPLY_STRING )
		{
            LOG_ERROR("need:%d, actual:%d", REDIS_REPLY_STRING, reply->type);
            return REPLY_TYPE_VALID;
		}

//�����ù��������⣬����ע�͵�
/*	
        CHECK_REPLY(reply);
        CHECK_NIL(reply);
        CHECK_STRING(reply);
*/        
        reStr.assign(reply->str,reply->len);
        return 0;
    }

	
private:
	void _freeReplyObject(void *reply);
    /*
       char** VecStr2Char(std::vector<std::string>& vecStr)
       {
       static char strlist[MAXKEY][MAXVAL] = {0};
       for(int i = 0; i < (int)vecStr.size(); i++)
       {
       vecStr[i].copy(strlist[i],vecStr[i].size());
       strlist[i][vecStr[i].size()] = '\0';
       }
       return strlist;
       }
       */
private:
    redisContext *_DBContext;

    redisReply *_pReply;

    char _errmsg[1024];

    bool _connected;

    int _iTimeout ;
    std::string m_strIp;
    int m_iPort;

    std::string m_sPasswd ;
};

#endif //_REDIS_WRAPPER_H

