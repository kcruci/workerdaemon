/**
 * udp�ͻ�����
 *
 * @version 3.0.0
 * @author open.qq.com
 * @copyright (c) 2013, Tencent Corporation. All rights reserved.
 * @ History:
 *               3.0.0 | jixingguan | 2013-05-06 10:18:11 | initialization
 */

#include "OpenApiUdpClient.h"

#define CHECK_INIT_HOST() \
if ( m_host_ip.length() <= 0 || m_host_port ==0 ) \
{ \
		snprintf(m_err_msg,sizeof(m_err_msg),"[%s][%d]invalid ip or port not init error\n", \
								__FILE__,__LINE__); \
		return -1; \
}

#define CHECK_INIT_SOCK() \
if ( m_sock_fd <= 0 ) \
{ \
		snprintf(m_err_msg,sizeof(m_err_msg),"[%s][%d]invalid socket fd not init error\n", \
								__FILE__,__LINE__); \
		return -1; \
}

CUdpClient::CUdpClient()
{
	m_sock_fd = -1;
	
	m_host_ip = "";
	m_host_port = 0;
	
	m_connect_flag = false;
	
	bzero(&m_host_addr,sizeof(struct sockaddr_in));
	m_host_addr_len = sizeof(struct sockaddr_in);	
}

CUdpClient::~CUdpClient()
{
	Close();
}

/***********************************************************
**������ Open
**����:   
**          host_ip      server�˵�IP
            host_port   server�˵�IP
**���:  
**              
**����: 0�ɹ�
**        ����ʧ��
**����:��ʼ��Ҫ���ӵ�IP��PORT
**            
**************************************************************/ 
int CUdpClient::Open(const char* host_ip,unsigned short host_port)
{
	m_host_ip = host_ip;
	m_host_port = host_port;
	
	bzero(&m_host_addr,sizeof(struct sockaddr_in));
	m_host_addr.sin_family = AF_INET;
	inet_aton(host_ip,&m_host_addr.sin_addr);
	m_host_addr.sin_port = htons(host_port);	
	
	return 0;
}

/***********************************************************
**������ Close
**����:   
            ��
**���:  
**              
**����:  0    �ɹ�
**        ����ʧ��
**����:�ر���������
**            
**************************************************************/
int CUdpClient::Close()
{
	if ( m_sock_fd > 0 ) close(m_sock_fd);
	m_sock_fd = -1;
	
	return 0;
}

/***********************************************************
**������ GetHost
**����:   
            ��
**���:  
**              host_ip      ��ʼ����IP
            host_port   ��ʼ���Ķ˿�
**              
**����:  0    �ɹ�
**        ����ʧ��
**����:��ȡ��ʼ����IP��PORT
**************************************************************/ 
void CUdpClient::GetHost(unsigned& host_ip,unsigned short& host_port)
{
	host_ip = SockStr2Int(m_host_ip.c_str());
	host_port = m_host_port;
}

/***********************************************************
**������ Connect
**����:   
            ��
**���:  
            ��
**              
**����:  0    �ɹ�
**        ����ʧ��
**����:����udp connect
**************************************************************/ 
int CUdpClient::Connect()
{	
	CHECK_INIT_HOST();
	if ( m_sock_fd <= 0 )
	{
		m_sock_fd = socket(AF_INET,SOCK_DGRAM,0);
    	if( m_sock_fd <= 0 )
    	{
			snprintf(m_err_msg,sizeof(m_err_msg),"[%s][%d]create udp socket ip:%s port:%d err:%s\n",
					__FILE__,__LINE__,m_host_ip.c_str(),m_host_port,strerror(errno));		
			return ENUM_SOCKRSP_ERR_CONNECT;
		}
	}
	CHECK_INIT_SOCK();
	
	m_err_msg[0] = '\0';
	
	int _ret;
	_ret = connect(m_sock_fd, (const struct sockaddr *)&m_host_addr, m_host_addr_len);
	if ( _ret )
	{
		snprintf(m_err_msg,sizeof(m_err_msg),"[%s][%d]socket connect ip:%s port:%d ret:%d err:%s\n",
				__FILE__,__LINE__,
				m_host_ip.c_str(),m_host_port,_ret,strerror(errno));
		return ENUM_SOCKRSP_ERR_CONNECT;
	}
	m_connect_flag = true;
	
	return 0;
}

/***********************************************************
**������ SendBuf
**����:   
                pbuf        ���ͻ�����
                buf_len    �������ĳ���
**���:  
            ��
**              
**����:  0    �ɹ�
**         ����ʧ��
**����:��������
**************************************************************/ 

int CUdpClient::SendBuf(const char* pbuf,int buf_len)
{
    if(NULL == pbuf)
    {
		snprintf(m_err_msg,sizeof(m_err_msg),"[%s][%d]invalid send buf\n", \
								__FILE__,__LINE__); \
        return ENUM_SOCKRSP_ERR_SYSTEM;
    }

	if ( m_sock_fd <= 0 )
	{
		m_sock_fd = socket(AF_INET,SOCK_DGRAM,0);
    	if( m_sock_fd <= 0 )
    	{
			snprintf(m_err_msg,sizeof(m_err_msg),"[%s][%d]create udp socket ip:%s port:%d err:%s\n",
					__FILE__,__LINE__,m_host_ip.c_str(),m_host_port,strerror(errno));		
			return ENUM_SOCKRSP_ERR_CONNECT;
		}
	}
	
	m_err_msg[0] = '\0';
	
	int _ret;
	if ( m_connect_flag )
		_ret = write(m_sock_fd,pbuf,buf_len);
	else
		_ret = sendto(m_sock_fd,pbuf,buf_len,0,(struct sockaddr*)&m_host_addr,m_host_addr_len);	
	if ( _ret != buf_len )
	{
		snprintf(m_err_msg,sizeof(m_err_msg),"[%s][%d]socket sendto ip:%s port:%d buf-len:%d ret:%d err:%s\n",
				__FILE__,__LINE__,
				m_host_ip.c_str(),m_host_port,buf_len,_ret,strerror(errno));		
		return ENUM_SOCKRSP_ERR_SEND;
	}
	
	return 0;
}
unsigned CUdpClient::SockStr2Int(const char* host_ip)
{
	return (unsigned)inet_addr(host_ip);	
}




