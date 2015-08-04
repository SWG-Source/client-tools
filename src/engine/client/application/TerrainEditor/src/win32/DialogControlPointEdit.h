//===================================================================
//
// DialogControlPointEdit.h
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_DialogControlPointEdit_H
#define INCLUDED_DialogControlPointEdit_H

//===================================================================

#include "Resource.h"
#include "sharedMath/Vector2d.h"

//===================================================================

class DialogControlPointEdit : public CDialog
{
public:

	explicit DialogControlPointEdit (const Vector2d& point);
	explicit DialogControlPointEdit (const Vector2d& point, const float height);

	const Vector2d& getPoint () const;
	const float getHeight() const;

protected:

	//{{AFX_DATA(DialogControlPointEdit)
	enum { IDD = IDD_DIALOG_CONTROL_POINT_EDIT };
	NumberEdit	m_z;
	NumberEdit	m_x;
	NumberEdit  m_height;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(DialogControlPointEdit)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

protected:

	//{{AFX_MSG(DialogControlPointEdit)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

	Vector2d m_point;
	float m_heightVal;
	bool m_hasHeight;
};

//===================================================================

inline const Vector2d& DialogControlPointEdit::getPoint () const
{
	return m_point;
}

//-------------------------------------------------------------------

inline const float DialogControlPointEdit::getHeight () const
{
	return m_heightVal;
}

//===================================================================

//{{AFX_INSERT_LOCATION}}

//===================================================================

#endif 
