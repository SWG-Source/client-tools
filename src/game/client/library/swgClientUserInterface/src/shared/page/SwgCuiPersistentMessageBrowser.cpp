//======================================================================
//
// SwgCuiPersistentMessageBrowser.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiPersistentMessageBrowser.h"

#include "UIButton.h"
#include "UIData.h"
#include "UIDataSource.h"
#include "UIManager.h"
#include "UIMessage.h"
#include "UIPage.h"
#include "UIPopupMenu.h"
#include "UITable.h"
#include "UITableModelDefault.h"
#include "UIText.h"
#include "UIUtils.h"
#include "UIVolumePage.h"
#include "UnicodeUtils.h"
#include "clientGame/AttachmentData.h"
#include "clientGame/Game.h"
#include "clientUserInterface/CuiActionManager.h"
#include "clientUserInterface/CuiActions.h"
#include "clientUserInterface/CuiChatManager.h"
#include "clientUserInterface/CuiChatRoomManager.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiPersistentMessageManager.h"
#include "clientUserInterface/CuiStringIdsPersistentMessage.h"
#include "clientUserInterface/CuiUtils.h"
#include "clientUserInterface/CuiWidget3dObjectListViewer.h"
#include "clientUserInterface/CuiWorkspace.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/ChatAvatarId.h"
#include "sharedNetworkMessages/ChatPersistentMessageToClientData.h"
#include "swgClientUserInterface/SwgCuiAttachmentList.h"
#include <list>

//======================================================================

namespace
{
	const Unicode::String include_prefix = Unicode::narrowToWide (" > ");
	const size_t include_prefix_size = include_prefix.size ();

	void constructIncludedBody (const Unicode::String & message, Unicode::String & includedBody)
	{
		includedBody.reserve (message.size () * 2);
		includedBody.push_back ('\n');
		includedBody.append (message);
		includedBody.insert (0, include_prefix);

		for (size_t endpos = 0; endpos != Unicode::String::npos; )
		{
			endpos = includedBody.find ('\n', endpos);
			if (endpos == Unicode::String::npos)
				break;

			++endpos;
			includedBody.insert (endpos, include_prefix);
			endpos += include_prefix_size;
		}

		includedBody.append (1, '\n');
		includedBody.insert (size_t(0), size_t(2), Unicode::unicode_char_t('\n'));
	}

	namespace PopupItems
	{
		const std::string save     = "popup_save";
		const std::string save_all = "popup_saveAll";
	};

	namespace Properties
	{
		const UILowerString PopupChildIndex ("PopupChildIndex");
	}
}

//----------------------------------------------------------------------

SwgCuiPersistentMessageBrowser::SwgCuiPersistentMessageBrowser (UIPage & page) :
CuiMediator              ("SwgCuiPersistentMessageBrowser", page),
UIEventCallback          (),
m_table                  (0),
m_buttonOk               (0),
m_buttonNew              (0),
m_buttonDelete           (0),
m_buttonReply            (0),
m_buttonForward          (0),
m_textBody               (0),
m_callback               (new MessageDispatch::Callback),
m_model                  (0),
m_volumeAttach           (0),
m_attachments            (new SwgCuiAttachmentList),
m_lastSelectedRowVisual  (0L),
m_lastScrollPosY         (0L)
{
	getCodeDataObject (TUITable,      m_table,         "table");
	getCodeDataObject (TUIButton,     m_buttonOk,      "buttonOk");
	getCodeDataObject (TUIButton,     m_buttonNew,     "buttonNew");
	getCodeDataObject (TUIButton,     m_buttonDelete,  "buttonDelete");
	getCodeDataObject (TUIButton,     m_buttonReply,   "buttonReply");
	getCodeDataObject (TUIButton,     m_buttonForward, "buttonForward");
	getCodeDataObject (TUIText,       m_textBody,      "textBody");
	getCodeDataObject (TUIVolumePage, m_volumeAttach,  "volumeAttach");

	m_textBody->SetPreLocalized (true);

	m_model = NON_NULL (safe_cast<UITableModelDefault *>(m_table->GetTableModel ()));
	m_model->Attach (0);

	setState (MS_closeable);
	setState (MS_closeDeactivates);

	// disable New/Reply/Forward button if squelched or in tutorial
	const bool isSquelched = Game::isPlayerSquelched();
	m_buttonNew->SetEnabled(!isSquelched && !Game::isTutorial());
	m_buttonReply->SetEnabled(!isSquelched && !Game::isTutorial());
	m_buttonForward->SetEnabled(!isSquelched && !Game::isTutorial());

	registerMediatorObject (*m_table,         true);
	registerMediatorObject (*m_buttonNew,     true);
	registerMediatorObject (*m_buttonDelete,  true);
	registerMediatorObject (*m_buttonOk,      true);
	registerMediatorObject (*m_buttonReply,   true);
	registerMediatorObject (*m_buttonForward, true);
	registerMediatorObject (*m_volumeAttach,  true);
}

//----------------------------------------------------------------------

SwgCuiPersistentMessageBrowser::~SwgCuiPersistentMessageBrowser ()
{
	delete m_callback;
	m_callback = 0;
	
	delete m_attachments;
	m_attachments = 0;

	m_table          = 0;
	m_buttonOk       = 0;
	m_buttonNew      = 0;
	m_buttonDelete   = 0;
	m_buttonReply    = 0;
	m_buttonForward  = 0;
	m_textBody       = 0;

	m_model->Detach (0);
	m_model          = 0;
}

//----------------------------------------------------------------------


void SwgCuiPersistentMessageBrowser::performActivate   ()
{
	CuiManager::requestPointer (true);

	update ();

	m_callback->connect (*this, &SwgCuiPersistentMessageBrowser::onReceiveHeader,       static_cast<CuiPersistentMessageManager::Messages::HeaderReceived *>   (0));
	m_callback->connect (*this, &SwgCuiPersistentMessageBrowser::onReceiveBody,         static_cast<CuiPersistentMessageManager::Messages::BodyReceived *>     (0));
	m_callback->connect (*this, &SwgCuiPersistentMessageBrowser::onEmailListUpdated, static_cast<CuiPersistentMessageManager::Messages::EmailListChanged *>     (0));

	m_volumeAttach->SetVisible (false);

	m_table->SelectRow     (m_lastSelectedRowVisual);
	m_table->ScrollToPoint (UIPoint (0L, m_lastScrollPosY));
	m_table->ScrollToRow   (m_lastSelectedRowVisual);
	m_lastSelectedRowVisual = 0;

	OnGenericSelectionChanged (m_table);

	setIsUpdating (true);
}

//----------------------------------------------------------------------

void SwgCuiPersistentMessageBrowser::performDeactivate ()
{
	CuiManager::requestPointer (false);

	m_callback->disconnect (*this, &SwgCuiPersistentMessageBrowser::onReceiveHeader,       static_cast<CuiPersistentMessageManager::Messages::HeaderReceived *>   (0));
	m_callback->disconnect (*this, &SwgCuiPersistentMessageBrowser::onReceiveBody,         static_cast<CuiPersistentMessageManager::Messages::BodyReceived *>     (0));
	m_callback->disconnect (*this, &SwgCuiPersistentMessageBrowser::onEmailListUpdated, static_cast<CuiPersistentMessageManager::Messages::EmailListChanged *>     (0));

	SwgCuiAttachmentList::Renderable::releaseIconsForPage (*m_volumeAttach);

	m_attachments->clear ();
	m_volumeAttach->Clear ();

	m_lastSelectedRowVisual = m_table->GetLastSelectedRow ();
	m_lastScrollPosY = m_table->GetScrollLocation ().y;

	setIsUpdating (false);
}

//----------------------------------------------------------------------

void  SwgCuiPersistentMessageBrowser::OnButtonPressed( UIWidget *context )
{
	if (context == m_buttonNew)
	{
		CuiPersistentMessageManager::startComposingNewMessage (std::string (), Unicode::emptyString, Unicode::emptyString, Unicode::emptyString);
	}
	else if (context == m_buttonOk)
	{
		if (getButtonClose ())
			getButtonClose ()->Press ();
	}
	else if (context == m_buttonDelete)
	{
		deleteSelection ();
	}
	else if (context == m_buttonReply)
	{
		const ChatPersistentMessageToClientData * const data = getLastSelectedData ();
	
		if (!data)
			return;

		const ChatAvatarId avatarId (data->fromGameCode, data->fromServerCode, data->fromCharacterName);
		reply (CuiStringIdsPersistentMessage::reply_prefix, avatarId);
	}
	else if (context == m_buttonForward)
	{
		reply (CuiStringIdsPersistentMessage::fwd_prefix, ChatAvatarId ());
	}
}

//----------------------------------------------------------------------

void SwgCuiPersistentMessageBrowser::OnGenericSelectionChanged (UIWidget * context)
{
	if (context == m_table)
	{
		updateBody ();
		updateAttachments ();

		const ChatPersistentMessageToClientData * const data = getLastSelectedData ();

		m_buttonReply->SetEnabled   (data != 0);
		m_buttonForward->SetEnabled (data != 0);
		m_buttonDelete->SetEnabled  (data != 0);
	}
}

//----------------------------------------------------------------------

bool SwgCuiPersistentMessageBrowser::OnMessage  (UIWidget * context, const UIMessage & msg)
{
	if (context->GetParent () == m_volumeAttach)
	{
		const UIBaseObject::UIObjectList & olist = m_volumeAttach->GetChildrenRef ();
		const UIBaseObject::UIObjectList::const_iterator it = std::find (olist.begin (), olist.end (), context);
		if (it == olist.end ())
		{
			WARNING (true, ("SwgCuiPersistentMessageBrowser::OnMessage bad attachment"));
			return false;
		}

		const int index = std::distance (olist.begin (), it);

		if (msg.Type == UIMessage::LeftMouseDoubleClick)
		{
			handleDoubleClick (index);
			return false;
		}
		else if (msg.Type == UIMessage::ContextRequest)
		{
			UIWidget * const child = context->GetWidgetFromPoint (msg.MouseCoords, true);
			int index = -1;

			if (child)
				child->GetPropertyInteger (UIBaseObject::PropertyName::Name, index);

			UIPopupMenu * const pop = createContextMenu (index);
			NOT_NULL (pop); 
			pop->SetLocation (context->GetWorldLocation () + msg.MouseCoords);
			UIManager::gUIManager ().PushContextWidget (*pop);
			return false;
		}
	}

	else if (context == m_table)
	{
		if (msg.Type == UIMessage::KeyDown)
		{
			if (msg.Keystroke == UIMessage::Delete)
			{
				deleteSelection ();
				return false;
			}
		}
	}

	return true;
}

//----------------------------------------------------------------------

void SwgCuiPersistentMessageBrowser::OnPopupMenuSelection (UIWidget * context)
{
	NOT_NULL (context);
	UIPopupMenu * const pop = safe_cast<UIPopupMenu *>(context);
	
	const std::string & selection = pop->GetSelectedName ();
	if (selection == PopupItems::save_all)
	{
		saveAttachment (-1);
		return;
	}
	
	int index = 0;
	if (context->GetPropertyInteger (Properties::PopupChildIndex, index))
	{
		if (selection == PopupItems::save)
		{
			saveAttachment (index);
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiPersistentMessageBrowser::saveAttachment (int index)
{
	if (index < 0)
	{		
		for (int i = 0; i < m_attachments->size (); ++i)
			saveAttachment (i);

		return;
	}

	if (index >= m_attachments->size ())
		return;

	SwgCuiAttachmentList::Renderable * const renderable = m_attachments->get (index);
	NOT_NULL (renderable);
	NOT_NULL (renderable->attachmentData);

	AttachmentData & ad = *renderable->attachmentData;
	CuiPersistentMessageManager::saveToDatapad (ad);
}

//----------------------------------------------------------------------

void SwgCuiPersistentMessageBrowser::handleDoubleClick (int index)
{
	const SwgCuiAttachmentList::Renderable * const r = m_attachments->get (index);
	if (!r)
	{
		WARNING (true, ("SwgCuiPersistentMessageBrowser createContextMenu bad"));
	}
}

//----------------------------------------------------------------------

UIPopupMenu * SwgCuiPersistentMessageBrowser::createContextMenu (int index)
{
	const SwgCuiAttachmentList::Renderable * renderable = 0;
	
	if (index >= 0)
	{
		renderable = m_attachments->get (index);
		if (!renderable)
		{
			WARNING (true, ("SwgCuiPersistentMessageBrowser createContextMenu bad"));
			return 0;
		}
	}

	UIPopupMenu * const pop = new UIPopupMenu(&getPage());
	pop->SetStyle (getPage ().FindPopupStyle ());
	pop->AddItem     (PopupItems::save,     CuiStringIdsPersistentMessage::popup_save.localize ());
	pop->AddItem     (PopupItems::save_all, CuiStringIdsPersistentMessage::popup_save_all.localize ());
	pop->SetVisible  (true);
	pop->AddCallback (this);
	pop->SetPropertyInteger (Properties::PopupChildIndex, index);

	return pop;
}

//----------------------------------------------------------------------

void SwgCuiPersistentMessageBrowser::onReceiveHeader    (const ChatPersistentMessageToClientData & messageData)
{
	appendMessageData (messageData);
}

//----------------------------------------------------------------------

void SwgCuiPersistentMessageBrowser::onReceiveBody      (const ChatPersistentMessageToClientData & data)
{
	updateMessageData (data);

	const ChatPersistentMessageToClientData * const selectedData = getLastSelectedData ();

	if (selectedData && selectedData->id == data.id)
	{
		updateBody ();
		updateAttachments ();
	}
}

//----------------------------------------------------------------------

void SwgCuiPersistentMessageBrowser::onEmailListUpdated(const bool &)
{
	update();
}

//----------------------------------------------------------------------

void SwgCuiPersistentMessageBrowser::update ()
{
	m_model->Attach (0);
	m_table->SetTableModel (0);
	
	m_model->ClearData ();
	
	CuiPersistentMessageManager::DataVector dv;
	CuiPersistentMessageManager::getDataVector (dv);
	
	for (CuiPersistentMessageManager::DataVector::const_iterator it = dv.begin (); it != dv.end (); ++it)
	{
		const CuiPersistentMessageManager::Data & messageData = *it;
		appendMessageData (messageData);
	}
	
	m_table->SetTableModel (m_model);
	m_model->Detach (0);

	updateBody ();
	updateAttachments ();
}

//----------------------------------------------------------------------

void SwgCuiPersistentMessageBrowser::updateBody ()
{
	const ChatPersistentMessageToClientData * const data = getLastSelectedData ();

	if (data)
	{
		if (data->isHeader)
		{
			m_textBody->SetLocalText (CuiStringIdsPersistentMessage::loading_msg.localize ());
			CuiPersistentMessageManager::requestMessageBody (data->id);
		}
		else
		{
			m_textBody->SetLocalText (data->message);
		}
	}
	else
	{
		m_textBody->Clear ();
	}
}

//----------------------------------------------------------------------

void SwgCuiPersistentMessageBrowser::updateAttachments ()
{
	const ChatPersistentMessageToClientData * const data = getLastSelectedData ();
	
	SwgCuiAttachmentList::Renderable::releaseIconsForPage (*m_volumeAttach);

	m_attachments->clear ();
	m_volumeAttach->Clear ();
	
	if (data && !data->isHeader)
	{
		const Attachments * const attachments = CuiPersistentMessageManager::getAttachments (data->id);
		
		if (attachments)
		{
			m_attachments->set (*attachments);
		}
	}

	if (m_attachments->empty ())
	{ 
		m_volumeAttach->SetVisible (false);
		return;
	}

	m_volumeAttach->SetVisible (true);

	const SwgCuiAttachmentList::RenderableVector & rv = m_attachments->get ();

	int index = 0;
	for (SwgCuiAttachmentList::RenderableVector::const_iterator it = rv.begin (); it != rv.end (); ++it)
	{
		const SwgCuiAttachmentList::Renderable * const r = *it;

		UIWidget * widget = r->fetchIcon (true);
		
		if (!widget)
		{
			widget = new UIPage;
		}

		if (widget)
		{
			widget->SetContextToParent (true);
			widget->SetPropertyInteger (UIBaseObject::PropertyName::Name, index++);
			m_volumeAttach->AddChild (widget);
		}
	}
}
		
//----------------------------------------------------------------------

const ChatPersistentMessageToClientData * SwgCuiPersistentMessageBrowser::getDataAtRow (const long visualRow) const
{
	const UIData * const data = m_model->GetCellDataVisual (visualRow, 0);
	
	if (data)
	{
		const int id = atoi (data->GetName ().c_str ());
		return CuiPersistentMessageManager::getData (id);
	}

	return 0;
}

//----------------------------------------------------------------------

const SwgCuiPersistentMessageBrowser::DataVector SwgCuiPersistentMessageBrowser::getSelectedData () const
{
	const UITable::LongVector & lv = m_table->GetSelectedRows ();

	DataVector dv;
	dv.reserve (lv.size ());

	for (UITable::LongVector::const_iterator it = lv.begin (); it != lv.end (); ++it)
	{
		const long rowVisual = *it;
		const ChatPersistentMessageToClientData * const data = getDataAtRow (rowVisual);
		NOT_NULL (data);
		dv.push_back (data);
	}

	return dv;
}

//----------------------------------------------------------------------

const ChatPersistentMessageToClientData * SwgCuiPersistentMessageBrowser::getLastSelectedData () const
{
	return getDataAtRow (m_table->GetLastSelectedRow ());
}

//----------------------------------------------------------------------

void SwgCuiPersistentMessageBrowser::reply (const StringId & subjectPrefix, const ChatAvatarId & recipient)
{
	if (!_stricmp (recipient.name.c_str (), "system") || (!recipient.name.empty () && recipient.name [0] == '@'))
	{
		CuiMessageBox::createInfoBox (CuiStringIdsPersistentMessage::err_reply_system.localize ());
		return;
	}

	const ChatPersistentMessageToClientData * const data = getLastSelectedData ();
	
	if (!data)
		return;

	const Unicode::String & prefix = subjectPrefix.localize ();
	
	Unicode::String subject;
	if (data->subject.compare (0, prefix.size (), prefix))
	{
		subject = prefix;
		subject.append (1, ' ');
	}
	
	subject += StringId::decodeString (data->subject);

	Unicode::String shortName;
	if (!recipient.name.empty ())
		CuiChatManager::getShortName (recipient, shortName);

	Unicode::String includedBody;
	constructIncludedBody (data->message, includedBody);
	
	Unicode::String includedAttachments;
	m_attachments->constructOob (includedAttachments);

	CuiPersistentMessageManager::startComposingNewMessage (Unicode::wideToNarrow (shortName), subject, includedBody, includedAttachments);
}

//----------------------------------------------------------------------

void SwgCuiPersistentMessageBrowser::deleteSelection ()
{
	const long selectedRowVisual = m_table->GetLastSelectedRow ();	
	const long lastScrollPosY    = m_table->GetScrollLocation ().y;

	const DataVector & dv = getSelectedData ();
	
	if (!dv.empty ())
	{
		for (DataVector::const_iterator it = dv.begin (); it != dv.end (); ++it)
		{
			const ChatPersistentMessageToClientData * const data = *it;
			NOT_NULL (data);
			
			if (data)
			{
				CuiPersistentMessageManager::deleteMessage (data->id);
			}
		}
		
		update ();
		
		m_table->SelectRow     (selectedRowVisual);
		m_table->ScrollToPoint (UIPoint (0L, lastScrollPosY));
		m_table->ScrollToRow   (selectedRowVisual);
	}
}

//----------------------------------------------------------------------

void SwgCuiPersistentMessageBrowser::updateMessageData (const MessageData & messageData)
{
	const long rowLogical = findLogicalRowForMessageDataLinear (messageData);

	if (rowLogical < 0)
		appendMessageData (messageData);
	else
	{
		const long rowVisual = m_model->GetVisualDataRowIndex (rowLogical);

		DEBUG_FATAL (rowVisual < 0, ("bad rowVisual"));

		Unicode::String statusString;
		statusString.append (1, messageData.status);
		m_model->SetValueAtText (rowVisual, 0, statusString);
	}
}

//----------------------------------------------------------------------

long SwgCuiPersistentMessageBrowser::findLogicalRowForMessageDataLinear (const MessageData & messageData) const
{
	const UIDataSource * const ds = m_model->GetColumnDataSource (0);
	const UIDataList & dataList = ds->GetData ();

	char buf [128];
	_itoa (messageData.id, buf, 10);

	const std::string nameToFind (buf);

	long count = 0;

	for (UIDataList::const_iterator it = dataList.begin (); it != dataList.end (); ++it, ++count)
	{
		const UIData * const data = *it;
		NOT_NULL (data);
		if (data->GetName () == nameToFind)
			return count;
	}

	return -1;
}

//----------------------------------------------------------------------

void SwgCuiPersistentMessageBrowser::appendMessageData (const MessageData & messageData)
{
	char buf [128];
	_itoa (messageData.id, buf, 10);

	const ChatAvatarId avatarId (messageData.fromGameCode, messageData.fromServerCode, messageData.fromCharacterName);

	Unicode::String statusString;
	statusString.append (1, messageData.status);

	m_model->AppendCell (0, buf, statusString);

	Unicode::String shortName;
	CuiChatManager::getShortName (avatarId, shortName);

	m_model->AppendCell (1, buf, shortName);
	m_model->AppendCell (2, 0,   messageData.subject);

	Unicode::String dateString;
	CuiUtils::FormatDate (dateString, messageData.timeStamp);
	UIData * const dateData = m_model->AppendCell (3, 0, dateString);
	NOT_NULL (dateData);
	
	snprintf (buf, 128, "%020d", messageData.timeStamp);
	dateData->SetPropertyNarrow (UITableModelDefault::DataProperties::SortValue, buf);
}

//----------------------------------------------------------------------

void SwgCuiPersistentMessageBrowser::update(float deltaTimeSecs)
{
	CuiMediator::update(deltaTimeSecs);

	// disable New/Reply/Forward button if squelched or in tutorial
	const bool isSquelched = Game::isPlayerSquelched();
	if (m_buttonNew)
		m_buttonNew->SetEnabled(!isSquelched && !Game::isTutorial());
	if (m_buttonReply)
		m_buttonReply->SetEnabled(!isSquelched && !Game::isTutorial());
	if (m_buttonForward)
		m_buttonForward->SetEnabled(!isSquelched && !Game::isTutorial());
}

//======================================================================
