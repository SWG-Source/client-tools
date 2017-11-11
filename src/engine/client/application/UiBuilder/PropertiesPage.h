#if !defined(AFX_PROPERTIESPAGE_H__1C7E5F78_6F0B_4AA1_8755_5DEDBCDB709B__INCLUDED_)
#define AFX_PROPERTIESPAGE_H__1C7E5F78_6F0B_4AA1_8755_5DEDBCDB709B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PropertiesPage.h : header file
//

#include "ObjectPropertiesEditor.h"

#include "UIPropertyDescriptor.h"

#include <map>

class UIBaseObject;
class ObjectEditor;
class PropertyListControl;

namespace PropertiesPageNamespace {
	class Property;
}

/////////////////////////////////////////////////////////////////////////////
// PropertiesPage dialog

class PropertiesPage : public CPropertyPage, public ObjectPropertiesEditor::PropertyCategory
{
// Construction
public:
	PropertiesPage(ObjectEditor &i_editor, UIPropertyCategories::Category i_category);
	~PropertiesPage();

// Dialog Data
	//{{AFX_DATA(PropertiesPage)
	enum { IDD = IDD_PROPERTIES_PAGE };
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(PropertiesPage)
	public:
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation

protected:

	// ----------------------------------------------------------------

	virtual ObjectPropertiesEditor::PropertyList *_newPropertyList(
		const UIPropertyGroup &propertyGroup, 
		const PropertyListMap::iterator &insertionPoint
	);

	virtual void _freePropertyList(
		ObjectPropertiesEditor::PropertyList *pl,
		const PropertyListMap::iterator &listIter
	);

	// ----------------------------------------------------------------

	// Generated message map functions
	//{{AFX_MSG(PropertiesPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROPERTIESPAGE_H__1C7E5F78_6F0B_4AA1_8755_5DEDBCDB709B__INCLUDED_)
