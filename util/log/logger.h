// Copyright (C) 2014 - isky Inc.
// Author: davidluan
// CreateTime: 2014-02-13


#ifndef _LOGGER_H
#define _LOGGER_H


#include    <string>
#define LOG4CPP_FIX_ERROR_COLLISION 1
#include    "log4cpp/Category.hh"
#include    "log4cpp/FileAppender.hh"
#include    "log4cpp/BasicLayout.hh"
#include    "log4cpp/RollingFileAppender.hh"
#include    "log4cpp/PatternLayout.hh"
#include    "singleton.h"


#define SETUID(uid) CSingleton<CRLLog>::Instance()->SetUid(uid)
#define MAX_SIZE 512*1024*1024
#define MAX_COUNT 20


//log pretty
#define INIT_LOG(path,filename,logconf) CSingleton<CRLLog>::Instance()->Init(path,filename,logconf)
#define LOG_ERROR(fmt, args...) CSingleton<CRLLog>::Instance()->Log_ERROR(__FILE__, __LINE__, fmt, ##args)
#define LOG_DEBUG(fmt, args...) CSingleton<CRLLog>::Instance()->Log_DEBUG(__FILE__, __LINE__, fmt, ##args)
#define LOG_NOTICE(fmt, args...) CSingleton<CRLLog>::Instance()->Log_NOTICE(__FILE__, __LINE__, fmt, ##args)

class CRLLog
{
public:
    CRLLog();
    ~CRLLog();
    void Init(const std::string& path,const std::string& name, const std::string& logconf="");
	void SetUid(const std::string& sUid);
    //void Log_ERROR(const std::string& msg);
    //void Log_DEBUG(const std::string& msg);

	//formated log
    void Log_ERROR(const char* srcfile, int line, const char* fmt, ...);
    void Log_DEBUG(const char* srcfile, int line, const char* fmt, ...);
    void Log_NOTICE(const char* srcfile, int line, const char* fmt, ...);


protected:
    void Log_ERROR(const char * msg);
    void Log_NOTICE(const char * msg);
    void Log_DEBUG(const char * msg);


private:
    void Log(log4cpp::Priority::PriorityLevel lv,const char*  msg);

    void readConfig();
    bool _isInited;
    log4cpp::PatternLayout* layout;
    log4cpp::RollingFileAppender* appender;
    log4cpp::Category* pCate;
	std::string _sUid;
	std::string _sLogConf;
};



#endif//_LOGGER_H
