// ======================================================================
//
// SwgCuiLfg.h
// copyright (c) 2008 Sony Online Entertainment LLC
//
// ======================================================================

#ifndef INCLUDED_SwgCuiLFG_H
#define INCLUDED_SwgCuiLFG_H

// ======================================================================

#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"

#include "sharedGame/LfgDataTable.h"

class UIComboBox;

// ----------------------------------------------------------------------

class SwgCuiLfg : 
public CuiMediator, 
public UIEventCallback
{
public:
	SwgCuiLfg(UIPage & page, bool externalCriteriaOnly = false);
	~SwgCuiLfg();

	virtual void OnButtonPressed(UIWidget * context);
	virtual void OnCheckboxSet(UIWidget * context);
	virtual void OnCheckboxUnset(UIWidget * context);
	virtual void OnGenericSelectionChanged(UIWidget * context);

	void disableWindow ();
	void enableWindow  ();
	
	void saveOrLoadBackup (bool const shouldSave);
	void getAllSearchCriteria (BitArray & criteriaBits);
	void sendInterestsToServer ();
	
	void updateBranchStateByName(const std::string & name);
	UIWidget * getWidgetByName (std::string const & parentName, std::string const & widgetName);
	
	static void sendSavedInterests ();

protected:
	virtual void performActivate();
	virtual void performDeactivate();

private:

	enum CriteriaState
	{
		CS_NONE,
		CS_SOME,
		CS_ALL
	};

	void populateBranchPage(LfgDataTable::LfgNode const * node, UIPage * page);
	void populateChildrenPage(LfgDataTable::LfgNode const * node, UIPage * page, int tier);
	void wrapParentToChildren(UIPage * parentPage);
	void wrapParentPages(UIPage * page);
 	void setName(UIPage * page, std::string const & name, UIColor const & color);
	void checkAllChildren(UIPage * const page, const bool check);
	void initPageDefaults(UIPage * const page, LfgDataTable::LfgNode const * node);
	void linkPageItems(UIPage * const page, std::string const & name);
	void setCheckAnyAll(UIPage * const page, const std::string & widgetName, const bool check, const bool ignoreCheck = false,  const bool disable = false, const bool hide = false);

	void changeBranchColor (UIPage  * const page, const UIColor & color, const bool changeChildren = false);
	void updateParentStates (UIPage * const  page);
	void calculateBranchColor (UIPage * const  page);
	void updateMinMaxPage (UIPage * const  page, const bool enable, const bool setDefaults = false, const bool useParams = false, const int minVal = 0, const int maxVal = 0);
	void getPageMinMaxValues (UIPage * const page, int & min, int & max);

	void loadCriteria(char const * filename);
	void saveCriteria(char const * filename);
	void resetAllCriteria ();
	void loadTable();
	void updateCombobox();

	CriteriaState getCriteriaState (UIPage const *  page);
		
	UICheckbox * getCheckAnyAll(UIPage const *  page, std::string const & widgetName);
	UIWidget * getWidget(UIPage const *  page, std::string const & widgetName);
 	UIPage * getOffsetPage(UIPage const *  page);
	UIPage * getParentBranch(UIPage *  page);

	bool m_externalCriteriaOnly;
	
	UIComposite * m_sampleMinMax;
	UIComposite * m_sampleChild;
	UIComposite * m_samplePage;
	UIComposite * m_criteriaPage;
	UIComboBox  * m_comboBox;

	typedef stdmap<std::string, UIPage * >::fwd StringMap;
	StringMap * m_stringLinks;

	typedef stdmap<UIPage *, LfgDataTable::LfgNode const * >::fwd PageMap;
	PageMap * m_nodeLinks;
	
	SwgCuiLfg();
	SwgCuiLfg(SwgCuiLfg const & rhs);
	SwgCuiLfg &operator=(SwgCuiLfg const & rhs);
};

// ======================================================================

#endif   //INCLUDED_SwgCuiLFG_H