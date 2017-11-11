#include "FirstLagger.h"
#include "Packet.h"

// --------------------------------------------------------------------------

namespace
{
	const UINT cs_invalidPacket = 0xffffffff;
	UINT       s_currentPacketId = 0;

	LARGE_INTEGER s_timeBase;
	double        s_inverseClockFreq;
}

// --------------------------------------------------------------------------

void Packet::install()
{
	QueryPerformanceCounter( &s_timeBase );

	LARGE_INTEGER freq;
	QueryPerformanceFrequency( &freq );
	s_inverseClockFreq = 1.0 / (double)freq.QuadPart;
}

// --------------------------------------------------------------------------

Packet::Packet() :
m_data       ( 0 ),
m_dataSize   ( 0 ),
m_socket     ( INVALID_SOCKET ),
m_address    ( 0 ),
m_addressSize( 0 ),
m_packetId   ( cs_invalidPacket ),
m_sendFlags  ( 0 )
{
	QueryPerformanceCounter( &m_created );
}

// --------------------------------------------------------------------------

Packet::Packet(
	const void *    data,
	UINT            count,
	SOCKET          s,
	const sockaddr *addr,
	int             addrlen,
	int             flags) :
m_data       ( 0 ),
m_dataSize   ( count ),
m_socket     ( s ),
m_address    ( 0 ),
m_addressSize( addrlen ),
m_packetId   ( s_currentPacketId++ ), // <- not thread safe
m_sendFlags  ( flags )
{
	m_data = new BYTE[ count ];
	memcpy( m_data, data, count );

	m_address = (sockaddr *)new BYTE[ m_addressSize ];
	memcpy( m_address, addr, m_addressSize );

	QueryPerformanceCounter( &m_created );
	m_created.QuadPart -= s_timeBase.QuadPart;
}

// --------------------------------------------------------------------------

Packet::Packet( const Packet &arg ) :
m_data       ( 0 ),
m_dataSize   ( arg.m_dataSize ),
m_socket     ( arg.m_socket ),
m_address    ( 0 ),
m_addressSize( arg.m_addressSize ),
m_packetId   ( arg.m_packetId ),
m_created    ( arg.m_created ),
m_sendFlags  ( arg.m_sendFlags )
{
	m_data = new BYTE[ m_dataSize ];
	memcpy( m_data, arg.getData(), m_dataSize );

	m_address = (sockaddr *)new BYTE[ m_addressSize ];
	memcpy( m_address, arg.getAddress(), m_addressSize );
}

// --------------------------------------------------------------------------

Packet &Packet::operator=( const Packet &rhs )
{
	// make sure this packet hasn't already been initialized
	assert( m_data == 0 );

	m_dataSize = rhs.getDataSize();
	m_data = new BYTE[ m_dataSize ];
	memcpy( m_data, rhs.getData(), m_dataSize );


	m_addressSize = rhs.getAddressSize();
	m_address = (sockaddr *)new BYTE[ m_addressSize ];
	memcpy( m_address, rhs.getAddress(), m_addressSize );

	m_packetId  = rhs.getPacketId();
	m_created   = rhs.m_created;
	m_sendFlags = rhs.getSendFlags();
	m_socket    = rhs.getSocket();

	return *this;
}

// --------------------------------------------------------------------------

Packet::~Packet()
{
	delete [] m_data;
	delete [] (BYTE*)m_address;
}

// --------------------------------------------------------------------------

double Packet::getAge() const
{
	LARGE_INTEGER now;
	QueryPerformanceCounter( &now );

	return (double)( now.QuadPart - s_timeBase.QuadPart - m_created.QuadPart ) * s_inverseClockFreq;
}

// ==========================================================================
