// Copyright (C) 2014 - isky Inc.
// Author: davidluan
// CreateTime: 2014-02-13

#ifndef MYSQL_CONF_H
#define MYSQL_CONF_H

#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <iostream>

#define INIT_MYSQLCONF(srcfile) CSingleton<CMysqlConf>::Instance()->SetFile(srcfile)
#define QUERY_MYSQLDEST(uid,stPoint) CSingleton<CMysqlConf>::Instance()->GetIpPort(uid,stPoint)

struct MYSQLNETPOINT
{
	std::string ip;
	int port;
    std::string dbname;    //db name
    std::string tblprefix; //table name,
    std::string user;      //user
    std::string pwd;       //password
};

//mod(100)后全部ip
typedef std::map<int, MYSQLNETPOINT> MYSQLCONFMAP;
class CMysqlConf
{
public:
	CMysqlConf();
	int GetIpPort(unsigned int uid, MYSQLNETPOINT& stPoint);
    void SetFile(const std::string & confFile);
    int LoadConf(const std::string& confFile);
private:
	MYSQLCONFMAP _mapUserMysql;//用户redis
	int Init();
	int InitOne(const std::string& strFilePath);
    //获取指定uid对应的map映射idx
    int GetKeyIndex(unsigned int uid);

	bool bInit;
	int InitRet;
    std::string _confFile;
};
#endif
