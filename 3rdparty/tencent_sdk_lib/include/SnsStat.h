/**
 * ͳ���ϱ��ӿڵ������
 *
 * @version 3.0.0
 * @author open.qq.com
 * @copyright (c) 2013, Tencent Corporation. All rights reserved.
 * @ History:
 *               3.0.0 | jixingguan | 2013-05-16 10:33:04 | initialize 
 */

#ifndef TENCENT_OPENAPI_SNSSTAT_CLIENT_H_
#define TENCENT_OPENAPI_SNSSTAT_CLIENT_H_
#include <string>
#include <map>
#include "OpenApiUdpClient.h"


using namespace std;


enum CLIENT_NETMODE
{
	CNETMODE_UDP=1,
};
struct SServerInfo
{
	string strIP;
	uint16_t usPort;
	uint8_t ubNetMode;
	SServerInfo()
	{
		strIP = "";
		usPort = 0;
		ubNetMode = CNETMODE_UDP;
	}
};

/*	SPackage��������:
*	"appid":10881, //Ӧ��ID uint32_t
*	"pf":"qzone", //ƽ̨��ʶ
*	"rc":2003,    //�ӿڷ����� 
*	"svr_name":"172.1.2.56",            //���ʷ�������ַ 
*	"interface":"\/v3\/user\/get_info", //�ӿ�����
*	"protocol":"http",                  //Э������ 
*	"method":"post",                    //����ʽ  post/get
*	"time":0.0014,                      //����ʱ��  
*	"timestamp":1332507980,             //�ϱ���ʱ���  
*	"collect_point":"sdk-php-v3"        //�ռ���  
*/
struct SPackage
{
	SPackage()
	{
		rv = -1;
		appid = 0;
		rc = 0;
	}
	unsigned int appid;
	int rc;
	int rv;
	int timestamp;
	double time;
	string pf;
	string svr_name;
	string interface;
	string protocol;
	string method;
	string collect_point;
};

class COpenApiSnsStatClient
{
public:
	COpenApiSnsStatClient();
	~COpenApiSnsStatClient();

	
	/***********************************************************
	**������ Report
	**����:   
	**			SPackage   �ϱ��Ĳ���
				stStatUrl    ��������ַ

	**���:  ��
	**				
	**����: ��
	**����:  �ϱ������Ҫ������
	**			  
	**************************************************************/
	int Report(SPackage& pkg, string& stStatUrl);

	 /***********************************************************
	 **������ GetErrMsg
	 **����:   ��
	 ** 		
	 **���:  ��
	 ** 			 
	 **����: ������Ϣ
	 **����:  ���ش�����Ϣ
	 ** 		   
	 **************************************************************/
	 const char* GetErrMsg(){return m_errmsg;}

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
	string getUdpClientMsg();
protected:
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
	int Connect(SServerInfo& sSvrInfo);
	/***********************************************************
	**������ SendBuffer
	**����:    pBuf       ���ͻ������ĵ�ַ
			      iBufLen   �������ĳ���
				sSvrInfo  server�˵ĵ�ַ
	**		   
	**���:  ��
	**				
	**����: 0	�ɹ�
			   ���� ʧ��
	**����:  ��server�˷�����Ϣ
	**			  
	**************************************************************/
	int SendBuffer(const char* pBuf, int32_t iBufLen, SServerInfo& sSvrInfo);
	CUdpClient* getUpdClient(SServerInfo& sSvrInfo);
private:
	bool m_bIsInit;
	CUdpClient* m_objUdpClient;
	map<string, CUdpClient*> m_oObjUdpClient;
	char m_errmsg[512];
};
#endif /* OPENAPI_MONITOR_CLIENT_H_ */

