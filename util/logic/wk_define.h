// Copyright (C) 2014  .
// Author: davidluan
// CreateTime: 2014-02-17

#ifndef _UTIL_LOGIC_WK_DEFINE_H
#define _UTIL_LOGIC_WK_DEFINE_H

#include "log/logger.h"
#include "wk_error.h"



#define CHECKNOTNULL(node) \
    if(node == NULL) \
{\
    LOG_ERROR("xml error format");\
    InitRet = ERR_WK_WRONG_CONF;\
    return  ERR_WK_WRONG_CONF  ;\
}\





#endif // _UTIL_LOGIC_WK_DEFINE_H
