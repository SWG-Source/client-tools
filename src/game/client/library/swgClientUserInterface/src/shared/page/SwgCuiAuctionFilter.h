//======================================================================
//
// SwgCuiAuctionFilter.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiAuctionFilter_H
#define INCLUDED_SwgCuiAuctionFilter_H

//======================================================================

#include "clientUserInterface/CuiMediator.h"
#include "sharedNetworkMessages/AuctionQueryHeadersMessage.h"
#include "UIEventCallback.h"

class UIButton;
class UIComboBox;
class UIScrollbar;
class UIText;
class UITextbox;
class UITreeView;
class UITable;
class UITableHeader;
class UICheckbox;
class UITabbedPane;
class AuctionManagerClientData;

namespace MessageDispatch
{
	class Callback;
	template <typename MessageType, typename IdentifierType> class Transceiver;
};

//----------------------------------------------------------------------

class SwgCuiAuctionFilter :
public CuiMediator,
public UIEventCallback
{
public:

	struct Messages
	{
		struct Changed
		{
			typedef SwgCuiAuctionFilter Payload;
		};
	};

	struct Transceivers
	{
		typedef MessageDispatch::Transceiver <const Messages::Changed::Payload &, Messages::Changed> Changed;
	};

	typedef stdvector<int>::fwd  ObjectTypeVector;

	                             SwgCuiAuctionFilter           (UIPage & page, const char * const debugName);

	void                         OnButtonPressed               (UIWidget * context);
	void                         OnCheckboxSet                 (UIWidget * context);
	void                         OnCheckboxUnset               (UIWidget * context);
	void                         OnGenericSelectionChanged     (UIWidget * context);

	void                         onItemTypeListChanged         (const std::string &);
	void                         onResourceTypeListChanged     (const std::string &);

	int                          getSelectedObjectType         (int & selectedObjectTemplateId, bool & selectedObjectIsResourceContainer, bool & selectedObjectIsSpecialGeneralGot) const;
	int                          getSelectedLocation           () const;

	const Unicode::String &      getTextFilterAll              () const;
	const Unicode::String &      getTextFilterAny              () const;

	int                          getPriceFilterMin             () const;
	int                          getPriceFilterMax             () const;
	bool                         getPriceFilterIncludesEntranceFee() const;
	std::list<AuctionQueryHeadersMessage::SearchCondition> const & getItemAttributeFilter(AuctionQueryHeadersMessage::AdvancedSearchMatchAllAny & matchAllAny) const;

	void                         checkpointTextPriceFilterValues     ();
	bool                         hasTextPriceFilterValuesChangedSinceCheckpoint() const;

	Transceivers::Changed &      getTransceiverChanged         ();

	int                          getSelectedObjectType         () const;
	void                         setSelectedObjectType         (int type, bool requestRefresh);

	void                         update                        (float deltaTimeSecs);

	static void                  buildAttributeFilterDisplayString();

protected:

	                            ~SwgCuiAuctionFilter           ();

	void                         performActivate               ();
	void                         performDeactivate             ();

private:

	SwgCuiAuctionFilter ();
	SwgCuiAuctionFilter (const SwgCuiAuctionFilter & rhs);
	SwgCuiAuctionFilter operator= (const SwgCuiAuctionFilter & rhs);

	void                        requestUpdate             ();
	void                        reset                     ();
	void                        updateFilterTree          ();

	void                        enableButtonsForItemAttributeFilter();
	void                        enableControlsForItemAttributeFilter(bool refreshValueCombobox);
	void                        refreshItemAttributeFilter();
	void                        populateSearchableAttributeNameCombo(stdmap<Unicode::String, std::pair<Unicode::String, std::string> >::fwd const & searchAttribute, stdmap<Unicode::String, std::pair<Unicode::String, std::string> >::fwd const * additionalSearchAttribute, std::string const & excludeSearchAttribute);

	void                        saveNamePriceFilter();
	void                        restoreNamePriceFilter();

	MessageDispatch::Callback * m_callback;

	UITreeView *                m_treeTypes;

	UICheckbox *                m_radioGalaxy;
	UICheckbox *                m_radioPlanet;
	UICheckbox *                m_radioCity;
	UICheckbox *                m_radioMarket;

	UITextbox *                 m_textboxTextFilter;
	UITextbox *                 m_textboxMinPriceFilter;
	UITextbox *                 m_textboxMaxPriceFilter;

	UICheckbox *                m_includeEntranceFee;

	UICheckbox *                m_enableItemAttributeFilter;
	UIComboBox *                m_comboboxMatchAllAny;
	UITableHeader *             m_attributeFilterTableHeader;
	UITable *                   m_attributeFilterTable;
	UIScrollbar *               m_attributeFilterTableScrollbar;
	UIButton *                  m_buttonAddFilterAttribute;
	UIButton *                  m_buttonUpdateFilterAttribute;
	UIButton *                  m_buttonRemoveFilterAttribute;
	UIButton *                  m_buttonRemoveAllFilterAttribute;
	UIText *                    m_labelAttributeName;
	UIComboBox *                m_comboboxAttributeName;
	UIText *                    m_labelMinValue;
	UIPage *                    m_pageMinIntValue;
	UITextbox *                 m_textboxMinIntValue;
	UIPage *                    m_pageMinFloatValue;
	UITextbox *                 m_textboxMinFloatValue;
	UIComboBox *                m_comboboxStringCompare;
	UIComboBox *                m_comboboxEnumCompare;
	UIText *                    m_labelMaxValue;
	UIText *                    m_labelValue;
	UIPage *                    m_pageMaxIntValue;
	UITextbox *                 m_textboxMaxIntValue;
	UIPage *                    m_pageMaxFloatValue;
	UITextbox *                 m_textboxMaxFloatValue;
	UIPage *                    m_pageStringValue;
	UITextbox *                 m_textboxStringValue;
	UIPage *                    m_pageComboValue;
	UIComboBox *                m_comboboxValue;
	UICheckbox *                m_requiredAttribute;

	Transceivers::Changed *     m_transceiverChanged;

	bool                        m_requestUpdateNextFrame;

	bool                        m_ignoreFilterChangeRequestUpdate;

	std::string                 m_itemTypeListVersion;
	std::string                 m_resourceTypeListVersion;

	void *                      m_currentlySelectedRowDSC;

	Unicode::String             m_checkpointTextFilterAll;
	Unicode::String             m_checkpointTextFilterAny;
	int                         m_checkpointPriceFilterMin;
	int                         m_checkpointPriceFilterMax;
	bool                        m_checkpointPriceFilterIncludesEntranceFee;
	AuctionQueryHeadersMessage::AdvancedSearchMatchAllAny m_checkpointItemAttributeFilterMatchAllAny;
	std::list<AuctionQueryHeadersMessage::SearchCondition> m_checkpointAttributeFilter;

	int                         m_currentlySelectedObjectType;
	int                         m_currentlySelectedObjectTemplateId;
	bool                        m_currentlySelectedObjectIsSpecialGeneralGot;
};

//----------------------------------------------------------------------

inline SwgCuiAuctionFilter::Transceivers::Changed & SwgCuiAuctionFilter::getTransceiverChanged ()
{
	return *NON_NULL (m_transceiverChanged);
}

//======================================================================

#endif
