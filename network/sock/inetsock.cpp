#include <stdint.h>
#include <string>
#include "sockintf.h"

////////////////////////////////////////////
// Declaration of classes
//

namespace sym
{

class CInetSocketAddress : public IInetSocketAddress
{
public:
	uint32_t GetIPAddress() const;
	int      GetIPAddressStr(std::string& ipaddr) const;
	uint16_t GetPort() const;
	int      SetIPAddress(uint32_t addr);
	int      SetIPAddressStr(const std::string& ipaddr);
	int      SetPort(uint16_t port);
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


