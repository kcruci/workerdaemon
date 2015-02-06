/**
 * udp�ͻ�����
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
	**������ Open
	**����:   
	**			host_ip      server�˵�IP
				host_port   server�˵�IP
	**���:  
	**				
	**����: 0�ɹ�
	**		  ����ʧ��
	**����:��ʼ��Ҫ���ӵ�IP��PORT
	**			  
	**************************************************************/		
	int Open(const char* host_ip,unsigned short host_port);

	/***********************************************************
	**������ Close
	**����:   
				��
	**���:  
	**				
	**����:  0    �ɹ�
	**		  ����ʧ��
	**����:�ر���������
	**			  
	**************************************************************/		
	int Close();

	/***********************************************************
	**������ GetHost
	**����:   
				��
	**���:  
	**   			host_ip      ��ʼ����IP
				host_port   ��ʼ���Ķ˿�
	**				
	**����:  0    �ɹ�
	**		  ����ʧ��
	**����:��ȡ��ʼ����IP��PORT��Ĭ�ϳ�ʱ����Ϣ
	**************************************************************/	
	void GetHost(unsigned& host_ip,unsigned short& host_port);
	
	/***********************************************************
	**������ Connect
	**����:   
				��
	**���:  
				��
	**				
	**����:  0    �ɹ�
	**		  ����ʧ��
	**����:����udp connect
	**************************************************************/	
	int Connect();

	/***********************************************************
	**������ SendBuf
	**����:   
				��
	**���:  
				��
	**				
	**����:  0    �ɹ�
	**		   ����ʧ��
	**����:��������
	**************************************************************/	
	int SendBuf(const char* pbuf,int buf_len);

	/***********************************************************
	**������ GetErrMsg
	**����:   
				��
	**���:  
				��
	**				
	**����:  ������Ϣ
	**����:���س�����Ϣ
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
