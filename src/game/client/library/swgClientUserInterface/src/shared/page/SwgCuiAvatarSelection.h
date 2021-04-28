#ifndef SwgCuiAvatarSelection_H
#define SwgCuiAvatarSelection_H

//-----------------------------------------------------------------

#include "sharedMessageDispatch/Receiver.h"
#include "UIEventCallback.h"
#include "clientUserInterface/CuiMediator.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/NetworkId.h"

class CreatureObject;
class CuiMessageBox;
class CuiWidget3dObjectListViewer;
class UIButton;
class UICheckbox;
class UIDataSource;
class UIList;
class UIPage;
class UIText;
class UITable;
class CuiLoginManagerAvatarInfo;
class CuiLoginManagerClusterInfo;
class SwgCuiDeleteAvatarConfirmation;

namespace MessageDispatch
{
	class Callback;
}

//-----------------------------------------------------------------

/**
*
*/
class SwgCuiAvatarSelection :
public CuiMediator,
public UIEventCallback,
public MessageDispatch::Receiver
{
public:

	explicit                   SwgCuiAvatarSelection     (UIPage & page);

	//-- PS UI callbacks
	void                       OnButtonPressed           ( UIWidget *context );
	void                       OnGenericSelectionChanged ( UIWidget *context );
	void					   OnCheckboxSet             ( UIWidget *context );
	void					   OnCheckboxUnset           ( UIWidget *context );

	void                       receiveMessage            (const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message);

	void                       clearCharacterList        ();

	void                       onClusterConnection       (bool b);
	void                       onStartScene              (bool b);

	void                       onSceneChanged            (bool);
	void                       onAvatarListChanged       (bool b);
	void                       onClusterStatusChanged	 (bool b);
	void                       onDeleteAvatarConfirmation(CuiLoginManagerAvatarInfo const &info);

	void                       update                    (float deltaTimeSecs);

protected:

	void                       performActivate           ();
	void                       performDeactivate         ();

private:
	                          ~SwgCuiAvatarSelection      ();
	                           SwgCuiAvatarSelection      (const SwgCuiAvatarSelection &);
	SwgCuiAvatarSelection &    operator=                  (const SwgCuiAvatarSelection &);

	void                       refreshList                (bool updateSelection);

	void                       addAvatar                  (const CuiLoginManagerAvatarInfo & avatarInfo);

	void                       updateAvatarSelection      ();
	void                       requestAvatarSelection     ();
	void                       requestAvatarDeletion      ();
	bool                       getCurrentlySelectedAvatar (bool checkCluster = true);
	void                       performDelete              ();
	bool                       autoConnectOk              () const;

	void                       reconnectLoginServer       (bool forDelete);
	void                       handleCreate               ();

private:

	UIButton *                 m_okButton;
	UIButton *                 m_cancelButton;

	UIButton *                 m_createButton;
	UIButton *                 m_deleteButton;

	UIText *                   m_avatarNameText;

	UITable *                  m_table;

	CuiWidget3dObjectListViewer * m_objectViewer;

	CuiMessageBox *            m_messageBox;
	CuiMessageBox *            m_messageBoxDeleteWait;
	CuiMessageBox *            m_messageBoxLoginWait;

	bool                       m_waitingLoginForDelete;
	bool                       m_waitingLogin;
	bool                       m_waitingLoginForSelect;
	bool                       m_waitingLoginForCreate;

	bool                       m_autoConnected;

	bool                       m_proceed;

	MessageDispatch::Callback *       m_callback;

	CuiLoginManagerAvatarInfo *       m_selectedAvatar;
	bool                              m_waitingDeletion;
	CuiLoginManagerAvatarInfo *       m_deletingAvatar;

	int                               m_updateAvatar;

	bool                              m_waitingForConnection;
	bool                              m_dropFromCluster;
	uint32                            m_waitingForClusterId;

	float                             m_connectionTimeout;
	bool                              m_connectingToGame;

	bool                              m_avatarPopulateFirstTime;

	UIPage *                          m_deleteAvatarConfirmationPage;
	SwgCuiDeleteAvatarConfirmation *  m_deleteAvatarConfirmationMediator;

	UICheckbox *					  m_hideClosed;

	bool                              m_waitForConnectionRetry;
	bool                              m_hasAlreadyRetriedConnection;
};

//-----------------------------------------------------------------

#endif

//-----------------------------------------------------------------
