#ifndef	__TYPE_DEFS_H__
#define	__TYPE_DEFS_H__

#include <typeinfo>
#include <stdint.h>
#include <map>




typedef unsigned int  uin_t;
typedef unsigned int  uin_u;

//有序参数map
typedef std::map<std::string, std::string , std::less<std::string> > ParamMap;
#ifdef INT_MIN
#undef INT_MIN
#endif
#ifdef INT_MAX
#undef INT_MAX
#endif
#ifdef UINT_MAX
#undef UINT_MAX
#endif
enum LIMIT_TYPE
{
INT_MIN    = -0x7FFFFFFF,        /*!< ÕûÐÍÄ¬ÈÏ×îÐ¡Öµ */
INT_MAX    = +0x7FFFFFFF,        /*!< ÕûÐÍÄ¬ÈÏ×î´óÖµ */
UINT_MAX    = +0xFFFFFFFF,        /*!< ÕûÐÍÄ¬ÈÏ×î´óÖµ */

};

//#define TRUE 1
//#define FALSE 0

#endif

