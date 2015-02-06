// Copyright (C) 2014
// Author: davidluan
// CreateTime: 2014-02-11

#include "mysql_wrapper.h"
#include "log/logger.h"

#define MAX_ESCAPE_DATA 1024*1024


#define CHECH_HANDLE_NOTNULL(handle)        \
do{                                         \
    if(handle == NULL)                      \
    {                                       \
        LOG_ERROR("handle is null, connect again");        \
        bool reconnect = this->Connect();   \
        if(!reconnect)                      \
        {                                   \
            LOG_ERROR("reconnect error again");        \
            return false;                   \
        }                                   \
    }                                       \
}while(0);                                  \

char CMysql::_error_msg[1024] = {'\0'};

CMysql::CMysql(const char * host,
    int port ,
    const char * user,
    const char * pwd,
    int timeoutms)
:_handle(NULL)
,_host(host)
,_port(port)
,_user(user)
,_pwd(pwd)
    ,_timeout_ms(timeoutms)
{
}

CMysql::~CMysql()
{
    Close();
}


//建立链接
bool CMysql::Connect()
{
    if(NULL == _handle)
    {
        _handle= mysql_init(NULL);

        if(NULL == _handle)
        {
            LOG_ERROR("mysql init error, handle null\n");
            return false;
        }
    }

    mysql_options(_handle, MYSQL_OPT_CONNECT_TIMEOUT,  (char *)&_timeout_ms);

    if(!mysql_real_connect(_handle, _host.c_str(), _user.c_str(), _pwd.c_str(), NULL, _port, NULL, 0 ))
    {
        LOG_ERROR("Real Connect error, %s:%d, user:%s, pwd:%s\n, mysql errno :%d, msg:%s",
                                _host.c_str(), _port, _user.c_str(), _pwd.c_str(),
                                mysql_errno(_handle),  mysql_error(_handle));
        Close();
        return false;
    }

    return true;
}

//查询sql
bool CMysql::Query(const char * sql, size_t sql_len)
{
    //reconnect once;
    CHECH_HANDLE_NOTNULL(_handle);

    if(mysql_real_query(_handle, sql, sql_len))
    {
        if(mysql_errno(_handle) == CR_SERVER_LOST||
            mysql_errno(_handle) == CR_SERVER_GONE_ERROR)
        {

            //断开重连
            Close();
            if(Connect())
            {
                if(mysql_real_query(_handle, sql, sql_len))
                {
                    LOG_ERROR("query again error: %s\n", mysql_error(_handle) );
                }
                else
                {
                    return true;
                }
            }
            else
            {
                LOG_ERROR("connect again error: %s\n", mysql_error(_handle) );
            }
        }
        //error
        LOG_ERROR("%s\n",  mysql_error(_handle) );
        snprintf(_error_msg, sizeof(_error_msg),
            "mysql query error:%s", mysql_error(_handle));
        return false;
    }
    else
    {
        return true;
    }

}

bool CMysql::InitSqlString(std::string& sql, const char* data)
{
    sql = data;
    return true;
}


bool CMysql::AppendSqlString(std::string& sql, const char* data )
{
    sql.append(data);
    return true;
}

// append escape blob data
bool CMysql::AppendSqlEscapeString(std::string& sql, const char* data, size_t data_len )
{
    CHECH_HANDLE_NOTNULL(_handle);

    if(data_len > MAX_ESCAPE_DATA)
    {
        LOG_ERROR("overflow datasize:%u, max:%u\n",  data_len , MAX_ESCAPE_DATA);
        return false;
    }
    static char escape_data[MAX_ESCAPE_DATA];
    unsigned long ret = mysql_real_escape_string(_handle, escape_data, data, data_len);
    LOG_DEBUG("escape ret:%lld", ret);
    sql.append(escape_data, ret);
    return true;
}

bool CMysql::ExecSql(const char * sql, size_t sql_len)
{
    return Query(sql, sql_len);
}

void CMysql::Close()
{
    if(NULL != _handle)
    {
        mysql_close(_handle);
        _handle = NULL;
    }

}

// select 获取sql结果
void CMysql::FetchResult(ResultSet & result)
{
    MYSQL_RES* res = mysql_store_result(_handle);
    result.SetRes(res);
}

// insert/update/delete
bool CMysql::AffectedRows(unsigned int& rows)
{
    my_ulonglong num = mysql_affected_rows(_handle);
    if((my_ulonglong) -1 == num)
    {
        rows = 0;
        return false;
    }
    rows = num;
    return true;
}

//
char * CMysql::GetError() const
{
    return _error_msg;
}


