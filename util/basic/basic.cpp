#include "trans.h"
#include "basic.h"
#include "ctime.h"
#include <sstream>
#include <iomanip>
#include <sys/ioctl.h>
#include <net/if.h>
#include <algorithm>
#include "csconv.h"
#include <sstream>
#include "md5c.h"
#include <string.h>
#include <functional>
#include <stdlib.h>
#include <arpa/inet.h>
#include <iconv.h>
#include <errno.h>


/*初始化默认ID*/
std::string CBasic::sCallerID = "test";

/*初始化默认IP*/
std::string CBasic::sWebServerIP = "unknown";

int CBasic::SetCaller(const std::string &sCaller)
{
	if("" != sCaller)
		sCallerID = sCaller;
	return 0;
}

const std::string CBasic::GetCaller(void)
{
	return sCallerID;
}

const std::string CBasic::GetAtomCaller(void)
{
		std::string::size_type iIndex = std::string::npos;
	    iIndex = sCallerID.size() >  29 ? sCallerID.size() - 29 : 0;
		return sCallerID.substr(iIndex);
}

int CBasic::RecWebServerIP(const std::string &sIP)
{
	if(!sIP.empty())
	{
		sWebServerIP = sIP;
	}
	return 0;
}

const std::string CBasic::GetWebServerIP(void)
{
	return sWebServerIP;
}


std::string& CBasic::StringReplace(std::string& sData, const std::string& sSrc, const std::string& sDst)
{
	std::string::size_type pos = 0;
	std::string::size_type slen = sSrc.size();
	std::string::size_type dlen = sDst.size();
	while ((pos=sData.find(sSrc, pos)) != std::string::npos)
	{
		sData.replace(pos, slen, sDst);
		pos += dlen;
	}

	return sData;
}

std::vector<std::string>& CBasic::StringSplit(const std::string& sData, const std::string& sDelim, std::vector<std::string>& vItems)
{
	vItems.clear();

	std::string::size_type bpos = 0;
	std::string::size_type epos = 0;
	std::string::size_type nlen = sDelim.size();
	while ((epos=sData.find(sDelim, epos)) != std::string::npos)
	{
		vItems.push_back(sData.substr(bpos, epos-bpos));
		epos += nlen;
		bpos = epos;
	}

	vItems.push_back(sData.substr(bpos, sData.size()-bpos));

	return vItems;
}

std::vector<int>& CBasic::StringSplit(const std::string& sData, const std::string& sDelim, std::vector<int>& vItems)
{
	std::vector<std::string> vStrings;

	vItems.clear();

	CBasic::StringSplit(sData, sDelim, vStrings);

	for (std::vector<std::string>::size_type iIndex=0; iIndex<vStrings.size(); ++iIndex)
	{
		vItems.push_back(CTrans::STOI(vStrings[iIndex]));
	}

	return vItems;
}

std::vector<unsigned int>& CBasic::StringSplit(const std::string& sData, const std::string& sDelim, std::vector<unsigned int>& vItems)
{
	std::vector<std::string> vStrings;

	vItems.clear();

	CBasic::StringSplit(sData, sDelim, vStrings);

	for (std::vector<std::string>::size_type iIndex=0; iIndex<vStrings.size(); ++iIndex)
	{
		vItems.push_back(CTrans::STOI(vStrings[iIndex]));
	}

	return vItems;
}


std::vector<std::string>& CBasic::StringSplitTrim(const std::string& sData, const std::string& sDelim, std::vector<std::string>& vItems)
{
	vItems.clear();

	std::string::size_type bpos = 0;
	std::string::size_type epos = 0;
	std::string::size_type nlen = sDelim.size();

	while(sData.substr(epos,nlen) == sDelim)
	{
		epos += nlen;
	}

	bpos = epos;

	while ((epos=sData.find(sDelim, epos)) != std::string::npos)
	{
		vItems.push_back(sData.substr(bpos, epos-bpos));

		epos += nlen;

		while(sData.substr(epos,nlen) == sDelim)
		{
			epos += nlen;
		}

		bpos = epos;
	}

	if(bpos != sData.size())
	{
		vItems.push_back(sData.substr(bpos, sData.size()-bpos));
	}

	return vItems;
}

std::string CBasic::StringTrim(std::string& sData, const std::string& sDelim)
{
	// trim right
	sData.erase(sData.find_last_not_of(sDelim)+1);
	// trim left
	return sData.erase(0,sData.find_first_not_of(sDelim));
}

std::string CBasic::StringErase(std::string& sData, const char sDelim )
{
	sData.erase(std::remove_if(sData.begin(),sData.end(),std::bind2nd(std::equal_to<char>(),sDelim)),sData.end());
	return sData;
}

std::string CBasic::StringJoin(const std::vector<std::string>& vstElem, const std::string& sDelim)
{
	std::string sText;
	std::vector<std::string>::const_iterator bpos = vstElem.begin();
	std::vector<std::string>::const_iterator epos = vstElem.end();
	std::vector<std::string>::const_iterator cpos = bpos;
	while (cpos != epos)
	{
		if (cpos != bpos)
		{
			sText += sDelim;
		}
		sText += *cpos++;
	}
	return sText;
}

std::string CBasic::StringJoin(const std::vector<int>& vstElem, const std::string& sDelim)
{
    std::string sText;
    std::vector<int>::const_iterator bpos = vstElem.begin();
    std::vector<int>::const_iterator epos = vstElem.end();
    std::vector<int>::const_iterator cpos = bpos;
    while (cpos != epos)
    {
        if (cpos != bpos)
        {
            sText += sDelim;
        }
        sText += CTrans::ITOS(*cpos++);
    }
    return sText;
}

std::string CBasic::StringJoin(const std::vector<unsigned int>& vstElem, const std::string& sDelim)
{
    std::string sText;
    std::vector<unsigned int>::const_iterator bpos = vstElem.begin();
    std::vector<unsigned int>::const_iterator epos = vstElem.end();
    std::vector<unsigned int>::const_iterator cpos = bpos;
    while (cpos != epos)
    {
        if (cpos != bpos)
        {
            sText += sDelim;
        }
        sText += CTrans::ITOS(*cpos++);
    }
    return sText;
}

std::string CBasic::StringEscape(const std::string& sData)
{
	std::string sValue;
	for (std::string::size_type xpos=0; xpos<sData.size(); ++xpos)
	{
		sValue += CTrans::CTOH(sData[xpos]);
	}
	return sValue;
}

std::string CBasic::StringUnEscape(const std::string& sData)
{
	std::string sValue(sData);
	std::string::size_type cpos = 0;
	while (std::string::npos != (cpos=sValue.find("%", cpos)))
	{
		sValue.replace(cpos, 3, 1, CTrans::HTOC(sValue.data() + cpos));
		cpos += 1;
	}
	return sValue;
}
std::string CBasic::urldecode(const std::string& sData)
{
	struct HEX
	{
		static bool isHex(const char ch)
		{
			return ((ch >='0' && ch <='9') || (ch >='A' && ch <='F') || (ch >='a' && ch <='f'));
		}
		static bool isSuper(const unsigned char ch)
		{
			return (ch>='A' && ch<='Z') ? true:false;
		}
		static unsigned char toLower(const unsigned char ch)
		{
			if(isSuper(ch))
				return (unsigned char) ((ch -'A')+'a');
			return ch;
		}
	};

	char szBuffer[sData.size()];
	memset(szBuffer,0x00,sizeof(szBuffer));
	char* pcValue = szBuffer;

	const char* bpos = sData.data();
	const char* epos = sData.data()+sData.size();

	unsigned char c;
	unsigned int oneChar=0;

   	while (bpos < epos)
   	{
		if (*bpos == '+') {
			*pcValue = ' ';
		}
		else if (*bpos == '%' && epos-bpos >= 2 && HEX::isHex(*(bpos + 1))&& HEX::isHex(*(bpos + 2)))
		{
		    c = (unsigned char)*((unsigned char *)(bpos+1));
		    if (HEX::isSuper(c))
		        c = HEX::toLower(c);
		    oneChar = (unsigned int)(c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10) * 16;
			//printf("c high = %c , onechar=%d\n",  c,oneChar);
		    c = (unsigned char)* ((unsigned char *)(bpos+2));

		    if (HEX::isSuper(c))
		        c = HEX::toLower(c);

		    oneChar += c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10;
				//printf("c low = %c , onechar=%d\n",  c,oneChar);
		    *pcValue = (unsigned char)oneChar ;
		    bpos += 2;
		}
		else {
		    *pcValue = *bpos;
		}
		bpos++;

		pcValue++;

	}


	std::string sRetValue("");

	sRetValue.assign(szBuffer,pcValue- szBuffer);
	return sRetValue;

}

std::string CBasic::UrlEncode(const std::string& sData)
{
	static char hex[] = "0123456789ABCDEF";
	std::string dst;

	for (size_t i = 0; i < sData.size(); i++)
	{
		unsigned char ch = sData[i];
		if (isalnum(ch))
		{
			dst += ch;
		}
		else
			if (sData[i] == ' ')
			{
				dst += '+';
			}
			else
			{
				unsigned char c = static_cast<unsigned char>(sData[i]);
				dst += '%';
				dst += hex[c / 16];
				dst += hex[c % 16];
			}
	}
	return dst;
}

#if 0

std::string CBasic::XMLEncode(const std::string& sData)
{

	char vx2[3] = { 0, 0, 0 };
	char vx4[5] = { 0, 0, 0, 0, 0 };
	std::string sValue;
	const char* bpos = sData.data();
	const char* epos = sData.data()+sData.size();
	static bool s_esc[256] =
	{
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
	};
	while (bpos < epos)
	{
		if (*bpos < 0)
		{
			sValue += "&#x";
			sValue += CTrans::CTOX4(bpos, vx4);
			sValue += ";";
			bpos += 2;
		}
		else if (!s_esc[*(unsigned char*)bpos])
		{
			sValue += *bpos;
			bpos += 1;
		}
        else if (*bpos!=13 && *bpos!=10 && *bpos < 32)
		{
            sValue += "&#x";
			sValue += "3F";    //全部替换为"?"
			sValue += ";";
			bpos += 1;
		}
		else
		{
			sValue += "&#x";
			sValue += CTrans::CTOX2(bpos, vx2);
			sValue += ";";
			bpos += 1;
		}
	}
	return sValue;
}


std::string CBasic::XMLMiniEncode(const std::string& sData)
{
	char vx2[3] = { 0, 0, 0 };
	std::string sValue;
	const char* bpos = sData.data();
	const char* epos = sData.data()+sData.size();
	static bool s_esc[256] =
	{
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
	};
	while (bpos < epos)
	{
		if (*bpos < 0)
		{
			sValue += bpos[0];
			sValue += bpos[1];
			bpos += 2;
		}
		else if (!s_esc[*(unsigned char*)bpos])
		{
			sValue += *bpos;
			bpos += 1;
		}
        else if (*bpos!=13 && *bpos!=10 && *bpos < 32)
		{
            sValue += "&#x";
			sValue += "3F";    //全部替换为"?"
			sValue += ";";
			bpos += 1;
		}
		else
		{
			sValue += "&#x";
			sValue += CTrans::CTOX2(bpos, vx2);
			sValue += ";";
			bpos += 1;
		}
	}
	return sValue;
}

std::string CBasic::escape(const std::string& sData)
{
	char vx2[3] = { 0, 0, 0 };
	char vx4[5] = { 0, 0, 0, 0, 0 };
	std::string sValue;
	const char* bpos = sData.data();
	const char* epos = sData.data()+sData.size();
	static bool s_esc[256] =
	{
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
	};
	while (bpos < epos)
	{
		if (*bpos < 0)
		{
			sValue += "%u";
			sValue += CTrans::CTOX4(bpos, vx4);
			bpos += 2;
		}
		else if (!s_esc[*(unsigned char*)bpos])
		{
			sValue += *bpos;
			bpos += 1;
		}
		else
		{
			sValue += "%";
			sValue += CTrans::CTOX2(bpos, vx2);
			bpos += 1;
		}
	}
	return sValue;
}


std::string CBasic::unescape(const std::string& sData)
{
	struct HEX
	{
		static bool isHex(char ch)
		{
			return ( (ch >='0' && ch <='9') || (ch >='A' && ch <='F') || (ch >='a' && ch <='f'));
		}
	};

	std::string sValue(sData);
	std::string::size_type cpos = 0;
	std::string::size_type nlen = sData.size();
	while (std::string::npos != (cpos=sValue.find("%", cpos)))
	{
        char vc1[2] = { 0, 0 };
    	char vc2[3] = { 0, 0, 0 };

		if ((sValue[cpos+1]=='u' || sValue[cpos+1]=='U') && (nlen>cpos+5) && HEX::isHex(sValue[cpos+2]) && HEX::isHex(sValue[cpos+3]) && HEX::isHex(sValue[cpos+4]) && HEX::isHex(sValue[cpos+5]))
		{
			sValue.replace(cpos, 6, CTrans::X4TOC(sValue.data()+cpos+2, vc2), 2);
			cpos += 2;
	/*
			std::string sUTF8;
			sUTF8.resize(4*6);
			int iTemp=CTrans::X4TOI(sValue.data()+cpos+2);
			int utf_8_size=XUCUTF8::cv((T_UC*)&iTemp, 2, (T_UTF8*)sUTF8.data(), sUTF8.size());
			sValue.replace(cpos, 6,sUTF8.data(), utf_8_size-1);

			fprintf(stderr,"%s\n",sUTF8.data());
			fprintf(stderr,"size:%d\n",utf_8_size);
			cpos += (utf_8_size-1);
	*/
			}
		else if ((nlen>cpos+2) && HEX::isHex(sValue[cpos+1]) && HEX::isHex(sValue[cpos+2]))
		{

			/*std::string sUTF8;
			sUTF8.resize(2*6);
			int iTemp=CTrans::X2TOI(sValue.data()+cpos+1);

			fprintf(stderr,"%d\n",iTemp);

			int utf_8_size=XUCUTF8::cv((T_UC*)&iTemp,sizeof(T_UC), (T_UTF8*)sUTF8.data(), sUTF8.size());


			sValue.replace(cpos, 3,sUTF8.data(), utf_8_size-1);

			fprintf(stderr,"%s\n",sUTF8.data());
			fprintf(stderr,"size:%d\n",utf_8_size);
			cpos += (utf_8_size-1);
			*/

			CTrans::X2TOC(sValue.data()+cpos+1, vc1);

			char c2[2];

			if((unsigned short)vc1[0]>127)
			{

				XUCGB::cv((T_UC*)&vc1[0], 1, c2, 2);
				sValue.replace(cpos, 3, c2, 2);
				cpos += 2;
			}
			else
			{

				sValue.replace(cpos, 3, CTrans::X2TOC(sValue.data()+cpos+1, vc1), 1);
				cpos += 1;
			}
		}
		else
		{
			cpos += 1;
		}
	}
	return sValue;
}

//gb ->utf8
std::string CBasic::UTF8(const std::string& sData)
{
    if(IsUTF8(sData.c_str()))
        return sData;

	std::string sUTF8;
	T_UC* tUC = new T_UC[sData.size()];
	int nUClen = XGBUC::cv(sData.data(), sData.size(), tUC, sData.size());
	if (nUClen > 0)
	{
		sUTF8.resize(nUClen*3);
		sUTF8.resize(XUCUTF8::cv(tUC, nUClen, (T_UTF8*)sUTF8.data(), sUTF8.size()));
	}
	delete[] tUC;
	return sUTF8;
}


std::string CBasic::GB2UTF8(const std::string& sData)
{
    if(IsUTF8(sData.c_str()))
        return sData;

	return from_to_ex(sData, "GB18030","utf-8");
}

std::string CBasic::Utf8ToGbk(const std::string& sData)
{
	iconv_t cd = iconv_open("gbk", "utf-8") ;
	char dest[4096]= {0} ;

	char * in = const_cast<char*>( sData.c_str() ) ;
	char * out = dest ;
	size_t ileft = strlen( in ) ;
	size_t oleft = sizeof( dest ) ;
	iconv( cd, &in, &ileft, &out, &oleft ) ;
	iconv_close( cd ) ;

	return std::string( dest, sizeof(dest) - oleft ) ;
}


std::string  CBasic::from_to_ex( const std::string& src, const std::string& enc_from, const std::string& enc_to )
{
    if ( enc_from == enc_to )
        return src.c_str() ;

    iconv_t cd = iconv_open( enc_to.c_str(), enc_from.c_str() ) ;
    static char dest[4096]= {0} ;
    memset( dest, 0, sizeof( dest ) );

    char * in = const_cast<char*>( src.c_str() ) ;
    char * out = dest ;
    size_t ileft = strlen( in ) ;
    size_t oleft = sizeof( dest ) ;


    while ( ileft > 0 && oleft > 0 )
    {
        size_t ret = iconv( cd, &in, &ileft, &out, &oleft );
        if ( ileft <= 0 || oleft <= 1 || ret == 0 )
        {
            break;
        }

        ileft--;
        if ( errno ==  EILSEQ )
        {
            in++;
        }
        else
        {
            break;
        }
    }

    iconv_close( cd ) ;

    return std::string( dest, sizeof(dest) - oleft ) ;
}
#endif

int CBasic::GetUTF8ByByteNum( const char* input, unsigned uiGetSize, std::string& out )
{
    if( input == NULL)
        return -1;
    uiGetSize *= 2;
    const char*ptr = input;
    unsigned char c = *ptr;
    char szTmp[4] = "";
    unsigned int uiSize = 0;
    while ( *ptr != '\0' )
    {
        c=*ptr;
        memset( szTmp, 0, sizeof(szTmp) );

        if( c > 0 && c <=0x1F )
        {
            ptr++;
            continue;
        }
        else if ( !(c&0x80) )
        {
            ptr++;
            uiSize++;
            szTmp[0] = c;
        }
        else if ( (c&0xE0) == 0xE0 )
        {
            if( *(ptr+1) == '\0' || *(ptr+2) == '\0')
            {
                break;
            }
            else if( (*(ptr+1)&0xC0) != 0x80 || (*(ptr+2)&0xC0) != 0x80 )
            {
                ptr++;
                continue;
            }
            else
            {
                memcpy( szTmp, ptr, 3 );
                ptr+=3;
                uiSize += 2;//一个中文算两个字符
            }
        }
        else if ( (c&0xE0) == 0xC0 )
        {
            if( *(ptr+1) == '\0' )
            {
                break;
            }
            else if( (*(ptr+1)&0xC0) != 0x80 )
            {
                ptr++;
                continue;
            }
            else
            {
                memcpy( szTmp, ptr, 2 );
                ptr+=2;
                uiSize += 2;
            }
        }
        else
        {
            ptr++;
            continue;
        }

        out.append( szTmp );
        if ( uiSize >= uiGetSize && c != '\0' )
        {
            out.append( "..." );
            break;
        }
    }


    return 0;
}

int CBasic::GetUTF8SizeByByte( const char* input)
{
    if( input == NULL)
        return 0;
    const char*ptr = input;
    unsigned char c = *ptr;
    unsigned int uiSize = 0;
    while ( *ptr != '\0' )
    {
        c=*ptr;

        if( c > 0 && c <=0x1F )
        {
            ptr++;
            continue;
        }
        else if ( !(c&0x80) )
        {
            ptr++;
            uiSize++;
        }
        else if ( (c&0xE0) == 0xE0 )
        {
            if( *(ptr+1) == '\0' || *(ptr+2) == '\0')
            {
                break;
            }
            else if( (*(ptr+1)&0xC0) != 0x80 || (*(ptr+2)&0xC0) != 0x80 )
            {
                ptr++;
                continue;
            }
            else
            {
                ptr+=3;
                uiSize += 2;//一个中文算两个字符
            }
        }
        else if ( (c&0xE0) == 0xC0 )
        {
            if( *(ptr+1) == '\0' )
            {
                break;
            }
            else if( (*(ptr+1)&0xC0) != 0x80 )
            {
                ptr++;
                continue;
            }
            else
            {
                ptr+=2;
                uiSize += 2;
            }
        }
        else
        {
            ptr++;
            continue;
        }
    }


    return uiSize;
}

int CBasic::GetUTF8ByWordNum( const char* input, unsigned uiGetSize, std::string& out )
{
    if ( input == NULL )
    {
        return -1;
    }

    const char*ptr = input;
    unsigned char c = *ptr;
    char szTmp[4] = "";

    unsigned int uiSize = 0;
    while ( *ptr != '\0' )
    {
        c=*ptr;
        memset( szTmp, 0, sizeof(szTmp) );

        if( c > 0 && c <=0x1F )
        {
            ptr++;
            continue;
        }
        else if ( !(c&0x80) )
        {
            ptr++;
            uiSize++;
            szTmp[0] = c;
        }
        else if ( (c&0xE0) == 0xE0 )
        {
            if( *(ptr+1) == '\0' || *(ptr+2) == '\0')
            {
                break;
            }
            else if( (*(ptr+1)&0xC0) != 0x80 || (*(ptr+2)&0xC0) != 0x80 )
            {
                ptr++;
                continue;
            }
            else
            {
                memcpy( szTmp, ptr, 3 );
                ptr+=3;
                uiSize++;
            }
        }
        else if ( (c&0xE0) == 0xC0 )
        {
            if( *(ptr+1) == '\0' )
            {
                break;
            }
            else if( (*(ptr+1)&0xC0) != 0x80 )
            {
                ptr++;
                continue;
            }
            else
            {
                memcpy( szTmp, ptr, 2 );
                ptr+=2;
                uiSize++;
            }
        }
        else
        {
            ptr++;
            continue;
        }

        out.append( szTmp );
        if ( uiSize >= uiGetSize && c != '\0' )
        {
            out.append( "..." );
            break;
        }
    }

    return 0;
}

void CBasic::TruncateUtf8(const std::string& sInput, unsigned int iByteSizeLimit, std::string & sOutput)
{
	sOutput.clear();
    	const char* ptr = sInput.c_str();
	const char* ptrFirst = ptr;
	int iInputLen = sInput.size();
    	unsigned char c = *ptr;
    	char szTmp[4] = "";

    	while ( *ptr != '\0' )
    	{
       	c=*ptr;
        	memset( szTmp, 0, sizeof(szTmp) );

        	if( c > 0 && c <=0x1F )
        	{
            		ptr++;
            		continue;
        	}
        	else if ( !(c&0x80) )
        	{
            		ptr++;
            		szTmp[0] = c;
        	}
        	else if ( (c&0xE0) == 0xE0 )
        	{
            		if( *(ptr+1) == '\0' || *(ptr+2) == '\0')
            		{
                		break;
            		}
	            	else if( (*(ptr+1)&0xC0) != 0x80 || (*(ptr+2)&0xC0) != 0x80 )
	            	{
	               	 ptr++;
	                	continue;
	            	}
	            	else
	            	{
	            		if(ptr + 3 - ptrFirst > iInputLen)
					break;

	               	memcpy( szTmp, ptr, 3 );
	                	ptr+=3;
	            	}
        	}
       	 else if ( (c&0xE0) == 0xC0 )
        	{
	        	if( *(ptr+1) == '\0' )
	            	{
	              	break;
	            	}
	            	else if( (*(ptr+1)&0xC0) != 0x80 )
	            	{
	                	ptr++;
	                	continue;
	            	}
	            	else
	            	{
	            		if(ptr + 2 - ptrFirst > iInputLen)
					break;

	                	memcpy( szTmp, ptr, 2 );
	                	ptr+=2;
	            	}
        	}
	       else
	       {
	       	ptr++;
	            	continue;
	       }

		if(sOutput.size() + strlen(szTmp) > iByteSizeLimit)
		{
			break;
		}
		else
			sOutput.append( szTmp );
    	}
}

bool CBasic::IsUTF8( const char* input )
{
    if ( input == NULL )
    {
        return false;
    }

    const char*ptr = input;
    unsigned char c = *ptr;

    while ( *ptr != '\0' )
    {
        c=*ptr;

        if( c > 0 && c <=0x1F )
        {
            ptr++;
            continue;
        }
        else if ( !(c&0x80) )
        {
            ptr++;
        }
        else if ( (c&0xE0) == 0xE0 )
        {
            if( *(ptr+1) == '\0' || *(ptr+2) == '\0')
            {
                return false;
            }
            else if( (*(ptr+1)&0xC0) != 0x80 || (*(ptr+2)&0xC0) != 0x80 )
            {
                return false;
            }
            else
            {
                ptr+=3;
            }
        }
        else if ( (c&0xE0) == 0xC0 )
        {
            if( *(ptr+1) == '\0' )
            {
                return false;
            }
            else if( (*(ptr+1)&0xC0) != 0x80 )
            {
                return false;
            }
            else
            {
                ptr+=2;
            }
        }
        else
        {
            return false;
        }
    }

    return true;
}

std::string& CBasic::StringChnCut(std::string& sData, int nSize)
{
	if (nSize >= (int)sData.size())
	{
		return sData;
	}
	sData.erase(CheckChn(sData.data(), nSize), std::string::npos);
	return sData;
}

int CBasic::CheckChn(const char* pcData, int nSize)
{
	const char* pcEnd = pcData+nSize;
	while (pcData < pcEnd)
	{
		if (*pcData >= 0)
		{
			++pcData;
		}
		else
		{
			pcData += 2;
		}
	}
	return (pcData==pcEnd) ? (nSize) : (nSize-1);
}

int CBasic::Wordlen(const std::string& sData)
{
	int nlen = 0;
	const char* bp = sData.data();
	const char* ep = bp+sData.size();
	while (bp < ep)
	{
		nlen++;
		if (*bp>=0)
		{
			++bp;
		}
		else
		{
			bp += 2;
		}
	}
	return nlen;
}

std::string CBasic::Wordreserve(const std::string& sData, int nSize)
{
	const char* bp = sData.data();
	const char* ep = bp+sData.size();
	while (nSize>0 && bp<ep)
	{
		nSize--;
		if (*bp>=0)
		{
			++bp;
		}
		else
		{
			bp += 2;
		}
	}
	if (bp==ep)
	{
		return sData;
	}
	return sData.substr(0, bp-sData.data());
}

int CBasic::IsUinValid(uin_t iUin)
{
	const unsigned int MAX_UIN = 4000000000u;
	return (iUin>=10000 && (unsigned int)iUin <= MAX_UIN) ? 0 : -1;
}

int CBasic::IsQQHomeItemnoValid(int iItemNo)
{
	return (iItemNo>=1 && iItemNo<=50000) ? 0 : -1;
}


// 现网机器上是双网卡，只记录内网IP（172开头的IP）
std::string CBasic::GetLocalIP()
{
	static char localip[16] = {0};

	//保证IP只取一次
	if(localip[0] == 0)
	{
		int fd;
		struct ifconf if_conf; /* net/if.h */
		struct sockaddr_in *sin;

		fd = socket(PF_INET, SOCK_DGRAM, 0);

		if_conf.ifc_req = (struct ifreq *)malloc(10 * sizeof(struct ifreq));
		if_conf.ifc_len = 10 * sizeof(struct ifreq);

		if(ioctl(fd, SIOCGIFCONF, &if_conf) == -1)
		{
			free(if_conf.ifc_req);
			close(fd);
			return "取IP信息失败";
		}

		//char p[20];
		for (unsigned int i = 0; i < if_conf.ifc_len / sizeof(struct ifreq); i++)
		{
			sin = (struct sockaddr_in *)&if_conf.ifc_req[i].ifr_addr;
			{
				char buf[64];
				const char *p;
				p = inet_ntop(AF_INET, &sin->sin_addr, buf, sizeof(buf));
				if(strncmp(p, "172", 3) == 0 || 0==strncmp(p, "192", 3) || 0==strncmp(p, "10.", 3))
				{
					strncpy(localip,p,15);
					break;
				}
			}
		}

		free(if_conf.ifc_req);
		close(fd);
	}
    return std::string(localip);
}

std::string CBasic::InetNtop(unsigned int iNetAddr)
{
	struct in_addr addr;
	memset(&addr, 0, sizeof(addr));
	memcpy(&addr, &iNetAddr, sizeof(addr));

	char szBuff[64] = {0};
	inet_ntop(AF_INET, &addr, szBuff, sizeof(szBuff) - 1);

	return szBuff;
}


std::string CBasic::removeInvalidWML(const std::string & str)
{
	std::string temp;
	for (unsigned int i = 0; i < str.size(); i++)
	{
		char c = str.at(i);

		char ctr = 0;
		if (i < (str.size() - 1))
			ctr = str.at(i + 1);

		if (c >= '\x00' && c <= '\x1F')
			continue;
		if (c >= '\xE0' && c <= '\xF8' && ctr >= '\x00' && ctr <= '\xFF')
		{
			i++;
			continue;

		}
		if (c == '\xFF' && ctr >= '\xF0' && ctr <= '\xFF')
		{
			i++;
			continue;
		}

		switch (c)
		{
		case '\xFF':
		case '\x24':
			break;
		case '^':
		case '`':
			break;
		default:
			temp.append(1, c);
			break;
		}

	}

	return temp;
}

int CBasic::InetPton(const std::string& strIp)
{
	struct in_addr s;
	inet_pton(AF_INET, strIp.c_str(), (void *)&s);
	return s.s_addr;
}
std::string CBasic::Md5(const std::string& source)
{
    char pData[1024];
    source.copy(pData,source.length());
    int iLen = source.length();
    MD5_CTX stCTX;
    unsigned char uchOput[16];
    memset(uchOput, 0, sizeof(uchOput));

    MD5cInit(&stCTX);
    MD5cUpdate(&stCTX, (unsigned char*)pData, iLen);
    MD5cFinal(uchOput, &stCTX);

    std::stringstream strm;
    strm.setf(std::ios::uppercase);

    for (unsigned int i=0; i<sizeof(uchOput); i++)
    {
        strm<<std::setw(2)<<std::setfill('0')<<std::hex<<((int)(uchOput[i]));
    }
    return strm.str();
}
bool CBasic::IsInOneDay(int time1, int time2)
{
    int time1_re = time1 + 8*3600;
    int time2_re = time2 + 8*3600;

    if ( (time1_re / 86400) == (time2_re / 86400) )
        return true;
    return false;
}
