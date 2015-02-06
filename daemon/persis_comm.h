#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "db/mysql_conf.h"
#include "db/redis_conf.h"
#include "db/keylist_conf.h"

#include "log/logger.h"
#include "db/redis_wrapper.h"
#include "db/mysql_wrapper.h"
#include "db/mysql_pool.h"
#include "db/redis_pool.h"
#include "logic/fnf_modbitmap.pb.h"

// local head_file.
#include "hash_table.h"
#include "oi_list.h"
#include "oi_timer.h"
#include "basic/crc.h"

//#define READ_QNODE_MAXNUM 	(1)
#define READ_PER_QUERY (50)
#define ErrStr strerror(errno)
#define CallProcId (p_persDat->usCallPid)
#define ParentCallProcId  (p_persDat->usParCallPid)

typedef enum {
		RetSucc	= 0,
		RetErr = -1
}ENUMRET;
//define persistence flags...
typedef enum {
		PERSIS = 1,
		NOPERSIS = 2
}ENUMPERSIS;

typedef enum {
		NOTMOUT  = 0,
		TMOUT 	 = 1
}ENUMTMOUT;

typedef struct PersisDat{
		//mysql conf obj:
		CMysqlConf PisDBCnfObj;
		//queue redis conf obj:
		CRedisConf PisQRdsCnfObj;
		//cache redis conf obj:
		CRedisConf PisCRdsCnfObj;

		//cache redis_svr obj:
		//CRedisPool PisCacheRdsObj;

		// conf path name.
		char sDbCnfPath[256];
		char sQRdsCnfPath[256];
		char sCheRdsCnfPath[256];
		char sKeyLstCnfPath[256];
		char sLogConfPath[256];

		//flag for tmout:
		int iTmOutFlg;
		//tm out val for query queue.
		unsigned int iTmOutVal;
		// one key persistence to db time..
		unsigned int  iPerTmVal; // us level
		//timer sess_seq
		unsigned int iSessSeq;
		//timerOut callback.
		void (*TmOutCallBackFunc)(unsigned int iDatLen,char *pDatVal);

		// query_queue's <ip,port> vector..
		VECNETPORT QIpPort;

		//conn_pool obj pointer:
		CRedis* connOjbArray[];

		#if 0
		<pair>
	        <queen key="back_pack" desc="user back_pack queen, sorted by timestamp"></queen>
	        <cache keyprefix="back_pack:" desc="redis user back_pack"></cache>
	        <mysql colname="back_pack" desc="mysql user back_pack"></mysql>
	    </pair>
	    <pair>
	        <queen key="growup" desc="user growup queen, sorted by timestamp"></queen>
	        <cache keyprefix="growup:" desc="redis user growup"></cache>
	        <mysql colname="growup" desc="mysql user growup"></mysql>
	    </pair>
	    #endif
	    //key num read from keylist conf
		int iKeysNum ;
		CKeyListConf KeyLstCnfObj;
		std::vector<KEYLISTNODE> KeyListItem;
		std::string queenKey;

		unsigned long long uiCurrTm;
		//each process, ip,port for each query_queue_redis_cnf
		struct NETPOINT stOnePrcssQIpPort;
		//process' begin index:
		int iProcBIdx;
		//process' end index:
		int iProcEIdx;
		pid_t usCallPid;
		pid_t usParCallPid;

}PERSISST;

typedef struct _stQTailNode{
		int ibgIndex;
		int iStepLen;

		std::vector <std::string>  stVal;
		std::vector <std::string>  stScore;
}STQTailNode, *PSTTailNode;


typedef enum proc_ret{
		RET_CHPROC = 1,
		RET_PARPROC = 2,
}ENUMPROCRET;

inline int InitQTailNode( PSTTailNode pstNode ,int ibgval,int istplen )
{
		if( pstNode == NULL || ibgval<0 || istplen<0 )
		{
			LOG_ERROR("input is invailed ");
			return -1;
		}
		pstNode->ibgIndex = ibgval;
		pstNode->iStepLen = istplen;
		pstNode->stVal.clear();
		pstNode->stScore.clear();
		pstNode->stVal.reserve(READ_PER_QUERY);
		pstNode->stScore.reserve(READ_PER_QUERY);
		LOG_DEBUG("init queue tail node ");
		return 0;
}

// init/read conf
int InitObj( PERSISST *pstPsisDat, int argc, char **argv);
//check single process...
 int IsSingle( char *psProcName );
//daemon_init
int InitDaemon();
// create conn_obj accoarding <ip,port> in conf..
int ConnToQRdisSvr();
// main_loop
int PersisMainLoop();
// free resource and deconstruct many objs...
int PersisDestroy();


