//======================================================================
//
// SwgCuiCraftAssemblyDialogOption.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiCraftAssemblyDialogOption.h"

#include "UIButton.h"
#include "UIData.h"
#include "UIDataSource.h"
#include "UIList.h"
#include "UIPage.h"
#include "UIText.h"
#include "UnicodeUtils.h"
#include "clientGame/ClientObject.h"
#include "clientGame/DraftSchematicInfo.h"
#include "clientUserInterface/CuiCraftManager.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiStringIdsCraft.h"
#include "sharedObject/CachedNetworkId.h"
#include "swgClientUserInterface/SwgCuiCraftAssembly.h"

//======================================================================

namespace
{
	namespace DataProperties
	{
		const UILowerString LOCALDESCRIPTION = UILowerString ("LocalDesc");
	}
}

//----------------------------------------------------------------------

SwgCuiCraftAssemblyDialogOption::SwgCuiCraftAssemblyDialogOption (UIPage & page) :
CuiMediator    ("SwgCuiCraftAssemblyDialogOption", page),
m_buttonOk     (0),
m_buttonCancel (0),
m_list         (0),
m_text         (0),
m_object       (new CachedNetworkId),
m_slotIndex    (-1)
{
	getCodeDataObject(TUIButton, m_buttonOk,     "buttonOk");
	getCodeDataObject(TUIButton, m_buttonCancel, "buttonCancel");
	getCodeDataObject(TUIList,   m_list,         "list");
	getCodeDataObject(TUIText,   m_text,         "text");
	
	m_text->SetPreLocalized (true);
}

//----------------------------------------------------------------------

SwgCuiCraftAssemblyDialogOption::~SwgCuiCraftAssemblyDialogOption ()
{
	m_buttonOk = 0;
	m_buttonCancel = 0;
	m_list = 0;
	m_text = 0;

	delete m_object;
	m_object = 0;
}

//----------------------------------------------------------------------

void SwgCuiCraftAssemblyDialogOption::performActivate   ()
{
	*m_object = NetworkId::cms_invalid;
	m_slotIndex = -1;

	//-- attempt to center the dialog
	UIPage * page = 0;
	getCodeDataObject(TUIPage,   page,         "page", true);

	if (page)
	{
		page->Center ();
	}

	getPage ().SetFocus ();

	m_buttonOk->AddCallback (this);
	m_buttonCancel->AddCallback (this);
	m_list->AddCallback (this);

	setPointerInputActive (true);
	setKeyboardInputActive (true);
	setInputToggleActive (false);
}

//----------------------------------------------------------------------

void SwgCuiCraftAssemblyDialogOption::performDeactivate ()
{
	m_buttonOk->RemoveCallback (this);
	m_buttonCancel->RemoveCallback (this);
	m_list->RemoveCallback (this);
}

//----------------------------------------------------------------------

void SwgCuiCraftAssemblyDialogOption::OnButtonPressed( UIWidget *context )
{
	if (context == m_buttonOk)
	{
		completeTransfer (m_list->GetLastSelectedRow ());
	}
	else if (context == m_buttonCancel)
	{
		deactivate ();
	}
}

//----------------------------------------------------------------------

void SwgCuiCraftAssemblyDialogOption::OnGenericSelectionChanged (UIWidget * context)
{
	if (context == m_list)
	{
		update ();
	}
}

//----------------------------------------------------------------------

void SwgCuiCraftAssemblyDialogOption::update ()
{
	const int index = m_list->GetLastSelectedRow ();
	
	Unicode::String str;
	
	const UIData * const data = m_list->GetDataAtRow (index);
	if (data)
	{
		IGNORE_RETURN (data->GetProperty (DataProperties::LOCALDESCRIPTION, str));
	}

	m_text->SetLocalText (str);
}

//----------------------------------------------------------------------

void SwgCuiCraftAssemblyDialogOption::setSlot (const ClientObject & transferringObject, int slotIndex, int optionIndex)
{
	UIDataSource * ds = m_list->GetDataSource ();
	if (!ds)
	{
		m_list->SetDataSource (new UIDataSource);
		ds = NON_NULL (m_list->GetDataSource ());
	}

	ds->Clear ();

	//----------------------------------------------------------------------

	const Slot * const slot = CuiCraftManager::getSlot (slotIndex);

	*m_object = NetworkId::cms_invalid;
	m_slotIndex = -1;

	if (!slot)
	{
		WARNING (true, ("bad slot index"));
		deactivate ();
		return;
	}

	*m_object = CachedNetworkId (transferringObject);
	m_slotIndex = slotIndex;

	if (slot->options.empty ())
	{
		WARNING (true, ("slot has empty ingredients vector."));
		deactivate ();
		return;
	}

	CuiCraftManager::IntVector iv;
	CuiCraftManager::getValidSlotOptions (transferringObject, slotIndex, iv);

	if (iv.empty ())
	{
		CuiMessageBox::createInfoBox (CuiStringIdsCraft::err_ingredient_not_for_slot.localize ());
		deactivate ();
		return;
	}

	const int numOptions      = static_cast<int>(slot->options.size ());
	const int numValidOptions = static_cast<int>(iv.size ());

	if (numValidOptions == 1)
	{
		completeTransfer (iv.front ());
		return;
	}

	if (optionIndex >= 0)
	{
		DEBUG_FATAL (optionIndex >= numOptions, ("bad options"));
		completeTransfer (optionIndex);
		return;
	}

	//----------------------------------------------------------------------

	if (!iv.empty ())
	{
		typedef MessageQueueDraftSlots::Option Option;
		typedef stdvector<Option>::fwd OptionVector;
		
		const ManufactureSchematicObject * const manf_schem = CuiCraftManager::getManufactureSchematic ();

		for (CuiCraftManager::IntVector::const_iterator it = iv.begin (); it != iv.end (); ++it)
		{
			const int optionIndex = *it;
			DEBUG_FATAL (optionIndex < 0, ("bad"));
			if (optionIndex >= numOptions)
				WARNING (true, ("bad option"));
			else
			{
				const Option & opt = slot->options [optionIndex];

				UIData * const data = new UIData;
				const Unicode::String & nameStr = opt.name.localize ();
								
				data->SetName     (Unicode::wideToNarrow (nameStr));
				data->SetProperty (UIList::DataProperties::LOCALTEXT, nameStr);

				Unicode::String desc;
				if (DraftSchematicInfo::createOptionDescription  (*slot, manf_schem, optionIndex, desc, true, true))
					data->SetProperty (DataProperties::LOCALDESCRIPTION,  desc);

				ds->AddChild      (data);
			}
		}
	}
	
	m_list->SelectRow (-1);
	update ();
}

//----------------------------------------------------------------------

void SwgCuiCraftAssemblyDialogOption::completeTransfer (int optionIndex)
{
	if (optionIndex < 0)
	{
		CuiMessageBox::createInfoBox (CuiStringIdsCraft::err_no_option_selected.localize ());
		return;
	}

	CuiCraftManager::transferToSlot (*m_object, m_slotIndex, optionIndex);
	deactivate ();
}

//======================================================================
