//
// FormLayerItem.h
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_FormLayerItem_H
#define INCLUDED_FormLayerItem_H

//-------------------------------------------------------------------

#include "PropertyView.h"
#include "TerrainEditorDoc.h"

//-------------------------------------------------------------------

class FormLayerItem : public PropertyView
{
protected:

	FormLayerItem ();           
	explicit FormLayerItem (UINT nIDTemplate);
	DECLARE_DYNCREATE(FormLayerItem)

public:

	//{{AFX_DATA(FormLayerItem)
	enum { IDD = IDD_DEFAULT_PROPVIEW };
	//}}AFX_DATA

public:

	class FormLayerItemViewData : public PropertyView::ViewData
	{
	public:

		TerrainEditorDoc::Item* item;  //lint !e1925  //-- public data member

	public:

		FormLayerItemViewData (void);
		virtual ~FormLayerItemViewData (void);
	};

public:

	virtual void Initialize (PropertyView::ViewData* vd);
	virtual bool HasChanged (void) const;
	virtual void ApplyChanges (void);

	//{{AFX_VIRTUAL(FormLayerItem)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

protected:

	virtual ~FormLayerItem();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	//{{AFX_MSG(FormLayerItem)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//-------------------------------------------------------------------

//{{AFX_INSERT_LOCATION}}

//-------------------------------------------------------------------

#endif 
