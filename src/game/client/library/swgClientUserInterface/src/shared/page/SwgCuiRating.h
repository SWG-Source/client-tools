//======================================================================
//
// SwgCuiRating.h
// copyright (c) 2009 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiRating_H
#define INCLUDED_SwgCuiRating_H

//======================================================================
#include "clientUserInterface/CuiMediator.h"

#include "UIEventCallback.h"

class UIButton;
class UIComboBox;
class UIPage;
class UITable;
class UIText;
class UITextbox;
class UIComposite;
//----------------------------------------------------------------------

class SwgCuiRating: public CuiMediator, public UIEventCallback
{
public:
	explicit SwgCuiRating(UIPage & page);
	~SwgCuiRating();

	void         OnButtonPressed(UIWidget *context );
	void		 OnCheckboxSet(UIWidget *context);
	void		 OnCheckboxUnset(UIWidget *context);

	void		 setWindowTitle(std::string title);
	void		 setWindowDescription(std::string desc);

protected:
	virtual void performActivate();
	virtual void performDeactivate();

private:
	//disabled
	SwgCuiRating(const SwgCuiRating & rhs);
	SwgCuiRating & operator= (const SwgCuiRating & rhs);

	void		 sendRating(int value);
	void		 setupRatingBoxes(int checkedIndex);

	UIText *	m_title;
	UIText *    m_description;

	UIButton *  m_okButton;
	UIButton *  m_cancelButton;
	UIButton *  m_close;


};

//======================================================================

#endif