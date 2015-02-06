/**
 * URI decode encode�ӿ�
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
	**������ encodeURIValue
	**����:    sourceStr  Դ�ַ���
	**		   
	**���:  ��
	**				
	**����: �����Ľ����
	**����:  ��URI�еĲ������б�׼����
	**		   �淶�� RFC 1738 	  
	**************************************************************/
	std::string encodeURIValue(const std::string& sourceStr);
	
	/***********************************************************
	**������ encodeURIValue
	**����:    sourceStr  Դ�ַ���
	**		   
	**���:  resultStr  �������ַ���
	**				
	**����:  0           �ɹ�
			    ����  ʧ��	
	**����:  ��URI�еĲ������б�׼���������ֵ�汾
	**		   �淶�� RFC 1738 	  
	**************************************************************/
	int encodeURIValue(std::string& resultStr, const std::string& sourceStr);
	

	/***********************************************************
	**������ encodeURIValue
	**����:     sourceStr  Դ�ַ���
	**			resultBufferSize �������������С����

	**		   
	**���:  resultBuffer  �������ַ���
	**				
	**����:  0           �ɹ�
			    ����  ʧ��	
	**����:  ��URI�еĲ������б�׼���������ֵ�汾
	**		   �淶�� RFC 1738 	  
	**************************************************************/
	int encodeURIValue(char * resultBuffer, const char * sourceStr, size_t resultBufferSize);

	/***********************************************************
	**������ decodeURIValue
	**����:     sourceStr  Դ�ַ���
	**		   
	**���: 
	**				
	**����:  �������ַ���	
	**����:  ��URI�еĲ������б�׼����
	**		   �淶�� RFC 1738 	  
	**************************************************************/
	std::string decodeURIValue(const std::string& sourceStr);
	
	/***********************************************************
	**������ decodeURIValue
	**����: 	sourceStr  Դ�ַ���
	**		   
	**���: 	resultStr  �������ַ��� 
	**				
	**����:  0 �ɹ�
			   ����ʧ��
	**����:  ��URI�еĲ������б�׼���룬������ֵ�汾
	**		   �淶�� RFC 1738	  
	**************************************************************/
	int decodeURIValue(std::string& resultStr, const std::string& sourceStr);

	/***********************************************************
	**������ decodeURIValue
	**����: 	sourceStr  Դ�ַ���
				resultBufferSize �������������С����
	**		   
	**���: 	resultStr  �������ַ��� 
	**				
	**����:  0 �ɹ�
			   ����ʧ��
	**����:  ��URI�еĲ������б�׼���룬������ֵ��C�汾
	**		   �淶�� RFC 1738	  
	**************************************************************/
	int decodeURIValue(char * resultBuffer, const char * sourceStr, size_t resultBufferSize);
	
};

#endif
