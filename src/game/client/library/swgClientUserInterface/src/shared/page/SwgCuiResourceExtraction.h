// ======================================================================
//
// SwgCuiResourceExtraction.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgCuiResourceExtraction_H
#define INCLUDED_SwgCuiResourceExtraction_H

// ======================================================================

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/NetworkId.h"
#include "sharedMessageDispatch/Receiver.h"

#include "clientUserInterface/CuiMediator.h"

#include "UIEventCallback.h"

class ClientInstallationSynchronizedUi;
class CuiWidget3dObjectListViewer;
class SwgCuiResourceExtraction_Hopper;
class SwgCuiResourceExtraction_Quantity;
class SwgCuiResourceExtraction_SetResource;
class Hopper;
class UIImage;
class UIButton;
class UIPage;
class UIPie;
class UIRunner;
class UIText;
class UITextbox;
class UIVolumePage;

//-----------------------------------------------------------------

namespace MessageDispatch
{
	class Callback;
}

//-----------------------------------------------------------------

class SwgCuiResourceExtraction : public CuiMediator, public UIEventCallback, public MessageDispatch::Receiver
{
public:
	explicit                          SwgCuiResourceExtraction (UIPage & page);
	virtual void                      receiveMessage           (const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message);
	virtual void                      OnButtonPressed          (UIWidget *context);
	void                              toggleShowData           (bool showData);

public:
	static SwgCuiResourceExtraction * getInstance              ();

protected:
	virtual void                      performActivate          ();
	virtual void                      performDeactivate        ();

private:
//disabled
	                                 ~SwgCuiResourceExtraction ();
	                                  SwgCuiResourceExtraction ();
	                                  SwgCuiResourceExtraction (const SwgCuiResourceExtraction & rhs);
	SwgCuiResourceExtraction&         operator=                (const SwgCuiResourceExtraction & rhs);

private:
	void                              onChangeExtractionRate    (const ClientInstallationSynchronizedUi & sync);
	void                              onChangeExtractionRateMax (const ClientInstallationSynchronizedUi & sync);
	void                              onChangeActive            (const ClientInstallationSynchronizedUi & sync);
	void                              onChangeResourcePool      (const ClientInstallationSynchronizedUi & sync);
	void                              onChangeHopper            (const ClientInstallationSynchronizedUi & sync);
	void                              onChangeHopperMax         (const ClientInstallationSynchronizedUi & sync);
	void                              onUpdateResourcePoolData  (const int & data);
	void                              onChangeResourcePoolList  (const ClientInstallationSynchronizedUi & sync);
	void                              onChangeCondition         (const ClientInstallationSynchronizedUi & sync);
	void                              onDataReady               (const ClientInstallationSynchronizedUi & sync);
	void                              onChangeName              ();
	void                              update                    ();
	void                              updateResourceElement     (const NetworkId & id, bool available, const UIWidget * volumePage) const;
	void                              setActive                 (bool active, bool calledByServer);
	void                              updateCurrentResourceBar  ();
	void                              updateResourceList        (const ClientInstallationSynchronizedUi & sync) const;
	void                              showHopperPage            ();
	void                              updateActivateButtons     ();

private:
	MessageDispatch::Callback *       m_extractionCallback;

	//main page data
	UIPage   *                        m_window;
	UIButton *                        m_buttonTurnOn;
	UIButton *                        m_buttonTurnOff;
	UIButton *                        m_buttonChangeResource;
	UIButton *                        m_buttonManageHopper;
	UIText   *                        m_textTitleBar;
	UIText   *                        m_textExtractionRate;
	UIText   *                        m_textExtractionRateSpec;
	UIText   *                        m_textHopperPercent;
	UIText   *                        m_textIntactPercent;
	UIText   *                        m_currentResourceName;
	CuiWidget3dObjectListViewer *     m_currentResourceIcon;
	UIText   *                        m_currentResourceEfficiencyText;
	UIPage   *                        m_pageCurrentResourceVolume;
	UIPie    *                        m_damagePie;
	UIPie    *                        m_hopperPie;
	UIWidget *                        m_currentResourceBar;
	UIWidget *                        m_currentResourceBarBack;
	UIButton *                        m_buttonOk;
	UIRunner  *                       m_runnerL;
	UIRunner  *                       m_runnerR;

	SwgCuiResourceExtraction_Hopper *      m_mediatorHopper;
	SwgCuiResourceExtraction_SetResource * m_mediatorSetResource;

	bool                              m_hopperActive;
	bool                              m_harvesterActive;
};

// ======================================================================

#endif
