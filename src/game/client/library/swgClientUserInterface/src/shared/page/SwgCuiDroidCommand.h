//===================================================================
//
// SwgCuiDroidCommand.h
// copyright 2004, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_SwgCuiDroidCommand_H
#define INCLUDED_SwgCuiDroidCommand_H

//===================================================================

#include "UIEventCallback.h"
#include "clientGame/ClientObject.h"
#include "clientUserInterface/CuiMediator.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Watcher.h"

//===================================================================

class ClientObject;
class CuiMessageBox;
class UIButton;
class UIList;
class UIPage;
class UIText;

//-----------------------------------------------------------------

namespace MessageDispatch
{
	class Callback;
}

//===================================================================

class SwgCuiDroidCommand
: public UIEventCallback
, public CuiMediator
{
public:
	explicit SwgCuiDroidCommand(UIPage & page);

	virtual void OnGenericSelectionChanged (UIWidget * context);

	void setDroidControlDevice(ClientObject & droidControlDevice);
	void OnButtonPressed(UIWidget * context);
	bool OnMessage (UIWidget * context, UIMessage const & msg);
	virtual void update (float deltaTimeSecs);
	void onObjectAddedToContainer(const ClientObject::Messages::ContainerMsg & payload);
	void onObjectRemovedFromContainer(const ClientObject::Messages::ContainerMsg & payload);

protected:
	virtual void performActivate();
	virtual void performDeactivate();

private:
	SwgCuiDroidCommand();
	SwgCuiDroidCommand(SwgCuiDroidCommand const &);
	SwgCuiDroidCommand & operator=(SwgCuiDroidCommand const &);
	virtual ~SwgCuiDroidCommand();

private:
	void clearData();
	void buildData();
	void rebuildData();
	void buildDroidProgrammingChipsInInventory();
	void buildDroidProgrammingChipsInInventoryRecursive(ClientObject const & containerObject);
	void buildDroidProgrammingChipsInDroidDatapad() const;
	void addCommandsToLeftList();
	void addChipsInInventoryToLeftList();
	void addDroidCommandsToRightList();
	void updateCommandStates();
	void addChipCallback (CuiMessageBox const & box);
	void removeChipCallback (CuiMessageBox const & box);
	void sendDataToServerChipCallback (CuiMessageBox const & box);
	void addSelectedLeftCommandToRightList();
	void removeSelectedRightCommandFromRightList();
	void sendDataToServer();
	bool isChipCurrentlyInDroid(NetworkId const & chip) const;
	bool chipWillBeInDroid(NetworkId const & chip);
	bool recalculateVolumes(UIList const * listCurrentlySelected, std::string const & commandNameCurrentlySelected);
	void setLocalizedNameForChip(NetworkId const & chip, Unicode::String const & text);
	void removeChipFromLists(NetworkId const & chip);
	void addCommand(UIList & uilist, Unicode::String const & text, std::string const & name);
	void removeCommand(UIList & uilist, int row);

private:
	MessageDispatch::Callback * m_callBack;
	UIList * m_listLeft;
	UIList * m_listRight;
	UIButton * m_buttonLoad;
	UIButton * m_buttonUnload;
	UIButton * m_buttonCancel;
	UIButton * m_buttonCommit;
	UIText * m_textLeft;
	UIText * m_textRight;
	Watcher<ClientObject> m_droidControlDevice;
	UIPage * m_pageVolumeHolster;
	UIPage * m_pageVolumeBar;
	UIPage * m_pageVolumeBarDelta;
};

//===================================================================

#endif
