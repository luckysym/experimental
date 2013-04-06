#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string>
#include <errno.h>
#include "localchannel.h"

////////////////////////////////////////////////
// Implementation of classes


#ifndef UNIX_PATH_MAX
#define UNIX_PATH_MAX 108
#endif 

namespace sym
{

namespace detail
{

class CLocalChannelImpl
{
public:
	CLocalChannelImpl()
	{
		this->fd = -1;
		laddr.sun_family = AF_LOCAL;
		laddr.sun_path[0] = 0;
		raddr.sun_family = AF_LOCAL;
		raddr.sun_path[0] = 0;
		taddr.sun_family = AF_LOCAL;
		taddr.sun_path[0] = 0;
		return ;
	}
public:
	int  fd;
	sockaddr_un laddr;
	sockaddr_un raddr;
	sockaddr_un taddr;
};

int MakeLocalSockAddress(const std::string& path, sockaddr_un& addr)
{
	if ( path.size() >= UNIX_PATH_MAX ){
		errno = ENAMETOOLONG;
		return -1;
	}
	addr.sun_family = AF_LOCAL;
	strcpy(addr.sun_path, path.c_str());
	return 0;
}


} // end of detail


CLocalChannel::CLocalChannel()
	: m_pImpl(new detail::CLocalChannelImpl())
{
	m_pImpl->fd = ::socket(PF_UNIX, SOCK_DGRAM, 0);
	return ;
}

CLocalChannel::CLocalChannel(const CLocalChannel& ch)
	: m_pImpl(new detail::CLocalChannelImpl())
{
	*m_pImpl = *ch.m_pImpl;
	return ;
}

CLocalChannel::~CLocalChannel()
{
	if ( m_pImpl->fd >=0 ) this->Close();
	delete m_pImpl;
	m_pImpl = 0;
	return ;
}

std::string CLocalChannel::GetLocalAddr() const
{
	return std::string(m_pImpl->laddr.sun_path);
}

std::string CLocalChannel::GetRemoteAddr() const
{
	return std::string(m_pImpl->raddr.sun_path);
}

std::string CLocalChannel::GetRecvAddr() const
{
	return std::string(m_pImpl->taddr.sun_path);
}

int CLocalChannel::Bind(const std::string& path)
{
	int ret = detail::MakeLocalSockAddress(path, m_pImpl->laddr);
	if ( ret != 0 ) {
		m_pImpl->laddr.sun_path[0] = 0;
		return ret;
	}
	ret = ::bind(m_pImpl->fd, (const struct sockaddr *)&m_pImpl->laddr, sizeof(m_pImpl->laddr)); 
	return ret;
}

int CLocalChannel::Connect(const std::string& path)
{
	int ret = detail::MakeLocalSockAddress(path, m_pImpl->raddr);
	if ( ret != 0 ) m_pImpl->raddr.sun_path[0] = 0;
	return ret;	
}

int CLocalChannel::Close()
{
	int ret = ::close(m_pImpl->fd);
	m_pImpl->fd = -1;
	return ret;
}

int CLocalChannel::Send(const char *buf, size_t size, int flags)
{
	if ( !buf || !size) return 0;
	return ::sendto(m_pImpl->fd, buf, size, flags,
	                (const struct sockaddr*)&m_pImpl->raddr,
					sizeof(m_pImpl->raddr));
}

int CLocalChannel::Recv(char *buf, size_t size, int flags)
{
	if ( !buf || !size ) return 0;
	socklen_t len = sizeof(m_pImpl->taddr);
	return ::recvfrom(m_pImpl->fd, buf, size, flags,
	                  (struct sockaddr *)&m_pImpl->taddr, &len);
}



} // end of namespace sym
