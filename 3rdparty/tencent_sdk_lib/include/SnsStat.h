/**
 * 统计上报接口调用情况
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

/*	SPackage参数解释:
*	"appid":10881, //应用ID uint32_t
*	"pf":"qzone", //平台标识
*	"rc":2003,    //接口返回码 
*	"svr_name":"172.1.2.56",            //访问服务器地址 
*	"interface":"\/v3\/user\/get_info", //接口名称
*	"protocol":"http",                  //协议类型 
*	"method":"post",                    //请求方式  post/get
*	"time":0.0014,                      //返回时间  
*	"timestamp":1332507980,             //上报的时间戳  
*	"collect_point":"sdk-php-v3"        //收集点  
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
	**函数名 Report
	**输入:   
	**			SPackage   上报的参数
				stStatUrl    服务器地址

	**输出:  无
	**				
	**返回: 无
	**描述:  上报监控需要的数据
	**			  
	**************************************************************/
	int Report(SPackage& pkg, string& stStatUrl);

	 /***********************************************************
	 **函数名 GetErrMsg
	 **输入:   无
	 ** 		
	 **输出:  无
	 ** 			 
	 **返回: 错误信息
	 **描述:  返回错误信息
	 ** 		   
	 **************************************************************/
	 const char* GetErrMsg(){return m_errmsg;}

	/***********************************************************
	**函数名 getUdpClientMsg
	**输入:   无
	**		   
	**输出:  无
	**				
	**返回: 无
	**描述:  返回已连接的IP地址以空格分隔如"10.1.1.2 10.1.1.3"
	**			  
	**************************************************************/
	string getUdpClientMsg();
protected:
	/***********************************************************
	**函数名 Connect
	**输入:   sSvrInfo  服务器的IP和端口
	**		   
	**输出:  无
	**				
	**返回: 无
	**描述:  连接对端的服务器
	**			  
	**************************************************************/
	int Connect(SServerInfo& sSvrInfo);
	/***********************************************************
	**函数名 SendBuffer
	**输入:    pBuf       发送缓存区的地址
			      iBufLen   缓冲区的长度
				sSvrInfo  server端的地址
	**		   
	**输出:  无
	**				
	**返回: 0	成功
			   其他 失败
	**描述:  向server端发送信息
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

