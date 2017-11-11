// StationAPISession.h: interface for the StationAPISession class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_StationAPISession_H__5C1759A1_A9D2_11D4_83B3_00C04F6C1CA6__INCLUDED_)
#define AFX_StationAPISession_H__5C1759A1_A9D2_11D4_83B3_00C04F6C1CA6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string.h>

/// Session class, converts to or from a string.
class StationAPISession
{
public:
	enum { SESSION_ID_LENGTH = 64 };

	/// default constructor
	StationAPISession();
    /// copy constructor
	StationAPISession( StationAPISession &other );
    /// constructor
	StationAPISession( const char *sessionID );

	~StationAPISession();

	bool IsValid() const;

	/// equality function
	bool Equals( const StationAPISession &other ) const;
    /// == operator
	bool operator == (const StationAPISession &) const;
    /// != operator
	bool operator != (const StationAPISession &) const;

	/// get session string
	const char *GetString() const;

	/// & operator
	StationAPISession &operator = (const StationAPISession &);
	
private:
	bool m_valid;
	char m_sessionID[SESSION_ID_LENGTH];
};

#endif // !defined(AFX_StationAPISession_H__5C1759A1_A9D2_11D4_83B3_00C04F6C1CA6__INCLUDED_)
