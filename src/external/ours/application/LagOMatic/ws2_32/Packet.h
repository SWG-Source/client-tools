#ifndef INCLUDED__Packet_H
#define INCLUDED__Packet_H

#include <queue>

class Packet
{
public:
	Packet();

	Packet(
		const void *    data,
		UINT            count,
		SOCKET          s,
		const sockaddr *addr,
		int             addrlen,
		int             flags = 0 );

	Packet( const Packet &arg );

	Packet &operator=( const Packet &arg );

	~Packet();

	const BYTE *        getData() const;
	UINT                getDataSize() const;
	SOCKET              getSocket() const;
	const sockaddr *    getAddress() const;
	UINT                getAddressSize() const;
	UINT                getPacketId() const;
	double              getAge() const;
	int                 getSendFlags() const;

private:

	BYTE *              m_data;
	UINT                m_dataSize;
	SOCKET              m_socket;
	sockaddr *          m_address;
	UINT                m_addressSize;
	UINT                m_packetId;
	LARGE_INTEGER       m_created;
	int                 m_sendFlags;

public:
	typedef std::queue< Packet > Queue;

	static void         install();

};

inline const BYTE *Packet::getData() const
{
	return m_data;
}

inline UINT Packet::getDataSize() const
{
	return m_dataSize;
}

inline SOCKET Packet::getSocket() const
{
	return m_socket;
}

inline const sockaddr * Packet::getAddress() const
{
	return m_address;
}

inline UINT Packet::getAddressSize() const
{
	return m_addressSize;
}

inline UINT Packet::getPacketId() const
{
	return m_packetId;
}

inline int Packet::getSendFlags() const
{
	return m_sendFlags;
}

#endif //INCLUDED__Packet_H