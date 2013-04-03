#ifndef _SYM_INETCHANNEL_H
#define _SYM_INETCHANNEL_H

////////////////////////////////////////////
// Declaration of classes
//

namespace sym
{

namespace detail 
{
	class CTcpChannelImpl;
	class CTcpChannelListenerImpl;
	class CUdpChannelImpl;
}

class CTcpChannel
{
public:
	CTcpChannel(bool created = true);
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

	int  Send(const char * buf, size_t size, int flags = 0);
	int  Recv(char * buf, size_t size, int flags = 0);

private:
	CTcpChannel& operator=(const CTcpChannel& ch); // no impl

private:
	detail::CTcpChannelImpl * m_pImpl;

	friend class CTcpChannelListener;
}; // class CTcpChannel

class CTcpChannelListener
{
public:
	CTcpChannelListener();
	CTcpChannelListener(const CTcpChannelListener& ch);
	~CTcpChannelListener();

	int GetHandle() const;
	std::string GetLocalAddr() const;
	uint16_t    GetLocalPort() const;

	int Bind(const std::string& ipaddr, uint16_t port);
	int Listen(int backlog);
	int Accept(CTcpChannel& ch);
	int Close();

private:
	CTcpChannelListener& operator=(const CTcpChannelListener& lis); // no impl

private:
	detail::CTcpChannelListenerImpl * m_pImpl;	
}; // class CTcpChannelListener

class CUdpChannel
{
public:
	CUdpChannel();
	CUdpChannel(const CUdpChannel& ch);
	~CUdpChannel();

	int GetHandle() const;
	std::string GetRemoteAddr() const;
	uint16_t    GetRemotePort() const;
	std::string GetLocalAddr() const;
	uint16_t    GetLocalPort() const;
	std::string GetRecvAddr()  const;
	uint16_t    GetRecvPort()  const;
	
	int Bind(const std::string& ipaddr, uint16_t port);
	int Connect(const std::string& ipaddr, uint16_t port);
	int Close();

	int Send(const char *buf, size_t size, int flags = 0);
	int Recv(char *buf, size_t size, int flags = 0);

private:
	CUdpChannel& operator=(const CUdpChannel& ch); // no impl

private:
	detail::CUdpChannelImpl * m_pImpl;
};

} // end of namespace sym



#endif // _SYM_INETCHANNEL_H
