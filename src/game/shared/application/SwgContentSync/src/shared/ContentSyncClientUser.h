// ======================================================================
//
// ContentSyncClientUser.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ContentSyncClientUser_H
#define INCLUDED_ContentSyncClientUser_H

// ======================================================================

class ApplicationWindow;

#define strcasecmp _stricmp
#include <clientapi.h>
#include <qstring.h>
#include <vector>

// ======================================================================

class ContentSyncClientUser : public ClientUser
{
public:
	typedef std::vector<QString> ErrorList;

	ContentSyncClientUser(ApplicationWindow &applicationWindow);

	virtual void 	OutputError( const_char *errBuf );
	virtual void	OutputInfo( char level, const_char *data );
	virtual void 	OutputText( const_char *data, int length );

	ErrorList::const_iterator errorsBegin() const;
	ErrorList::const_iterator errorsEnd() const;

private:
	ContentSyncClientUser();
	ContentSyncClientUser(const ContentSyncClientUser &);
	ContentSyncClientUser &operator =(const ContentSyncClientUser &);

private:
	ApplicationWindow    &m_applicationWindow;
	bool                  m_error;
	std::vector<QString>  m_errors;
};

// ======================================================================

#endif
