//======================================================================
//
// SwgCuiCraftExperiment.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiCraftExperiment_H
#define INCLUDED_SwgCuiCraftExperiment_H

//======================================================================

#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"

class CuiWidget3dObjectListViewer;
class SwgCuiInventoryInfo;
class UIButton;
class UIColorEffector;
class UIComposite;
class UIPage;
class UIText;
class CuiMessageBox;
class UIRunner;
class UISliderbar;
class ClientObject;

namespace MessageDispatch
{
	class Callback;
};

//----------------------------------------------------------------------

class SwgCuiCraftExperiment :
public CuiMediator,
public UIEventCallback
{
public:

	class Attrib;

	explicit                    SwgCuiCraftExperiment (UIPage & page);

	virtual void                performActivate       ();
	virtual void                performDeactivate     ();

	virtual void                OnButtonPressed       (UIWidget *context );
	virtual void				OnSliderbarChanged    (UIWidget *context );
	bool                        OnMessage             (UIWidget *context, const UIMessage & msg);

	int                         fetchPoints           (int requestedCount);
	int                         releasePoints         (int requestedCount);
	void                        update                (float deltaTimeSecs);
	bool                        close                 ();

	void                        setInitializePage     (bool init);

private:
	                           ~SwgCuiCraftExperiment ();
	                            SwgCuiCraftExperiment (const SwgCuiCraftExperiment &);
	SwgCuiCraftExperiment &     operator=             (const SwgCuiCraftExperiment &);

	typedef stdvector<Attrib *>::fwd AttribMediatorVector;

	void                        releaseMediators           ();
	void                        onCustomize                (const int & );
	void                        onExperiment               (const int &);
	void                        onSessionEnded             (const bool &);
	void                        onComplexityChange         (const ClientObject & obj);

	void                        updatePoints               ();
	void                        updateAttribs              ();

	int                         computeSpentPoints         ();
	void                        populateAttribs            ();
	Attrib *                    createAttribMediator       (int attribIndex);
	void                        recoverAllExperimentPoints ();
	void                        updateComplexity           ();

	void                        onMessageBoxClosedNext     (const CuiMessageBox& box);
	void						onExperimentBoxClosedNext  (const CuiMessageBox& box);

	UIButton *                  m_buttonNext;
	UIButton *                  m_buttonExperiment;
	bool                        m_transition;

	MessageDispatch::Callback * m_callback;

	AttribMediatorVector *      m_attribMediators;

	UIPage *                    m_attribSample;

	UIComposite *               m_compositeAttribs;

	UIComposite *               m_compositePoints;
	UIPage *                    m_samplePoint;

	UIWidget *                  m_pagePointsCurParent;
	UIPage *                    m_pageRiskCur;
	UIPage *                    m_pageComplexityCur;
	UIText *                    m_textPointsCur;
	UIText *                    m_textComplexityCur;
	UIText *                    m_textRiskPercent;
	UIRunner *                  m_runnerRisk;

	int                         m_remainingPoints;
	bool                        m_updatePoints;
	
	UIColorEffector *           m_effectorRisk;
	UIColorEffector *           m_effectorComplexity;

	SwgCuiInventoryInfo *       m_mediatorInfo;
	CuiWidget3dObjectListViewer * m_viewer;

	bool                        m_sessionEnded;

	UISliderbar*				m_coreLevelSlider;
	UIText*                     m_minCoreLevel;
	UIText*                     m_maxCoreLevel;
	UIPage*						m_experimentPage;
	UIPage*						m_coreLevelPage;

	bool                        m_coreLevelSet;

	UISize						m_experimentDefaultSize;

	bool                        m_initializePage;
};

//======================================================================

#endif
