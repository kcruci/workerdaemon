/**
 * 发送HTTP网络请求类
 *
 * @version 3.0.1
 * @author open.qq.com
 * @copyright (c) 2013, Tencent Corporation. All rights reserved.
 * @ History:	 
 *				 3.0.1 | harryhlli  | 2013-09-06 09:50:19 | fixed compilation errors
 *               3.0.0 | jixingguan | 2013-05-27 15:33:04 | initialization				 
 */

#include "http_request_helper.h"
#include "curl/curl.h"
#include "uri_new.h"
#include <sstream>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <libgen.h>

using namespace std;


struct StConnFree
{
	StConnFree(CURL* conn)
	{
		m_conn = conn;
	}
	~StConnFree()
	{
		if (m_conn)
		{
			curl_easy_cleanup(m_conn);
			m_conn = NULL;
		}
	}
	CURL* m_conn;
};

struct StSlistFree
{
    StSlistFree(struct curl_slist *list)
    {
        m_list = list;
    }
    ~StSlistFree()
    {
        if (m_list)
        {
            curl_slist_free_all (m_list);
        }
    }
    struct curl_slist *m_list;
};

struct StHttpPostFree
{
    StHttpPostFree(struct curl_httppost *post)
    {
        m_post= post;
    }
    ~StHttpPostFree()
    {
        if (m_post)
        {
            curl_formfree(m_post);
        }
    }
    struct curl_httppost *m_post;
};

/***********************************************************
**函数名 GetErrMsg
**输入:   无
             
**输出:  无
            
**              
**返回:  错误信息
**描述:
**************************************************************/
char* HttpRequestHelper::GetErrMsg()
{
	return m_szErrMsg;
}

/***********************************************************
**函数名 EncodeParams
**输入:   params  参数
             
**输出:  
            
**              
**返回:  URL编码后的参数
**描述:
**************************************************************/
string HttpRequestHelper::EncodeParams(map<string, string> &params)
{
	stringstream ss;
	for(map<string, string>::iterator it = params.begin(); it != params.end(); ++it)
	{
		if (it != params.begin())
		{
			ss << "&";
		}
		ss << it->first;
		ss << "=";
		ss << WebCharacter_NEW::encodeURIValue(it->second.c_str());
	}

	return ss.str();
}

/***********************************************************
**函数名 EncodeParams
**输入:   params  参数
             
**输出:  
            
**              
**返回:  URL编码后的参数
**描述:
**************************************************************/
string HttpRequestHelper::EncodeParams(const multimap<string,string>& params)
{
    stringstream ss;
    multimap<string,string>::const_iterator iter = params.begin();
    while( iter != params.end() )
    {
        ss<< iter->first
          <<"="
          << WebCharacter_NEW::encodeURIValue(iter->second.c_str())
          <<"&";
        ++iter;
    }

    string line = ss.str();
    line.erase( line.size() -1 );

    return line;
}

int static responesWriteCallback(void *ptr, size_t size, size_t nmemb, 
											 void *userdata)
{
	string * ptrStrRes = (string *)userdata;

	unsigned long sizes = size * nmemb;
	if (!ptr)
	{
		return 0;
	}
	(*ptrStrRes).append((char *)ptr, sizes);

	return sizes;
}

/***********************************************************
**函数名 GetHttpRequest
**输入:   
             url : http://IP:PORT/CGI or  http://host:port/cgi
             parasm : 参数
             timeout_sec: 超时时间
**输出:  
            rsp 响应body (不含header), 失败时返回出错信息
**              
**返回:  0    成功
**         其他失败
**描述:模拟发送Http Get Request
**************************************************************/     
int HttpRequestHelper::GetHttpRequest(string url, map<string, string>& params, float timeout_sec, string& rsp)
{
	string full_url;

	stringstream ss;
	ss<<url.c_str();
	ss<<"?";
	ss<<EncodeParams(params).c_str();
	full_url = ss.str();

	CURL *curl = 0;
	CURLcode ret;

	curl = curl_easy_init();
	if (0 == curl)
	{
		snprintf(m_szErrMsg, sizeof(m_szErrMsg), "curl_easy_init() failed");
		return -1;
	}

	StConnFree conn_free(curl);
    struct curl_slist *headerlist=NULL;

	char buf[128] = {0};
	
	// disable Expect: 100-continue 
	snprintf(buf, sizeof(buf), "%s", "Expect:");
    headerlist = curl_slist_append(headerlist, buf);

    //printf("bafore Host: %s\n", m_host.c_str());

	if (!m_host.empty())
	{
		snprintf(buf, sizeof(buf), "Host: %s", m_host.c_str());
		headerlist = curl_slist_append(headerlist, buf);
		//printf("Host: %s\n", m_host.c_str());
	}

    StSlistFree slist_free(headerlist);
    
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);

	static char errorBuffer[CURL_ERROR_SIZE];

	curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorBuffer);
	curl_easy_setopt(curl, CURLOPT_URL, full_url.c_str()); // ok
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &rsp); // ok
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, responesWriteCallback); // ok

	curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, (int)(timeout_sec*1000));
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, (int)(timeout_sec*1000));

	curl_easy_setopt(curl,CURLOPT_SSL_VERIFYPEER,false);
	curl_easy_setopt(curl,CURLOPT_SSL_VERIFYHOST,false);

	ret = curl_easy_perform(curl);
	if (ret != CURLE_OK) // CURLE_OK : 0
	{
		snprintf(m_szErrMsg, sizeof(m_szErrMsg), "Failed to get '%s' [%s]\n", full_url.c_str(), errorBuffer);
		return ret;
	}
	else
	{//如果访问正常但http状态码不是200，直接返回http的状态码
		int http_code = 0;
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
		if(200 != http_code)
		{
			snprintf(m_szErrMsg, sizeof(m_szErrMsg), "succ to get '%s',but response http_code[%d]\n", full_url.c_str(), http_code);
			return http_code;
		}
		return 0;//状态码为200时返回0
	}
}

/***********************************************************
**函数名 PostHttpRequest
**输入:   
             url : http://IP:PORT/CGI or  http://host:port/cgi
             parasm : 参数
             timeout_sec: 超时时间
**输出:  
            rsp 响应body (不含header), 失败时返回出错信息
**              
**返回:  0    成功
**         其他失败
**描述:模拟发送Http Post Request
**************************************************************/
int HttpRequestHelper::PostHttpRequest(string url, map<string, string>& params, float timeout_sec, string& rsp)
{
    multimap<string,string> NewParams(params.begin(), params.end());
    return PostHttpRequest(url, NewParams, timeout_sec, rsp);
}

/***********************************************************
**函数名 PostHttpRequest
**输入:   
             url : http://IP:PORT/CGI or  http://host:port/cgi
             parasm : 参数 (把map<string,string>& param改成multimap<string,string>形式)
             timeout_sec: 超时时间
**输出:  
            rsp 响应body (不含header), 失败时返回出错信息
**              
**返回:  0    成功
**         其他失败
**描述:模拟发送Http Post Request
**************************************************************/ 
int HttpRequestHelper::PostHttpRequest(const string& url, const multimap<string,string>& params,  float timeout_sec, string& rsp)
{
	CURL *curl = 0;
	CURLcode ret;

	curl = curl_easy_init();
	if (0 == curl)
	{
		snprintf(m_szErrMsg, sizeof(m_szErrMsg), "curl_easy_init() failed");
		return -1;
	}
	StConnFree conn_free(curl);
    struct curl_slist *headerlist=NULL;

	char buf[128] = {0};
	
	// disable Expect: 100-continue 
	snprintf(buf, sizeof(buf), "%s", "Expect:");
    headerlist = curl_slist_append(headerlist, buf);

	if (!m_host.empty())
	{
		snprintf(buf, sizeof(buf), "Host: %s", m_host.c_str());
		headerlist = curl_slist_append(headerlist, buf);
	}

    StSlistFree slist_free(headerlist);
    
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);

	static char errorBuffer[CURL_ERROR_SIZE];

	curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorBuffer);
	curl_easy_setopt(curl, CURLOPT_URL, url.c_str()); // ok
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &rsp); // ok
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, responesWriteCallback); // ok

	curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, (int)(timeout_sec*1000));
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, (int)(timeout_sec*1000));
	
	curl_easy_setopt(curl,CURLOPT_SSL_VERIFYPEER,false);
	curl_easy_setopt(curl,CURLOPT_SSL_VERIFYHOST,false);


	//post fields
	curl_easy_setopt(curl, CURLOPT_POST, 1);
    //必须先把string单独生成出来，如果直接调用setopt的话，会导致postfields为空，很奇怪。
    //但是cookie又不会
    string encode_params = EncodeParams(params);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, encode_params.c_str());

	ret = curl_easy_perform(curl);
	if (ret != CURLE_OK) // CURLE_OK : 0
	{
		snprintf(m_szErrMsg, sizeof(m_szErrMsg), "Failed to get '%s' [%s]\n", url.c_str(), errorBuffer);
		return ret;
	}
	else
	{//如果访问正常但http状态码不是200，直接返回http的状态码
		int http_code = 0;
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
		if(200 != http_code)
		{
			snprintf(m_szErrMsg, sizeof(m_szErrMsg), "succ to get '%s',but response http_code[%d]\n", url.c_str(), http_code);
			return http_code;
		}
		return 0;//状态码为200时返回0
	}

}

/***********************************************************
**函数名 PostHttpRequestWithFile
**输入:   
             url : http://IP:PORT/CGI or  http://host:port/cgi
             parasm : 参数
             file : 文件包含文件的表单参数
             timeout_sec: 超时时间
**输出:  
            rsp 响应body (不含header), 失败时返回出错信息
**              
**返回:  0    成功
**         其他失败
**描述:模拟发送Http Post Request
**************************************************************/
int HttpRequestHelper::PostHttpRequestWithFile(string url, map<string, string>& params, const struct FormFileElement &file, float timeout_sec, string& rsp)
{
    // refer: 
	//  1 : http://curl.haxx.se/libcurl/c/multi-post.html
	//  2 : http://curl.haxx.se/libcurl/c/curl_formadd.html

	CURL *curl = 0;
	CURLcode ret;

	curl = curl_easy_init();
	if (0 == curl)
	{
		snprintf(m_szErrMsg, sizeof(m_szErrMsg), "curl_easy_init() failed");
		return -1;
	}
	StConnFree conn_free(curl);

	static char errorBuffer[CURL_ERROR_SIZE];

    struct curl_httppost *formpost=NULL;
    struct curl_httppost *lastptr=NULL;
    struct curl_slist *headerlist=NULL;

	string file_name_for_peer = file.file_name_for_peer;
	if (file_name_for_peer.empty())
	{
		file_name_for_peer = basename((char *)file.file_full_name.c_str());
	}
    
	// 增加文件表单项
    curl_formadd(&formpost,
               &lastptr,
               CURLFORM_COPYNAME, file.field_name.c_str(),
               CURLFORM_FILE, file.file_full_name.c_str(),
			   CURLFORM_FILENAME, file_name_for_peer.c_str(),
               CURLFORM_END);

	// 增加普通表单项
    for (map<string, string>::iterator it=params.begin(); 
            it!=params.end(); ++it)
    {
        curl_formadd(&formpost,
               &lastptr,
               CURLFORM_COPYNAME, it->first.c_str(),
               CURLFORM_COPYCONTENTS, it->second.c_str(),
               CURLFORM_END); 
    }

    StHttpPostFree post_free(formpost);

	
	char buf[128] = {0};
	
	// disable Expect: 100-continue 
	snprintf(buf, sizeof(buf), "%s", "Expect:");
    headerlist = curl_slist_append(headerlist, buf);

	if (!m_host.empty())
	{
		snprintf(buf, sizeof(buf), "Host: %s", m_host.c_str());
		headerlist = curl_slist_append(headerlist, buf);
	}

    StSlistFree slist_free(headerlist);
    
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
    curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);

	curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorBuffer);
	curl_easy_setopt(curl, CURLOPT_URL, url.c_str()); // ok
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &rsp); // ok
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, responesWriteCallback); // ok
	curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, (int)(timeout_sec*1000));
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, (int)(timeout_sec*1000));
	
	curl_easy_setopt(curl,CURLOPT_SSL_VERIFYPEER,false);
	curl_easy_setopt(curl,CURLOPT_SSL_VERIFYHOST,false);

	ret = curl_easy_perform(curl);
	if (ret != CURLE_OK) // CURLE_OK : 0
	{
		snprintf(m_szErrMsg, sizeof(m_szErrMsg), "Failed to get '%s' curl errcode:%d, errmsg:[%s]\n",
			url.c_str(), 
			ret,
			errorBuffer
			);
		return ret;
	}
	else
	{//如果访问正常但http状态码不是200，直接返回http的状态码
		int http_code = 0;
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
		if(200 != http_code)
		{
			snprintf(m_szErrMsg, sizeof(m_szErrMsg), "succ to get '%s',but response http_code[%d]\n", url.c_str(), http_code);
			return http_code;
		}
		return 0;//状态码为200时返回0
	}
}


