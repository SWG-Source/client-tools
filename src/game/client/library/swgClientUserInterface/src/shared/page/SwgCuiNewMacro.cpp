//======================================================================
//
// SwgCuiNewMacro.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiNewMacro.h"

#include "UIButton.h"
#include "UICanvas.h"
#include "UIData.h"
#include "UIImage.h"
#include "UIImageStyle.h"
#include "UIManager.h"
#include "UINamespace.h"
#include "UIPage.h"
#include "UIText.h"
#include "UIVolumePage.h"
#include "UnicodeUtils.h"
#include "clientGame/ClientMacroManager.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiStringIds.h"
#include "sharedMessageDispatch/Transceiver.h"

#include <algorithm>
#include <iterator>

//======================================================================

namespace SwgCuiNewMacroNamespace
{
	const std::string cs_baseIconPath     = "/styles.icon.";
	const std::string cs_commandIconPath  = cs_baseIconPath + "command";
	const std::string cs_socialIconPath   = cs_baseIconPath + "social";
	const std::string cs_stateIconPath    = cs_baseIconPath + "state";
	const std::string cs_moodIconPath     = cs_baseIconPath + "mood";
	const std::string cs_postureIconPath  = cs_baseIconPath + "posture";
	const std::string cs_musicIconPath    = cs_baseIconPath + "music";
	const std::string cs_uiIconPath       = cs_baseIconPath + "ui";
	const std::string cs_factionIconPath  = cs_baseIconPath + "faction";
	const std::string cs_combatIconPath   = cs_baseIconPath + "combat";

	std::vector<std::string>             cs_iconPaths;

	//debugging flag, sets the style name into the icon tooltip (good for searching out bad icon references)
	bool                                 cs_showIconTooltipStyleNames = false;

	const UILowerString s_imageStylePropertyName("ImageStyleName");
	
	struct ImageRect
	{
		const UICanvas * canvas;
		UIRect           rect;
		
		ImageRect (const UICanvas * _canvas, const UIRect & _rect) :
		canvas (_canvas),
		rect   (_rect)
		{
		}

		ImageRect (const ImageRect & rhs) :
		canvas (rhs.canvas),
		rect   (rhs.rect)
		{
		}

		ImageRect () :
		canvas (0),
		rect ()
		{
		}

		ImageRect & operator= (const ImageRect & rhs)
		{
			canvas = rhs.canvas;
			rect   = rhs.rect;
		}
		
		//-- arbitrary sorting
		
		bool operator<(const ImageRect & rhs) const 
		{
			if (canvas < rhs.canvas) return true;
			
			if (canvas == rhs.canvas)
			{
				if (rect.left < rhs.rect.left) return true;
				
				if (rect.left == rhs.rect.left)
				{
					if (rect.right < rhs.rect.right) return true;
					
					if (rect.right == rhs.rect.right)
					{
						if (rect.top < rhs.rect.top) return true;
						
						if (rect.top == rhs.rect.top)
						{
							if (rect.bottom < rhs.rect.bottom) return true;
						}
					}
				}
			}
			
			return false;
		}
	};	
}

using namespace SwgCuiNewMacroNamespace;

//======================================================================

SwgCuiNewMacro::SwgCuiNewMacro (UIPage & page) : 
CuiMediator        ("SwgCuiNewMacro", page),
UIEventCallback    (),
m_callback         (new MessageDispatch::Callback),
m_okButton         (0),
m_cancelButton     (0),
m_nameText         (0),
m_textText         (0),
m_iconVolume       (0),
m_sampleIcon       (0),
m_macroCommandName ()
{
	cs_iconPaths.clear();
	cs_iconPaths.push_back(cs_commandIconPath);
	cs_iconPaths.push_back(cs_socialIconPath);
	cs_iconPaths.push_back(cs_stateIconPath);
	cs_iconPaths.push_back(cs_moodIconPath);
	cs_iconPaths.push_back(cs_postureIconPath);
	cs_iconPaths.push_back(cs_musicIconPath);
	cs_iconPaths.push_back(cs_uiIconPath);
	cs_iconPaths.push_back(cs_factionIconPath);

	setState    (MS_closeable);
	setState    (MS_closeDeactivates);
	removeState (MS_iconifiable);
	
	getCodeDataObject (TUIButton,     m_okButton,     "buttonok");
	getCodeDataObject (TUIButton,     m_cancelButton, "buttoncancel");
	getCodeDataObject (TUIText,       m_nameText,     "textname");
	getCodeDataObject (TUIText,       m_textText,     "texttext");
	getCodeDataObject (TUIVolumePage, m_iconVolume,   "volumeicon");
	getCodeDataObject (TUIImage,      m_sampleIcon,   "sampleicon");

	registerMediatorObject (*m_okButton,       true);
	registerMediatorObject (*m_cancelButton,   true);

	m_iconVolume->Clear();

	std::string name;

	const UIPage * const rootPage = UIManager::gUIManager ().GetRootPage ();
	if(rootPage)
	{
		typedef stdset<ImageRect>::fwd ImageRectSet;
		ImageRectSet irs;

		for(std::vector<std::string>::iterator i = cs_iconPaths.begin(); i != cs_iconPaths.end(); ++i)
		{
			UINamespace* namesp = safe_cast<UINamespace*>(rootPage->GetObjectFromPath (i->c_str ()));
			if(namesp)
			{
				for(unsigned int j = 0; j < namesp->GetChildCount(); ++j)
				{
					UIImageStyle * const imageStyle = safe_cast<UIImageStyle *>(namesp->GetChildByPositionLinear(j));
					if (imageStyle)
					{
						//-- see if we already have this rect
						const ImageRect ir (imageStyle->GetSourceCanvas (), imageStyle->GetSourceRect ());
						if (irs.find (ir) != irs.end ())
							continue;

						irs.insert (ir);

						UIImage * const dupe = NON_NULL(safe_cast<UIImage *>(m_sampleIcon->DuplicateObject ()));
						dupe->SetVisible(true);
						dupe->SetStyle(imageStyle);
						name  = *i;
						name.push_back ('.');
						name += imageStyle->GetName();

						if (cs_showIconTooltipStyleNames)
						{
							dupe->SetTooltip(Unicode::narrowToWide(name));
						}

						dupe->SetProperty(s_imageStylePropertyName, Unicode::narrowToWide(imageStyle->GetName()));
						m_iconVolume->AddChild(dupe);
					}
				}
			}
		}
	}

	m_iconVolume->Link ();
	m_iconVolume->Pack ();

	m_nameText->Clear();
	m_textText->Clear();
	m_iconVolume->SetSelectionIndex(-1);
}

//----------------------------------------------------------------------

SwgCuiNewMacro::~SwgCuiNewMacro ()
{
	delete m_callback;
	m_callback     = 0;

	m_okButton     = 0;
	m_cancelButton = 0;
	m_nameText     = 0;
	m_textText     = 0;
	m_iconVolume   = 0;
}

//----------------------------------------------------------------------

void SwgCuiNewMacro::performActivate   ()
{
}

//----------------------------------------------------------------------

void SwgCuiNewMacro::performDeactivate ()
{
}

//----------------------------------------------------------------------

void SwgCuiNewMacro::setParams(const Unicode::String& params)
{
	if (!params.empty ())
	{
		//-- 0 name
		//-- 1 user defined name
		//-- 2 icon
		//-- 3 color
		//-- . command string

		static const size_t numParams = 4;
		Unicode::String paramArray [numParams];
		
		size_t endpos = 0;
		
		int i = 0;
		for (i = 0; i < numParams; ++i)
		{
			Unicode::String token;
			if (!Unicode::getFirstToken (params, endpos, endpos, paramArray [i]) || endpos == Unicode::String::npos)
				break;
			
			++endpos;
		}

		Unicode::String commandString;
		if (endpos != Unicode::String::npos)
			commandString = params.substr (endpos);

		m_macroCommandName = Unicode::wideToNarrow (paramArray [0]);
		m_nameText->SetLocalText (paramArray [1]);
		m_textText->SetLocalText (commandString);


		long max = m_iconVolume->GetCellMax();
		for(i = 0; i < max; ++i)
		{
			UIWidget* w = m_iconVolume->FindCell(i);
			if(w)
			{
				Unicode::String style;
				w->GetProperty(s_imageStylePropertyName, style);
				if(style == paramArray[2])
				{
					m_iconVolume->SetSelectionIndex(i);
					break;
				}
			}
		}
		return;
	}
	
	m_macroCommandName.clear ();
	m_nameText->Clear ();
	m_textText->Clear ();
}

//----------------------------------------------------------------------

void SwgCuiNewMacro::OnButtonPressed   (UIWidget * context)
{
	if (context == m_okButton)
	{
		UIString name = m_nameText->GetLocalText();
		UIString text = m_textText->GetLocalText();

		std::string narrowText = Unicode::wideToUTF8 (text);

		const std::vector<std::string> & disallowedMacroCommands = ClientMacroManager::getDisallowedCommands();
		for(std::vector<std::string>::const_iterator i = disallowedMacroCommands.begin(); i != disallowedMacroCommands.end(); ++i)
		{
			std::string lowertext;
			std::string lowercommand;
			std::transform(text.begin(), text.end(), std::back_inserter(lowertext),    tolower);
			std::transform(i->begin(),   i->end(),   std::back_inserter(lowercommand), tolower);

			if(lowertext.find(lowercommand) != text.npos)
			{
				Unicode::String msg = CuiStringIds::macro_badcommand1.localize();
				msg += Unicode::narrowToWide(*i);
				msg += CuiStringIds::macro_badcommand2.localize();
				CuiMessageBox::createInfoBox(msg);
				return;
			}
		}

		//refuse to accept macros with spaces, but gracefully replace the string in the GUI with an unspaced one
		if(name.find(Unicode::narrowToWide(" ")) != name.npos)
		{
			CuiMessageBox::createInfoBox(CuiStringIds::macro_nospaces.localize());
			bool done = false;
			while(!done)
			{
				std::string::size_type p = name.find_first_of(Unicode::narrowToWide(" "));
				if(p == name.npos)
					done = true;
				else
					name.erase(p, 1);
			}
			m_nameText->SetLocalText(name);
		}
		else
		{
			//get the icon to use
			UIImage * const icon  = safe_cast<UIImage *>(m_iconVolume->GetLastSelectedChild ());
			if (!icon)
			{
				CuiMessageBox::createInfoBox(CuiStringIds::macro_selecticon.localize());
			}
			else
			{
				UIImageStyle const * const style  = NON_NULL(icon->GetImageStyle());
				if(style)
				{
					const std::string & styleName = style->GetName();

					if (m_macroCommandName.empty ())
						ClientMacroManager::createNewMacroData (Unicode::wideToUTF8 (name), Unicode::wideToUTF8 (text), styleName);
					else
						ClientMacroManager::modifyMacroData    (m_macroCommandName, Unicode::wideToUTF8 (name), Unicode::wideToUTF8 (text), styleName);

					ClientMacroManager::save ();
				}
				deactivate();
			}
		}
	}
	else if (context == m_cancelButton)
	{
		deactivate();
	}
}

//----------------------------------------------------------------------

SwgCuiNewMacro *  SwgCuiNewMacro::createInto (UIPage & parent)
{
	UIPage * const dupe = NON_NULL (UIPage::DuplicateInto (parent, "/skill.newmacro"));
	return new SwgCuiNewMacro (*dupe);
}

//======================================================================
