/**
 * ����ǩ����
 *
 * @version 3.0.0
 * @author open.qq.com
 * @copyright (c) 2013, Tencent Corporation. All rights reserved.
 * @ History:
 *               3.0.0 | jixingguan | 2013-05-06 11:11:11 | initialization
 */

#include "SnsSigCheck.h"
//oauth����gcc�����
#ifdef __cplusplus
extern "C"{
#endif
#include "oauth.h"
#ifdef __cplusplus
}
#endif


bool IsSpecialChar(char c)
{
    switch(c)
    {
        case '!':
        case '(':
        case ')':
        case '*':
            return true;
        default:
            return false;
    }
    return false;
}

/***********************************************************
**������ makeSig
**����:   
**          method ���󷽷� "get" or "post"
            url_path   openapi����
            params    �������
            secret      ��Կ
**���:  
**              
**����:  ǩ��
**����: ����ǩ��
**            
**************************************************************/ 
string CSnsSigCheck::makeSig(string& method, string& url_path, map<string, string>& params, string& secret) 
{
    string source;

    transform(method.begin(),method.end(), method.begin(),::toupper);
    source.append(method);
    source.append("&");
    source.append(url_encode(url_path));
    source.append("&");
    source.append(url_encode(join_params(params)));

    char* p_sig = oauth_sign_hmac_sha1_raw(
        source.c_str(),
        source.size(), 
        secret.c_str(),
        secret.size());

        if (p_sig == NULL)
        {
            return "";
        }

        string sig = p_sig;;

        delete [] p_sig;
        p_sig = NULL;

        return sig;
}

string CSnsSigCheck:: url_encode(const string& src)
{
    char* p_dest = oauth_url_escape(src.c_str());
    if (p_dest == NULL)
    {
        return "";
    }
    string str_dest = p_dest;

    delete [] p_dest;
    p_dest = NULL;

    str_dest = replace_str(str_dest, "~", "%7E");

    return str_dest;
}
/***********************************************************
**������ join_params
**����:   
**          params  �������
**���:  
**              
**����:  ƴ�Ӻ�Ĳ���
**����: ��������&����ƴ��
**            
**************************************************************/ 
string CSnsSigCheck::join_params(map<string,string> &params)
{
    string source;
    for(map<string, string>::iterator it = params.begin(); it != params.end(); ++it)
    {
        if (it != params.begin())
        {
            source.append("&");
        }
        source.append(it->first);
        source.append("=");
        source.append(it->second);
    }

    return source;
}

string CSnsSigCheck::replace_str(string src,const string& old_value,const string& new_value)
{
    for(string::size_type pos(0); pos != string::npos; pos += new_value.size()) 
    {
        if((pos=src.find(old_value, pos)) != string::npos)
        {
            src.replace(pos, old_value.size(), new_value);
        }
        else 
        {
            break;
        }
    }

    return src;
}


/***********************************************************
**������ encodevalue
**����:   
**          method ���󷽷� "get" or "post"
            url_path   openapi����
            params    �������
            secret      ��Կ
**���:  
**              
**����:  ǩ��
**����: �ص�����URLר�õı����㷨
**            �������Ϊ������ 0~9 a~z A~Z !*()֮�������ַ�����ASCII���ʮ�����Ƽ�%���б�ʾ������"-"����Ϊ"%2D"
**       http://wiki.open.qq.com/wiki/%E5%9B%9E%E8%B0%83%E5%8F%91%E8%B4%A7URL%E7%9A%84%E5%8D%8F%E8%AE%AE%E8%AF%B4%E6%98%8E_V3
**************************************************************/ 
string CSnsSigCheck::encodevalue(string& value)
{
  string encodedata;  
  
  char c;
  char tmp[4]={'\0'};
  for (unsigned int i = 0; i < value.size(); i++)
  {
      c = value[i];   
      if (!isalpha(c) && !isdigit(c)&& !IsSpecialChar(c))
      {
          snprintf(tmp,sizeof(tmp),"%%%02X",c);
          encodedata.append(tmp);
      }
      else
      {
          encodedata.append(1,c); 
      }
  }
  return encodedata;
}

