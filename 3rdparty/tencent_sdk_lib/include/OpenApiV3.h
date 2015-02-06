/**
 * C++ SDK for  OpenAPI V3
 *
 * @version 3.0.0
 * @author open.qq.com
 * @copyright (c) 2013, Tencent Corporation. All rights reserved.
 * @ History:
 *               3.0.0 | jixingguan | 2013-05-06 10:18:11 | initialization
 */

#ifndef OPEN_API_V3_H
#define OPEN_API_V3_H
#include <string>
#include <map>
#include "http_request_helper.h"
#include "SnsStat.h"
#include "SnsSigCheck.h"
#include "json/json.h"
using namespace std;

enum OPEN_API_V3_RET
{
	OPENAPI_ERROR_REQUIRED_PARAMETER_EMPTY=1801,
	OPENAPI_ERROR_REQUIRED_PARAMETER_INVALID=1802,
	OPENAPI_ERROR_RESPONSE_DATA_INVALID=1803,
	OPENAPI_ERROR_CURL=1900,
};

typedef enum ENUM_OPEN_API_V3_METHOD
{
	OPEN_API_V3_METHOD_GET=1,
	OPEN_API_V3_METHOD_POST=2,
}OPEN_API_V3_METHOD;

typedef enum ENUM_OPEN_API_V3_PROTOCOL
{
	OPEN_API_V3_METHOD_HTTP=1,
	OPEN_API_V3_METHOD_HTTPS=2,
}OPEN_API_V3_PROTOCOL;

#define OPEN_API_v3_DEFAULT_TIME_OUT double(1)
#define OPEN_API_V3_STAT_URL "apistat.tencentyun.com"


typedef class OpenApiV3
{
	
public:

	/***********************************************************
	**������ ���캯��
	**����:   
	**			uiAppid Ӧ�õ�ID
				stAppkey Ӧ�õ�kye

	**���:  ��
	**				
	**����: ��
	**����:
	**			  
	**************************************************************/

	OpenApiV3(unsigned int& uiAppid, string& stAppkey)
	{
		m_appid = uiAppid;
		m_appkey = stAppkey;
		m_initflag=0;

	}

	/***********************************************************
	**������ init
	**����:   
	**			stServer_name  openapi���������ַ������ǵ����봫��"119.147.19.43"
	**			stStat_url : ͳ�������������ò�Ҫ������и���
	**			format:  ��json��xml,����SDKֻ�ṩJSON�Ľ����⣬�����ֵ����
	**			IsStat: �Ƿ����ͳ�ƣ����鿪���������������Ų�
	**
	**���:  JsonRes  ������Ӧ��
	**				
	**����:��
	**����:
	**			 ����API�ӿڵĳ�ʼ�������ýӿ�ʱ�����ȵ�������ӿڣ���������
	**************************************************************/
	void init(string stServer_name="openapi.tencentyun.com",string stStat_url="apistat.tencentyun.com",
			  string format="json",bool IsStat=true);	
	
	/***********************************************************
	**������ setServerName
	**����:   
	**			stServer_name��openapi�������ĵ�ַ
	**���:  
	**				
	**����:��
	**����: ����openapi�ĵ�ַ
	**			  
	**************************************************************/
	void setServerName(string& stServer_name)
	{
		m_server_name = stServer_name;
	}
	
	/***********************************************************
	**������ setStatUrl
	**����:   
	**			stStat_url
	**���:  
	**				
	**����:��
	**����:����ͳ�Ʒ������ĵ�ַ,��ǰֻ��Ϊ"apistat.tencentyun.com"
	**			  
	**************************************************************/
	void setStatUrl(string& stStat_url)
	{
		m_stat_url = stStat_url;
	}

	/***********************************************************
	**������ setStatUrl
	**����:   
	**			stStat_url
	**���:  
	**				
	**����:��
	**����:����ͳ�Ʒ������ĵ�ַ
	**			  
	**************************************************************/	
	void  setIsStat(bool is_stat)
	{
		m_is_stat = is_stat;
	}

	/***********************************************************
	**������ setDebugSwitch
	**����:   
	**			setDebugSwitch
	**���:  
	**				
	**����:��
	**����:���õ��Կ���
	**			  
	**************************************************************/	
	void setDebugSwitch(bool debugswitch)
	{
		m_debugswitch = debugswitch;
	}
	

	/***********************************************************
	**������ api
	**����:   
	**			script_name : ���õ�API����������/v3/user/get_info���ο� http://wiki.open.qq.com/wiki/API_V3.0%E6%96%87%E6%A1%A3
	**			params : ����APIʱ���Ĳ���
	**			method : OPEN_API_V3_METHOD_GET ��OPEN_API_V3_METHOD_POST �����Ƿ�
	**			protocol: Э������֧��HTTP��HTTPS
	**			timeout_sec:��ʱʱ��
	**
	**���:  JsonRes  ������Ӧ��
	**				
	**����:0 �ɹ�
	               ����ʧ��
	**����:
	**			  ִ��API���ã�����openapi server���������ص���Ӧ��
	**************************************************************/
	int api(string& script_name, map<string,string>& params, Json::Value& JsonRes,OPEN_API_V3_METHOD method=OPEN_API_V3_METHOD_GET, OPEN_API_V3_PROTOCOL protocol=OPEN_API_V3_METHOD_HTTP,double timeout_sec=OPEN_API_v3_DEFAULT_TIME_OUT);
	

	/***********************************************************
	**������ api
	**����:   
	**			script_name : ���õ�API����������/v3/user/get_info���ο� http://wiki.open.qq.com/wiki/API_V3.0%E6%96%87%E6%A1%A3
	**			params : ����APIʱ���Ĳ���
	**			FileParam:�ϴ����ļ�����
	**			protocol: Э������֧��HTTP��HTTPS
	**			timeout_sec:��ʱʱ��
	**
	**���:  JsonRes  ������Ӧ��
	**				
	**����:0 �ɹ�
	               ����ʧ��
	**����:
	**			  �����ļ����ϴ�,���ط���������Ӧ��
	**************************************************************/
	int  apiUploadFile(string& script_name, map<string,string>& params,Json::Value& JsonRes,struct FormFileElement& FileParam, OPEN_API_V3_PROTOCOL protocol=OPEN_API_V3_METHOD_HTTP,double timeout_sec=OPEN_API_v3_DEFAULT_TIME_OUT);

	/***********************************************************
	**������ GetErrMsg
	**����:   
	**			��
	**
	**���:  
	**				
	**����:��
	**����:
	**			���س�����Ϣ
	**************************************************************/
    const char* GetErrMsg()
    {
        return m_errmsg;
    }

	/***********************************************************
	**������ verifySig
	**����:   
	**			method    ���󷽷� "get" or "post"
	**			url_path
	**			params   ��Ѷ���÷����ص�URLЯ�����������
	**			sig         ��Ѷ���÷����ص�URLʱ���ݵ�ǩ��
	**
	**���:  
	**				
	**����:��
	**����:
	**			��֤�ص�����URL��ǩ�� (ע�����ͨ��OpenAPIǩ���㷨��һ�������
	**                http://wiki.open.qq.com/wiki/%E5%9B%9E%E8%B0%83%E5%8F%91%E8%B4%A7URL%E7%9A%84%E5%8D%8F%E8%AE%AE%E8%AF%B4%E6%98%8E_V3)
	**************************************************************/
	bool verifySig(OPEN_API_V3_METHOD method,string& url_path, map<string,string>& params,string& sig);

    string generatesig(OPEN_API_V3_METHOD method,string& url_path, map<string,string>& params);

	private:

	/***********************************************************
	**������ printRequest
	**����:   
	**			url     
	**			params   �������
	**			method   ���󷽷� "get" or "post"
	**
	**���:  
	**				
	**����:��
	**����:
	**			��ӡ������Ϣ
	**************************************************************/
	void printRequest(const string& url,map<string,string>& params,const string& method);	

	/***********************************************************
	**������ IsOpenId
	**����:   
	**			OpenId : 
	
	**���:  ��
	**				
	**����:true   �Ϸ�
				   false  �Ƿ�
	**����:
	**			 �ж�openid�ĸ�ʽ�Ƿ�Ϸ�
	**************************************************************/
	bool IsOpenId(string& OpenId);
	void StatReport(int iRet,struct timeval& start_tv,string& pf,string& script_name);
	int ParamIsValid(map<string,string>& params,OPEN_API_V3_METHOD method);
	int JsonResolve(Json::Value& JsonRes,string& res);
	unsigned int m_appid;
	std::string  m_stappid;
	std::string m_appkey ;
	std::string m_server_name;
	std::string m_format ;
	std::string m_stat_url;
	bool m_is_stat;
	bool m_debugswitch;
	char m_errmsg[512];
	int m_initflag;
	class HttpRequestHelper HttpRequestApi;
	class COpenApiSnsStatClient MonitorReport;
	class CSnsSigCheck  SigCheckApi;
	int m_iRet;	
}COpenApiV3;
#endif
