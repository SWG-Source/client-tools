// ======================================================================
//
// SwgCuiSceneSelection.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiSceneSelection.h"

#include "UIButton.h"
#include "UIData.h"
#include "UIImage.h"
#include "UIListbox.h"
#include "UIPage.h"
#include "UIText.h"
#include "UITextbox.h"
#include "UnicodeUtils.h"
#include "clientGame/ConfigClientGame.h"
#include "clientGame/ConnectionManager.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/GameNetwork.h"
#include "clientGame/GroundScene.h"
#include "clientGraphics/Graphics.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiStringIds.h"
#include "clientUserInterface/CuiTransition.h"
#include "sharedFile/TreeFile.h"
#include "swgClientUserInterface/SwgCuiMediatorTypes.h"

// ======================================================================

SwgCuiSceneSelection::SwgCuiSceneSelection (UIPage & page) :
CuiMediator         ("SwgCuiSceneSelection", page),
UIEventCallback     (),
m_okButton          (0),
m_cancelButton      (0),
m_descriptionText   (0),
m_placeholderImage  (0),
m_selectionListbox  (0),
m_textbox           (0),
m_buttonCreate      (0)
{
	getCodeDataObject (TUIButton,  m_okButton,         "OkButton");
	getCodeDataObject (TUIButton,  m_cancelButton,     "CancelButton");
	getCodeDataObject (TUIButton,  m_buttonCreate,     "buttonCreate");

	getCodeDataObject (TUIText,    m_descriptionText,  "DescriptionText");
	getCodeDataObject (TUIImage,   m_placeholderImage, "PlaceholderImage");

	getCodeDataObject (TUIListbox, m_selectionListbox, "SelectionListbox");
	getCodeDataObject (TUITextbox, m_textbox,          "textbox");

}

//-----------------------------------------------------------------

SwgCuiSceneSelection::~SwgCuiSceneSelection ()
{
	m_okButton = 0;
	m_cancelButton = 0;
	m_descriptionText = 0;
	m_placeholderImage = 0;
	m_selectionListbox = 0;

	m_buttonCreate = 0;
	m_textbox      = 0;
}

//-----------------------------------------------------------------

void SwgCuiSceneSelection::performActivate ()
{
	setPointerInputActive   (true);
	setKeyboardInputActive  (true);
	setInputToggleActive    (false);

	if (m_selectionListbox->GetSelectionIndex () < 0)
		m_selectionListbox->SetSelectionIndex (0);
	updateSceneSelection ();

	m_okButton->AddCallback (this);
	m_cancelButton->AddCallback (this);
	m_selectionListbox->AddCallback (this);
	m_buttonCreate->AddCallback (this);

	CuiTransition::signalTransitionReady (CuiMediatorTypes::SceneSelection);

}

//-----------------------------------------------------------------

void SwgCuiSceneSelection::performDeactivate ()
{
	setPointerInputActive   (false);
	setKeyboardInputActive  (false);
	setInputToggleActive    (true);

	m_okButton->RemoveCallback (this);
	m_cancelButton->RemoveCallback (this);
	m_selectionListbox->RemoveCallback (this);
	m_buttonCreate->RemoveCallback (this);

}

//-----------------------------------------------------------------

void SwgCuiSceneSelection::OnButtonPressed   (UIWidget *context)
{
	if (context == m_buttonCreate ||
		context == m_okButton)
	{
		Unicode::String selectedSceneFile;
		m_textbox->GetLocalText (selectedSceneFile);
		if (!selectedSceneFile.empty ())
		{
			if (context == m_buttonCreate)
			{
				//-- go to avatar customization screen
				CuiTransition::startTransition (CuiMediatorTypes::SceneSelection, CuiMediatorTypes::AvatarCreation);
			}
			
			//-- try to jump right in
			else
			{
				std::string avatarName;
				if (ConfigClientGame::getAvatarSelection())
					avatarName = ConfigClientGame::getAvatarSelection ();
				else
					avatarName = "object/creature/player/shared_human_male.iff";
				
				if (startScene (avatarName, 0))
				{
					deactivate ();
				}
			}
		}	
		else
		{
			// no selection
			CuiMessageBox::createInfoBox (CuiStringIds::scene_no_scene_selected.localize ());
		}
	}
	
	else if (context == m_cancelButton)
	{
		CuiTransition::startTransition (CuiMediatorTypes::SceneSelection, CuiMediatorTypes::LoginScreen);
	}
}

//-----------------------------------------------------------------

bool SwgCuiSceneSelection::OnMessage         (UIWidget *context, const UIMessage & )
{
	UNREF (context);
	return true;
}

//-----------------------------------------------------------------

void SwgCuiSceneSelection::OnListboxSelectionChanged ( UIWidget * )
{
	updateSceneSelection ();
}

//-----------------------------------------------------------------

void SwgCuiSceneSelection::updateSceneSelection ()
{
	UIString description;

	UIImage * screenshot = 0;

	if (m_selectionListbox->GetSelectionIndex () >= 0)
	{
		UIString imageName;
		if (m_selectionListbox->GetProperty (UILowerString ("SelectedItem.screenshot"), imageName))
		{
			UIBaseObject * obj = getPage ().GetObjectFromPath (imageName, TUIImage);

			DEBUG_FATAL (obj->IsA (TUIImage) == false, ("screenshot is not an image.\n"));

			screenshot = static_cast<UIImage*>(obj);
		}

		if (m_selectionListbox->GetProperty (UILowerString ("SelectedItem.Description"), description) == false)
			description.erase ();

		Unicode::String file;
		if (m_selectionListbox->GetProperty (UILowerString ("SelectedItem.filename"), file))
			m_textbox->SetLocalText (file);
	}

	else
	{
		m_textbox->SetLocalText (Unicode::String ());
	}

	if (screenshot)
	{
		UISize size = m_placeholderImage->GetSize ();
		UIPoint loc = m_placeholderImage->GetLocation ();

		const std::string oldName(m_placeholderImage->GetName ());
		m_placeholderImage->CopyPropertiesFrom (*screenshot);
		m_placeholderImage->SetVisible (true);

		m_placeholderImage->SetSize (size);
		m_placeholderImage->SetLocation (loc);

		// -- placeholder image must retain name so that it can be found by name at any time
		m_placeholderImage->SetName (oldName);
	}
	else
	{
		m_placeholderImage->SetVisible (false);
	}
	
	m_descriptionText->SetText (description);
}

//-----------------------------------------------------------------

bool SwgCuiSceneSelection::startScene ( const UINarrowString & avatarName, CreatureObject * customizedPlayer) const
{
//	GameNetwork::getConnectionManager ().selectCharacter (Unicode::narrowToWide (avatarName));	
		
	Unicode::String selectedSceneFile;
	m_textbox->GetLocalText (selectedSceneFile);

	std::string n_selectedSceneFile (Unicode::wideToNarrow (selectedSceneFile));

	if (selectedSceneFile.empty ())
	{
		// no file
		CuiMessageBox::createInfoBox (CuiStringIds::scene_no_scene_selected.localize ());
		return false;
	}
	
	if (!TreeFile::exists (n_selectedSceneFile.c_str ()))
	{
		// no file
		CuiMessageBox::createInfoBox (CuiStringIds::scene_no_scene_file.localize ());
		return false;
	}

	//-- jww: hack to show loading dialog
	CuiMessageBox * box = CuiMessageBox::createWaitingBox (Unicode::narrowToWide ("Please Wait.  Loading Scene..."));
	
	CuiManager::forceRender ();

	box->closeMessageBox ();
	
	//-- reset the mood to prevent bogus animation
	if (customizedPlayer)
	{
		customizedPlayer->clientSetMood  (1);
		customizedPlayer->clientSetMood (0);
	}

	Game::setScene(true, n_selectedSceneFile.c_str (), avatarName.c_str (), customizedPlayer);
	
	return true;
}

// ======================================================================
