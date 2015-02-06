#include "daemon/persis_comm.h"
#include "logic/fnf_modbitmap.pb.h"
#include "basic/crc.h"
#include "basic/trans.h"

using namespace std;
#define SLEEP_US 5000

typedef struct  _stTestDat{
	long long  uid;
	char sKey[256];
	char sVal[1024];

	//cache  redis conf
	CRedisConf CRdsCnfObj;
	//keylist cnf
    NETPOINT queenRdsIpPort;

    CMysqlConf stDBCnfObj;

	std::vector<KEYLISTNODE> KeyListItem;
    std::string queenKey;
	int iKeysNum ;
}CliTestDat, *PCliTestDat;

CliTestDat stTestDat, *pstTestDat;



static inline unsigned int GetLocalTm( )
{
    return time(NULL);
#if 0
	long long iRet = 0;
	struct timeval tt;
	if( -1 == gettimeofday(&tt,NULL))
	{
		return -1;
	}
	iRet = tt.tv_sec*1000*1000 +tt.tv_usec;
	return iRet;
    #endif
}
int test_crc32()
{
    std::string t = "18354222797946880";
    int ret= load_crc32(0,t.c_str(),t.size());
    printf("test, crc32(%s)=%d\n", t.c_str(), ret);
    return 0;
}

static unsigned int GetUidCrc32(unsigned long long udid)
{
    //need to check crc32(uid);
    std::string sAccUid = CTrans::ITOS(udid);
    unsigned int iCrcKeyUid= load_crc32(0,sAccUid.c_str(),sAccUid.size());
    return iCrcKeyUid;
}
static char * GetDbTableName(PCliTestDat pstDat,unsigned long long uuid)
{
    MYSQLNETPOINT dbCnfDat;
    unsigned  int iCrcUid = GetUidCrc32(uuid);
    int iRet = pstDat->stDBCnfObj.GetIpPort(iCrcUid,dbCnfDat);
    if( iRet == RetSucc )
    {
        unsigned int iFlag = 0;
        iFlag = (iCrcUid/100)%100;
        static char sDbTabName[128];
        /** 12345; 23 is databasename, 45 tablename **/
        snprintf(sDbTabName,sizeof(sDbTabName),"%s.%s%u",dbCnfDat.dbname.c_str(),dbCnfDat.tblprefix.c_str(),iFlag);

        fprintf(stderr, "uid:%lld, dbinfo %s:%d %s\n", uuid, dbCnfDat.ip.c_str(),dbCnfDat.port, sDbTabName );
        return sDbTabName;

    }
    LOG_ERROR("get db conf dat failed, ret:%d, crc32(%lld)=[%u]", iRet, uuid, iCrcUid);
    return NULL;

}

int InitCnf(PCliTestDat pStDat, int argc, char **argv);
int main(int argc, char **argv)
{
    //    test_crc32();

	//load conf file:::
	pstTestDat = &stTestDat;

	int iRet = 0;
	iRet = InitCnf(pstTestDat,argc, argv);
	if( iRet != 0 )
	{
		printf(" init conf failed \n");
		return 0;
	}
	std::string ckeys;
	std::string qKeys;

	//if uid:[100,10000,100000, 1000000]
	//#define MAXUID (10000)
	//long long uid = MAXUID;
	unsigned int keyid = 0;
	long long iAccUid = 1;
    //insert profile:xx, growup:xxx, backpack:xxx
    //insert queen profile,growup, backpack
	while ( true)
	{
        std::string strPlat = "ios:";
        ::FNF::EM_PLAT plat =  ::FNF::PLAT_IOS;
        if(iAccUid % 2 ==  0)
        {
            plat = ::FNF::PLAT_AND;
            strPlat = "and:";
        }


        for(int iKeyN =0; iKeyN < pstTestDat->iKeysNum; iKeyN++)
        {
            NETPOINT cIpPort;

            CRedis *cacheRedis = NULL;
            ckeys = pstTestDat->KeyListItem[iKeyN].cacheKeyPrefix;

            std::string sAccUid = CTrans::ITOS(iAccUid);

            keyid = load_crc32(0,sAccUid.c_str(),sAccUid.size());
            LOG_DEBUG("keyIdx: %u by keystr: %s ",keyid, sAccUid.c_str());
            iRet = pstTestDat->CRdsCnfObj.GetUserIpPort(keyid,cIpPort);
            if(iRet != 0)
            {
                LOG_ERROR("get cache <ip,port> failed by keyid: %d",keyid);
                return iRet;
            }

            int iPort = atoi(cIpPort.port.c_str());
            cacheRedis = GET_REDIS(cIpPort.ip,iPort);
            if( cacheRedis == NULL )
            {
                LOG_ERROR("conn to cache redis failed  ");
                return -1;
            }

            snprintf(pstTestDat->sVal,sizeof(pstTestDat->sVal),"%s is %lld, time:%u","cache content@", iAccUid, unsigned(time(NULL)));
            //ios:profile:2321421
            //and:growup:234
            std::string cacheKey = strPlat + ckeys + sAccUid;
            cacheRedis->SetKey(cacheKey,pstTestDat->sVal);
            fprintf(stderr, "Cache key:%s, data:%s\n", cacheKey.c_str(), pstTestDat->sVal);
        }

        qKeys = pstTestDat->queenKey;
        unsigned int   accTm;
        accTm =GetLocalTm(); // time(NULL);

        FNF::mod_bitmap cBitMap;
        cBitMap.set_uid(iAccUid);
        cBitMap.set_plat(plat);

        std::string vcData;
        cBitMap.SerializeToString(&vcData);
        //dbname
        GetDbTableName(pstTestDat, iAccUid );

        //send uid,acctime to which<ip,port> query_queue_redis.?
        //query....
        int iPort = atoi(pstTestDat->queenRdsIpPort.port.c_str());
        CRedis* qRdis = GET_REDIS(pstTestDat->queenRdsIpPort.ip, iPort);
        if(qRdis == NULL)
        {
            LOG_ERROR("get/conn qry_que_redis failed by ip:%s,port:%s",pstTestDat->queenRdsIpPort.ip.c_str(),pstTestDat->queenRdsIpPort.port.c_str());
            return -1;
        }
        iRet = qRdis->ZADD(qKeys,accTm,vcData);
        if(iRet != 0 )
        {
            LOG_ERROR("add key to query_queue failed ");
            return iRet;
        }

        usleep(SLEEP_US );
        //nanosleep
        iAccUid ++;
    }
    return 0;
}

//
int InitCnf(PCliTestDat pstDat, int argc, char **argv)
{
    int iRet = 0;
	if( argc < 6 )
	{
		printf("usage<%s> userredis_cnf db_conf keylist_cnf  queenip queenport\n",argv[0]);
		return -1;
	}

	CKeyListConf stTKeyLstCnfObj;
	INIT_LOG("./","clitest.log", "../conf/log.conf");
	iRet = pstDat->CRdsCnfObj.LoadConf(argv[1]); //get cache conf ...  _mapUserRedis
	if( iRet != 0 )
	{
		LOG_ERROR("load cache cnf failed ");
		iRet = -1;
		goto errFlg;
	}


	iRet = stTKeyLstCnfObj.LoadConf(argv[3]);
	if( iRet != 0 )
	{
		LOG_ERROR("load keylist cnf failed ");
		iRet = -2;
		goto errFlg;
	}
	iRet = stTKeyLstCnfObj.Get( pstDat->KeyListItem );
	if( iRet != 0 )
	{
		LOG_ERROR("get keylist failed ");
		iRet = -3;
		goto errFlg;
	}
	pstDat->iKeysNum = pstDat->KeyListItem.size();
    stTKeyLstCnfObj.GetQueenKeyName(pstDat->queenKey);

    pstDat->queenRdsIpPort.ip = argv[4];
    pstDat->queenRdsIpPort.port = argv[5];
	LOG_DEBUG("key num: %d ",pstDat->iKeysNum);
    //db conf
    iRet = pstDat->stDBCnfObj.LoadConf(argv[2]);
    if( iRet != 0)
    {
        LOG_ERROR("Load db conf failed ");
        iRet = RetErr;
        return iRet;
    }


errFlg:
	return iRet;
}
