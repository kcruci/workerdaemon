#ifndef _QOS_AGENT_H_
#define _QOS_AGENT_H_
#include <netdb.h> 
#include <sys/socket.h> 
#include <stdio.h> 
#include <errno.h> 
#include <sys/time.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <stdint.h>
#include <string>
#include <vector>
extern int h_errno;
using namespace std;

#define QOS_TM_MAX			20

//ApiQosReqCheck的返回码
enum _QOS_RTN
{
    QOS_RTN_OK	   		= 0x00,
    QOS_RTN_ACCEPT,					//接收
    QOS_RTN_LROUTE,
    QOS_RTN_TROUTE,
    QOS_RTN_STATIC_ROUTE,  // return route from local static route
    QOS_RTN_INITED,
    QOS_RTN_OVERLOAD	= -10000,	//过载
    QOS_RTN_TIMEOUT,				//超时
    QOS_RTN_SYSERR,					//系统错误
    QOS_RTN_SENDERR,
    QOS_RTN_RECVERR,
    QOS_MSG_INCOMPLETE,
    QOS_CMD_ERROR,
    QOS_MSG_CMD_ERROR,
    QOS_INIT_CALLERID_ERROR,
    QOS_RTN_PARAM_ERROR
};

//QOS运行模式
enum _QOS_RUN_TYPE
{
    QOS_TYPE_REQ = 0x00,		//访问量控制方式
    QOS_TYPE_LIST				//并发量控制方式
};

struct QOSREQUESTTMEXTtag;
//单次访问的基本信息
typedef struct QOSREQUESTtag
{
    int			    _flow;
    int			    _modid;		//被调模块编码
    int			    _cmd;		//被调接口编码
    string		    _host_ip;	//被调主机IP
    unsigned short  _host_port;	//被调主机PORT
    int _pre;     

    QOSREQUESTtag(const QOSREQUESTTMEXTtag &route);
    QOSREQUESTtag():_flow(0),_modid(0),_cmd(0),_host_port(0),_pre(0){}
}QOSREQUEST;

//新增带时延信息的接口
struct QOSREQUESTTMEXTtag
{
    int			_flow;
    int			_modid;		//被调模块编码
    int			_cmd;		//被调接口编码
    string		_host_ip;	//被调主机IP
    unsigned short _host_port; //被调主机PORT
    int _pre;        
    int _delay;        

    QOSREQUESTTMEXTtag( ):_flow(0),_modid(0),_cmd(0),_host_port(0),_pre(0),_delay(0){}
    QOSREQUESTTMEXTtag( const QOSREQUEST & route);
};
typedef struct QOSREQUESTTMEXTtag QOSREQUEST_TMEXT;

typedef struct QOSREQUESTMTTCEXTtag
{
    int32_t			_modid;		//被调模块编码
    int32_t			_cmdid;
    int64_t			_key;		//被调接口编码
    int32_t			_funid;
    string		    _host_ip;	//被调主机IP
    unsigned short	_host_port;	//被调主机PORT
}QOSREQUEST_MTTCEXT;

typedef struct QOSREQUESTMTTC_TMEXTtag
{
    int32_t			_modid;		//被调模块编码
    int32_t			_cmdid;
    int64_t			_key;		//被调接口编码
    int32_t			_funid;
    string		    _host_ip;	//被调主机IP
    unsigned short  _host_port;	//被调主机PORT
    int32_t			_delay;     //路由访问时延
}QOSREQUEST_MTTCEXT_TM;

// 新增用于返回指定modid和cmdid下的路由表的相关数据结构定义 
typedef struct ROUTEtag
{
    string _host_ip;
    unsigned short _host_port;
}QOSROUTE;
typedef struct QOSREQUESTROUTETABLEtag
{
    int			_modid;		//被调模块编码
    int			_cmdid;		//被调接口编码
    vector<QOSROUTE> _route_tb;

    QOSREQUESTROUTETABLEtag():_modid(0),_cmdid(0){}
}QOSREQUEST_RTB;

//单次访问的基本信息
typedef struct CALLERQOSREQUESTtag
{
    int			_flow;
    int         _callermod;
    int         _callercmd;
    int			_calledmodid;	//被调模块编码
    int			_calledcmd;		//被调接口编码
    string		_host_ip;	    //被调主机IP
    unsigned short _host_port;	//被调主机PORT
}CALLERQOSREQUEST;

typedef struct CALLERQOSREQUESTMTTCEXTtag
{
    int         _callermod;
    int         _callercmd;
    int			_calledmodid;	//被调模块编码
    int			_calledcmdid;
    int64_t		_calledkey;		//被调接口编码
    int32_t		_funid;
    string		_host_ip;	    //被调主机IP
    unsigned short	_host_port;	//被调主机PORT
}CALLERQOSREQUEST_MTTCEXT;

//QOS访问量控制的配置信息
struct QOSREQCFG
{
    int			_req_max;			//访问量控制的最大值
    int			_req_min;			//访问量控制的最小值
    float		_req_err_min;		//错误的最大阀值[小于这个值认为是无错]
    float		_req_extend_rate;	//无错误的时候的阀值扩张率

    QOSREQCFG()
    {
        _req_max = 0;
        _req_min = 0;
        _req_err_min = 0.0;
        _req_extend_rate = 0.0;
    };
};

//QOS并发量控制的配置信息
struct QOSLISTCFG
{
    int			_list_max;			//并发量控制的最大值
    int			_list_min;			//并发量控制的最小值
    float		_list_err_min;		//并发的最大阀值[小于这个值认为是无错]
    float		_list_extend_rate;	//并发无错误的时候的阀值扩张率

    QOSLISTCFG()
    {
        _list_max = 0;
        _list_min = 0;
        _list_err_min = 0.0;
        _list_extend_rate = 0.0;
    };
};

//QOS访问按时间分段的配置信息
struct QOSTMCFG
{
    int		_tm_cfg_count;					//配置的个数
    int		_begin_usec[QOS_TM_MAX];		//返回时间段开始时间
    int		_end_usec[QOS_TM_MAX];			//返回时间段结束时间
    int		_ret[QOS_TM_MAX];				//返回值[0:成功 -1:失败]

    QOSTMCFG()
    {
        _tm_cfg_count = 0;
        for ( int i=0;i<QOS_TM_MAX;i++ )
        {
            _begin_usec[i] = 0;
            _end_usec[i] = 0;
            _ret[i] = 0;
        };
    };
};

typedef struct Route_CycTm
{
    int _modid;
    int _cmd;
    int tm;
    int minreq;
    int maxreq;
    float ext;
    float min_err;

} ROUTE_CYCTM;

/*
 *   Function: ApiInitRoute
 *   Args:
 *       modid:      modid needed to be inited
 *       cmdid:      cmdid needed to be inited
 *       time_out:   time limit to fetch route    
 *       err_msg:    error messange when return value<0
 *   Ret:
 *       0 for route(modid,cmdid) alreadly in l5agent 
 *       4 for alreadly notify dnsagent to fetch route(modid,cmdid) 
 *       <0 for errors,while err_msg will be filled 
 */
int ApiInitRoute(int modid, int cmdid, float time_out, string& err_msg);

/*
 *   Function: ApiGetRoute
 *   Args:
 *       qos_req:	feedback info reported to L5_agent,including modid, cmdid
 *       time_out:	time limit to fetch route,the actrual time limit is 1s* time_out	
 *       err_msg:	error messange when return value<0
 *       tm_val:	time stamp transfered to api,in order to reduce gettimeofday,default as NULL
 *   Ret:
 *       0 for OK
 *       <0 for errors,while err_msg will be filled 
 */

//路由
int ApiGetRoute(QOSREQUEST& qos_req,float time_out,string& err_msg,struct timeval* tm_val=NULL);
int ApiGetRoute(QOSREQUEST_MTTCEXT& qos_req,float time_out,string& err_msg,struct timeval* tm_val=NULL);
int ApiGetRoute(QOSREQUEST_TMEXT& qos_req,float time_out,string& err_msg,struct timeval* tm_val=NULL);
int ApiGetRoute(QOSREQUEST_MTTCEXT_TM& qos_req,float time_out,string& err_msg,struct timeval* tm_val=NULL);
int ApiAntiParallelGetRoute(QOSREQUEST_MTTCEXT& qos_req,float time_out,string& err_msg,struct timeval* tm_val=NULL);

/*
 *   Function: ApiRouteResultUpdate
 *   Args:
 *       qos_req:	feedback info reported to L5_agent,including modid, cmdid
 *       ret:		status report to L5_agent,0 for nomal ,<0 for abnormal
 *       tm_val:	time stamp transfered to api,in order to reduce gettimeofday,default as NULL
 *       err_msg:	error messange when return value<0
 *   Ret:
 *       0 for OK
 *       <0 for errors,while err_msg will be filled 
 */

int ApiRouteResultUpdate(QOSREQUEST& qos_req,int ret, int usetime_usec,string& err_msg,struct timeval* tm_val=NULL);
int ApiRouteResultUpdate(QOSREQUEST_MTTCEXT& qos_req,int ret, int usetime_usec,string& err_msg,struct timeval* tm_val=NULL);
int ApiRouteResultUpdate(QOSREQUEST_TMEXT& qos_req,int ret,int usetime_usec,string& err_msg,struct timeval* tm_val=NULL);
int ApiAntiParallelUpdate(QOSREQUEST_MTTCEXT& qos_req,int ret,int usetime_usec,string& err_msg,struct timeval* tm_val=NULL);
int ApiRouteResultUpdate(QOSREQUEST_MTTCEXT_TM& qos_req,int ret,int usetime_usec,string& err_msg,struct timeval* tm_val=NULL);

#endif
