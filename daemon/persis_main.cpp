#include "persis_comm.h" 

using namespace std;
static size_t auNodeNum[] = \
	{100, 100, 100, 100, 100};

//define seconds level...
// 100 ms per tida
#define TIMER_CHECK_INTERVAL_US (100*1000) 

PERSISST g_persDat, *p_persDat;

int main(int argc ,char **argv)
{
	//mysql_server_init();

	int iRet = RetSucc;			
	p_persDat = &g_persDat;

	//load cnf to redis/mysql obj.
	iRet = InitObj(p_persDat,argc,argv);
	if( iRet == RetErr )
	{
		LOG_ERROR("init obj init failed main");
		fprintf(stderr, "init obj init failed\n");
		return iRet;
	}
	//iRet = IsSingle(argv[0]);
	//if( iRet == RetErr )
	//{
	//	LOG_ERROR("proc has running .....");
	//	return 0;
	//}

	//conn query_queue redis svr.
	iRet = ConnToQRdisSvr();	
	if( iRet == RetErr )
	{
		LOG_ERROR("conn to queue query redis svr failed ");
		fprintf(stderr, "conn to queue query redis svr failed\n");
		return iRet;
	}
	
	//parent process 
	if( iRet == RET_PARPROC )
	{
		LOG_NOTICE("parent process id:[%u] go out, end now.... ",CallProcId);
		return iRet;
	}

	LOG_DEBUG("child proc now running,pid: [%u].....",CallProcId);

	iRet = InitTimer(sizeof(int),DIM(auNodeNum), auNodeNum,TIMER_CHECK_INTERVAL_US, CONST_DATA_LEN_TIMER );
	if( iRet == RetErr )
	{
		LOG_ERROR("init timer failed ");
		fprintf(stderr, "init timer failed \n");
		return iRet;
	}

	//parent fork child when child cored;
	iRet = InitDaemon();
	if( iRet == -1 )
	{
		exit(-1);
	}

	iRet = PersisMainLoop();
	PersisDestroy();
	
	//mysql_server_end();
	return 0;
}

