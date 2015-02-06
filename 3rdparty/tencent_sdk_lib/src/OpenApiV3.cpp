/**
 * C++ SDK for  OpenAPI V3
 *
 * @version 3.0.0
 * @author open.qq.com
 * @copyright (c) 2013, Tencent Corporation. All rights reserved.
 * @ History:
 *               3.0.0 | jixingguan | 2013-05-06 10:18:11 | initialization
 */
#include "OpenApiV3.h"
#include "http_request_helper.h"
#include "SnsStat.h"
#include "openapi_comm_def.h"
#include <sys/time.h>

const string HTTP_METHOD_POST = "POST";
const string HTTP_METHOD_GET =  "GET";
const string PROTOCOL_HTTP="http";
const string PROTOCOL_HTTPS="https";

/***********************************************************
**函数名 init
**输入:   
**          stServer_name  openapi的域名或地址，如果是调试请传入"119.147.19.43"
**          stStat_url : 统计域名，这个最好不要对其进行更改
**          format:  有json和xml,现在SDK只提供JSON的解析库，建议此值不改
**          IsStat: 是否进行统计，建议开启，有助于问题排查
**
**输出:  JsonRes  请求响应包
**              
**返回:无
**描述:
**           进行API接口的初始化，调用接口时，请先调用这个接口，否则会出错
**************************************************************/
void OpenApiV3::init(string stServer_name,string stStat_url,string format,bool IsStat)
{
    if(  0 == m_initflag )
    { 
        char stAppid[32]={'\0'};
        snprintf(stAppid,sizeof(stAppid),"%d",m_appid);
        m_stappid = stAppid;
        m_server_name = stServer_name;
        m_format = format;
        m_stat_url = stStat_url;
        m_initflag=1;
        m_is_stat=IsStat;
        m_debugswitch =0;
    } 
}


/***********************************************************
**函数名 api
**输入:   
**          script_name : 调用的API方法，比如/v3/user/get_info，参考 http://wiki.open.qq.com/wiki/API_V3.0%E6%96%87%E6%A1%A3
**          params : 调用API时带的参数
**          method : OPEN_API_V3_METHOD_GET 或OPEN_API_V3_METHOD_POST 其他非法
**          protocol: 协议类型支持HTTP和HTTPS
**          timeout_sec:超时时间
**
**输出:  JsonRes  请求响应包
**              
**返回:0 成功
               其他失败
**描述:
**            执行API调用，返回openapi server服务器返回的响应包
**************************************************************/
int OpenApiV3::api(string& script_name, map<string,string>& params, Json::Value& JsonRes ,OPEN_API_V3_METHOD method, OPEN_API_V3_PROTOCOL protocol,double timeout_sec)
{
            
    int iRet=0;
    m_iRet=0;

    iRet=ParamIsValid(params,method);
    if( 0 != iRet)
    {
        return iRet;
    }

    // 无需传sig, 会自动生成
    params.erase("sig");
    
    // 添加一些参数    
    params["appid"] = m_stappid;
    params["format"] = m_format;
    
    // 生成签名
    string HttpMethod;
    if( OPEN_API_V3_METHOD_GET ==  method)
    {   
        HttpMethod = HTTP_METHOD_GET;   
    }
    else
    {
        
        HttpMethod = HTTP_METHOD_POST;   
    }
    
    
    string secret = m_appkey +"&";
    string sig = SigCheckApi.makeSig(HttpMethod, script_name, params, secret);
    params["sig"] = sig;
     
    string url;
    if( OPEN_API_V3_METHOD_HTTP == protocol)
    {
        url = PROTOCOL_HTTP+"://"+m_server_name+script_name;
    }
    else
    {
        url = PROTOCOL_HTTPS+"://"+m_server_name+script_name;
    }

    if( m_debugswitch)
    {
        //打印出最终发送到openapi服务器的请求参数以及url
        printRequest(url,params,HttpMethod);
    }   
    
    //记录接口调用开始时间
    struct timeval start_tv;
    gettimeofday(&start_tv, NULL);

    string res;
    if( OPEN_API_V3_METHOD_GET  == method)
    {
        iRet = HttpRequestApi.GetHttpRequest(url, params,timeout_sec,res);
    }
    else
    {
        iRet = HttpRequestApi.PostHttpRequest(url, params,timeout_sec,res);
    }

    
    if( 0 != iRet )
    {
        OPENAPIV3_ERROR("PostHttpRequest failed err[%s]",HttpRequestApi.GetErrMsg());
        m_iRet =(iRet+OPENAPI_ERROR_CURL);
    }
    else
    {
    
        if( m_debugswitch)
        {
            OutPutDebug("JsonRes=%s\n",res.c_str());
        }

        //对JSON数据进行解析    
        m_iRet=JsonResolve(JsonRes,res);
    } 

    // 统计上报    
    StatReport(m_iRet,start_tv,params["pf"],script_name);    
    
    return m_iRet;
}

/***********************************************************
**函数名 apiUploadFile
**输入:   
**          script_name : 调用的API方法，比如/v3/user/get_info，参考 http://wiki.open.qq.com/wiki/API_V3.0%E6%96%87%E6%A1%A3
**          params : 调用API时带的参数
**          FileParam:上传的文件参数
**          protocol: 协议类型支持HTTP和HTTPS
**          timeout_sec:超时时间
**
**输出:  JsonRes  请求响应包
**              
**返回:0 成功
               其他失败
**描述:
**            进行文件的上传,返回服务器的响应包
**************************************************************/
int  OpenApiV3::apiUploadFile(string& script_name, map<string,string>& params,Json::Value& JsonRes,struct FormFileElement& FileParam, OPEN_API_V3_PROTOCOL protocol,double timeout_sec)
{
    int iRet=0;
    m_iRet=0;
    
    iRet=ParamIsValid(params,OPEN_API_V3_METHOD_POST);
    if( 0 != iRet)
    {
        return iRet;
    }
    
    // 无需传sig, 会自动生成
    params.erase("sig");

    // 添加一些参数
    params["appid"] = m_stappid;
    params["format"] = m_format;

    string HttpMethod = HTTP_METHOD_POST; 
    // 生成签名
    string secret = m_appkey +"&";
    string sig = SigCheckApi.makeSig(HttpMethod, script_name, params, secret);
    params["sig"] = sig;
    
    string url;
    if( OPEN_API_V3_METHOD_HTTP == protocol)
    {
        url = PROTOCOL_HTTP+"://"+m_server_name+script_name;
    }
    else
    {
        url = PROTOCOL_HTTPS+"://"+m_server_name+script_name;
    }

    if( m_debugswitch)
    {
        //通过调用以下方法，可以打印出最终发送到openapi服务器的请求参数以及url
        printRequest(url,params,HttpMethod);
    }   
    
    
    //记录接口调用开始时间
    struct timeval start_tv;
    gettimeofday(&start_tv, NULL);

    string res;
    iRet= HttpRequestApi.PostHttpRequestWithFile(url,params,FileParam,timeout_sec,res);    
    if( 0 != iRet )
    {
        OPENAPIV3_ERROR("PostHttpRequest failed err[%s]",HttpRequestApi.GetErrMsg());
        m_iRet =(OPENAPI_ERROR_CURL+iRet);
    }
    else
    {
        if( m_debugswitch)
        {
            OutPutDebug("JsonRes=%s\n",res.c_str());
        }
        m_iRet=JsonResolve(JsonRes,res);

    }     
    
    // 统计上报
    StatReport(m_iRet,start_tv,params["pf"],script_name);    
    
    return m_iRet;
}

/***********************************************************
**函数名 ParamIsValid
**输入:   
**          params  传入参数
             params  get or port   
**输出:  无
**              
**返回:true   合法
               false  非法
**描述:
**           判断参数是否合法
**************************************************************/
int OpenApiV3::ParamIsValid(map<string,string>& params,OPEN_API_V3_METHOD method)
{
    //判断是否调用了初始化接口
    if( 0 == m_initflag)
    {
        OPENAPIV3_ERROR("must call init");
        return OPENAPI_ERROR_REQUIRED_PARAMETER_EMPTY;
    }    

    // 检查 openid 是否为空
    if (  params.find("openid") == params.end() )
    {
        OPENAPIV3_ERROR("openid is empty");
        return OPENAPI_ERROR_REQUIRED_PARAMETER_EMPTY;
    }

    if( (OPEN_API_V3_METHOD_GET !=  method) && ( OPEN_API_V3_METHOD_POST != method) )
    {
        OPENAPIV3_ERROR("method is invalid");
        return OPENAPI_ERROR_REQUIRED_PARAMETER_INVALID;
    }

    if( !IsOpenId(params["openid"]))
    {
        OPENAPIV3_ERROR("openid is invalid");
        return OPENAPI_ERROR_REQUIRED_PARAMETER_INVALID;        
    }
    return 0;
}

/***********************************************************
**函数名 JsonResolve
**输入:   
**              res  待解析的字符串  
**输出:  JsonRes 解析后的JSON结构体
**              
**返回: 解析的返回值
**描述:
**           判断参数是否合法
**************************************************************/
int  OpenApiV3::JsonResolve(Json::Value& JsonRes,string& res)
{
    int iRet=0;
    //对JSON数据进行解析    
    Json::Reader _reader;
    
    if( !_reader.parse(res, JsonRes))
    {   
        OPENAPIV3_ERROR("Json::Reader parse Error, Msg=%s",_reader.getFormatedErrorMessages().c_str());
        iRet= OPENAPI_ERROR_RESPONSE_DATA_INVALID; 
    }
    else
    {
        iRet = JsonRes["ret"].asInt();
        if( 0 != iRet)
        {
            OPENAPIV3_ERROR("%s",JsonRes["msg"].asString().c_str());
        }
    }
    return iRet;
}

/***********************************************************
**函数名 StatReport
**输入:   
**              iRet       返回值
                 start_tv  接口开始的时间
                 pf           平台类型
                 script_name  接口名称
**输出:  无
**              
**返回: 无
**描述:
**           进行统计上报
**************************************************************/
void OpenApiV3::StatReport(int iRet,struct timeval& start_tv,string& pf,string& script_name)
{
    if (m_is_stat)
    {
        struct timeval now_tv;
        gettimeofday(&now_tv, NULL);
        double pasttime_sec = (now_tv.tv_sec  - start_tv.tv_sec ) + ((double)(now_tv.tv_usec - start_tv.tv_usec)) / 1000000;
        
        SPackage pack_req;
        pack_req.appid = m_appid;
        pack_req.rc = iRet;
        pack_req.timestamp = time(NULL);
        pack_req.time = pasttime_sec;
        pack_req.pf = pf;
        pack_req.svr_name = m_server_name;
        pack_req.interface = script_name;
        pack_req.protocol = "http";
        pack_req.method = "POST";
        pack_req.collect_point = "sdk-c++-v3";               
        iRet=MonitorReport.Report(pack_req, m_stat_url);
        if( 0 != iRet)
        {
            OutPutDebug("MonitorReport err:%s",MonitorReport.GetErrMsg());    
        }
    }
    return;
}

/***********************************************************
**函数名 verifySig
**输入:   
**          method    请求方法 "get" or "post"
**          url_path
**          params   腾讯调用发货回调URL携带的请求参数
**          sig         腾讯调用发货回调URL时传递的签名
**
**输出:  
**              
**返回:无
**描述:
**          验证回调发货URL的签名 (注意和普通的OpenAPI签名算法不一样，详见
**                http://wiki.open.qq.com/wiki/%E5%9B%9E%E8%B0%83%E5%8F%91%E8%B4%A7URL%E7%9A%84%E5%8D%8F%E8%AE%AE%E8%AF%B4%E6%98%8E_V3)
**************************************************************/

bool OpenApiV3::verifySig(OPEN_API_V3_METHOD method,string& url_path, map<string,string>& params,string& sig)
{
  string sig_new;  
  string HttpMethod;

  if( OPEN_API_V3_METHOD_GET ==  method)
  {
       HttpMethod = HTTP_METHOD_GET;
  }
  else
  {
       HttpMethod =  HTTP_METHOD_POST;
  }
  
  foreach(params,it)
  {
       it->second = SigCheckApi.encodevalue(it->second); 
  }  
  
  string secret = m_appkey +"&";
  sig_new = SigCheckApi.makeSig(HttpMethod, url_path, params, secret);

  if( sig_new == sig )
  {
       return true;
  }

  return false; 
}

string OpenApiV3::generatesig(OPEN_API_V3_METHOD method,string& url_path, map<string,string>& params)
{
    string sig_new;
    string HttpMethod;

    if( OPEN_API_V3_METHOD_GET ==  method)
    {
        HttpMethod = HTTP_METHOD_GET;
    }
    else
    {
        HttpMethod =  HTTP_METHOD_POST;   
    }

    foreach(params,it)
    {
        it->second = SigCheckApi.encodevalue(it->second);
    }

    string secret = m_appkey +"&";
    sig_new = SigCheckApi.makeSig(HttpMethod, url_path, params, secret);
	sig_new = SigCheckApi.encodevalue(sig_new);

    return sig_new;
}

/***********************************************************
**函数名 IsOpenId
**输入:   
**          OpenId : 

**输出:  无
**              
**返回:true   合法
               false  非法
**描述:
**           判断openid的格式是否合法
**************************************************************/
bool OpenApiV3::IsOpenId(string& OpenId)
{
    if( 32 != OpenId.size())
    {
        return false;
    }

    char c;
    for (unsigned int i = 0; i < OpenId.size(); i++)
    {
      c = OpenId[i]; 
      
      if (!isalpha(c) && !isdigit(c))
      {
          return false;
      }
      else
      {
          continue;
      }
    }
    return true;    
}
void OpenApiV3::printRequest(const string& url,map<string,string>& params,const string& method)
{
    string urltemp; 
    string query_string = SigCheckApi.join_params(params);
    if(method == HTTP_METHOD_GET)
    {
        urltemp = url+"?"+query_string;
    }
    urltemp=url;
    OutPutDebug("\n============= request info ================\n\n");
    OutPutDebug("method : %s\n",method.c_str());
    OutPutDebug("url    : %s\n",urltemp.c_str());
    if(method == HTTP_METHOD_POST)
    {
        OutPutDebug("query_string : %s\n",query_string.c_str());
    }
    OutPutDebug("\n");
}

