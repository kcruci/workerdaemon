#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>
#include <time.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sys/types.h>

#include "fcgi_stdio.h"

#include "db/mysql_conf.h"
#include "db/redis_conf.h"
#include "db/keylist_conf.h"
#include "tinyxml.h"

#include "log/logger.h"
#include "db/redis_wrapper.h"
#include "db/mysql_wrapper.h"
#include "db/mysql_pool.h"
#include "db/redis_pool.h"
#include "logic/fnf_user.pb.h"
#include "logic/fnf_mail.pb.h"

#include "basic/crc.h"
#include "basic/trans.h"
#include "logic/wk_error.h"
#include "logic/wk_define.h"
#include "macrodef.h"

#include "qos_client.h"
#include "OpenApiV3.h"

#define RET_LINE() {return -__LINE__;}
#define RET_TRUE_LINE(x) do{ if(x) return -__LINE__; }while(0)
#define TOOLCHECKNOTNULL(node) \
    if(node == NULL) \
{\
    LOG_ERROR("xml error format");\
    return  ERR_WK_WRONG_CONF  ;\
}\


#define DIAMOND_ID 2
#define MIN_DIAMOND_NUM 10
#define MAX_EXPIRE_TIME 800		//15*60

#pragma pack(1)
typedef struct
{
	uint32_t dwMailType;
	uint32_t dwMailPlat;
	uint32_t dwVipLevel;
	uint32_t dwVipScore;
	uint32_t dwPrizeId;
	
	std::string strMailTitle;
	std::string strMailContent;
}mail_prize_node;

typedef struct
{
	char cPayServerConfPath[256];

	char cLogLevelConf[256];
	char cLogFilePath[256];
	char cCheRdsCnfPath[256];
	char cDbCnfPath[256];
	char cTlogDefCnfPath[256];
	char cMailPrizeConf[256];

	CRedisConf CRdsCnfObj;			//cache redis conf obj
	CRedisConf UidCacheRdsCnfObj;	//open-uid;

	MYSQLCONFMAP mysql_conf_map;
	REDISCONFMAP uid_cache_conf_map;
	REDISCONFMAP mail_conf_map;

	uint32_t dwChildNum;

	std::vector<mail_prize_node> stMailPrizeNodeVec;
	std::vector<std::string> stTlogColNameVec;

	string strGameSvrId;
	string strMailPrefix;
//	string strNumPrefix;
//	string strPlatId;
//	string strPlatPrefix;

	uint32_t dwAndAppId;
	string strAndAppKey;
	string strAndPlatId;
	string strAndPlatPrefix;
	string strAndNumPrefix;

	uint32_t dwIosAppId;
	string strIosAppKey;
	string strIosPlatId;
	string strIosPlatPrefix;
	string strIosNumPrefix;

	int iTlogSocket;
	uint32_t dwOldVipScore;
	uint32_t dwOldVipLevel;		
	
	uint32_t dwNewVipScore;
	uint32_t dwNewVipLevel;			
	
//	uint32_t dwAppId;
//	string strAppKey;
	uint32_t dwZoneId;			
	uint32_t dwMailExpireTime;
	uint64_t ddwMailSenderUid;
	uint32_t dwModId;
	uint32_t dwCmdId;
	
	string strTlogData;

	uint64_t ddwUin;
	string strOpenId;
}t_pay_callback_config;
#pragma pack()

t_pay_callback_config g_stConfig, *pstConfig;

string strOk;
string strSysBusy;
string strTokenExpire;
string strTokenNotExists;
string strParamsError;
const string strLeftBracket = "(";
const string strRigthBracket = ")";
string strUrlPath = "/t_pay_callback.cgi";
string strVersion = "v3";

string strUidCachePrefix = "openid:";
string query_string;
map<string, string> query_string_map;

//string g_strMailTitle = "VIP特权礼包";
string g_strTlogName = "UserUpVipFlow";
string strSigKey = "sig";
string strSig;

uint32_t g_dwDiamondNum = 0;

// check the conf path can be access and compose absolute dir...
static int IsVaildAndCompse(const char *psCnfPath, char *psFinalPath, int iFinPathLen)
{
    char tmpPath[256];

    if( psCnfPath == NULL || psFinalPath == NULL || iFinPathLen <= 0 )
    {
        printf("input invailed \n");
        RET_LINE();
    }

    if( access(psCnfPath,F_OK)<0 )
    {
        printf("cnf: %s can't access\n",psCnfPath);
        RET_LINE();
    }

    if( psCnfPath[0] != '/' )
    {
        if( getcwd(tmpPath,sizeof(tmpPath)) == NULL )
        {
            printf("get cur dir failed\n");
            RET_LINE();
        }
        snprintf(psFinalPath,iFinPathLen,"%s/%s",tmpPath,psCnfPath);
    }
    else
    {
        snprintf(psFinalPath,iFinPathLen,"%s",psCnfPath);
    }

    printf("final conf path:[%s]\n",psFinalPath);

    return 0;
}

//daemon_init
static int InitDaemon()
{
    int st;
    pid_t tpid, mpid;

    /* shield some signals */
    signal(SIGINT,  SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    if( -1 == daemon(1,1) )
    {
        LOG_ERROR("daemon failed");
        RET_LINE();
    }
    while(1)
    {
        tpid = fork();
        if( tpid == -1 )
        {
            return -1;
        }
        // child process..
        else if( tpid == 0 )
        {
            return 0;
        }
        else
        {
            // parent process.
        }
        // child proccess exit,parent process catch.
        mpid = wait(&st);
        if( mpid == -1 )
        {
            return -1;
        }
        //exit normally, kill by user
        if( WIFEXITED(st) || WIFSIGNALED(st) )
        {
            if( WEXITSTATUS(st) )
            {
                break;
            }
            if(SIGKILL == WTERMSIG(st) ||
                SIGSTOP == WTERMSIG(st) )
            {
                break;
            }
            //	printf(" do something \n");
        }
        //core here
        LOG_NOTICE("child process coreed, restart by parent:%d now!", getpid());
        continue;
    }
    return 0;
}

static int fork_childs()
{
	pid_t pid = 0;
	uint32_t i = 0;

	for(i=0; i<pstConfig->dwChildNum; i++)
	{
		pid = fork();
		if(0 > pid)
		{
			LOG_ERROR("fork Error");
		}
		else if(0 == pid)
			return 1;
		else
			continue;
	}
	
	return 0;
}

static uint32_t get_expire_time()
{
	time_t tCur;
	time(&tCur);

	return abs(tCur-atoi(query_string_map["ts"].c_str()));
}

string decodevalue(string &value)
{
    string strRet = "";
    char tmp[3]={'\0'}, c, *pCur=NULL;

    for (unsigned int i = 0; i < value.size();)
    {
        c = value[i];
        if('%'==c && (i+2)<value.size())
        {
            pCur = &value[i+1];
            memcpy(tmp, pCur, 2);
            c = (char)strtoul(tmp, 0, 16);

            i += 3;
        }
        else
        {
            i += 1;
        }
        strRet.append(1, c);
    }

    return strRet;
}

static int parse_query_string()
{
	const char *pCur = query_string.c_str();
	const char *pKey=NULL, *pValue=NULL;
	string strKey, strValue;
	uint32_t key_len=0, value_len=0;

	query_string_map.clear();
	while(pCur && *pCur)
	{
		key_len = value_len = 0;
		strKey.clear();
		strValue.clear();
	
		pKey = pCur;
		while(pCur && *pCur)
		{
			if('=' == *pCur)
				break;
			else
			{
				pCur ++;
				key_len ++;
			}
		}

		strKey = string(pKey, key_len);

		pCur ++;
		pValue = pCur;
		while(pCur && *pCur && '&'!=*pCur)
		{
			pCur ++;
			value_len ++;
		}
		if(pValue && *pValue)
			strValue = string(pValue, value_len);

		LOG_DEBUG("Key=%s Value=%s", strKey.c_str(), strValue.c_str());

		if(strKey == "sig")
			strSig = strValue;
		else
			query_string_map[strKey] = strValue;

		if(pCur && *pCur)
			pCur ++;
	}
	
	return 0;
}

static int LoadMysqlConfMap(const std::string& strFilePath)
{
    pstConfig->mysql_conf_map.clear();

	TiXmlDocument doc(strFilePath);
	bool loadOkay = doc.LoadFile();
	if (loadOkay == false)
		return ERR_WK_LOAD_CONF;
	TiXmlElement* root = doc.RootElement();
    TiXmlNode* useritem = root->FirstChild("usermap");
    TOOLCHECKNOTNULL(useritem);
    for(TiXmlNode* dbitem = useritem->FirstChild("db");
        dbitem;
        dbitem = dbitem->NextSibling("db"))
    {
        TiXmlElement* dbelement = dbitem->ToElement();
        TOOLCHECKNOTNULL(dbelement->Attribute("bid"));
        TOOLCHECKNOTNULL(dbelement->Attribute("eid"));
        TOOLCHECKNOTNULL(dbelement->Attribute("ip"));
        TOOLCHECKNOTNULL(dbelement->Attribute("port"));
        TOOLCHECKNOTNULL(dbelement->Attribute("dbname"));
        TOOLCHECKNOTNULL(dbelement->Attribute("tbl"));
        TOOLCHECKNOTNULL(dbelement->Attribute("user"));
        TOOLCHECKNOTNULL(dbelement->Attribute("password"));
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
            pstConfig->mysql_conf_map[i] = net;
        }
	}

	return 0;
}

static int LoadUserUpVipFlowConf(const std::string &strTlogConf)
{
	pstConfig->stTlogColNameVec.clear();

	std::string strTlogName, strTlogColName;
	TiXmlDocument doc(strTlogConf);
	bool loadOkay = doc.LoadFile();
	if (loadOkay == false)
		return ERR_WK_LOAD_CONF;
	TiXmlElement* root = doc.RootElement();
    for(TiXmlNode* dbitem = root->FirstChild("struct");
        dbitem;
        dbitem = dbitem->NextSibling("struct"))
    {
        TiXmlElement* dbelement = dbitem->ToElement();
		strTlogName = dbelement->Attribute("name");
		if(g_strTlogName != strTlogName)
			continue;

		for(TiXmlNode *pstColItem = dbelement->FirstChild("entry");
			pstColItem;
			pstColItem = pstColItem->NextSibling("entry"))
		{
			TiXmlElement* pstColNode = pstColItem->ToElement();
			
			TOOLCHECKNOTNULL(pstColNode->Attribute("name"));
			strTlogColName = pstColNode->Attribute("name");
			pstConfig->stTlogColNameVec.push_back(strTlogColName);
		}

		break;
	}

	if(0 >= pstConfig->stTlogColNameVec.size())
	{
		LOG_ERROR("Can not Find TlogStruct %s", g_strTlogName.c_str());
		RET_LINE();
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


static void InitMailPrizeNode(mail_prize_node &stNode)
{
	stNode.dwMailPlat = 0;
	stNode.dwMailType = 0;
	stNode.dwPrizeId = 0;
	stNode.dwVipLevel = 0;
	stNode.dwVipScore = 0;

	stNode.strMailTitle.clear();
	stNode.strMailContent.clear();
}

static int LoadMailPrizeConf(const std::string &strMailPrizeConf)
{
	pstConfig->stMailPrizeNodeVec.clear();
	TiXmlDocument doc(strMailPrizeConf);
	
	bool loadOkay = doc.LoadFile();
	if (loadOkay == false)
		RET_LINE();
	
	TiXmlElement* root = doc.RootElement();
    TiXmlNode* useritem = root->FirstChild("mail_prize");
    TOOLCHECKNOTNULL(useritem);
    for(TiXmlNode* dbitem = useritem->FirstChild("node");
        dbitem;
        dbitem = dbitem->NextSibling("node"))
    {
        TiXmlElement* dbelement = dbitem->ToElement();

		TOOLCHECKNOTNULL(dbelement->Attribute("mail_type"));
        TOOLCHECKNOTNULL(dbelement->Attribute("mail_plat"));
        TOOLCHECKNOTNULL(dbelement->Attribute("vip_score"));
        TOOLCHECKNOTNULL(dbelement->Attribute("vip_level"));
        TOOLCHECKNOTNULL(dbelement->Attribute("prize_id"));
        TOOLCHECKNOTNULL(dbelement->Attribute("mail_title"));
		TOOLCHECKNOTNULL(dbelement->Attribute("mail_content"));

		mail_prize_node stMailPrizeNode;
		InitMailPrizeNode(stMailPrizeNode);
        stMailPrizeNode.dwMailType = (uint32_t)CTrans::STOI(dbelement->Attribute("mail_type"));
		stMailPrizeNode.dwMailPlat = (uint32_t)CTrans::STOI(dbelement->Attribute("mail_plat"));
		stMailPrizeNode.dwVipScore = (uint32_t)CTrans::STOI(dbelement->Attribute("vip_score"));
		stMailPrizeNode.dwVipLevel = (uint32_t)CTrans::STOI(dbelement->Attribute("vip_level"));
		stMailPrizeNode.dwPrizeId = (uint32_t)CTrans::STOI(dbelement->Attribute("prize_id"));
		stMailPrizeNode.strMailTitle = dbelement->Attribute("mail_title");
		stMailPrizeNode.strMailContent = dbelement->Attribute("mail_content");
		
        pstConfig->stMailPrizeNodeVec.push_back(stMailPrizeNode);
	}
		
	return 0;
}

static int LoadConfig(const std::string& strFilePath)
{
	int iRet = 0;
	std::string strRedisConf, strMysqlConf, strTlogConf, strLogFilePath, strLogLevelConf, strMailPrizeConf;
	
	TiXmlDocument doc(strFilePath);
	bool loadOkay = doc.LoadFile();
	if (loadOkay == false)
		return ERR_WK_LOAD_CONF;
	TiXmlElement* root = doc.RootElement();
    TiXmlNode* useritem = root->FirstChild("pay_server_conf");
    TOOLCHECKNOTNULL(useritem);

	for(TiXmlNode* dbitem = useritem->FirstChild("entry");
        dbitem;
        dbitem = dbitem->NextSibling("entry"))
	{
		TiXmlElement* dbelement = dbitem->ToElement();
		
		if(NULL == dbelement->Attribute("redis_conf_file"))
			strRedisConf = "/data/webgame/server/payserver/conf/fnf_worker_redis.xml";
		else
			strRedisConf = dbelement->Attribute("redis_conf_file");

		if(NULL == dbelement->Attribute("mysql_conf_file"))
			strMysqlConf = "/data/webgame/server/payserver/conf/fnf_worker_mysql.xml";
		else
			strMysqlConf = dbelement->Attribute("mysql_conf_file");

		if(NULL == dbelement->Attribute("log_file_path"))
			memcpy(pstConfig->cLogFilePath, "/data/webgame/logs/payserver/", sizeof(pstConfig->cLogFilePath));
		else
		{
			strLogFilePath = dbelement->Attribute("log_file_path");
			memcpy(pstConfig->cLogFilePath, strLogFilePath.c_str(), sizeof(pstConfig->cLogFilePath));
		}

		if(NULL == dbelement->Attribute("log_level_conf"))
			memcpy(pstConfig->cLogLevelConf, "/data/webgame/server/payserver/conf/log_level.conf", sizeof(pstConfig->cLogLevelConf));
		else
		{
			strLogLevelConf = dbelement->Attribute("log_level_conf");
			memcpy(pstConfig->cLogLevelConf, strLogLevelConf.c_str(), sizeof(pstConfig->cLogLevelConf));
		}

		if(NULL == dbelement->Attribute("mail_prize_conf"))
			memcpy(pstConfig->cMailPrizeConf, "/data/webgame/server/payserver/conf/mail_prize_conf.xml", sizeof(pstConfig->cMailPrizeConf));
		else
			strMailPrizeConf = dbelement->Attribute("mail_prize_conf");

		if(NULL == dbelement->Attribute("tlog_conf_file"))
			strTlogConf = "/data/webgame/server/payserver/conf/tlog_def.xml";
		else
			strTlogConf = dbelement->Attribute("tlog_conf_file");

		pstConfig->strAndPlatId = "1";
		pstConfig->strAndPlatPrefix = "and:profile:";
		pstConfig->strAndNumPrefix = "and:numbers:";
		if(NULL == dbelement->Attribute("and_appid"))
		{
			//default 1450000218
			pstConfig->dwAndAppId = 1450000218;
		}
		else
		{
			pstConfig->dwAndAppId = CTrans::STOI(dbelement->Attribute("and_appid"));
		}

		if(NULL == dbelement->Attribute("and_appkey"))
		{
			//default nHlwUInCrymnUqnUzoAOIvG6qVk3pSUw
			pstConfig->strAndAppKey = "nHlwUInCrymnUqnUzoAOIvG6qVk3pSUw";
		}
		else
		{
			pstConfig->strAndAppKey = dbelement->Attribute("and_appkey");
		}

		pstConfig->strIosPlatId = "0";
		pstConfig->strIosPlatPrefix = "ios:profile:";
		pstConfig->strIosNumPrefix = "ios:numbers:";
		if(NULL == dbelement->Attribute("ios_appid"))
		{
			//default 1450000305
			pstConfig->dwIosAppId = 1450000305;
		}
		else
		{
			pstConfig->dwIosAppId = CTrans::STOI(dbelement->Attribute("ios_appid"));
		}

		if(NULL == dbelement->Attribute("ios_appkey"))
		{
			//default vvBA82L4NsjLdGW0cmtt3geX64bmRZUY
			pstConfig->strIosAppKey = "vvBA82L4NsjLdGW0cmtt3geX64bmRZUY";
		}
		else
		{
			pstConfig->strIosAppKey = dbelement->Attribute("ios_appkey");
		}

		if(NULL == dbelement->Attribute("modid"))
		{
			//default 64015233
			pstConfig->dwModId = 64015233;
		}
		else
			pstConfig->dwModId = CTrans::STOI(dbelement->Attribute("modid"));

		if(NULL == dbelement->Attribute("cmdid"))
		{
			//default qq--65536
			pstConfig->dwCmdId = 65536;
		}
		else
			pstConfig->dwCmdId = CTrans::STOI(dbelement->Attribute("cmdid"));

		if(NULL == dbelement->Attribute("childnum"))
		{
			//default 10 children
			pstConfig->dwChildNum = 10;
		}
		else
			pstConfig->dwChildNum = CTrans::STOI(dbelement->Attribute("childnum"));

		if(NULL == dbelement->Attribute("mail_expire_time"))
		{
			//default 1 year
			pstConfig->dwMailExpireTime = 86400*365;
		}
		else
			pstConfig->dwMailExpireTime = CTrans::STOI(dbelement->Attribute("mail_expire_time"));

		if(NULL == dbelement->Attribute("mail_sender_uid"))
		{
			//default 10000
			pstConfig->ddwMailSenderUid = 10000;
		}
		else
			pstConfig->ddwMailSenderUid = CTrans::STOI(dbelement->Attribute("mail_sender_uid"));

		if(NULL == dbelement->Attribute("zone_id"))
		{
			//default 1
			pstConfig->dwZoneId = 1;
		}
		else
			pstConfig->dwZoneId = CTrans::STOI(dbelement->Attribute("zone_id"));

		if(NULL == dbelement->Attribute("pay_ok"))
			strOk = "OK";
		else
			strOk = dbelement->Attribute("pay_ok");

		if(NULL == dbelement->Attribute("sys_busy"))
			strSysBusy = "系统繁忙";
		else
			strSysBusy = dbelement->Attribute("sys_busy");

		if(NULL == dbelement->Attribute("token_expire"))
			strTokenExpire = "token已过期";
		else
			strTokenExpire = dbelement->Attribute("token_expire");

		if(NULL == dbelement->Attribute("token_not_exists"))
			strTokenNotExists = "token不存在";
		else
			strTokenNotExists = dbelement->Attribute("token_not_exists");

		if(NULL == dbelement->Attribute("params_error"))
			strParamsError = "请求参数错误：";
		else
			strParamsError = dbelement->Attribute("params_error");
	}

	pstConfig->strMailPrefix = "mail:";
	pstConfig->strGameSvrId = "1";

	iRet = IsVaildAndCompse(strRedisConf.c_str(), pstConfig->cCheRdsCnfPath, sizeof(pstConfig->cCheRdsCnfPath));
	if(0 > iRet)
	{
		cout<<"IsVaildAndCompse CheRdsCnfPath Error, iRet="<<iRet<<endl;
		RET_LINE();
	}

	iRet = IsVaildAndCompse(strMysqlConf.c_str(), pstConfig->cDbCnfPath, sizeof(pstConfig->cDbCnfPath));
	if(0 > iRet)
	{
		cout<<"IsVaildAndCompse DbCnfPath Error, iRet="<<iRet<<endl;
		RET_LINE();
	}

	iRet = IsVaildAndCompse(strTlogConf.c_str(), pstConfig->cTlogDefCnfPath, sizeof(pstConfig->cTlogDefCnfPath));
	if(0 > iRet)
	{
		cout<<"IsVaildAndCompse TlogDefCnfPath Error, iRet="<<iRet<<endl;
		RET_LINE();
	}

	iRet = IsVaildAndCompse(strMailPrizeConf.c_str(), pstConfig->cMailPrizeConf, sizeof(pstConfig->cMailPrizeConf));
	if(0 > iRet)
	{
		cout<<"IsVaildAndCompse MailPrizeConf Error, iRet="<<iRet<<endl;
		RET_LINE();
	}

	RET_TRUE_LINE(pstConfig->CRdsCnfObj.LoadConf(pstConfig->cCheRdsCnfPath));
	RET_TRUE_LINE(LoadMysqlConfMap(pstConfig->cDbCnfPath));
	RET_TRUE_LINE(LoadUidCacheRdsConf(pstConfig->cCheRdsCnfPath));
	RET_TRUE_LINE(LoadMailRdsConf(pstConfig->cCheRdsCnfPath));
	RET_TRUE_LINE(LoadUserUpVipFlowConf(pstConfig->cTlogDefCnfPath));
	RET_TRUE_LINE(LoadMailPrizeConf(pstConfig->cMailPrizeConf));

	return 0;
}

static int InitCallbackSvr(int argc, char **argv)
{
	int iRet = 0;
	
	iRet = IsVaildAndCompse(argv[1], pstConfig->cPayServerConfPath, sizeof(pstConfig->cPayServerConfPath));
	if(0 > iRet)
	{
		cout<<"IsVaildAndCompse cPayServerConfPath Error, iRet="<<iRet<<endl;
		RET_LINE();
	}

	iRet = LoadConfig(pstConfig->cPayServerConfPath);
	if(0 > iRet)
	{
		cout<<"LoadConfig Error, iRet="<<iRet<<endl;
		RET_LINE();
	}

	INIT_LOG(pstConfig->cLogFilePath, "TPayCallbackSvr.log", pstConfig->cLogLevelConf);
	
	return 0;
}

static int get_diamond_num(uint32_t *pdwDiamondNum)
{
	int iStarNum = 0;
	uint32_t dwItemId = 0;

	*pdwDiamondNum = 0;
	const char *pCur = query_string_map["payitem"].c_str();

	if(pCur && *pCur && strchr(pCur, ';')==NULL)
	{
		*pdwDiamondNum = atoi(pCur);
	}
	else
	{
		while(pCur && *pCur)
		{
			dwItemId = atoi(pCur);
			if(DIAMOND_ID == dwItemId)
			{
				iStarNum = 0;
				while(pCur && *pCur)
				{
					if('*' == *pCur)
					{
						iStarNum ++;
						if(2 <= iStarNum)
						{
							pCur ++;
							if(pCur && *pCur)
								*pdwDiamondNum = atoi(pCur);
							break;
						}
					}
					pCur ++;
				}
				
				break;
			}
			else
			{
				pCur = strchr(pCur, ';');
				if(pCur && *pCur)
					pCur ++;
			}
		}
	}
	
	return 0;
}

static char * GetDbTableName(uint32_t dwCrcId, MYSQLNETPOINT &dbCnfDat)
{
    int iFlag = 0;
    iFlag = (dwCrcId/100)%100;
    static char sDbTabName[128];
    /** 12345; 23 is databasename, 45 tablename **/
    snprintf(sDbTabName,sizeof(sDbTabName),"%s.%s%d",dbCnfDat.dbname.c_str(),dbCnfDat.tblprefix.c_str(),iFlag);
    return sDbTabName;
}


static int get_uid_by_openid(uint64_t *pddwUin)
{
	char *pDbTableName = NULL;
	uint32_t dwCrcId=0, dwIndex=0;
	int iRet=0, iPort=0;
	
	CRedis *pstRedis = NULL;
	NETPOINT cIpPort;
	string strUidCacheKey, strUidCacheValue;
	
	unsigned long retRows=0, j=0;
	MYSQLNETPOINT mysql_point;
	CMysql *pstMysql = NULL;
	string sql_string;

//	dwCrcId = load_crc32(0, (const unsigned char *)pstConfig->strOpenId.c_str(), (int)query_string_map["openid"].size());
	dwCrcId = load_crc32(0, (const unsigned char *)pstConfig->strOpenId.c_str(), (int)pstConfig->strOpenId.size());
	dwIndex = dwCrcId%100;

	if(pstConfig->uid_cache_conf_map.find(dwIndex) != pstConfig->uid_cache_conf_map.end())
	{
		cIpPort = pstConfig->uid_cache_conf_map[dwIndex];
		iPort = atoi(cIpPort.port.c_str());

	    pstRedis = GET_REDIS_AUTH(cIpPort.ip, iPort, cIpPort.passwd);
	    if(pstRedis)
	    {
	    	strUidCacheValue.clear();
	    	strUidCacheKey = strUidCachePrefix+pstConfig->strOpenId;
			iRet = pstRedis->GetKey(strUidCacheKey, strUidCacheValue);
			if(REPLY_NULL==iRet || NO_CONTEXT==iRet)
			{
				pstRedis->Close();
				if(0 == pstRedis->Connect())
				{
					iRet = pstRedis->GetKey(strUidCacheKey, strUidCacheValue);
					if(!iRet)
					{
						*pddwUin = strtoul(strUidCacheValue.c_str(), 0, 10);
						LOG_DEBUG("uid=%lu by redis", *pddwUin);
						
						return 0;
					}
				}
			}
			if(!iRet)
			{
				*pddwUin = strtoul(strUidCacheValue.c_str(), 0, 10);
				LOG_DEBUG("uid=%lu by redis", *pddwUin);
				
				return 0;
			}
	    }
	}

	if (pstConfig->mysql_conf_map.find(dwIndex) == pstConfig->mysql_conf_map.end())
	{
		LOG_ERROR("can't find mysql_ipport, pid=%u", getpid());
		RET_LINE();
	}

	if(pstConfig->mysql_conf_map.find(dwIndex) != pstConfig->mysql_conf_map.end())
		mysql_point = pstConfig->mysql_conf_map[dwIndex];
	else
		RET_LINE();
	
	pstMysql = GET_MYSQLHANDLE(mysql_point.user, mysql_point.pwd, mysql_point.ip, mysql_point.port);
	if(!pstMysql)
	{
		LOG_ERROR("can't connect to mysql(%s:%d), pid=%u", mysql_point.ip.c_str(), mysql_point.port,  getpid());
		RET_LINE();
	}

	pDbTableName = GetDbTableName(dwCrcId, mysql_point);
	pstMysql->InitSqlString(sql_string, "select distinct uid from ");
    pstMysql->AppendSqlString(sql_string, pDbTableName);
	pstMysql->AppendSqlString(sql_string, " where openid='");
	pstMysql->AppendSqlString(sql_string, pstConfig->strOpenId.c_str());
	pstMysql->AppendSqlString(sql_string, "'");

	if( false == pstMysql->Query(sql_string.data(), sql_string.size()) )
    {
       	LOG_ERROR("select sql:[%s] failed, msg:%s pid=%u", sql_string.c_str(), pstMysql->GetError(), getpid());
        RET_LINE();
    }

    ResultSet result;
    pstMysql->FetchResult(result);
    retRows = result.GetRowsNum();

    for(j=0; j<retRows; j++)
    {
    	if(true == result.FetchRow())
    	{
    		MYSQL_ROW stRow = result.GetOneRow();
    		*pddwUin = strtoul(stRow[0], 0, 10);

			break;
    	}
    }

	return 0;
}

static unsigned int GetUidCrc32(uint64_t ddwUin)
{
    //need to check crc32(uid);
    std::string sAccUid = CTrans::ITOS(ddwUin);
    unsigned int iCrcKeyUid= load_crc32(0, (const unsigned char *)sAccUid.c_str(), (int)sAccUid.size());
    return iCrcKeyUid;
}

static int get_redis_ipport(uint64_t ddwUin, NETPOINT &cIpPort)
{
    unsigned int iCrcKeyUid = GetUidCrc32(ddwUin);

    int iRet = pstConfig->CRdsCnfObj.GetUserIpPort(iCrcKeyUid, cIpPort);
    if(0 != iRet)
	{
		LOG_ERROR("get cache ip error. uid:%lu pid=%u", ddwUin, getpid());
		RET_LINE()
	}

	return 0;
}

static CRedis *local_get_redis(uint64_t ddwUin)
{
	int iRet=0, iPort=0;
	CRedis *pstRedis = NULL;

	NETPOINT cIpPort;
	iRet = get_redis_ipport(ddwUin, cIpPort);
	if(0 > iRet)
	{
		LOG_ERROR("get_redis_ipport Error, iRet=%d", iRet);
		return NULL;
	}

	iPort = atoi(cIpPort.port.c_str());

    pstRedis = GET_REDIS_AUTH(cIpPort.ip, iPort, cIpPort.passwd);
    if(!pstRedis)
    {
        LOG_ERROR("GET_REDIS Error, iRet=%d pid=%u", iRet, getpid());
        return NULL;
    }

	return pstRedis;
}

static int update_vipsocre(uint64_t ddwUin, uint32_t dwDiamondNum)
{
	int iRet = 0;
	uint32_t dwVipScore = dwDiamondNum/MIN_DIAMOND_NUM;
	CRedis *pstRedis = NULL;
	string strProfileKey, strProfileData;
	char cProfilekey[128] = {0};

	pstRedis = local_get_redis(ddwUin);
	if(!pstRedis)
	{
		LOG_ERROR("local_get_redis Error, Uin=%lu pid=%u", ddwUin, getpid());
		RET_LINE();
	}

	if(query_string_map.find("clientver")==query_string_map.end() || "android"==query_string_map["clientver"])
		snprintf(cProfilekey, sizeof(cProfilekey), "%s%lu", pstConfig->strAndPlatPrefix.c_str(), ddwUin);
	else	
		snprintf(cProfilekey, sizeof(cProfilekey), "%s%lu", pstConfig->strIosPlatPrefix.c_str(), ddwUin);
	strProfileKey = cProfilekey;
	iRet = pstRedis->GetKey(strProfileKey, strProfileData);
	if(iRet)
	{
		LOG_ERROR("get data from redis Error, Uin=%lu iRet=%d pid=%u", ddwUin, iRet, getpid());

		if(REPLY_NULL==iRet || NO_CONTEXT==iRet)
		{
			pstRedis->Close();
			if((iRet=pstRedis->Connect()))
			{
				LOG_ERROR("connect to redis_server Error, iRet=%d", iRet);
				RET_LINE();
			}

			iRet = pstRedis->GetKey(strProfileKey, strProfileData);
			if(iRet)
			{
				LOG_ERROR("get data from redis Error Again!!!!!!!!! Uin=%lu iRet=%d pid=%u", ddwUin, iRet, getpid());
				RET_LINE();
			}
		}
		else
			RET_LINE();
	}

	FNF::profile stProfile;

	if(!stProfile.ParseFromString(strProfileData))
		RET_LINE();

	if(stProfile.openid() != pstConfig->strOpenId)
	{
		LOG_ERROR("Error openid not match! Uin=%lu profileOpenid=%s strOpenid=%s", ddwUin, stProfile.openid().c_str(), pstConfig->strOpenId.c_str());
		RET_LINE();
	}
		

	LOG_DEBUG("openid=%s before update vip_score=%u", stProfile.openid().c_str(), stProfile.vipscore());

	pstConfig->dwOldVipScore = stProfile.vipscore();
	
	stProfile.set_vipscore(stProfile.vipscore()+dwVipScore);

	pstConfig->dwNewVipScore = stProfile.vipscore();

	LOG_DEBUG("openid=%s after update vip_score=%u", stProfile.openid().c_str(), stProfile.vipscore());

	strProfileData.clear();
	stProfile.SerializeToString(&strProfileData);

	iRet = pstRedis->SetKey(strProfileKey, strProfileData);
	if(iRet)
	{
		LOG_ERROR("save data to redis Error, Uin=%lu iRet=%d pid=%u", ddwUin, iRet, getpid());

		if(NO_CONTEXT == iRet)
		{
			pstRedis->Close();
			if((iRet=pstRedis->Connect()))
			{
				LOG_ERROR("connect to redis_server Error, iRet=%d", iRet);
				RET_LINE();
			}

			iRet = pstRedis->SetKey(strProfileKey, strProfileData);
			if(iRet)
			{
				LOG_ERROR("save data to redis Error Again!!!!!!!!!!!! Uin=%lu iRet=%d pid=%u", ddwUin, iRet, getpid());
				RET_LINE();
			}
		}
		else
			RET_LINE();
	}
	
	return 0;
}

static int GetVipLevel(uint32_t dwVipScore, uint32_t *pdwVipLevel, uint32_t *pdwPrizeId)
{
	if(NULL == pdwVipLevel)
		RET_LINE();

	for(std::vector<mail_prize_node>::iterator iter=pstConfig->stMailPrizeNodeVec.begin(); 
		iter!=pstConfig->stMailPrizeNodeVec.end(); iter++)
	{
		if(dwVipScore < iter->dwVipScore)
		{
			break;
		}
		*pdwVipLevel = iter->dwVipLevel;
		*pdwPrizeId = iter->dwPrizeId;
	}

	return 0;
}

static int NeedSendPrizeMail()
{
	int iNeedSendPrize = 0;
	uint32_t dwOldPrizeId=0, dwNewPrizeId=0;
	
	RET_TRUE_LINE(GetVipLevel(pstConfig->dwOldVipScore, &pstConfig->dwOldVipLevel, &dwOldPrizeId));
	RET_TRUE_LINE(GetVipLevel(pstConfig->dwNewVipScore, &pstConfig->dwNewVipLevel, &dwNewPrizeId));

	if((pstConfig->dwNewVipLevel)>(pstConfig->dwOldVipLevel) && 0<dwNewPrizeId)
		iNeedSendPrize = 1;

	return iNeedSendPrize;
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

static int generate_one_mail_id(FNF::mailbox &stMailbox, uint32_t dwUSec, uint32_t *pdwMailId)
{
	if(NULL == pdwMailId)
		RET_LINE();

	uint8_t cMailId[5000] = {0};
	uint32_t dwMailId=0, i=0;
	::google::protobuf::RepeatedPtrField< ::FNF::mail_item > stMailItemVec = stMailbox.system_mail();
	for(::google::protobuf::RepeatedPtrField< ::FNF::mail_item >::iterator iter=stMailItemVec.begin(); 
		iter!=stMailItemVec.end(); iter++)
	{
		if(5000 >= iter->id())
			cMailId[iter->id()] = 1;
	}

	srand(dwUSec);
	while(5000 > i)
	{
		dwMailId = 501+rand()%4499;
		if(5000>=dwMailId && 0==cMailId[dwMailId])
		{
			*pdwMailId = dwMailId;
			break;
		}

		i++;
	}

	return 0;
}

static int update_prize_mail(mail_prize_node *pstNode)
{
	if(NULL == pstNode)
		RET_LINE();

	int iRet = 0;
	string strMailKey, strMailData;
	char cMailKey[128] = {0};
	CRedis *pstRedis = NULL;

	strMailData.clear();
	pstRedis = get_mail_redis_by_uid();
	if(NULL == pstRedis)
		RET_LINE();

	snprintf(cMailKey, sizeof(cMailKey), "%s%lu", pstConfig->strMailPrefix.c_str(), pstConfig->ddwUin);
	strMailKey = cMailKey;
	iRet = pstRedis->GetKey(strMailKey, strMailData);
	if(iRet)
	{
		LOG_ERROR("get data from redis Error, Uin=%lu iRet=%d pid=%u", pstConfig->ddwUin, iRet, getpid());

		if(REPLY_NULL==iRet || NO_CONTEXT==iRet)
		{
			pstRedis->Close();
			if((iRet=pstRedis->Connect()))
			{
				LOG_ERROR("connect to redis_server Error, iRet=%d", iRet);
				RET_LINE();
			}

			iRet = pstRedis->GetKey(strMailKey, strMailData);
			if(iRet)
			{
				LOG_ERROR("get data from redis Error Again!!!!!!!!! Uin=%lu iRet=%d pid=%u", pstConfig->ddwUin, iRet, getpid());
				RET_LINE();
			}
		}
		else if(REDIS_NO_DATA != iRet){
			LOG_ERROR("get data from redis Error, Uin=%lu iRet=%d pid=%u", pstConfig->ddwUin, iRet, getpid());
			RET_LINE();
		}	
	}

	FNF::mailbox stMailbox;

	if(strMailData.empty())
	{
		LOG_ERROR("uid=%lu strMaildata empty!", pstConfig->ddwUin);
	}
	if( !strMailData.empty() && 
		!stMailbox.ParseFromString(strMailData))
		RET_LINE();

	LOG_DEBUG("uid=%lu before update system_mail_count=%lu", pstConfig->ddwUin, stMailbox.system_mail_size());

	if(4999 <= stMailbox.system_mail_size())
	{
		LOG_ERROR("uid=%lu has too many sys_mail, sys_mail_cout=%lu",
			pstConfig->ddwUin, stMailbox.system_mail_size());
		RET_LINE();
	}

	uint32_t dwMailId = 0;
	struct timeval stCur;
	gettimeofday(&stCur, NULL);

	iRet = generate_one_mail_id(stMailbox, (uint32_t)stCur.tv_usec, &dwMailId);
	if(iRet)
	{
		LOG_ERROR("uid=%lu generate_one_mail_id Error", pstConfig->ddwUin);
		RET_LINE();
	}

	FNF::mail_item *pstOneMailItem  = stMailbox.add_system_mail();
	pstOneMailItem->set_id(dwMailId);
	pstOneMailItem->set_type(pstNode->dwMailType);
	pstOneMailItem->set_plat_id(pstNode->dwMailPlat);
	pstOneMailItem->set_title(pstNode->strMailTitle);
	pstOneMailItem->set_content(pstNode->strMailContent);
	pstOneMailItem->set_expire_time(stCur.tv_sec+pstConfig->dwMailExpireTime);
	pstOneMailItem->set_sender_uid(pstConfig->ddwMailSenderUid);
	pstOneMailItem->set_send_time(stCur.tv_sec);
	pstOneMailItem->set_prize_id(pstNode->dwPrizeId);
	pstOneMailItem->set_prize_status(0);

	LOG_DEBUG("uid=%lu after update system_mail_count=%lu", pstConfig->ddwUin, stMailbox.system_mail_size());

	strMailData.clear();
	stMailbox.SerializeToString(&strMailData);

	iRet = pstRedis->SetKey(strMailKey, strMailData);
	if(iRet)
	{
		LOG_ERROR("save data to redis Error, Uin=%lu iRet=%d pid=%u", pstConfig->ddwUin, iRet, getpid());

		if(NO_CONTEXT == iRet)
		{
			pstRedis->Close();
			if((iRet=pstRedis->Connect()))
			{
				LOG_ERROR("connect to redis_server Error, iRet=%d", iRet);
				RET_LINE();
			}

			iRet = pstRedis->SetKey(strMailKey, strMailData);
			if(iRet)
			{
				LOG_ERROR("save data to redis Error Again!!!!!!!!!!!! Uin=%lu iRet=%d pid=%u", pstConfig->ddwUin, iRet, getpid());
				RET_LINE();
			}
		}
		else
			RET_LINE();
	}
	
	return 0;
}

static int update_user_numbers()
{
	int iRet = 0;
	string strNumbersKey, strNumbersData;
	char cNumbersKey[128] = {0};
	CRedis *pstRedis = NULL;

	pstRedis = local_get_redis(pstConfig->ddwUin);
	if(!pstRedis)
	{
		LOG_ERROR("local_get_redis Error, Uin=%lu pid=%u", pstConfig->ddwUin, getpid());
		RET_LINE();
	}

	if(query_string_map.find("clientver")==query_string_map.end()|| "android"==query_string_map["clientver"])
		snprintf(cNumbersKey, sizeof(cNumbersKey), "%s%lu", pstConfig->strAndNumPrefix.c_str(), pstConfig->ddwUin);
	else
		snprintf(cNumbersKey, sizeof(cNumbersKey), "%s%lu", pstConfig->strIosNumPrefix.c_str(), pstConfig->ddwUin);
	strNumbersKey = cNumbersKey;
	iRet = pstRedis->GetKey(strNumbersKey, strNumbersData);
	if(iRet)
	{
		LOG_ERROR("get data from redis Error, Uin=%lu iRet=%d pid=%u", pstConfig->ddwUin, iRet, getpid());

		if(REPLY_NULL==iRet || NO_CONTEXT==iRet)
		{
			pstRedis->Close();
			if((iRet=pstRedis->Connect()))
			{
				LOG_ERROR("connect to redis_server Error, iRet=%d", iRet);
				RET_LINE();
			}

			iRet = pstRedis->GetKey(strNumbersKey, strNumbersData);
			if(iRet)
			{
				LOG_ERROR("get data from redis Error Again!!!!!!!!! Uin=%lu iRet=%d pid=%u", pstConfig->ddwUin, iRet, getpid());
				RET_LINE();
			}
		}
		else
			RET_LINE();
	}

	FNF::numbers stNumbers;

	if(!stNumbers.ParseFromString(strNumbersData))
		RET_LINE();

	LOG_DEBUG("uid=%lu before update mail_count=%u", pstConfig->ddwUin, stNumbers.mail_count());

	stNumbers.set_mail_count(stNumbers.mail_count()+1);

	LOG_DEBUG("uid=%lu after update mail_count=%u", pstConfig->ddwUin, stNumbers.mail_count());

	strNumbersData.clear();
	stNumbers.SerializeToString(&strNumbersData);

	iRet = pstRedis->SetKey(strNumbersKey, strNumbersData);
	if(iRet)
	{
		LOG_ERROR("save data to redis Error, Uin=%lu iRet=%d pid=%u", pstConfig->ddwUin, iRet, getpid());

		if(NO_CONTEXT == iRet)
		{
			pstRedis->Close();
			if((iRet=pstRedis->Connect()))
			{
				LOG_ERROR("connect to redis_server Error, iRet=%d", iRet);
				RET_LINE();
			}

			iRet = pstRedis->SetKey(strNumbersKey, strNumbersData);
			if(iRet)
			{
				LOG_ERROR("save data to redis Error Again!!!!!!!!!!!! Uin=%lu iRet=%d pid=%u", pstConfig->ddwUin, iRet, getpid());
				RET_LINE();
			}
		}
		else
			RET_LINE();
	}
	
	return 0;
}

static int send_one_prize_mail(uint32_t dwVipLevel)
{
	int iRet = 0;

	for(std::vector<mail_prize_node>::iterator iter=pstConfig->stMailPrizeNodeVec.begin(); 
		iter!=pstConfig->stMailPrizeNodeVec.end(); iter++)
	{
		if(dwVipLevel == iter->dwVipLevel)
		{
			mail_prize_node stNode;
			InitMailPrizeNode(stNode);

			if(query_string_map.find("clientver")==query_string_map.end() || "android"==query_string_map["clientver"])
				stNode.dwMailPlat = 1;
			else
				stNode.dwMailPlat = 2;
			stNode.dwMailType = iter->dwMailType;
			stNode.dwPrizeId = iter->dwPrizeId;
			stNode.strMailTitle = iter->strMailTitle;
			stNode.strMailContent = iter->strMailContent;

			iRet = update_prize_mail(&stNode);
			if(iRet)
			{
				LOG_ERROR("update_prize_mail Error, iRet=%d Uin=%lu pid=%u", 
					iRet, pstConfig->ddwUin, getpid());
				RET_LINE();
			}
		
			iRet = update_user_numbers();
			if(iRet)
			{
				LOG_ERROR("update_user_numbers Error, iRet=%d Uin=%lu pid=%u",
					iRet, pstConfig->ddwUin, getpid());
				RET_LINE();
			}
			
			break;
		}
	}
	
	return 0;
}

static int send_all_prize_mail()
{
	int iRet = 0;
	uint32_t iVipLevel = 0;

	iRet = NeedSendPrizeMail();
	if(0 > iRet)
	{
		LOG_ERROR("NeedSendPrizeMail Error, iRet=%d", iRet);
		RET_LINE();
	}

	if(0 == iRet)
	{
		//needn't send prize
		return 0;
	}

	//升多少级，送多少个礼品
	for(iVipLevel=(pstConfig->dwOldVipLevel+1); iVipLevel<=pstConfig->dwNewVipLevel; iVipLevel++)
	{
		iRet = send_one_prize_mail(iVipLevel);
		if(iRet)
		{
			LOG_ERROR("send_one_prize_mail Error, iRet=%d uid=%lu vipLevel=%u", 
				iRet, pstConfig->ddwUin, iVipLevel);
		}
	}
	
	return 0;
}

static int pay_callback()
{
	int iRet;
	pstConfig->ddwUin = 0;

	iRet = get_uid_by_openid(&pstConfig->ddwUin);
	if(0 > iRet)
	{
		LOG_ERROR("get_uid_by_openid Error, iRet=%d pid=%u", iRet, getpid());
		RET_LINE();
	}

	if(0 == pstConfig->ddwUin)
	{
		LOG_ERROR("can not get uid by openid=%s", pstConfig->strOpenId.c_str());
		RET_LINE();
	}

	iRet = update_vipsocre(pstConfig->ddwUin, g_dwDiamondNum);
	if(iRet)
	{
		LOG_ERROR("update_vipsocre Error, Uin=%lu DiamondNum=%u, iRet=%d pid=%u", pstConfig->ddwUin, g_dwDiamondNum, iRet, getpid());
		RET_LINE();
	}
	
	return 0;
}

static int verifyParams(Json::Value &JsonRes)
{
	if(query_string_map.find("clientver")==query_string_map.end()|| "android"==query_string_map["clientver"])
	{
		if(pstConfig->dwAndAppId != (uint32_t)atoi(query_string_map["appid"].c_str()))
		{
			JsonRes["ret"] = Json::Value(4);
			JsonRes["msg"] = Json::Value(strParamsError+string("appid"));
			RET_LINE();
		}
	}
	else
	{
		if(pstConfig->dwIosAppId != (uint32_t)atoi(query_string_map["appid"].c_str()))
		{
			JsonRes["ret"] = Json::Value(4);
			JsonRes["msg"] = Json::Value(strParamsError+string("appid"));
			RET_LINE();
		}
	}

	if(pstConfig->dwZoneId != atoi(query_string_map["zoneid"].c_str()))
	{
		JsonRes["ret"] = Json::Value(4);
		JsonRes["msg"] = Json::Value(strParamsError+string("zoneid"));
		RET_LINE();
	}

	if(strVersion != query_string_map["version"])
	{
		JsonRes["ret"] = Json::Value(4);
		JsonRes["msg"] = Json::Value(strParamsError+string("version"));
		RET_LINE();
	}

	if(MAX_EXPIRE_TIME < get_expire_time())
	{
		JsonRes["ret"] = Json::Value(3);
		JsonRes["msg"] = Json::Value(strTokenNotExists);
		RET_LINE();
	}
	
	return 0;
}

static inline int getMillSecs(struct timeval *pTime1, struct timeval *pTime2)
{
	return abs((pTime2->tv_sec-pTime1->tv_sec)*1000 + (pTime2->tv_usec-pTime1->tv_usec));
}

static int GenerateTlogData()
{
	pstConfig->strTlogData = "UserUpVipFlow";
	for(std::vector<std::string>::iterator iter=pstConfig->stTlogColNameVec.begin();
		iter!=pstConfig->stTlogColNameVec.end(); iter++)
	{
		if("GameSvrId" == *iter)
		{
			pstConfig->strTlogData += std::string("|")+pstConfig->strGameSvrId;
		}
		else if("dtEventTime" == *iter)
		{
			char cCurTime[64] = {0};
			std::string strCurTime;
			time_t tCur;

			time(&tCur);
			struct tm *pstCurTime = localtime(&tCur);
			snprintf(cCurTime, sizeof(cCurTime), "%04d-%02d-%02d %02d:%02d:%02d",
				pstCurTime->tm_year+1900, pstCurTime->tm_mon+1, pstCurTime->tm_mday, 
				pstCurTime->tm_hour, pstCurTime->tm_min, pstCurTime->tm_sec);
			strCurTime = cCurTime;

			pstConfig->strTlogData += std::string("|")+strCurTime;
		}
		else if("GameAppID" == *iter)
		{
			char cGameAppID[64] = {0};
			std::string strGameAPPID;

			if(query_string_map.find("clientver")==query_string_map.end()|| "android"==query_string_map["clientver"])
				snprintf(cGameAppID, sizeof(cGameAppID), "%u", pstConfig->dwAndAppId);
			else
				snprintf(cGameAppID, sizeof(cGameAppID), "%u", pstConfig->dwIosAppId);
			
			strGameAPPID = cGameAppID;

			pstConfig->strTlogData += std::string("|")+strGameAPPID;
		}
		else if("OpenId" == *iter)
		{
			pstConfig->strTlogData += std::string("|")+query_string_map["openid"];
		}
		else if("PlatID" == *iter)
		{
			if(query_string_map.find("clientver")==query_string_map.end()|| "android"==query_string_map["clientver"])
				pstConfig->strTlogData += std::string("|")+pstConfig->strAndPlatId;
			else			
				pstConfig->strTlogData += std::string("|")+pstConfig->strIosPlatId;
		}
		else if("OldVipLevel" == *iter)
		{
			char cOldVipLevel[32] = {0};
			std::string strOldVipLevel;

			snprintf(cOldVipLevel, sizeof(cOldVipLevel), "%u", pstConfig->dwOldVipLevel);
			strOldVipLevel = cOldVipLevel;

			pstConfig->strTlogData += std::string("|")+strOldVipLevel;
		}
		else if("UpVipLevel" == *iter)
		{
			char cNewVipLevel[32] = {0};
			std::string strNewVipLevel;

			snprintf(cNewVipLevel, sizeof(cNewVipLevel), "%u", pstConfig->dwNewVipLevel);
			strNewVipLevel = cNewVipLevel;

			pstConfig->strTlogData += std::string("|")+strNewVipLevel;
		}
	}

	pstConfig->strTlogData += "\n";

	return 0;
}

static int SendTlog()
{
	int iRet = 0;
	string strErrMsg;
	QOSREQUEST stReq;
	struct sockaddr_in stTlogSvrAddr;

	if(pstConfig->dwNewVipLevel <= pstConfig->dwOldVipLevel)
	{
		//needn't send tlog
		return 0;
	}
	
	stReq._modid = pstConfig->dwModId;
	stReq._cmd = pstConfig->dwCmdId;

	memset(&stTlogSvrAddr, 0, sizeof(struct sockaddr_in));

	iRet = ApiGetRoute(stReq, 1, strErrMsg);
	if(0 > iRet)
	{
		LOG_ERROR("ApiGetRoute Error, iRet=%d err_msg=%s", iRet, strErrMsg.c_str());
		RET_LINE();
	}
	
	stTlogSvrAddr.sin_family = AF_INET;
	stTlogSvrAddr.sin_addr.s_addr = inet_addr(stReq._host_ip.c_str());
	stTlogSvrAddr.sin_port = htons(stReq._host_port);

	iRet = GenerateTlogData();
	if(0 > iRet)
	{
		LOG_ERROR("GenerateTlogData Error, iRet=%d", iRet);
		RET_LINE();
	}

	LOG_DEBUG("PID=%u tlog_svr=%s:%u tlog_data=%s", 
		 getpid(), stReq._host_ip.c_str(), stReq._host_port, pstConfig->strTlogData.c_str());

	iRet = sendto(pstConfig->iTlogSocket, pstConfig->strTlogData.c_str(), pstConfig->strTlogData.size(), 0, 
		(struct sockaddr *)&stTlogSvrAddr, sizeof(struct sockaddr_in));
	if(0 > iRet)
	{
		LOG_ERROR("sendto Error");
		RET_LINE();
	}

	return 0;
}

static int InitTlogSocket()
{
	int flags = 1;
	pstConfig->iTlogSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(0 > pstConfig->iTlogSocket)
	{
		LOG_ERROR("socket Error");
		RET_LINE();
	}

	if(ioctl(pstConfig->iTlogSocket, FIONBIO, &flags) &&
		((flags=fcntl(pstConfig->iTlogSocket, F_GETFL, 0))<0 || fcntl(pstConfig->iTlogSocket, F_SETFL, flags|O_NONBLOCK)<0))
	{
		close(pstConfig->iTlogSocket);
		RET_LINE();
	}

	if(0 > setsockopt(pstConfig->iTlogSocket, SOL_SOCKET, SO_REUSEADDR, &flags, sizeof(flags)))
	{
		LOG_ERROR("setsockopt Error");
		
		close(pstConfig->iTlogSocket);
		RET_LINE();
	}
	
	return 0;
}

static int InitTlogData()
{
	pstConfig->dwOldVipScore = 0;
	pstConfig->dwNewVipScore = 0;
	
	pstConfig->dwOldVipLevel = 0;
	pstConfig->dwNewVipLevel = 0;

	pstConfig->ddwUin = 0;
	
	pstConfig->strTlogData.clear();
	pstConfig->strOpenId.clear();
	
	return 0;
}

int main(int argc, char **argv)
{
	if(2 > argc)
	{
		cout<<"usage: "<<argv[0]<<" pay_server_conf.xml"<<endl;
		return 0;
	}

	int iRet=0, iTlogRet=0;
	Json::Value JsonRes;
	Json::FastWriter JsonWriter;

	pstConfig = &g_stConfig;
	
	iRet = InitCallbackSvr(argc, argv);
	if(0 > iRet)
	{
		cout << "InitCallbackSvr Error, iRet="<<iRet<<endl;
		return 0;
	}

	if(0 == fork_childs())
		return 0;

	iRet = InitDaemon();
	if( 0 > iRet)
	{
		LOG_ERROR("InitDaemon Error, iRet=%d pid=%u", iRet, getpid());
		exit(-1);
	}

	iRet = InitTlogSocket();
	if(iRet)
	{
		LOG_ERROR("InitTlogSocket Error, iRet=%d", iRet);
		exit(-1);
	}

	COpenApiV3 and_sdk(pstConfig->dwAndAppId, pstConfig->strAndAppKey);      
    and_sdk.init();

	COpenApiV3 ios_sdk(pstConfig->dwIosAppId, pstConfig->strIosAppKey);      
    ios_sdk.init();

    while(0 <= FCGI_Accept())
    {
        FCGI_printf( "Content-type:text/html\r\n\r\n" );

		//获取参数
        query_string = getenv("QUERY_STRING");

		LOG_DEBUG("query_string=%s pid=%u", query_string.c_str(), getpid());

		iTlogRet = InitTlogData();
		if(iTlogRet)
		{
			LOG_ERROR("InitTlogData Error, iRet=%d", iRet);
		}
		
		iRet = parse_query_string();
		if(iRet)
		{
			LOG_DEBUG("parse_query_string Error, iRet=%d pid=%u", iRet, getpid());

			if(0 > iRet)
			{
				JsonRes["ret"] = Json::Value(4);
				JsonRes["msg"] = Json::Value(strParamsError+strLeftBracket+strSigKey+strRigthBracket);

				FCGI_printf("%s", JsonWriter.write(JsonRes).c_str());
			}
			
			continue;
		}

		//验证除sig以外的参数的合法性
		iRet = verifyParams(JsonRes);
		if(iRet)
		{
			LOG_ERROR("verifyParams Error pid=%u", getpid());

			FCGI_printf("%s", JsonWriter.write(JsonRes).c_str());

			continue;
		}

		if(query_string_map.find("openid") != query_string_map.end())
			pstConfig->strOpenId = decodevalue(query_string_map["openid"]);
		else
		{
			JsonRes["ret"] = Json::Value(4);
			JsonRes["msg"] = Json::Value(strParamsError+string("openid"));

			FCGI_printf("%s", JsonWriter.write(JsonRes).c_str());
			
			continue;
		}

		iRet = get_diamond_num(&g_dwDiamondNum);
		if(0 > iRet)
		{
			LOG_ERROR("get_diamond_num Error, iRet=%d pid=%u", iRet, getpid());
			JsonRes["ret"] = Json::Value(4);
			JsonRes["msg"] = Json::Value(strSysBusy);

			FCGI_printf("%s", JsonWriter.write(JsonRes).c_str());
			
			continue;
		}

		if(MIN_DIAMOND_NUM > g_dwDiamondNum)
		{
			JsonRes["ret"] = Json::Value(0);
			JsonRes["msg"] = Json::Value("OK");

			FCGI_printf("%s", JsonWriter.write(JsonRes).c_str());
			
			continue;
		}

		string new_sig = "";
		if(query_string_map.find("clientver")==query_string_map.end()|| "android"==query_string_map["clientver"])
			new_sig = and_sdk.generatesig(OPEN_API_V3_METHOD_GET, strUrlPath, query_string_map).c_str();
		else
			new_sig = ios_sdk.generatesig(OPEN_API_V3_METHOD_GET, strUrlPath, query_string_map).c_str();

		LOG_DEBUG("new_sig = %s", new_sig.c_str());

		//验证签名
//		bool verRes=sdk.verifySig(OPEN_API_V3_METHOD_GET, strUrlPath, query_string_map, strSig);
//		if(!verRes)	
		if(new_sig != strSig)
		{
			LOG_ERROR("verifySig Error pid=%u", getpid());
			JsonRes["ret"] = Json::Value(4);
			JsonRes["msg"] = Json::Value(strParamsError+strLeftBracket+string("sig")+strRigthBracket);

			FCGI_printf("%s", JsonWriter.write(JsonRes).c_str());
			
			continue;
		}

		iRet = pay_callback();
		if(0 > iRet)
		{
			LOG_ERROR("pay_callback Error, iRet=%d pid=%u", iRet, getpid());

			JsonRes["ret"] = Json::Value(1);
			JsonRes["msg"] = Json::Value(strSysBusy);

			FCGI_printf("%s", JsonWriter.write(JsonRes).c_str());
			
			continue;
		}

		iTlogRet = send_all_prize_mail();
		if(iTlogRet)
		{
			LOG_ERROR("send_prize_mail Error, iRet=%d", iRet);
		}

		iTlogRet = SendTlog();
		if(iTlogRet)
		{
			LOG_ERROR("SendTlog Error, iRet=%d", iTlogRet);
		}

		JsonRes["ret"] = Json::Value(0);
		JsonRes["msg"] = Json::Value("OK");
		FCGI_printf("%s", JsonWriter.write(JsonRes).c_str());
    }

    return 0;
}
