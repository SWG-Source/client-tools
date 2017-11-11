// ======================================================================
//
// SwgCuiAvatarCustomizationBase.h
// copyright (c) 2001-2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgCuiAvatarCustomizationBase_H
#define INCLUDED_SwgCuiAvatarCustomizationBase_H

// ======================================================================

#include "swgClientUserInterface/SwgCuiAvatarCustomizationBase.h"

//#include "sharedGame/CustomizationManager.h"
#include "sharedGame/CustomizationManager_MorphParameter.h"
#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"

class CuiColorPicker;
class CreatureObject;
class ClientObject;
class UIButton;
class CuiWidget3dObjectListViewer;
class UIVolumePage;
class RangedIntCustomizationVariable;
class CustomizationData;
class UINamespace;
class UIComposite;
class UISliderbar;
class UIComposite;
class UIText;
class CustomizationVariable;
class TangibleObject;

//-----------------------------------------------------------------

class SwgCuiAvatarCustomizationBase : public CuiMediator, public UIEventCallback
{
public:
	struct CustomizationGroup;

	typedef stdvector<std::string>::fwd           HairStyleVector;
	typedef stdvector<CustomizationGroup>::fwd    CustomizationGroupVector;
	typedef stdvector<UIPage *>::fwd              PageVector;
	typedef stdvector<UISliderbar *>::fwd         SliderVector;
	typedef stdvector<CustomizationManager::MorphParameter>::fwd MorphParameterVector;
	typedef stdmap<CustomizationManager::MorphParameter *, float>::fwd  MorphParameterModificationMap;
	typedef stdmap<std::string , int>::fwd        IndexParameterModificationMap;
	typedef stdvector<ClientObject *>::fwd        ClientObjectVector;
	typedef stdset<ClientObject *>::fwd           ClientObjectSet;
	typedef stdvector<UIButton *>::fwd            ButtonVector;

	SwgCuiAvatarCustomizationBase                             (std::string const & className, UIPage & page);
	~SwgCuiAvatarCustomizationBase                            ();

	//-- PS UI callbacks
	virtual void               OnSliderbarChanged             (UIWidget * context);
	virtual void               OnVolumePageSelectionChanged   (UIWidget * context);
	virtual void               OnHoverIn                      (UIWidget * context);
	virtual void               OnHoverOut                     (UIWidget * context);
	virtual void               OnButtonPressed                (UIWidget * context);
	virtual void               update                         (float delta);

	CreatureObject *           getObject                      ();

	void                       setObject                      (ClientObject * obj, int hairSkillModValue);
	void                       setYawOnCustomizationSelection (bool yawOnCustomizationSelection);

	static bool                loadHairStyles                 (const std::string & templateName, HairStyleVector & result);

protected:
	virtual void               performActivate                ();
	virtual void               performDeactivate              ();

protected:
	void                       randomize                        (bool all);
	void                       setCustomizationGroupNameVisible (bool visible);
	void                       SetDisableSubPagesWhenNotSelected(bool disableSubPages);
	UISliderbar *              findMorphSlider                  (const UIWidget & widget, bool & isMorphH, bool & isMorphV, CustomizationManager::MorphParameter *& morphParam, bool outputWarning = true);
	float                      dataValueToFloat                 (long l, float range);
	TangibleObject *           getCurrentHair                   ();
	std::string const &        getGroup                         ();
	virtual void               setGroup                         (std::string const & groupName);
	virtual void               updateSelectedHairColorState     ();
	void                       updateMorphSliders               (bool notify);
	const CustomizationGroup * findHairCustomizationGroup       () const;
	CustomizationGroup *       findHairCustomizationGroup       ();
	void                       resetDefaults                    ();

private:
	//disabled
	                           SwgCuiAvatarCustomizationBase  ();
	                           SwgCuiAvatarCustomizationBase  (const SwgCuiAvatarCustomizationBase & rhs);
	SwgCuiAvatarCustomizationBase &   operator=               (const SwgCuiAvatarCustomizationBase & rhs);

private:
	void                       setupCustomizationData         (ClientObject * object, int hairSkillModValue);
	void                       setupHairStyles                (const std::string & templateName, int hairSkillModValue);
	void                       populatePatterns               ();
	void                       updatePlayerPattern            ();
	void                       setupCustomizationGroups       (const ClientObject & object);
	void                       setupCustomizationGroupButtons ();

	const CustomizationGroup * findCustomizationGroup         (const std::string & groupName) const;
	CustomizationGroup *       findCustomizationGroup         (const std::string & groupName);

	void                       setupPageMorph                 ();
	void                       attachToMorphWidgets           (PageVector & pages, SliderVector & sliders, const UIPage & pageMorph) const;
	void                       manageMorphSliderCallbacks     (bool onOff);
	void                       manageMorphSliderCallbacks     (SliderVector & sliders, PageVector & pages, bool onOff);
	void                       detachFromMorphWidgets         ();


	void                       saveDefaults                   ();

	void                       findHairObjects                (ClientObjectVector & cov) const;

	void                       randomizeHairColors            (const std::string & varName, int value = -1);

	void                       createCustomizationLink        (TangibleObject & wearable);

	void                       setIsCreation                  (bool isCreation);

protected:
	UIVolumePage *                   m_hairVolumePage;
	CuiColorPicker *                 m_colorPicker [2];
	PageVector *                     m_morphSliderPagesH;
	SliderVector *                   m_morphSlidersH;
	PageVector *                     m_morphSliderPagesV;
	SliderVector *                   m_morphSlidersV;
	CuiWidget3dObjectListViewer *    m_objectViewer;

private:
	UIComposite *                    m_compositeGroups;
	UIButton *                       m_sampleGroupButton;

	UIPage *                         m_pageMorph;
	UIComposite *                    m_compositeColorPicker;

	UIPage *                         m_pageHair;

	Unicode::String                  m_dataFolder;
	HairStyleVector *                m_hairStyleVector;
	RangedIntCustomizationVariable * m_patternVar;
	CustomizationData *              m_cdata;
	CustomizationGroupVector *       m_customizationGroups;
	MorphParameterModificationMap *  m_morphParameterModificationMap;

	bool                             m_morphInProgress;
	bool                             m_ignoreHairChange;

	std::string                      m_currentGroup;
	CustomizationManager::MorphParameter * m_curMorphParameter;
	CustomizationManager::MorphParameter * m_lastModifiedMorphParameter;

	UIWidget *                       m_sampleHairBox;

	float                            m_defaultCameraYaw;

	UIText *                         m_textCurrentGroup;

	bool                             m_allowBald;

	int                              m_defaultHairIndex;
	
	ClientObjectSet *                m_ownedHairSet;

	bool                             m_transitionFinished;

	int                              m_lastShouldUpdateYawFromColorIndex;

	bool                             m_customizationGroupNameVisible;
	bool                             m_disableSubPagesWhenNotSelected;

	ButtonVector *                   m_groupButtons;

	bool                             m_creation;
	bool                             m_yawOnCustomizationSelection;
};

// ======================================================================

#endif

