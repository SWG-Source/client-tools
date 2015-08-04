// ======================================================================
//
// ViewerDoc.h
// copyright 1999, Bootprint Enterainment
// copyright 2002, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_ViewerDoc_H
#define INCLUDED_ViewerDoc_H

// ======================================================================

#include "DialogMapAnimation.h"
#include "SkeletalAnimationKey.h"

class Appearance;
class CStringArray;
class CViewerApp;
class Iff;
class Light;
class MessageQueue;
class Object;
class SkeletalAppearance2;
class SkeletalAppearanceTemplateView;
class SkeletalAppearanceInstanceData;
class SkeletalAppearanceTemplate;
class Texture;
class TextureRenderer;

// ======================================================================

class CViewerDoc : public CDocument
{
public:

	typedef bool (CViewerDoc::*FileTypeLoader)(const char *filename);

protected: 

	CViewerDoc();
	DECLARE_DYNCREATE(CViewerDoc)

	//{{AFX_VIRTUAL(CViewerDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	virtual void OnCloseDocument();
	//}}AFX_VIRTUAL

public:

	virtual ~CViewerDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	BOOL    load (LPCTSTR lpszPathName);
	Object * attachStandardAppearanceToHardpoint(Object * parent, char const * const filename, char const * const hardpointName);
	void removeAttachedObject(Object * child);

public:

	static void                     install();
	static void                     remove();

	static void setBuildingAsynchronousLoaderData(bool buildingAsynchronousLoaderData);

private:

	static FileTypeLoader           getFileTypeLoader(const char *extensionNoDot);

public:

	Object*                         getObject (void) const;
	const CString&                  getFileName () const;

	void                            alter(real elapsedTime);
	void                            stopAnimation(void);

	void                            processKeyPress(UINT key);
	void                            reload (void);

	void                            updateAnimationView (const CView* newView, bool force=false);

	void							updateAnimationTreeView(const CView* newView, bool force=false);
	void							playAnim(const char* filename, bool queueAnimation, bool usePrimarySkeleton);

	void                            newAnimation (void);
	void                            removeAnimation (char key);
	void                            editAnimation (char key);

	bool                            hasVariableSetView() const;
	bool                            hasSkeletalAppearanceTemplateView() const;

	void                            saveSkeletalAppearanceTemplate(const char *filename);
	SkeletalAppearanceTemplate     *getSkeletalAppearanceTemplate();
	const CString                  &getSkeletalAppearanceTemplateFilename() const;

	void                            closeSkeletalAppearanceTemplateView();
	void                            closeVariableSetView();
	void                            setSkeletalAppearanceTemplate(SkeletalAppearanceTemplate *appearanceTemplate);

	const CString                  &getSkeletalAppearanceWorkspaceFilename() const;
	void                            setSkeletalAppearanceWorkspaceFilename(const CString &filename);
	bool                            saveSkeletalAppearanceWorkspace(const CString &filename);
	bool                            loadSkeletalAppearanceWorkspace(const char *filename);

	Texture                        *getTexture();
	TextureRenderer                *getTextureRenderer();
	void                            renderTextures();

	void                            rebuildAppearance();

	void                            writeObjectTemplateCustomizationData(bool allowOverwrite);
	void                            debugDump ();

private:

	bool                            loadSkeletalAppearanceWorkspace_0001(Iff *iff);
	bool                            loadSkeletalAppearanceWorkspace_0002(Iff *iff);

	bool                            loadShaderTemplate(const char *filename);
	bool                            loadTexture(const char *filename);
	bool                            loadTextureRendererTemplate(const char *filename);
	bool                            loadSkeletonTemplate(const char *filename);
	bool                            loadMeshGenerator(const char *filename);
	bool                            loadSkeletalAppearanceTemplate(const char *filename);
	bool                            loadStandardAppearance(const char *filename);
	bool                            loadPortalObject(const char *filename);
	bool                            loadHardpointHierarchy(const char *filename);
	void                            loadAllHardpoints(Object * object, int depth);

	void                            mapSkeletalAnimation (DialogMapAnimation& dlg);
	void                            clearSkeletalAnimationList (void);

	void                            skeletalAnimationSet_load0000 (Iff& iff);
	void                            skeletalAnimationSet_load0001 (Iff& iff);
	void                            skeletalAnimationSet_load0002 (Iff& iff);

	SkeletalAppearanceTemplateView *getSkeletalAppearanceTemplateView() const;

	void                            resetSkeletalAppearanceWorkspace();
	
	CViewerApp                     *getOurApp() const;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Generated message map functions
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
public:
	afx_msg void                    OnViewSkeletalAppearanceTemplate();

protected:
	//{{AFX_MSG(CViewerDoc)
	afx_msg void OnAnimationAddkeymappingSkeletalanimation();
	afx_msg void OnUpdateAnimationAddkeymappingSkeletalanimation(CCmdUI* pCmdUI);
	afx_msg void OnAnimationSkeletalanimationStopwearing();
	afx_msg void OnUpdateAnimationSkeletalanimationStopwearing(CCmdUI* pCmdUI);
	afx_msg void OnAnimationSkeletalanimationWear();
	afx_msg void OnUpdateAnimationSkeletalanimationWear(CCmdUI* pCmdUI);
	afx_msg void OnAnimationMeshanimationLoadanimationset();
	afx_msg void OnUpdateAnimationMeshanimationLoadanimationset(CCmdUI* pCmdUI);
	afx_msg void OnAnimationMeshanimationSaveanimationset();
	afx_msg void OnUpdateAnimationMeshanimationSaveanimationset(CCmdUI* pCmdUI);
	afx_msg void OnAnimationSkeletalanimationLoadanimationset();
	afx_msg void OnUpdateAnimationSkeletalanimationLoadanimationset(CCmdUI* pCmdUI);
	afx_msg void OnAnimationSkeletalanimationSaveanimationset();
	afx_msg void OnUpdateAnimationSkeletalanimationSaveanimationset(CCmdUI* pCmdUI);
	afx_msg void OnAnimationSkeletalanimationAttachtoselectedbone();
	afx_msg void OnUpdateAnimationSkeletalanimationAttachtoselectedbone(CCmdUI* pCmdUI);
	afx_msg void OnAnimationSkeletalanimationDetach();
	afx_msg void OnUpdateAnimationSkeletalanimationDetach(CCmdUI* pCmdUI);
	afx_msg void OnAnimationSkeletalanimationSelectshadowvolume();
	afx_msg void OnUpdateAnimationSkeletalanimationSelectshadowvolume(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewRenderSkeleton(CCmdUI* pCmdUI);
	afx_msg void OnViewRenderSkeleton();
	afx_msg void OnUpdateViewRenderMesh(CCmdUI* pCmdUI);
	afx_msg void OnViewRenderMesh();
	afx_msg void OnUpdateViewVariableSetView(CCmdUI* pCmdUI);
	afx_msg void OnViewVariableSetView();
	afx_msg void OnUpdateViewSkeletalAppearanceTemplate(CCmdUI* pCmdUI);
	afx_msg void OnSkeletonTemplateAdd();
	afx_msg void OnMeshGeneratorAdd();
	afx_msg void OnFileWriteObjTemplateCustomizationData();
	afx_msg void OnUpdateFileWriteCustomizationData(CCmdUI* pCmdUI);
	afx_msg void OnButtonCenterObject();
	afx_msg void OnButtonToggleLocomotion();
	afx_msg void OnUpdateButtonToggleLocomotion(CCmdUI* pCmdUI);
	afx_msg void OnButtonDebugdump();
	afx_msg void OnFileWriteCdfBakedWearableData();
	afx_msg void OnSaveHardpointHierarchy();
	afx_msg void OnLoadAllHardpoints();
	afx_msg void OnUpdateLoadAllHardpoints(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

	struct FileTypeLoaderContainer;

private:

	static SkeletalAppearance2 *createAppearanceFromMeshGenerator(const char *filename);
	static void                 setupObjectCustomizationData(Object *object, Appearance *appearance);


private:

	static bool                     ms_installed;
	static FileTypeLoaderContainer *ms_fileTypeLoaders;

public:
	typedef stdvector<std::string>::fwd AnimationFileList;

private:

	typedef stdvector<Light*>::fwd LightList;

	SkeletalAnimationKey::Map       skeletalAnimationKeyMap;

	CString                         defaultAnimationFileName;
	float                           defaultBlendTime;
	CString                         defaultKey;
	float                           defaultTimeScale;
	int                             defaultPlayModeIndex;
	float                           defaultWeight;
	BOOL                            defaultIsLooping;
	BOOL                            defaultIsInterruptible;
	CString                         defaultSoundFilename;

	int                             badAlterCount;

	MessageQueue*                   messageQueue;

	int                             hardpointIndex;

	Object*                         object;
	CString                         filename;

	SkeletalAppearanceTemplate     *m_skeletalAppearanceTemplate;
	CString                         m_skeletalAppearanceTemplateFilename;

	CString                         m_skeletalAppearanceWorkspaceFilename;
	Texture                        *m_texture;
	TextureRenderer                *m_textureRenderer;

	bool                            m_fatalThrowsExceptionsOnLoad;
	LightList                      *m_lightList;
	AnimationFileList              *m_animationFileList[2];

	bool                            m_locomotionEnabled;
};

// ======================================================================

inline Object* CViewerDoc::getObject (void) const
{
	return object;
}

// ----------------------------------------------------------------------

inline const CString& CViewerDoc::getFileName () const
{
	return filename;
}

// ----------------------------------------------------------------------

inline void CViewerDoc::setSkeletalAppearanceTemplate(SkeletalAppearanceTemplate *appearanceTemplate)
{
	m_skeletalAppearanceTemplate = appearanceTemplate;
}

// ----------------------------------------------------------------------

inline SkeletalAppearanceTemplate *CViewerDoc::getSkeletalAppearanceTemplate()
{
	return m_skeletalAppearanceTemplate;
}

// ----------------------------------------------------------------------

inline const CString &CViewerDoc::getSkeletalAppearanceTemplateFilename() const
{
	return m_skeletalAppearanceTemplateFilename;
}

// ----------------------------------------------------------------------

inline const CString &CViewerDoc::getSkeletalAppearanceWorkspaceFilename() const
{
	return m_skeletalAppearanceWorkspaceFilename;
}

// ----------------------------------------------------------------------

inline void CViewerDoc::setSkeletalAppearanceWorkspaceFilename(const CString &newFilename)
{
	m_skeletalAppearanceWorkspaceFilename = newFilename;
}

// ----------------------------------------------------------------------

inline TextureRenderer *CViewerDoc::getTextureRenderer()
{
	return m_textureRenderer;
}

// ----------------------------------------------------------------------

inline Texture *CViewerDoc::getTexture()
{
	return m_texture;
}

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 
