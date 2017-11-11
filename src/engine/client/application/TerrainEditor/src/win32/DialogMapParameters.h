//
// DialogMapParameters.h
// asommers 10-9-2000
//
// copyright 2000, verant interactive
//

//-------------------------------------------------------------------

#ifndef DIALOGMAPPARAMETERS_H
#define DIALOGMAPPARAMETERS_H

//-------------------------------------------------------------------

#include "resource.h"

//-------------------------------------------------------------------

class DialogMapParameters : public CDialog
{
protected:

	virtual void OnOK (void);

protected:

	void recalculate ();

	//{{AFX_MSG(DialogMapParameters)
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonGlobalwatershaderbrowse();
	afx_msg void OnCheckUseglobalwater();
	afx_msg void OnSelchangeComboChunkWidth();
	afx_msg void OnSelchangeComboTilesPerChunk();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:

	//{{AFX_VIRTUAL(DialogMapParameters)
	virtual void DoDataExchange(CDataExchange* pDX);    
	//}}AFX_VIRTUAL

//lint -save -e1925

public:

	int shaderSize;
	int mapWidth;
	int chunkWidth;
	int tilesPerChunk;

public:

	//{{AFX_DATA(DialogMapParameters)
	enum { IDD = IDD_DIALOG_MAPPARAMETERS };
	CComboBox	m_shaderSize;
	CComboBox	m_tilesPerChunk;
	CComboBox	m_mapWidth;
	CComboBox	m_chunkWidth;
	BOOL	m_useGlobalWater;
	float	m_globalWaterHeight;
	CString	m_globalWaterShader;
	float	m_globalWaterShaderSize;
	int		m_heightPoleDistance;
	int		m_tileSize;
	int		m_hours;
	int		m_minutes;
	BOOL	m_legacyMap;
	//}}AFX_DATA

//lint -restore

public:

	explicit DialogMapParameters(CWnd* pParent = NULL); 
};

//-------------------------------------------------------------------

//{{AFX_INSERT_LOCATION}}

//-------------------------------------------------------------------

#endif 
