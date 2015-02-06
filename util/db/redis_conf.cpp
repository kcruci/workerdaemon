#include "redis_conf.h"
#include "tinyxml.h"
#include "log/logger.h"
#include "basic/trans.h"
#include "logic/wk_error.h"
#include "logic/wk_define.h"

CRedisConf::CRedisConf()
{
	bInit = false;
	InitRet = 0;
}

int CRedisConf::GetAllUserIpPort(std::map<int, NETPOINT> & allPoint)
{
	if (bInit == false)
	{
		InitRet = Init();
		bInit = true;
	}
	if (InitRet != 0)
		return InitRet;

    allPoint = _mapUserRedis;
	return 0;
}


int CRedisConf::GetUserIpPort(unsigned long long uid, NETPOINT& stPoint)
{
	if (bInit == false)
	{
		InitRet = Init();
		bInit = true;
	}
	if (InitRet != 0)
		return InitRet;

    //TODO
    int keyidx = GetKeyIndex(uid);
	LOG_DEBUG("keyidx: %d ",keyidx);
	//查找
	if (_mapUserRedis.find(keyidx) == _mapUserRedis.end())
	{
		LOG_DEBUG("there is no uid:%lld, index:%d", uid, keyidx);
		return ERR_WK_NO_IPPORT;
	}
	stPoint = _mapUserRedis[keyidx];
	return 0;
}

int CRedisConf::GetKeyIndex(long long uid)
{
    //crc32
    //TODO
    return uid%100;
}

int CRedisConf::GetQueenAllIpPort(std::vector<NETPOINT>& allPoint)
{
	if (bInit == false)
	{
		InitRet = Init();
		bInit = true;
	}
	if (InitRet != 0)
		return InitRet;

    allPoint = _allModRedis;
	return 0;
}

int CRedisConf::LoadConf(const std::string& confFile)
{
    _confFile = confFile;
    return Init();
}

int CRedisConf::Init()
{

    int ret = InitOne(_confFile);
    if (ret != 0)
    {

        return ret;
    }

    return 0;
}
int CRedisConf::InitOne(const std::string& strFilePath)
{
    _mapUserRedis.clear();
    _allModRedis.clear();

	LOG_DEBUG("Init File %s",strFilePath.c_str());
	TiXmlDocument doc(strFilePath);
	bool loadOkay = doc.LoadFile();
	if (loadOkay == false)
		return ERR_WK_LOAD_CONF;
	TiXmlElement* root = doc.RootElement();
    TiXmlNode* useritem = root->FirstChild("user");
    CHECKNOTNULL(useritem);
    for(TiXmlNode* dbitem = useritem->FirstChild("redis");
        dbitem;
        dbitem = dbitem->NextSibling("redis"))
    {
        TiXmlElement* dbelement = dbitem->ToElement();
        CHECKNOTNULL(dbelement->Attribute("bid"));
        CHECKNOTNULL(dbelement->Attribute("eid"));
        CHECKNOTNULL(dbelement->Attribute("ip"));
        CHECKNOTNULL(dbelement->Attribute("port"));
        CHECKNOTNULL(dbelement->Attribute("password"));
        NETPOINT net;
        net.ip = dbelement->Attribute("ip");
     	//net.port= CTrans::STOI(dbelement->Attribute("port"));
		net.port= dbelement->Attribute("port");
        net.passwd= dbelement->Attribute("password");
        int begin = CTrans::STOI(dbelement->Attribute("bid"));
        int end= CTrans::STOI(dbelement->Attribute("eid"));
        for(int i = begin; i<=end; i++)
        {
            _mapUserRedis[i] = net;
        }
	}

    TiXmlNode* modbitmapitem = root->FirstChild("modbitmap");
    CHECKNOTNULL(modbitmapitem);
    for(TiXmlNode* dbitem = modbitmapitem->FirstChild("redis");
        dbitem;
        dbitem = dbitem->NextSibling("redis"))
    {
        TiXmlElement* dbelement = dbitem->ToElement();
        CHECKNOTNULL(dbelement->Attribute("ip"));
        CHECKNOTNULL(dbelement->Attribute("port"));
        CHECKNOTNULL(dbelement->Attribute("password"));

        NETPOINT net;
        net.ip = dbelement->Attribute("ip");
        net.port= dbelement->Attribute("port");
        net.passwd= dbelement->Attribute("password");
        //net.port= CTrans::STOI(dbelement->Attribute("port"));
        _allModRedis.push_back(net);
	}
	return 0;
}
