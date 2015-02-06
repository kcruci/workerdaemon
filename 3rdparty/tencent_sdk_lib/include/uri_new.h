/**
 * URI decode encode接口
 *
 * @version 3.0.0
 * @author open.qq.com
 * @copyright (c) 2013, Tencent Corporation. All rights reserved.
 * @ History:
 *               3.0.0 | jixingguan | 2013-05-16 10:33:04 | initialize 
 */


#ifndef _WEBCHARACTER_URI_NEW
#define _WEBCHARACTER_URI_NEW


#include <string>

namespace WebCharacter_NEW {

    enum errorCode {
        ERR_NULL_PARAMS = -100,
        ERR_BUFFER_TOO_SMALL = -99,
        ERR_ENCODE = -98,
        OK = 0
    };

    enum charsetCheck {
        NO_CHECK = 0,
        UTF8_CHECK = 1,
        GBK_CHECK = 2
    };

	struct encodeURIDefineNode {
		size_t len;
		char ta[4];
	};
	
	
	struct decodeURIDefineNode {
		int highVal;
		int lowVal;
	};
	
	
	/***********************************************************
	**函数名 encodeURIValue
	**输入:    sourceStr  源字符串
	**		   
	**输出:  无
	**				
	**返回: 编码后的结果串
	**描述:  对URI中的参数进行标准编码
	**		   规范见 RFC 1738 	  
	**************************************************************/
	std::string encodeURIValue(const std::string& sourceStr);
	
	/***********************************************************
	**函数名 encodeURIValue
	**输入:    sourceStr  源字符串
	**		   
	**输出:  resultStr  编码后的字符串
	**				
	**返回:  0           成功
			    其他  失败	
	**描述:  对URI中的参数进行标准编码带返回值版本
	**		   规范见 RFC 1738 	  
	**************************************************************/
	int encodeURIValue(std::string& resultStr, const std::string& sourceStr);
	

	/***********************************************************
	**函数名 encodeURIValue
	**输入:     sourceStr  源字符串
	**			resultBufferSize 结果缓冲区的最小容量

	**		   
	**输出:  resultBuffer  编码后的字符串
	**				
	**返回:  0           成功
			    其他  失败	
	**描述:  对URI中的参数进行标准编码带返回值版本
	**		   规范见 RFC 1738 	  
	**************************************************************/
	int encodeURIValue(char * resultBuffer, const char * sourceStr, size_t resultBufferSize);

	/***********************************************************
	**函数名 decodeURIValue
	**输入:     sourceStr  源字符串
	**		   
	**输出: 
	**				
	**返回:  解码后的字符串	
	**描述:  对URI中的参数进行标准解码
	**		   规范见 RFC 1738 	  
	**************************************************************/
	std::string decodeURIValue(const std::string& sourceStr);
	
	/***********************************************************
	**函数名 decodeURIValue
	**输入: 	sourceStr  源字符串
	**		   
	**输出: 	resultStr  解码后的字符串 
	**				
	**返回:  0 成功
			   其他失败
	**描述:  对URI中的参数进行标准解码，带返回值版本
	**		   规范见 RFC 1738	  
	**************************************************************/
	int decodeURIValue(std::string& resultStr, const std::string& sourceStr);

	/***********************************************************
	**函数名 decodeURIValue
	**输入: 	sourceStr  源字符串
				resultBufferSize 结果缓冲区的最小容量
	**		   
	**输出: 	resultStr  解码后的字符串 
	**				
	**返回:  0 成功
			   其他失败
	**描述:  对URI中的参数进行标准解码，带返回值的C版本
	**		   规范见 RFC 1738	  
	**************************************************************/
	int decodeURIValue(char * resultBuffer, const char * sourceStr, size_t resultBufferSize);
	
};

#endif
