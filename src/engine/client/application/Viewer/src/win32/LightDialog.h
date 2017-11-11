#if !defined(AFX_LIGHTDIALOG_H__D70D1EFD_ED58_462A_B61B_635C580D5E83__INCLUDED_)
#define AFX_LIGHTDIALOG_H__D70D1EFD_ED58_462A_B61B_635C580D5E83__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LightDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// LightDialog dialog

#include "Resource.h"
#include "clientObject/LightManager.h"

class LightDialog : public CDialog
{
public:

	enum Mode
	{
		M_add,
		M_edit
	};

private:

	Mode                                     mode;
	LightManager::LightData              lightData;

	const LightManager&                  lightManager;
	int                                      myIndex;

private:

	void updateLightData (void);

public:

	LightDialog (CWnd* pParent, Mode newMode, const LightManager& newLightManager, int newMyIndex);   // standard constructor

	void                               setLightData (const LightManager::LightData& newLightData);
	const LightManager::LightData& getLightData (void) const;


// Dialog Data
	//{{AFX_DATA(LightDialog)
	enum { IDD = IDD_LIGHTS };
	CStatic	m_windowLightSpecular;
	CStatic	m_windowLightColor;
	float	m_colorB;
	float	m_colorG;
	float	m_colorR;
	BOOL	m_disableLight;
	CString	m_lightName;
	int		m_lightType;
	int		m_pitch;
	int		m_yaw;
	float	m_posX;
	float	m_posY;
	float	m_posZ;
	float	m_colorBSpecular;
	float	m_colorGSpecular;
	float	m_colorRSpecular;
	float	m_attenuationConstant;
	float	m_attenuationLinear;
	float	m_attenuationQuadratic;
	float	m_range;
	float	m_spotFalloff;
	float	m_spotPhi;
	float	m_spotTheta;
	BOOL	m_dot3;
	BOOL	m_shadowSource;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(LightDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(LightDialog)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonLightColor();
	afx_msg void OnButtonLightColorSpecular();
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

inline void  LightDialog::setLightData (const LightManager::LightData& newLightData)
{
	lightData = newLightData;
}

inline const LightManager::LightData& LightDialog::getLightData (void) const
{
	return lightData;
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LIGHTDIALOG_H__D70D1EFD_ED58_462A_B61B_635C580D5E83__INCLUDED_)
