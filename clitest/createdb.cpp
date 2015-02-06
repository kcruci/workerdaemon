/*****
 * create 100 table foreach database..
 * each table include column: uid<bigint>,profile<varchar>,
 *    back_pack<varchar>,growup<varchar>
 *    ;
 *    database name:fnf_db_0
 *    			   :fnf_db_1
 *    				....
 *    			   :fnf_db_9
 *
 *   hostname: 192.168.2.11 port:3306
 *   username: idreamsky
 *   passwd  : idreamsky
 *
 *
 * ***********/

#include <stdio.h>
#include <mysql.h>
#include <stdlib.h>
#include <string.h>

typedef struct _test_mysql_st{
		MYSQL *pDbHandle;
		char sHost[128];
		char sUser[128];
		char sPassWd[128];
		unsigned int uiPort;

		char mysql_buf[256];			
		int iDbNum;
		int iTbNum;
}DbStrDat, PDbStrDat;

DbStrDat g_stDb, *p_gstDb;

#ifdef _test_create_db
int main()
{
	p_gstDb = &g_stDb;
	p_gstDb->pDbHandle = NULL;
	memcpy(p_gstDb->sHost,"192.168.2.11",strlen("192.168.2.11")+1);
	memcpy(p_gstDb->sUser,"idreamsky",strlen("idreamsky")+1);
	memcpy(p_gstDb->sPassWd,"idreamsky",strlen("idreamsky")+1);
	p_gstDb->uiPort = 3306;
	p_gstDb->iDbNum  = 10;
	p_gstDb->iTbNum = 100;

	p_gstDb->pDbHandle = mysql_init(NULL);
	if( NULL == p_gstDb->pDbHandle )
	{
		return 0;
	}
	if( NULL == mysql_real_connect(p_gstDb->pDbHandle,p_gstDb->sHost,p_gstDb->sUser,\
					p_gstDb->sPassWd,NULL,p_gstDb->uiPort,NULL,0) )
	{
		printf("conn_mysql failed: %s \n",mysql_error(p_gstDb->pDbHandle));
		mysql_close(p_gstDb->pDbHandle);
		return -1;
	}

	memset( p_gstDb->mysql_buf,0,sizeof(p_gstDb->mysql_buf));
	int idb;
	char tmpBuf[128] = {0};
	char sqlBuf[1024] = {0};

	#define dbpre "fnf_db_"
	#define tbpre "t_user_"
	int iRet= 0;
	for(idb =0; idb<p_gstDb->iDbNum; idb++)
	{
	#if 0
		snprintf(tmpBuf,sizeof(tmpBuf),"create database %s%d",dbpre,idb);
		iRet = mysql_real_query(p_gstDb->pDbHandle,tmpBuf,strlen(tmpBuf)+1);
		if(iRet != 0 )
		{
			printf("query db failed as:%s \n",mysql_error(p_gstDb->pDbHandle));
			return -1;
		}
	#endif
	}
	int itb = 0;
	for(idb = 0; idb<p_gstDb->iDbNum; idb ++ )
	{
		for(itb = 0; itb<p_gstDb->iTbNum; itb ++)
		{
			snprintf(sqlBuf,sizeof(sqlBuf),"create table %s%d.%s%d (uid BIGINT,profile varchar(255),back_pack varchar(255),growup varchar(255));",dbpre,idb,tbpre,itb);
			iRet = mysql_real_query(p_gstDb->pDbHandle,sqlBuf,strlen(sqlBuf)+1);
			if(iRet != 0 )
			{
					printf("query db failed as:%s \n",mysql_error(p_gstDb->pDbHandle));
					return -1;
			}
		}
	}

	return 0;
}
#endif
