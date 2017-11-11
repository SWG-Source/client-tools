//======================================================================
//
// SwgCuiCraftAssembly.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiCraftAssembly_H
#define INCLUDED_SwgCuiCraftAssembly_H

//======================================================================

#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"
#include "UITypes.h"
#include "sharedMath/Vector.h"

class CachedNetworkId;
class ClientObject;
class CuiWidget3dObjectListViewer;
class ManufactureSchematicObject;
class NetworkId;
class SwgCuiCraftAssemblyDialogOption;
class SwgCuiInventoryContainer;
class SwgCuiInventoryInfo;
class UIButton;
class UIImage;
class UIImageStyle;
class UIPage;
class UIText;
class MessageQueueDraftSlotsDataOption;
class SwgCuiContainerProviderDefault;
class UITabbedPane;
class UICheckbox;
class UIVolumePage;

namespace MessageDispatch
{
	class Emitter;
	class Callback;
};

//----------------------------------------------------------------------

class SwgCuiCraftAssembly :
public CuiMediator,
public UIEventCallback
{
public:

	explicit                SwgCuiCraftAssembly              (UIPage & page);

	void                    performActivate                  ();
	void                    performDeactivate                ();

	void                    OnButtonPressed                  (UIWidget *context );
	bool                    OnMessage                        (UIWidget *context, const UIMessage & msg );
	void                    OnCheckboxSet                    (UIWidget *context);
	void                    OnCheckboxUnset                  (UIWidget *context);

	void                    onSelectionChanged               (const std::pair<int, ClientObject *> & payload);

	static void             createObjectTooltipString        (const ClientObject & obj, Unicode::String & str);

	void                    OnTabbedPaneChanged              (UIWidget * context);

	bool                    close                            ();

	bool                    overrideDoubleClick              (const UIWidget & viewerWidget);

	void                    checkSizes                       ();

	void                    onAttributesChanged              (const NetworkId & id);
	
private:
	~SwgCuiCraftAssembly ();
	SwgCuiCraftAssembly (const SwgCuiCraftAssembly &);
	SwgCuiCraftAssembly & operator= (const SwgCuiCraftAssembly &);

	typedef stdvector<UIText *>::fwd                  TextVector;
	typedef stdvector<CuiWidget3dObjectListViewer *>::fwd ViewerVector;
	typedef stdvector<UIImage *>::fwd                 IconVector;
	typedef stdvector<UIPage *>::fwd                  QualityBarVector;

	CuiWidget3dObjectListViewer *       findSlotViewer      (int index);
	const CuiWidget3dObjectListViewer * findSlotViewer      (int index) const;
	CuiWidget3dObjectListViewer *       findSlotViewer      (const UIWidget * context);
	int                                 findSlotViewerIndex (const UIWidget * context);
  	void                                hoverSlot           (CuiWidget3dObjectListViewer * slotViewer);

	Vector                              findSlotPosition_o (int index);

	void                                onComplexityChange           (const ClientObject & target);
	void                                onManufactureSchematicChange (const ManufactureSchematicObject & schem);
	void                                onSlotTransferStatusChanged  (const bool & );
	void                                onStageChange                (const int & stage);
	void                                onSessionEnded               (const bool &);
	void                                onAssembling                 (const int & stage);
	void                                onDoneAssembling             (const int & stage);

	void                                resetSlots             ();
	void                                enableSlots            ();
	void                                updateIconState        (const ClientObject * const obj, bool showBlocked, bool handleViewer);
	bool                                objectCanGoInSlot      (const ClientObject & obj, int slotNum, int & slotContents, int & slotMaxContents, int & optionIndex);

	void                                updateSchematicText          ();
	void                                updateQualityBar       (int i);

	SwgCuiInventoryContainer *			getActiveContainer     ();

	TextVector   *                    m_textSlots;
	TextVector   *                    m_textLoadingSlots;
	ViewerVector *                    m_viewerSlots;
	IconVector *                      m_iconSlots;
	QualityBarVector *                m_qualityBarSlots;

	int                               m_maxNumSlots;

	UIButton *                        m_buttonBack;
	UIButton *                        m_buttonNext;

	UIText *                          m_textDesc;

	UIText *                          m_textComplexity;
	UIText *                          m_textSchematic;

	SwgCuiInventoryContainer *        m_containerHopper;
	SwgCuiInventoryContainer *        m_containerInventory;
	SwgCuiInventoryInfo *             m_hopperInfo;

	CuiWidget3dObjectListViewer *     m_viewer;

	int                               m_selectedSlot;
	int                               m_hoverSlot;

	Vector                            m_selectedTarget;
	Vector                            m_hoverTarget;

	MessageDispatch::Callback *       m_callback;

	UIColor                           m_colorRequiredFilled;
	UIColor                           m_colorRequiredEmpty;
	UIColor                           m_colorOptionalFilled;
	UIColor                           m_colorOptionalEmpty;
	UIColor                           m_colorRequiredEmptyInvalid;
	UIColor                           m_colorOptionalEmptyInvalid;

	SwgCuiCraftAssemblyDialogOption * m_dialogOption;

	class ContainerFilter;
	class ProviderFilter;

	ContainerFilter *                 m_containerFilter;
	ProviderFilter *                  m_providerFilter;

	friend ProviderFilter;

	ClientObject *                    m_dummyObject;

	bool                              m_transitionForward;

	std::string                       m_overrideShaderName;

	UIPage *                          m_pageUnloadHopper;
	UIPage *                          m_pageUnloadInventory;

	UIImageStyle *                    m_iconSlotInvalid;
	UIImageStyle *                    m_iconSlotValid;
	UIImageStyle *                    m_iconSlotValidBlocked;
	UIColor                           m_iconColorInvalid;
	UIColor                           m_iconColorValid;
	UIColor                           m_iconColorValidBlocked;

	SwgCuiContainerProviderDefault *  m_containerProviderInventory;
	SwgCuiContainerProviderDefault *  m_containerProviderHopper;

	UITabbedPane *                    m_tabs;
	UICheckbox *                      m_checkFilter;

	UIVolumePage *                    m_volumeSlots;

	std::pair<uint32, uint32>         m_currentDraftSchematic;
};

//======================================================================

#endif

