// ======================================================================
//
// TextureBuilder.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#if !defined(AFX_TEXTUREBUILDER_H__FE82AD07_1A1A_4DE2_B701_648DF4DE1346__INCLUDED_)
#define AFX_TEXTUREBUILDER_H__FE82AD07_1A1A_4DE2_B701_648DF4DE1346__INCLUDED_

// ======================================================================

#if _MSC_VER > 1000
#pragma once
#endif

// ======================================================================

class CMultiDocTemplate;
class MainFrame;

// ======================================================================

class TextureBuilderApp : public CWinApp
{
public:

	static TextureBuilderApp &getApp();

public:

	TextureBuilderApp();
	virtual ~TextureBuilderApp();

	void               setLastTexturePathname(const char *pathname);
	const std::string &getLastTexturePathname() const;

	CMultiDocTemplate &getTextureBakeViewDocTemplate();
	CMultiDocTemplate &getConstructionTreeViewDocTemplate();
	CMultiDocTemplate &getVariableEditViewDocTemplate();

	MainFrame         &getMainFrame();
	const MainFrame   &getMainFrame() const;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// ClassWizard generated virtual function overrides
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//{{AFX_VIRTUAL(TextureBuilderApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// ClassWizards generated message map handlers
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//{{AFX_MSG(TextureBuilderApp)
	afx_msg void OnAppAbout();
	afx_msg void OnFileNew();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

	void saveSettings();
	void loadSettings();

private:

	std::string       *m_lastTexturePathname;
	CMultiDocTemplate *m_textureBakeViewDocTemplate;
	CMultiDocTemplate *m_constructionTreeViewDocTemplate;
	CMultiDocTemplate *m_variableEditViewDocTemplate;
	MainFrame         *m_mainFrame;

private:
	// disabled
	TextureBuilderApp(const TextureBuilderApp&);
	TextureBuilderApp &operator =(const TextureBuilderApp&);
};

// ======================================================================

inline TextureBuilderApp &TextureBuilderApp ::getApp()
{
	return dynamic_cast<TextureBuilderApp&>(*NON_NULL(AfxGetApp()));
}

// ----------------------------------------------------------------------

inline const std::string &TextureBuilderApp ::getLastTexturePathname() const
{
	return *m_lastTexturePathname;
}

// ----------------------------------------------------------------------

inline CMultiDocTemplate &TextureBuilderApp ::getTextureBakeViewDocTemplate()
{
	return *NON_NULL(m_textureBakeViewDocTemplate);
}

// ----------------------------------------------------------------------

inline CMultiDocTemplate &TextureBuilderApp ::getConstructionTreeViewDocTemplate()
{
	return *NON_NULL(m_constructionTreeViewDocTemplate);
}

// ----------------------------------------------------------------------

inline CMultiDocTemplate &TextureBuilderApp ::getVariableEditViewDocTemplate()
{
	return *NON_NULL(m_variableEditViewDocTemplate);
}

// ----------------------------------------------------------------------

inline MainFrame &TextureBuilderApp ::getMainFrame()
{
	return *NON_NULL(m_mainFrame);
}

// ----------------------------------------------------------------------

inline const MainFrame &TextureBuilderApp ::getMainFrame() const
{
	return *NON_NULL(m_mainFrame);
}

// ======================================================================

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
//}}AFX_INSERT_LOCATION

// ======================================================================

#endif
