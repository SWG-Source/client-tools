// DialogPalette.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDialogPalette dialog

#include "resource.h"
#include "sharedTerrain/ColorRamp256.h"

class CDialogPalette : public CDialog
{
// Construction
public:

	CDialogPalette(CWnd* pParent=0, const char* windowName=0);   // standard constructor

	const ColorRamp256& getColorRamp () const;
	void                setColorRamp (const ColorRamp256&);

private:

	bool         m_dragging;
	ColorRamp256 colorRamp;
	CString      m_windowName;
	bool         m_dragOldNode;
	int          m_dragOldIndex;
	PackedRgb    m_dragOldColor;

	PackedRgb    m_dragNewColor;

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	int m_space;
	int m_height;
	int m_width;
  int m_oldi;
  int m_oldj;

	CUIntArray m_ArrayNode;
  int m_selected;
  
	CBitmap m_Bitmap;
	CDC m_MemDC;

	void DrawPalette();
  void MakePalette();
  void DrawItem(CDC *pDC,COLORREF LineColor,COLORREF FillColor,CRect &rect);
  void DrawPaletteItem(int i,int j);
  void DrawLine(CDC *pDC,int x1,int y1,int x2,int y2,COLORREF color = RGB(0,0,0),int PenStyle = PS_SOLID);
	void Init();

// Dialog Data
	//{{AFX_DATA(CDialogPalette)
	enum { IDD = IDD_DIALOG_PALETTE };
	CString	m_StringBlue;
	CString	m_StringGreen;
	CString	m_StringRed;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDialogPalette)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CToolTipCtrl m_tooltip;

	// Generated message map functions
	//{{AFX_MSG(CDialogPalette)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg void OnClose();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnButtonChange();
	afx_msg void OnButtonAdd();
	afx_msg void OnButtonDelete();
	afx_msg void OnButtonDeleteAll();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnButtonLoad();
	afx_msg void OnButtonSave();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
