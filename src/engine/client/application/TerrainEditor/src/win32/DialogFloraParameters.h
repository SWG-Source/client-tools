//===================================================================
//
// DialogFloraParameters.h
// asommers
//
// copyright 2001, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_DialogFloraParameters_H
#define INCLUDED_DialogFloraParameters_H

//===================================================================

#include "Resource.h"

//===================================================================

class DialogFloraParameters : public CDialog
{
public:

	DialogFloraParameters(CWnd* pParent = NULL);

	//{{AFX_DATA(DialogFloraParameters)
	enum { IDD = IDD_DIALOG_FLORAPARAMETERS };
	float   m_collidableMinimumDistance;
	float   m_collidableMaximumDistance;
	UINT	m_collidableSeed;
	float	m_collidableTileBorder;
	float	m_collidableTileSize;
	int		m_collidableTotalFlora;
	float   m_nonCollidableMinimumDistance;
	float   m_nonCollidableMaximumDistance;
	UINT	m_nonCollidableSeed;
	float	m_nonCollidableTileBorder;
	float	m_nonCollidableTileSize;
	int		m_nonCollidableTotalFlora;
	float   m_radialMinimumDistance;
	float   m_radialMaximumDistance;
	UINT	m_radialSeed;
	float	m_radialTileBorder;
	float	m_radialTileSize;
	int		m_radialTotalFlora;
	float   m_farRadialMinimumDistance;
	float   m_farRadialMaximumDistance;
	UINT	m_farRadialSeed;
	float	m_farRadialTileBorder;
	float	m_farRadialTileSize;
	int		m_farRadialTotalFlora;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(DialogFloraParameters)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	//}}AFX_VIRTUAL

protected:

	//{{AFX_MSG(DialogFloraParameters)
	afx_msg void recalculate ();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//===================================================================

//{{AFX_INSERT_LOCATION}}

//===================================================================

#endif 
