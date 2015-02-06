#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>

#include "db/redis_conf.h"
#include "db/redis_wrapper.h"

#include "tinyxml.h"

#include "log/logger.h"
#include "db/redis_pool.h"


#include "basic/crc.h"
#include "basic/trans.h"
#include "logic/wk_error.h"
#include "logic/wk_define.h"
#include "logic/fnf_user.pb.h"
#include "macrodef.h"

std::string strProfileKey = "*:profile:*";
std::string strWalletKey = ":wallet:";
std::string strAndPlat = "and";
std::string strIosPlat = "ios";

#define ErrStr strerror(errno)
#define LOG_DIR "../log/"

#define RET_LINE() {return -__LINE__;}
#define RET_TRUE_LINE(x) do{ if(x) return -__LINE__; }while(0)
#define TOOLCHECKNOTNULL(node) \
    if(node == NULL) \
{\
    LOG_ERROR("xml error format");\
    return  ERR_WK_WRONG_CONF  ;\
}\

typedef enum proc_ret{
		RET_CHPROC = 1,
		RET_PARPROC = 2,
}ENUMPROCRET;

using namespace std;

#pragma pack(1)
typedef struct
{
	// conf path name.
	char cCheRdsCnfPath[256];
	char cLogConfPath[256];
	
	std::vector<NETPOINT> stAllUserPointVec;
	NETPOINT stMyRedisPoint;

	pid_t myPid;
}TransGemSvrConfig;
#pragma pack()

TransGemSvrConfig g_stConfig, *pstConfig;

static void usage(int argc, char **argv)
{
	printf("usage: %s worker_redis_cnf log_conf\n", argv[0]);
	printf("sample: %s ../conf/fnf_worker_redis.xml ../conf/log_conf\n", argv[0]);

	return;
}

//daemon_init
int InitDaemon()
{
    int st;
    pid_t tpid, mpid;

    /* shield some signals */
    signal(SIGINT,  SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    if( -1 == daemon(1,1) )
    {
        LOG_ERROR("daemon failed as: %s \n",ErrStr);
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

int LoadRedisProfileConf(const std::string& strFilePath)
{
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
        NETPOINT net;
        net.ip = dbelement->Attribute("ip");
		net.port= dbelement->Attribute("port");

		pstConfig->stAllUserPointVec.push_back(net);
	}
	return 0;
}


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
        printf("cnf: %s can't access,as:%s\n",psCnfPath, ErrStr);
        RET_LINE();
    }

    if( psCnfPath[0] != '/' )
    {
        if( getcwd(tmpPath,sizeof(tmpPath)) == NULL )
        {
            printf("get cur dir failed,as:%s\n", ErrStr);
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

static int GetLocalConfigFromCmdLine(int argc, char **argv)
{
	int iRet = 0;

	iRet = IsVaildAndCompse(argv[1], pstConfig->cCheRdsCnfPath, sizeof(pstConfig->cCheRdsCnfPath));
	if(0 > iRet)
	{
		printf("IsVaildAndCompse CheRdsCnfPath Error, iRet=%d\n", iRet);
		RET_LINE();
	}

	memcpy(pstConfig->cLogConfPath, argv[2], sizeof(pstConfig->cLogConfPath));

	return 0;
}

static int Init(int argc, char **argv)
{
	int iRet = 0;

	if(argc < 3)
	{
		usage(argc, argv);
		RET_LINE();
	}

	iRet = GetLocalConfigFromCmdLine(argc, argv);
	if(0 > iRet)
	{
		printf("GetLocalConfig Error, iRet=%d\n", iRet);
		RET_LINE();
	}

	RET_TRUE_LINE(LoadRedisProfileConf(pstConfig->cCheRdsCnfPath));
	
	INIT_LOG(LOG_DIR, "TransGemSvr.log", pstConfig->cLogConfPath);

	return 0;
}

static int ConnectToRedis()
{
	int iRet = 0;
	std::string tmpPort, tmpIp;
    
    for(std::vector<NETPOINT>::iterator iter=pstConfig->stAllUserPointVec.begin(); 
		iter!=pstConfig->stAllUserPointVec.end(); iter++)
    {
        tmpPort = iter->port;
        tmpIp = iter->ip;
        pid_t iProcId = fork();
        if(0 == iProcId)
        {
            //child process..
            if( NULL ==	GET_REDIS(tmpIp, atoi(tmpPort.c_str())))
            {
                LOG_ERROR("conn redis_conn_svr failed by <ip,port>[%s,%s] ",tmpIp.c_str(), tmpPort.c_str());
                RET_LINE();
            }
            pstConfig->stMyRedisPoint = *iter;
            pstConfig->myPid = getpid();
            return RET_CHPROC;
        }
        else if(0 < iProcId) //parent proccess..
        {
            LOG_DEBUG("Create child process_pid: %lld ", iProcId);
            continue;
        }
        else
        {
            RET_LINE();
        }
    }
	
    return RET_PARPROC; //parent return ...
}

static int GetUidAndPlatFromProfileKey(std::string strOneProfileKey, std::string &strUin, std::string &strPlat)
{
	const char *pCur = strOneProfileKey.c_str();
	const char *pValue = pCur;
	uint32_t dwValueLen=0, dwHasValue=0;

	//get plat
	dwValueLen = 0;
	while(pCur && *pCur && ':'!=*pCur)
	{
		pCur ++;
		dwValueLen ++;

		dwHasValue = 1;
	}

	if(':'==*pCur && 1==dwHasValue)
		strPlat = string(pValue, dwValueLen);
	else
		RET_LINE();

	//get uid
	if(pCur && *pCur)
	{
		pCur ++;
		dwHasValue = 0;
		while(pCur && *pCur && ':'!=*pCur)
		{
			pCur ++;

			dwHasValue = 1;
		}

		if(':'==*pCur && 1==dwHasValue)
		{
			pCur ++;
			if(pCur && *pCur)
			{
				dwHasValue = 0;
				dwValueLen = 0;
				pValue = pCur;
				
				while(pCur && *pCur)
				{
					pCur ++;
					dwValueLen ++;

					dwHasValue = 1;
				}
				if(1 == dwHasValue)
				{
					strUin = string(pValue, dwValueLen);
					return 0;
				}
				else
					RET_LINE();
			}
			else
				RET_LINE();
		}
		else
			RET_LINE();
	}
	
	return 0;
}

static int IsSetThisWallet(CRedis *pstRedis, std::string &strOneWalletKey, std::string &strWalletValue)
{
	int iRet = 0;

	iRet = pstRedis->GetKey(strOneWalletKey, strWalletValue);
	if(NO_CONTEXT==iRet || REPLY_NULL==iRet)
	{
		//re-connect
		pstRedis->Close();
		if(0 == pstRedis->Connect())
		{
			if(NULL == pstRedis)
			{
				LOG_ERROR("GetRedis <%s:%s> Error, pid=%u", pstConfig->stMyRedisPoint.ip.c_str(), 
					pstConfig->stMyRedisPoint.port.c_str(), pstConfig->myPid);
				RET_LINE();
			}

			iRet = pstRedis->GetKey(strOneWalletKey, strWalletValue);
			if(0>iRet && REDIS_NO_DATA!=iRet)
			{
				LOG_ERROR("GetWalletValue Error, iRet=%d WalletKey=%s pid=%u", 
				iRet, strOneWalletKey.c_str(), pstConfig->myPid);
				RET_LINE();
			}
		}
	}
	else if(0>iRet && REDIS_NO_DATA!=iRet)
	{
		LOG_ERROR("GetWalletValue Error, iRet=%d WalletKey=%s pid=%u", 
			iRet, strOneWalletKey.c_str(), pstConfig->myPid);
		RET_LINE();
	}
	
	return 0;
}

static int GetOneProfileValue(CRedis *pstRedis, std::string strOneProfileKey, std::string &strPlat, FNF::ptnumber &stPtNum, int *piHasPtNum)
{
	int iRet = 0;
	std::string strOneProfileValue, strOneWalletValue;
	FNF::profile stProfile;

	strOneProfileValue.clear();
	strOneWalletValue.clear();
	stProfile.Clear();

	iRet = pstRedis->GetKey(strOneProfileKey, strOneProfileValue);
	if(NO_CONTEXT==iRet || REPLY_NULL==iRet)
	{
		//re-connect
		pstRedis->Close();
		if(0 == pstRedis->Connect())
		{
			iRet = pstRedis->GetKey(strOneProfileKey, strOneProfileValue);
			if(iRet)
			{
				LOG_ERROR("GetProfileValue Error, iRet=%d profileKey=%s pid=%u", 
					iRet, strOneProfileKey.c_str(), pstConfig->myPid);
				RET_LINE();
			}

			if(!stProfile.ParseFromString(strOneProfileValue))
				RET_LINE();

			if("and" == strPlat)
			{
				if(stProfile.has_and_number())
				{
					stPtNum = stProfile.and_number();
					*piHasPtNum = 1;

					return 0;
				}
				else
				{
					*piHasPtNum = 0;
					return 0;
				}
			}
			else if("ios" == strPlat)
			{
				if(stProfile.has_ios_number())
				{
					stPtNum = stProfile.ios_number();
					*piHasPtNum = 1;

					return 0;
				}
				else
				{
					*piHasPtNum = 0;
					return 0;
				}
			}
		}
		else
		{
			LOG_ERROR("Re-connect Redis Error, <%s:%s> pid=%u", 
				pstConfig->stMyRedisPoint.ip.c_str(), pstConfig->stMyRedisPoint.port.c_str(), pstConfig->myPid);
			RET_LINE();
		}
	}
	else
	{
		if(!stProfile.ParseFromString(strOneProfileValue))
			RET_LINE();

		if("and" == strPlat)
		{
			if(stProfile.has_and_number())
			{
				stPtNum = stProfile.and_number();
				*piHasPtNum = 1;

				return 0;
			}
			else
			{
				*piHasPtNum = 0;
				return 0;
			}
		}
		else if("ios" == strPlat)
		{
			if(stProfile.has_ios_number())
			{
				stPtNum = stProfile.ios_number();
				*piHasPtNum = 1;

				return 0;
			}
			else
			{
				*piHasPtNum = 0;
				return 0;
			}
		}
	}
	
	return 0;
}

static int UpdateOneWallet(CRedis *pstRedis, std::string strOneWalletKey, FNF::ptnumber &stPtNum)
{
	int iRet = 0;
	time_t tCur;
	FNF::wallet stOneWallet;
	std::string strOneWalletValue;

	stOneWallet.Clear();
	strOneWalletValue.clear();

	time(&tCur);

	LOG_ERROR("walletKey=%s gem=%u gem_add=%u gem_del=%u update_time=%u, pid=%u",
		strOneWalletKey.c_str(), stPtNum.gem(), stPtNum.gem_add(), stPtNum.gem_del(),
		tCur, pstConfig->myPid);

	stOneWallet.set_gem(stPtNum.gem());
	stOneWallet.set_gem_add(stPtNum.gem_add());
	stOneWallet.set_gem_del(stPtNum.gem_del());
	stOneWallet.set_update_time((uint32_t)tCur);

	stOneWallet.SerializeToString(&strOneWalletValue);
	iRet = pstRedis->SetKey(strOneWalletKey, strOneWalletValue);
	if(iRet)
	{
		if(NO_CONTEXT == iRet)
		{
			//re-connect
			pstRedis->Close();
			if((iRet=pstRedis->Connect()))
			{
				LOG_ERROR("connect to redis_server Error, iRet=%d <%s:%s> pid=%u", 
					iRet, pstConfig->stMyRedisPoint.ip.c_str(), pstConfig->stMyRedisPoint.port.c_str(), pstConfig->myPid);
				RET_LINE();
			}

			iRet = pstRedis->SetKey(strOneWalletKey, strOneWalletValue);
			if(iRet)
			{
				LOG_ERROR("save data to redis Error Again! iRet=%d walletKey=%s pid=%u", 
					iRet, strOneWalletKey.c_str(), pstConfig->myPid);
				RET_LINE();
			}
		}
		else
			RET_LINE();
	}
	
	return 0;
}

static int TransOneUserGem(CRedis *pstRedis, std::string strOneProfileKey, std::string strOneWalletKey, std::string &strPlat)
{
	int iRet=0, iHasPtNum=0;
	FNF::ptnumber stPtNum;

	stPtNum.Clear();
	iRet = GetOneProfileValue(pstRedis, strOneProfileKey, strPlat, stPtNum, &iHasPtNum);
	if(0 > iRet)
	{
		LOG_ERROR("GetOneProfileValue Error, iRet=%d profileKey=%s pid=%u",
			iRet, strOneProfileKey.c_str(), pstConfig->myPid);
		RET_LINE();
	}
	if(0 == iHasPtNum)
		return 0;

	iRet = UpdateOneWallet(pstRedis, strOneWalletKey, stPtNum);
	if(0 > iRet)
	{
		LOG_ERROR("UpdateOneWallet Error, iRet=%d walletKey=%s pid=%u",
			iRet, strOneWalletKey.c_str(), pstConfig->myPid);
		RET_LINE();
	}
	
	return 0;
}

static int HandleOneProfile(CRedis *pstRedis, std::string strOneProfileKey)
{
	int iRet = 0;
	std::string strUin, strPlat, strOneWalletKey, strOneWalletValue;

	strUin.clear();
	strPlat.clear();
	strOneWalletKey.clear();
	strOneWalletValue.clear();

	iRet = GetUidAndPlatFromProfileKey(strOneProfileKey, strUin, strPlat);
	if(0 > iRet)
	{
		LOG_ERROR("profileKey=%s GetUidAndPlatFromProfileKey Error, iRet=%d pid=%u",
			strOneProfileKey.c_str(), iRet, pstConfig->myPid);
		RET_LINE();
	}

	if("and"!=strPlat && "ios"!=strPlat)
	{
		LOG_ERROR("Invalid plat=%s pid=%u", strPlat.c_str(), pstConfig->myPid);
		RET_LINE();
	}
	
	strOneWalletKey = strPlat+strWalletKey+strUin;
	iRet = IsSetThisWallet(pstRedis, strOneWalletKey, strOneWalletValue);
	if(0 > iRet)
	{
		LOG_ERROR("profileKey=%s IsSetThisWallet Error, iRet=%d pid=%u",
			strOneProfileKey.c_str(), iRet, pstConfig->myPid);
		RET_LINE();
	}

	if(0 == strOneWalletValue.size())
	{
		//need trans
		iRet = TransOneUserGem(pstRedis, strOneProfileKey, strOneWalletKey, strPlat);
		if(0 > iRet)
		{
			LOG_ERROR("TransOneUserGem Error, iRet=%d profileKey=%s walletKey=%s pid=%u", 
				iRet, strOneProfileKey.c_str(), strOneWalletKey.c_str(), pstConfig->myPid);
			RET_LINE();
		}
	}
	
	return 0;
}

static int TransGemEntry()
{
	int iRet = 0;
	CRedis *pstRedis = NULL;
	std::vector<std::string> stProfileKeysVec;
	stProfileKeysVec.clear();
	
	pstRedis = GET_REDIS(pstConfig->stMyRedisPoint.ip, atoi(pstConfig->stMyRedisPoint.port.c_str()));
	if(NULL == pstRedis)
	{
		LOG_ERROR("GetRedis <%s:%s> Error, pid=%u", pstConfig->stMyRedisPoint.ip.c_str(), 
			pstConfig->stMyRedisPoint.port.c_str(), pstConfig->myPid);
		RET_LINE();
	}

	iRet = pstRedis->KEYS(strProfileKey, stProfileKeysVec);
	if(0 > iRet)
	{
		if(NO_CONTEXT==iRet  || REPLY_NULL==iRet)
		{
			//re-connect
			pstRedis->Close();
			if((iRet=pstRedis->Connect()))
			{
				LOG_ERROR("connect to redis_server Error, iRet=%d <%s:%s> pid=%u", 
					iRet, pstConfig->stMyRedisPoint.ip.c_str(), pstConfig->stMyRedisPoint.port.c_str(), pstConfig->myPid);
				RET_LINE();
			}

			iRet = pstRedis->KEYS(strProfileKey, stProfileKeysVec);
			if(0 > iRet)
			{
				LOG_ERROR("<%s:%s> KEYS Error, iRet=%d pid=%u", pstConfig->stMyRedisPoint.ip.c_str(),
					pstConfig->stMyRedisPoint.port.c_str(), iRet, pstConfig->myPid);
					RET_LINE();
			}
		}
		else
		{
			LOG_ERROR("<%s:%s> KEYS Error, iRet=%d pid=%u", pstConfig->stMyRedisPoint.ip.c_str(),
				pstConfig->stMyRedisPoint.port.c_str(), iRet, pstConfig->myPid);
			RET_LINE();
		}
	}

	for(std::vector<std::string>::iterator iter=stProfileKeysVec.begin(); iter!=stProfileKeysVec.end(); iter++)
	{
		iRet = HandleOneProfile(pstRedis, *iter);
		if(0 > iRet)
		{
			LOG_ERROR("HandleOneProfile Error, iRet=%u profileKey=%s pid=%u",
				iRet, (*iter).c_str(), pstConfig->myPid);

			continue;
		}
	}

	return 0;
}

int main(int argc ,char **argv)
{
	int iRet = 0;

	pstConfig = &g_stConfig;

	iRet = Init(argc, argv);
	if(0 > iRet)
	{
		LOG_ERROR("Init Error, iRet=%d", iRet);
		exit(-1);
	}

	iRet = ConnectToRedis();
	if(0 > iRet)
	{
		LOG_ERROR("ConnectToRedis Error, iRet=%d", iRet);
		exit(-1);
	}

	if(RET_PARPROC == iRet)
	{
		LOG_NOTICE("I'm parent, now exit");
		return 0;
	}

	time_t stBegin;
	time(&stBegin);
	LOG_NOTICE("-----------------ChildPid=%u begin=%u ----------------", getpid(), stBegin);

	iRet = TransGemEntry();
	if(0 > iRet)
	{
		LOG_ERROR("TransGemEntry Error, MyRedis(%s:%s), iRet=%d", 
			pstConfig->stMyRedisPoint.ip.c_str(), pstConfig->stMyRedisPoint.port.c_str(), iRet);
		exit(-1);
	}

	time_t stEnd;
	time(&stEnd);
	LOG_NOTICE("-----------------ChildPid=%u end=%u ----------------", getpid(), stEnd);
	
	return 0;
}

