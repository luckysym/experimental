#include <stdint.h>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "inetchannel.h"

///////////////////////////////////////////
// Implementation

namespace sym
{

namespace detail
{
class CTcpChannelImpl
{
public:
	CTcpChannelImpl() {
		fd = -1;
		laddr.sin_addr.s_addr = INADDR_ANY;
		raddr.sin_addr.s_addr = INADDR_ANY;
		raddr.sin_port = 0;
		raddr.sin_port = 0;
		return ;
	}

public:
	int                fd;
	struct sockaddr_in laddr;
	struct sockaddr_in raddr;
}; // class CTcpChannelImpl

class CTcpChannelListenerImpl
{
public:
	CTcpChannelListenerImpl() {
		fd = -1;
		laddr.sin_addr.s_addr = INADDR_ANY;
		laddr.sin_port = 0;
		return ;
	}

public:
	int                fd;
	struct sockaddr_in laddr;
};

class CUdpChannelImpl
{
public:
	CUdpChannelImpl() {
		fd = -1;
		laddr.sin_addr.s_addr = INADDR_ANY;
		laddr.sin_port = 0;
		return ;
	}

public:
	int                fd;
	struct sockaddr_in laddr;
};

int MakeInetSockAddr(const std::string& ipaddr, uint16_t port, struct sockaddr_in& addr)
{
	int ret = ::inet_aton(ipaddr.c_str(), &addr.sin_addr);
	if ( !ret ) return -1;
	addr.sin_port = ::htons(port);
	return 0;
}

} // end of namespace detail


CTcpChannel::CTcpChannel(bool created) : m_pImpl(new detail::CTcpChannelImpl())
{
	if ( created )
		m_pImpl->fd = ::socket(PF_INET, SOCK_STREAM, 0);
	return ;	
};

CTcpChannel::CTcpChannel(const CTcpChannel& ch) : m_pImpl(new detail::CTcpChannelImpl())
{
	*m_pImpl = *ch.m_pImpl;
	return ;
}

CTcpChannel::~CTcpChannel()
{
	if ( m_pImpl->fd >= 0 ) this->Close();
	delete m_pImpl;
	return ;
}

int CTcpChannel::GetHandle() const
{
	return m_pImpl->fd;
}

std::string CTcpChannel::GetRemoteAddr() const
{
	char * str = ::inet_ntoa(m_pImpl->raddr.sin_addr);
	return std::string(str);
}

std::string CTcpChannel::GetLocalAddr() const
{
	char * str = ::inet_ntoa(m_pImpl->laddr.sin_addr);
	return std::string(str);
}

uint16_t CTcpChannel::GetRemotePort() const
{
	return ::ntohs(m_pImpl->raddr.sin_port);
}

uint16_t CTcpChannel::GetLocalPort() const
{
	return ::ntohs(m_pImpl->laddr.sin_port);
}

int CTcpChannel::Bind(const std::string& ipaddr, uint16_t port)
{
	struct sockaddr_in addr;
	int ret = detail::MakeInetSockAddr(ipaddr, port, addr);
	if ( ret < 0 ) return ret;
	ret = ::bind(m_pImpl->fd, (const struct sockaddr *)&addr, sizeof(addr));
	if ( ret == 0 ) this->m_pImpl->laddr = addr;
	return ret;
}

int CTcpChannel::Connect(const std::string& ipaddr, uint16_t port)
{
	struct sockaddr_in addr;
	int ret = detail::MakeInetSockAddr(ipaddr, port, addr);
	if ( ret < 0 ) return ret;
	ret = ::connect(m_pImpl->fd, (const struct sockaddr *)&addr, sizeof(addr));
	if ( ret == 0 ) this->m_pImpl->raddr = addr;
	return ret;
}

int CTcpChannel::Close()
{
	int ret = ::close(m_pImpl->fd);
	m_pImpl->fd = -1;
	return ret;
}


////////////////////////////////
// Implemenatation of CTcpChannelListener

CTcpChannelListener::CTcpChannelListener()
{
	m_pImpl->fd = ::socket(PF_INET, SOCK_STREAM, 0);
	return ;	
}

CTcpChannelListener::CTcpChannelListener(const CTcpChannelListener& lis)
	: m_pImpl(new detail::CTcpChannelListenerImpl())
{
	*this->m_pImpl = *lis.m_pImpl;
	return;
}

CTcpChannelListener::~CTcpChannelListener()
{
	if ( m_pImpl->fd >= 0 ) this->Close();
	delete this->m_pImpl;
	return ;
}

int CTcpChannelListener::GetHandle() const
{
	return this->m_pImpl->fd;
}

std::string CTcpChannelListener::GetLocalAddr() const
{
	char * str = ::inet_ntoa(m_pImpl->laddr.sin_addr);
	return std::string(str);
}

uint16_t CTcpChannelListener::GetLocalPort() const 
{
	return ::ntohs(m_pImpl->laddr.sin_port); 
}

int CTcpChannelListener::Bind(const std::string& ipaddr, uint16_t port)
{
	struct sockaddr_in addr;
	int ret = detail::MakeInetSockAddr(ipaddr, port, addr);
	if ( ret < 0 ) return ret;
	return ::bind(m_pImpl->fd, (struct sockaddr*)&addr, sizeof(addr));
}

int CTcpChannelListener::Listen(int backlog)
{
	return ::listen(m_pImpl->fd, backlog);	
}

int CTcpChannelListener::Accept(CTcpChannel& ch)
{
	struct sockaddr_in addr;
	socklen_t          len = sizeof(addr);
	int ret = ::accept(m_pImpl->fd, (struct sockaddr*)&addr, &len);
	if ( ret < 0) return ret;
	if ( ch.GetHandle() >= 0) ch.Close();
	ch.m_pImpl->fd = ret;
	ch.m_pImpl->laddr = this->m_pImpl->laddr;
	ch.m_pImpl->raddr = addr;
	return 0;
}

int CTcpChannelListener::Close() {
	int ret = ::close(m_pImpl->fd);
	m_pImpl->fd = -1;
	return ret;
}

//////////////////////////////////////////
// Implementation of CUdpChannel

CUdpChannel::CUdpChannel() : m_pImpl(new detail::CUdpChannelImpl())
{
	m_pImpl->fd = ::socket(PF_INET, SOCK_DGRAM, 0);
	return ;
}

CUdpChannel::CUdpChannel(const CUdpChannel& ch)
	: m_pImpl(new detail::CUdpChannelImpl())
{
	*this->m_pImpl = *ch.m_pImpl;
	return ;
}

CUdpChannel::~CUdpChannel()
{
	if ( this->m_pImpl->fd >= 0) this->Close();
	delete this->m_pImpl;
	this->m_pImpl = 0;
	return ;
}

int CUdpChannel::GetHandle() const
{
	return m_pImpl->fd;
}

std::string CUdpChannel::GetLocalAddr() const
{
	char * str = ::inet_ntoa(m_pImpl->laddr.sin_addr);
	return std::string(str);
}

uint16_t CUdpChannel::GetLocalPort() const
{
	return ::ntohs(m_pImpl->laddr.sin_port);
}

int CUdpChannel::Bind(const std::string& ipaddr, uint16_t port)
{
	struct sockaddr_in addr;
	int ret = detail::MakeInetSockAddr(ipaddr, port, addr);
	if ( ret < 0 ) return ret;
	ret = ::bind(m_pImpl->fd, (const struct sockaddr *)&addr, sizeof(addr));
	if ( ret == 0 ) this->m_pImpl->laddr = addr;
	return ret;
}

int CUdpChannel::Close()
{
	int ret = ::close(m_pImpl->fd);
	m_pImpl->fd = -1;
	return ret;
}


}; // end of namespace sym



