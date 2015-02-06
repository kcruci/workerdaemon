#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>

#include "db/mysql_conf.h"
#include "db/redis_conf.h"
#include "db/keylist_conf.h"
#include "tinyxml.h"
//#include "persis_comm.h"

#include "log/logger.h"
#include "db/redis_wrapper.h"
#include "db/mysql_wrapper.h"
#include "db/mysql_pool.h"
#include "db/redis_pool.h"
#include "logic/fnf_modbitmap.pb.h"

#include "basic/crc.h"
#include "basic/trans.h"
#include "logic/wk_error.h"
#include "logic/wk_define.h"
#include "macrodef.h"

#define MAX_TABLENUM_PER_DB (100)
#define READ_QNODE_MAXNUM 	(100)
#define READ_PER_QUERY (20)
#define SQL_BUFF_SIZE (2048)
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
	char cQRdsCnfPath[256];
	char cCheRdsCnfPath[256];
	char cDbCnfPath[256];
	char cKeyLstCnfPath[256];
	char cLogConfPath[256];

	//(bid, endid) of Redis_Cache
	uint32_t dwBeginId;
	uint32_t dwEndid;

	CMysqlConf DBCnfObj;			//mysql conf obj
	CRedisConf CRdsCnfObj;		//cache redis conf obj
	CKeyListConf KeyLstCnfObj;		//key list conf obj

	std::vector<MYSQLNETPOINT> stMysqlnetPointVec;
	MYSQLNETPOINT stSelfMysqlnetPoint;
	CMysql * pstMysqlHandle;

	uint32_t dwKeysNum;
	std::vector<KEYLISTNODE> stKeyList;
	std::string queenKey;
}ReLoadCacheToolConfig;
#pragma pack()

ReLoadCacheToolConfig g_stConfig, *pstConfig;

static void usage(int argc, char **argv)
{
	printf("usage: <%s> worker_redis_cnf  db_cnf keylist_cnf log_conf cache_bid cache_eid \n", argv[0]);
	printf("sample:<%s> ../conf/fnf_worker_redis.xml ../conf/fnf_worker_mysql.xml ../conf/keylist.xml ../conf/log.conf 1 2\n", argv[0]);

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

	iRet = IsVaildAndCompse(argv[2], pstConfig->cDbCnfPath, sizeof(pstConfig->cDbCnfPath));
	if(0 > iRet)
	{
		printf("IsVaildAndCompse DbCnfPath Error, iRet=%d\n", iRet);
		RET_LINE();
	}

	iRet = IsVaildAndCompse(argv[3], pstConfig->cKeyLstCnfPath, sizeof(pstConfig->cKeyLstCnfPath));
	if(0 > iRet)
	{
		printf("IsVaildAndCompse KeyLstCnfPath Error, iRet=%d\n", iRet);
		RET_LINE();
	}

	memcpy(pstConfig->cLogConfPath, argv[4], sizeof(pstConfig->cLogConfPath));

	pstConfig->dwBeginId = (uint32_t)atoi(argv[5]);
	pstConfig->dwEndid = (uint32_t)atoi(argv[6]);

	if(pstConfig->dwBeginId<0 || pstConfig->dwEndid>100 || pstConfig->dwBeginId>pstConfig->dwEndid)
	{
		printf("bid(%d) or eid(%d) Error\n", pstConfig->dwBeginId, pstConfig->dwEndid);
		RET_LINE();
	}

	return 0;
}

int ParseKeyListInfo(CKeyListConf &keyLstCnf)
{
    int iRet = 0;

    iRet = keyLstCnf.Get(pstConfig->stKeyList);
    if(0 != iRet)
    {
        LOG_ERROR("get keylist item failed, iRet=%d\n", iRet);
        RET_LINE();
    }

    iRet = keyLstCnf.GetQueenKeyName(pstConfig->queenKey);
    if(0 != iRet)
    {
        LOG_ERROR("get queen nake failed, iRet=%d\n", iRet);
        RET_LINE();
    }

    pstConfig->dwKeysNum = pstConfig->stKeyList.size();

    LOG_DEBUG("key num,keylist.size():%d in keylist ", pstConfig->stKeyList.size());

    return 0;
}

//-1--error, 0--cannot select, 1--can select
static int CanSelectMysql(int iCmdBid, int iCmdEid, int iMysqlConfBid, int iMysqlConfEid)
{
	if(iMysqlConfBid > iMysqlConfEid)
		return -1;

	if(iCmdBid>iMysqlConfEid || iCmdEid<iMysqlConfBid)
		return 0;

	return 1;
}

int GetMysqlHostInfo(const std::string& strFilePath)
{
	int bid=0, eid=0, iRet=0;

	TiXmlDocument doc(strFilePath);
	bool loadOkay = doc.LoadFile();
	if (loadOkay == false)
		return ERR_WK_LOAD_CONF;
	TiXmlElement* root = doc.RootElement();
    TiXmlNode* useritem = root->FirstChild("user");
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

        bid = CTrans::STOI(dbelement->Attribute("bid"));
        eid = CTrans::STOI(dbelement->Attribute("eid"));

        iRet = CanSelectMysql((int)pstConfig->dwBeginId, (int)pstConfig->dwEndid, bid, eid);
        if(0 > iRet)
        {
        	LOG_ERROR("ERROR: bid(%d)>eid(%d)", bid, eid);
        	continue;
        }
        else if(0 == iRet)
        	continue;

        MYSQLNETPOINT net;
        net.ip = dbelement->Attribute("ip");
        net.port= CTrans::STOI(dbelement->Attribute("port"));
        net.dbname= dbelement->Attribute("dbname");
        net.tblprefix= dbelement->Attribute("tbl");
        net.user= dbelement->Attribute("user");
        net.pwd= dbelement->Attribute("password");
        
       	pstConfig->stMysqlnetPointVec.push_back(net);

       	LOG_DEBUG("GetDBNet: ip=%s, port=%u, dbname=%s, tblprefix=%s, user=%s, password=%s", 
       		net.ip.c_str(), net.port, net.dbname.c_str(), net.tblprefix.c_str(), net.user.c_str(), net.pwd.c_str());
	}

	return 0;
}

static int LoadConfig()
{
	RET_TRUE_LINE(pstConfig->CRdsCnfObj.LoadConf(pstConfig->cCheRdsCnfPath));
	RET_TRUE_LINE(pstConfig->KeyLstCnfObj.LoadConf(pstConfig->cKeyLstCnfPath));
	RET_TRUE_LINE(GetMysqlHostInfo(pstConfig->cDbCnfPath));

	return 0;
}

static int Init(int argc, char **argv)
{
	int iRet = 0;

	if(argc < 7)
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

	iRet = LoadConfig();
	if(0 > iRet)
	{
		printf("LoadConfig Error, iRet=%d\n", iRet);
		RET_LINE();
	}

	INIT_LOG(LOG_DIR, "ReBuildCacheTool.log", pstConfig->cLogConfPath);

	iRet = ParseKeyListInfo(pstConfig->KeyLstCnfObj);
	if(0 > iRet)
	{
		LOG_ERROR("ParseKeyListInfo Error, iRet=%d", iRet);
		RET_LINE();
	}

	return 0;
}

static int GetSelfMysqlnetPoint()
{
	for(std::vector<MYSQLNETPOINT>::iterator iter=pstConfig->stMysqlnetPointVec.begin();
		iter!=pstConfig->stMysqlnetPointVec.end();
		iter ++)
	{
		pid_t pid = fork();
		if(0 > pid)
		{
			//error
			LOG_ERROR("fork Error, iRet=%d", pid);
			exit(1);
		}
		else if(0 == pid)
		{
			//child
			pstConfig->stSelfMysqlnetPoint.ip = iter->ip;
			pstConfig->stSelfMysqlnetPoint.port = iter->port;
			pstConfig->stSelfMysqlnetPoint.dbname = iter->dbname;
			pstConfig->stSelfMysqlnetPoint.tblprefix = iter->tblprefix;
			pstConfig->stSelfMysqlnetPoint.user = iter->user;
			pstConfig->stSelfMysqlnetPoint.pwd = iter->pwd;

			LOG_DEBUG("SelfDBInfo: ip=%s, port=%u, dbname=%s, tblprefix=%s, user=%s, pwd=%s", 
				pstConfig->stSelfMysqlnetPoint.ip.c_str(), pstConfig->stSelfMysqlnetPoint.port,
				pstConfig->stSelfMysqlnetPoint.dbname.c_str(), pstConfig->stSelfMysqlnetPoint.tblprefix.c_str(), 
				pstConfig->stSelfMysqlnetPoint.user.c_str(), pstConfig->stSelfMysqlnetPoint.pwd.c_str());

			return RET_CHPROC;
		}
		else
		{
			//parent
			continue;
		}
	}

	return RET_PARPROC;
}

static int GetOneDBTableName(int iTableIndex, char *cTableName, int iTableNameLen)
{
	if(MAX_TABLENUM_PER_DB < iTableIndex)
		RET_LINE();

	snprintf(cTableName, iTableNameLen, "%s.%s%d", 
		pstConfig->stSelfMysqlnetPoint.dbname.c_str(), 
		pstConfig->stSelfMysqlnetPoint.tblprefix.c_str(), iTableIndex);

	return 0;
}

static unsigned int GetUidCrc32(uint64_t ddwUin)
{
    //need to check crc32(uid);
    std::string sAccUid = CTrans::ITOS(ddwUin);
    unsigned int iCrcKeyUid= load_crc32(0,sAccUid.c_str(),sAccUid.size());
    return iCrcKeyUid;
}

static int HandleOneUin(uint64_t ddwUin, std::vector<std::string> &stColNameVec, char *cDBTableName)
{
	int iRet=0, i=0, j=0, retRows=0;
	static std::string querySql;
	unsigned long *lengths;
	unsigned int iCrcKeyUid = GetUidCrc32(ddwUin);

	int iRedisIndex = iCrcKeyUid%100;
	if(iRedisIndex<pstConfig->dwBeginId || iRedisIndex>pstConfig->dwEndid)
		return 0;

//	LOG_DEBUG("---------------uin=%lu", ddwUin);

	NETPOINT cIpPort;
	iRet = pstConfig->CRdsCnfObj.GetUserIpPort(iCrcKeyUid, cIpPort);
    if(0 != iRet)
	{
		LOG_ERROR("get cache ip error. uid:%llu", ddwUin);
		RET_LINE();
	}

	CRedis *pstRdsHdl = GET_REDIS(cIpPort.ip, atoi(cIpPort.port.c_str()));
	if(!pstRdsHdl)
	{
		LOG_ERROR("GET_REDIS Error, RedisHost(%s:%s)", cIpPort.ip.c_str(), cIpPort.port.c_str());
		RET_LINE();
	}

	
   	pstConfig->pstMysqlHandle->InitSqlString(querySql, "select uid,plat");
    for(std::vector<std::string>::iterator iter=stColNameVec.begin(); iter!=stColNameVec.end(); iter++)
    {
    	pstConfig->pstMysqlHandle->AppendSqlString(querySql, ",");
    	pstConfig->pstMysqlHandle->AppendSqlString(querySql, (*iter).c_str());
    }
    pstConfig->pstMysqlHandle->AppendSqlString(querySql, " from ");
    pstConfig->pstMysqlHandle->AppendSqlString(querySql, cDBTableName);
    pstConfig->pstMysqlHandle->AppendSqlString(querySql, " where uid=");
    pstConfig->pstMysqlHandle->AppendSqlString(querySql, CTrans::ITOS(ddwUin).c_str());
    
//	LOG_DEBUG("querySql=%s", querySql.c_str());

	if( false == pstConfig->pstMysqlHandle->Query(querySql.data(), querySql.size()) )
    {
       	LOG_ERROR("select sql:[%s] failed, msg:%s",querySql.c_str(), pstConfig->pstMysqlHandle->GetError());
        RET_LINE();
    }
    ResultSet result;
    pstConfig->pstMysqlHandle->FetchResult(result);
    retRows = result.GetRowsNum();

    for(i=0; i<retRows; i++)
    {
    	if(true == result.FetchRow())
    	{
    		MYSQL_ROW stRow = result.GetOneRow();
    		int iPlat = atoi(stRow[1]);
			std::string platStr;
			if( ::FNF::PLAT_AND == iPlat)
			{
				platStr="and:";
			}
			else if( ::FNF::PLAT_IOS == iPlat)
			{
				platStr="ios:";
			}

			j = 0;
			lengths = result.FetchLengths();
			FOR_EACH(itKeyList,  pstConfig->stKeyList)
			{
				if(NULL == stRow[2+j])
					RET_LINE();

				std::string ckey = platStr + itKeyList->cacheKeyPrefix + CTrans::ITOS(ddwUin);
				std::string data = "";
				pstConfig->pstMysqlHandle->AppendSqlEscapeString(data, stRow[2+j], lengths[2+j]);

				pstRdsHdl->SetKey(ckey, data);

//				LOG_DEBUG("key=%s data=%s datalen=%u", ckey.c_str(), stRow[2+j], lengths[2+j]);

				j++;
			}
    	}
    }

	return 0;
}

static int ConnectToSelfMysqlnetPoint()
{
	if(0==pstConfig->stSelfMysqlnetPoint.ip.size() || 0==pstConfig->stSelfMysqlnetPoint.port ||
		0==pstConfig->stSelfMysqlnetPoint.user.size())
	{
		RET_LINE();
	}

	pstConfig->pstMysqlHandle = GET_MYSQLHANDLE(pstConfig->stSelfMysqlnetPoint.user, pstConfig->stSelfMysqlnetPoint.pwd, 
							pstConfig->stSelfMysqlnetPoint.ip, pstConfig->stSelfMysqlnetPoint.port);
	if(!pstConfig->pstMysqlHandle)
	{
		LOG_ERROR("GET_MYSQLHANDLE Error");
		RET_LINE();
	}

	return 0;
}

static int ReBuildRedisCache()
{
	int i=0, j=0, iRet=0;
	char cDBTableName[128] = {0};
	static std::string querySql;
	unsigned int retRows=0;
	uint64_t ddwUin = 0;

	std::vector<std::string> stColNameVec;
	stColNameVec.clear();
	FOR_EACH(itKeyList,  pstConfig->stKeyList)
    {
    	std::string ckey = itKeyList->mysqlColname;
    	stColNameVec.push_back(ckey);
        LOG_DEBUG("dbcolname:[%s]",ckey.c_str());
    }

	for(i=0; i<MAX_TABLENUM_PER_DB; i++)
	{
		memset(cDBTableName, 0, sizeof(cDBTableName));

		iRet = GetOneDBTableName(i, cDBTableName, sizeof(cDBTableName));
		if(0 > iRet)
		{
			LOG_ERROR("GetOneDBTableName Error, dbname=%s, TableIndex=%d, iRet=%d", 
				pstConfig->stSelfMysqlnetPoint.dbname.c_str(), i, iRet);
			continue;
		}

    	pstConfig->pstMysqlHandle->InitSqlString(querySql, "select distinct uid from ");
    	pstConfig->pstMysqlHandle->AppendSqlString(querySql, cDBTableName);

    	if( false == pstConfig->pstMysqlHandle->Query(querySql.data(), querySql.size()) )
    	{
       		LOG_ERROR("select sql:[%s] failed, msg:%s",querySql.c_str(), pstConfig->pstMysqlHandle->GetError());
        	continue;
    	}

    	ResultSet result;
    	pstConfig->pstMysqlHandle->FetchResult(result);
    	retRows = result.GetRowsNum();

    	for(j=0; j<retRows; j++)
    	{
    		if(true == result.FetchRow())
    		{
    			MYSQL_ROW stRow = result.GetOneRow();
    			ddwUin = strtoul(stRow[0], 0, 10);

    			iRet = HandleOneUin(ddwUin, stColNameVec, cDBTableName);
    			if(0 > iRet)
    			{
    				LOG_ERROR("HandleOneUin Error, Uin=%lu, iRet=%d", ddwUin, iRet);
    				continue;
    			}
    		}
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
		LOG_ERROR("InitLocalConfig Error, iRet=%d", iRet);
		exit(-1);
	}

	iRet = GetSelfMysqlnetPoint();
	if(0 > iRet)
	{
		LOG_ERROR("ConnectToMysqlHost Error, iRet=%d", iRet);
		exit(-1);
	}

	if(RET_PARPROC == iRet)
	{
		LOG_NOTICE("I'm parent, end now.... ");
		return 0;
	}

	//parent fork child when child cored;
	#if 0
	iRet = InitDaemon();
	if( 0 > iRet)
	{
		LOG_ERROR("InitDaemon Error, iRe=%d", iRet);
		exit(-1);
	}
	#endif

	time_t stBegin;
	time(&stBegin);
	LOG_NOTICE("-----------------ChildPid=%u begin=%u ----------------", getpid(), stBegin);

	iRet = ConnectToSelfMysqlnetPoint();
	if(0 > iRet)
	{
		LOG_ERROR("ConnectToSelfMysqlnetPoint Error, MysqlHost(%s:%u), iRet=%d",
			pstConfig->stSelfMysqlnetPoint.ip.c_str(), pstConfig->stSelfMysqlnetPoint.port, iRet);
		exit(-1);
	}

	iRet = ReBuildRedisCache();
	if(0 > iRet)
	{
		LOG_ERROR("ReBuildRedisCache Error, MysqlHost(%s:%u), iRet=%d", 
			pstConfig->stSelfMysqlnetPoint.ip.c_str(), pstConfig->stSelfMysqlnetPoint.port, iRet);
		exit(-1);
	}

	time_t stEnd;
	time(&stEnd);
	LOG_NOTICE("-----------------ChildPid=%u end=%u ----------------", getpid(), stEnd);
	
	return 0;
}

