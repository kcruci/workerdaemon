/**
 * 发送HTTP网络请求类
 *
 * @version 3.0.0
 * @author open.qq.com
 * @copyright (c) 2013, Tencent Corporation. All rights reserved.
 * @ History:	 
 *               3.0.0 | jixingguan | 2013-05-16 15:33:04 | initialization				 
 */

#ifndef __HTTP_REQUEST_HELPER_H__
#define __HTTP_REQUEST_HELPER_H__

#include <map>
#include <string>
#include "uri_new.h"
using namespace std;

struct FormFileElement
{
	// form表单的文件元素的name eg <input type="file" name="picture" >, field_name = picture
	string field_name;		

	// 包含文件路径的文件名(必须是绝对路径, 可以是软链) curl将会从文件加载数据
	string file_full_name;	
	
	// 发送的数据包中包含的文件名. 如果没有设置file_name_send, 那么将basename(file_full_name)设为文件名
	// 此参数作用:在对方收到数据包时, 将file_name_for_peer作为其文件名.
	// 举例: Content-Disposition: form-data; name="picture"; filename="16.png" // 16.png就是file_name_for_peer
	string file_name_for_peer;	
};

class HttpRequestHelper
{

public:
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
	**		   其他失败
	**描述:模拟发送Http Get Request
	**************************************************************/		
	int GetHttpRequest(string url, map<string, string>& params, float timeout_sec, string& rsp);	


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
	**		   其他失败
	**描述:模拟发送Http Post Request
	**************************************************************/
	int PostHttpRequest(string url, map<string, string>& params, float timeout_sec, string& rsp);

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
	**		   其他失败
	**描述:模拟发送Http Post Request
	**************************************************************/	
    int PostHttpRequest(const string& url, const multimap<string,string>& params,  float timeout_sec, string& rsp);

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
	**		   其他失败
	**描述:模拟发送Http Post Request
	**************************************************************/
	int PostHttpRequestWithFile(string url, map<string, string>& params, const FormFileElement &file, float timeout_sec, string& rsp);

	
	
	/***********************************************************
	**函数名 SetHost
	**输入:   
				 host   服务器的IP
	**输出:  
				
	**				
	**返回:  0    成功
	**		   其他失败
	**描述:设置HTTP Header的Host信息
			  默认可以忽略, 对于一些不支持DNS解析的机器, 可以设置host, 以便通过如下方式模拟dns
			  url通过IP访问cgi, header头部host指明域名
	**************************************************************/
	void SetHost(const string &host)
	{
		m_host = host;
	}

	/***********************************************************
	**函数名 GetHost
	**输入:   无
				 
	**输出:  无
				
	**				
	**返回:  服务器的IP
	**描述:
	**************************************************************/
    string& GetHost(void)
    {
        return m_host;
    }

	
	/***********************************************************
	**函数名 GetErrMsg
	**输入:   无
				 
	**输出:  无
				
	**				
	**返回:  错误信息
	**描述:
	**************************************************************/
	char* GetErrMsg();
private:

	/***********************************************************
	**函数名 EncodeParams
	**输入:   params  参数
				 
	**输出:  
				
	**				
	**返回:  URL编码后的参数
	**描述:
	**************************************************************/
	string EncodeParams(map<string, string> &params);
    string EncodeParams(const multimap<string,string>& params);
private:

private:
	char m_szErrMsg[512];
	string m_host;


};

#endif
