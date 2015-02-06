/**
 * 生成签名类
 *
 * @version 3.0.0
 * @author open.qq.com
 * @copyright (c) 2013, Tencent Corporation. All rights reserved.
 * @ History:
 *               3.0.0 | jixingguan | 2013-05-06 11:11:11 | initialization
 */

#include "SnsSigCheck.h"
//oauth库是gcc编译的
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
**函数名 makeSig
**输入:   
**          method 请求方法 "get" or "post"
            url_path   openapi名称
            params    请求参数
            secret      密钥
**输出:  
**              
**返回:  签名
**描述: 生成签名
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
**函数名 join_params
**输入:   
**          params  请求参数
**输出:  
**              
**返回:  拼接后的参数
**描述: 将参数以&进行拼接
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
**函数名 encodevalue
**输入:   
**          method 请求方法 "get" or "post"
            url_path   openapi名称
            params    请求参数
            secret      密钥
**输出:  
**              
**返回:  签名
**描述: 回调发货URL专用的编码算法
**            编码规则为：除了 0~9 a~z A~Z !*()之外其他字符按其ASCII码的十六进制加%进行表示，例如"-"编码为"%2D"
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

