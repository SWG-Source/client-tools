//======================================================================
//
// UIIMEManager.h
// copyright (c) 2004 Sony Online Entertainment
//
//====================================================================== 

#ifndef UIIMEMANAGER_H
#define UIIMEMANAGER_H

//This class is an interface for the IME Manager

class UIPage;
class UIWidget;

#include "Unicode.h"
#include "UITypes.h"


namespace UIIMEManagerNamespace
{
	const Unicode::String colorModifiers[6] = 
	{
		L"\\#DDDD00",	//ATTR_INPUT 	The character being entered by a user, not yet converted by the IME.
		L"\\#00DD00",	//ATTR_INPUT_ERROR 	The character is the error character and cannot be converted by the IME.
		L"\\#DDDDDD",	//ATTR_TARGET_CONVERTED 	The character converted by the IME. A user has selected this character, and the IME has converted it.
		L"\\#0000DD",	//ATTR_CONVERTED 	A converted character. The IME has already converted this character.
		L"\\#DD0000",	//ATTR_TARGET_NOTCONVERTED 	The character being converted. A user has selected this character, but the IME has not yet converted it.
		L"\\#DDDDDD",	//ATTR_FIXEDCONVERTED
	};
}

class UIIMEManager
{	
public:
	typedef ui_stdvector<Unicode::String>::fwd CandidateList_t;

protected:
	// disabled
	UIIMEManager(void) {}
	UIIMEManager(const UIIMEManager &);
	UIIMEManager &operator =(const UIIMEManager &);


public:
	
	virtual ~UIIMEManager() {}

	virtual Unicode::String& GetCompString() = 0;
	virtual unsigned char * GetCompStringAttrs() = 0;

	virtual unsigned int GetCompCaratIndex() = 0;

	virtual CandidateList_t& GetCandidateList() = 0;

	virtual void SetEnabled(bool enabled) = 0;
	virtual bool GetEnabled() = 0;
	virtual void SetContextWidget(UIWidget *contextWidget) = 0;
	virtual UIWidget* GetContextWidget() = 0;

	// Get selected index of the listbox element.  This is 1 indexed
	virtual int GetSelectedCandidateIndex() = 0;

	// Get the number of the selected candidate.  This is 1 indexed
	virtual int GetSelectedCandidate() = 0;

	// Get the total number of candidates
	virtual int GetNumCandidates() = 0;

	// Get the max number of candidates that can be displayed per page
	virtual void SetMaxCandidatesPerPage(unsigned int maxCandsPerPage) = 0;

	// Send this character back to the windows message queue
	virtual void ResendCharacter(unsigned short ch) = 0;

	virtual void SetConsumeEnter(bool consume) = 0;
	virtual bool GetConsumeEnter() = 0;

    virtual bool GetCandidateListOpen() = 0;
};
 
#endif