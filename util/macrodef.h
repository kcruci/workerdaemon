// Copyright (C) 2013 - Inc.
// Author: davidluan
// CreateTime: 2013-09-07

#ifndef _UTIL_MACRODEF_H
#define _UTIL_MACRODEF_H

//curl 默认超时时间
#define DEFAULT_CURL_TIMEOUT_SECOND 2

#define FOR_EACH(it, container)                         \
    typeof((container).begin()) it = (container).begin();  \
    for(;it!= container.end(); ++it)                     \



#define MAX(a, b)  (a)>(b)?(a):(b)
#define MIN(a, b)  (a)>(b)?(b):(a)


#endif // _UTIL_MACRODEF_H
