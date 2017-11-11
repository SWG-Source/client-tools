// ======================================================================
//
// Viewer.h
// copyright 1999, Bootprint Entertainment
// copyright 2002, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_Viewer_H
#define INCLUDED_Viewer_H

// ==================================================================

#include "resource.h"
#include "sharedMath/VectorArgb.h"

class CViewerDoc;

// ==================================================================

class CViewerApp : public CWinApp
{
public:

	CViewerApp();
	~CViewerApp();

	//{{AFX_VIRTUAL(CViewerApp)
	public:
	virtual BOOL InitInstance();
	virtual int  Run(void);
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

	CViewerDoc*        getDocument () const;
	void               setCleanDatabinDirectory (const CString& newCleanDatabinDirectory);
	const CString&     getCleanDatabinDirectory (void) const;

	CMultiDocTemplate *getSkeletalAppearanceTemplateViewDocTemplate();
	CMultiDocTemplate *getVariableSetViewDocTemplate();

	const char        *getAnimationMapFilename() const;
	const char        *getOpenFilename() const;
	const char        *getLastMeshGeneratorFilename() const;
	const char        *getLastSkeletalAppearanceInstanceFilename() const;
	const char        *getLastSkeletalAppearanceTemplateFilename() const;
	const char        *getLastSkeletalAppearanceWorkspaceFilename() const;
	const char        *getLastSkeletonTemplateFilename() const;

	void               setAnimationMapFilename(const char *filename);
	void               setLastMeshGeneratorFilename(const char *filename);
	void               setLastSkeletalAppearanceTemplateFilename(const char *filename);
	void               setLastSkeletalAppearanceWorkspaceFilename(const char *filename);
	void               setLastSkeletonTemplateFilename(const char *filename);

	const char        *getMeshGeneratorReferenceDirectory() const;
	const char        *getSkeletonTemplateReferenceDirectory() const;
	const char        *getSkeletalAppearanceTemplateReferenceDirectory() const;

	//{{AFX_MSG(CViewerApp)
	afx_msg void OnAppAbout();
	afx_msg void OnToolsPreferences();
	afx_msg void OnToolsPopupDebugMenu();
	afx_msg void OnFileCloseall();
	afx_msg void OnFileNewSkeletalAppearanceTemplate();
	afx_msg void OnFileOpen();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

	char               m_applicationDirectory[MAX_PATH];

	CMultiDocTemplate *m_variableSetViewDocTemplate;
	CMultiDocTemplate *m_skeletalAppearanceTemplateViewDocTemplate;

	char               m_animationMapFilename[MAX_PATH];
	char               m_openFilename[MAX_PATH];
	char               m_lastMeshGeneratorFilename[MAX_PATH];
	char               m_lastSkeletalAppearanceTemplateFilename[MAX_PATH];
	char               m_lastSkeletalAppearanceWorkspaceFilename[MAX_PATH];
	char               m_lastSkeletonTemplateFilename[MAX_PATH];

	char               m_meshGeneratorReferenceDirectory[MAX_PATH];
	char               m_skeletonTemplateReferenceDirectory[MAX_PATH];
	char               m_skeletalAppearanceTemplateReferenceDirectory[MAX_PATH];

private:

	void alterDocuments(void);
	void loadPreferences (void);
	void savePreferences (void);
};

// ----------------------------------------------------------------------

inline CViewerApp* GetViewerApp (void)
{
	return static_cast<CViewerApp*> (AfxGetApp ());
}

// ----------------------------------------------------------------------

extern void CONSOLE_PRINT (const CString& newMessage);

// ----------------------------------------------------------------------

inline CMultiDocTemplate *CViewerApp::getSkeletalAppearanceTemplateViewDocTemplate()
{
	return m_skeletalAppearanceTemplateViewDocTemplate;
}

// ----------------------------------------------------------------------

inline CMultiDocTemplate *CViewerApp::getVariableSetViewDocTemplate()
{
	return m_variableSetViewDocTemplate;
}

// ----------------------------------------------------------------------

inline const char *CViewerApp::getAnimationMapFilename() const
{
	return m_animationMapFilename;
}

// ----------------------------------------------------------------------

inline void CViewerApp::setAnimationMapFilename(const char *filename)
{
	if (filename)
		strcpy(m_animationMapFilename, filename);
	else
		strcpy(m_animationMapFilename, "");
}

// ----------------------------------------------------------------------

inline const char *CViewerApp::getOpenFilename() const
{
	return m_openFilename;
}

// ----------------------------------------------------------------------

inline const char *CViewerApp::getLastMeshGeneratorFilename() const
{
	return m_lastMeshGeneratorFilename;
}

// ----------------------------------------------------------------------

inline const char *CViewerApp::getLastSkeletalAppearanceTemplateFilename() const
{
	return m_lastSkeletalAppearanceTemplateFilename;
}

// ----------------------------------------------------------------------

inline const char *CViewerApp::getLastSkeletalAppearanceWorkspaceFilename() const
{
	return m_lastSkeletalAppearanceWorkspaceFilename;
}

// ----------------------------------------------------------------------

inline const char *CViewerApp::getLastSkeletonTemplateFilename() const
{
	return m_lastSkeletonTemplateFilename;
}

// ----------------------------------------------------------------------

inline void CViewerApp::setLastMeshGeneratorFilename(const char *filename)
{
	if (filename)
		strcpy(m_lastMeshGeneratorFilename, filename);
}

// ----------------------------------------------------------------------

inline void CViewerApp::setLastSkeletalAppearanceTemplateFilename(const char *filename)
{
	if (filename)
		strcpy(m_lastSkeletalAppearanceTemplateFilename, filename);
}

// ----------------------------------------------------------------------

inline void CViewerApp::setLastSkeletalAppearanceWorkspaceFilename(const char *filename)
{
	if (filename)
		strcpy(m_lastSkeletalAppearanceWorkspaceFilename, filename);
}

// ----------------------------------------------------------------------

inline void CViewerApp::setLastSkeletonTemplateFilename(const char *filename)
{
	if (filename)
		strcpy(m_lastSkeletonTemplateFilename, filename);
}

// ----------------------------------------------------------------------

inline const char *CViewerApp::getMeshGeneratorReferenceDirectory() const
{
	return m_meshGeneratorReferenceDirectory;
}

// ----------------------------------------------------------------------

inline const char *CViewerApp::getSkeletonTemplateReferenceDirectory() const
{
	return m_skeletonTemplateReferenceDirectory;
}

// ----------------------------------------------------------------------

inline const char *CViewerApp::getSkeletalAppearanceTemplateReferenceDirectory() const
{
	return m_skeletalAppearanceTemplateReferenceDirectory;
}

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 
