//======================================================================
//
// CuiDragInfo.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiDragInfo.h"

#include "UnicodeUtils.h"
#include "clientUserInterface/CuiWidget3dObjectViewer.h"
#include "clientUserInterface/CuiWidget3dObjectListViewer.h"
#include "clientGame/ClientObject.h"
#include "clientGame/AttachmentData.h"

//======================================================================

const char * const CuiDragInfo::DragTypes::GameObject      = "GameObject";
const char * const CuiDragInfo::DragTypes::Command         = "Command";
const char * const CuiDragInfo::DragTypes::AttachmentData  = "AttachmentData";
const char * const CuiDragInfo::DragTypes::Macro           = "Macro";

//----------------------------------------------------------------------

const UILowerString CuiDragInfo::Properties::CommandName   = UILowerString ("CmdName");
const UILowerString CuiDragInfo::Properties::DragInfoName  = UILowerString ("DragInfoName");
const UILowerString CuiDragInfo::Properties::CommandString = UILowerString ("CmdStr");
const UILowerString CuiDragInfo::Properties::CommandValue  = UILowerString ("DragCommandValue");
const UILowerString CuiDragInfo::Properties::Oob           = UILowerString ("DragOob");

//----------------------------------------------------------------------

CuiDragInfo::CuiDragInfo (const UIBaseObject & dragWidget) :
name              (),
str               (),
objectId          (),
type              (CuiDragInfoTypes::CDIT_none),
commandValue      (0.0f),
commandValueValid (false),
m_attachmentData  (0)
{
	set (dragWidget);
}

//----------------------------------------------------------------------

CuiDragInfo::CuiDragInfo () :
name              (),
str               (),
cmd               (),
objectId          (),
type              (CuiDragInfoTypes::CDIT_none),
commandValue      (0.0f),
commandValueValid (false),
m_attachmentData  (0)
{
}

//----------------------------------------------------------------------

CuiDragInfo::~CuiDragInfo ()
{
	clear ();
}

//----------------------------------------------------------------------

void CuiDragInfo::clear ()
{
	name.clear ();
	str.clear  ();
	cmd.clear  ();

	objectId          = NetworkId::cms_invalid;
	type              = CuiDragInfoTypes::CDIT_none;
	commandValue      = 0.0f;
	commandValueValid = false;

	if (m_attachmentData)
	{
		delete m_attachmentData;
		m_attachmentData = 0;
	}
}

//----------------------------------------------------------------------

void CuiDragInfo::setWidget   (UIBaseObject & dragWidget) const
{
	UNREF (dragWidget);

	if (commandValueValid)
		dragWidget.SetPropertyFloat   (Properties::CommandValue, commandValue);
	else
		dragWidget.PurgeProperty          (Properties::CommandValue);

	if (name.empty ())
		dragWidget.RemoveProperty (Properties::DragInfoName);
	else
		dragWidget.SetProperty (Properties::DragInfoName, name);

	dragWidget.RemoveProperty (UIWidget::PropertyName::Tooltip);

	switch (type)
	{
	case CuiDragInfoTypes::CDIT_command:
		{
			if (str.empty ())
				dragWidget.RemoveProperty (Properties::CommandString);
			else
				dragWidget.SetPropertyNarrow (Properties::CommandString, str);

			if (cmd.empty ())
				dragWidget.RemoveProperty (Properties::CommandName);
			else
				dragWidget.SetPropertyNarrow (Properties::CommandName, cmd);

			dragWidget.SetPropertyNarrow (UIWidget::PropertyName::DragType, DragTypes::Command);
		}
		break;
	case CuiDragInfoTypes::CDIT_macro:
		{
			if (str.empty ())
				dragWidget.RemoveProperty (Properties::CommandString);
			else
				dragWidget.SetPropertyNarrow (Properties::CommandString, str);

			if (cmd.empty ())
				dragWidget.RemoveProperty (Properties::CommandName);
			else
				dragWidget.SetPropertyNarrow (Properties::CommandName, cmd);

			dragWidget.SetPropertyNarrow (UIWidget::PropertyName::DragType, DragTypes::Macro);
		}
		break;
	case CuiDragInfoTypes::CDIT_object:
		{
			dragWidget.SetPropertyNarrow (UIWidget::PropertyName::DragType, DragTypes::GameObject);
			dragWidget.RemoveProperty (Properties::CommandName);
			dragWidget.RemoveProperty (Properties::CommandString);
		}
		break;
	case CuiDragInfoTypes::CDIT_attachmentData:
		{
			dragWidget.SetPropertyNarrow (UIWidget::PropertyName::DragType, DragTypes::AttachmentData);

			if (m_attachmentData)
			{
				Unicode::String oob;
				m_attachmentData->packToOob (oob, -3);
				dragWidget.SetProperty (Properties::Oob, oob);
			}
			else
				dragWidget.RemoveProperty (Properties::Oob);
		}
		break;
	}

	dragWidget.SetPropertyBoolean (UIWidget::PropertyName::Dragable, true);
}

//----------------------------------------------------------------------

void   CuiDragInfo::set (const UIBaseObject & dragWidget)
{
	clear ();
	
	if (!dragWidget.GetProperty (Properties::DragInfoName, name))
		name.clear ();

	UIString dragType;
	if (!dragWidget.GetProperty (UIWidget::PropertyName::DragType, dragType))
		return;
	
	//----------------------------------------------------------------------
	//-- objects must be dropped via a 3d object viewer

	if (Unicode::caseInsensitiveCompare (dragType, Unicode::narrowToWide (DragTypes::GameObject)))
	{		
		const CuiWidget3dObjectViewer * const droppingViewer = dynamic_cast<const CuiWidget3dObjectViewer *>(&dragWidget);
		
		if (droppingViewer)
		{
			const ClientObject * const droppingObj = safe_cast<const ClientObject *>(droppingViewer->getObject ());
			if (droppingObj)
			{
				objectId = CachedNetworkId (droppingObj->getNetworkId ());
				type = CuiDragInfoTypes::CDIT_object;
			}
			return;
		}
		
		const CuiWidget3dObjectListViewer * const droppingListViewer = dynamic_cast<const CuiWidget3dObjectListViewer *>(&dragWidget);
		
		if (droppingListViewer)
		{
			const ClientObject * const droppingObj = safe_cast<const ClientObject *>(droppingListViewer->getLastObject ());
			
			if (droppingObj)
			{
				objectId = CachedNetworkId (droppingObj->getNetworkId ());
				type = CuiDragInfoTypes::CDIT_object;
			}
			
			return;
		}

		return;
	}
	
	static const Unicode::String s_tmp_uiCommand    = Unicode::narrowToWide ("UICommand");
	static const Unicode::String s_tmp_skillCommand = Unicode::narrowToWide ("SkillCommand");

	//----------------------------------------------------------------------
	//-- ui commands are dropped using the DragCommandProperty

	if (Unicode::caseInsensitiveCompare (dragType, Unicode::narrowToWide (DragTypes::Command)) ||
			Unicode::caseInsensitiveCompare (dragType, s_tmp_uiCommand) ||
			Unicode::caseInsensitiveCompare (dragType, s_tmp_skillCommand))
	{
		if (!dragWidget.GetPropertyNarrow (Properties::CommandName, cmd))
			cmd.clear ();
		
		if (!dragWidget.GetPropertyNarrow (Properties::CommandString, str))
			str.clear ();
		
		if (cmd.empty () && str.empty ())
		{
			DEBUG_WARNING (true, ("Can't set drag info for DragTypes::Command with no DragCommandProperty for [%s]", dragWidget.GetFullPath ().c_str ()));
		}

		commandValueValid = dragWidget.GetPropertyFloat   (Properties::CommandValue, commandValue);

		type = CuiDragInfoTypes::CDIT_command;

		return;
	}

	//----------------------------------------------------------------------
	//-- ui commands are dropped using the DragCommandProperty

	if (Unicode::caseInsensitiveCompare (dragType, Unicode::narrowToWide (DragTypes::Macro)))
	{
		if (!dragWidget.GetPropertyNarrow (Properties::CommandName, cmd))
			cmd.clear ();
		
		if (!dragWidget.GetPropertyNarrow (Properties::CommandString, str))
			str.clear ();
		
		DEBUG_WARNING (cmd.empty () || name.empty (), ("Can't set drag info for DragTypes::Macro with no command or name for [%s]", dragWidget.GetFullPath ().c_str ()));

		commandValueValid = false;

		type = getDragType (Unicode::wideToNarrow (dragType));
		return;
	}


	//----------------------------------------------------------------------
	//--

	if (Unicode::caseInsensitiveCompare (dragType, Unicode::narrowToWide (DragTypes::AttachmentData)))
	{
		Unicode::String oob;
		dragWidget.GetProperty (Properties::Oob, oob);
		type              = CuiDragInfoTypes::CDIT_attachmentData;
		commandValueValid = false;
		commandValue      = 0.0f;
		cmd.clear ();
		m_attachmentData  = new AttachmentData (oob);

		return;
	}
}

//----------------------------------------------------------------------

void CuiDragInfo::set (const AttachmentData & ad)
{
	clear ();
	type              = CuiDragInfoTypes::CDIT_attachmentData;
	commandValueValid = false;
	commandValue      = 0.0f;
	cmd.clear ();
	m_attachmentData  = new AttachmentData (ad);
}

//----------------------------------------------------------------------

ClientObject * CuiDragInfo::getClientObject () const
{
	if (type == CuiDragInfoTypes::CDIT_object)
		return dynamic_cast<ClientObject *>(objectId.getObject ());

	return 0;
}

//----------------------------------------------------------------------

const char * const CuiDragInfo::getDragTypeString  (int type)
{
	switch (type)
	{
	case CuiDragInfoTypes::CDIT_object:
		return DragTypes::GameObject;
	case CuiDragInfoTypes::CDIT_command:
		return DragTypes::Command;
	case CuiDragInfoTypes::CDIT_macro:
		return DragTypes::Macro;
	case CuiDragInfoTypes::CDIT_attachmentData:
		return DragTypes::AttachmentData;
	}

	return 0;
}

//----------------------------------------------------------------------

CuiDragInfoTypes::Type CuiDragInfo::getDragType    (const std::string & str)
{
	if (str == DragTypes::GameObject)
		return CuiDragInfoTypes::CDIT_object;
	if (str == DragTypes::Command)
		return CuiDragInfoTypes::CDIT_command;
	if (str == DragTypes::Macro)
		return CuiDragInfoTypes::CDIT_macro;
	if (str == DragTypes::AttachmentData)
		return CuiDragInfoTypes::CDIT_attachmentData;

	return CuiDragInfoTypes::CDIT_none;
}


//======================================================================
