// ======================================================================
//
// TemplateEditorPerforceUser.h
// Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_TemplateEditorPerforceUser_H
#define INCLUDED_TemplateEditorPerforceUser_H

// ======================================================================

#include "ClientApi.h"

//-----------------------------------------------------------------

class TemplateEditorPerforceUser : public ClientUser, public MessageDispatch::Emitter
{
public:

	typedef std::vector<std::string> StringVector;

	TemplateEditorPerforceUser();

	virtual void        HandleError        (Error*err);
	bool                errorOccurred      () const;
	int                 getLastError       () const;
	void                clearLastError     ();
	void                addFilteredError   (int error);
	void                clearFilteredErrors();
	void                OutputInfo         (char level, char* data);
	const std::string & getReturnValue     () const;

	bool                runCommand(const char* command,  const char* arg1);
	bool                runCommand(const char* command, const StringVector & args);
	
private:
	bool                m_errorOccurred;
	int                 m_lastError;
	std::vector<int>    m_filteredErrors;
	std::string         m_return_value;

private:

	// Disabled

	TemplateEditorPerforceUser(const TemplateEditorPerforceUser & rhs);
	TemplateEditorPerforceUser& operator=(const TemplateEditorPerforceUser & rhs);
};
//-----------------------------------------------------------------

inline bool TemplateEditorPerforceUser::errorOccurred() const
{
	return m_errorOccurred;
}

//-----------------------------------------------------------------

inline int TemplateEditorPerforceUser::getLastError() const
{
	return m_lastError;
}

//-----------------------------------------------------------------

inline void TemplateEditorPerforceUser::clearLastError()
{
	m_errorOccurred = false;
	m_lastError = 0;
}

//-----------------------------------------------------------------

inline void TemplateEditorPerforceUser::addFilteredError(int error)
{
	m_filteredErrors.push_back(error);
}

//-----------------------------------------------------------------

inline void TemplateEditorPerforceUser::clearFilteredErrors()
{
	m_filteredErrors.clear();
}

//-----------------------------------------------------------------

inline const std::string & TemplateEditorPerforceUser::getReturnValue() const
{
	return m_return_value;
}

// ======================================================================

#endif // INCLUDED_TemplateEditorPerforceUser_H
