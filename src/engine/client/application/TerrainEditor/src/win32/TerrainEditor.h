//
// TerrainEditor.h
// asommers 10-9-2000
//
// copyright 2000, verant interactive
//

//-------------------------------------------------------------------

#ifndef TERRAINEDITOR_H
#define TERRAINEDITOR_H

//-------------------------------------------------------------------

#include "resource.h"

class TerrainEditorDoc;

//-------------------------------------------------------------------

class TerrainEditorApp : public CWinApp
{
private:

	//-- document templates
	CMultiDocTemplate* mapViewTemplate;
	CMultiDocTemplate* floraGroupTemplate;
	CMultiDocTemplate* radialGroupTemplate;
	CMultiDocTemplate* bookmarkTemplate;
	CMultiDocTemplate* shaderGroupTemplate;
	CMultiDocTemplate* consoleTemplate;
	CMultiDocTemplate* layerTemplate;
	CMultiDocTemplate* propertyTemplate;
	CMultiDocTemplate* blendGroupTemplate;
	CMultiDocTemplate* fractalPreviewTemplate;
	CMultiDocTemplate* bitmapPreviewTemplate;
	CMultiDocTemplate* view3dTemplate;
	CMultiDocTemplate* helpTemplate;
	CMultiDocTemplate* profileTemplate;
	CMultiDocTemplate* warningTemplate;
	CMultiDocTemplate* findTemplate;
	CMultiDocTemplate* environmentTemplate;
	CMultiDocTemplate* fractalGroupTemplate;
	CMultiDocTemplate* bitmapGroupTemplate;

private:

	void ShowTipAtStartup(void);
	void ShowTipOfTheDay(void);

	void onViewLayers         (TerrainEditorDoc* terrainDocument);
	void onViewMap            (TerrainEditorDoc* terrainDocument);
	void onViewFlora          (TerrainEditorDoc* terrainDocument);
	void onViewRadial         (TerrainEditorDoc* terrainDocument);
	void onViewShaders        (TerrainEditorDoc* terrainDocument);
	void onViewBookmarks      (TerrainEditorDoc* terrainDocument);
	void onViewHelp           (TerrainEditorDoc* terrainDocument);
	void onViewConsole        (TerrainEditorDoc* terrainDocument);
	void onViewProperties     (TerrainEditorDoc* terrainDocument);
	void onViewBlends         (TerrainEditorDoc* terrainDocument);
	void onViewFractals       (TerrainEditorDoc* terrainDocument);
	void onViewBitmaps        (TerrainEditorDoc* terrainDocument);
	void onViewFractalPreview (TerrainEditorDoc* terrainDocument);
	void onViewBitmapPreview  (TerrainEditorDoc* terrainDocument);
	void onView3d             (TerrainEditorDoc* terrainDocument);
	void onViewProfile        (TerrainEditorDoc* terrainDocument);
	void onViewWarning        (TerrainEditorDoc* terrainDocument, bool clear);
	void onViewFind           (TerrainEditorDoc* terrainDocument, bool clear);
	void onViewEnvironment    (TerrainEditorDoc* terrainDocument);

public:

	virtual BOOL PreTranslateMessage(MSG* pMsg);

	TerrainEditorApp (void);
	virtual ~TerrainEditorApp (void);

	void onOpenDefaultViews (TerrainEditorDoc* terrainDocument);
	void showFractalPreview (void);
	void showBitmapPreview (void);
	void showConsole (void);
	void showProfile (void);
	void showLayers (void);
	void showWarning (bool clear);
	void showFind (bool clear);

	TerrainEditorDoc* getDocument (void);

	void SaveWindowPosition(const CWnd* cwnd, const char* name);
	bool RestoreWindowPosition(CWnd* cwnd, const char* name);

	//{{AFX_VIRTUAL(TerrainEditorApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	virtual CDocument* OpenDocumentFile(LPCTSTR lpszFileName);
	//}}AFX_VIRTUAL

	//{{AFX_MSG(TerrainEditorApp)
	afx_msg void OnAppAbout();
	afx_msg void OnEditFlora();
	afx_msg void OnUpdateEditFlora(CCmdUI* pCmdUI);
	afx_msg void OnViewbookmarks();
	afx_msg void OnUpdateViewbookmarks(CCmdUI* pCmdUI);
	afx_msg void OnEditShaders();
	afx_msg void OnUpdateEditShaders(CCmdUI* pCmdUI);
	afx_msg void OnViewconsole();
	afx_msg void OnUpdateViewconsole(CCmdUI* pCmdUI);
	afx_msg void OnButtonLayerview();
	afx_msg void OnUpdateButtonLayerview(CCmdUI* pCmdUI);
	afx_msg void OnViewproperties();
	afx_msg void OnUpdateViewproperties(CCmdUI* pCmdUI);
	afx_msg void OnViewmap();
	afx_msg void OnUpdateViewmap(CCmdUI* pCmdUI);
	afx_msg void On3dView();
	afx_msg void OnUpdate3dView(CCmdUI* pCmdUI);
	afx_msg void OnEditBlends();
	afx_msg void OnUpdateEditBlends(CCmdUI* pCmdUI);
	afx_msg void OnFractalPreview();
	afx_msg void OnBitmapPreview();
	afx_msg void OnUpdateFractalPreview(CCmdUI* pCmdUI);
	afx_msg void OnUpdateBitmapPreview(CCmdUI* pCmdUI);
	afx_msg void OnEditRadial();
	afx_msg void OnUpdateEditRadial(CCmdUI* pCmdUI);
	afx_msg void OnViewHelp();
	afx_msg void OnUpdateViewHelp(CCmdUI* pCmdUI);
	afx_msg void OnViewProfile();
	afx_msg void OnUpdateViewProfile(CCmdUI* pCmdUI);
	afx_msg void OnViewWarning();
	afx_msg void OnUpdateViewWarning(CCmdUI* pCmdUI);
	afx_msg void OnViewFind();
	afx_msg void OnUpdateViewFind(CCmdUI* pCmdUI);
	afx_msg void OnViewEnvironment();
	afx_msg void OnUpdateViewEnvironment(CCmdUI* pCmdUI);
	afx_msg void OnEditFractal();
	afx_msg void OnEditBitmap();
	afx_msg void OnUpdateEditFractal(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditBitmap(CCmdUI* pCmdUI);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//-------------------------------------------------------------------

inline TerrainEditorApp* GetApp (void)
{
	return safe_cast<TerrainEditorApp*> (AfxGetApp ());
}

//-------------------------------------------------------------------

//{{AFX_INSERT_LOCATION}}

//-------------------------------------------------------------------

#endif
