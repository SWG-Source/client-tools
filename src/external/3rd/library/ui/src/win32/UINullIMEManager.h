//======================================================================
//
// UINullIMEManager.h
// copyright (c) 2004 Sony Online Entertainment
//
//====================================================================== 

#ifndef UIINULLMEMANAGER_H
#define UIINULLMEMANAGER_H

//This class is a null implementation of the interface for the IME Manager

class UIPage;

#include "Unicode.h"
#include "UIIMEManager.h"
#include "UITypes.h"

class UINullIMEManager : public UIIMEManager
{	

	typedef ui_stdvector<Unicode::String>::fwd CandidateList_t;

protected:
	// disabled
		UINullIMEManager(const UIIMEManager &);
	UINullIMEManager &operator =(const UIIMEManager &);

	CandidateList_t m_emptyCandidateList;
	bool m_enabled;
	bool m_consume;
	Unicode::String m_emptyString;

public:

	virtual ~UINullIMEManager() {}

	UINullIMEManager(void) : m_enabled(false), m_consume(false) {}
	
	virtual Unicode::String& GetCompString() { return m_emptyString; }
	virtual unsigned char * GetCompStringAttrs() { return NULL; }

	virtual unsigned int GetCompCaratIndex() { return 0; }

	virtual CandidateList_t& GetCandidateList() { return m_emptyCandidateList; }

	virtual void SetEnabled(bool enabled) { m_enabled = enabled; }
	virtual bool GetEnabled() { return m_enabled; }
	virtual void SetContextWidget(UIWidget *contextWidget)  { UNREF(contextWidget); }
	virtual UIWidget* GetContextWidget()  { return NULL; }

	// Get selected index of the listbox element.  This is 1 indexed
	virtual int GetSelectedCandidateIndex() { return 1; }

	// Get the number of the selected candidate.  This is 1 indexed
	virtual int GetSelectedCandidate() { return 1; }

	// Get the total number of candidates
	virtual int GetNumCandidates() { return 0; }

	// Get the max number of candidates that can be displayed per page
	virtual void SetMaxCandidatesPerPage(unsigned int maxCandsPerPage) { UNREF(maxCandsPerPage); }

	// Send this character back to the windows message queue
	virtual void ResendCharacter(unsigned short ch) { UNREF(ch); }

	virtual void SetConsumeEnter(bool consume) { m_consume = consume; }
	virtual bool GetConsumeEnter() { return m_consume; }

    virtual bool GetCandidateListOpen() { return false; }
};
 

#endif