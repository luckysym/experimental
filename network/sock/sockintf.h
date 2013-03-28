#ifndef _SYM_SOCKINTF_H
#define _SYM_SOCKINTF_H

namespace sym
{

class IInetSocketAddress 
{
public:
	virtual uint32_t GetIPAddress() const         = 0;
	virtual int      GetIPAddressStr(std::string& ipaddr) const = 0;
  	virtual uint16_t GetPort() const              = 0;
	virtual int      SetIPAddress(uint32_t addr);
	virtual int      SetIPAddressStr(const std::string& ipaddr) = 0;
	virtual int      SetPort(uint16_t port) = 0;
};

class ILocalSocketAddress 
{
public:
	virtual int  GetAddressPath(std::string& path) const = 0;
	virtual int  SetAddressPath(const std::string& path) = 0;
};

class ISocket
{
public:
	virtual int GetHandle() const = 0;
	virtual int Close() = 0;
};

class IServerSocket : public ISocket
{
public:
	virtual int Listen(int backlog) = 0;
};

class ITcpSocket : public ISocket
{
public:
	virtual int Bind(const IInetSocketAddress& laddr) = 0;
	virtual int Connect(const IInetSocketAddress& raddr) = 0;
};

class ITcpServerSocket : public IServerSocket
{
public:
	virtual int Accept(ITcpSocket& rsock) = 0;
	virtual int Bind(const IInetSocketAddress& laddr) = 0;
};

class IUdpSocket : public ISocket
{
public:
	virtual int Bind(const IInetSocketAddress& laddr) = 0;
	virtual int Connect(const IInetSocketAddress& raddr) = 0;
};

class ILocalSocket : public ISocket
{
public:	
	virtual int Bind(const ILocalSocketAddress& laddr) = 0;
	virtual int Connect(const ILocalSocketAddress& raddr) = 0;
};

class ILocalServerSocket : public IServerSocket
{
public:
	virtual int Accept(ILocalSocket& rsock) = 0;
	virtual int Bind(const ILocalSocketAddress& laddr) = 0;
};

} // end of namespace sym


#endif // _SYM_SOCKINTF_H

