// Copyright (C) 2014  .
// Author: davidluan
// CreateTime: 2014-02-17

#include "mysql_pool.h"
#include "log/logger.h"


using namespace std;

CMysqlPool::CMysqlPool()
{
    _iTimeout = 3;

}
CMysqlPool::~CMysqlPool ()
{
    map<string,CMysql*>::iterator itr = _pool.begin();
    for( ; itr != _pool.end(); itr++)
    {
        if (itr->second != NULL){
            delete(itr->second);
            itr->second = NULL;
        }
    }
}

CMysql* CMysqlPool::GetMysql(const std::string& user, const std::string& pwd, const std::string& sIp, int iPort)
{
    LOG_DEBUG("GetMysql ip:%s. port:%d",sIp.c_str(), iPort);

    string ip_port = genkey(sIp,iPort);
    map<string,CMysql*>::iterator itr = _pool.find(ip_port);
    if ( itr != _pool.end() )
    {
         return itr->second;
    }
    else
    {
        CMysql* newCon = new CMysql(sIp.c_str(), iPort, user.c_str(), pwd.c_str(), _iTimeout);
        if(newCon == NULL)
        {
            LOG_ERROR("new conn error, ip:%s. port:%d",sIp.c_str(), iPort);
            return NULL;
        }

        newCon->Connect();
        _pool[ip_port] = newCon;
        return newCon;
    }
}
void CMysqlPool::DeleteMysql(const std::string& sIp, int iPort)
{
    LOG_DEBUG("DeleteMysql ip:%s. port:%d",sIp.c_str(), iPort);
    string ip_port = genkey(sIp,iPort);
    map<string,CMysql*>::iterator itr = _pool.find(ip_port);
    if ( itr == _pool.end() )
        return;
    if (itr->second != NULL)
        delete itr->second;
    _pool.erase(itr);
}

std::string CMysqlPool::genkey(const std::string& sIp, int iPort)
{

    static char tmp[1024] = {0};
    snprintf(tmp,1024,"%s:%d",sIp.c_str(),iPort);
    string ip_port = tmp;
    return ip_port;
}
