#ifndef _BASIC_H_
#define _BASIC_H_
#include <string>
#include <vector>
#include <time.h>
//#include "csconv.h"
//#include <iconv.h>

/*!
 * \brief 基础库类，实现一些公共函数，一般以静态成员方式提供
*/
class CBasic
{
public:
    /*! <MD5报文长度*/
    enum enMd5LenRange
    {
        MD5_MIN_LEN = 4,
        MD5_MAX_LEN = 22,
        MD5_OLD_LEN = 32
    };

    /*!
     * \brief 字符串替换
     * \param[in,out] sData: 要进行替换的对象并对之进行替换
     * \param[in] sSrc: 要替换的子串
     * \param[in] sDst: 用于替换的子串
     * \return std::string 返回sData对象引用
    */
    static std::string& StringReplace(std::string& sData, const std::string& sSrc, const std::string& sDst);

    /*!
     * \brief 分解字符串
     * \param[in] sData: 要进行分解的字符串
     * \param[in] sDelim: 分隔字符串
     * \param[out] vItems: 返回字符串列表包含空串
     * \return std::vector<std::string> vItems的引用
    */
    static std::vector<std::string>& StringSplit(const std::string& sData, const std::string& sDelim, std::vector<std::string>& vItems);

	/*!
     * \brief 分解字符串(连续的分割符看作一个如 |||==|)
     * \param[in] sData: 要进行分解的字符串
     * \param[in] sDelim: 分隔字符串
     * \param[out] vItems: 返回字符串列表包含空串
     * \return std::vector<std::string> vItems的引用
    */
    static std::vector<std::string>& StringSplitTrim(const std::string& sData, const std::string& sDelim, std::vector<std::string>& vItems);

	/*!
     * \brief 去除字符串前后的空格
     * \param[in/out] sData: 要进行Trim的字符串
     * \param[in] sDelim: 分隔字符串,默认为空格
     * \return std::string sData
    */
    static std::string StringTrim(std::string& sData, const std::string& sDelim =" ");

	/*!
	* \brief 去除字符串某个字符
	* \param[in/out] sData: 要进行去掉的字符串
	* \param[in] sDelim: 分隔字符串,默认为空格
	* \return std::string sData
	*/
	static std::string StringErase(std::string& sData, const char SDelim = ' ');

	/*!
	 * \brief 把数组合成字符串
	 * \param[in] vstElem: 要合成的数组
	 * \param[in] sDelim:  分隔符
	 * \return std::string 合成的字符串
	*/
	static std::string StringJoin(const std::vector<std::string>& vstElem, const std::string& sDelim);

	/*!
	 * \brief 把数组合成字符串
	 * \param[in] vstElem: 要合成的数组
	 * \param[in] sDelim:  分隔符
	 * \return std::string 合成的字符串
	*/
	static std::string StringJoin(const std::vector<int>& vstElem, const std::string& sDelim);

    /*!
	 * \brief 把数组合成字符串
	 * \param[in] vstElem: 要合成的数组
	 * \param[in] sDelim:  分隔符
	 * \return std::string 合成的字符串
	*/
	static std::string StringJoin(const std::vector<unsigned int>& vstElem, const std::string& sDelim);

    /*!
     * \brief 分解字符串, 返回数据转换为整型
     * \param[in] sData: 要进行分解的字符串
     * \param[in] sDelim: 分隔字符串
     * \param[out] vItems: 返回字符串列表包含空串
     * \return std::vector<int> vItems的引用
    */
    static std::vector<int>& StringSplit(const std::string& sData, const std::string& sDelim, std::vector<int>& vItems);
	static std::vector<unsigned int>& StringSplit(const std::string& sData, const std::string& sDelim, std::vector<unsigned int>& vItems);
    /*! \brief 对字符串编译C->%XX*/
    static std::string StringEscape(const std::string& sData);

    /*! \brief 对字符串编码进行解码%XX->C*/
    static std::string StringUnEscape(const std::string& sData);
	//支持urlencode标准解码，支持标准form提交中文
	static std::string urldecode(const std::string& sData);

	//urlencode support
	static std::string UrlEncode(const std::string& sData);

#if 0

    /*! \brief 对字符串编码进行XML编码功能 gb2312*/
	static std::string XMLEncode(const std::string& sData);

    /*! \brief 对字符串编码进行XML部分编码功能 */
	static std::string XMLMiniEncode(const std::string& sData);

	/*! \brief 对字符串编译功能与JavaScript.escape()类似 */
	static std::string escape(const std::string& sData);


    /*! \brief 对字符串编码进行XML编码功能 gb2312*/
	static std::string XMLEncode(const std::string& sData);

    /*! \brief 对字符串编码进行XML部分编码功能 */
	static std::string XMLMiniEncode(const std::string& sData);

	/*! \brief 对字符串编译功能与JavaScript.escape()类似 */
	static std::string escape(const std::string& sData);


    /*! \brief 对字符串编码进行解码功能与JavaScript.unescape()类似 */
	static std::string unescape(const std::string& sData);

	/*! \brief 对GB2312编译转换为UTF-8编译 */
	static std::string UTF8(const std::string& sData);

	/*! \brief 将UTF8字符转成GBK */
	static std::string Utf8ToGbk(const std::string& sData);

	static	std::string from_to_ex( const std::string& src, const std::string& enc_from, const std::string& enc_to );

#endif

	/*! \brief 检查进行中文完整截断保存最多nlen字节 */
	static std::string& StringChnCut(std::string& sData, int nSize);

	static int GetUTF8ByWordNum( const char* input, unsigned uiGetSize, std::string& out );
    /*
    *截取uiGetSize个中文字符，2英文=1个中文字符
    */
    static int GetUTF8ByByteNum( const char* input, unsigned uiGetSize, std::string& out );
     /*
    *取得字符串的长度，2英文=1个中文字符
    */
    static int GetUTF8SizeByByte( const char* input);
    static bool IsUTF8( const char* input );

	static void TruncateUtf8(const std::string& sContent, unsigned int iByteSizeLimit, std::string& sOutput);
    /*!
     * \brief 检查双字节字符的截断位置,保证数据的正确性
     * \param[in] pcData: 字符指针首地址
     * \param[in] nSize:  最大字符数(注:应保证不能大于字符串长度)
     * \return int <= nSize 应该保留的字节数 [ =nSize OR =nSize-1 ]
    */
    static int CheckChn(const char* pcData, int nSize);

	/*!
	 * \brief 检查词数(中文词=2字节)
	 * \param[in] sData: 要检查的字符串
	 * \return int 词数
	*/
	static int Wordlen(const std::string& sData);

	/*!
	 * \brief
	 * \param[in] sData: 源字符串
	 * \return std::string 返回目的串
	*/
	static std::string Wordreserve(const std::string& sData, int nSize);

    /*!
     * \brief 根据MD5算法生成签名
     * \param[in] sData: 待签名数据
     * \param[in] iDataLen: 待签名数据长度
     * \param[in] sKey: 签名公钥
     * \param[in] iKeyLen: 公钥长度
     * \param[in] iMD5Len: 签名结果长度,不包括结尾所需的'\0'
     * \param[out] sMD5: 签名结果
     * \return int [ 0--签名成功  !0--签名失败，或者结果长度小于4 ]
    */
    static int MD5Make(const std::string& sData, int iDataLen, const std::string& sKey, int iKeyLen, std::string& sMD5, int iMD5Len);

    /*!
     * \brief 验证MD5算法生成的签名
     * \param[in] sData: 待签名数据
     * \param[in] iDataLen: 待签名数据长度
     * \param[in] sKey: 签名公钥
     * \param[in] iKeyLen: 公钥长度
     * \param[out] sMD5: 签名
     * \param[in] iMD5Len: 签名结果长度,不包括结尾所需的'\0'
     * \return int [ 0--签名成功  !0--签名失败 ]
    */
    static int MD5NCmp(const std::string& sData, int iDataLen,  const std::string& sKey, int iKeyLen, const std::string& sMD5, int iMD5Len);

	/*!
	 * \brief 对MD5Make的封装
	*/
	static std::string MD5Make(const std::string& sData, const std::string& sKey);

	/*!
	 * \brief 对MD5NCmp的封装
	*/
	static int MD5NCmp(const std::string& sData, const std::string& sKey, const std::string& sMD5);

   /*!
     * \brief 生成通用MD5
     * \param[in] sData:
     * \param[out] sMD5:
	 * \param[in] bOutHex:是否以十六进制输出
     * \return int [ 0--succ !0--fail ]
    */
    static int MD5Comm(const std::string& sData, std::string& sMD5, bool bOutHex = true);

    static std::string Md5(const std::string& source);
    /*!
     * \brief 对ID的访问次数做限制，在 /etc/tbase_limit.conf 中配置appid
     * \param[in] iAppID    配置ID号
     * \param[in] szID        独立ID， 仅前14位(不包括结尾'\0')有效
     * \return int [ 0--限制成功  !0--限制失败 ]
    */
    static int CheckLimit(int iAppID, const std::string& sID);


	static std::string GB2UTF8(const std::string& sData);

public:

    /*!
     * \brief 测试UIN正确性
     * \return int [ 0--OK  !0--FAIL ]
    */
    static int IsUinValid(unsigned int iUin);

     /*!
     * \brief 测试UIN正确性
     * \return int [ 0--OK  !0--FAIL ]
    */
    static int IsQQHomeItemnoValid(int iItemNo);

     /*!
     * \brief 设置当前调用者身份
     * \parma[in] sCallerID 调用者身份
     * \return int [ 0--OK  !0--FAIL ]
    */
    static int SetCaller(const std::string &sCallerID);

     /*!
     * \brief 获得当前调用者身份
     * \return 调用者身份
    */
    static const std::string GetCaller(void);

      /*!
     * \brief 获得当前调用者身份针对原子服务器进行了删减
     * \return 调用者身份
    */
    static const std::string GetAtomCaller(void);

     /*!
     * \brief 记录WEB服务器IP
     * \parma[in] sCallerID 调用者身份
     * \return int [ 0--OK  !0--FAIL ]
    */
    static int RecWebServerIP(const std::string &sIP);

     /*!
     * \brief 获得WEB服务器IP
     * \return 服务器IP
    */
    static const std::string GetWebServerIP(void);


    /*
    *获取内网IP
    *成功：返回IP 失败：返回空
    */
    static std::string GetLocalIP();


	/*
    *将整型的地址转换成字符串
    *成功：返回IP 失败：返回空
    */
	static std::string InetNtop(unsigned int iNetAddr);

	static std::string removeInvalidWML(const std::string & str);

	static int InetPton(const std::string& strIp);

    static bool IsInOneDay(int time1,int time2);
private:

    /*! 调用者身份 */
    static std::string sCallerID;

    /*! 记录发送请求的WEB服务器IP*/
    static std::string sWebServerIP;

};

#endif

