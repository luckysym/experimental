#ifndef _SYM_LOCALCHANNEL_H
#define _SYM_LOCALCHANNEL_H

///////////////////////////////////////////
// Declaration of classes

namespace sym
{

namespace detail
{
	class CLocalChannelImpl;
} // end of namespace detail


class CLocalChannel
{
public:
	CLocalChannel();
	CLocalChannel(const CLocalChannel& ch);
	~CLocalChannel();

	std::string GetLocalAddr() const;
	std::string GetRemoteAddr() const;
	std::string GetRecvAddr()  const;
	
	int Bind(const std::string& path);
	int Connect(const std::string& path);
	int Close();

	int Send(const char *buf, size_t size, int flags = 0);
	int Recv(char *buf, size_t size, int flags = 0); 

	static int CreatePipeChannel(CLocalChannel* channels[2]);

private:
	CLocalChannel& operator=(const CLocalChannel& ch); // no impl

private:
	detail::CLocalChannelImpl * m_pImpl;
};


} // end of namespace sym


#endif // _SYM_LOCALCHANNEL

