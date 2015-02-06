#include "keylist_conf.h"
#include "tinyxml.h"
#include "log/logger.h"
#include "basic/trans.h"
#include "logic/wk_error.h"
#include "logic/wk_define.h"

CKeyListConf::CKeyListConf()
{
	bInit = false;
	InitRet = 0;
}


int CKeyListConf::Get(std::vector<KEYLISTNODE>& vctKeyList)
{
    if(_confFile.empty())
    {
        LOG_ERROR("conf file is empty!");
        return -1;
    }
	if (bInit == false)
	{
		InitRet = Init();
		bInit = true;
	}
	if (InitRet != 0)
		return InitRet;

    vctKeyList= _vctKeyList;
	return 0;
}

int CKeyListConf::GetQueenKeyName(std::string& queenKey)
{
    queenKey = _queenKey;
    return 0;
}

int CKeyListConf::LoadConf(const std::string& confFile)
{
    _confFile = confFile;
    return Init();
}

int CKeyListConf::Init()
{
    int ret = InitOne(_confFile);
    if (ret != 0)
    {
        LOG_ERROR("init conf error, ret:%d", ret);
        return ret;
    }

    return 0;
}
int CKeyListConf::InitOne(const std::string& strFilePath)
{
    _vctKeyList.clear();

	LOG_DEBUG("Init File %s",strFilePath.c_str());
	TiXmlDocument doc(strFilePath);
	bool loadOkay = doc.LoadFile();
	if (loadOkay == false)
		return ERR_WK_LOAD_CONF;
	TiXmlElement* root = doc.RootElement();


    TiXmlNode* queen= root->FirstChild("queen");
    CHECKNOTNULL(queen);
    TiXmlElement* queenelement = queen->ToElement();
    CHECKNOTNULL(queenelement);
    CHECKNOTNULL(queenelement->Attribute("key"));
    _queenKey = queenelement->Attribute("key");


    for(TiXmlNode* pair= root->FirstChild("pair");
        pair;
        pair = pair->NextSibling("pair"))
    {
        KEYLISTNODE stNode;
    
        TiXmlNode* cache= pair->FirstChild("cache");
        CHECKNOTNULL(cache);
        TiXmlElement* cacheelement = cache->ToElement();
        CHECKNOTNULL(cacheelement);
        CHECKNOTNULL(cacheelement->Attribute("keyprefix"));
        stNode.cacheKeyPrefix= cacheelement->Attribute("keyprefix");

        TiXmlNode* mysql= pair->FirstChild("mysql");
        CHECKNOTNULL(mysql);
        TiXmlElement* mysqlelement= mysql->ToElement();
        CHECKNOTNULL(mysqlelement);
        CHECKNOTNULL(mysqlelement->Attribute("colname"));
        stNode.mysqlColname = mysqlelement->Attribute("colname");

        _vctKeyList.push_back(stNode);
    }

    return 0;
}

