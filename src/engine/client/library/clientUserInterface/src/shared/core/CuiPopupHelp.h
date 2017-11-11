//======================================================================
//
// CuiPopupHelp.h
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiPopupHelp_H
#define INCLUDED_CuiPopupHelp_H

#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"

class UIText;
class UIButton;
class CuiPopupHelpData;
class UIWidget;

//======================================================================

class CuiPopupHelp :
public CuiMediator,
public UIEventCallback
{
public:

	explicit          CuiPopupHelp  (UIPage & page);

	bool              setData       (CuiMediator & parentMediator, const CuiPopupHelpData & data);

	void              autoPosition  ();

	void              OnButtonPressed (UIWidget * context);

	void              forceNext     ();

	void              setForceClose (bool b);

	void              update        (float deltaTimeSecs);

protected:

	void performActivate   ();
	void performDeactivate ();

	bool close             ();

	std::pair<bool, int>  autoPosition      (int desiredPosition, bool force);

private:

	~CuiPopupHelp ();

	CuiPopupHelp (const CuiPopupHelp & rhs);
	CuiPopupHelp & operator= (const CuiPopupHelp & rhs);

private:

	UIButton *         m_buttonNext;

	UIWidget *         m_ptrSW;
	UIWidget *         m_ptrNW;
	UIWidget *         m_ptrNE;
	UIWidget *         m_ptrSE;

	bool               m_okNext;

	UIWidget *         m_target;

	int                m_desiredPosition;

	CuiMediator *      m_parentMediator;

	UIText *           m_text;

	Unicode::String    m_scriptPost;

	bool               m_forceClose;
};

//======================================================================

#endif
