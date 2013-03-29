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

class CInetSocketAddress : virtual public IInetSocketAddress
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

public:
	//---- Public variables
	struct sockaddr_in m_addr;
};

class CLocalSocketAddress : virtual public ILocalSocketAddress
{
public:
	int  GetAddressPath(std::string& path) const;
	int  SetAddressPath(const std::string& path);
};

class CSocket : virtual public ISocket
{
public:
	//-- Inherited for ISocket
	virtual int GetHandle() const;
	virtual int Close();

public:
	//-- Local memver functions
	CSocket(int af, int type);
	CSocket(int sock);

protected:
	int m_hsock;   // socket file descriptor
};

class CServerSocket : public CSocket, virtual public IServerSocket
{
public: //-- Inheritted from IServerSocket
	int Listen(int backlog);

public:
	CServerSocket(int af);
};

class CTcpSocket : public CSocket, virtual public ITcpSocket
{
public: //-- Inherited from ITcpSocket
	int Bind(const IInetSocketAddress& laddr);
	int Connect(const IInetSocketAddress& raddr);

public:
	CTcpSocket();
	CTcpSocket(int hsock);
};

class CTcpServerSocket : public CServerSocket, virtual public ITcpServerSocket
{
public:
	int Accept(IInetSocketAddress& raddr, TObjectHolder<ITcpSocket>& sock);
	int Bind(const IInetSocketAddress& laddr);

public:
	CTcpServerSocket();
};

class CUdpSocket : public CSocket, virtual public IUdpSocket
{
public:
	int Bind(const IInetSocketAddress& laddr);

public:
	CUdpSocket();
};

class CLocalSocket : public CSocket, virtual public ILocalSocket
{
public:
	int Bind(const ILocalSocketAddress& laddr);
	int Connect(const ILocalSocketAddress& raddr);
};

class CLocalServerSocket : public CServerSocket, virtual public ILocalServerSocket
{
public:
	int Accept(ILocalSocketAddress& raddr, TObjectHolder<ILocalSocket>& rsock);
	int Bind(const ILocalSocketAddress& laddr);
};

} // end of namespace sym

////////////////////////////////////////////
// Implementation of classes
//

namespace sym
{

/////////////////////////////////////////////
// Implementation of class CInetSocketAddress

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

////////////////////////////////////////////////
// implementation of class CSocket

CSocket::CSocket(int af, int type) : m_hsock(-1)
{
	m_hsock = ::socket(af, type, 0);
	return ;
}

CSocket::CSocket(int sock) : m_hsock(sock) { }

int CSocket::GetHandle() const 
{
	return m_hsock;
}

int CSocket::Close()
{
	if ( m_hsock >= 0) ::close(m_hsock);
	m_hsock = -1;
	return 0;
}

/////////////////////////////////////////
// Implementation of class CServerSocket

CServerSocket::CServerSocket(int af) : CSocket(af, SOCK_STREAM){ }

int CServerSocket::Listen(int backlog)
{
	new CSocket(4);
	return ::listen(m_hsock, backlog);
}

////////////////////////////////////////
// Implementation of class CTcpSocket

CTcpSocket::CTcpSocket() : CSocket(PF_INET, SOCK_STREAM) {}

CTcpSocket::CTcpSocket(int hsock) : CSocket(hsock) {}

int CTcpSocket::Bind(const IInetSocketAddress& laddr)
{
	struct sockaddr_in addr;
	addr.sin_addr.s_addr = ::htonl(laddr.GetHostAddress());
	addr.sin_port = ::htons(laddr.GetPort());

	return ::bind(m_hsock, (struct sockaddr *)&addr, (socklen_t)sizeof(addr));
}

int CTcpSocket::Connect(const IInetSocketAddress& raddr)
{
	struct sockaddr_in addr;
	addr.sin_addr.s_addr = ::htonl(raddr.GetHostAddress());
	addr.sin_port = ::htons(raddr.GetPort());

	return ::connect(m_hsock, (struct sockaddr*)&addr, (socklen_t)sizeof(addr)); 
}

//////////////////////////////////////////
// Implementation of class CTcpServerSocket

CTcpServerSocket::CTcpServerSocket() : CServerSocket(PF_INET) {}

int CTcpServerSocket::Accept(IInetSocketAddress& raddr, 
		                     TObjectHolder<ITcpSocket>& sockHolder)
{
	struct    sockaddr_in addr;
	socklen_t len = sizeof(addr);
	int s = ::accept(m_hsock, (struct sockaddr *)&addr, &len);
	if ( s >= 0) {
		ITcpSocket * psock = sockHolder.Alloc(sizeof(CTcpSocket));
		psock = (ITcpSocket *)new(psock) CTcpSocket(s);
		raddr.SetHostAddress(::ntohl(addr.sin_addr.s_addr));
		raddr.SetPort(::ntohs(addr.sin_port));
		return 0;
	}  else {
		return s;
	}
}

int CTcpServerSocket::Bind(const IInetSocketAddress& laddr)
{
	struct sockaddr_in addr;
	addr.sin_addr.s_addr = ::htonl(laddr.GetHostAddress());
	addr.sin_port = ::htons(laddr.GetPort());

	return ::bind(m_hsock, (struct sockaddr*)&addr, sizeof(addr));
}

/////////////////////////////////////////////
// Implementation of class CUdpSocket

CUdpSocket::CUdpSocket() : CSocket(PF_INET, SOCK_DGRAM){}

int CUdpSocket::Bind(const IInetSocketAddress& laddr) 
{
	struct sockaddr_in addr;
	addr.sin_addr.s_addr = ::htonl(laddr.GetHostAddress());
	addr.sin_port = ::htons(laddr.GetPort());

	return ::bind(m_hsock, (struct sockaddr*)&addr, sizeof(addr));
}

} // end of namespace sym

