// Copyright (C) 2014 - isky Inc.
// Author: davidluan
// CreateTime: 2014-02-13

#include "logger.h"
#include <unistd.h>
#include "basic/trans.h"
#include <stdlib.h>
#include <string.h>

using namespace std;

static char g_szLogBuff[1024];
static char g_szLogBuffFuncionLine[2048];
CRLLog::CRLLog()
{
    _isInited = false;
    layout = NULL;
    appender = NULL;
    pCate = NULL;
}
CRLLog::~CRLLog()
{
    if (layout != NULL){
        free(layout);
        layout = NULL;
    }
    if (appender != NULL){
        free(appender);
        appender = NULL;
    }
    if (pCate != NULL){
        free(pCate);
        pCate = NULL;
    }
}

void CRLLog::Init(const std::string& path,const std::string& filename, const std::string& logconf)
{
    if (_isInited)
        return;
    if (access(path.c_str(),W_OK) != 0)
    {
        mkdir(path.c_str(),S_IRWXU|S_IRGRP|S_IROTH );
    }

    if (access(path.c_str(),W_OK) != 0)
    {
        fprintf(stderr, "log path:%s no exist!\n", path.c_str());
        return;
    }

    layout = new log4cpp::PatternLayout();
    layout->setConversionPattern("%d: %p %c %x: %m%n");
    std::string  logname = filename;
    size_t pos = logname.find_last_of('/');
    if(pos!= string::npos)
    {
        logname = logname.substr(pos);
    }
    pos = logname.rfind('.');
    if(pos!= string::npos)
    {
        logname = logname.substr(0, pos);
    }

    logname += ".log";
    appender = new log4cpp::RollingFileAppender("FileAppender",(path+logname).c_str(),MAX_SIZE,MAX_COUNT);
    if (layout == NULL || appender == NULL)
        return;
    appender->setLayout(layout);
    pCate = &(log4cpp::Category::getInstance(""));
    pCate->setAdditivity(false);
    pCate->setAppender(appender);
    pCate->setPriority(log4cpp::Priority::WARN);
    _isInited = true;
    _sLogConf = logconf;
    readConfig();
}
void CRLLog::SetUid(const std::string& sUid)
{
	_sUid = sUid;
}

void CRLLog::Log_ERROR(const char* msg)
{
    Log(log4cpp::Priority::ERROR, msg);
}
void CRLLog::Log_DEBUG(const char*  msg)
{
    Log(log4cpp::Priority::DEBUG, msg);
}
void CRLLog::Log_NOTICE(const char*  msg)
{
    Log(log4cpp::Priority::NOTICE, msg);
}


void CRLLog::Log(log4cpp::Priority::PriorityLevel lv,const char*  msg)
{
    if (!_isInited)
        return;
    if(_sUid.empty())
    {
        pCate->log(lv, "%s", msg);
    }
    else
    {
        pCate->log(lv, "uid[%s]%s",_sUid.c_str(), msg);
    }
}


void CRLLog::Log_ERROR(const char* file, int line, const char* fmt, ...)
{
    //memset(g_szLogBuff, 0, sizeof(g_szLogBuff));
    va_list vl;
    va_start(vl, fmt);
    ::vsnprintf(g_szLogBuff, sizeof(g_szLogBuff), fmt, vl);
    va_end(vl);
    std::string filename= file;
    size_t pos = filename.find_last_of('/');
    if(pos!= string::npos)
    {
        filename= filename.substr(pos+1);
    }

    //memset(g_szLogBuffFuncionLine, 0, sizeof(g_szLogBuffFuncionLine));
    snprintf(g_szLogBuffFuncionLine, sizeof(g_szLogBuffFuncionLine),
        "[%s:%d]%s", filename.c_str(), line, g_szLogBuff);

    Log_ERROR(g_szLogBuffFuncionLine);
}

void CRLLog::Log_NOTICE(const char* file, int line, const char* fmt, ...)
{
    va_list vl;
    va_start(vl, fmt);
    ::vsnprintf(g_szLogBuff, sizeof(g_szLogBuff), fmt, vl);
    va_end(vl);
    std::string filename= file;
    size_t pos = filename.find_last_of('/');
    if(pos!= string::npos)
    {
        filename= filename.substr(pos+1);
    }
    snprintf(g_szLogBuffFuncionLine, sizeof(g_szLogBuffFuncionLine),
        "[%s:%d]%s", filename.c_str(), line, g_szLogBuff);

    Log_NOTICE(g_szLogBuffFuncionLine);

}
void CRLLog::Log_DEBUG(const char* file, int line, const char* fmt, ...)
{
    //memset(g_szLogBuff, 0, sizeof(g_szLogBuff));
    va_list vl;
    va_start(vl, fmt);
    ::vsnprintf(g_szLogBuff, sizeof(g_szLogBuff), fmt, vl);
    va_end(vl);
    std::string filename= file;
    size_t pos = filename.find_last_of('/');
    if(pos!= string::npos)
    {
        filename= filename.substr(pos+1);
    }
    snprintf(g_szLogBuffFuncionLine, sizeof(g_szLogBuffFuncionLine),
        "[%s:%d]%s", filename.c_str(), line, g_szLogBuff);

    Log_DEBUG(g_szLogBuffFuncionLine);
}

void CRLLog::readConfig()
{
    //默认error 级别
    if(_sLogConf.empty())
    {
        pCate->setPriority(log4cpp::Priority::ERROR);
        return;
    }
    FILE *fp = fopen(_sLogConf.c_str(),"r");
    if (fp == NULL)
        return;
    char tmp[512];
    if ( fgets(tmp,500,fp) == NULL)
        return;
    if ( strncmp("DEBUG",tmp,5) == 0)
        pCate->setPriority(log4cpp::Priority::DEBUG);
    else if ( strncmp("NOTICE",tmp,5) == 0)
        pCate->setPriority(log4cpp::Priority::NOTICE);
    else if ( strncmp("ERROR",tmp,5) == 0)
        pCate->setPriority(log4cpp::Priority::ERROR);

}

