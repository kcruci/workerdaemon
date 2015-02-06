/**
 * C++ SDK for  OpenAPI V3
 *
 * @version 3.0.0
 * @author open.qq.com
 * @copyright (c) 2013, Tencent Corporation. All rights reserved.
 * @ History:
 *               3.0.0 | jixingguan | 2013-05-06 10:18:11 | initialization
 */
#ifndef TENCENT_OPEN_API_COMM_DEF_H
#define TENCENT_OPEN_API_COMM_DEF_H
#include <string>
#include <vector>
#include <string.h>
#include <sstream>
#include <map>

using namespace std;
#ifndef foreach
#define foreach(container,it) \
    for(typeof((container).begin()) it = (container).begin();it!=(container).end();++it)
#endif

#ifndef OPENAPIV3_ERROR
#define OPENAPIV3_ERROR(fmt, args...) \
    snprintf(m_errmsg, sizeof(m_errmsg), "[%s][%d][%s]"fmt, \
             __FILE__, __LINE__,__FUNCTION__, ##args)
#endif

#define OutPutDebug(fmt, args...)  printf(fmt,##args)


#endif
