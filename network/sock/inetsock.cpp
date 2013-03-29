#include <stdint.h>
#include <string>
#include "sockintf.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

////////////////////////////////////////////
// Declaration of classes
//

namespace sym
{

class CInetSocketAddress : public IInetSocketAddress
{
public: 
	//---- Inherited from IInetSocketAddress

	uint32_t GetHostAddress() const;
	int      GetHostAddressStr(std::string& ipaddr) const;
	uint16_t GetPort() const;
	int      SetHostAddress(uint32_t addr);
	int      SetHostAddressStr(const std::string& ipaddr);
	int      SetPort(uint16_t port);

public:
	// Local member functions
	CInetSocketAddress();
	virtual ~CInetSocketAddress() {};  // keep empty

public:
	//---- Public variables
	struct sockaddr_in m_addr;
};

class CLocalSocketAddress : public ILocalSocketAddress
{
public:
	int  GetAddressPath(std::string& path) const;
	int  SetAddressPath(const std::string& path);
};

class CSocket : public ISocket
{
public:
	int GetHandle() const;
	int Close();
protected:
	int m_hsock;
};

class CServerSocket : public CSocket, public IServerSocket
{
public:
	int Listen(int backlog);
};

class CTcpSocket : public CSocket, public ITcpSocket
{
public:
	int Bind(const IInetSocketAddress& laddr);
	int Connect(const IInetSocketAddress& raddr);
};

class CTcpServerSocket : public CServerSocket, public ITcpServerSocket
{
public:
	int Accept(ITcpSocket& sock);
	int Bind(const IInetSocketAddress& laddr);
};

class CUdpSocket : public CSocket, public IUdpSocket
{
public:
	int Bind(const IInetSocketAddress& laddr);
};

class CLocalSocket : public CSocket, public ILocalSocket
{
public:
	int Bind(const ILocalSocketAddress& laddr);
	int Connect(const ILocalSocketAddress& raddr);
};

class CLocalServerSocket : public CServerSocket, public ILocalServerSocket
{
public:
	int Accept(ILocalSocket& rsock);
	int Bind(const ILocalSocketAddress& laddr);
};

} // end of namespace sym

////////////////////////////////////////////
// Implementation of classes
//

namespace sym
{

uint32_t CInetSocketAddress::GetHostAddress() const
{
	return ::ntohl(m_addr.sin_addr.s_addr);
}

int CInetSocketAddress::GetHostAddressStr(std::string& ipaddr) const
{
	char * pstr = ::inet_ntoa(m_addr.sin_addr);	
	ipaddr.assign(pstr);
	return 0;
}

uint16_t CInetSocketAddress::GetPort() const
{
	return ::ntohs(m_addr.sin_port);
}

int CInetSocketAddress::SetHostAddress(uint32_t addr)
{
	m_addr.sin_addr.s_addr = ::htonl(addr);
	return 0;
}

int CInetSocketAddress::SetHostAddressStr(const std::string& ipaddr) 
{
	int ret = ::inet_aton(ipaddr.c_str(), &m_addr.sin_addr);
	return ( ret )?0:-1;
};

int CInetSocketAddress::SetPort(uint16_t port)
{
	m_addr.sin_port = ::htons(port);
	return 0;
}

} // end of namespace sym

