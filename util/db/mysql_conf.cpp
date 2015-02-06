#include "mysql_conf.h"
#include "tinyxml.h"
#include "log/logger.h"
#include "basic/trans.h"
#include "logic/wk_error.h"
#include "logic/wk_define.h"

CMysqlConf::CMysqlConf()
{
	bInit = false;
	InitRet = 0;
}


void CMysqlConf::SetFile(const std::string & confFile)
{
    _confFile= confFile;
}

int CMysqlConf::GetIpPort(unsigned int uid, MYSQLNETPOINT& stPoint)
{
	if (bInit == false)
	{
		InitRet = Init();
		bInit = true;
	}
	if (InitRet != 0)
    {
		LOG_ERROR("init ret uid:%lld, ret:%d", uid, InitRet);
		return InitRet;
    }


    //TODO
    int keyidx = GetKeyIndex(uid);

	//查找
	if (_mapUserMysql.find(keyidx) == _mapUserMysql.end())
	{
		LOG_ERROR("there is no uid:%lld, index:%d", uid, keyidx);
		return ERR_WK_NO_IPPORT;
	}
	stPoint = _mapUserMysql[keyidx];
	return 0;
}
//最低两位分库
int CMysqlConf::GetKeyIndex(unsigned int key)
{
    return (key)%100;
}

int CMysqlConf::LoadConf(const std::string& confFile)
{
    _confFile = confFile;
    return Init();
}

int CMysqlConf::Init()
{
    int ret = InitOne(_confFile);
    if (ret != 0)
    {
        return ret;
    }

    return 0;
}

int CMysqlConf::InitOne(const std::string& strFilePath)
{
    _mapUserMysql.clear();

	LOG_DEBUG("Init File %s",strFilePath.c_str());
	TiXmlDocument doc(strFilePath);
	bool loadOkay = doc.LoadFile();
	if (loadOkay == false)
		return ERR_WK_LOAD_CONF;
	TiXmlElement* root = doc.RootElement();
    TiXmlNode* useritem = root->FirstChild("user");
    CHECKNOTNULL(useritem);
    for(TiXmlNode* dbitem = useritem->FirstChild("db");
        dbitem;
        dbitem = dbitem->NextSibling("db"))
    {
        TiXmlElement* dbelement = dbitem->ToElement();
        CHECKNOTNULL(dbelement->Attribute("bid"));
        CHECKNOTNULL(dbelement->Attribute("eid"));
        CHECKNOTNULL(dbelement->Attribute("ip"));
        CHECKNOTNULL(dbelement->Attribute("port"));
        CHECKNOTNULL(dbelement->Attribute("dbname"));
        CHECKNOTNULL(dbelement->Attribute("tbl"));
        CHECKNOTNULL(dbelement->Attribute("user"));
        CHECKNOTNULL(dbelement->Attribute("password"));
        MYSQLNETPOINT net;
        net.ip = dbelement->Attribute("ip");
        net.port= CTrans::STOI(dbelement->Attribute("port"));
        net.dbname= dbelement->Attribute("dbname");
        net.tblprefix= dbelement->Attribute("tbl");
        net.user= dbelement->Attribute("user");
        net.pwd= dbelement->Attribute("password");
        int begin = CTrans::STOI(dbelement->Attribute("bid"));
        int end= CTrans::STOI(dbelement->Attribute("eid"));
        for(int i = begin; i<=end; i++)
        {
            _mapUserMysql[i] = net;
        }
	}

	return 0;
}
