#ifndef TIME_H_
#define TIME_H_

#include <ctime>
#include <string>
#include <sys/time.h>
#include <stdint.h>


class CTime
{
public:

    /*!
     * \brief 定义常用常量
    */
    enum
    {
        SECONDS_OF_DAY = 24*60*60,
        SECONDS_OF_HOUR = 60*60,
        SECONDS_OF_MIN = 60,
        TIMES_2020_12_20 = 1608393600,     //2012时间戳
	TIME_END
    };

public:

    /*!
     * \brief 获取某时间点之间的天数: 如 2005-01-10 12:00:00 -> 2005-01-11: 06:00:00 = 1天
     * \param[in] tFrom: 时间起点
     * \param[in] tTo:   时间终点
     * \return int [ 时间天数差, 分正负 ]
    * \remark 以本地时间为准进行处理
    */
    static int GetDayInterval(time_t tFrom, time_t tTo);

	//线程可重入版
    static int GetDayInterval_r(time_t tTime1, time_t tTime2);


	static int GetMonthDiff(time_t tTime1, time_t tTime2);
    /*!
     * \brief 获取当前年份
     * \return 年份
    * \remark 以本地时间为准进行处理
     */
    static int GetCurrentYear( void );

		/*!
     * \brief 获取系统微秒数
     * \return int [ 当前微妙 ]
    */
	static int GetCurrentuTime(bool bFast = true);

	/*!
     * \brief 获取系统毫秒数
     * \return int [ 当前毫秒 ]
    */
	static uint64_t GetCurrentMSTime(bool bFast = true);

    /*!
     * \brief 返回时间点所以日开始时间点: 如 2005-01-10 12:10:29 -> 2005-01-10 00:00:00
     * \param[in] tTIme: 时间点
     * \return int [ 返回一天开始时间 ]
    * \remark 以本地时间为准进行处理
    */
    static time_t GetDayBeginTime(time_t tTime);


    /*!
     * \breif 格式化时间
    */
    static const std::string FormatTime(const std::string& sFmt, const tm& stTime);


    /*!
     * \breif 格式化时间
    * \remark 以本地时间为准进行处理
    */
    static const std::string FormatTime(const std::string& sFmt, time_t tTime);


    /*!
     * \breif 时间结构转换
    */
    static time_t MakeTime(tm& stTime);


    /*!
     * \breif 时间结构转换
    */
    static time_t MakeTime(int iYear, int iMon, int iDay, int iHour=0, int iMin=0, int iSec=0);


    /*!
     * \brief 时间解析函数把 YYYY-MM-DD 格式分析成日期
     * \param[in]  sDate:  源日期数据 YYYY-MM-DD | YYYY/MM/DD
     * \param[out] iYear:  年
     * \param[out] iMon:   月
     * \param[out] iDay:   日
     * \param[in]  sDelim: 分隔符
     * \return int [ 0--succ !0--fail ]
    * \remark 本函数亦可作 HH:MM:SS 时间的分析
    */
    static int ParseDate(const std::string& sDate, int& iYear, int& iMon, int& iDay, const std::string& sDelim="-");

    /*!
     * \brief 返回时间代表的年月日
     * \param[out] iYear:  年
     * \param[out] iMon:   月
     * \param[out] iDay:   日
     * \param[in]  iTime:  时间, <0 时为当前时间
     * \return int [ 返回iTime, 如 iTime<0, 返回当前时间值 ]
    * \remark 以本地时间为准进行处理
    */
    static int GetDate(int& iYear, int& iMon, int& iDay, time_t iTime=-1);

	//时间戳可以为负，如果为负则返回1970年以前的年月日
    static int GetDateExt(int& iYear, int& iMon, int& iDay, time_t iTime);

    /*!
     * \brief 返回时间代表的星期几(0-6, 0表示星期天)
     * \param[out] iWeekDay:  星期几
     * \param[in]  iTime:  时间, <0 时为当前时间
     * \return int [ 返回iTime, 如 iTime<0, 返回当前时间值 ]
    * \remark 以本地时间为准进行处理
    */
    static int GetWeek(int& iWeekDay, time_t iTime=-1);




    static std::string GetWeekStr(time_t iTime = -1);

    /*!
     * \brief 检查日期合法性并作修正
     * \return int [ 0--合法 !0--不合法并作修正 ]
    */
    static int CheckDate(int& iYear, int& iMonth, int& iDay);

    /*!
     * \brief 时间格式化为UTC格式: Tue, 21 Feb 2006 02:20:04 UTC
     * \param[in] 时间
     * \remark 返回全球标准时间而非本地时间
    */
    static std::string UTCTime(time_t tTime);

	/*!
	 * \brief 取当前时间
	*/
	static timeval GetTimeOfDay();

	/*!
	 * \brief 计算两时间之间微秒差
	 * \return time_t[tvto-tvfrom]
	*/
	static time_t GetUSInterval(const timeval& tvfrom, const timeval& tvto);

	/*!
	 * \brief 计算两时间之间毫秒差
	 * \return time_t[tvto-tvfrom]
	*/
	static time_t GetMSInterval(const timeval& tvfrom, const timeval& tvto);

	/*!
	* \brief 计算时间是否落天当天之内
	* \return 0，当天; 非0，不是当天
	*/
	static int IsToday(time_t tTime);


    //距离xx本周周期 结束时间
    //默认0：周日作为一周周期开始 //0~6

    static time_t GetDurationEnd(int iEndWeekDay, int iEndWeekHour, int iEndWeekMin, time_t now=0);

    static bool  IsInDuration(time_t tDurationEnd, time_t iTime = 0);

	/* \brief 时间解析函数把 YYYY-MM-DD HH:MI:SS 格式分析成日期存为整型
		* \param[in]  sDate:  源日期数据 YYYY-MM-DD HH:MI:SS
		* \param[out] tTime:  整型保存的日期
		* \param[in]  sDelim1: 年月日分隔符
		* \param[in]  sDelim1: 时分秒分隔符
		* \return int [ 0--succ !0--fail ]
	*/
	static int ParseDate(const std::string& sDate, time_t& tTime, const std::string& sDelim1="-", const std::string& sDelim2=":");


	/*!
	* \brief 检查时间合法性并作修正
	* \return int [ 0--合法 !0--不合法并作修正 ]
	*/
	static void CheckTime(int& iHour, int& iMinute, int& iSecond);
};
#endif

