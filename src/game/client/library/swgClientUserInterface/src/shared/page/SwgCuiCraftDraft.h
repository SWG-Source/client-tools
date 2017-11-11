//======================================================================
//
// SwgCuiCraftDraft.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiCraftDraft_H
#define INCLUDED_SwgCuiCraftDraft_H

//======================================================================

#include "UIEventCallback.h"
#include "clientUserInterface/CuiMediator.h"

#if WIN32
#pragma warning (disable:4503)
#endif

class CuiMessageBox;
class CuiWorkspace;
class MessageQueueDraftSlots;
class MessageQueueDraftSlotsData;
class NetworkId;
class SwgCuiInventoryInfo;
class UIButton;
class UICheckbox;
class UIDataSourceContainer;
class UIPage;
class UITabbedPane;
class UITreeView;
struct MessageQueueDraftSchematicsData;

#include <vector>

namespace MessageDispatch
{
	class Emitter;
	class Callback;
};

//----------------------------------------------------------------------

class SwgCuiCraftDraft :
public CuiMediator,
public UIEventCallback
{
public:

	explicit                    SwgCuiCraftDraft (UIPage & page);

	void                        performActivate   ();
	void                        performDeactivate ();

	void                        OnButtonPressed           (UIWidget *context);
	void                        OnGenericSelectionChanged (UIWidget *context);
	void                        OnTabbedPaneChanged       (UIWidget *context); 

	bool                        OnMessage                 (UIWidget * context, const UIMessage & msg);

	bool                        close                     ();

	void                        OnCheckboxSet             (UIWidget *Context );
	void                        OnCheckboxUnset           (UIWidget *Context );

private:
	                           ~SwgCuiCraftDraft ();
	                            SwgCuiCraftDraft (const SwgCuiCraftDraft &);
	SwgCuiCraftDraft &          operator= (const SwgCuiCraftDraft &);

	typedef std::vector<MessageQueueDraftSchematicsData> SchematicVector;
	typedef MessageQueueDraftSlotsData                   Slot;
	typedef stdvector<Slot>::fwd                         SlotVector;

	void                        reset                     ();
	void                        updateInfo                ();
	void                        onDraftSlotsReceived      (const SlotVector & slots);
	void                        onDraftSchematicsReceived (const SchematicVector        & schematics);
	void                        onSessionEnded            (const bool &);
	void                        ok                        (bool fromTree);
	void                        resetCategory             ();

	UIButton *                  m_buttonBack;
	UIButton *                  m_buttonNext;

	MessageDispatch::Callback * m_callback;

	struct InternalSchematicData
	{
		std::pair<uint32, uint32> crc;
		Unicode::String           displayName;
		int                       gameType;
		int                       category;
		int                       index;

		InternalSchematicData (const std::pair<uint32, uint32> & _crc, int _category, int _index);

		bool operator< (const InternalSchematicData & rhs) const
		{
			return displayName < rhs.displayName;
		}
	};

	struct InternalCategoryData
	{
		Unicode::String displayName;
		int             category;

		InternalCategoryData (int _category);

		bool operator< (const InternalCategoryData & rhs) const
		{
			return displayName < rhs.displayName;
		}
	};

	struct InternalGameTypeData
	{
		Unicode::String displayName;
		int             gameType;

		InternalGameTypeData (int _gameType);
		bool operator< (const InternalGameTypeData & rhs) const
		{
			return displayName < rhs.displayName;
		}
	};

	typedef stdvector<InternalSchematicData>::fwd                  SchematicDataVector;
	typedef stdmap<InternalGameTypeData, SchematicDataVector>::fwd SchematicDataMap;
	typedef stdmap<InternalCategoryData, SchematicDataMap>::fwd    SchematicCategoryMap;
	
	static UIDataSourceContainer * populateGameTypeDraftData (const SchematicDataVector & sdv, UIDataSourceContainer & dsc_gameType, const uint32 match_crc);

	SchematicCategoryMap *      m_schematics;

	UITabbedPane *              m_tabs;
	SwgCuiInventoryInfo *       m_info;

	int                         m_selectedCategory;

	UITreeView *                m_treeView;

	int                         m_lastCategorySelected;

	UICheckbox *                m_checkShowAll;

	typedef stdmap<int, uint32>::fwd CategoryCrcMap;
	CategoryCrcMap *            m_lastSchematicSelectedCrcs;

};

//======================================================================

#endif
