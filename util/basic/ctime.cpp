
#include <ctime>
#include <string>
#include <vector>
#include <sys/times.h>

#include "basic.h"
#include "trans.h"
#include "ctime.h"
#include "log/logger.h"
#include <stdlib.h>

int CTime::GetDayInterval(time_t tFrom, time_t tTo)
{
    return ((int)CTime::GetDayBeginTime(tTo)-(int)CTime::GetDayBeginTime(tFrom))/CTime::SECONDS_OF_DAY;
}

//线程安全版
int CTime::GetDayInterval_r(time_t tTime1, time_t tTime2)
{
    if(tTime1 <= 0 || tTime2 <= 0)
        return -1;

    struct tm p1, p2;

    localtime_r(&tTime1, &p1);
    localtime_r(&tTime2, &p2);

    int iDayDiff(0);
    int iYearDiff = p2.tm_year - p1.tm_year;

    if(0 != iYearDiff)
    {
        iDayDiff = 365*iYearDiff;
    }

    iDayDiff +=	(p2.tm_yday - p1.tm_yday);
    return iDayDiff;
}


int CTime::GetMonthDiff(time_t tTime1, time_t tTime2)
{
    if(tTime1 <= 0 || tTime2 <= 0 || tTime2 < tTime1)
        return -1;

    struct tm p1, p2;

    localtime_r(&tTime1, &p1);
    localtime_r(&tTime2, &p2);

    int iMonDiff(0);
    int iYearDiff = p2.tm_year - p1.tm_year;

    if(0 != iYearDiff)
    {
        iMonDiff = 12*iYearDiff;
    }

    iMonDiff +=	(p2.tm_mon - p1.tm_mon);
    return iMonDiff;

}


time_t CTime::GetDayBeginTime(time_t tTime)
{
    tm tTm = *localtime(&tTime);
    tTm.tm_hour	= 0;
    tTm.tm_min	= 0;
    tTm.tm_sec	= 0;
    return CTime::MakeTime(tTm);
}

const std::string CTime::FormatTime(const std::string& sFmt, const tm& stTime)
{
    std::string sFmtTime;
    sFmtTime.resize(255);
    strftime(const_cast<char *>(sFmtTime.data()), sFmtTime.size(), sFmt.c_str(), &stTime);
    return std::string(sFmtTime.c_str());
}

const std::string CTime::FormatTime(const std::string& sFmt, time_t tTime)
{
    std::string sFmtTime;
    sFmtTime.resize(255);
    strftime(const_cast<char *>(sFmtTime.data()), sFmtTime.size(), sFmt.c_str(), localtime((time_t*)&(tTime)));
    return std::string(sFmtTime.c_str());
}

time_t CTime::MakeTime(tm& stTime)
{
    return mktime(&stTime);
}

time_t CTime::MakeTime(int iYear, int iMon, int iDay, int iHour, int iMin, int iSec)
{
    tm stTime;
    stTime.tm_year	= iYear-1900;
    stTime.tm_mon	= iMon-1;
    stTime.tm_mday	= iDay;
    stTime.tm_hour	= iHour;
    stTime.tm_min	= iMin;
    stTime.tm_sec	= iSec;

    return mktime(&stTime);
}


int CTime::ParseDate(const std::string& sDate, int& iYear, int& iMon, int& iDay, const std::string& sDelim)
{
    std::vector<std::string> vItems;
    CBasic::StringSplit(sDate, sDelim, vItems);

    if (vItems.size() < 3)
    {
        return -1;
    }

    iYear	= CTrans::STOI(vItems[0]);
    iMon	= CTrans::STOI(vItems[1]);
    iDay	= CTrans::STOI(vItems[2]);

    return 0;
}

int CTime::GetDate(int& iYear, int& iMon, int& iDay, time_t iTime)
{
    if (iTime < 0)
    {
        iTime = time(NULL);
    }

    tm* ptm = localtime( &iTime);

    iYear	= ptm->tm_year+1900;
    iMon	= ptm->tm_mon+1;
    iDay	= ptm->tm_mday;

    return iTime;
}

int CTime::GetDateExt(int& iYear, int& iMon, int& iDay, time_t iTime)
{

    tm* ptm = localtime(&iTime);

    iYear	= ptm->tm_year+1900;
    iMon	= ptm->tm_mon+1;
    iDay	= ptm->tm_mday;

    return iTime;
}

int CTime::GetWeek(int& iWeekDay, time_t iTime)
{
    if (iTime < 0)
    {
        iTime = time(NULL);
    }

    struct tm ptm  ;
    //tm* ptm = localtime((time_t*)&iTime);
    localtime_r( &iTime , &ptm);
    iWeekDay = ptm.tm_wday;

    return iTime;
}

int CTime::GetCurrentYear( void )
{
    int iYear = 0;
    time_t iTime = time(NULL);
    tm* ptm = localtime(&iTime);
    iYear	= ptm->tm_year+1900;

    return iYear;
}

int CTime::CheckDate(int& iYear, int& iMonth, int& iDay)
{
    static int s_nDays[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    int iRetCode = 0;
    if (iMonth < 1)
    {
        iMonth = 1;
        iRetCode = 0x02;
    }
    else if (iMonth > 12)
    {
        iMonth = 12;
        iRetCode = 0x02;
    }
    else if (iMonth == 2)
    {
        if (iDay < 1)
        {
            iDay = 1;
            iRetCode |= 0x01;
            return iRetCode;
        }
        int nDays = ((iYear%400==0) || (iYear%100!=0 && iYear%4==0)) ? 29 : 28;
        if (iDay > nDays)
        {
            iDay = nDays;
            iRetCode |= 0x01;
            return iRetCode;
        }
        return iRetCode;
    }
    if (iDay < 1)
    {
        iDay = 1;
        iRetCode |= 0x01;
    }
    else if (iDay > s_nDays[iMonth-1])
    {
        iDay = s_nDays[iMonth-1];
        iRetCode |= 0x01;
    }

    return iRetCode;
}

std::string CTime::UTCTime(time_t tTime)
{
    std::string sFmtTime;
    sFmtTime.resize(255);
    sFmtTime.resize(strftime(const_cast<char *>(sFmtTime.data()), sFmtTime.size(), "%a, %d %b %Y %H:%M:%S UTC", gmtime((time_t*)&(tTime))));
    return sFmtTime;
}

timeval CTime::GetTimeOfDay()
{
    timeval tv;
    gettimeofday(&tv,NULL);
    return tv;
}

time_t CTime::GetUSInterval(const timeval& tvfrom, const timeval& tvto)
{
    return (tvto.tv_sec-tvfrom.tv_sec)*1000000+(tvto.tv_usec-tvfrom.tv_usec);
}

time_t CTime::GetMSInterval(const timeval& tvfrom, const timeval& tvto)
{
    return (tvto.tv_sec-tvfrom.tv_sec)*1000+(tvto.tv_usec-tvfrom.tv_usec)/1000;
}

int CTime::GetCurrentuTime(bool bFast/* = true*/)
{
    uint64_t time=0;
    struct timeval now;
    if(bFast)
        gettimeofday( &now ,NULL);
    else
        gettimeofday(&now, NULL);
    time = now.tv_sec;
    time = time*1000000;
    time += now.tv_usec;
    return time;
}

uint64_t CTime::GetCurrentMSTime(bool bFast/* = true*/)
{
    uint64_t time=0;
    struct timeval now;
    if(bFast)
        gettimeofday(&now, NULL);
    else
        gettimeofday(&now, NULL);
    time = now.tv_sec;
    time = time*1000;
    time += now.tv_usec / 1000;

    return time;
}

int CTime::IsToday(time_t tTime)
{
    if(tTime <= 0)
        return -1;

    time_t now = time(NULL);

    struct tm p1, p2;

    localtime_r(&tTime, &p1);
    localtime_r(&now, &p2);

    if(p1.tm_year == p2.tm_year
        && p1.tm_mon == p2.tm_mon
        && p1.tm_mday == p2.tm_mday)
        return 0;

    return -1;
}

bool  CTime::IsInDuration( time_t iTime , time_t tDurationEnd )
{
    if(iTime == 0)
    {
        iTime = time(NULL);
    }

    if(iTime >= tDurationEnd)
    {
        return false;
    }
    else
    {
        int iDelta = tDurationEnd - iTime;
        if(iDelta <= 7* SECONDS_OF_DAY)
            return true;

        return false;

    }

}

//距离xx本周周期 结束时间
//iEndWeekDay 周几
//默认0：周日作为一周周期开始 //0~6
time_t CTime::GetDurationEnd(int iEndWeekDay, int iEndWeekHour, int iEndWeekMin, time_t now)
{
    if(now == 0)
    {
        now = time(NULL);
    }
	if( iEndWeekDay < 0
        || iEndWeekDay> 6 )
	{
        LOG_ERROR("wrong param, weekday:%d", iEndWeekDay);
		return 0;
	}

    time_t beginday = GetDayBeginTime(now);
	int weekday(0);
	CTime::GetWeek( weekday, now);

	int i(0);
	for( ; i < 7; ++i )
	{
		if( iEndWeekDay == ( weekday + i) % 7 )
		{
			break;
		}
	}

    //最后一天清榜时间
    int lastDaySeconds = iEndWeekHour*SECONDS_OF_HOUR + iEndWeekMin * SECONDS_OF_MIN;
    //same day
    if(i == 0)
    {
        if(now - beginday > lastDaySeconds)
        {
            //下周同一天
            i = 7;
        }
    }


	return ( SECONDS_OF_DAY * i + beginday)+ lastDaySeconds ;


//
//    //周期结束时间
//    time_t endtime(0);
//
//    struct tm tmnow;
//    localtime_r(&now, &tmnow);
//
//    int delta_week_day = tmnow.tm_wday - iEndWeekDay;
//    if(delta_week_day < 0)
//    {
//        int delta_sec = abs( delta_week_day) *SECONDS_OF_DAY + iEndWeekHour*SECONDS_OF_HOUR + iEndWeekMin*SECONDS_OF_MIN;
//        endtime =  beginday + delta_sec;
//        return endtime;
//    }
//    else if(delta_week_day == 0)
//    {
//        struct tm tmend;
//        localtime_r(&now, &tmend);
//
//        tmend.tm_hour = iEndWeekHour;
//        tmend.tm_min= iEndWeekMin;
//        tmend.tm_sec= 0;
//
//        time_t tEndTime = MakeTime(tmend);
//
//        //old 现在4点，结束12点，
//        if(tEndTime > now)
//        {
//            return tEndTime;
//        }
//        else
//        {
//            endtime = (7* SECONDS_OF_DAY) + tEndTime ;
//            return endtime;
//        }
//    }
//    else
//    {
//        int delta_sec = (7 -delta_week_day)*SECONDS_OF_DAY + iEndWeekHour*SECONDS_OF_HOUR + iEndWeekMin* SECONDS_OF_MIN;
//
//        endtime =  beginday + delta_sec;
//        return endtime;
//    }
//

}

void CTime::CheckTime(int& iHour, int& iMinute, int& iSecond)
{
    if(iHour < 0)
        iHour = 0;
    else if(iHour >= 24)
        iHour = 23;

    if(iMinute < 0)
        iMinute = 0;
    else if(iMinute > 59)
        iMinute = 59;

    if(iSecond < 0)
        iSecond = 0;
    else if(iSecond > 59)
        iSecond = 59;
}

int CTime::ParseDate(const std::string& sDate, time_t& tTime,
    const std::string& sDelim1/*="-"*/, const std::string& sDelim2/*=":"*/)
{
    std::vector<std::string> vItems;
    CBasic::StringSplit(sDate, " ", vItems);
    if(vItems.size() <= 0)
        return -1;

    int iYear(0), iMon(0), iDay(0);
    int iRet = ParseDate(vItems[0], iYear, iMon, iDay, sDelim1);
    if(iRet < 0)
        return -2;
    CheckDate(iYear, iMon, iDay);

    int iHour(0), iMinute(0), iSecond(0);
    if(vItems.size() > 1)
    {
        iRet = ParseDate(vItems[1], iHour, iMinute, iSecond, sDelim2);
        if(iRet < 0)
            return -4;

        CheckTime(iHour, iMinute, iSecond);
    }

    tTime = MakeTime(iYear, iMon, iDay, iHour, iMinute, iSecond);
    return 0;
}

std::string CTime::GetWeekStr( time_t iTime/*= -1*/ )
{
    if (iTime < 0)
    {
        iTime = time(NULL);
    }
    int iWeekDay = 0;
    GetWeek(iWeekDay, iTime);
    static std::string arrayWeekStr[] = {"周日","周一","周二","周三","周四","周五","周六"};
    if(iWeekDay <0 ||iWeekDay>6)
    {
        iWeekDay = 0;
    }
    return arrayWeekStr[iWeekDay];

}


