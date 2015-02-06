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
	**函数名 构造函数
	**输入:   
	**			uiAppid 应用的ID
				stAppkey 应用的kye

	**输出:  无
	**				
	**返回: 无
	**描述:
	**			  
	**************************************************************/

	OpenApiV3(unsigned int& uiAppid, string& stAppkey)
	{
		m_appid = uiAppid;
		m_appkey = stAppkey;
		m_initflag=0;

	}

	/***********************************************************
	**函数名 init
	**输入:   
	**			stServer_name  openapi的域名或地址，如果是调试请传入"119.147.19.43"
	**			stStat_url : 统计域名，这个最好不要对其进行更改
	**			format:  有json和xml,现在SDK只提供JSON的解析库，建议此值不改
	**			IsStat: 是否进行统计，建议开启，有助于问题排查
	**
	**输出:  JsonRes  请求响应包
	**				
	**返回:无
	**描述:
	**			 进行API接口的初始化，调用接口时，请先调用这个接口，否则会出错
	**************************************************************/
	void init(string stServer_name="openapi.tencentyun.com",string stStat_url="apistat.tencentyun.com",
			  string format="json",bool IsStat=true);	
	
	/***********************************************************
	**函数名 setServerName
	**输入:   
	**			stServer_name，openapi服务器的地址
	**输出:  
	**				
	**返回:无
	**描述: 设置openapi的地址
	**			  
	**************************************************************/
	void setServerName(string& stServer_name)
	{
		m_server_name = stServer_name;
	}
	
	/***********************************************************
	**函数名 setStatUrl
	**输入:   
	**			stStat_url
	**输出:  
	**				
	**返回:无
	**描述:设置统计服务器的地址,当前只能为"apistat.tencentyun.com"
	**			  
	**************************************************************/
	void setStatUrl(string& stStat_url)
	{
		m_stat_url = stStat_url;
	}

	/***********************************************************
	**函数名 setStatUrl
	**输入:   
	**			stStat_url
	**输出:  
	**				
	**返回:无
	**描述:设置统计服务器的地址
	**			  
	**************************************************************/	
	void  setIsStat(bool is_stat)
	{
		m_is_stat = is_stat;
	}

	/***********************************************************
	**函数名 setDebugSwitch
	**输入:   
	**			setDebugSwitch
	**输出:  
	**				
	**返回:无
	**描述:设置调试开关
	**			  
	**************************************************************/	
	void setDebugSwitch(bool debugswitch)
	{
		m_debugswitch = debugswitch;
	}
	

	/***********************************************************
	**函数名 api
	**输入:   
	**			script_name : 调用的API方法，比如/v3/user/get_info，参考 http://wiki.open.qq.com/wiki/API_V3.0%E6%96%87%E6%A1%A3
	**			params : 调用API时带的参数
	**			method : OPEN_API_V3_METHOD_GET 或OPEN_API_V3_METHOD_POST 其他非法
	**			protocol: 协议类型支持HTTP和HTTPS
	**			timeout_sec:超时时间
	**
	**输出:  JsonRes  请求响应包
	**				
	**返回:0 成功
	               其他失败
	**描述:
	**			  执行API调用，返回openapi server服务器返回的响应包
	**************************************************************/
	int api(string& script_name, map<string,string>& params, Json::Value& JsonRes,OPEN_API_V3_METHOD method=OPEN_API_V3_METHOD_GET, OPEN_API_V3_PROTOCOL protocol=OPEN_API_V3_METHOD_HTTP,double timeout_sec=OPEN_API_v3_DEFAULT_TIME_OUT);
	

	/***********************************************************
	**函数名 api
	**输入:   
	**			script_name : 调用的API方法，比如/v3/user/get_info，参考 http://wiki.open.qq.com/wiki/API_V3.0%E6%96%87%E6%A1%A3
	**			params : 调用API时带的参数
	**			FileParam:上传的文件参数
	**			protocol: 协议类型支持HTTP和HTTPS
	**			timeout_sec:超时时间
	**
	**输出:  JsonRes  请求响应包
	**				
	**返回:0 成功
	               其他失败
	**描述:
	**			  进行文件的上传,返回服务器的响应包
	**************************************************************/
	int  apiUploadFile(string& script_name, map<string,string>& params,Json::Value& JsonRes,struct FormFileElement& FileParam, OPEN_API_V3_PROTOCOL protocol=OPEN_API_V3_METHOD_HTTP,double timeout_sec=OPEN_API_v3_DEFAULT_TIME_OUT);

	/***********************************************************
	**函数名 GetErrMsg
	**输入:   
	**			无
	**
	**输出:  
	**				
	**返回:无
	**描述:
	**			返回出错信息
	**************************************************************/
    const char* GetErrMsg()
    {
        return m_errmsg;
    }

	/***********************************************************
	**函数名 verifySig
	**输入:   
	**			method    请求方法 "get" or "post"
	**			url_path
	**			params   腾讯调用发货回调URL携带的请求参数
	**			sig         腾讯调用发货回调URL时传递的签名
	**
	**输出:  
	**				
	**返回:无
	**描述:
	**			验证回调发货URL的签名 (注意和普通的OpenAPI签名算法不一样，详见
	**                http://wiki.open.qq.com/wiki/%E5%9B%9E%E8%B0%83%E5%8F%91%E8%B4%A7URL%E7%9A%84%E5%8D%8F%E8%AE%AE%E8%AF%B4%E6%98%8E_V3)
	**************************************************************/
	bool verifySig(OPEN_API_V3_METHOD method,string& url_path, map<string,string>& params,string& sig);

    string generatesig(OPEN_API_V3_METHOD method,string& url_path, map<string,string>& params);

	private:

	/***********************************************************
	**函数名 printRequest
	**输入:   
	**			url     
	**			params   请求参数
	**			method   请求方法 "get" or "post"
	**
	**输出:  
	**				
	**返回:无
	**描述:
	**			打印请求信息
	**************************************************************/
	void printRequest(const string& url,map<string,string>& params,const string& method);	

	/***********************************************************
	**函数名 IsOpenId
	**输入:   
	**			OpenId : 
	
	**输出:  无
	**				
	**返回:true   合法
				   false  非法
	**描述:
	**			 判断openid的格式是否合法
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
