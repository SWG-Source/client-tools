// ======================================================================
//
// GodClientPerforceUser.h
// copyright(c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_GodClientPerforceUser_H
#define INCLUDED_GodClientPerforceUser_H

// ======================================================================

#pragma warning (disable:4100) // unreferenced formal parameters abound in the perforce clientAPI
#include "ClientApi.h"
#pragma warning (default:4100)

//-----------------------------------------------------------------

class GodClientPerforceUser : public ClientUser, public MessageDispatch::Emitter
{
public:

	typedef std::vector<std::string> StringVector;

	GodClientPerforceUser();

	bool                errorOccurred      () const;
	int                 getLastError       () const;
	const std::string&  getLastErrorText   () const;
	void                clearLastError     ();
	void                addFilteredError   (int error);
	void                clearFilteredErrors();
	void                OutputInfo         (char level, char* data);
	const std::string&  getReturnValue     () const;

	bool                runCommand(const char* command,  const char* arg1);
	bool                runCommand(const char* command, const StringVector & args);
	
private:
	bool                m_errorOccurred;
	int                 m_lastError;
	std::string         m_lastErrorText;
	std::vector<int>    m_filteredErrors;
	std::string         m_return_value;

	virtual void        HandleError        (Error*err);
	virtual void        Prompt             (const StrPtr& msg, StrBuf& rsp, int noEcho, Error* e);

private:
	//disabled
	GodClientPerforceUser(const GodClientPerforceUser & rhs);
	GodClientPerforceUser& operator=(const GodClientPerforceUser & rhs);
};
//-----------------------------------------------------------------

inline bool GodClientPerforceUser::errorOccurred() const
{
	return m_errorOccurred;
}

//-----------------------------------------------------------------

inline int GodClientPerforceUser::getLastError() const
{
	return m_lastError;
}

//-----------------------------------------------------------------

inline const std::string & GodClientPerforceUser::getLastErrorText() const
{
	return m_lastErrorText;
}


//-----------------------------------------------------------------

inline void GodClientPerforceUser::clearLastError()
{
	m_errorOccurred = false;
	m_lastError = 0;
	m_lastErrorText.clear();
}

//-----------------------------------------------------------------

inline void GodClientPerforceUser::addFilteredError(int error)
{
	m_filteredErrors.push_back(error);
}

//-----------------------------------------------------------------

inline void GodClientPerforceUser::clearFilteredErrors()
{
	m_filteredErrors.clear();
}

//-----------------------------------------------------------------

inline const std::string & GodClientPerforceUser::getReturnValue() const
{
	return m_return_value;
}

// ======================================================================

#endif
