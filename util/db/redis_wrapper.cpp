#include "redis_wrapper.h"
#include "basic/trans.h"
#include <stdlib.h>
#include <string.h>

CRedis::CRedis()
{
    _errmsg[0] = 0;
	_DBContext = NULL;
    _pReply = NULL;

    m_strIp = "127.0.0.1";
    m_iPort = 1688;
    m_sPasswd = "";
}

CRedis::CRedis(const std::string& sHost, int iPort, int iTimeout, const std::string& sPasswd)
{
    if(! sHost.empty() && iPort != 0)
    {
        m_strIp = sHost;
        m_iPort = iPort;
    }
    _iTimeout = iTimeout;
    m_sPasswd = sPasswd;
}

CRedis::~CRedis()
{
    Close();
    FREEREPLYOBJ(_pReply);
}


int CRedis::Connect()
{
    struct timeval timeout = { 1, 100*_iTimeout }; // 1.x seconds
    _DBContext = redisConnectWithTimeout(m_strIp.c_str(), m_iPort, timeout);
    if (_DBContext == NULL || _DBContext->err) {
        if (_DBContext) {
            snprintf(_errmsg , sizeof(_errmsg), "Connection error: %s\n",_DBContext->errstr);
            redisFree(_DBContext);
          } else {
            snprintf(_errmsg ,sizeof(_errmsg), "Connection error: can't allocate redis context\n");
        }
        _DBContext = NULL;
        return -1;
    }
    _connected = true;

    Auth();
    return 0;
}

int CRedis::Auth()
{
    //auth if needed
    if(!m_sPasswd.empty())
    {
        _pReply = (redisReply *)redisCommand(_DBContext,"AUTH %s", m_sPasswd.c_str());
        CHECK_REPLY(_pReply);
        FREEREPLYOBJ(_pReply);
    }
    return 0;
}

int CRedis::Close()
{
    if (_DBContext)
    {
        redisFree(_DBContext);
        LOG_DEBUG("close redis context");
        _DBContext = NULL;
    }

    _connected = false;
    return 0;
}

/*!\brief ping server*/
/*!
   \Return: int
    0  --succ
    !0 --fail
*/
int CRedis::Ping(std::string& sRes)
{
	/* PING server */

	char sMsg[255];
	_pReply = (redisReply *)redisCommand(_DBContext,"PING");
	sprintf(sMsg,"PING: %s\n", _pReply->str);
	sRes = std::string(sMsg);
	FREEREPLYOBJ(_pReply);
	return 0;
}

/*!\brief ping server*/
/*!
   \Return: int
    0  --succ
    !0 --fail
*/


int CRedis::EVAL(const std::string& skey, const std::string& value, const std::string& cmdString, std::vector<std::string>& vctRsp)
{
	CHECK_CONTEXT(_DBContext);
	_pReply = (redisReply *)redisCommand(_DBContext,"EVAL %s 1 %s %b",cmdString.c_str(), GenerateKey(skey).c_str(),value.data(),value.size());
	CHECK_REPLY(_pReply);
    CHECK_MULTI(_pReply);
    for(int i = 0; i < (int)_pReply->elements; i++)
    {
        std::string reStr;
        int ret = GetOneStrInArray(reStr,_pReply->element[i]);
        if (ret != 0)
            return ret;
        vctRsp.push_back(reStr);
    }

    FREEREPLYOBJ(_pReply);
	return 0;

}
int CRedis::EVAL(const std::vector<std::string>& vctKey, const std::vector<std::string>& vctValue, const std::string& cmdString, std::vector<std::string>& vctRsp)
{
	std::string keys;
	std::string values;

	size_t sz = vctKey.size();
	if( sz == 0 )
	{
		return -1;
	}
	size_t i=0;
	std::vector<std::string> realKeys;
	for(; i < sz; ++i )
	{
		realKeys.push_back(GenerateKey( vctKey[i] ));
		if (i == 0)
		{
			keys = GenerateKey( vctKey[i] );
		}
		else
		{
			keys += " " + GenerateKey( vctKey[i] );
		}
	}

	LOG_ERROR("keys is %s", keys.c_str());
	LOG_ERROR("keys size is %u", vctKey.size());
	sz = vctValue.size();
	i = 0;
	for(; i < sz; ++i )
	{
		if (i == 0)
		{
			values = vctValue[i];
		}
		else
		{
			values += " " + vctValue[i];
		}
	}

	std::string skey = GenerateKey( vctKey[0] );
	size_t keySize  = vctKey.size();
	size_t valueSize = vctValue.size();
    std::vector<const char*> argv( keySize + valueSize +3);
    std::vector<size_t> argvlen(keySize + valueSize +3);

    static char meval[] = "EVAL";
    int len = 0;
    argv[len] = meval;
    argvlen[len++] = sizeof(meval) - 1;
	argv[len] = cmdString.c_str();
	argvlen[len++] = cmdString.size();

	std::string keySizeStr = CTrans::ITOS( keySize );
	argv[len] = keySizeStr.c_str();
	argvlen[len++] = keySizeStr.size();
    for(i = 0; i < keySize; i++)
	{
		argv[len] = realKeys[i].c_str();
		argvlen[len++]= realKeys[i].size();
	}
    for(i = 0; i < valueSize; i++)
	{
		argv[len] = vctValue[i].c_str();
		argvlen[len++] = vctValue[i].size();
	}
    CHECK_CONTEXT(_DBContext);
    _pReply = (redisReply *)redisCommandArgv(_DBContext,argv.size(),&(argv[0]),&(argvlen[0]));

	//_pReply = (redisReply *)redisCommand(_DBContext,"EVAL %s %u %s %b",cmdString.c_str(), vctKey.size(), keys.c_str(),values.data(),values.size());
	//_pReply = (redisReply *)redisCommand(_DBContext,"EVAL %s %u %s %s %s %s %s %s %b",cmdString.c_str(), vctKey.size(), vctKey[0].c_str()
	//									, vctKey[1].c_str(), vctKey[2].c_str(), vctKey[3].c_str(), vctKey[4].c_str(), values.data(),values.size());
	CHECK_REPLY(_pReply);
    CHECK_MULTI(_pReply);
    for(int i = 0; i < (int)_pReply->elements; i++)
    {
        std::string reStr;
        int ret = GetOneStrInArray(reStr,_pReply->element[i]);
        if (ret != 0)
            return ret;
		LOG_DEBUG("key is %s", reStr.c_str());
        vctRsp.push_back(reStr);
    }
    FREEREPLYOBJ(_pReply);
	return 0;
}

int CRedis::LIST_RPUSH(const std::string& skey,const std::string& sVal,int& iLen)
{
    CHECK_CONTEXT(_DBContext);
    _pReply = (redisReply *)redisCommand(_DBContext,"RPUSH %s %b",GenerateKey(skey).c_str(),sVal.data(),sVal.size());
    CHECK_REPLY(_pReply);
    CHECK_INT(_pReply);
    iLen = (int)_pReply->integer;
    FREEREPLYOBJ(_pReply);
    return 0;
}

int CRedis::LIST_LPUSH(const std::string& skey,const std::string& sVal,int& iLen)
{
    CHECK_CONTEXT(_DBContext);
    _pReply = (redisReply *)redisCommand(_DBContext,"LPUSH %s %b",GenerateKey(skey).c_str(),sVal.data(),sVal.size());
    CHECK_REPLY(_pReply);
    CHECK_INT(_pReply);
    iLen = (int)_pReply->integer;
    FREEREPLYOBJ(_pReply);
    return 0;
}
int CRedis::LIST_LPUSH(const std::string& skey,const void* Value, const int iLength,int& iLen)
{
    CHECK_CONTEXT(_DBContext);
    _pReply = (redisReply *)redisCommand(_DBContext,"LPUSH %s %b",GenerateKey(skey).c_str(),Value,iLength);
    CHECK_REPLY(_pReply);
    CHECK_INT(_pReply);
    iLen = (int)_pReply->integer;
    FREEREPLYOBJ(_pReply);
    return 0;
}
int CRedis::LIST_INDEX(const std::string& skey,int iIndex, std::string& reStr)
{
    CHECK_CONTEXT(_DBContext);
    _pReply = (redisReply *)redisCommand(_DBContext,"LINDEX %s %d",GenerateKey(skey).c_str(),iIndex);
    CHECK_REPLY(_pReply);
    CHECK_NIL(_pReply);
    CHECK_STRING(_pReply);
    reStr.assign(_pReply->str,_pReply->len);
    FREEREPLYOBJ(_pReply);
    return 0;
}
int CRedis::LIST_LTRIM(const std::string& skey, int iStart, int iStop)
{
    CHECK_CONTEXT(_DBContext);
    _pReply = (redisReply *)redisCommand(_DBContext,"LTRIM %s %d %d",GenerateKey(skey).c_str(),iStart,iStop);
    CHECK_REPLY(_pReply);
    CHECK_STATU_OK(_pReply);
    FREEREPLYOBJ(_pReply);
    return 0;
}
int CRedis::LIST_SET(const std::string& skey,int iIndex,const std::string& sVal)
{
    CHECK_CONTEXT(_DBContext);
    _pReply = (redisReply *)redisCommand(_DBContext,"LSET %s %d %b",GenerateKey(skey).c_str(),iIndex,sVal.data(),sVal.size());
    CHECK_REPLY(_pReply);
    CHECK_STATU_OK(_pReply);
    FREEREPLYOBJ(_pReply);
    return 0;
}
int CRedis::LIST_DEL(const std::string& skey,int iIndex)
{
    CHECK_CONTEXT(_DBContext);

    _pReply = (redisReply *)redisCommand(_DBContext,"WATCH %s",GenerateKey(skey).c_str());
    CHECK_REPLY(_pReply);
    FREEREPLYOBJ(_pReply);

    _pReply = (redisReply *)redisCommand(_DBContext,"MULTI");
    CHECK_REPLY(_pReply);
    FREEREPLYOBJ(_pReply);

    //设置值
    _pReply = (redisReply *)redisCommand(_DBContext,"LSET %s %d %s",GenerateKey(skey).c_str(),iIndex,LISTDELFLAG);
    CHECK_REPLY(_pReply);
    //CHECK_STATU_OK(_pReply);
    FREEREPLYOBJ(_pReply);
    //删除值
    _pReply = (redisReply *)redisCommand(_DBContext,"LREM %s 1 %s",GenerateKey(skey).c_str(),LISTDELFLAG);
    CHECK_REPLY(_pReply);
    //CHECK_INT(_pReply);
    FREEREPLYOBJ(_pReply);
    //提交之
    _pReply = (redisReply *)redisCommand(_DBContext,"EXEC");
    CHECK_REPLY(_pReply);
    CHECK_MULTI(_pReply);
    for(int i = 0; i < (int)_pReply->elements; i++)
    {
        CHECK_REPLY(_pReply->element[i]);
        if(i == 0)
            CHECK_STATU_OK(_pReply->element[i]);
        if (i == 1)
            CHECK_INT(_pReply->element[i]);
    }
    FREEREPLYOBJ(_pReply);
    return 0;
}
int CRedis::LIST_DEL(const std::string& skey,const std::string& sVal)
{
    CHECK_CONTEXT(_DBContext);
    _pReply = (redisReply *)redisCommand(_DBContext,"LREM %s 0 %b",GenerateKey(skey).c_str(),sVal.data(),sVal.size());
    CHECK_REPLY(_pReply);
    CHECK_INT(_pReply);
    FREEREPLYOBJ(_pReply);
    return 0;
}
int CRedis::LIST_RANGE(const std::string& skey,int iStart, int iStop,std::vector<std::string>& vecBack)
{
    vecBack.clear();
    CHECK_CONTEXT(_DBContext);
    _pReply = (redisReply *)redisCommand(_DBContext, "LRANGE %s %d %d", GenerateKey(skey).c_str(),iStart,iStop);
    CHECK_REPLY(_pReply);
    CHECK_NIL(_pReply);
    CHECK_MULTI(_pReply);
    for(int i = 0; i < (int)_pReply->elements; i++)
    {
        std::string reStr;
        int ret = GetOneStrInArray(reStr,_pReply->element[i]);
        if (ret != 0)
            return ret;
        vecBack.push_back(reStr);
    }
    FREEREPLYOBJ(_pReply);
    return 0;
}
int CRedis::LIST_LEN(const std::string& skey,int &iLen)
{
    CHECK_CONTEXT(_DBContext);
    _pReply = (redisReply *)redisCommand(_DBContext,"LLEN %s",skey.c_str());
    CHECK_REPLY(_pReply);
    CHECK_NIL(_pReply);
    CHECK_INT(_pReply);
    iLen = (int)_pReply->integer;
    FREEREPLYOBJ(_pReply);
    return 0;
}
int CRedis::ZSCORE(const std::string& skey,const std::string& sValue,int& iScore)
{
	CHECK_CONTEXT(_DBContext);
	_pReply = (redisReply *)redisCommand(_DBContext,"ZSCORE %s %s",GenerateKey(skey).c_str(),sValue.c_str());
	CHECK_REPLY(_pReply);
	CHECK_NIL(_pReply);
	CHECK_STRING(_pReply);
	std::string strTmp;
	strTmp.assign(_pReply->str,_pReply->len);
	iScore = CTrans::STOI(strTmp);
    FREEREPLYOBJ(_pReply);
	return 0;
}
int CRedis::ZADD(const std::string& skey, int iScore, const std::string& sValue)
{
    CHECK_CONTEXT(_DBContext);
    _pReply = (redisReply *)redisCommand(_DBContext,"ZADD %s %d %b", GenerateKey(skey).c_str(), iScore, sValue.data(),sValue.size());
    CHECK_REPLY(_pReply);
    FREEREPLYOBJ(_pReply);
    return 0;
}
int CRedis::ZADD(const std::string& skey,const std::vector<std::string>& vecVal,const std::vector<int>& vecScore)
{
	//检查参数
	if (vecVal.size() != vecScore.size())
		return REQUEST_PARA_ERROR;
	if (vecVal.size() == 0)
		return REQUEST_PARA_ERROR;
	CHECK_CONTEXT(_DBContext);
    std::vector<std::string> genKey;
	std::vector<std::string> genScore;
    for(int i = 0; i < (int)vecVal.size(); i++)
        genKey.push_back(vecVal[i]);
	for(int i = 0; i < (int)vecScore.size(); i++)
		genScore.push_back(CTrans::ITOS(vecScore[i]));

	std::string sNewKey = GenerateKey(skey);
    std::vector<const char*> argv(vecVal.size()*2+2);
    std::vector<size_t> argvlen(vecVal.size()*2+2);

    static char mgetcmd[] = "ZADD";
    int len = 0;
    argv[len] = mgetcmd;
    argvlen[len] = sizeof(mgetcmd) - 1;
    len++;
	argv[len] = sNewKey.c_str();
	argvlen[len++] = sNewKey.size();
    for(int i = 0; i < (int)genKey.size(); i++)
	{
		argv[len] = genScore[i].c_str(),argvlen[len++]=genScore[i].size();
        argv[len] = genKey[i].c_str(),argvlen[len++]=genKey[i].size();
	}
    CHECK_CONTEXT(_DBContext);
    _pReply = (redisReply *)redisCommandArgv(_DBContext,argv.size(),&(argv[0]),&(argvlen[0]));
    CHECK_REPLY(_pReply);
	CHECK_INT(_pReply);
	FREEREPLYOBJ(_pReply);
	return 0;
}
int CRedis::ZINCRBY(const std::string& skey,const std::string& sValue,int iIncrVal)
{
    CHECK_CONTEXT(_DBContext);
    _pReply = (redisReply *)redisCommand(_DBContext,"ZINCRBY %s %d %s",GenerateKey(skey).c_str(),iIncrVal,sValue.c_str());
    CHECK_REPLY(_pReply);
    FREEREPLYOBJ(_pReply);
    return 0;
}
int CRedis::ZUNIONSTORE(const std::string& from,const std::string& to)
{
	std::string skey = from;
    CHECK_CONTEXT(_DBContext);
    _pReply = (redisReply *)redisCommand(_DBContext,"ZUNIONSTORE %s 1 %s",GenerateKey(to).c_str(),GenerateKey(from).c_str());
    CHECK_REPLY(_pReply);
    CHECK_INT(_pReply);
    FREEREPLYOBJ(_pReply);
    return 0;
}
int CRedis::DEL(const std::string& skey)
{
    CHECK_CONTEXT(_DBContext);
    _pReply = (redisReply *)redisCommand(_DBContext,"DEL %s",GenerateKey(skey).c_str());
    CHECK_REPLY(_pReply);
    CHECK_INT(_pReply);
    FREEREPLYOBJ(_pReply);
    return 0;
}
int CRedis::ZCARD(const std::string& skey,int& iCount)
{
    CHECK_CONTEXT(_DBContext);
    _pReply = (redisReply *)redisCommand(_DBContext,"ZCARD %s", GenerateKey(skey).c_str());
    CHECK_REPLY(_pReply);
    CHECK_NIL(_pReply);
    CHECK_INT(_pReply);
    iCount = (int)_pReply->integer;
    FREEREPLYOBJ(_pReply);
    return 0;
}
int CRedis::ZCOUNT(const std::string& skey,int iMin, int iMax,int& iCount)
{
    CHECK_CONTEXT(_DBContext);
    _pReply = (redisReply *)redisCommand(_DBContext,"ZCOUNT %s %d %d", GenerateKey(skey).c_str(), iMin, iMax);
    CHECK_REPLY(_pReply);
    CHECK_NIL(_pReply);
    CHECK_INT(_pReply);
    iCount = (int)_pReply->integer;
    FREEREPLYOBJ(_pReply);
    return 0;
}

int CRedis::ZRANGE(const std::string& skey,int iStart,int iStop,std::vector<std::string>& vecVal)
{
    vecVal.clear();
    CHECK_CONTEXT(_DBContext);
    _pReply = (redisReply *)redisCommand(_DBContext,"ZRANGE %s %d %d", GenerateKey(skey).c_str(), iStart, iStop);
    CHECK_REPLY(_pReply);
    CHECK_NIL(_pReply);
    CHECK_MULTI(_pReply);
    for(int i = 0; i < ((int)(_pReply->elements));i++)
    {
        std::string reStr;
        int ret = GetOneStrInArray(reStr,_pReply->element[i]);
        if (ret != 0)
            SRETURN(_pReply, ret);
        vecVal.push_back(reStr);
    }
    FREEREPLYOBJ(_pReply);
    return 0;
}

int CRedis::ZRANGEWITHSCORE(const std::string& skey,int iStart,int iStop,std::vector<std::string>& vecVal, std::vector<std::string>& vecScore)
{
    vecVal.clear();
    vecScore.clear();
    CHECK_CONTEXT(_DBContext);
    _pReply = (redisReply *)redisCommand(_DBContext,"ZRANGE %s %d %d WITHSCORES", GenerateKey(skey).c_str(), iStart, iStop);
    CHECK_REPLY(_pReply);
    CHECK_NIL(_pReply);
    CHECK_MULTI(_pReply);

    CHECK_ARRAY_DOUBLE(_pReply)
    for(int i = 0; i < ((int)(_pReply->elements))/2;i++)
    {
        std::string reStr;
        int ret = GetOneStrInArray(reStr,_pReply->element[i*2]);
        if (ret != 0){
             SRETURN(_pReply, ret);
        }
        vecVal.push_back(reStr);

        ret = GetOneStrInArray(reStr,_pReply->element[i*2+1]);
        if (ret != 0){
            SRETURN(_pReply, ret);
        }
        vecScore.push_back(reStr);
    }

    if(vecScore.size() != vecVal.size())
    {
        LOG_ERROR("not match socre value");
        SRETURN(_pReply, -11);
    }

    FREEREPLYOBJ(_pReply);
    return 0;
}

int CRedis::ZSET(const std::string& skey,int iscore,const std::string& sValue)
{
    CHECK_CONTEXT(_DBContext);

    _pReply = (redisReply *)redisCommand(_DBContext,"WATCH %s",GenerateKey(skey).c_str());
    CHECK_REPLY(_pReply);
    FREEREPLYOBJ(_pReply);

    _pReply = (redisReply *)redisCommand(_DBContext,"MULTI");
    CHECK_REPLY(_pReply);
    FREEREPLYOBJ(_pReply);

    //删数据
    _pReply = (redisReply *)redisCommand(_DBContext,"ZREMRANGEBYSCORE %s %d %d", GenerateKey(skey).c_str(),iscore,iscore);
    CHECK_REPLY(_pReply);
    FREEREPLYOBJ(_pReply);
    //加上数据
    _pReply = (redisReply *)redisCommand(_DBContext,"ZADD %s %d %b", GenerateKey(skey).c_str(), iscore, sValue.data(),sValue.size());
    CHECK_REPLY(_pReply);
    FREEREPLYOBJ(_pReply);

    _pReply = (redisReply *)redisCommand(_DBContext,"EXEC");
    CHECK_REPLY(_pReply);
    CHECK_MULTI(_pReply);
    for(int i = 0; i < (int)_pReply->elements; i++)
    {
        CHECK_REPLY(_pReply->element[i]);
        if(i == 0)
            CHECK_INT(_pReply->element[i]);
        if (i == 1)
            CHECK_INT(_pReply->element[i]);
    }

    FREEREPLYOBJ(_pReply);
    return 0;

}
int CRedis::ZREMRANGEBYSCORE(const std::string& skey,int min, int max)
{
    CHECK_CONTEXT(_DBContext);
    _pReply = (redisReply *)redisCommand(_DBContext,"ZREMRANGEBYSCORE %s %d %d", GenerateKey(skey).c_str(),min,max);
    CHECK_REPLY(_pReply);
    CHECK_INT(_pReply);
    FREEREPLYOBJ(_pReply);
    return 0;
}
int CRedis::ZREMRANGEBYRANK(const std::string& skey,int iStart,int iStop)
{
    CHECK_CONTEXT(_DBContext);
    _pReply = (redisReply *)redisCommand(_DBContext,"ZREMRANGEBYRANK %s %d %d", GenerateKey(skey).c_str(),iStart,iStop);
    CHECK_REPLY(_pReply);
    CHECK_INT(_pReply);
    FREEREPLYOBJ(_pReply);
    return 0;

}
int CRedis::ZRANGEALLWITHSCORE(const std::string& skey,int iOffset, int iCount,std::vector<std::string>& vecVal,std::vector<std::string>& vecScore,bool mainseq/* = false*/)
{
    vecVal.clear();
    vecScore.clear();
    CHECK_CONTEXT(_DBContext);
	if (mainseq == false)
		_pReply = (redisReply *)redisCommand(_DBContext,"ZREVRANGEBYSCORE %s +inf -inf WITHSCORES LIMIT %d %d", GenerateKey(skey).c_str(),iOffset,iCount);
	else
		_pReply = (redisReply *)redisCommand(_DBContext,"ZRANGEBYSCORE %s -inf +inf WITHSCORES LIMIT %d %d", GenerateKey(skey).c_str(),iOffset,iCount);
    CHECK_REPLY(_pReply);
    CHECK_NIL(_pReply);
    CHECK_MULTI(_pReply);
    CHECK_ARRAY_DOUBLE(_pReply)
    for(int i = 0; i < ((int)(_pReply->elements))/2;i++)
    {
        std::string reStr;
        int ret = GetOneStrInArray(reStr,_pReply->element[i*2]);
        if (ret != 0){
            return ret;
        }
        vecVal.push_back(reStr);
        ret = GetOneStrInArray(reStr,_pReply->element[i*2+1]);
        if (ret != 0){
            return ret;
        }
        vecScore.push_back(reStr);
    }
    FREEREPLYOBJ(_pReply);
    return 0;

}
int CRedis::ZRANGEALL(const std::string& skey,int iOffset,int iCount,std::vector<std::string>& vecVal)
{
    std::vector<std::string> vecScore;
    return ZRANGEALLWITHSCORE(skey,iOffset,iCount,vecVal,vecScore);
}
int CRedis::ZRANGEBYSCORE(const std::string& skey,int iLeftScore, int iRightScore,int iOffset, int iCount, std::vector<std::string>& vecVal,bool reverse /*=false*/,bool allclose /*false*/)
{
    vecVal.clear();
    CHECK_CONTEXT(_DBContext);
    if (reverse)
    {
        //printf("cmd is: ZREVRANGEBYSCORE %s (%d %d WITHSCORES LIMIT %d %d\n",GenerateKey(skey).c_str(),iRightScore,iLeftScore,iOffset,iCount);
        if (allclose == false)
            _pReply = (redisReply *)redisCommand(_DBContext,"ZREVRANGEBYSCORE %s (%d %d WITHSCORES LIMIT %d %d", GenerateKey(skey).c_str(),iRightScore,iLeftScore,iOffset,iCount);
        else
            _pReply = (redisReply *)redisCommand(_DBContext,"ZREVRANGEBYSCORE %s %d %d WITHSCORES LIMIT %d %d", GenerateKey(skey).c_str(),iRightScore,iLeftScore,iOffset,iCount);

    }
    else
    {
        //printf("cmd is: ZRANGEBYSCORE %s %d (%d WITHSCORES LIMIT %d %d\n",GenerateKey(skey).c_str(),iLeftScore,iRightScore,iOffset,iCount);
        if (allclose == false)
            _pReply = (redisReply *)redisCommand(_DBContext,"ZRANGEBYSCORE %s %d (%d WITHSCORES LIMIT %d %d", GenerateKey(skey).c_str(),iLeftScore,iRightScore,iOffset,iCount);
        else
            _pReply = (redisReply *)redisCommand(_DBContext,"ZRANGEBYSCORE %s %d %d WITHSCORES LIMIT %d %d", GenerateKey(skey).c_str(),iLeftScore,iRightScore,iOffset,iCount);
    }
    CHECK_REPLY(_pReply);
    CHECK_NIL(_pReply);
    CHECK_MULTI(_pReply);
    CHECK_ARRAY_DOUBLE(_pReply)
    for(int i = 0; i < ((int)(_pReply->elements))/2;i++)
    {
        std::string reStr;
        int ret = GetOneStrInArray(reStr,_pReply->element[i*2]);
        if (ret != 0){
            return ret;
        }
        vecVal.push_back(reStr);
    }
    FREEREPLYOBJ(_pReply);
    return 0;
}

int CRedis::ZRANK(const std::string& skey,const std::string& sValue, int& iReIndex)
{
    CHECK_CONTEXT(_DBContext);
    _pReply = (redisReply *)redisCommand(_DBContext,"ZRANK %s %s", GenerateKey(skey).c_str(), sValue.c_str());
    CHECK_REPLY(_pReply);
    CHECK_NIL(_pReply);
    CHECK_INT(_pReply);
    iReIndex = _pReply->integer;
    FREEREPLYOBJ(_pReply);
    return 0;
}
int CRedis::ZREVRANK(const std::string& skey,const std::string& sValue, int& iReIndex)
{
    CHECK_CONTEXT(_DBContext);
    _pReply = (redisReply *)redisCommand(_DBContext,"ZREVRANK %s %s", GenerateKey(skey).c_str(), sValue.c_str());
    CHECK_REPLY(_pReply);
    CHECK_NIL(_pReply);
    CHECK_INT(_pReply);
    iReIndex = _pReply->integer;
    FREEREPLYOBJ(_pReply);
    return 0;
}
int CRedis::ZREM(const std::string& skey,const std::string& sValue)
{
    CHECK_CONTEXT(_DBContext);
    _pReply = (redisReply *)redisCommand(_DBContext,"ZREM %s %s",GenerateKey(skey).c_str(),sValue.c_str());
    CHECK_REPLY(_pReply);
    CHECK_INT(_pReply);
    FREEREPLYOBJ(_pReply);
    return 0;
}
int CRedis::SAddKey(const std::string& skey, const std::string& value,int& iCount)
{
    CHECK_CONTEXT(_DBContext);
    _pReply = (redisReply *)redisCommand(_DBContext,"SADD %s %b", GenerateKey(skey).c_str(), value.data(),value.size());
    CHECK_REPLY(_pReply);
    CHECK_INT_OK(_pReply);
	iCount = (int)_pReply->integer;
    FREEREPLYOBJ(_pReply);
	return 0;
}

int CRedis::IsInSet(const std::string &skey,const std::string& value,bool& bIn)
{
    CHECK_CONTEXT(_DBContext);
    _pReply = (redisReply *)redisCommand(_DBContext,"SISMEMBER %s %s", GenerateKey(skey).c_str(), value.c_str());
    CHECK_REPLY(_pReply);
    CHECK_INT_OK(_pReply);
	bIn = false;
	if (_pReply->integer == 1)
		bIn = true;
    FREEREPLYOBJ(_pReply);
	return 0;
}
int CRedis::SREM(const std::string& skey,const std::string& value,int& iDelNum)
{
	CHECK_CONTEXT(_DBContext);
	_pReply = (redisReply *)redisCommand(_DBContext,"SREM %s %s",GenerateKey(skey).c_str(),value.c_str());
	CHECK_REPLY(_pReply);
	CHECK_INT_OK(_pReply);
	iDelNum = (int)_pReply->integer;
    FREEREPLYOBJ(_pReply);
	return 0;
}
int CRedis::SRANDMEMBER(const std::string& skey,std::string& value)
{
	CHECK_CONTEXT(_DBContext);
	_pReply = (redisReply *)redisCommand(_DBContext,"SRANDMEMBER %s",GenerateKey(skey).c_str());
	CHECK_REPLY(_pReply);
    CHECK_NIL(_pReply);
	CHECK_STRING(_pReply);
	value = _pReply->str;
    FREEREPLYOBJ(_pReply);
	return 0;
}

int CRedis::GetSetKey(const std::string& skey, int iValue, int& iLastVal)
{
    CHECK_CONTEXT(_DBContext);
    _pReply = (redisReply *)redisCommand(_DBContext,"GETSET %s %d",GenerateKey(skey).c_str(),iValue);
    CHECK_REPLY(_pReply);
    CHECK_STRING(_pReply);
    iLastVal = CTrans::STOI(_pReply->str);
    FREEREPLYOBJ(_pReply);
    return 0;
}
int CRedis::SetKeyNX(const std::string& skey,long long iValue, int& iRes)
{
    CHECK_CONTEXT(_DBContext);
	std::string enckey = GenerateKey(skey);
    _pReply = (redisReply *)redisCommand(_DBContext,"SETNX %b %lld", enckey.data(),enckey.size(), iValue);
    CHECK_REPLY(_pReply);
    CHECK_INT(_pReply);
    iRes = (int)_pReply->integer;
    FREEREPLYOBJ(_pReply);
    return 0;
}
int CRedis::SetKeyNX(const std::string& skey, long long iValue)
{
    CHECK_CONTEXT(_DBContext);
    _pReply = (redisReply *)redisCommand(_DBContext,"SETNX %s %lld", GenerateKey(skey).c_str(), iValue);
    CHECK_REPLY(_pReply);
    CHECK_INT(_pReply);
    int ret = (int)_pReply->integer;
    if (ret == 1)
    {
        //靠靠靠0
        return 0;
    }
    FREEREPLYOBJ(_pReply);
    return 1;
}
int CRedis::SetKey(const std::string& skey, const int iValue)
{
    CHECK_CONTEXT(_DBContext);
	 /* Set a key */
    _pReply = (redisReply *)redisCommand(_DBContext,"SET %s %d", GenerateKey(skey).c_str(), iValue);
    CHECK_REPLY(_pReply);
    CHECK_NIL(_pReply);
    CHECK_STATU_OK(_pReply);

    FREEREPLYOBJ(_pReply);
	return 0;
}
int CRedis::SetKey(const std::string& skey, long long lValue)
{
    CHECK_CONTEXT(_DBContext);
	 /* Set a key */
    _pReply = (redisReply *)redisCommand(_DBContext,"SET %s %lld", GenerateKey(skey).c_str(), lValue);
    CHECK_REPLY(_pReply);
    CHECK_NIL(_pReply);
    CHECK_STATU_OK(_pReply);

    FREEREPLYOBJ(_pReply);
	return 0;
}

int CRedis::SetKey(const std::string& skey, const float fValue)
{
    CHECK_CONTEXT(_DBContext);
    _pReply = (redisReply *)redisCommand(_DBContext,"WATCH %s", GenerateKey(skey).c_str());
    CHECK_REPLY(_pReply);
    FREEREPLYOBJ(_pReply);

    _pReply = (redisReply *)redisCommand(_DBContext,"MULTI");
    CHECK_REPLY(_pReply);
    FREEREPLYOBJ(_pReply);
	/* Set a key */
	_pReply = (redisReply *)redisCommand(_DBContext,"SET %s %f", GenerateKey(skey).c_str(), fValue);
    CHECK_REPLY(_pReply);
    CHECK_STATU_OK(_pReply);
    FREEREPLYOBJ(_pReply);
    _pReply = (redisReply *)redisCommand(_DBContext,"EXEC");
    CHECK_REPLY(_pReply);
    CHECK_NIL(_pReply);

    FREEREPLYOBJ(_pReply);
	return 0;
}

int CRedis::SetKey(const std::string& skey, const double dValue)
{
    CHECK_CONTEXT(_DBContext);
    _pReply = (redisReply *)redisCommand(_DBContext,"WATCH %s", GenerateKey(skey).c_str());
    CHECK_REPLY(_pReply);
    FREEREPLYOBJ(_pReply);

    _pReply = (redisReply *)redisCommand(_DBContext,"MULTI");
    CHECK_REPLY(_pReply);
    FREEREPLYOBJ(_pReply);
	/* Set a key */
	_pReply = (redisReply *)redisCommand(_DBContext,"SET %s %f", GenerateKey(skey).c_str(), dValue);
    CHECK_REPLY(_pReply);
    CHECK_STATU_OK(_pReply);
    FREEREPLYOBJ(_pReply);

    _pReply = (redisReply *)redisCommand(_DBContext,"EXEC");
    CHECK_REPLY(_pReply);
    CHECK_NIL(_pReply);
    FREEREPLYOBJ(_pReply);
    return 0;
}

int CRedis::SetKey(const std::string& skey, const std::string sValue)
{

    CHECK_CONTEXT(_DBContext);
     /* _pReply = (redisReply *)redisCommand(_DBContext,"WATCH %s", GenerateKey(skey).c_str());
    CHECK_REPLY(_pReply);
    FREEREPLYOBJ(_pReply);

    _pReply = (redisReply *)redisCommand(_DBContext,"MULTI");
    CHECK_REPLY(_pReply);
    FREEREPLYOBJ(_pReply);
    */
	/* Set a key */
	_pReply = (redisReply *)redisCommand(_DBContext,"SET %s %b", GenerateKey(skey).c_str(), sValue.data(),sValue.size());
    CHECK_STATU_OK(_pReply);
    FREEREPLYOBJ(_pReply);
    /*
    _pReply = (redisReply *)redisCommand(_DBContext,"EXEC");
    CHECK_REPLY(_pReply);
    CHECK_NIL(_pReply);
    FREEREPLYOBJ(_pReply);
    */
	return 0;
}

int CRedis::SetKey(const std::string& skey, const void* Value, const int iLength)
{
    CHECK_CONTEXT(_DBContext);
    /*
    _pReply = (redisReply *)redisCommand(_DBContext,"WATCH %s", GenerateKey(skey).c_str());
    CHECK_REPLY(_pReply);
    FREEREPLYOBJ(_pReply);

    _pReply = (redisReply *)redisCommand(_DBContext,"MULTI");
    CHECK_REPLY(_pReply);
    FREEREPLYOBJ(_pReply);

    */
    /* Set a key */
	_pReply = (redisReply *)redisCommand(_DBContext,"SET %b %b", GenerateKey(skey).c_str(),GenerateKey(skey).length(),Value, iLength);
    CHECK_STATU_OK(_pReply);
    FREEREPLYOBJ(_pReply);
    /*
    _pReply = (redisReply *)redisCommand(_DBContext,"EXEC");
    CHECK_REPLY(_pReply);
    CHECK_NIL(_pReply);
    FREEREPLYOBJ(_pReply);
    */
	return 0;
}

int CRedis::SetKey(const std::string& skey, const std::list<std::string> listValue)
{
    CHECK_CONTEXT(_DBContext);
    _pReply = (redisReply *)redisCommand(_DBContext,"WATCH %s", GenerateKey(skey).c_str());
    CHECK_REPLY(_pReply);
    FREEREPLYOBJ(_pReply);

    _pReply = (redisReply *)redisCommand(_DBContext,"MULTI");
    CHECK_REPLY(_pReply);
    FREEREPLYOBJ(_pReply);

    /* Set a key */
	for (std::list<std::string>::const_iterator it = listValue.begin();it != listValue.end();it++) {

		_pReply = (redisReply *)redisCommand(_DBContext,"LPUSH %s %s", GenerateKey(skey).c_str(),(std::string *)it->c_str());
        FREEREPLYOBJ(_pReply);
	}

    _pReply = (redisReply *)redisCommand(_DBContext,"EXEC");
    CHECK_REPLY(_pReply);
    CHECK_NIL(_pReply);
    FREEREPLYOBJ(_pReply);
	return 0;
}

int CRedis::IncKey(const std::string& skey,long long & iCurVal, int iValue/*=0*/)
{
    CHECK_CONTEXT(context);
	/* Set a key */
	_pReply = (redisReply *)redisCommand(_DBContext,"INCR %s",GenerateKey(skey).c_str());
    CHECK_REPLY(_pReply);
    if ( _pReply->type != REDIS_REPLY_INTEGER ){
        snprintf(_errmsg,1024,"_pReply->type valid.%d",_pReply->type);
        return -5;
    }
    iCurVal = _pReply->integer;
	FREEREPLYOBJ(_pReply);
	return 0;
}
int CRedis::DecrKey(const std::string& skey,long long& iCurVal)
{
    CHECK_CONTEXT(context);
    _pReply = (redisReply *) redisCommand(_DBContext,"DECR %s",GenerateKey(skey).c_str());
    CHECK_REPLY(_pReply);
    CHECK_INT(_pReply);
    iCurVal = _pReply->integer;
    FREEREPLYOBJ(_pReply);
    return 0;
}


int CRedis::IncKeyBy(const std::string& skey,long long & iCurVal, int iValue/*=1*/)
{
    CHECK_CONTEXT(context);
	/* Set a key */
	_pReply = (redisReply *)redisCommand(_DBContext,"INCRBY %s %d",GenerateKey(skey).c_str(), iValue);
    CHECK_REPLY(_pReply);
    if ( _pReply->type != REDIS_REPLY_INTEGER )
    {
        snprintf(_errmsg,1024,"_pReply->type valid.%d",_pReply->type);
        return -5;
    }
    iCurVal = _pReply->integer;
	FREEREPLYOBJ(_pReply);
	return 0;
}
int CRedis::DecrKeyBy(const std::string& skey,long long& iCurVal, int iValue)
{
    CHECK_CONTEXT(context);
    _pReply = (redisReply *) redisCommand(_DBContext,"DECRBY %s %d",GenerateKey(skey).c_str(), iValue);
    CHECK_REPLY(_pReply);
    CHECK_INT(_pReply);
    iCurVal = _pReply->integer;
    FREEREPLYOBJ(_pReply);
    return 0;
}

int CRedis::GetKey(const std::string skey, int& iValue)
{
    CHECK_CONTEXT(_DBContext)
	/* Set a key */
	_pReply  = (redisReply *)redisCommand(_DBContext,"GET %s",GenerateKey(skey).c_str());
    CHECK_REPLY(_pReply)
    CHECK_NIL(_pReply)
    CHECK_STRING(_pReply)
	iValue = CTrans::STOI(_pReply->str);
	FREEREPLYOBJ(_pReply);
	return 0;
}

int CRedis::GetKey(const std::string skey, long long& lValue)
{
    CHECK_CONTEXT(_DBContext)
	/* Set a key */
	_pReply  = (redisReply *)redisCommand(_DBContext,"GET %s",GenerateKey(skey).c_str());
    CHECK_REPLY(_pReply)
    CHECK_NIL(_pReply)
    CHECK_STRING(_pReply)
	lValue = (long)CTrans::STOUL(_pReply->str);
	FREEREPLYOBJ(_pReply);
	return 0;
}

int CRedis::GetKey(const std::string skey, float& fValue)
{
    CHECK_CONTEXT(_DBContext)
	/* Set a key */
	_pReply  = (redisReply *)redisCommand(_DBContext,"GET %s",GenerateKey(skey).c_str());
    CHECK_REPLY(_pReply)
    CHECK_NIL(_pReply)
	fValue = CTrans::STOF(std::string(_pReply->str));
	FREEREPLYOBJ(_pReply);
	return 0;
}

int CRedis::GetKey(const std::string skey, double& dValue)
{
    CHECK_CONTEXT(_DBContext);
	/* Set a key */
	_pReply  = (redisReply *)redisCommand(_DBContext,"GET %s",GenerateKey(skey).c_str());
    CHECK_REPLY(_pReply)
    CHECK_NIL(_pReply)
    CHECK_STRING(_pReply)
	dValue = CTrans::STOF(std::string(_pReply->str));
	FREEREPLYOBJ(_pReply);
	return 0;
}

int CRedis::GetKey(const std::string skey, std::string& sValue)
{
    LOG_DEBUG("come replay:");
    CHECK_CONTEXT(_DBContext);
	/* Set a key */
	_pReply  = (redisReply *)redisCommand(_DBContext,"GET %s",GenerateKey(skey).c_str());
    LOG_DEBUG("replay:");
    CHECK_REPLY(_pReply)
    CHECK_NIL(_pReply)
    CHECK_STRING(_pReply)
    LOG_DEBUG("str_len:%d", _pReply->len);
    sValue.assign(_pReply->str,_pReply->len);
	FREEREPLYOBJ(_pReply);
	return 0;
}

int CRedis::GetKey(const std::string skey, char* chrValue)
{
    LOG_DEBUG("111come replay:");
    CHECK_CONTEXT(_DBContext);
	/* Set a key */
	_pReply  = (redisReply *)redisCommand(_DBContext,"GET %s",GenerateKey(skey).c_str());
    CHECK_REPLY(_pReply)
    CHECK_NIL(_pReply)
    CHECK_STRING(_pReply)
	memcpy(chrValue,_pReply->str,_pReply->len);
	FREEREPLYOBJ(_pReply);
	return 0;

}

int CRedis::GetKey(const std::string skey, std::list<std::string>& listValue)
{
	if(_DBContext)
	{
		return -1;
	}
	unsigned int j;
	/* Let's check what we have inside the list */
	_pReply = (redisReply *)redisCommand(_DBContext,"LRANGE %s 0 -1",GenerateKey(skey).c_str());
	if (_pReply->type == REDIS_REPLY_ARRAY) {
		for (j = 0; j < _pReply->elements; j++) {
			listValue.push_back(std::string(_pReply->element[j]->str));
		}
	}
	FREEREPLYOBJ(_pReply);
	return 0;
}

int CRedis::DelKey(const std::string& skey)
{
    CHECK_CONTEXT(_DBContext);
    _pReply = (redisReply *)redisCommand(_DBContext,"DEL %s",GenerateKey(skey).c_str());
    CHECK_REPLY(_pReply)
    CHECK_INT(_pReply)
    FREEREPLYOBJ(_pReply);
    return 0;
}


void CRedis::_freeReplyObject(void *reply) {
    redisReply *r = (redisReply *)reply;
    size_t j;

    if (r == NULL)
        return;

    switch(r->type) {
    case REDIS_REPLY_INTEGER:
        break; /* Nothing to free */
    case REDIS_REPLY_ARRAY:
        if (r->element != NULL) {
            for (j = 0; j < r->elements; j++)
                if (r->element[j] != NULL)
                    _freeReplyObject(r->element[j]);
            free(r->element);
        }
        break;
    case REDIS_REPLY_ERROR:
    case REDIS_REPLY_STATUS:
    case REDIS_REPLY_STRING:
        if (r->str != NULL)
            free(r->str);
        break;
    }
    free(r);
}
int CRedis::MGetKey(const std::vector<std::string> vecKey, std::vector<std::string>& vecVal)
{
	if (vecKey.size() == 0)
		return REDIS_VEC_ERROR;
	
	int ret = 0;
	std::string skey = vecKey[0];
	std::string reStr;
    std::vector<std::string> genKey;
	genKey.clear();
	
    for(int i = 0; i < (int)vecKey.size(); i++)
        genKey.push_back(GenerateKey(vecKey[i]));

    std::vector<const char*> argv(vecKey.size()+1);
    std::vector<size_t> argvlen(vecKey.size()+1);

    static char mgetcmd[] = "MGET";
    int len = 0;
    argv[len] = mgetcmd;
    argvlen[len] = sizeof(mgetcmd) - 1;
    len++;
    for(int i = 0; i < (int)genKey.size(); i++)
        argv[len] = genKey[i].c_str(),argvlen[len++]=genKey[i].size();
    CHECK_CONTEXT(_DBContext);
    _pReply = (redisReply *)redisCommandArgv(_DBContext,argv.size(),&(argv[0]),&(argvlen[0]));
    CHECK_REPLY(_pReply);
    CHECK_MULTI(_pReply);
    for (int i = 0; i < (int)_pReply->elements; i++)
    {
        reStr.clear();
        ret = GetOneStrInArray(reStr,_pReply->element[i]);
        if (ret < 0){
			if (_pReply != NULL)
			{ 
				_freeReplyObject(_pReply);
			}
            return ret;
        }
        if (ret == REDIS_NO_DATA)
            reStr = REDIS_NO_DATA_VAL;
        vecVal.push_back(reStr);
    }

   // FREEREPLYOBJ(_pReply);
	if (_pReply != NULL){ 
		_freeReplyObject(_pReply);
	}
    return 0;
}

int CRedis::KEYS(const std::string &skey, std::vector<std::string> &vecVal)
{
	CHECK_CONTEXT(_DBContext);
	unsigned int j;
	/* Let's check what we have inside the list */
	_pReply = (redisReply *)redisCommand(_DBContext,"KEYS %s", skey.c_str());
	if (_pReply->type == REDIS_REPLY_ARRAY) {
		for (j = 0; j < _pReply->elements; j++) {
			vecVal.push_back(std::string(_pReply->element[j]->str));
		}
	}
	FREEREPLYOBJ(_pReply);
	return 0;
}

