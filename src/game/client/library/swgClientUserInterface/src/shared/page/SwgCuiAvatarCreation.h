#ifndef SwgCuiAvatarCreation_H
#define SwgCuiAvatarCreation_H

#include "UIEventCallback.h"
#include "clientUserInterface/CuiMediator.h"

class CreatureObject;
class CuiMessageBox;
class CuiWidget3dObjectListViewer;
class GameCamera;
class UIButton;
class UICheckbox;
class UIList;
class UIPage;
class UIText;

//-----------------------------------------------------------------

class SwgCuiAvatarCreation :
public CuiMediator,
public UIEventCallback
{
public:
	explicit                   SwgCuiAvatarCreation      (UIPage & page);

	//-- PS UI callbacks
	virtual void               OnButtonPressed           ( UIWidget *context );
	virtual void               OnGenericSelectionChanged ( UIWidget *context );

	virtual void               update                    (float);

protected:
	virtual void               performActivate   ();
	virtual void               performDeactivate ();

private:
	void                       updateAvatarSelection       ();
	CreatureObject *           createPlayerModel           (const std::string & templateName);
	bool                       areAllDetailLevelsAvailable ();
	void                       loadAllPlayerModels         ();

	typedef stdmap<UINarrowString, CreatureObject *>::fwd AvatarMap_t;

	virtual                   ~SwgCuiAvatarCreation ();
	                           SwgCuiAvatarCreation ();
	                           SwgCuiAvatarCreation (const SwgCuiAvatarCreation &);
	SwgCuiAvatarCreation &     operator=          (const SwgCuiAvatarCreation &);

	bool                       autoConnectOk () const;

	UIButton *                     m_okButton;
	UIButton *                     m_cancelButton;
	UIList *                       m_avatarList;
	UIText *                       m_infoText;
	UIText *                       m_avatarNameText;
	CreatureObject *               m_avatar;
	CuiWidget3dObjectListViewer *  m_objectViewer;
	CuiWidget3dObjectListViewer *  m_bgViewer;
	AvatarMap_t *                  m_avatarMap;
	CuiMessageBox *                m_messageBox;
	UIButton *                     m_buttonGender;
	bool                           m_isLoading;

	enum Gender
	{
		G_Male,
		G_Female
	};

	Gender                         m_gender;

	bool                           m_autoConnected;

	bool                           m_purgePoolOnDeactivate;
};

//-----------------------------------------------------------------

#endif

//-----------------------------------------------------------------
