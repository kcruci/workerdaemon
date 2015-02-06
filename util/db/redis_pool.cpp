#include "redis_pool.h"
#include "log/logger.h"
#include "basic/trans.h"
#include "macrodef.h"
#include <time.h>
using namespace std;

CRedisPool::CRedisPool()
{
    _iTimeout = 300;
}

CRedisPool::~CRedisPool()
{
   FOR_EACH(itredis, _redisPool)
    {
        if (itredis->second != NULL){
            delete(itredis->second);
            itredis->second = NULL;
        }
    }
}
CRedis* CRedisPool::GetRedis(const std::string& sIp, int iPort, const std::string& passwd)
{
	LOG_DEBUG("Get redis ip:%s. port:%d",sIp.c_str(), iPort);
    string ip_port = genkey(sIp,iPort);
    map<string,CRedis*>::iterator itr = _redisPool.find(ip_port);
    if ( itr != _redisPool.end() )
        return itr->second;

    //如果不存在，那么就新建一个
    LOG_DEBUG("new connect");
    //300ms超时
    CRedis * redis = new CRedis(sIp.c_str(), iPort, _iTimeout, passwd);
    if(redis == NULL)
    {
        LOG_ERROR("new redis error!");
        return NULL;
    }
    if( 0 !=  redis->Connect() )
	{
		LOG_ERROR("conn to redis svr failed ");
		return NULL;
	}
		LOG_DEBUG("conn redis-svr:%s:%d ",sIp.c_str(),iPort);
    _redisPool[ip_port] = redis;
    return redis;
}

void CRedisPool::DeleteRedis(const std::string& sIp, int iPort)
{
    LOG_DEBUG("del redis ip:%s. port:%d",sIp.c_str(), iPort);
    string ip_port = genkey(sIp,iPort);
    map<string,CRedis*>::iterator itr = _redisPool.find(ip_port);

    if ( itr == _redisPool.end() )
        return;
    if (itr->second != NULL)
        delete (itr->second);
    _redisPool.erase(itr);
}


std::string CRedisPool::genkey(const std::string& sIp, int iPort)
{
    static char tmp[1024] = {0};
    snprintf(tmp,1024,"%s:%d",sIp.c_str(),iPort);
    string ip_port = tmp;
    return ip_port;
}
