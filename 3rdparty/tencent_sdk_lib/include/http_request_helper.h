/**
 * ����HTTP����������
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
	// form�����ļ�Ԫ�ص�name eg <input type="file" name="picture" >, field_name = picture
	string field_name;		

	// �����ļ�·�����ļ���(�����Ǿ���·��, ����������) curl������ļ���������
	string file_full_name;	
	
	// ���͵����ݰ��а������ļ���. ���û������file_name_send, ��ô��basename(file_full_name)��Ϊ�ļ���
	// �˲�������:�ڶԷ��յ����ݰ�ʱ, ��file_name_for_peer��Ϊ���ļ���.
	// ����: Content-Disposition: form-data; name="picture"; filename="16.png" // 16.png����file_name_for_peer
	string file_name_for_peer;	
};

class HttpRequestHelper
{

public:
	/***********************************************************
	**������ GetHttpRequest
	**����:   
				 url : http://IP:PORT/CGI or  http://host:port/cgi
				 parasm : ����
				 timeout_sec: ��ʱʱ��
	**���:  
				rsp ��Ӧbody (����header), ʧ��ʱ���س�����Ϣ
	**				
	**����:  0    �ɹ�
	**		   ����ʧ��
	**����:ģ�ⷢ��Http Get Request
	**************************************************************/		
	int GetHttpRequest(string url, map<string, string>& params, float timeout_sec, string& rsp);	


	/***********************************************************
	**������ PostHttpRequest
	**����:   
				 url : http://IP:PORT/CGI or  http://host:port/cgi
				 parasm : ����
				 timeout_sec: ��ʱʱ��
	**���:  
				rsp ��Ӧbody (����header), ʧ��ʱ���س�����Ϣ
	**				
	**����:  0    �ɹ�
	**		   ����ʧ��
	**����:ģ�ⷢ��Http Post Request
	**************************************************************/
	int PostHttpRequest(string url, map<string, string>& params, float timeout_sec, string& rsp);

	/***********************************************************
	**������ PostHttpRequest
	**����:   
				 url : http://IP:PORT/CGI or  http://host:port/cgi
				 parasm : ���� (��map<string,string>& param�ĳ�multimap<string,string>��ʽ)
				 timeout_sec: ��ʱʱ��
	**���:  
				rsp ��Ӧbody (����header), ʧ��ʱ���س�����Ϣ
	**				
	**����:  0    �ɹ�
	**		   ����ʧ��
	**����:ģ�ⷢ��Http Post Request
	**************************************************************/	
    int PostHttpRequest(const string& url, const multimap<string,string>& params,  float timeout_sec, string& rsp);

	/***********************************************************
	**������ PostHttpRequestWithFile
	**����:   
				 url : http://IP:PORT/CGI or  http://host:port/cgi
				 parasm : ����
				 file : �ļ������ļ��ı�����
				 timeout_sec: ��ʱʱ��
	**���:  
				rsp ��Ӧbody (����header), ʧ��ʱ���س�����Ϣ
	**				
	**����:  0    �ɹ�
	**		   ����ʧ��
	**����:ģ�ⷢ��Http Post Request
	**************************************************************/
	int PostHttpRequestWithFile(string url, map<string, string>& params, const FormFileElement &file, float timeout_sec, string& rsp);

	
	
	/***********************************************************
	**������ SetHost
	**����:   
				 host   ��������IP
	**���:  
				
	**				
	**����:  0    �ɹ�
	**		   ����ʧ��
	**����:����HTTP Header��Host��Ϣ
			  Ĭ�Ͽ��Ժ���, ����һЩ��֧��DNS�����Ļ���, ��������host, �Ա�ͨ�����·�ʽģ��dns
			  urlͨ��IP����cgi, headerͷ��hostָ������
	**************************************************************/
	void SetHost(const string &host)
	{
		m_host = host;
	}

	/***********************************************************
	**������ GetHost
	**����:   ��
				 
	**���:  ��
				
	**				
	**����:  ��������IP
	**����:
	**************************************************************/
    string& GetHost(void)
    {
        return m_host;
    }

	
	/***********************************************************
	**������ GetErrMsg
	**����:   ��
				 
	**���:  ��
				
	**				
	**����:  ������Ϣ
	**����:
	**************************************************************/
	char* GetErrMsg();
private:

	/***********************************************************
	**������ EncodeParams
	**����:   params  ����
				 
	**���:  
				
	**				
	**����:  URL�����Ĳ���
	**����:
	**************************************************************/
	string EncodeParams(map<string, string> &params);
    string EncodeParams(const multimap<string,string>& params);
private:

private:
	char m_szErrMsg[512];
	string m_host;


};

#endif
