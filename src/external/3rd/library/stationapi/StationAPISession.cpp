// StationAPISession.cpp: implementation of the StationAPISession class.
//
//////////////////////////////////////////////////////////////////////

#include "StationAPISession.h"

/* constructors */
StationAPISession::StationAPISession()
{
	m_valid = false;
//	strcpy( m_sessionID, "" );
	memset(m_sessionID,0,SESSION_ID_LENGTH);
}
StationAPISession::StationAPISession( StationAPISession &rhs )
{
	m_valid = rhs.IsValid();
	if (m_valid)
	{
		strcpy( m_sessionID, rhs.GetString() );
	}
}
StationAPISession::StationAPISession( const char *sessionID )
{
	if( sessionID == NULL )
	{
		m_valid = false;
		strcpy( m_sessionID, "" );
		return;
	}

	m_valid = true;
	strcpy( m_sessionID, sessionID );
}

/* destructor */
StationAPISession::~StationAPISession()
{
}

bool StationAPISession::IsValid() const
{
	return( m_valid );
}

/* caller should not modify */
const char *StationAPISession::GetString() const
{
	if( !m_valid )
		return "";
	return( m_sessionID );
}

/* assignment */
StationAPISession &StationAPISession::operator =(const StationAPISession &rhs)
{
	if( this == &rhs )
		return *this;

	if( rhs.IsValid() )
	{
		strcpy( m_sessionID, rhs.GetString() );
		m_valid = true;
	}
	else
	{
		m_valid = false;
	}

	return *this;
}

/* comparison */
bool StationAPISession::Equals( const StationAPISession &rhs ) const
{
	if( !IsValid() || !rhs.IsValid() )
		return false;

	return ( strcmp( GetString(), rhs.GetString() ) == 0 );
}

bool StationAPISession::operator == ( const StationAPISession &rhs ) const
{
	return Equals( rhs );
}

bool StationAPISession::operator != ( const StationAPISession &rhs ) const
{
	return !( *this == rhs );
}
