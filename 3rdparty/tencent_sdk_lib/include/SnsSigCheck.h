/**
 * ����ǩ����
 *
 * @version 3.0.1
 * @author open.qq.com
 * @copyright (c) 2013, Tencent Corporation. All rights reserved.
 * @ History:
 *				 3.0.1 | harryhlli  | 2013-09-06 09:50:19 | fixed compilation errors
 *               3.0.0 | jixingguan | 2013-05-27 11:11:11 | initialization
 */
#ifndef TENCENT_OPEN_API_SNS_CHECK_H
#define TENCENT_OPEN_API_SNS_CHECK_H
#include <string>
#include <map>
#include <algorithm>
#include <stdio.h>

/**
 * ����ǩ����
 */
using namespace std;
class CSnsSigCheck
{
	public:
	CSnsSigCheck(){};
	~CSnsSigCheck() {};

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
	const char* GetErrMsg() { return m_szErrMsg;}

	/***********************************************************
	**������ makeSig
	**����:   
	**			method ���󷽷� "get" or "post"
				url_path   openapi����
				params    �������
				secret      ��Կ
	**���:  
	**				
	**����:  ǩ��
	**����: ����ǩ��
	**			  
	**************************************************************/	
    string makeSig(string& method, string& url_path, map<string, string>& params, string& secret);

	 /***********************************************************
	 **������ encodevalue
	 **����:   
	 ** 		 method ���󷽷� "get" or "post"
				 url_path	openapi����
				 params    �������
				 secret 	 ��Կ
	 **���:  
	 ** 			 
	 **����:  ǩ��
	 **����: �ص�����URLר�õı����㷨
	 **            �������Ϊ������ 0~9 a~z A~Z !*()֮�������ַ�����ASCII���ʮ�����Ƽ�%���б�ʾ������"-"����Ϊ"%2D"
	 **		  http://wiki.open.qq.com/wiki/%E5%9B%9E%E8%B0%83%E5%8F%91%E8%B4%A7URL%E7%9A%84%E5%8D%8F%E8%AE%AE%E8%AF%B4%E6%98%8E_V3
	 **************************************************************/ 
	 string encodevalue(string& value);

	 /***********************************************************
	 **������ join_params
	 **����:   
	 ** 		 params  �������
	 **���:  
	 ** 			 
	 **����:  ƴ�Ӻ�Ĳ���
	 **����: ��������&����ƴ��
	 ** 		   
	 **************************************************************/ 
	 string join_params(map<string,string> &params);
    
 private:
	string url_encode(const string& src);	
	string replace_str(string src,const string& old_value,const string& new_value);
	char m_szErrMsg[512];

};
#endif
