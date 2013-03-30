#include <stdint.h>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

////////////////////////////////////////////
// Declaration of classes
//

namespace sym
{


namespace detail 
{
	class CTcpChannelImpl;
}

class CTcpChannel
{
public:
	CTcpChannel();
	CTcpChannel(const CTcpChannel& ch);
	~CTcpChannel();

	int         GetHandle() const;
	std::string GetRemoteAddr() const;
	uint16_t    GetRemotePort() const;
	std::string GetLocalAddr()  const;
	uint16_t    GetLocalPort()  const;

	int  Bind(const std::string& ipaddr, uint16_t port);
	int  Connect(const std::string& ipaddr, uint16_t port);
	int  Close();

private:
	CTcpChannel& operator=(const CTcpChannel& ch);

private:
	detail::CTcpChannelImpl * m_pImpl;
}; // class CTcpChannel

} // end of namespace sym

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
	int                 fd;
	struct sockaddr_in  laddr;
	struct sockaddr_in raddr;
}; // class CTcpChannelImpl

int MakeInetSockAddr(const std::string& ipaddr, uint16_t port, struct sockaddr_in& addr)
{
	int ret = ::inet_aton(ipaddr.c_str(), &addr.sin_addr);
	if ( !ret ) return -1;
	addr.sin_port = ::htons(port);
	return 0;
}

} // end of namespace detail


CTcpChannel::CTcpChannel() : m_pImpl(new detail::CTcpChannelImpl())
{
	m_pImpl->fd = ::socket(PF_INET, SOCK_STREAM, 0);
	return ;	
};

CTcpChannel::CTcpChannel(const CTcpChannel& ch)
{
	if ( ch.m_pImpl == this->m_pImpl ) return ;
	if ( m_pImpl->fd >= 0 && m_pImpl->fd != ch.m_pImpl->fd) {
		this->Close();
	}
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

}; // end of namespace sym


