// Copyright (C) 2014 - isky Inc.
// Author: davidluan
// CreateTime: 2014-02-13

#ifndef REDIS_POOL_H
#define REDIS_POOL_H

//redis的连接_对象池，长连接时用
//使用简单map实现，仅支持单线程，同步
//可以考虑使用双链表支持对象回收，支持多线程、异步
#include "hiredis.h"
#include "redis_wrapper.h"
#include <string>
#include <map>

#define GET_REDIS(ip,port) CSingleton<CRedisPool>::Instance()->GetRedis((ip),(port))
#define GET_REDIS_AUTH(ip,port,passwd) CSingleton<CRedisPool>::Instance()->GetRedis((ip),(port),(passwd))
#define DEL_REDIS(ip,port) CSingleton<CRedisPool>::Instance()->DeleteRedis((ip),(port))

class CRedisPool
{
public:
    CRedisPool();
    ~CRedisPool();

    //redis pool
    CRedis* GetRedis(const std::string& sIp, int iPort, const std::string& passwd="");
    void DeleteRedis(const std::string& sIp, int iPort);

private:
    std::string genkey(const std::string& sIp, int iPort);
    //此对象为"ip:port--redis"的映射
    std::map<std::string,CRedis*> _redisPool;
    char    _errmsg[1024];
    int     _iTimeout;
};
#endif
