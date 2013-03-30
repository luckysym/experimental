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

template<class T>
class TObjectHolder
{
public:
	typedef T OBJTYPE, *POBJTYPE;

private:
	POBJTYPE m_pobj;

public:	
	TObjectHolder() : m_pobj(0) {}
	TObjectHolder(POBJTYPE pobj) : m_pobj(pobj) {}
	~TObjectHolder() {
		if ( m_pobj ) Free(m_pobj);
		return ;
	}

	POBJTYPE Get() const { return m_pobj; }
	
	void Put(POBJTYPE pobj)
	{ 
		if ( m_pobj ) Free(pobj);
		m_pobj = pobj; 
	}

	POBJTYPE Pop() 
	{
		POBJTYPE p = m_pobj;
		m_pobj = 0;
		return p;
	}

	virtual POBJTYPE Alloc(size_t size) 
	{
		if ( m_pobj ) Free();	
		m_pobj =  (POBJTYPE)::operator new(size); 
		return m_pobj;
	}
	virtual void  Free() { delete m_pobj; m_pobj = 0; }

private:
	TObjectHolder(const TObjectHolder<T>&);
	TObjectHolder<T>& operator=(const TObjectHolder<T>&);
};

class CInetSocketAddress 
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

class CLocalSocketAddress
{
public:
	int  GetAddressPath(std::string& path) const;
	int  SetAddressPath(const std::string& path);
};

class CSocket 
{
public:
	virtual int GetHandle() const;
	virtual int Close();

protected:
	CSocket(int af, int type);
	CSocket(int sock);

protected:
	int m_hsock;   // socket file descriptor
};

class CServerSocket : public CSocket
{
public: 
	int Listen(int backlog);

protected:
	CServerSocket(int af);
};

class CTcpSocket : public CSocket
{
public:
	int Bind(const std::string& ipaddr, uint16_t port);
	int Connect(const std::string& ipaddr, uint16_t port);

public:
	CTcpSocket();
	CTcpSocket(int hsock);
};

class CTcpServerSocket : public CServerSocket
{
public:
	int Accept(TObjectHolder<CTcpSocket>& sock);
	int Bind(const CInetSocketAddress& laddr);

public:
	CTcpServerSocket();
};

class CUdpSocket : public CSocket
{
public:
	int Bind(const CInetSocketAddress& laddr);

public:
	CUdpSocket();
};

class CLocalSocket : public CSocket
{
public:
	int Bind(const CLocalSocketAddress& laddr);
	int Connect(const CLocalSocketAddress& raddr);
};

class CLocalServerSocket : public CServerSocket
{
public:
	int Accept(CLocalSocketAddress& raddr, TObjectHolder<CLocalSocket>& rsock);
	int Bind(const CLocalSocketAddress& laddr);
};



class CTcpClientChannel
{
public:
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
	return ::listen(m_hsock, backlog);
}

////////////////////////////////////////
// Implementation of class CTcpSocket

CTcpSocket::CTcpSocket() : CSocket(PF_INET, SOCK_STREAM) {}

CTcpSocket::CTcpSocket(int hsock) : CSocket(hsock) {}

int CTcpSocket::Bind()
{
	struct sockaddr_in addr;
	addr.sin_addr.s_addr = ::htonl(laddr.GetHostAddress());
	addr.sin_port = ::htons(laddr.GetPort());

	return ::bind(m_hsock, (const struct sockaddr *)&addr, (socklen_t)sizeof(addr));
}

int CTcpSocket::Connect(const CInetSocketAddress& raddr)
{
	return ::connect(m_hsock, (const struct sockaddr *)&raddr.m_addr, (socklen_t)sizeof(raddr.m_addr)); 
}

//////////////////////////////////////////
// Implementation of class CTcpServerSocket

CTcpServerSocket::CTcpServerSocket() : CServerSocket(PF_INET) {}

int CTcpServerSocket::Accept(CInetSocketAddress& raddr, 
		                     TObjectHolder<CTcpSocket>& sockHolder)
{
	struct    sockaddr_in addr;
	socklen_t len = sizeof(addr);
	int s = ::accept(m_hsock, (struct sockaddr *)&addr, &len);
	if ( s >= 0) {
		CTcpSocket * psock = sockHolder.Alloc(sizeof(CTcpSocket));
		psock = (CTcpSocket *)new(psock) CTcpSocket(s);
		raddr.SetHostAddress(::ntohl(addr.sin_addr.s_addr));
		raddr.SetPort(::ntohs(addr.sin_port));
		return 0;
	}  else {
		return s;
	}
}

int CTcpServerSocket::Bind(const CInetSocketAddress& laddr)
{
	struct sockaddr_in addr;
	addr.sin_addr.s_addr = ::htonl(laddr.GetHostAddress());
	addr.sin_port = ::htons(laddr.GetPort());

	return ::bind(m_hsock, (struct sockaddr*)&addr, sizeof(addr));
}

/////////////////////////////////////////////
// Implementation of class CUdpSocket

CUdpSocket::CUdpSocket() : CSocket(PF_INET, SOCK_DGRAM){}

int CUdpSocket::Bind(const CInetSocketAddress& laddr) 
{
	struct sockaddr_in addr;
	addr.sin_addr.s_addr = ::htonl(laddr.GetHostAddress());
	addr.sin_port = ::htons(laddr.GetPort());

	return ::bind(m_hsock, (struct sockaddr*)&addr, sizeof(addr));
}

} // end of namespace sym

