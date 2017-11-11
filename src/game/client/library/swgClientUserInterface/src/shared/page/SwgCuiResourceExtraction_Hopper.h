//======================================================================
//
// SwgCuiResourceExtraction_Hopper.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiResourceExtraction_Hopper_H
#define INCLUDED_SwgCuiResourceExtraction_Hopper_H

#include "UITableModel.h"

#include "sharedMessageDispatch/Receiver.h"

#include "clientUserInterface/CuiMediator.h"

#include "UIEventCallback.h"

class UIButton;
class UITable;
class UITableModelDefault;
class UIText;
class UIVolumePage;
class UIWidget;
class ClientInstallationSynchronizedUi;
class CuiWidget3dObjectListViewer;
class SwgCuiResourceExtraction_Quantity;

#include <vector>

//======================================================================

namespace MessageDispatch
{
	class Callback;
}

//======================================================================

class SwgCuiResourceExtraction_Hopper : public CuiMediator, public UIEventCallback
{
public:
	class TableModel;

	explicit                     SwgCuiResourceExtraction_Hopper (UIPage & page);
	virtual void                 OnButtonPressed           (UIWidget *context);
	void                         update                    ();
	NetworkId                    getResourceAtPosition     (int index);
	CuiWidget3dObjectListViewer* getResourceIconAtPosition (int index);
	UIPage*                      getResourceHopperAtPosition (int index);
	int                          getNumResources             () const;
	float                        getHopperSizeForResource  (const NetworkId& resource);
	void                         onEmptyCompleted (const bool & success);
	bool                         OnMessage(UIWidget *context, const UIMessage & msg);

protected:
	virtual void            performActivate    ();
	virtual void            performDeactivate  ();

private:
	                       ~SwgCuiResourceExtraction_Hopper             ();
	                        SwgCuiResourceExtraction_Hopper             ();
	                        SwgCuiResourceExtraction_Hopper             (const SwgCuiResourceExtraction_Hopper & rhs);
	                        SwgCuiResourceExtraction_Hopper & operator= (const SwgCuiResourceExtraction_Hopper & rhs);

	void                    updateHopperContentBarPage (const std::pair<NetworkId, uint16> & elem, float curContents, int maxContents, const UIWidget * page) const;
	void                    clearResources             ();

	typedef std::pair<NetworkId, float>              HopperContentElement;
	typedef stdvector<HopperContentElement>::fwd     HopperContentsVector;

	UIButton *              m_buttonOk;
	UIButton *              m_buttonDiscard;
	UIButton *              m_buttonRetrieve;
	UIButton *              m_buttonDumpAll;
	UIText *                m_textDisplay;
	UITable *               m_hopperTable;
	SwgCuiResourceExtraction_Quantity *              m_mediatorQuantity;
	UIWidget *                                       m_sampleResource;
	HopperContentsVector *                           m_hopperContents;
	TableModel*                                      m_resourceTableModel;
	stdvector<CuiWidget3dObjectListViewer*>::fwd     m_resourceIcons;
	stdvector<NetworkId>::fwd                        m_resources;
	stdvector<UIPage*>::fwd                          m_resourceHoppers;

	MessageDispatch::Callback * m_callback;
};

//======================================================================

#endif















