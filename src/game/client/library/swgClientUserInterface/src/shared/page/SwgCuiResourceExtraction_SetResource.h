//======================================================================
//
// SwgCuiResourceExtraction_SetResource.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiResourceExtraction_SetResource_H
#define INCLUDED_SwgCuiResourceExtraction_SetResource_H

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
class CuiWidget3dObjectListViewer;

#include <vector>

//======================================================================

class SwgCuiResourceExtraction_SetResource : public CuiMediator, public UIEventCallback
{
public:
	class TableModel;

	explicit                     SwgCuiResourceExtraction_SetResource               (UIPage & page);
	void                         update                    ();
	NetworkId                    getResourceAtPosition     (int index);
	CuiWidget3dObjectListViewer* getResourceIconAtPosition (int index);
	int                          getResourceEfficiencyAtPosition (int index);
	Unicode::String              getResourceNameAtPosition (int index);
	int                          getNumResources                () const;
	void                         clearResources                 ();
	void                         addResource(const NetworkId& resourceId, const Unicode::String& name, int efficiency);
	void                         OnButtonPressed    (UIWidget *context);
	bool                         OnMessage(UIWidget *context, const UIMessage & msg);

protected:
	virtual void            performActivate    ();
	virtual void            performDeactivate  ();

private:
	                       ~SwgCuiResourceExtraction_SetResource             ();
	                        SwgCuiResourceExtraction_SetResource             ();
	                        SwgCuiResourceExtraction_SetResource             (const SwgCuiResourceExtraction_SetResource & rhs);
	                        SwgCuiResourceExtraction_SetResource & operator= (const SwgCuiResourceExtraction_SetResource & rhs);

	typedef std::pair<NetworkId, uint16>             SetResourceContentElement;
	typedef stdvector<SetResourceContentElement>::fwd     HopperContentsVector;

	UITable *               m_setResourceTable;
	HopperContentsVector *  m_setResourceContents;
	TableModel*                                      m_resourceTableModel;
	UIButton *                                       m_buttonOk;
	UIButton *                                       m_buttonCancel;
	stdvector<CuiWidget3dObjectListViewer*>::fwd     m_resourceIcons;
	stdvector<NetworkId>::fwd                        m_resources;

	stdvector<Unicode::String>::fwd                  m_names;
	stdvector<int>::fwd                              m_efficiencies;
};

//======================================================================

#endif