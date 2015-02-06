/**
 * udp客户端类
 *
 * @version 3.0.1
 * @author open.qq.com
 * @copyright (c) 2013, Tencent Corporation. All rights reserved.
 * @ History:
 *				 3.0.1 | harryhlli  | 2013-09-06 09:50:19 | fixed compilation errors
 *               3.0.0 | jixingguan | 2013-05-27 10:18:11 | initialization
 */
#ifndef OPEN_API_UDP_CLIENT_H
#define OPEN_API_UDP_CLIENT_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <errno.h>
#include <string.h>
#include <stdio.h>

enum ENUM_SOCK_RSP
{
	ENUM_SOCKRSP_SUC = 0x00,
	ENUM_SOCKRSP_ERR_SYSTEM = -1,
	ENUM_SOCKRSP_ERR_SEND=-30,
	ENUM_SOCKRSP_ERR_CONNECT,
};


class CUdpClient
{
public:
	CUdpClient();
	~CUdpClient();
	
	/***********************************************************
	**函数名 Open
	**输入:   
	**			host_ip      server端的IP
				host_port   server端的IP
	**输出:  
	**				
	**返回: 0成功
	**		  其他失败
	**描述:初始化要连接的IP、PORT
	**			  
	**************************************************************/		
	int Open(const char* host_ip,unsigned short host_port);

	/***********************************************************
	**函数名 Close
	**输入:   
				无
	**输出:  
	**				
	**返回:  0    成功
	**		  其他失败
	**描述:关闭网络链接
	**			  
	**************************************************************/		
	int Close();

	/***********************************************************
	**函数名 GetHost
	**输入:   
				无
	**输出:  
	**   			host_ip      初始化的IP
				host_port   初始化的端口
	**				
	**返回:  0    成功
	**		  其他失败
	**描述:获取初始化的IP、PORT、默认超时等信息
	**************************************************************/	
	void GetHost(unsigned& host_ip,unsigned short& host_port);
	
	/***********************************************************
	**函数名 Connect
	**输入:   
				无
	**输出:  
				无
	**				
	**返回:  0    成功
	**		  其他失败
	**描述:进行udp connect
	**************************************************************/	
	int Connect();

	/***********************************************************
	**函数名 SendBuf
	**输入:   
				无
	**输出:  
				无
	**				
	**返回:  0    成功
	**		   其他失败
	**描述:发送数据
	**************************************************************/	
	int SendBuf(const char* pbuf,int buf_len);

	/***********************************************************
	**函数名 GetErrMsg
	**输入:   
				无
	**输出:  
				无
	**				
	**返回:  出错信息
	**描述:返回出错信息
	**************************************************************/	
	const char* GetErrMsg() { return (char*)&m_err_msg[0]; };
private:
	unsigned SockStr2Int(const char* host_ip);
private:
	char				m_err_msg[512];			//
	int					m_sock_fd;				//
	std::string				m_host_ip;				//
	unsigned short		m_host_port;			//
	bool				m_connect_flag;			//	
	struct sockaddr_in 	m_host_addr;
	socklen_t			m_host_addr_len;
};
#endif
