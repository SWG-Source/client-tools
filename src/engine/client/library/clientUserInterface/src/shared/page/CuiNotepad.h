//======================================================================
//
// CuiNotepad.h
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiNotepad_H
#define INCLUDED_CuiNotepad_H

//======================================================================


//======================================================================

#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"

//-----------------------------------------------------------------

class UIButton;
class UIPage;
class UIText;

//-----------------------------------------------------------------

namespace MessageDispatch
{
	class Callback;
};

//-----------------------------------------------------------------

class CuiNotepad : public CuiMediator, public UIEventCallback
{
public:
	explicit                CuiNotepad        (UIPage & page);
	void                    performActivate   ();
	void                    performDeactivate ();
	void                    loadFromFile      ();
	void                    saveToFile        ();

	static void             setUserSpecifiedFileName(const std::string & fileName);

private:
	//disabled
	                        CuiNotepad (const CuiNotepad & rhs);
	                        CuiNotepad & operator= (const CuiNotepad & rhs);

private:
	UIText*                 m_noteText; 
	UIText*                 m_windowTitle;
	std::string             m_currentFilePath;
};

//======================================================================

#endif
