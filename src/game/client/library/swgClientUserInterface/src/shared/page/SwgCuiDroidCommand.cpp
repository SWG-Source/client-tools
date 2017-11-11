//===================================================================
//
// SwgCuiDroidCommand.cpp
// copyright 2004, sony online entertainment
// tford
//
//===================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiDroidCommand.h"

#include "clientGame/ContainerInterface.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/DroidProgramSizeManager.h"
#include "clientGame/Game.h"
#include "clientGame/ObjectAttributeManager.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiStringIdsDroidProgramming.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedGame/sharedObjectTemplate.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/DroidCommandProgrammingMessage.h"
#include "sharedObject/Container.h"
#include "sharedObject/Controller.h"
#include "sharedObject/VolumeContainer.h"
#include "sharedObject/NetworkIdManager.h"
#include "UIButton.h"
#include "UIData.h"
#include "UIDataSource.h"
#include "UIList.h"
#include "UIMessage.h"
#include "UIPage.h"
#include "UIText.h"

//===================================================================

namespace SwgCuiDroidCommandNamespace
{
	std::string const cms_droidCommandPre("droidcommand_");
	std::string const cms_droidProgramNameStringFile("space/droid_commands");
	std::string const cms_droidIntangibleCommandChipTemplateName("object/intangible/data_item/shared_droid_command.iff");
	std::string const cms_droidCommandAttributeName("droid_command_name");

	std::set<NetworkId> ms_droidProgrammingChipsInInventory;
	std::set<NetworkId> ms_droidProgrammingChipsInDroidDatapad;
	bool ms_dataBuilt = false;
	bool ms_updatingListSelections = false;
	std::vector<std::string> ms_commands;
	std::vector<NetworkId> ms_chipsToAdd;
	std::vector<NetworkId> ms_chipsToRemove;
	std::set<NetworkId> ms_chipsWithUnknownStatus;

	float ms_currentTimer = 0.0f;
	float ms_timeSinceRequest = 0.0f;
	float const ms_requestTimeIncrement = 0.5f;
	float const ms_maxTimer = 10.0f;

	Unicode::String const cms_newLine(Unicode::narrowToWide("\n"));

	void setCommandDescriptionIntoText(UIList const & uilist, int selectedRow, UIText & text);
	std::string getListItemName(UIList const & uilist, int row);
	bool commandNameExistsIn(UIList const & uilist, std::string const & name);
	bool isAChipInInventory(std::string const & listItemName);
	bool isAChipInDatapad(std::string const & listItemName);
	bool isAChipInInventoryOrDatapad(std::string const & listItemName);
	int getRowForCommandName(UIList const & uilist, std::string const & name);
	int getRowForListItemName(UIList const & uilist, std::string const & name);
	Unicode::String getLocalizedDroidCommandName(std::string const & commandName);
	std::string getDroidCommandNameFromListItemName(std::string const & listItemName);
	void setLocalizedListItemName(UIList & uilist, int row, Unicode::String const & text);
	ClientObject * getDroidDatapad(ClientObject & droidControlDevice);
}

using namespace SwgCuiDroidCommandNamespace;

//-------------------------------------------------------------------

void SwgCuiDroidCommandNamespace::setCommandDescriptionIntoText(UIList const & uilist, int const selectedRow, UIText & resultText)
{
	std::string const listItemName = getListItemName(uilist, selectedRow);
	std::string const droidCommandName = getDroidCommandNameFromListItemName(listItemName);
	StringId sid("cmd_d", droidCommandName);
	if(sid.isValid())
		resultText.SetLocalText(sid.localize());
	else
		resultText.Clear();
}

//----------------------------------------------------------------------

std::string SwgCuiDroidCommandNamespace::getListItemName(UIList const & uilist, int const row)
{
	UIData const * const data = uilist.GetDataAtRow(row);
	return data ? data->GetName() : "";
}

//----------------------------------------------------------------------

bool SwgCuiDroidCommandNamespace::commandNameExistsIn(UIList const & uilist, std::string const & nameToSearchFor)
{
	for(int i = 0; i < uilist.GetRowCount(); ++i)
	{
		UIData const * const data = uilist.GetDataAtRow(i);
		if(data)
		{
			std::string const droidCommandName = getDroidCommandNameFromListItemName(data->GetName());
			if(droidCommandName == nameToSearchFor)
				return true;
		}
	}
	return false;
}

//----------------------------------------------------------------------

int SwgCuiDroidCommandNamespace::getRowForCommandName(UIList const & uilist, std::string const & nameToSearchFor)
{
	for(int i = 0; i < uilist.GetRowCount(); ++i)
	{
		UIData const * const data = uilist.GetDataAtRow(i);
		if(data)
		{
			std::string const droidCommandName = getDroidCommandNameFromListItemName(data->GetName());
			if(droidCommandName == nameToSearchFor)
				return i;
		}
	}
	return -1;
}


//----------------------------------------------------------------------

int SwgCuiDroidCommandNamespace::getRowForListItemName(UIList const & uilist, std::string const & nameToSearchFor)
{
	for(int i = 0; i < uilist.GetRowCount(); ++i)
	{
		UIData const * const data = uilist.GetDataAtRow(i);
		if(data)
		{
			if(data->GetName() == nameToSearchFor)
				return i;
		}
	}
	return -1;
}

//----------------------------------------------------------------------

bool SwgCuiDroidCommandNamespace::isAChipInInventory(std::string const & listItemName)
{
	NetworkId nid(listItemName);
	Object const * const o = NetworkIdManager::getObjectById(nid);
	ClientObject const * const co = o ? o->asClientObject() : NULL;
	if(co)
	{
		if(co->getGameObjectType() == SharedObjectTemplate::GOT_misc_droid_programming_chip)
			return true;
	}
	return false;
}

//----------------------------------------------------------------------

bool SwgCuiDroidCommandNamespace::isAChipInDatapad(std::string const & listItemName)
{
	NetworkId nid(listItemName);
	Object const * const o = NetworkIdManager::getObjectById(nid);
	ClientObject const * const co = o ? o->asClientObject() : NULL;
	if(co)
	{
		if(co->getTemplateName() == cms_droidIntangibleCommandChipTemplateName)
			return true;
	}
	return false;
}

//----------------------------------------------------------------------

bool SwgCuiDroidCommandNamespace::isAChipInInventoryOrDatapad(std::string const & listItemName)
{
	return isAChipInInventory(listItemName) || isAChipInDatapad(listItemName);
}

//----------------------------------------------------------------------

Unicode::String SwgCuiDroidCommandNamespace::getLocalizedDroidCommandName(std::string const & commandName)
{
	StringId sid(cms_droidProgramNameStringFile, commandName);
	return sid.localize();
}

//----------------------------------------------------------------------

std::string SwgCuiDroidCommandNamespace::getDroidCommandNameFromListItemName(std::string const & listItemName)
{
	NetworkId nid(listItemName);
	if(!nid.isValid())
		return listItemName;
	else
	{
		ObjectAttributeManager::AttributeVector attribs;
		IGNORE_RETURN(ObjectAttributeManager::getAttributes(nid, attribs));
		for(ObjectAttributeManager::AttributeVector::const_iterator it = attribs.begin(); it != attribs.end(); ++it)
		{
			ObjectAttributeManager::AttributePair const attrib = *it;
			if(attrib.first == cms_droidCommandAttributeName)
			{
				return Unicode::wideToNarrow(attrib.second);
			}
		}
	}
	return "";
}

//----------------------------------------------------------------------

void SwgCuiDroidCommandNamespace::setLocalizedListItemName(UIList & uilist, int row, Unicode::String const & name)
{
	IGNORE_RETURN(uilist.SetLocalText(row, name));
}

//----------------------------------------------------------------------

ClientObject * SwgCuiDroidCommandNamespace::getDroidDatapad(ClientObject & droidControlDevice)
{
	return ContainerInterface::getObjectInSlot(droidControlDevice, "datapad");
}

//======================================================================

SwgCuiDroidCommand::SwgCuiDroidCommand(UIPage & page)
: UIEventCallback(), CuiMediator("SwgCuiDroidCommand", page),
	m_callBack(new MessageDispatch::Callback),
	m_listLeft(NULL),
	m_listRight(NULL),
	m_buttonLoad(NULL),
	m_buttonUnload(NULL),
	m_buttonCancel(NULL),
	m_buttonCommit(NULL),
	m_textLeft(NULL),
	m_textRight(NULL),
	m_droidControlDevice(),
	m_pageVolumeHolster(NULL),
	m_pageVolumeBar(NULL),
	m_pageVolumeBarDelta(NULL)
{
	getCodeDataObject(TUIList, m_listLeft, "listLeft");
	getCodeDataObject(TUIList, m_listRight, "listRight");
	getCodeDataObject(TUIButton, m_buttonLoad, "buttonLoad");
	getCodeDataObject(TUIButton, m_buttonUnload, "buttonUnload");
	getCodeDataObject(TUIButton, m_buttonCancel, "buttonCancel");
	getCodeDataObject(TUIButton, m_buttonCommit, "buttonOk");
	getCodeDataObject(TUIText, m_textLeft, "textLeft");
	getCodeDataObject(TUIText, m_textRight, "textRight");
	getCodeDataObject(TUIPage, m_pageVolumeHolster, "volumeHolster");
	getCodeDataObject(TUIPage, m_pageVolumeBar, "volumeBar");
	getCodeDataObject(TUIPage, m_pageVolumeBarDelta, "volumeBarDelta");

	registerMediatorObject(*m_buttonLoad, true);
	registerMediatorObject(*m_buttonUnload, true);
	registerMediatorObject(*m_buttonCancel, true);
	registerMediatorObject(*m_buttonCommit, true);

	m_textLeft->SetPreLocalized(true);
	m_textRight->SetPreLocalized(true);
	m_listLeft->SetRenderSeperatorLines(false);
	m_listRight->SetRenderSeperatorLines(false);

	//hide volume bars until their size is calculated
	m_pageVolumeBar->SetHeight (0);
	m_pageVolumeBarDelta->SetHeight (0);

	m_buttonLoad->SetEnabled(false);
	m_buttonUnload->SetEnabled(false);

	clearData();

	setShowFocusedGlowRect(false);

	buildDroidProgrammingChipsInInventory();
	for(std::set<NetworkId>::const_iterator i = ms_droidProgrammingChipsInInventory.begin(); i != ms_droidProgrammingChipsInInventory.end(); ++i)
	{
		ObjectAttributeManager::requestUpdate(*i, true);
	}

	buildDroidProgrammingChipsInDroidDatapad();
	for(std::set<NetworkId>::const_iterator j = ms_droidProgrammingChipsInDroidDatapad.begin(); j != ms_droidProgrammingChipsInDroidDatapad.end(); ++j)
	{
		ObjectAttributeManager::requestUpdate(*j, true);
	}

	ms_commands.clear();
	ms_chipsToAdd.clear();
	ms_chipsToRemove.clear();

	setState(MS_closeable);
	setState(MS_closeDeactivates);
}

//----------------------------------------------------------------------

SwgCuiDroidCommand::~SwgCuiDroidCommand()
{
	delete m_callBack;
	m_callBack = NULL;

	m_listLeft = NULL;
	m_listRight = NULL;
	m_buttonLoad = NULL;
	m_buttonUnload = NULL;
	m_buttonCancel = NULL;
	m_buttonCommit = NULL;
	m_textLeft = NULL;
	m_textRight = NULL;
	m_droidControlDevice = NULL;
	m_pageVolumeHolster = NULL;
	m_pageVolumeBar = NULL;
	m_pageVolumeBarDelta = NULL;

	ms_droidProgrammingChipsInInventory.clear();
	ms_dataBuilt = false;
	ms_updatingListSelections = false;
}

//----------------------------------------------------------------------

void SwgCuiDroidCommand::performActivate()
{
	CuiMediator::performActivate();
	CuiManager::requestPointer(true);

	m_listLeft->AddCallback(this);
	m_listRight->AddCallback(this);

	if(!ms_dataBuilt)
		buildData();

	buildDroidProgrammingChipsInInventory();
	for(std::set<NetworkId>::const_iterator i = ms_droidProgrammingChipsInInventory.begin(); i != ms_droidProgrammingChipsInInventory.end(); ++i)
	{
		ObjectAttributeManager::requestUpdate(*i, true);
	}

	buildDroidProgrammingChipsInDroidDatapad();
	for(std::set<NetworkId>::const_iterator j = ms_droidProgrammingChipsInDroidDatapad.begin(); j != ms_droidProgrammingChipsInDroidDatapad.end(); ++j)
	{
		ObjectAttributeManager::requestUpdate(*j, true);
	}

	ms_currentTimer = 0.0f;

	setIsUpdating(true);

	IGNORE_RETURN(recalculateVolumes(NULL, ""));

	m_callBack->connect (*this, &SwgCuiDroidCommand::onObjectAddedToContainer, static_cast<ClientObject::Messages::AddedToContainer *>(0));
	m_callBack->connect (*this, &SwgCuiDroidCommand::onObjectRemovedFromContainer, static_cast<ClientObject::Messages::RemovedFromContainer *>(0));
}

//-------------------------------------------------------------------

void SwgCuiDroidCommand::performDeactivate()
{
	m_callBack->disconnect (*this, &SwgCuiDroidCommand::onObjectRemovedFromContainer, static_cast<ClientObject::Messages::RemovedFromContainer *>(0));
	m_callBack->disconnect (*this, &SwgCuiDroidCommand::onObjectAddedToContainer, static_cast<ClientObject::Messages::AddedToContainer *>(0));

	m_listRight->RemoveCallback(this);
	m_listLeft->RemoveCallback(this);
	setIsUpdating(false);
	CuiMediator::performDeactivate();
	CuiManager::requestPointer(false);
	clearData();
}

//-------------------------------------------------------------------

void SwgCuiDroidCommand::clearData()
{
	UIDataSource * const leftDS = m_listLeft->GetDataSource();
	if(leftDS)
		leftDS->Clear();

	UIDataSource * const rightDS = m_listRight->GetDataSource();
	if(rightDS)
		rightDS->Clear();

	m_textLeft->Clear();
	m_textRight->Clear();

	ms_dataBuilt = false;
}

//-------------------------------------------------------------------

void SwgCuiDroidCommand::buildData()
{
	buildDroidProgrammingChipsInInventory();
	buildDroidProgrammingChipsInDroidDatapad();
	addCommandsToLeftList();
	addChipsInInventoryToLeftList();
	addDroidCommandsToRightList();
	updateCommandStates();
	ms_dataBuilt = true;
}

//-------------------------------------------------------------------

void SwgCuiDroidCommand::rebuildData()
{
	clearData();
	buildData();
}

//-------------------------------------------------------------------

void SwgCuiDroidCommand::OnGenericSelectionChanged (UIWidget * const context)
{
	if(context == m_listLeft)
	{
		if(!ms_updatingListSelections)
		{
			int const selectedRow = m_listLeft->GetLastSelectedRow();
			setCommandDescriptionIntoText(*m_listLeft, selectedRow, *m_textLeft);
			std::string const listItemName = getListItemName(*m_listLeft, selectedRow);
			std::string const droidCommandName = getDroidCommandNameFromListItemName(listItemName);
			
			ms_updatingListSelections = true;
			m_listRight->RemoveRowSelection(m_listRight->GetLastSelectedRow());

			bool buttonLoadEnabled = true;

			//only allow loading if there's room
			if(recalculateVolumes(m_listLeft, droidCommandName))
			{
				buttonLoadEnabled = true;
			}
			else
				buttonLoadEnabled = false;

			//you cant' load a loaded command
			if(commandNameExistsIn(*m_listRight, droidCommandName))
				buttonLoadEnabled = false;

			m_buttonLoad->SetEnabled(buttonLoadEnabled);

			IGNORE_RETURN(recalculateVolumes(m_listLeft, droidCommandName));
			ms_updatingListSelections = false;
		}
		m_buttonUnload->SetEnabled(false);
	}
	else if(context == m_listRight)
	{
		if(!ms_updatingListSelections)
		{
			ms_updatingListSelections = true;
			m_listLeft->RemoveRowSelection(m_listLeft->GetLastSelectedRow());
			m_buttonUnload->SetEnabled(true);

			int const selectedRow = m_listRight->GetLastSelectedRow();
			setCommandDescriptionIntoText(*m_listRight, selectedRow, *m_textRight);
			std::string const listItemName = getListItemName(*m_listRight, selectedRow);
			std::string const droidCommandName = getDroidCommandNameFromListItemName(listItemName);
			IGNORE_RETURN(recalculateVolumes(m_listRight, droidCommandName));
			ms_updatingListSelections = false;
		}
		m_buttonLoad->SetEnabled(false);
	}
	else
	{
		if(m_listLeft->GetLastSelectedRow() == -1)
		{
			m_buttonLoad->SetEnabled(false);
		}

		if(m_listRight->GetLastSelectedRow() == -1)
		{
			m_buttonUnload->SetEnabled(false);
		}
	}
}

//-------------------------------------------------------------------

void SwgCuiDroidCommand::OnButtonPressed(UIWidget * const context)
{
	if(context == m_buttonLoad)
	{
		int const selectedLeftRow = m_listLeft->GetLastSelectedRow();
		std::string const listItemName = getListItemName(*m_listLeft, selectedLeftRow);
		std::string droidCommandName = getDroidCommandNameFromListItemName(listItemName);

		//if the command is already in the droid's command list, return
		if(commandNameExistsIn(*m_listRight, droidCommandName))
			return;

		//if it's a chip, adding it to the droid causes the chip to be destroyed, warn them
		if(isAChipInInventory(listItemName))
		{
			CuiMessageBox * const messageBox = CuiMessageBox::createYesNoBox (CuiStringIdsDroidProgramming::load_warning.localize());
			m_callBack->connect(messageBox->getTransceiverClosed(), *this, &SwgCuiDroidCommand::addChipCallback);
			return;
		}
		addSelectedLeftCommandToRightList();
	}
	else if(context == m_buttonUnload)
	{
		int const selectedRightRow = m_listRight->GetLastSelectedRow();
		std::string const listItemName = getListItemName(*m_listRight, selectedRightRow);
		std::string const droidCommandName = getDroidCommandNameFromListItemName(listItemName);
		CreatureObject const * const player = Game::getPlayerCreature();
		if(!player)
			return;
		//if the player doesn't have this command, they could only put it back in with a chip, warn them
		if(!player->hasCommand(droidCommandName))
		{
			CuiMessageBox * const messageBox = CuiMessageBox::createYesNoBox (CuiStringIdsDroidProgramming::unload_warning.localize());
			m_callBack->connect(messageBox->getTransceiverClosed(), *this, &SwgCuiDroidCommand::removeChipCallback);
			return;
		}
		removeSelectedRightCommandFromRightList();
	}
	else if(context == m_buttonCancel)
	{
		clearData();
		closeThroughWorkspace();
	}
	else if(context == m_buttonCommit)
	{
		sendDataToServer();
	}
}

//-------------------------------------------------------------------

void SwgCuiDroidCommand::update (float const deltaTimeSecs)
{
	if(!ms_chipsWithUnknownStatus.empty())
	{
		ms_currentTimer += deltaTimeSecs;
		ms_timeSinceRequest += deltaTimeSecs;

		if(ms_timeSinceRequest > ms_requestTimeIncrement)
		{
			ms_timeSinceRequest = 0.0f;
			//see if we have attributes for items we're waiting on
			ObjectAttributeManager::AttributeVector attribs;
			for(std::set<NetworkId>::iterator i = ms_chipsWithUnknownStatus.begin(); i != ms_chipsWithUnknownStatus.end(); /*iterated in loop*/)
			{
				bool incremented = false;
				Object const * const o = NetworkIdManager::getObjectById(*i);
				ClientObject const * const co = o ? o->asClientObject() : NULL;
				if(co)
				{
					attribs.clear();
					IGNORE_RETURN(ObjectAttributeManager::getAttributes(co->getNetworkId(), attribs));
					for(ObjectAttributeManager::AttributeVector::const_iterator it = attribs.begin(); it != attribs.end(); ++it)
					{
						ObjectAttributeManager::AttributePair const attrib = *it;
						if(attrib.first == cms_droidCommandAttributeName)
						{
							setLocalizedNameForChip(*i, getLocalizedDroidCommandName(Unicode::wideToNarrow(attrib.second)));
							std::set<NetworkId>::iterator eraseThis = i;
							++i;
							ms_chipsWithUnknownStatus.erase(eraseThis);
							updateCommandStates();
							incremented = true;
						}
					}
				}

				if(!incremented)
					++i;
			}

			//if enough time has passed, give up (especially with unprogrammed chips which don't have the attrib)
			if(ms_currentTimer >= ms_maxTimer)
			{
				for(std::set<NetworkId>::iterator i2 = ms_chipsWithUnknownStatus.begin(); i2 != ms_chipsWithUnknownStatus.end(); ++i2)
				{
					removeChipFromLists(*i2);
				}
				ms_chipsWithUnknownStatus.clear();
			}
		}
	}

	int const selectedLeftRow = m_listLeft->GetLastSelectedRow();
	m_buttonLoad->SetEnabled(selectedLeftRow != -1 ? true : false);
	int const selectedRightRow = m_listRight->GetLastSelectedRow();
	m_buttonUnload->SetEnabled(selectedRightRow != -1 ? true : false);
}

//-------------------------------------------------------------------

void SwgCuiDroidCommand::addSelectedLeftCommandToRightList()
{
	if (!recalculateVolumes(NULL, ""))
	{
		CuiMessageBox::createInfoBox (CuiStringIdsDroidProgramming::not_enough_space.localize());
		return;
	}

	//if this is a removed chip, remove it from the left
	int const selectedLeftRow = m_listLeft->GetLastSelectedRow();
	Unicode::String text;
	IGNORE_RETURN(m_listLeft->GetLocalText(selectedLeftRow, text));
	std::string const name = getListItemName(*m_listLeft, selectedLeftRow);
	size_t const pos = text.find(CuiStringIdsDroidProgramming::removed.localize());
	if(pos != text.npos) //lint !e737 signed/unsigned mismatch from npos bogosity
	{
		removeCommand(*m_listLeft, selectedLeftRow);
		IGNORE_RETURN(text.erase(pos));
	}

	addCommand(*m_listRight, text, name);
	updateCommandStates();
	IGNORE_RETURN(recalculateVolumes(NULL, ""));
}

//-------------------------------------------------------------------

void SwgCuiDroidCommand::removeSelectedRightCommandFromRightList()
{
	CreatureObject const * const player = Game::getPlayerCreature();
	if(!player)
		return;

	int const selectedRightRow = m_listRight->GetLastSelectedRow();
	std::string const listItemName = getListItemName(*m_listRight, selectedRightRow);
	std::string const droidCommandName = getDroidCommandNameFromListItemName(listItemName);

	if(isAChipInDatapad(listItemName) && !player->hasCommand(droidCommandName))
	{
		Unicode::String result = getLocalizedDroidCommandName(droidCommandName);
		result += CuiStringIdsDroidProgramming::removed.localize();
		addCommand(*m_listLeft, result, listItemName);
	}

	removeCommand(*m_listRight, selectedRightRow);
	updateCommandStates();
	IGNORE_RETURN(recalculateVolumes(NULL, ""));
}

//-------------------------------------------------------------------

void SwgCuiDroidCommand::updateCommandStates()
{
	for(int i = 0; i < m_listLeft->GetRowCount(); ++i)
	{
		Unicode::String text;
		IGNORE_RETURN(m_listLeft->GetLocalText(i, text));
		std::string const listItemName = getListItemName(*m_listLeft, i);
		std::string const droidCommandName = getDroidCommandNameFromListItemName(listItemName);
		if(commandNameExistsIn(*m_listRight, droidCommandName) && (!droidCommandName.empty()))
		{
			//do something to item on left
			if(text.find(CuiStringIdsDroidProgramming::loaded.localize()) == text.npos) //lint !e737 signed/unsigned mismatch from npos bogosity
				text += CuiStringIdsDroidProgramming::loaded.localize();
			IGNORE_RETURN(m_listLeft->SetLocalText(i, text));
		}
		else
		{
			size_t const pos = text.find(CuiStringIdsDroidProgramming::loaded.localize());
			if(pos != text.npos) //lint !e737 signed/unsigned mismatch from npos bogosity
			{
				IGNORE_RETURN(text.erase(pos));
				IGNORE_RETURN(m_listLeft->SetLocalText(i, text));
			}
		}
	}
}

//-------------------------------------------------------------------

void SwgCuiDroidCommand::addCommandsToLeftList()
{
	CreatureObject const * const player = Game::getPlayerCreature();
	if(!player)
		return;

	std::map<std::string, int> const & commands = player->getCommands();
	std::vector<Unicode::String> result;
	std::map<Unicode::String, std::string> commandNameMap;
	for(std::map<std::string, int>::const_iterator i = commands.begin(); i != commands.end(); ++i)
	{
		std::string const & command = (*i).first;
		if(command.find(cms_droidCommandPre) != command.npos) //lint !e737 signed/unsigned mismatch from npos bogosity
		{
			Unicode::String const localizedCommandName = getLocalizedDroidCommandName(command);
			result.push_back(localizedCommandName);
			commandNameMap[localizedCommandName] = command;
		}
	}

	std::sort(result.begin(), result.end());
	for(std::vector<Unicode::String>::const_iterator j = result.begin(); j != result.end(); ++j)
	{
		addCommand(*m_listLeft, *j, commandNameMap[*j]);
	}
}

//-------------------------------------------------------------------

void SwgCuiDroidCommand::addChipsInInventoryToLeftList()
{
	ObjectAttributeManager::AttributeVector attribs;
	for(std::set<NetworkId>::const_iterator i = ms_droidProgrammingChipsInInventory.begin(); i != ms_droidProgrammingChipsInInventory.end(); ++i)
	{
		bool addedItem = false;
		Object const * const o = NetworkIdManager::getObjectById(*i);
		ClientObject const * const co = o ? o->asClientObject() : NULL;
		if(co)
		{
			attribs.clear();
			IGNORE_RETURN(ObjectAttributeManager::getAttributes(co->getNetworkId(), attribs));
			for(ObjectAttributeManager::AttributeVector::const_iterator it = attribs.begin(); it != attribs.end(); ++it)
			{
				ObjectAttributeManager::AttributePair const attrib = *it;
				if(attrib.first == cms_droidCommandAttributeName)
				{
					addCommand(*m_listLeft, co->getLocalizedName(), i->getValueString());
					addedItem = true;
				}
			}
			if(!addedItem)
			{
				addCommand(*m_listLeft, CuiStringIdsDroidProgramming::querying_chip_status.localize(), i->getValueString());
				IGNORE_RETURN(ms_chipsWithUnknownStatus.insert(*i));
			}
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiDroidCommand::addDroidCommandsToRightList()
{
	ObjectAttributeManager::AttributeVector attribs;
	for(std::set<NetworkId>::const_iterator i = ms_droidProgrammingChipsInDroidDatapad.begin(); i != ms_droidProgrammingChipsInDroidDatapad.end(); ++i)
	{
		bool addedItem = false;
		Object * const object = NetworkIdManager::getObjectById(*i);
		ClientObject * const co = object ? object->asClientObject() : NULL;
		if(co)
		{
			if(isAChipInDatapad(co->getNetworkId().getValueString()))
			{
				attribs.clear();
				IGNORE_RETURN(ObjectAttributeManager::getAttributes(co->getNetworkId(), attribs));
				for(ObjectAttributeManager::AttributeVector::const_iterator it = attribs.begin(); it != attribs.end(); ++it)
				{
					ObjectAttributeManager::AttributePair const attrib = *it;
					if(attrib.first == cms_droidCommandAttributeName)
					{
						addCommand(*m_listRight, getLocalizedDroidCommandName(Unicode::wideToNarrow(attrib.second)), co->getNetworkId().getValueString());
						addedItem = true;
					}
				}
				if(!addedItem)
				{
					addCommand(*m_listRight, CuiStringIdsDroidProgramming::querying_chip_status.localize(), co->getNetworkId().getValueString());
					IGNORE_RETURN(ms_chipsWithUnknownStatus.insert(*i));
				}
			}
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiDroidCommand::setDroidControlDevice(ClientObject & droidControlDevice)
{
	m_droidControlDevice = &droidControlDevice;

	buildDroidProgrammingChipsInDroidDatapad();

	rebuildData();
	IGNORE_RETURN(recalculateVolumes(NULL, ""));

	for(std::set<NetworkId>::const_iterator j = ms_droidProgrammingChipsInDroidDatapad.begin(); j != ms_droidProgrammingChipsInDroidDatapad.end(); ++j)
	{
		ObjectAttributeManager::requestUpdate(*j, true);
	}
}

//----------------------------------------------------------------------

void SwgCuiDroidCommand::buildDroidProgrammingChipsInInventory()
{
	ms_droidProgrammingChipsInInventory.clear();
	ClientObject const * const player = Game::getPlayerCreature();
	if(player)
		buildDroidProgrammingChipsInInventoryRecursive(*player);
}

//----------------------------------------------------------------------

void SwgCuiDroidCommand::buildDroidProgrammingChipsInInventoryRecursive(ClientObject const & containerObject)
{
	CreatureObject const * const player = Game::getPlayerCreature();
	if(!player)
		return;

	//don't look in the player's bank
	if(&containerObject == player->getBankObject())
		return;

	//don't look in crafting tools
	if(containerObject.getGameObjectType() == SharedObjectTemplate::GOT_tool_crafting)
		return;

	Container const * const container = ContainerInterface::getContainer(containerObject);
	{
		if(container)
		{
			for(ContainerConstIterator i = container->begin(); i != container->end(); ++i)
			{
				Object * const object = (*i).getObject();
				ClientObject * const co = object ? object->asClientObject() : NULL;
				if(co)
				{
					buildDroidProgrammingChipsInInventoryRecursive(*co);
					if(co->getGameObjectType() == SharedObjectTemplate::GOT_misc_droid_programming_chip)
					{
						IGNORE_RETURN(ms_droidProgrammingChipsInInventory.insert(co->getNetworkId()));
					}
				}
			}
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiDroidCommand::buildDroidProgrammingChipsInDroidDatapad() const
{
	if(!m_droidControlDevice)
		return;

	ClientObject * const droidControlDeviceDatapad = getDroidDatapad(*m_droidControlDevice);
	if(!droidControlDeviceDatapad)
		return;

	Container const * const container = ContainerInterface::getContainer(*droidControlDeviceDatapad);
	if(!container)
		return;

	ObjectAttributeManager::AttributeVector attribs;

	ms_droidProgrammingChipsInDroidDatapad.clear();

	for(ContainerConstIterator i = container->begin(); i != container->end(); ++i)
	{
		Object * const object = (*i).getObject();
		ClientObject * const co = object ? object->asClientObject() : NULL;
		if(co)
		{
			if(isAChipInDatapad(co->getNetworkId().getValueString()))
			{
				IGNORE_RETURN(ms_droidProgrammingChipsInDroidDatapad.insert(co->getNetworkId()));
			}
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiDroidCommand::addChipCallback (CuiMessageBox const & box)
{
	if(box.completedAffirmative())
	{
		addSelectedLeftCommandToRightList();
	}
}

//----------------------------------------------------------------------

void SwgCuiDroidCommand::removeChipCallback (CuiMessageBox const & box)
{
	if(box.completedAffirmative())
	{
		removeSelectedRightCommandFromRightList();
	}
}

//----------------------------------------------------------------------

void SwgCuiDroidCommand::sendDataToServerChipCallback (CuiMessageBox const & box)
{
	if(box.completedAffirmative())
	{
		CreatureObject * const player = Game::getPlayerCreature();
		if(player)
		{
			Controller * const controller = player->getController();
			if(controller)
			{
				DroidCommandProgrammingMessage * const msg = new DroidCommandProgrammingMessage;
				msg->setCommands(ms_commands);
				msg->setChipsToAdd(ms_chipsToAdd);
				msg->setChipsToRemove(ms_chipsToRemove);
				if(m_droidControlDevice.getPointer())
				{
					msg->setDroidControlDevice(m_droidControlDevice->getNetworkId());
				}
				controller->appendMessage (CM_droidCommandProgramming, 0.0f, msg, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_SERVER);

				clearData();
				closeThroughWorkspace();
			}
		}
	}

	ms_commands.clear();
	ms_chipsToAdd.clear();
	ms_chipsToRemove.clear();
}

//----------------------------------------------------------------------

void SwgCuiDroidCommand::sendDataToServer()
{
	ms_commands.clear();
	ms_chipsToAdd.clear();
	ms_chipsToRemove.clear();

	for(int i = 0; i < m_listRight->GetRowCount(); ++i)
	{
		UIData const * const data = m_listRight->GetDataAtRow(i);
		if(data)
		{
			//name is either a networkId or a "droidcommand_" prefixed command
			std::string const listItemName = data->GetName();
			if(isAChipInInventory(listItemName))
			{
				ms_chipsToAdd.push_back(NetworkId(listItemName));
			}
			else if(!isAChipInInventoryOrDatapad(listItemName))
			{
				ms_commands.push_back(listItemName);
			}
		}
	}

	for(std::set<NetworkId>::const_iterator it = ms_droidProgrammingChipsInDroidDatapad.begin(); it != ms_droidProgrammingChipsInDroidDatapad.end(); ++it)
	{
		if(isChipCurrentlyInDroid(*it) && !chipWillBeInDroid(*it))
			ms_chipsToRemove.push_back(*it);
	}

	if(ms_commands.empty() && ms_chipsToAdd.empty() && ms_chipsToRemove.empty())
	{
		IGNORE_RETURN(CuiMessageBox::createInfoBox (CuiStringIdsDroidProgramming::no_changes.localize()));
		closeThroughWorkspace();
		return;
	}

	Unicode::String result;
	result = CuiStringIdsDroidProgramming::changes_message_pre.localize();

	bool commandBreakShown = false;
	for(std::vector<std::string>::const_iterator itr = ms_commands.begin(); itr != ms_commands.end(); ++itr)
	{
		if(!commandBreakShown)
		{
			result += cms_newLine;
			commandBreakShown = true;
		}
		result += cms_newLine;
		result += CuiStringIdsDroidProgramming::command_added.localize() + getLocalizedDroidCommandName(*itr);
	}

	bool chipAddBreakShown = false;
	std::vector<NetworkId>::const_iterator it2;
	for(it2 = ms_chipsToAdd.begin(); it2 != ms_chipsToAdd.end(); ++it2)
	{
		if(!chipAddBreakShown)
		{
			result += cms_newLine;
			chipAddBreakShown = true;
		}
		std::string const droidCommandName = getDroidCommandNameFromListItemName(it2->getValueString());
		result += cms_newLine;
		result += CuiStringIdsDroidProgramming::chip_added.localize() + getLocalizedDroidCommandName(droidCommandName);
	}
	
	bool chipRemoveBreakShown = false;
	for(it2 = ms_chipsToRemove.begin(); it2 != ms_chipsToRemove.end(); ++it2)
	{
		if(!chipRemoveBreakShown)
		{
			result += cms_newLine;
			chipRemoveBreakShown = true;
		}
		std::string const droidCommandName = getDroidCommandNameFromListItemName(it2->getValueString());
		result += cms_newLine;
		result += CuiStringIdsDroidProgramming::chip_removed.localize() + getLocalizedDroidCommandName(droidCommandName);
	}

	result += cms_newLine;
	result += cms_newLine;
	result += CuiStringIdsDroidProgramming::changes_message_post.localize();

	CuiMessageBox * const messageBox = CuiMessageBox::createYesNoBox (result);
	m_callBack->connect(messageBox->getTransceiverClosed(), *this, &SwgCuiDroidCommand::sendDataToServerChipCallback);
}

//----------------------------------------------------------------------

bool SwgCuiDroidCommand::isChipCurrentlyInDroid(NetworkId const & chip) const
{
	if(!m_droidControlDevice)
		return false;

	ClientObject * const droidControlDeviceDatapad = getDroidDatapad(*m_droidControlDevice);
	if(!droidControlDeviceDatapad)
		return false;

	Container const * const container = ContainerInterface::getContainer(*droidControlDeviceDatapad);
	if(!container)
		return false;

	ObjectAttributeManager::AttributeVector attribs;

	ms_droidProgrammingChipsInDroidDatapad.clear();

	for(ContainerConstIterator i = container->begin(); i != container->end(); ++i)
	{
		Object * const object = (*i).getObject();
		if(object && object->getNetworkId() == chip)
			return true;
	}
	return false;
}

//----------------------------------------------------------------------

bool SwgCuiDroidCommand::chipWillBeInDroid(NetworkId const & chip)
{
	for(int i = 0; i < m_listRight->GetRowCount(); ++i)
	{
		UIData const * const data = m_listRight->GetDataAtRow(i);
		if(data)
		{
			NetworkId nid(data->GetName());
			if(nid == chip)
				return true;
		}
	}
	return false;
}

//----------------------------------------------------------------------

bool SwgCuiDroidCommand::recalculateVolumes(UIList const * const listCurrentlySelected, std::string const & commandNameCurrentlySelected)
{
	if(!m_listLeft)
		return false;
	if(!m_listRight)
		return false;

	if(!m_droidControlDevice)
		return false;

	ClientObject * const droidControlDeviceDatapad = getDroidDatapad(*m_droidControlDevice);
	if(!droidControlDeviceDatapad)
		return false;

	VolumeContainer const * const container = ContainerInterface::getVolumeContainer(*droidControlDeviceDatapad);
	if(!container)
		return false;

	int const droidMaxVolume = container->getTotalVolume();
	int usedVolume = 0;
	int deltaVolume = 0;
	bool hasEnoughSpaceForSelection = true;
	for(int i = 0; i < m_listRight->GetRowCount(); ++i)
	{
		UIData const * const data = m_listRight->GetDataAtRow(i);
		if(data)
		{
			std::string const droidCommandName = getDroidCommandNameFromListItemName(data->GetName());
			int const programSize = DroidProgramSizeManager::getDroidProgramSize(droidCommandName);

			if((listCurrentlySelected == m_listRight) && (commandNameCurrentlySelected == droidCommandName))
			{
				deltaVolume = programSize;
			}
			else
			{
				usedVolume += programSize;
			}
		}
	}

	if(listCurrentlySelected == m_listLeft)
	{
		UIData const * const data = m_listLeft->GetDataAtRow(m_listLeft->GetLastSelectedRow());
		if(data)
		{
			std::string const droidCommandName = getDroidCommandNameFromListItemName(data->GetName());
			int const programSize = DroidProgramSizeManager::getDroidProgramSize(droidCommandName);
			if(commandNameExistsIn(*m_listLeft, droidCommandName) && !commandNameExistsIn(*m_listRight, droidCommandName))
			{
				deltaVolume = programSize;
			}
		}
	}	

	usedVolume = std::max(0, usedVolume);

	if(usedVolume + deltaVolume > droidMaxVolume)
	{
		hasEnoughSpaceForSelection = false;
	}

	if(droidMaxVolume > 0)
	{
		float const ratioBar = static_cast<float>(usedVolume) / static_cast<float>(droidMaxVolume);
		long const holsterSize = m_pageVolumeHolster->GetHeight ();
		m_pageVolumeBar->SetHeight (static_cast<long>(holsterSize * ratioBar));
		m_pageVolumeBar->SetLocation(m_pageVolumeBar->GetLocation().x, static_cast<long>(holsterSize * (1- ratioBar)));
		float const ratioBarDelta = static_cast<float>(deltaVolume) / static_cast<float>(droidMaxVolume);
		m_pageVolumeBarDelta->SetHeight (static_cast<long>(holsterSize * ratioBarDelta));
		int const newDeltaBarX = m_pageVolumeBarDelta->GetLocation().x;
		int const newDeltaBarY = m_pageVolumeBar->GetLocation().y - m_pageVolumeBarDelta->GetSize().y;
		m_pageVolumeBarDelta->SetLocation(newDeltaBarX, newDeltaBarY);
	}

	return hasEnoughSpaceForSelection;
}

//----------------------------------------------------------------------

bool SwgCuiDroidCommand::OnMessage (UIWidget * const context, UIMessage const & msg)
{
	if (msg.Type == UIMessage::LeftMouseDoubleClick)
	{
		if (context == m_listLeft)
		{
			int const selectedRow = m_listLeft->GetLastSelectedRow();
			std::string const listItemName = getListItemName(*m_listLeft, selectedRow);
			std::string const droidCommandName = getDroidCommandNameFromListItemName(listItemName);
			if (!recalculateVolumes(m_listLeft, droidCommandName))
				CuiMessageBox::createInfoBox (CuiStringIdsDroidProgramming::not_enough_space.localize());
			else
				m_buttonLoad->Press();
			return false;
		}
		else if (context == m_listRight)
		{
			m_buttonUnload->Press();
			return false;
		}
	}

	return true;
}

//----------------------------------------------------------------------

void SwgCuiDroidCommand::setLocalizedNameForChip(NetworkId const & chip, Unicode::String const & text)
{
	int const leftRow = getRowForListItemName(*m_listLeft, chip.getValueString());
	int const rightRow = getRowForListItemName(*m_listRight, chip.getValueString());

	if(leftRow != -1)
	{
		setLocalizedListItemName(*m_listLeft, leftRow, text);
	}
	if(rightRow != -1)
	{
		setLocalizedListItemName(*m_listRight, rightRow, text);
	}
}

//----------------------------------------------------------------------

void SwgCuiDroidCommand::removeChipFromLists(NetworkId const & chip)
{
	int const leftRow = getRowForListItemName(*m_listLeft, chip.getValueString());
	int const rightRow = getRowForListItemName(*m_listRight, chip.getValueString());

	if(leftRow != -1)
	{
		removeCommand(*m_listLeft, leftRow);
		updateCommandStates();
		IGNORE_RETURN(recalculateVolumes(NULL, ""));
	}
	if(rightRow != -1)
	{
		removeCommand(*m_listRight, rightRow);
		updateCommandStates();
		IGNORE_RETURN(recalculateVolumes(NULL, ""));
	}
}

//----------------------------------------------------------------------

/**
Capture object movements between containers (includes creates into and deletes from).
If an object moves into the droid datapad, rebuild this UI
*/
void SwgCuiDroidCommand::onObjectAddedToContainer(const ClientObject::Messages::ContainerMsg & payload)
{
	if(!m_droidControlDevice)
		return;

	ClientObject const * const container = payload.first;
	ClientObject * const droidControlDeviceDatapad = getDroidDatapad(*m_droidControlDevice);
	if(!droidControlDeviceDatapad)
		return;

	if (container == droidControlDeviceDatapad)
		rebuildData();
}


//----------------------------------------------------------------------

/**
Capture object movements between containers (includes creates into and deletes from).
If an object leaves the droid datapad, rebuild this UI
*/
void SwgCuiDroidCommand::onObjectRemovedFromContainer(const ClientObject::Messages::ContainerMsg & payload)
{
	if(!m_droidControlDevice)
		return;

	ClientObject const * const container = payload.first;
	ClientObject * const droidControlDeviceDatapad = getDroidDatapad(*m_droidControlDevice);
	if(!droidControlDeviceDatapad)
		return;

	if (container == droidControlDeviceDatapad)
		rebuildData();
}


//----------------------------------------------------------------------

void SwgCuiDroidCommand::addCommand(UIList & uilist, Unicode::String const & text, std::string const & name)
{
	uilist.AddRow(text, name);
	OnGenericSelectionChanged(&uilist);
}

//----------------------------------------------------------------------

void SwgCuiDroidCommand::removeCommand(UIList & uilist, int const row)
{
	IGNORE_RETURN(uilist.RemoveRow(row));
	OnGenericSelectionChanged(&uilist);
}

//======================================================================
