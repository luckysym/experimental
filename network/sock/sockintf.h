#ifndef _SYM_SOCKINTF_H
#define _SYM_SOCKINTF_H

namespace sym
{

class ISocketAddress
{
public:
	virtual int GetType() = 0;
};

class IInetSocketAddress : public ISocketAddress
{
public:
};

class ILocalSocketAddress : public ISocketAddress
{

};

class ISocket
{
public:
	virtual int GetHandle() const = 0;
	virtual int Bind(const ISocketAddress& addr) = 0;
	virtual int Connect(const ISocketAddress& raddr) = 0;
	virtual int Close() = 0;
};

class IServerSocket
{
public:
	virtual int GetHandle() const = 0;
	virtual int Bind(const ISocketAddress& addr) = 0;
	virtual int Listen(int backlog) = 0;
	virtual int Accept() = 0;
	virtual int Close() = 0;
};


} // end of namespace sym


#endif // _SYM_SOCKINTF_H

