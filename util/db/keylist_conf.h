// Copyright (C) 2014  .
// Author: davidluan
// CreateTime: 2014-02-22

#ifndef _KEYLIST_CONF_H
#define _KEYLIST_CONF_H


#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <iostream>


typedef struct stKeyListNode
{
	std::string cacheKeyPrefix;
	std::string mysqlColname;
}KEYLISTNODE;

//mod(100)后全部ip
class CKeyListConf
{
    
public:
	CKeyListConf();

    //获取队列全部 ip port
    int Get(std::vector<KEYLISTNODE>& vctKeyList);

    int GetQueenKeyName(std::string& queenKey);

    //更新配置
    int LoadConf(const std::string& confFile);

private:

    std::vector<KEYLISTNODE> _vctKeyList;
    std::string _queenKey;

	int Init();
	int InitOne(const std::string& strFilePath);
    //获取指定uid对应的map映射idx

	bool bInit;
	int InitRet;

    std::string _confFile;
};


#endif // _KEYLIST_CONF_H
