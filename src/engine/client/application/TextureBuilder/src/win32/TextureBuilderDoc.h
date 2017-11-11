// ======================================================================
//
// TextureBuilderDoc.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#if !defined(AFX_TEXTUREBUILDERDOC_H__EF6D5B81_20F5_4611_870B_B9FB004B7901__INCLUDED_)
#define AFX_TEXTUREBUILDERDOC_H__EF6D5B81_20F5_4611_870B_B9FB004B7901__INCLUDED_

// ======================================================================

#if _MSC_VER > 1000
#pragma once
#endif

// ======================================================================

class CommandElement;
class ConstructionTreeView;
class RegionElement;
class DestinationTextureElement;
class Element;
class Iff;
class Texture;
class TextureBakeView;
class VariableEditView;
class VariableElement;

// ======================================================================

class TextureBuilderDoc : public CDocument
{
public:

	typedef stdvector<Element*>::fwd          ElementVector;
	typedef stdvector<VariableElement*>::fwd  VariableElementVector;

	struct Selection
	{
		float m_x0;
		float m_y0;
		float m_x1;
		float m_y1;
	};

public:

	virtual ~TextureBuilderDoc();

	ElementVector       &getElements();
	const ElementVector &getElements() const;

	void                 getProjectRelativePath(const char *fullPathname, std::string &relativePathname) const;

	void                 addSingleTexture(const std::string &relativePathname);
	void                 addDrawTextureCommand();

	void                 notifyElementDragAndDrop(Element &dragSourceElement, Element &dropTargetElement);
	void                 notifyElementLeftDoubleClick(Element &element);
	void                 notifyVariableElementModified(const VariableElement &variableElement);
	void                 notifyElementSelected(Element &element);

	Texture             &getTexture();
	const Texture       &getTexture() const;

	int                  getTextureWidth() const;
	int                  getTextureHeight() const;

	void                 layoutViews();

	bool                 hasActiveSelection() const;
	void                 getActiveSelection(Selection &selection) const;
	void                 setActiveSelection(const Selection &selection);

	void                 requestTextureBake();

	void                 setHueMode(int value);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// ClassWizard generated virtual function overrides
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//{{AFX_VIRTUAL(TextureBuilderDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	//}}AFX_VIRTUAL

protected:

	TextureBuilderDoc();
	DECLARE_DYNCREATE(TextureBuilderDoc)

	// ----------------------------------------------------------------------
	// Generated message map functions
	// ----------------------------------------------------------------------
	//{{AFX_MSG(TextureBuilderDoc)
	afx_msg void OnViewBakeTexture();
	afx_msg void OnTextureSourceModeOneTexture();
	afx_msg void OnTextureSourceModeTwoTextureBlend();
	afx_msg void OnAddTextureArray1d();
	afx_msg void OnTextureArray1dAdd();
	afx_msg void OnTextureWriteModeOverwrite();
	afx_msg void OnTextureWriteModeAlphaBlend();
	afx_msg void OnRenderElementSnapSize();
	afx_msg void OnFileExportAs();
	afx_msg void OnHueModeNone();
	afx_msg void OnHueModeOneColor();
	afx_msg void OnTexture1dElementMoveUp();
	afx_msg void OnTexture1dElementMoveDown();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

	void                  getProjectDirectory(std::string &directory) const;

	void                  displayTextureBakeView(bool visible);
	void                  displayVariableEditView(bool visible);

	void                  viewAppendElementToParent(const Element &parent, Element &element, bool ensureVisible) const;
	void                  viewUpdateElement(const Element &element, bool ensureVisible) const;
	void                  viewNotifyNewActiveSelection() const;
	void                  viewNotifyNoActiveSelection() const;

	ConstructionTreeView *getConstructionTreeView();
	TextureBakeView      *getTextureBakeView();
	VariableEditView     *getVariableEditView();

	CommandElement       *whichCommandHasDescendant(const Element &element);

	void                  setTextureSourceMode(int value);
	void                  setTextureWriteMode(int value);

	void                  collectVariables();

	void                  bakeTexture() const;

	bool                  loadWorkspace_0000(Iff &iff);
	bool                  loadWorkspace_0001(Iff &iff);

	void                  exportTextureRenderer(const std::string &pathname) const;
	void                  createDestinationTexture();

private:

	std::auto_ptr<std::string> m_projectPathname;
	std::auto_ptr<std::string> m_exportPathname;

	ElementVector             *m_elements;
	VariableElementVector     *m_variableElements;

	DestinationTextureElement *m_destinationTextureElement;
	Element                   *m_textureGroupElement;
	Element                   *m_commandGroupElement;

	mutable bool               m_textureBakeRequested;
	mutable Texture           *m_texture;

	RegionElement             *m_activeSelection;

private:
	// disabled
	TextureBuilderDoc(const TextureBuilderDoc&);
	TextureBuilderDoc &operator =(const TextureBuilderDoc&);
};

// ======================================================================

inline TextureBuilderDoc::ElementVector &TextureBuilderDoc::getElements()
{
	return *m_elements;
}

// ----------------------------------------------------------------------

inline const TextureBuilderDoc::ElementVector &TextureBuilderDoc::getElements() const
{
	return *m_elements;
}

// ----------------------------------------------------------------------

inline bool TextureBuilderDoc::hasActiveSelection() const
{
	return (m_activeSelection != 0);
}

// ======================================================================

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
//}}AFX_INSERT_LOCATION

#endif
