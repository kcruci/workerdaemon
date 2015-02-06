// Copyright (C) 2014 - isky Inc.
// Author: davidluan
// CreateTime: 2014-02-11

#ifndef _MYSQL_WRAPPER_H
#define _MYSQL_WRAPPER_H

#include <mysql.h>
#include <errmsg.h>

#include <string>

class ResultSet;

class CMysql {
public:

    CMysql(const char * host,
        int port ,
        const char * user,
        const char * pwd,
        int timeoutms);
	CMysql(){ }
    virtual ~CMysql();


    //建立链接
    bool Connect();

    //查询sql
    bool Query(const char * sql, size_t sql_len);

    //exec sql
    bool ExecSql(const char * sql, size_t sql_len);

    bool InitSqlString(std::string& sql, const char* data);

    bool AppendSqlString(std::string& sql, const char* data );

    // append escape blob data
    bool AppendSqlEscapeString(std::string& sql, const char* data, size_t data_len );


    void Close();

    // select 获取sql结果
    void FetchResult(ResultSet & result);

    // insert/update/delete
    bool AffectedRows(unsigned int& rows);

    //
    char * GetError() const;


private:
    /* data */
    MYSQL * _handle;

    std::string _host;
    int _port;
    std::string _user;
    std::string _pwd;
    int _timeout_ms;

    static char _error_msg[1024];

};


//查询结果
class ResultSet {
public:
    ResultSet()
        :_res(NULL),
        _curr_row(NULL),
        _fields_lengths(NULL)
    {} ;

    virtual ~ResultSet ()
    {
        mysql_free_result(_res);
        _res = NULL;
        _curr_row = NULL;
        _fields_lengths = NULL;
    };

    //下一行
    bool FetchRow()
    {
        _curr_row = mysql_fetch_row(_res);
        _fields_lengths = mysql_fetch_lengths(_res);
        if(NULL == _curr_row )
        {
            return false;
        }

        return true;
    }

	inline MYSQL_ROW GetOneRow()
    {
        return _curr_row;
    }

    //获取结果行数
    unsigned long GetRowsNum() const
    {
        return mysql_num_rows(_res);
    }
    unsigned int GetFieldsNum() const
    {
        return mysql_num_fields(_res) ;
    }

    //TODO 根据字段名获取列
    //mysql_fetch_fields

	unsigned long *FetchLengths()
    {
        return mysql_fetch_lengths(_res);
    }

    void SetRes(MYSQL_RES * res)
    {
        _res = res;
    }

private:
    MYSQL_RES * _res;

    //mysql 一行数据
    MYSQL_ROW _curr_row;
    //一行中没列数据大小
    unsigned long * _fields_lengths;
    /* data */
};

#endif // _MYSQL_WRAPPER_H
