#ifndef _SYM_SOCKINTF_H
#define _SYM_SOCKINTF_H

namespace sym
{

class TDefaultAllocator
{
public:
	template<class T>
	T*   Alloc() { return new T();}

	template<class T>
	void Free(T* p) { delete p;}
};

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

	POBJTYPE Alloc(size_t size) 
	{
		if ( m_pobj ) Free();	
		m_pobj =  (POBJTYPE)::operator new(size); 
		return m_pobj;
	}
	void  Free() { delete m_pobj; m_pobj = 0; }

private:
	TObjectHolder(const TObjectHolder<T>&);
	TObjectHolder<T>& operator=(const TObjectHolder<T>&);
};

class IInetSocketAddress 
{
public:
	virtual uint32_t GetHostAddress() const         = 0;
	virtual int      GetHostAddressStr(std::string& ipaddr) const = 0;
  	virtual uint16_t GetPort() const              = 0;
	virtual int      SetHostAddress(uint32_t addr);
	virtual int      SetHostAddressStr(const std::string& ipaddr) = 0;
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

class IServerSocket : virtual public ISocket
{
public:
	virtual int Listen(int backlog) = 0;
};

class ITcpSocket : virtual public ISocket
{
public:
	virtual int Bind(const IInetSocketAddress& laddr) = 0;
	virtual int Connect(const IInetSocketAddress& raddr) = 0;
};

class ITcpServerSocket : virtual public IServerSocket
{
public:
	virtual int Accept(IInetSocketAddress& raddr, 
			           TObjectHolder<ITcpSocket>& rsock) = 0;
	virtual int Bind(const IInetSocketAddress& laddr) = 0;
};

class IUdpSocket : virtual public ISocket
{
public:
	virtual int Bind(const IInetSocketAddress& laddr) = 0;
	virtual int Connect(const IInetSocketAddress& raddr) = 0;
};

class ILocalSocket : virtual public ISocket
{
public:	
	virtual int Bind(const ILocalSocketAddress& laddr) = 0;
	virtual int Connect(const ILocalSocketAddress& raddr) = 0;
};

class ILocalServerSocket : virtual public IServerSocket
{
public:
	virtual int Accept(ILocalSocketAddress& raddr, 
			           TObjectHolder<ILocalSocket>& rsock) = 0;
	virtual int Bind(const ILocalSocketAddress& laddr) = 0;
};

} // end of namespace sym


#endif // _SYM_SOCKINTF_H

