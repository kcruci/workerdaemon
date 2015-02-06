/**
 * ͳ���ϱ��ӿڵ������
 *
 * @version 3.0.0
 * @author open.qq.com
 * @copyright (c) 2013, Tencent Corporation. All rights reserved.
 * @ History:
 *               3.0.0 | jixingguan | 2013-05-16 10:33:04 | initialize 
 */

#include "SnsStat.h"
#include <time.h>
#include <ctime>
#include "SnsStat.h"
#include <iostream>
#include "OpenApiUdpClient.h"
#include "openapi_comm_def.h"
#include <netdb.h>

#define OPENAPI_STAT_PORT 19888

COpenApiSnsStatClient::COpenApiSnsStatClient()
{
}

COpenApiSnsStatClient::~COpenApiSnsStatClient()
{
	foreach(m_oObjUdpClient, it)
	{
		delete it->second;
	}
	m_oObjUdpClient.clear();
}

/***********************************************************
**������ Report
**����:   
**          SPackage   �ϱ��Ĳ���
            stStatUrl    ��������ַ

**���:  ��
**              
**����: ��
**����:  �ϱ������Ҫ������
**            
**************************************************************/
int COpenApiSnsStatClient::Report(SPackage& pkg, string& stStatUrl)
{
    SServerInfo stSvrInfo;	
    struct hostent *ht = NULL;  


    ht = gethostbyname(stStatUrl.c_str());
    if( NULL == ht)
    {
        OPENAPIV3_ERROR("gethostbyname url=%s failed ",stStatUrl.c_str());
        return -1;
    }

    char SnsStatIp[128]={'\0'};
    inet_ntop(ht->h_addrtype, ht->h_addr, SnsStatIp, sizeof(SnsStatIp));
       
	stSvrInfo.strIP = SnsStatIp;
    stSvrInfo.usPort=OPENAPI_STAT_PORT;
	
	int32_t iRet = Connect(stSvrInfo);
	if(iRet != 0)
	{
		return iRet;
	}
	pkg.timestamp = time(NULL);

	if(pkg.rv == -1)
	{
		if(pkg.rc == 0)
		{
			pkg.rv = 0;
		}
		else
		{
			pkg.rv = 1;
		}
	}

	string request;
	ostringstream oss;
	oss<<"{\"appid\":"<<pkg.appid<<",\"rc\":"<<pkg.rc<<",\"rv\":"<<pkg.rv<<",\"timestamp\":"<<pkg.timestamp<<",\"pf\":\""<<pkg.pf.c_str()
			<<"\",\"svr_name\":\""<<pkg.svr_name.c_str()<<"\",\"interface\":\""<<pkg.interface.c_str()<<"\",\"protocol\":\""<<pkg.protocol.c_str()
			<<"\",\"method\":\""<<pkg.method.c_str()<<"\",\"time\":"<<pkg.time<<",\"collect_point\":\""<<pkg.collect_point.c_str()<<"\"}";
	request = oss.str();

	iRet = SendBuffer(request.c_str(), request.length(), stSvrInfo);
	return iRet;
}

/***********************************************************
**������ Connect
**����:   sSvrInfo  ��������IP�Ͷ˿�
**         
**���:  ��
**              
**����: ��
**����:  ���ӶԶ˵ķ�����
**            
**************************************************************/
int32_t COpenApiSnsStatClient::Connect(SServerInfo& sSvrInfo)
{
	int32_t iRet=0;
	if(sSvrInfo.ubNetMode == CNETMODE_UDP)
	{
		m_objUdpClient = getUpdClient(sSvrInfo);

		return iRet;
	}
	else
	{
        OPENAPIV3_ERROR("NOT_UDP_NETMODE");
		return -1;
	}
	return 0;
}

/***********************************************************
**������ SendBuffer
**����:    pBuf       ���ͻ������ĵ�ַ
              iBufLen   �������ĳ���
            sSvrInfo  server�˵ĵ�ַ
**         
**���:  ��
**              
**����: 0   �ɹ�
           ���� ʧ��
**����:  ��server�˷�����Ϣ
**            
**************************************************************/
int32_t COpenApiSnsStatClient::SendBuffer(const char* pBuf, int32_t iBufLen, SServerInfo& sSvrInfo)
{

	int32_t iRet = Connect(sSvrInfo);
	if(iRet != 0)
	{
		return iRet;
	}
	if(sSvrInfo.ubNetMode == CNETMODE_UDP)
	{
		iRet = m_objUdpClient->SendBuf(pBuf, iBufLen);
		if (iRet != 0)
		{
			OPENAPIV3_ERROR("CUpdClient.SendAndRecv error, ret=%d ip=%s port=%d  errmsg=%s",
			                iRet,sSvrInfo.strIP.c_str(),sSvrInfo.usPort,m_objUdpClient->GetErrMsg());
		}
	}
	return iRet;
}

/***********************************************************
**������ getUpdClient
**����:    sSvrInfo  server��Ϣ
**         
**���:  ��
**              
**����: UDP�Ŀͻ��˶���
**����:  ����IP�Ͷ˿ڻ�ȡUDP�Ŀͻ��˶���������򷵻أ�û���򴴽�
**            
**************************************************************/
CUdpClient*  COpenApiSnsStatClient::getUpdClient(SServerInfo& sSvrInfo)
{
	ostringstream oss;
	oss<<sSvrInfo.strIP<<"_"<<sSvrInfo.usPort;
	string key = oss.str();
	map<string, CUdpClient*>::iterator it = m_oObjUdpClient.find(key);
	if(it != m_oObjUdpClient.end())
	{
		return it->second;
	}
	else
	{
		CUdpClient* pUdpClient = new CUdpClient();
		pUdpClient->Open(sSvrInfo.strIP.c_str(),sSvrInfo.usPort);
		m_oObjUdpClient.insert(pair<string, CUdpClient*>(key, pUdpClient));
		return pUdpClient;
	}
}

/***********************************************************
**������ getUdpClientMsg
**����:   ��
**         
**���:  ��
**              
**����: ��
**����:  ���������ӵ�IP��ַ�Կո�ָ���"10.1.1.2 10.1.1.3"
**            
**************************************************************/
string COpenApiSnsStatClient::getUdpClientMsg()
{
	ostringstream oss;
	foreach(m_oObjUdpClient, it)
	{
		oss<<it->first.c_str()<<" ";
	}
	return oss.str();

}

