// ======================================================================
//
// CuiDeleteSkillConfirmation.h
// Copyright Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CuiDeleteSkillConfirmation_H
#define INCLUDED_CuiDeleteSkillConfirmation_H

#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"

class UIButton;
class UIText;
class UITextbox;

//-----------------------------------------------------------------
class CuiDeleteSkillConfirmation : public CuiMediator
                                 , public UIEventCallback
{
public:

	struct Message
	{
		struct DeleteSkillConfirmation
		{
			typedef std::string SkillName;
		};
	};

	CuiDeleteSkillConfirmation(UIPage &page);

	virtual void OnButtonPressed(UIWidget *context);

	void         setSelectedSkill(std::string const &selectedSkill);

protected:

	~CuiDeleteSkillConfirmation();

	virtual void performActivate();
	virtual void performDeactivate();

private:
	
	UIButton *  m_okButton;
	UIButton *  m_cancelButton;
	UITextbox * m_skillTextBox;
	UIText *    m_instructionsText;
	std::string m_selectedSkill;

	void            validateText();

	// Disabled

	CuiDeleteSkillConfirmation(CuiDeleteSkillConfirmation const &rhs);
	CuiDeleteSkillConfirmation &operator =(CuiDeleteSkillConfirmation const &rhs);
};

// ======================================================================

#endif // INCLUDED_CuiDeleteSkillConfirmation_H
