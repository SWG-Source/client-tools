// ======================================================================
//
// SwgCuiAvatarCustomize2.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiAvatarCustomize2.h"
#include "swgClientUserInterface/SwgCuiAvatarCustomizationBase_CustomizationGroup.h"

#include "clientGame/CreatureObject.h"
#include "clientUserInterface/CuiColorPicker.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiStringIds.h"
#include "clientUserInterface/CuiTransition.h"
#include "clientUserInterface/CuiWidget3dObjectListViewer.h"
#include "sharedGame/SharedImageDesignerManager.h"
#include "sharedMath/PaletteArgb.h"
#include "swgClientUserInterface/SwgCuiAvatarCreationHelper.h"
#include "swgClientUserInterface/SwgCuiMediatorTypes.h"

#include "UIButton.h"
#include "UIComposite.h"
#include "UIVolumePage.h"

// ======================================================================

namespace SwgCuiAvatarCustomize2Namespace
{	
	bool s_purgePoolOnDeactivate = false;
	bool s_purgeExtraPoolMembersOnDeactivate = false;

	void onMessageBoxClosedConfirmCancel (const CuiMessageBox & box)
	{
		if (box.completedAffirmative ())
		{
			s_purgePoolOnDeactivate = true;
			//-- go to avatar customization screen
			CuiTransition::startTransition (CuiMediatorTypes::AvatarCustomize, CuiMediatorTypes::AvatarCreation);
		}
	}
}

using namespace SwgCuiAvatarCustomize2Namespace;

// ----------------------------------------------------------------------

SwgCuiAvatarCustomize2::SwgCuiAvatarCustomize2  (UIPage & page)
: SwgCuiAvatarCustomizationBase("SwgCuiAvatarCustomize2", page),
  m_okButton(0),
  m_cancelButton(0),
  m_buttonRandom(0),
  m_buttonRandomAll(0)
{

	getCodeDataObject (TUIButton, m_okButton, "buttonNext");
	getCodeDataObject (TUIButton, m_cancelButton, "buttonPrev");
	getCodeDataObject (TUIButton, m_buttonRandom, "buttonRandom");
	getCodeDataObject (TUIButton, m_buttonRandomAll, "buttonRandomAll");

	registerMediatorObject (*m_okButton, true);
	registerMediatorObject (*m_cancelButton, true);
	registerMediatorObject (*m_buttonRandom, true);
	registerMediatorObject (*m_buttonRandomAll, true);
}

//-----------------------------------------------------------------------

SwgCuiAvatarCustomize2::~SwgCuiAvatarCustomize2()
{
	m_buttonRandom = 0;
	m_buttonRandomAll = 0;
}

//-----------------------------------------------------------------------

void SwgCuiAvatarCustomize2::performActivate()
{
	SwgCuiAvatarCustomizationBase::performActivate();

	setPointerInputActive(true);
	setKeyboardInputActive(true);
	setInputToggleActive(false);

	s_purgePoolOnDeactivate = false;
	s_purgeExtraPoolMembersOnDeactivate = false;

	ClientObject * const player = SwgCuiAvatarCreationHelper::getCreature ();
	setObject (player, 0);
	SwgCuiAvatarCreationHelper::purgeExtraPoolMembers();

	if (!SwgCuiAvatarCreationHelper::getCreatureCustomized())
	{
		randomize (true);
		OnVolumePageSelectionChanged (m_hairVolumePage);
		SwgCuiAvatarCreationHelper::setCreatureCustomized(true);
	}
}

//-----------------------------------------------------------------------

void SwgCuiAvatarCustomize2::performDeactivate ()
{
	if (s_purgePoolOnDeactivate)
	{
		SwgCuiAvatarCreationHelper::purgePool ();
		s_purgePoolOnDeactivate = false;
	}
	else if (s_purgeExtraPoolMembersOnDeactivate)
	{
		SwgCuiAvatarCreationHelper::purgeExtraPoolMembers ();
		s_purgeExtraPoolMembersOnDeactivate = false;
	}

	setPointerInputActive  (false);
	setKeyboardInputActive (false);
	setInputToggleActive   (true);

	SwgCuiAvatarCustomizationBase::performDeactivate();
}

//-----------------------------------------------------------------------

void SwgCuiAvatarCustomize2::OnButtonPressed(UIWidget *context)
{
	SwgCuiAvatarCustomizationBase::OnButtonPressed(context);

	if (context == m_okButton)
	{
		setObject (0, 0);

		CuiTransition::startTransition(CuiMediatorTypes::AvatarCustomize, CuiMediatorTypes::AvatarProfessionTemplateSelect);

		closeNextFrame();
	}

	else if (context == m_cancelButton)
	{
		CuiTransition::startTransition(CuiMediatorTypes::AvatarCustomize, CuiMediatorTypes::AvatarSimple);
		return;
	}

	else if (context == m_buttonRandom)
	{
		randomize (false);
	}

	else if (context == m_buttonRandomAll)
	{
		randomize (true);
	}

} //lint !e818


//----------------------------------------------------------------------

void SwgCuiAvatarCustomize2::updateSelectedHairColorState()
{
	SwgCuiAvatarCustomizationBase::updateSelectedHairColorState();
	setColorPickerColumnAndMaxIndexes();
}

//----------------------------------------------------------------------

void SwgCuiAvatarCustomize2::setGroup(std::string const & groupName)
{
	SwgCuiAvatarCustomizationBase::setGroup(groupName);
	setColorPickerColumnAndMaxIndexes();
}

//----------------------------------------------------------------------

/** Use data from SharedImageDesignerManager to determine and set the correct number of columns and maximum index to show for the current colorpickers
*/
void SwgCuiAvatarCustomize2::setColorPickerColumnAndMaxIndexes() const
{
	if(m_colorPicker[0])
	{
		PaletteArgb const * const palette = m_colorPicker[0]->getPalette();
		if(palette)
		{
			SharedImageDesignerManager::PaletteValues const paletteValues = SharedImageDesignerManager::getPaletteValuesForPaletteAndLevel(palette, SharedImageDesignerManager::IDPL_NOT_A_DESIGNER);
			m_colorPicker[0]->setForceColumns(paletteValues.columns);
			m_colorPicker[0]->setMaximumPaletteIndex(paletteValues.maxIndex);
			m_colorPicker[0]->reset();
		}
	}

	if(m_colorPicker[1])
	{
		PaletteArgb const * const palette = m_colorPicker[1]->getPalette();
		if(palette)
		{
			SharedImageDesignerManager::PaletteValues const paletteValues = SharedImageDesignerManager::getPaletteValuesForPaletteAndLevel(palette, SharedImageDesignerManager::IDPL_NOT_A_DESIGNER);
			m_colorPicker[1]->setForceColumns(paletteValues.columns);
			m_colorPicker[1]->setMaximumPaletteIndex(paletteValues.maxIndex);
			m_colorPicker[1]->reset();
		}
	}
}

// ======================================================================

