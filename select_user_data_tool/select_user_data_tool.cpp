#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>
#include <time.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sys/types.h>

#include "db/mysql_conf.h"
#include "db/redis_conf.h"
#include "db/keylist_conf.h"
#include "tinyxml.h"

#include "db/redis_wrapper.h"
#include "db/mysql_wrapper.h"
#include "db/mysql_pool.h"
#include "db/redis_pool.h"
#include "logic/fnf_user.pb.h"
#include "logic/fnf_mail.pb.h"
#include "logic/fnf_friends.pb.h"
#include "logic/fnf_modbitmap.pb.h"

#include "basic/crc.h"
#include "basic/trans.h"
#include "logic/wk_error.h"
#include "logic/wk_define.h"
#include "macrodef.h"

#include <iostream>

using namespace std;

#define RET_LINE() {return -__LINE__;}
#define RET_TRUE_LINE(x) do{ if(x) return -__LINE__; }while(0)
#define TOOLCHECKNOTNULL(node) \
    if(node == NULL) \
{\
    return  ERR_WK_WRONG_CONF  ;\
}\

#define SELECT_CMD_BY_UID 1
#define SELECT_CMD_BY_OPENID 2
#define CLEAR_CMD_BY_UID 101
#define CLEAR_CMD_BY_OPENID 102
#define SELECT_GAME_COUNT_BY_OPENIDLIST 201
#define SELECT_MODBITMAP_QUEUE 301

typedef struct
{
	char cUserDataRedisConf[256];
	CRedisConf CRdsCnfObj;

	REDISCONFMAP uid_cache_conf_map;			//openid-uid
	REDISCONFMAP mail_conf_map;					//mailbox
	REDISCONFMAP userfriend_conf_map;			//userfriend
	REDISCONFMAP mod_bitmap_conf_map;			//mod_bitmap
	VECNETPORT stNetportVec;

	uint32_t dwCmdId;			//1--select by uid, 2--select by openid
	uint64_t ddwUin;
	std::string strOpenId;
	std::string strPlatInfo;
	std::string strUserDataCol;
	std::string strAtomicItem;

	std::map<std::string, uint32_t> stGameCountMap;
}select_user_data_tool_config;

select_user_data_tool_config g_stConfig, *pstConfig;

// check the conf path can be access and compose absolute dir...
static int IsVaildAndCompse(const char *psCnfPath, char *psFinalPath, int iFinPathLen)
{
    char tmpPath[256];

    if( psCnfPath == NULL || psFinalPath == NULL || iFinPathLen <= 0 )
    {
        cout<<"input invailed"<<endl;
        RET_LINE();
    }

    if( access(psCnfPath,F_OK)<0 )
    {
        cout<<"cnf: "<<psCnfPath<<" can't access"<<endl;
        RET_LINE();
    }

    if( psCnfPath[0] != '/' )
    {
        if( getcwd(tmpPath,sizeof(tmpPath)) == NULL )
        {
            cout<<"get cur dir failed"<<endl;
            RET_LINE();
        }
        snprintf(psFinalPath,iFinPathLen,"%s/%s",tmpPath,psCnfPath);
    }
    else
    {
        snprintf(psFinalPath,iFinPathLen,"%s",psCnfPath);
    }

    cout<<"final conf path: "<<psFinalPath<<endl;

    return 0;
}

static int LoadMailRdsConf(const std::string& strFilePath)
{
	pstConfig->mail_conf_map.clear();

	TiXmlDocument doc(strFilePath);
	bool loadOkay = doc.LoadFile();
	if (loadOkay == false)
		return ERR_WK_LOAD_CONF;
	TiXmlElement* root = doc.RootElement();
    TiXmlNode* useritem = root->FirstChild("usermail");
    TOOLCHECKNOTNULL(useritem);
    for(TiXmlNode* dbitem = useritem->FirstChild("redis");
        dbitem;
        dbitem = dbitem->NextSibling("redis"))
    {
        TiXmlElement* dbelement = dbitem->ToElement();
        TOOLCHECKNOTNULL(dbelement->Attribute("bid"));
        TOOLCHECKNOTNULL(dbelement->Attribute("eid"));
        TOOLCHECKNOTNULL(dbelement->Attribute("ip"));
        TOOLCHECKNOTNULL(dbelement->Attribute("port"));
		TOOLCHECKNOTNULL(dbelement->Attribute("password"));
        NETPOINT net;
        net.ip = dbelement->Attribute("ip");
		net.port= dbelement->Attribute("port");
		net.passwd = dbelement->Attribute("password");
        int begin = CTrans::STOI(dbelement->Attribute("bid"));
        int end= CTrans::STOI(dbelement->Attribute("eid"));
        for(int i = begin; i<=end; i++)
        {
            pstConfig->mail_conf_map[i] = net;
        }
	}

	return 0;
}

static int LoadUidCacheRdsConf(const std::string& strFilePath)
{
	pstConfig->uid_cache_conf_map.clear();

	LOG_DEBUG("Init File %s",strFilePath.c_str());
	TiXmlDocument doc(strFilePath);
	bool loadOkay = doc.LoadFile();
	if (loadOkay == false)
		return ERR_WK_LOAD_CONF;
	TiXmlElement* root = doc.RootElement();
    TiXmlNode* useritem = root->FirstChild("uidcache");
    TOOLCHECKNOTNULL(useritem);
    for(TiXmlNode* dbitem = useritem->FirstChild("redis");
        dbitem;
        dbitem = dbitem->NextSibling("redis"))
    {
        TiXmlElement* dbelement = dbitem->ToElement();
        TOOLCHECKNOTNULL(dbelement->Attribute("bid"));
        TOOLCHECKNOTNULL(dbelement->Attribute("eid"));
        TOOLCHECKNOTNULL(dbelement->Attribute("ip"));
        TOOLCHECKNOTNULL(dbelement->Attribute("port"));
		TOOLCHECKNOTNULL(dbelement->Attribute("password"));
        NETPOINT net;
        net.ip = dbelement->Attribute("ip");
		net.port= dbelement->Attribute("port");
		net.passwd = dbelement->Attribute("password");
        int begin = CTrans::STOI(dbelement->Attribute("bid"));
        int end= CTrans::STOI(dbelement->Attribute("eid"));
        for(int i = begin; i<=end; i++)
        {
            pstConfig->uid_cache_conf_map[i] = net;
        }
	}

	return 0;
}

static int LoadUserFriendRdsConf(const std::string& strFilePath)
{
	pstConfig->userfriend_conf_map.clear();

	LOG_DEBUG("Init File %s",strFilePath.c_str());
	TiXmlDocument doc(strFilePath);
	bool loadOkay = doc.LoadFile();
	if (loadOkay == false)
		return ERR_WK_LOAD_CONF;
	TiXmlElement* root = doc.RootElement();
    TiXmlNode* useritem = root->FirstChild("userfriend");
    TOOLCHECKNOTNULL(useritem);
    for(TiXmlNode* dbitem = useritem->FirstChild("redis");
        dbitem;
        dbitem = dbitem->NextSibling("redis"))
    {
        TiXmlElement* dbelement = dbitem->ToElement();
        TOOLCHECKNOTNULL(dbelement->Attribute("bid"));
        TOOLCHECKNOTNULL(dbelement->Attribute("eid"));
        TOOLCHECKNOTNULL(dbelement->Attribute("ip"));
        TOOLCHECKNOTNULL(dbelement->Attribute("port"));
		TOOLCHECKNOTNULL(dbelement->Attribute("password"));
        NETPOINT net;
        net.ip = dbelement->Attribute("ip");
		net.port= dbelement->Attribute("port");
		net.passwd = dbelement->Attribute("password");
        int begin = CTrans::STOI(dbelement->Attribute("bid"));
        int end= CTrans::STOI(dbelement->Attribute("eid"));
        for(int i = begin; i<=end; i++)
        {
            pstConfig->userfriend_conf_map[i] = net;
        }
	}

	return 0;
}

static int LoadModBitmapRdsConf(const std::string& strFilePath)
{
	pstConfig->uid_cache_conf_map.clear();

	LOG_DEBUG("Init File %s",strFilePath.c_str());
	TiXmlDocument doc(strFilePath);
	bool loadOkay = doc.LoadFile();
	if (loadOkay == false)
		return ERR_WK_LOAD_CONF;
	TiXmlElement* root = doc.RootElement();
    TiXmlNode* useritem = root->FirstChild("modbitmap");
    TOOLCHECKNOTNULL(useritem);
    for(TiXmlNode* dbitem = useritem->FirstChild("redis");
        dbitem;
        dbitem = dbitem->NextSibling("redis"))
    {
        TiXmlElement* dbelement = dbitem->ToElement();
        TOOLCHECKNOTNULL(dbelement->Attribute("bid"));
        TOOLCHECKNOTNULL(dbelement->Attribute("eid"));
        TOOLCHECKNOTNULL(dbelement->Attribute("ip"));
        TOOLCHECKNOTNULL(dbelement->Attribute("port"));
		TOOLCHECKNOTNULL(dbelement->Attribute("password"));
        NETPOINT net;
        net.ip = dbelement->Attribute("ip");
		net.port= dbelement->Attribute("port");
		net.passwd = dbelement->Attribute("password");
        int begin = CTrans::STOI(dbelement->Attribute("bid"));
        int end= CTrans::STOI(dbelement->Attribute("eid"));
        for(int i = begin; i<=end; i++)
        {
            pstConfig->mod_bitmap_conf_map[i] = net;
        }
	}

	return 0;
}


int LoadNetportVec(const std::string& strFilePath)
{
    pstConfig->stNetportVec.clear();

	TiXmlDocument doc(strFilePath);
	bool loadOkay = doc.LoadFile();
	if (loadOkay == false)
		return ERR_WK_LOAD_CONF;
	TiXmlElement* root = doc.RootElement();
    TiXmlNode* useritem = root->FirstChild("user");
    TOOLCHECKNOTNULL(useritem);
    for(TiXmlNode* dbitem = useritem->FirstChild("redis");
        dbitem;
        dbitem = dbitem->NextSibling("redis"))
    {
        TiXmlElement* dbelement = dbitem->ToElement();
        TOOLCHECKNOTNULL(dbelement->Attribute("bid"));
        TOOLCHECKNOTNULL(dbelement->Attribute("eid"));
        TOOLCHECKNOTNULL(dbelement->Attribute("ip"));
        TOOLCHECKNOTNULL(dbelement->Attribute("port"));
        TOOLCHECKNOTNULL(dbelement->Attribute("password"));
        NETPOINT net;
        net.ip = dbelement->Attribute("ip");
     	//net.port= CTrans::STOI(dbelement->Attribute("port"));
		net.port= dbelement->Attribute("port");
        net.passwd= dbelement->Attribute("password");

		pstConfig->stNetportVec.push_back(net);
	}
	
	return 0;
}


static int LoadRedisConfig(const std::string& strFilePath)
{
	int iRet = 0;
	
	iRet = IsVaildAndCompse(strFilePath.c_str(), pstConfig->cUserDataRedisConf, sizeof(pstConfig->cUserDataRedisConf));
	if(0 > iRet)
	{
		cout<<"IsVaildAndCompse CheRdsCnfPath Error, iRet="<<iRet<<endl;
		RET_LINE();
	}
	RET_TRUE_LINE(pstConfig->CRdsCnfObj.LoadConf(pstConfig->cUserDataRedisConf));
	RET_TRUE_LINE(LoadUidCacheRdsConf(pstConfig->cUserDataRedisConf));
	RET_TRUE_LINE(LoadMailRdsConf(pstConfig->cUserDataRedisConf));
	RET_TRUE_LINE(LoadUserFriendRdsConf(pstConfig->cUserDataRedisConf));
	RET_TRUE_LINE(LoadModBitmapRdsConf(pstConfig->cUserDataRedisConf));
	RET_TRUE_LINE(LoadNetportVec(pstConfig->cUserDataRedisConf));

	return 0;
}

static unsigned int GetUidCrc32(uint64_t ddwUin)
{
    //need to check crc32(uid);
    std::string sAccUid = CTrans::ITOS(ddwUin);
    unsigned int iCrcKeyUid= load_crc32(0, (const unsigned char *)sAccUid.c_str(), (int)sAccUid.size());
    return iCrcKeyUid;
}

static int get_uid_by_openid()
{
	uint32_t dwCrcId=0, dwIndex=0;
	int iRet=0, iPort=0;
	
	CRedis *pstRedis = NULL;
	NETPOINT cIpPort;
	string strUidCacheKey, strUidCacheValue;
	
	dwCrcId = load_crc32(0, (unsigned const char *)pstConfig->strOpenId.c_str(), (int)(pstConfig->strOpenId.size()));
	dwIndex = dwCrcId%100;

	if(pstConfig->uid_cache_conf_map.find(dwIndex) != pstConfig->uid_cache_conf_map.end())
	{
		cIpPort = pstConfig->uid_cache_conf_map[dwIndex];
		iPort = atoi(cIpPort.port.c_str());

	    pstRedis = GET_REDIS_AUTH(cIpPort.ip, iPort, cIpPort.passwd);
	    if(pstRedis)
	    {
	    	strUidCacheValue.clear();
	    	strUidCacheKey = "openid:"+pstConfig->strOpenId;
			iRet = pstRedis->GetKey(strUidCacheKey, strUidCacheValue);
			if(REPLY_NULL==iRet || NO_CONTEXT==iRet)
			{
				pstRedis->Close();
				if(0 == pstRedis->Connect())
				{
					iRet = pstRedis->GetKey(strUidCacheKey, strUidCacheValue);
					if(!iRet)
					{
						pstConfig->ddwUin = strtoul(strUidCacheValue.c_str(), 0, 10);
						cout<<"get uid="<<pstConfig->ddwUin<<" by openid="<<pstConfig->strOpenId<<endl;
						
						return 0;
					}
					else if(REDIS_NO_DATA == iRet)
					{
						cout<<"can not get uid by openid="<<pstConfig->strOpenId<<" ,reason is redis_no_data"<<endl;
						RET_LINE();
					}
				}
			}
			else if(REDIS_NO_DATA == iRet)
			{
				cout<<"can not get uid by openid="<<pstConfig->strOpenId<<endl;
				RET_LINE();
			}
			else if(!iRet)
			{
				pstConfig->ddwUin = strtoul(strUidCacheValue.c_str(), 0, 10);
				cout<<"get uid="<<pstConfig->ddwUin<<" by openid="<<pstConfig->strOpenId<<endl;
				
				return 0;
			}
			else
			{
				cout<<"get uid by openid="<<pstConfig->strOpenId<<" Error, iRet="<<iRet<<endl;
				RET_LINE();
			}
	    }
	}

	return 0;
}

static CRedis *get_redis_by_uid()
{
	int iRet = 0;
	NETPOINT cIpPort;
	CRedis *pstRedis = NULL;

	unsigned int iCrcKeyUid = GetUidCrc32(pstConfig->ddwUin);
	iRet = pstConfig->CRdsCnfObj.GetUserIpPort(iCrcKeyUid, cIpPort);
    if(0 != iRet)
	{
		cout<<"get cache ip error. Uid="<<pstConfig->ddwUin<<endl;
		return NULL;
	}

	pstRedis = GET_REDIS_AUTH(cIpPort.ip, atoi(cIpPort.port.c_str()), cIpPort.passwd);

	return pstRedis;
}

static CRedis *get_mail_redis_by_uid()
{
	int iPort = 0;
	NETPOINT cIpPort;
	CRedis *pstRedis = NULL;

	unsigned int iCrcKeyUid = GetUidCrc32(pstConfig->ddwUin);
	uint32_t dwRedisIndex = iCrcKeyUid%100;

	if(pstConfig->mail_conf_map.find(dwRedisIndex) != pstConfig->mail_conf_map.end())
	{
		cIpPort = pstConfig->mail_conf_map[dwRedisIndex];
		iPort = atoi(cIpPort.port.c_str());

		pstRedis = GET_REDIS_AUTH(cIpPort.ip, iPort, cIpPort.passwd);
	}

	return pstRedis;
}

static CRedis *get_user_friend_redis_by_uid()
{
	int iPort = 0;
	NETPOINT cIpPort;
	CRedis *pstRedis = NULL;

	unsigned int iCrcKeyUid = GetUidCrc32(pstConfig->ddwUin);
	uint32_t dwRedisIndex = iCrcKeyUid%100;

	if(pstConfig->userfriend_conf_map.find(dwRedisIndex) != pstConfig->userfriend_conf_map.end())
	{
		cIpPort = pstConfig->userfriend_conf_map[dwRedisIndex];
		iPort = atoi(cIpPort.port.c_str());

		pstRedis = GET_REDIS_AUTH(cIpPort.ip, iPort, cIpPort.passwd);
	}

	return pstRedis;
}

static int get_user_basic_data()
{
	int iRet = 0;
	std::string strBasicDataKey, strBasicDataValue, strOutput;
	char cBasicDataKey[128] = {0};
	CRedis *pstRedis = NULL;

	pstRedis = get_redis_by_uid();
	if(NULL == pstRedis)
		RET_LINE();

	
	snprintf(cBasicDataKey, sizeof(cBasicDataKey), "%s:%s:%lu", 
		pstConfig->strPlatInfo.c_str(), pstConfig->strUserDataCol.c_str(), pstConfig->ddwUin);
	
	strBasicDataKey = cBasicDataKey;

	iRet = pstRedis->GetKey(strBasicDataKey, strBasicDataValue);
	if(iRet)
	{
		cout<<"get_key iRet="<<iRet<<endl;

		if(REPLY_NULL==iRet || NO_CONTEXT==iRet)
		{
			pstRedis->Close();
			if((iRet=pstRedis->Connect()))
			{
				cout<<"connect to redis_server Error, iRet="<<iRet<<endl;;
				RET_LINE();
			}

			iRet = pstRedis->GetKey(strBasicDataKey, strBasicDataValue);
			if(iRet)
			{
				cout<<"get data from redis Error Again!!!!!!!!! Uin="<<pstConfig->ddwUin<<" ,iRet=%d"<<iRet<<endl;
				RET_LINE();
			}
		}
		else if(REDIS_NO_DATA == iRet)
		{
			cout<<"redis_no_data, Uid="<<pstConfig->ddwUin;
			cout<<" plat_info="<<pstConfig->strPlatInfo;
			cout<<" user_data_col="<<pstConfig->strUserDataCol<<endl;

			return 0;
		}
		else
			RET_LINE();
	}

	if("profile" == pstConfig->strUserDataCol)
	{
		FNF::profile stProfile;

		if(!stProfile.ParseFromString(strBasicDataValue))
			RET_LINE();

		strOutput = stProfile.Utf8DebugString();
	}
	else if("backpack" == pstConfig->strUserDataCol)
	{
		FNF::back_pack stBackPack;

		if(!stBackPack.ParseFromString(strBasicDataValue))
			RET_LINE();

		strOutput = stBackPack.Utf8DebugString();
	}
	else if("growup" == pstConfig->strUserDataCol)
	{
		FNF::grow_up stGrowUp;

		if(!stGrowUp.ParseFromString(strBasicDataValue))
			RET_LINE();

		strOutput = stGrowUp.Utf8DebugString();
	}
	else if("wallet" == pstConfig->strUserDataCol)
	{
		FNF::wallet stWallet;

		if(!stWallet.ParseFromString(strBasicDataValue))
			RET_LINE();

		strOutput = stWallet.Utf8DebugString();
	}

	cout<<strOutput;

	return 0;
}

static int get_mailbox()
{
	int iRet = 0;
	string strMailKey, strMailData, strOutput;
	char cMailKey[128] = {0};
	CRedis *pstRedis = NULL;

	pstRedis = get_mail_redis_by_uid();
	if(NULL == pstRedis)
		RET_LINE();

	snprintf(cMailKey, sizeof(cMailKey), "mail:%lu", pstConfig->ddwUin);
	strMailKey = cMailKey;
	iRet = pstRedis->GetKey(strMailKey, strMailData);
	if(iRet)
	{
		if(REPLY_NULL==iRet || NO_CONTEXT==iRet)
		{
			pstRedis->Close();
			if((iRet=pstRedis->Connect()))
			{
				cout<<"connect to redis_server Error, iRet="<<iRet<<endl;
				RET_LINE();
			}

			iRet = pstRedis->GetKey(strMailKey, strMailData);
			if(iRet)
			{
				cout<<"get data from redis Error Again!!!!!!!!! Uin="<<pstConfig->ddwUin<<" ,iRet=%d"<<iRet<<endl;
				RET_LINE();
			}
		}
		else if(REDIS_NO_DATA == iRet)
		{
			cout<<"redis_no_data, Uid="<<pstConfig->ddwUin;
			cout<<" plat_info="<<pstConfig->strPlatInfo;
			cout<<" user_data_col="<<pstConfig->strUserDataCol<<endl;

			return 0;
		}
		else
			RET_LINE();
	}

	FNF::mailbox stMailbox;

	if(!stMailbox.ParseFromString(strMailData))
		RET_LINE();

	strOutput = stMailbox.Utf8DebugString();
	cout<<strOutput;
	
	return 0;
}

static int get_user_friend()
{
	int iRet = 0;
	string strUserFriendKey, struserFriendData, strOutput;
	char cUserFriendKey[128] = {0};
	CRedis *pstRedis = NULL;

	pstRedis = get_user_friend_redis_by_uid();
	if(NULL == pstRedis)
		RET_LINE();

	snprintf(cUserFriendKey, sizeof(cUserFriendKey), "userfriend:%lu", pstConfig->ddwUin);
	strUserFriendKey = cUserFriendKey;
	iRet = pstRedis->GetKey(strUserFriendKey, struserFriendData);
	if(iRet)
	{
		if(REPLY_NULL==iRet || NO_CONTEXT==iRet)
		{
			pstRedis->Close();
			if((iRet=pstRedis->Connect()))
			{
				cout<<"connect to redis_server Error, iRet="<<iRet<<endl;
				RET_LINE();
			}

			iRet = pstRedis->GetKey(strUserFriendKey, struserFriendData);
			if(iRet)
			{
				cout<<"get data from redis Error Again!!!!!!!!! Uin="<<pstConfig->ddwUin<<" ,iRet=%d"<<iRet<<endl;
				RET_LINE();
			}
		}
		else if(REDIS_NO_DATA == iRet)
		{
			cout<<"redis_no_data, Uid="<<pstConfig->ddwUin;
			cout<<" plat_info="<<pstConfig->strPlatInfo;
			cout<<" user_data_col="<<pstConfig->strUserDataCol<<endl;

			return 0;
		}
		else
			RET_LINE();
	}

	FNF::user_friends stUserFriend;

	if(!stUserFriend.ParseFromString(struserFriendData))
		RET_LINE();

	strOutput = stUserFriend.Utf8DebugString();
	cout<<strOutput;
	return 0;
}

static int InitConfig(int argc, char **argv)
{
	if(100<pstConfig->dwCmdId && 7>argc)
	{
		cout<<"usage: "<<argv[0]<<"cmd_id uid/openid fnf_redis.xml plat_info user_data_name <item_name/queue_size>"<<endl;
		cout<<"cmd_id: 1--select by uid; 2--select by openid"<<endl;
		cout<<"\t101--clear by uid; 102--clear by openid"<<endl;
		cout<<"plat_info: and/ios"<<endl;
		cout<<"user_data_name: profile/backpack/growup/mailbox/userfriend"<<endl;
		cout<<"item_name: vipscore"<<endl;

		RET_LINE();
	}

	if(1 == (pstConfig->dwCmdId%2))
	{
		pstConfig->ddwUin = strtoul(argv[2], 0, 10);
		if(0 == pstConfig->ddwUin)
		{
			cout<<"Invalid Uin"<<endl;
			RET_LINE();
		}
	}
	else
	{
		pstConfig->strOpenId = argv[2];
	}

	pstConfig->strPlatInfo = argv[4];
	if("and"!=pstConfig->strPlatInfo && "ios"!=pstConfig->strPlatInfo)
	{
		cout<<"Invalid PlatInfo="<<pstConfig->strPlatInfo<<" ,PlatInfo should be and or ios"<<endl;
		RET_LINE();
	}

	pstConfig->strUserDataCol = argv[5];

	if(100 < pstConfig->dwCmdId)
		pstConfig->strAtomicItem = argv[6];
	
	return 0;
}

static int SelectUserData()
{
	int iRet = 0;
	
	if("profile"==pstConfig->strUserDataCol 
		|| "backpack" == pstConfig->strUserDataCol 
		|| "growup" == pstConfig->strUserDataCol 
		|| "wallet" == pstConfig->strUserDataCol)
	{
		iRet = get_user_basic_data();
	}
	else if("mailbox" == pstConfig->strUserDataCol)
	{
		iRet = get_mailbox();
	}
	else if("userfriend" == pstConfig->strUserDataCol)
	{
		iRet = get_user_friend();
	}

	if(iRet)
	{
		cout<<"Select User Data Error, Uid="<<pstConfig->ddwUin;
		cout<<", plat_info="<<pstConfig->strPlatInfo;
		cout<<", user_data_col="<<pstConfig->strUserDataCol<<endl;

		RET_LINE();
	}
	
	return 0;
}

static int ClearOneItemInProfile()
{
	int iRet = 0;
	std::string strBasicDataKey, strBasicDataValue;
	char cBasicDataKey[128] = {0};
	CRedis *pstRedis = NULL;

	pstRedis = get_redis_by_uid();
	if(NULL == pstRedis)
		RET_LINE();

	
	snprintf(cBasicDataKey, sizeof(cBasicDataKey), "%s:profile:%lu", 
		pstConfig->strPlatInfo.c_str(), pstConfig->ddwUin);
	
	strBasicDataKey = cBasicDataKey;

	iRet = pstRedis->GetKey(strBasicDataKey, strBasicDataValue);
	if(iRet)
	{
		cout<<"get_key iRet="<<iRet<<endl;

		if(REPLY_NULL==iRet || NO_CONTEXT==iRet)
		{
			pstRedis->Close();
			if((iRet=pstRedis->Connect()))
			{
				cout<<"connect to redis_server Error, iRet="<<iRet<<endl;;
				RET_LINE();
			}

			iRet = pstRedis->GetKey(strBasicDataKey, strBasicDataValue);
			if(iRet)
			{
				cout<<"get data from redis Error Again!!!!!!!!! Uin="<<pstConfig->ddwUin<<" ,iRet=%d"<<iRet<<endl;
				RET_LINE();
			}
		}
		else if(REDIS_NO_DATA == iRet)
		{
			cout<<"redis_no_data, Uid="<<pstConfig->ddwUin;
			cout<<" plat_info="<<pstConfig->strPlatInfo;
			cout<<" user_data_col="<<pstConfig->strUserDataCol<<endl;

			return 0;
		}
		else
			RET_LINE();
	}

	FNF::profile stProfile;

	if(!stProfile.ParseFromString(strBasicDataValue))
		RET_LINE();

	if("vipscore" == pstConfig->strAtomicItem)
		stProfile.set_vipscore(0);
	else
		return 0;

	strBasicDataValue.clear();
	stProfile.SerializeToString(&strBasicDataValue);

	iRet = pstRedis->SetKey(strBasicDataKey, strBasicDataValue);
	if(iRet)
	{
		if(NO_CONTEXT == iRet)
		{
			pstRedis->Close();
			if((iRet=pstRedis->Connect()))
			{
				cout<<"connect to redis_server Error, iRet="<<iRet<<endl;
				RET_LINE();
			}

			iRet = pstRedis->SetKey(strBasicDataKey, strBasicDataValue);
			if(iRet)
			{
				cout<<"save data to redis Error Again!!!!!!!!!!!! Uin="<<pstConfig->ddwUin<<", iRet="<<iRet<<endl;
				RET_LINE();
			}
		}
		else
			RET_LINE();
	}
	
	return 0;
}

static int ClearOneItemInMailbox()
{
	int iRet = 0;
	string strMailKey, strMailData, strOutput;
	char cMailKey[128] = {0};
	CRedis *pstRedis = NULL;

	pstRedis = get_mail_redis_by_uid();
	if(NULL == pstRedis)
		RET_LINE();

	snprintf(cMailKey, sizeof(cMailKey), "mail:%lu", pstConfig->ddwUin);
	strMailKey = cMailKey;
	iRet = pstRedis->GetKey(strMailKey, strMailData);
	if(iRet)
	{
		if(REPLY_NULL==iRet || NO_CONTEXT==iRet)
		{
			pstRedis->Close();
			if((iRet=pstRedis->Connect()))
			{
				cout<<"connect to redis_server Error, iRet="<<iRet<<endl;
				RET_LINE();
			}

			iRet = pstRedis->GetKey(strMailKey, strMailData);
			if(iRet)
			{
				cout<<"get data from redis Error Again!!!!!!!!! Uin="<<pstConfig->ddwUin<<" ,iRet=%d"<<iRet<<endl;
				RET_LINE();
			}
		}
		else if(REDIS_NO_DATA == iRet)
		{
			cout<<"redis_no_data, Uid="<<pstConfig->ddwUin;
			cout<<" plat_info="<<pstConfig->strPlatInfo;
			cout<<" user_data_col="<<pstConfig->strUserDataCol<<endl;

			return 0;
		}
		else
			RET_LINE();
	}

	FNF::mailbox stMailbox;

	if(!stMailbox.ParseFromString(strMailData))
		RET_LINE();

	if("sysmail" == pstConfig->strAtomicItem)
	{
		stMailbox.clear_system_mail();
	}
	else if("usermail" == pstConfig->strAtomicItem)
	{
		stMailbox.clear_user_mail();
	}

	strMailData.clear();
	stMailbox.SerializeToString(&strMailData);

	iRet = pstRedis->SetKey(strMailKey, strMailData);
	if(iRet)
	{
		if(NO_CONTEXT == iRet)
		{
			pstRedis->Close();
			if((iRet=pstRedis->Connect()))
			{
				cout<<"connect to redis_server Error, iRet="<<iRet<<endl;
				RET_LINE();
			}

			iRet = pstRedis->SetKey(strMailKey, strMailData);
			if(iRet)
			{
				cout<<"save data to redis Error Again!!!!!!!!!!!! Uin="<<pstConfig->ddwUin<<", iRet="<<iRet<<endl;
				RET_LINE();
			}
		}
		else
			RET_LINE();
	}

	return 0;
}

static int ClearUserData()
{
	int iRet = 0;

	if("profile" == pstConfig->strUserDataCol)
	{
		iRet = ClearOneItemInProfile();
		if(iRet)
		{
			cout<<"ClearOneItemInProfile Error, iRet="<<iRet<<endl;
			RET_LINE();
		}
	}

	if("mailbox" == pstConfig->strUserDataCol)
	{
		iRet = ClearOneItemInMailbox();
		if(iRet)
		{
			cout<<"ClearOneItemInProfile Error, iRet="<<iRet<<endl;
			RET_LINE();
		}
	}

	return 0;
}

static int InitGameCountMap(char *pcOpenidlistFile)
{
	if(NULL == pcOpenidlistFile)
		RET_LINE();

	FILE *fp = fopen(pcOpenidlistFile, "r");
	if(NULL == fp)
	{
		cout<<"invalid openid_list_file: "<<pcOpenidlistFile<<endl;
		RET_LINE();
	}

	char cOpenid[128] = {0};
	int i = 0;
	while(fgets(cOpenid, sizeof(cOpenid), fp))
	{
		for(i=0; i<128; i++)
		{
			if(0==isdigit(cOpenid[i]) && 0==isalpha(cOpenid[i]))
			{
				cOpenid[i] = '\0';
				break;
			}
		}
		
		pstConfig->stGameCountMap[cOpenid] = 0;
	}
	
	return 0;
}

static int SelectOneProfile(CRedis *pstRedis, std::string strOneProfileKey)
{
	if(NULL==pstRedis || 0>=strOneProfileKey.size())
		RET_LINE();

	int iRet = 0;
	FNF::profile stProfile;
	std::string strOneProfileValue, strOpenid;

	strOneProfileValue.clear();
	iRet = pstRedis->GetKey(strOneProfileKey, strOneProfileValue);
	if(0 > iRet)
	{
		if(NO_CONTEXT==iRet  || REPLY_NULL==iRet)
		{
			//re-connect
			pstRedis->Close();
			if((iRet=pstRedis->Connect()))
			{
				RET_LINE();
			}

			iRet = pstRedis->GetKey(strOneProfileKey, strOneProfileValue);
			if(0 > iRet)
			{
				RET_LINE();
			}
		}
		else
		{
			RET_LINE();
		}
	}

	stProfile.ParseFromString(strOneProfileValue);
	strOpenid = stProfile.openid();
	if(pstConfig->stGameCountMap.find(strOpenid) != pstConfig->stGameCountMap.end())
	{
		cout<<strOpenid<<"\t "<<stProfile.game_count()<<endl;
	}
	
	return 0;
}

static int SelectOneRedisSeg(std::string ip, std::string port, std::string password)
{
	int iRet = 0;
	CRedis *pstRedis = NULL;
	std::vector<std::string> stProfileKeysVec;
	
	pstRedis = GET_REDIS_AUTH(ip, atoi(port.c_str()), password);
	if(NULL == pstRedis)
	{
		cout<<"GET_REDIS_AUTH Error"<<endl;
		RET_LINE();
	}

	stProfileKeysVec.clear();
	iRet = pstRedis->KEYS("*:profile:*", stProfileKeysVec);
	if(0 > iRet)
	{
		if(NO_CONTEXT==iRet  || REPLY_NULL==iRet)
		{
			//re-connect
			pstRedis->Close();
			if((iRet=pstRedis->Connect()))
			{
				RET_LINE();
			}

			iRet = pstRedis->KEYS("*:profile:*", stProfileKeysVec);
			if(0 > iRet)
			{
				RET_LINE();
			}
		}
		else
		{
			RET_LINE();
		}
	}

	for(std::vector<std::string>::iterator iter=stProfileKeysVec.begin(); iter!=stProfileKeysVec.end(); iter++)
	{
		SelectOneProfile(pstRedis, *iter);
	}


	return 0;
}

static int SelectGameCountByOpenidList()
{
	int iRet = 0;

	for(std::vector<NETPOINT>::iterator iter=pstConfig->stNetportVec.begin();
		iter!=pstConfig->stNetportVec.end(); iter++)
	{
		iRet = SelectOneRedisSeg(iter->ip, iter->port, iter->passwd);
		if(iRet)
		{
			cout<<"SelectOneRedisSeg Error, ip="<<iter->ip<<" port="
				<<iter->port<<" password="<<iter->passwd<<" iRet="<<iRet<<endl;
		}
	}
	
	return 0;
}

static int SelectModBitmapQueue()
{
	int iRet = 0;
	unsigned int uCrcRet=0, uIndex=0, uPort=0;
	size_t i = 0;
	NETPOINT stPoint;
	CRedis *pstRedis = NULL;
	std::vector<std::string> stValVec, stScoreVec;
	FNF::mod_bitmap stBitMap;

	stValVec.clear();
	stScoreVec.clear();
	uCrcRet = GetUidCrc32(pstConfig->ddwUin);
	uIndex = uCrcRet%100;

	if(pstConfig->mod_bitmap_conf_map.find(uIndex) == pstConfig->mod_bitmap_conf_map.end())
	{
		cout<<"has no redis_conf for uIndex="<<uIndex<<endl;
		RET_LINE();
	}

	stPoint = pstConfig->mod_bitmap_conf_map[uIndex];
	uPort = atoi(stPoint.port.c_str());

	pstRedis = GET_REDIS_AUTH(stPoint.ip, uPort, stPoint.passwd);
	if(NULL == pstRedis)
	{
		cout<<"can not get redis for uIndex="<<uIndex<<endl;
		RET_LINE();
	}

	iRet = pstRedis->ZRANGEWITHSCORE("userbitmap", 0, 100, stValVec, stScoreVec);
    if( iRet)
    {
    	cout<<"zrangewithscore Error, iRet="<<iRet<<endl;
        RET_LINE();
    }

	iRet = 0;
	for(i=0; i<stValVec.size(); i++)
	{
		if(stBitMap.ParseFromString(stValVec[i]))
		{
			if(pstConfig->ddwUin==stBitMap.uid() && 
				((pstConfig->strPlatInfo=="and" && stBitMap.plat()==FNF::PLAT_AND) || 
					(pstConfig->strPlatInfo=="ios" && stBitMap.plat()==FNF::PLAT_IOS)))
			{
				iRet = 1;
			}
			
			std::string strOutput = stBitMap.Utf8DebugString();
			cout<<strOutput;
		}
	}

	cout<<endl<<"result of select_queue is:"<<endl;
	if(1 == iRet)
		cout<<"\t-------------Uin="<<pstConfig->ddwUin<<" plat="<<pstConfig->strPlatInfo<<" Yes--------------------"<<endl;
	else
		cout<<"\t-------------Uin="<<pstConfig->ddwUin<<" plat="<<pstConfig->strPlatInfo<<" No--------------------"<<endl;
	
	return 0;
}

int main(int argc, char **argv)
{
	if(6 > argc)
	{
		cout<<"usage: "<<argv[0]<<"cmd_id uid/openid/openid_list fnf_redis.xml plat_info user_data_name <item_name/queue_size>"<<endl;
		cout<<"cmd_id: 1--select by uid; 2--select by openid"<<endl;
		cout<<"\t101--clear by uid; 102--clear by openid"<<endl;
		cout<<"\t201 select game_count by openid_list"<<endl;
		cout<<"plat_info: and/ios"<<endl;
		cout<<"user_data_name: profile/backpack/growup/mailbox/wallet/userfriend/modbitmap"<<endl;
		cout<<"item_name: vipscore/sysmail"<<endl;
		
		return 0;
	}

	pstConfig = &g_stConfig;

	int iRet = 0;

	pstConfig->dwCmdId = (uint32_t)atoi(argv[1]);
	if(200 > pstConfig->dwCmdId)
	{
		iRet = InitConfig(argc, argv);
		if(iRet)
		{
			cout<<"InitConfig Error, iRet="<<iRet<<endl;
			RET_LINE();
		}
	}
	
	iRet = LoadRedisConfig(argv[3]);
	if(0 > iRet)
	{
		cout<<"LoadConfig Error, iRet="<<iRet<<endl;
		RET_LINE();
	}

	if(0 == (pstConfig->dwCmdId%2))
	{
		iRet = get_uid_by_openid();
		if(iRet)
		{
			cout<<"get_uid_by_openid Error, iRet="<<iRet<<" openid="<<pstConfig->strOpenId<<endl;
			RET_LINE();
		}
	}

	switch(pstConfig->dwCmdId)
	{
		case SELECT_CMD_BY_UID:
		case SELECT_CMD_BY_OPENID:
			iRet = SelectUserData();
			if(iRet)
			{
				cout<<"SelectUserData Error, iRet="<<iRet<<endl;
				RET_LINE();
			}
			break;
		case CLEAR_CMD_BY_UID:
		case CLEAR_CMD_BY_OPENID:
			iRet = ClearUserData();
			if(iRet)
			{
				cout<<"ClearUserData Error, iRet="<<iRet<<endl;
				RET_LINE();
			}
			break;
		case SELECT_GAME_COUNT_BY_OPENIDLIST:
			iRet = InitGameCountMap(argv[2]);
			if(iRet)
			{
				cout<<"InitGameCountMap Error, iRet="<<iRet<<endl;
				RET_LINE();
			}

			iRet = SelectGameCountByOpenidList();
			if(iRet)
			{
				cout<<"SelectGameCountByOpenidList Error, iRet="<<iRet<<endl;
				RET_LINE();
			}
			break;
		case SELECT_MODBITMAP_QUEUE:
			iRet = SelectModBitmapQueue();
			if(iRet)
			{
				cout<<"SelectModBitmapQueue Error, iRet="<<iRet<<endl;
				RET_LINE();
			}
			break;
		default:
			cout<<"cmd_id: 1--select by uid; 2--select by openid"<<endl;
			cout<<"\t101--clear by uid; 102--clear by openid"<<endl;
			break;
	}

    return 0;
}
