// Copyright (C) 2014  .
// Author: davidluan
// CreateTime: 2014-02-17

#ifndef _MYSQL_POOL_H
#define _MYSQL_POOL_H

#include "mysql_wrapper.h"
#include <string>
#include <map>

#define GET_MYSQLHANDLE(user,pwd,ip,port) CSingleton<CMysqlPool>::Instance()->GetMysql((user),(pwd),(ip),(port))
#define DEL_MYSQLHANDLE(ip,port) CSingleton<CMysqlPool>::Instance()->DeleteMysql((ip),(port))

class CMysqlPool {
public:
    CMysqlPool();
    virtual ~CMysqlPool ();

    CMysql* GetMysql(const std::string& user, const std::string& pwd, const std::string& sIp, int iPort);
    void DeleteMysql(const std::string& sIp, int iPort);
private:
    std::string genkey(const std::string& sIp, int iPort);
    //此对象为"ip:port--redis"的映射
    std::map<std::string,CMysql*> _pool;
    char    _errmsg[1024];
    int     _iTimeout;
};

#endif // _MYSQL_POOL_H
