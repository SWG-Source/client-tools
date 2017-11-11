//======================================================================
//
// CuiIMEManager.h
// copyright (c) 2004 Sony Online Entertainment
//
//====================================================================== 

#ifndef INCLUDED_CuiIMEManager_H
#define INCLUDED_CuiIMEManager_H

#include "clientUserInterface/FirstClientUserInterface.h"

#include "clientUserInterface/CuiMediatorTypes.h"
#include "UIIMEManager.h"
#include "Unicode.h"

#include <vector>

class CuiIMEManager : public UIIMEManager
{	
public:

	static void install();

	static CuiIMEManager *GetCuiIMEManager();

public:

	virtual ~CuiIMEManager();

	virtual Unicode::String& GetCompString();
	virtual unsigned char * GetCompStringAttrs();

	virtual unsigned int GetCompCaratIndex();

	virtual CandidateList_t& GetCandidateList();

	virtual void SetEnabled(bool enabled);
	virtual void SetContextWidget(UIWidget *contextWidget);
	virtual UIWidget* GetContextWidget();
	
	virtual bool GetEnabled();

	// Get selected index of the listbox element.  This is 1 indexed
	virtual int GetSelectedCandidateIndex();

	// Get the number of the selected candidate.  This is 1 indexed
	virtual int GetSelectedCandidate();

	// Get the total number of candidates
	virtual int GetNumCandidates();

	// Get the max number of candidates that can be displayed per page
	virtual void SetMaxCandidatesPerPage(unsigned int maxCandsPerPage);

	// Send this character back to the windows message queue
	virtual void ResendCharacter(unsigned short ch);

	virtual void SetConsumeEnter(bool consume);
	virtual bool GetConsumeEnter();

    virtual bool GetCandidateListOpen();
	
	virtual void SetIMEIndicator(CuiMediatorTypes::Type type);

	void NotifyIMEOpenStatus(bool open);

protected:

	// disabled
	CuiIMEManager();
	CuiIMEManager(const CuiIMEManager &);
	CuiIMEManager &operator =(const CuiIMEManager &);
	
	CuiMediatorTypes::Type               mIMEIndicatorType;
};

#endif
