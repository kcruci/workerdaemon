#include "persis_comm.h"
#include <sys/file.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include "macrodef.h"

using namespace std;
// one key response one queue....

#define LOG_DIR "/data/logs/workerserver/"
//define the global val for the dameon
//
extern PERSISST g_persDat, *p_persDat;


#define SQL_BUFF_SIZE 2048
// check and make sure the daemon is single proc..
#define RESET_TM(pDat) do {\
    (pDat)->iTmOutFlg = NOTMOUT;\
    \
}while(0)

static inline unsigned int GetLocalTm( )
{
	return time(NULL);
	#if 0
    unsigned long long iRet = 0;
    struct timeval tt;
    if( -1 == gettimeofday(&tt,NULL))
    {
        return -1;
    }
    iRet = tt.tv_sec*1000*1000 +tt.tv_usec;
    return iRet;
    #endif
}

int IsSingle( char *psProcName )
{
    int iRet		 		= RetSucc;
    char buf[512] 			= {'\0'};
    char sLockName[512] 	= {'\0'};
    char * bname 			= NULL;
    if( psProcName == NULL )
    {
        LOG_ERROR("input para invailed ");
        iRet = RetErr;
        return iRet;
    }

    memset(buf, 0, sizeof(buf));
    memcpy(buf, psProcName, strlen(psProcName));
    bname = basename(buf);

    memset(sLockName, 0, sizeof(sLockName));
    snprintf(sLockName,sizeof(sLockName), "/tmp/%s_0.lock",bname);
    int fd = open(sLockName, O_CREAT, 400);
    if (fd < 0 )
    {
        LOG_ERROR("open and create file:%s failed, as:%s ",sLockName,ErrStr);
        iRet = RetErr;
        return iRet;
    }

    if (flock(fd, LOCK_EX|LOCK_NB) < 0)
    {
        LOG_ERROR("flock <0 , maybe locked or error %s", ErrStr);
        iRet = RetErr;
        return iRet;
    }
    return iRet;
}
// check the conf path can be access and compose absolute dir...
static int IsVaildAndCompse(const char *psCnfPath, char *psFinalPath, int iFinPathLen)
{
    int iRet = RetSucc;
    char tmpPath[256];

    if( psCnfPath == NULL || psFinalPath == NULL || iFinPathLen <= 0 )
    {
        LOG_ERROR("input invailed ");
        iRet = RetErr;
        return iRet;
    }
    if( access(psCnfPath,F_OK)<0 )
    {
        LOG_ERROR("cnf: %s can't access,as:%s",psCnfPath, ErrStr);
        iRet = RetErr;
        return iRet;
    }
    if( psCnfPath[0] != '/' )
    {
        if( getcwd(tmpPath,sizeof(tmpPath)) == NULL )
        {
            LOG_ERROR("get cur dir failed,as:%s",ErrStr);
            iRet = RetErr;
            return iRet;
        }
        snprintf(psFinalPath,iFinPathLen,"%s/%s",tmpPath,psCnfPath);
    }
    else
    {
        snprintf(psFinalPath,iFinPathLen,"%s",psCnfPath);
    }
    LOG_DEBUG("final conf path:[%s]",psFinalPath);
    return iRet;
}

//just set the tmout flag to tmout.
void CallBkFuncExpire(unsigned int iDatLen, char *stDat)
{
    p_persDat->iTmOutFlg = TMOUT;
    LOG_DEBUG("proccessId: [%u] tm out, call tm_callback_func",CallProcId);
    return ;
}

int PraseKeyListInfo(CKeyListConf &keyLstCnf)
{
    int iRet = RetSucc;
    std::vector<KEYLISTNODE> ::iterator  iter;

    iRet = keyLstCnf.Get(p_persDat->KeyListItem);
    if(iRet != RetSucc )
    {
        LOG_ERROR("get keylist item failed \n");
        iRet = RetErr;
        return iRet;
    }

    iRet = keyLstCnf.GetQueenKeyName(p_persDat->queenKey);
    if(iRet != RetSucc )
    {
        LOG_ERROR("get queen nake failed \n");
        iRet = RetErr;
        return iRet;
    }

    p_persDat->iKeysNum  =p_persDat->KeyListItem.size();

    LOG_DEBUG("key num,keylist.size():%d in keylist ",p_persDat->KeyListItem.size());
    return iRet;
}

inline  int ParseProcNum(char *psPrNum,int iQRdsCnfIpPrtNm)
{
    int iRet = RetSucc;
    char cdelmiter = '-';
    char tmpBuf[128];

    if( psPrNum == NULL || iQRdsCnfIpPrtNm <= 0 )
    {
        LOG_ERROR("input para invailed ");
        iRet = RetErr;
        return iRet;
    }

    char *tmpStr = psPrNum;
    memset(tmpBuf,0,sizeof(tmpBuf));

    while( tmpStr != '\0' )
    {
        if (*tmpStr == cdelmiter )
            break;

        tmpStr++;
    }
    if( tmpStr != '\0' )
    {
        p_persDat->iProcEIdx = atoi(tmpStr+1);
        memcpy(tmpBuf,psPrNum,tmpStr-psPrNum);
        p_persDat->iProcBIdx = atoi(tmpBuf);
        LOG_DEBUG("parsed proc_bgin_no:%d, proc_end_no: %d ",p_persDat->iProcBIdx,p_persDat->iProcEIdx);
    }
    else
    {
        LOG_ERROR("Proc_num_str:[%s] invailed ",psPrNum);
        iRet = RetErr;
        return iRet;
    }
    if( p_persDat->iProcEIdx <p_persDat->iProcBIdx )
    {
        LOG_ERROR("proc_bgin_indx:[%d] > proc_end_indx:[%d] invailed ",p_persDat->iProcBIdx,p_persDat->iProcEIdx );
        iRet = RetErr;
        return iRet;
    }
    if(p_persDat->iProcBIdx >iQRdsCnfIpPrtNm )
    {
        LOG_ERROR("bgin_indx:[%d] > cnf_IpPortNum:[%d], invaild ",p_persDat->iProcBIdx, iQRdsCnfIpPrtNm);
        iRet = RetErr;
        return iRet;
    }
    if( p_persDat->iProcEIdx > iQRdsCnfIpPrtNm )
    {
        LOG_DEBUG("set end_indx:[%d] not :[%d]",iQRdsCnfIpPrtNm,p_persDat->iProcEIdx);
        p_persDat->iProcEIdx =  iQRdsCnfIpPrtNm ;
    }
    return iRet;
}

// init/read conf
int InitObj(PERSISST *pstPsisDat, int argc, char**argv)
{
    int iRet = RetSucc;
    int iQRdsIpPortNum ;
    std::vector<NETPOINT> QRdsIpPortStr;

    if( argc != 8 )
    {
        printf("usage: <%s> worker_redis_cnf  db_cnf keylist_cnf log_conf  Scantm(s) Expiretm(m) ProcNum[N1-N2] \n",argv[0]);
        printf("sample:<%s> ../conf/fnf_worker_redis.xml ../conf/fnf_worker_mysql.xml ../conf/keylist.xml ../conf/log.conf 1 2 1-10 \n",argv[0]);
        iRet = RetErr;
        return iRet;
    }

    INIT_LOG(LOG_DIR,"loadcache.log",argv[4]);

    iRet = IsVaildAndCompse(argv[1],pstPsisDat->sQRdsCnfPath, sizeof(pstPsisDat->sQRdsCnfPath));
    if( iRet == RetErr )
    {
        LOG_ERROR("get query_queue redis cnf path failed");
        goto cnferr;
    }
    LOG_DEBUG("Get query_queue_redis_svr conf path succ ! ");

    iRet = IsVaildAndCompse(argv[1],pstPsisDat->sCheRdsCnfPath, sizeof(pstPsisDat->sCheRdsCnfPath));
    if( iRet == RetErr )
    {
        LOG_ERROR("get cache redis cnf path failed ");
        goto cnferr;
    }
    LOG_DEBUG("Get Cache redis_svr cnf path succ !!!!");

    iRet = IsVaildAndCompse(argv[2], pstPsisDat->sDbCnfPath,sizeof(pstPsisDat->sDbCnfPath));
    if( iRet == RetErr )
    {
        LOG_ERROR("get mysql cnf path failed");
        goto cnferr;
    }
    LOG_DEBUG("Get Mysql_db cnf path succ !!!!");

    iRet = IsVaildAndCompse(argv[3], pstPsisDat->sKeyLstCnfPath,sizeof(pstPsisDat->sKeyLstCnfPath));
    if( iRet == RetErr )
    {
        LOG_ERROR("get keylist cnf path failed ");
        goto cnferr;
    }
    LOG_DEBUG("get keylist cnf path succ !!!!");

    pstPsisDat->iTmOutVal = atoi(argv[5]); /** senconds level ***/
    pstPsisDat->iPerTmVal = atoi(argv[6]);  //*60*1000*1000; /** senconds level***/
    pstPsisDat->iTmOutFlg = NOTMOUT;
    pstPsisDat->iSessSeq  = 0;
    pstPsisDat->TmOutCallBackFunc=CallBkFuncExpire;//TODO::need to define callbackfunc.
    //for each obj,load cnf file....
    //cache conf
    iRet = pstPsisDat->PisCRdsCnfObj.LoadConf(pstPsisDat->sCheRdsCnfPath);
    if( iRet != RetSucc )
    {
        LOG_ERROR("load cache redis conf failed");
        iRet = RetErr;
        return iRet;
    }
    //db conf
    iRet = pstPsisDat->PisDBCnfObj.LoadConf(pstPsisDat->sDbCnfPath);
    if( iRet != RetSucc )
    {
        LOG_ERROR("Load db conf failed ");
        iRet = RetErr;
        return iRet;
    }
    //query queue conf
    iRet = pstPsisDat->PisQRdsCnfObj.LoadConf(pstPsisDat->sQRdsCnfPath);
    if( iRet != RetSucc )
    {
        LOG_ERROR("load query queue conf failed ");
        iRet = RetErr;
        return iRet;
    }
    iRet = pstPsisDat->KeyLstCnfObj.LoadConf(pstPsisDat->sKeyLstCnfPath);
    if( iRet != RetSucc )
    {
        LOG_ERROR("load keylist conf failed ");
        iRet = RetErr;
        return iRet;
    }

    // init keynum val....
    pstPsisDat->iKeysNum = 0;
    iRet = PraseKeyListInfo( pstPsisDat->KeyLstCnfObj );
    if( iRet != RetSucc )
    {
        LOG_ERROR("statistic the key num in keylist failed ");
        iRet = RetErr;
        return iRet;
    }

    //parse the process_begin_no and process_end_no:::
    iRet = 	pstPsisDat->PisQRdsCnfObj.GetQueenAllIpPort( QRdsIpPortStr );
    iQRdsIpPortNum = QRdsIpPortStr.size();
    if( iQRdsIpPortNum <= 0 )
    {
        LOG_DEBUG("<ip,port> num[%d] for query_queue_redis_cnf is invaild ",iQRdsIpPortNum);
        iRet = RetErr;
    }
    LOG_DEBUG("<ip,port> num: %d for Queue_query_redis",iQRdsIpPortNum);

    iRet = ParseProcNum(argv[7], iQRdsIpPortNum );
    if( iRet != RetSucc )
    {
        LOG_ERROR("Parse process num from argv[6]:%s failed ",argv[6]);
        iRet = RetErr;
        goto cnferr;
    }
    LOG_NOTICE("[InitObj funcall succ] !!!!");
    LOG_NOTICE("the svr run process num:[%d-%d]",pstPsisDat->iProcBIdx, pstPsisDat->iProcEIdx);
cnferr:
    return iRet;
}

//daemon_init
int InitDaemon()
{
    int st;
    int iRet = RetSucc;
    pid_t tpid, mpid;

    /* shield some signals */
    signal(SIGINT,  SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    if( -1 == daemon(1,1) )
    {
        LOG_ERROR("daemon failed as: %s \n",ErrStr);
        iRet = RetErr;
        return iRet;
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
// connto query redis svr
// create conn_pool...
int ConnToQRdisSvr( )
{
    int iRet = RetSucc;
    std::vector<NETPOINT>::iterator iter;
    int iPort = 0;
    //get <ip,port>
    //for each<ip,port> create conn_pool..
    iRet = p_persDat->PisQRdsCnfObj.GetQueenAllIpPort(p_persDat->QIpPort);
    if( iRet != RetSucc )
    {
        LOG_ERROR("get query_queue <ip,port> failed ");
        iRet = RetErr;
        return iRet;
    }
    LOG_DEBUG("query_queue_ipport_num_cnf: %d ",p_persDat->QIpPort.size());
    for( int iPidNm = p_persDat->iProcBIdx-1; iPidNm < p_persDat->iProcEIdx; iPidNm ++ )
    {
        iPort =atoi((p_persDat->QIpPort[iPidNm]).port.c_str());
        std::string tmpIp = (p_persDat->QIpPort[iPidNm]).ip;
	  std::string strPasswd = (p_persDat->QIpPort[iPidNm]).passwd;
        int iProcId = fork( );
        if( iProcId == 0 )
        {
            //child process..
            if( NULL == GET_REDIS_AUTH(tmpIp, iPort, strPasswd) )
            {
                LOG_ERROR("conn redis_conn_svr failed by <ip,port,pwd>[%s,%d,%s] ",tmpIp.c_str(),iPort, strPasswd.c_str());
                iRet = RetErr;
                return iRet;
            }
            p_persDat->stOnePrcssQIpPort = p_persDat->QIpPort[iPidNm];
            p_persDat->usCallPid = getpid();
            p_persDat->usParCallPid = getppid();
            LOG_DEBUG("this is chld proc,it's pid:[%u]",p_persDat->usCallPid);
            LOG_NOTICE("start child worker succ! queen:<ip,port,pwd>[%s:%d:%s],by processId:[%u]",\
                tmpIp.c_str(),iPort, strPasswd.c_str() , CallProcId);
            return RET_CHPROC;
        }
        else if (iProcId >0 ) //parent proccess..
        {
            LOG_DEBUG("Create child process_pid: %lld ",iProcId);
            continue;
        }
        else
        {
            return RetErr;
        }
    }
    p_persDat->usCallPid = getpid();
    LOG_DEBUG("main_process  pid: [%u]",p_persDat->usCallPid);
    LOG_NOTICE("child process num:[%d]",p_persDat->iProcEIdx-p_persDat->iProcBIdx+1);
    return RET_PARPROC; //parent return ...

}

static unsigned int GetUidCrc32(unsigned long long udid)
{
    //need to check crc32(uid);
    std::string sAccUid = CTrans::ITOS(udid);
    unsigned int iCrcKeyUid= load_crc32(0,sAccUid.c_str(),sAccUid.size());
    return iCrcKeyUid;
}

int GetRedisIpPort(long long udid, NETPOINT& cIpPort)
{
    unsigned int iCrcKeyUid = GetUidCrc32(udid);

    int iRet = p_persDat->PisCRdsCnfObj.GetUserIpPort(iCrcKeyUid, cIpPort);
    if(0 != iRet)
	{
		LOG_ERROR("get cache ip error. uid:%lld", udid);
		return iRet;
	}
	LOG_DEBUG("cache redis <ip,port>[%s:%s] get by crc32:[%u],proccessId:[%u] ",\
        cIpPort.ip.c_str(),cIpPort.port.c_str(), iCrcKeyUid,CallProcId);

	return 0;
}

int GetMysqlIpPort(long long udid, MYSQLNETPOINT & dbCnfDat)
{
	unsigned int iCrcUid = GetUidCrc32(udid);

    LOG_DEBUG("udid:[%lld], db_uid(crc32):[%u],proccessId:[%u]",udid,iCrcUid,CallProcId);

    int iRet = p_persDat->PisDBCnfObj.GetIpPort(iCrcUid,dbCnfDat);
    if( iRet != RetSucc )
    {
        LOG_ERROR("get db conf dat failed, ret:%d, crc32(%lld)=[%u], proccessId:[%u]", iRet, udid, iCrcUid,CallProcId);
        iRet = RetErr;
        return iRet;
    }
    return 0;
}
/**********************************************************************
 * Function         : LoadKeyCache
 * Description      : load key cache val by key and udid from cache_redis
 * Calls            : others
 * Input            : key,udid
 * Output           : val
 * Return           : 0:succ, !0:failed
 * Others           : NO
 ************************************************************************/
int LoadKeyCache( long long udid, ::FNF::EM_PLAT emPlat, std::vector<std::string>& vecData)
{
	std::vector<std::string> vecKey;
	vecData.clear();

    //platform independ
    //and ios
    std::string platStr;
    if(emPlat == ::FNF::PLAT_AND)
    {
        platStr="and:";
    }
    else if(emPlat == ::FNF::PLAT_IOS)
    {
        platStr="ios:";
    }

    int iRet = 0;
    int iPort  = 0;
    CRedis *cacheRedis = NULL;
    NETPOINT cIpPort;

    iRet = GetRedisIpPort(udid, cIpPort);
    if( iRet != RetSucc )
    {
        LOG_ERROR("get <ip,port> by crc:%lld ",udid);
        iRet = RetErr;
        return iRet;
    }

    iPort = atoi(cIpPort.port.c_str());

    //get cache connected redis by ip and port..
    cacheRedis = GET_REDIS_AUTH(cIpPort.ip,iPort, cIpPort.passwd);
    if( cacheRedis == NULL )
    {
        LOG_ERROR("get cache redis failed,proccessId:[%u]",CallProcId);
        iRet = RetErr;
        return iRet;
    }
    LOG_DEBUG("conn cache_redis<ip,port,pwd>[%s:%d:%s] succ,proccessId:[%u]",cIpPort.ip.c_str(),iPort,cIpPort.passwd.c_str(),CallProcId);
    // get val by key....

    //p_persDat->KeyListItem[iKeyNum].cacheKeyPrefix;
    //p_persDat->KeyListItem[iKeyNum].mysqlColname;


    FOR_EACH(itKeyList,  p_persDat->KeyListItem)
    {
    	std::string ckey = platStr + itKeyList->cacheKeyPrefix + CTrans::ITOS(udid);
    	vecKey.push_back(ckey);
        LOG_DEBUG("cacheKey:[%s],proccessId:[%u] ",ckey.c_str(),CallProcId);
    }

    //LOG_DEBUG("db conlumName:[%s], proccessId:[%u]",dbKeys.c_str(),CallProcId);

    iRet = cacheRedis->MGetKey(vecKey,vecData);
    if( iRet != RetSucc )
    {
        LOG_ERROR("MGetKey redis cache failed,proccessId:[%u]",CallProcId);
        iRet = RetErr;
        return iRet;
    }
    if(vecKey.size() != vecData.size())
    {
        LOG_ERROR("MGetKey result not match,proccessId:[%u]",CallProcId);
        iRet = RetErr;
        return iRet;
    }

    LOG_DEBUG("LoadCache uid:[%lld],  proccesId:[%u]",udid,CallProcId);
    // connect db and send the key val to db(update or insert).
    return iRet;
}


static char * GetDbTableName(unsigned long long uuid, const MYSQLNETPOINT& dbCnfDat)
{
    unsigned  int iCrcUid = GetUidCrc32(uuid);
    int iFlag = 0;
    iFlag = (iCrcUid/100)%100;
    static char sDbTabName[128];
    /** 12345; 23 is databasename, 45 tablename **/
    snprintf(sDbTabName,sizeof(sDbTabName),"%s.%s%d",dbCnfDat.dbname.c_str(),dbCnfDat.tblprefix.c_str(),iFlag);
    return sDbTabName;
}
/**********************************************************************
 * Function         : SndDatToDb
 * Description      : conn_myql_db by cnf, query the key in db,
 * 					: if not exist,insert the key's val,else update val
 * Calls            : others
 * Input            : key,udid, val
 * Output           :
 * Return           : 0:succ, !0:failed
 * Others           : NO
 ************************************************************************/
int SndDatToDb(long long udid, ::FNF::EM_PLAT emPlat,  std::vector<std::string>& vecData)
{
	std::vector<std::string> vecColName;
	FOR_EACH(itKeyList,  p_persDat->KeyListItem)
    {
    	std::string ckey = itKeyList->mysqlColname;
    	vecColName.push_back(ckey);
        LOG_DEBUG("dbcolname:[%s],proccessId:[%u] ",ckey.c_str(),CallProcId);
    }


	if(vecColName.empty()||
		vecColName.size() != vecData.size())
	{
		LOG_ERROR("col count:%u, datasize:%u", vecColName.size(), vecData.size());
		return -10;
	}

    int iRet = RetSucc;
    MYSQLNETPOINT dbCnfDat;
    // get db conf:

	iRet =  GetMysqlIpPort(udid, dbCnfDat);
	if(iRet != 0)
	{
	 	return iRet;
	}

    CMysql * connDbDat = GET_MYSQLHANDLE(dbCnfDat.user, dbCnfDat.pwd, dbCnfDat.ip, dbCnfDat.port);

    //query the db..if exist then update the item, else
    char * sDbTabName= GetDbTableName(udid, dbCnfDat);

	//select * from db.table ;
    //if not exist, then insert
    //else update
    static std::string querySql;
    querySql.reserve(SQL_BUFF_SIZE);

    connDbDat->InitSqlString(querySql, "select uid from ");
    connDbDat->AppendSqlString(querySql, sDbTabName);
    connDbDat->AppendSqlString(querySql, " where uid=");
    connDbDat->AppendSqlString(querySql, CTrans::ITOS(udid).c_str());
	connDbDat->AppendSqlString(querySql, " and plat=");
    connDbDat->AppendSqlString(querySql, CTrans::ITOS(emPlat).c_str());

    LOG_DEBUG("sql: [%s],proccessId:[%u]",querySql.c_str(),CallProcId);
    if( false == connDbDat->Query(querySql.data(),querySql.size()) )
    {
        LOG_ERROR("select sql:[%s] failed, msg:%s, proccessId:[%u]",querySql.c_str(), connDbDat->GetError(), CallProcId);
        iRet = RetErr;
        return iRet;
    }
    ResultSet result;
    connDbDat->FetchResult(result);

    unsigned int retRows = 0;
    retRows = result.GetRowsNum();

	//TODO sync all to database;
	// not exist udid in database;
    if( retRows != 0 )//update
    {
    	static std::string updateSql;
        //update the tm...or ..

        connDbDat->InitSqlString(updateSql, "update ");
        connDbDat->AppendSqlString(updateSql, sDbTabName);
        connDbDat->AppendSqlString(updateSql, " set ");

		for(size_t i = 0; i < vecColName.size(); i++)
		{
			if(i == 0 )
			{
				connDbDat->AppendSqlString(updateSql, vecColName[i].c_str());
				connDbDat->AppendSqlString(updateSql, " = \'");
				connDbDat->AppendSqlEscapeString(updateSql, vecData[i].data(), vecData[i].size());
			}
			else
			{
                connDbDat->AppendSqlString(updateSql, "\', ");
                connDbDat->AppendSqlString(updateSql, vecColName[i].c_str());
				connDbDat->AppendSqlString(updateSql, " = \'");
				connDbDat->AppendSqlEscapeString(updateSql, vecData[i].data(), vecData[i].size());
			}
		}

        connDbDat->AppendSqlString(updateSql, "\', update_time=");
        connDbDat->AppendSqlString(updateSql, CTrans::ITOS(p_persDat->uiCurrTm).c_str());
        connDbDat->AppendSqlString(updateSql, " where uid=");
        connDbDat->AppendSqlString(updateSql, CTrans::ITOS(udid).c_str());
        connDbDat->AppendSqlString(updateSql, " and plat=");
        connDbDat->AppendSqlString(updateSql, CTrans::ITOS(emPlat).c_str());


        LOG_DEBUG("UPDATE sql: uid=%s plat=%s update_time=%s [%s],proccessId:[%u]", 
			CTrans::ITOS(udid).c_str(), CTrans::ITOS(emPlat).c_str(), CTrans::ITOS(p_persDat->uiCurrTm).c_str(), updateSql.c_str(), CallProcId);
        //update tbname set rank =  where uid =
        if( false == connDbDat->Query(updateSql.data(), updateSql.size()) )
        {
            LOG_ERROR("update sql:[%s],proccessId:[%u]",updateSql.c_str(),CallProcId);
            iRet = RetErr;
            return iRet;
        }
    }
    else//insert
    {
    	static std::string insertSql;
    	insertSql.reserve(SQL_BUFF_SIZE);
        //insert the tm...or ..

        connDbDat->InitSqlString(insertSql, "insert into ");
        connDbDat->AppendSqlString(insertSql, sDbTabName);
        connDbDat->AppendSqlString(insertSql, " (uid, plat, update_time");

		for(size_t i = 0; i < vecColName.size(); i++)
		{
            connDbDat->AppendSqlString(insertSql, ", ");
            connDbDat->AppendSqlString(insertSql, vecColName[i].c_str());
		}
        connDbDat->AppendSqlString(insertSql, ") values( ");
        connDbDat->AppendSqlString(insertSql, CTrans::ITOS(udid).c_str());
        connDbDat->AppendSqlString(insertSql, ", ");
        connDbDat->AppendSqlString(insertSql, CTrans::ITOS(unsigned(emPlat)).c_str());
        connDbDat->AppendSqlString(insertSql, ", ");
        connDbDat->AppendSqlString(insertSql, CTrans::ITOS(p_persDat->uiCurrTm).c_str());

		for(size_t i = 0; i < vecData.size(); i++)
		{
			if(i == 0)
			{
				connDbDat->AppendSqlString(insertSql, ", \'");
				connDbDat->AppendSqlEscapeString(insertSql, vecData[i].data(), vecData[i].size());
			}
			else
			{
				connDbDat->AppendSqlString(insertSql, "\', \'");
				connDbDat->AppendSqlEscapeString(insertSql, vecData[i].data(), vecData[i].size());
			}

		}
		connDbDat->AppendSqlString(insertSql, "\' ); ");

        LOG_DEBUG("insert sql:[%s],ProccessId:[%u]",insertSql.c_str(),CallProcId);
        if( false == connDbDat->Query(insertSql.data(),insertSql.size() ))
        {
            LOG_ERROR("insert sql:[%s] failed,proccessId:[%u]",insertSql.c_str(),CallProcId);
            iRet = RetErr;
            return iRet;
        }
    }

    return iRet;
}
//checktmout::
int CheckTmOut(std::string &vecVal, const std::string &score, int *iTmOutFlg, long long *pUid, int *pAccTm )
{
    int iRet 		 = 0;
    int accTime		 = 0;
    long long udid 	 = 0;
    unsigned int  tCurTime;


    if( iTmOutFlg == NULL )
    {
        LOG_ERROR("input failed ");
        iRet = RetErr;
        return iRet;
    }
    FNF::mod_bitmap cBitMap;
    cBitMap.ParseFromString(vecVal);
    udid = cBitMap.uid();

    *pUid = udid;
    *pAccTm = CTrans::STOI(score);
    tCurTime = p_persDat->uiCurrTm;
    if( (tCurTime - *pAccTm) > p_persDat->iPerTmVal )
    {
        *iTmOutFlg = TMOUT;
        LOG_DEBUG("tm out: true tm diff:[%d=%lld-%lld], predefined:[%lld],procesId:[%u]",\
            tCurTime-accTime,tCurTime,accTime,\
            p_persDat->iPerTmVal,\
            CallProcId);
    }
    else
    {
        *iTmOutFlg  = NOTMOUT ;
    }
    return iRet;
}
//load keys one by one...
int OneTmPersis( std::string & bitmapInfo)
{
    int iRet = 0;
    long long udid =0;
    ::FNF::EM_PLAT emPlat;

    unsigned int tCurTime = p_persDat->uiCurrTm;

    FNF::mod_bitmap cBitMap;
    cBitMap.ParseFromString(bitmapInfo);
    udid = cBitMap.uid();
    emPlat = cBitMap.plat();

    LOG_DEBUG("keys:[%s],in vail uid:[%lld],curTm:%u,proccesId: [%u]",\
        p_persDat->queenKey.c_str(), udid,tCurTime, CallProcId);
#if 0
    // TODO
    // CheckTmOut(bitmapInfo, &tmoutflg, &udid, &accTime);
    return NOPERSIS;

#endif

    std::vector<std::string> vecData;

    iRet = LoadKeyCache(udid, emPlat, vecData);
    if( iRet != RetSucc )
    {
        LOG_ERROR("LoadKeyCache not exist! uid:%d proccesId:[%u]", udid, CallProcId);
        iRet = RetErr;
        return iRet;
    }


    iRet = SndDatToDb(udid, emPlat, vecData);
    if( iRet != RetSucc )
    {
        LOG_ERROR("SndDatToDb uid:%d proccesId:[%u]", udid, CallProcId);
        iRet = RetErr;
        return  iRet;
    }
    return iRet;
}

// one query_queue_conn_redis...
int GetQTailNode(CRedis* pconnObj,const std::string & ip,const std::string & port)
{
    int iRet = RetSucc;

    std::string Keys;
    std::string cacheKeys;
    // traverse some keys and persistence <key,val>.
    LOG_DEBUG("key nums:[%d] in keylist in process id:[%u]",p_persDat->iKeysNum,CallProcId);
    int iStartIndex =0;
    int iStopIndex  =0;

    //process queenREAD_PER_QUERY * READ_QNODE_MAXNUM
    STQTailNode stQTailND;

    //int ivecNum  = 0;
    //start step stop conf
    InitQTailNode(&stQTailND, 0, READ_PER_QUERY);
    iStartIndex = stQTailND.ibgIndex;
    iStopIndex  = stQTailND.iStepLen-1;

    //queen bitmap
    Keys = p_persDat->queenKey;
    LOG_DEBUG("start_index: [%d], stopidx:[%d] for keys:[%s], process pid:[%d]",\
        iStartIndex,\
        iStopIndex,\
        Keys.c_str(),\
        CallProcId);
    iRet = pconnObj->ZRANGEWITHSCORE(Keys, iStartIndex, iStopIndex, stQTailND.stVal, stQTailND.stScore);
    if( iRet != RetSucc )
    {
        LOG_ERROR("get bitmap failed,keys:[%s]  in processId: [%u]", Keys.c_str(), CallProcId);
        iRet = RetErr;
        return iRet;
    }

    if( stQTailND.stVal.size() == 0 )
    {
        LOG_NOTICE("empty bitmap, keys:[%s]  in processId: [%u]", Keys.c_str(), CallProcId);
        iRet = RetErr;
        return iRet;
    }

    // check newest item
    // if it's not timeout, then
    int flgtmout  = 0;
    long long tmpUid = 0;
    int tmpTm = 0;

    //bool bNeedFetchMore(true);
    size_t uCount  = stQTailND.stScore.size();

    CheckTmOut(stQTailND.stVal[uCount-1], stQTailND.stScore[uCount-1], &flgtmout,&tmpUid, &tmpTm);
    if(flgtmout == NOTMOUT )
    {
        LOG_NOTICE("finish timeout bitmap, key:%s, count:%u, score:%d pid:[%u]",\
            Keys.c_str(),\
            uCount,\
            CTrans::STOI(stQTailND.stScore[uCount-1]),\
            CallProcId);
    }

    LOG_DEBUG("vector <string>s iStartIndex:[%d], iStopIndex:[%d], ProccessId:[%u]",\
        iStartIndex, iStopIndex, CallProcId);

    //process all fetched items
    int iTotalTmNum = 0;

    std::vector <std::string>& vecVal  = stQTailND.stVal;
    //std::vector <int>& vecScore = stQTailND.stScore[iAcc];
    for(size_t i = 0; i< vecVal.size();i++)
    {
        iRet = OneTmPersis(vecVal[i]);

        if( iRet != RetErr )
        {
            iTotalTmNum++;
            //del_queen
            pconnObj->ZREM(Keys, vecVal[i]);
        }
        else
        {
        	pconnObj->ZREM(Keys, vecVal[i]);
            goto del_queen;
        }
    }
    

del_queen:
    if( iTotalTmNum >= 1 )
    {
        LOG_NOTICE("Total process number:%d, proccessId:%u",iTotalTmNum, CallProcId);
//        pconnObj->ZREMRANGEBYRANK(Keys,0,iTotalTmNum - 1 );
        LOG_DEBUG("del keys[%s] from query_queue_redis<ip,port>:[%s,%s],processId:[%u] ",\
            Keys.c_str(),\
            ip.c_str(),\
            port.c_str(),\
            CallProcId);
    }
    //}
    return iRet;
}
// traverse all obj connected with queue_redis..
int TrvseAllConnObj( )
{
    int iRet = RetSucc;
    CRedis* pConnRedis = NULL;
    struct NETPOINT& tmpQIpPort =p_persDat->stOnePrcssQIpPort;
    //one processs do one connecting from query_queue_redis...
    pConnRedis = GET_REDIS_AUTH(tmpQIpPort.ip,atoi(tmpQIpPort.port.c_str()), tmpQIpPort.passwd);
    if( pConnRedis != NULL )
    {
        LOG_DEBUG("get query redis_svr: [%s:%s:%s] by process pid:[%u]",\
            tmpQIpPort.ip.c_str(),\
            tmpQIpPort.port.c_str(),\
            tmpQIpPort.passwd.c_str(),\
            CallProcId);
        GetQTailNode(pConnRedis, tmpQIpPort.ip,tmpQIpPort.port);
    }
    return iRet;
}
// main_loop
int PersisMainLoop( )
{
    int iRet = RetSucc;
    int tmpVal = 0;
    AddTimer(&p_persDat->iSessSeq,p_persDat->iTmOutVal,p_persDat->TmOutCallBackFunc,sizeof(int),(char*)&tmpVal);
    for(; ;)
    {
        /*
         * test for restart after core producation..
         sleep(5);
         abort();
         */
        //check timer whether tmout, if tmout, call tmout_callback_func.
        CheckTimer();
        p_persDat->uiCurrTm = GetLocalTm(); // time(NULL);
        if( p_persDat->iTmOutFlg == TMOUT )
        {
            RESET_TM(p_persDat);
            LOG_DEBUG("ProcessID: [%u] tm out, now query the queue_redis",CallProcId);
            DelTimer(p_persDat->iSessSeq);
            TrvseAllConnObj();
            AddTimer(&p_persDat->iSessSeq,p_persDat->iTmOutVal,p_persDat->TmOutCallBackFunc,sizeof(int),(char*)&tmpVal);
        }
        usleep(100);
    }
    return iRet;
}

int PersisDestroy()
{
    int iRet = RetSucc;
    return iRet;
}
