// ======================================================================
//
// IMEManager.h
// Portions copyright 2004 Sony Online Entertainment
// All Rights Reserved.
//
// ======================================================================


#ifndef INCLUDED_IMEManager_H
#define INCLUDED_IMEManager_H

#include "UIWidget.h"
#include "Unicode.h"

#include <vector>

typedef std::vector<Unicode::String> CandidateList_t;
typedef void (*IMEStatusFunctionPointer)(bool open);

class IMEManager
{
protected:
	// disabled
	IMEManager(void);
	IMEManager(const IMEManager &);
	IMEManager &operator =(const IMEManager &);

public:

	static void install(bool useIME);
	static void remove();

	static Unicode::String& GetCompString();
	static unsigned char * GetCompStringAttrs();

	static unsigned int GetCompCaratIndex();

	static CandidateList_t& GetCandidateList();

	static void SetEnabled(bool enabled);
	static bool GetEnabled();

	static void SetContextWidget(UIWidget *contextWidget);
	static UIWidget* GetContextWidget();

	// Get selected index of the listbox element.  This is 1 indexed
	static int GetSelectedCandidateIndex();

	// Get the number of the selected candidate.  This is 1 indexed
	static int GetSelectedCandidate();

	// Get the total number of candidates
	static int GetNumCandidates();

	// Get the max number of candidates that can be displayed per page
	static void SetMaxCandidatesPerPage(unsigned int maxCandsPerPage);

	// Send this character back to the windows message queue
	static void ResendCharacter(unsigned short ch);

	static void SetConsumeEnter(bool consume);
	static bool GetConsumeEnter();

    static bool GetCandidateListOpen();

	static void SetIMEOpenStatusNotifyFunction(IMEStatusFunctionPointer function);

	static bool IsIMEMessage(UINT uMsg);

	static void ClearCompositionString();
};

#endif /* INCLUDED_IMEManager_H */