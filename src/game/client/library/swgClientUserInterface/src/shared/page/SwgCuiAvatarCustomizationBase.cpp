// ======================================================================
//
// SwgCuiAvatarCustomizationBase.cpp
// copyright (c) 2001-2004 Sony Online Entertainment
//
// ======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiAvatarCustomizationBase_CustomizationGroup.h"

#include "clientGame/ClientImageDesignerManager.h"
#include "clientGame/ContainerInterface.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientUserInterface/CuiCharacterHairManager.h"
#include "clientUserInterface/CuiColorPicker.h"
#include "clientUserInterface/CuiIconManager.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiSoundManager.h"
#include "clientUserInterface/CuiSounds.h"
#include "clientUserInterface/CuiStringIds.h"
#include "clientUserInterface/CuiTransition.h"
#include "clientUserInterface/CuiWidget3dObjectListViewer.h"
#include "sharedCollision/BoxExtent.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/PerformanceTimer.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedGame/CustomizationManager.h"
#include "sharedGame/SharedImageDesignerManager.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/CustomizationData.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedObject/PaletteColorCustomizationVariable.h"
#include "sharedObject/SlottedContainer.h"
#include "sharedRandom/Random.h"
#include "swgClientUserInterface/SwgCuiAvatarCreationHelper.h"
#include "swgClientUserInterface/SwgCuiMediatorTypes.h"

#include "UIButton.h"
#include "UIComposite.h"
#include "UIData.h"
#include "UIDataSource.h"
#include "UINamespace.h"
#include "UIPage.h"
#include "UISliderbar.h"
#include "UIText.h"
#include "UIVolumePage.h"
#include "UnicodeUtils.h"

#include <set>

// ======================================================================

namespace SwgCuiAvatarCustomizationBaseNamespace
{
	Unicode::String const cms_textPre = Unicode::narrowToWide("@ui_cust:");

	namespace Properties
	{
		const UILowerString DefaultViewerPitch = UILowerString ("DefaultViewerPitch");
		const UILowerString DefaultViewerYaw   = UILowerString ("DefaultViewerYaw");
		const UILowerString IsOnHair           = UILowerString ("IsOnHair");
	}

	inline long floatToDataValue (const float r, const float range)
	{
		return static_cast<long> (r * range);
	}

	inline long floatToDataValue (const float r, const int range)
	{
		return std::min (static_cast<int>(r * (range + 1)), range);
	}

	namespace SlotNames
	{
		const std::string s_hair ("hair");
	}

	//----------------------------------------------------------------------

	CuiWidget3dObjectListViewer * findViewer (UIVolumePage const * const page, Object const * const object, int & index)
	{
		index = 0;
		const UIBaseObject::UIObjectList & olist = page->GetChildrenRef ();
		for (UIBaseObject::UIObjectList::const_iterator it = olist.begin (); it != olist.end (); ++it)
		{
			CuiWidget3dObjectListViewer * const viewer = dynamic_cast<CuiWidget3dObjectListViewer *>(*it);
			if (viewer)
			{
				if (viewer->getLastObject () == object)
					return viewer;

				++index;
			}
		}

		index = -1;
		return 0;
	}

	//----------------------------------------------------------------------

	void setupPageMorphWidgets (SwgCuiAvatarCustomizationBase::PageVector & pages, SwgCuiAvatarCustomizationBase::MorphParameterVector const & v)
	{
		const size_t count = v.size ();
		size_t index = 0;

		{
			for (SwgCuiAvatarCustomizationBase::PageVector::iterator it = pages.begin (); it != pages.end (); ++it, ++index)
			{
				UIPage * const page = *it;
				
				if (index >= count)
				{
					page->SetVisible (false);
				}
				else
				{
					const CustomizationManager::MorphParameter & morphParam = v [index];
					
					page->SetName (Unicode::wideToNarrow (morphParam.name));
					page->SetVisible (true);
				}
			}
		}
	}

	//----------------------------------------------------------------------

	void updateSliderVector (SwgCuiAvatarCustomizationBase::SliderVector & sliders, SwgCuiAvatarCustomizationBase::MorphParameterVector const & v, bool const notify)
	{
		size_t index = 0;
		const size_t numSliders = sliders.size ();
		for (SwgCuiAvatarCustomizationBase::MorphParameterVector::const_iterator it = v.begin (); it != v.end (); ++it, ++index)
		{
			if (index >= numSliders)
				break;
			
			const CustomizationManager::MorphParameter & morphParam = *it;

			UISliderbar * const slider = sliders [index];
	
			static const UILowerString prop_ptt = UILowerString ("parent.text.text");

			Unicode::String sliderName = cms_textPre + morphParam.name;
			slider->SetProperty (prop_ptt, sliderName );

			if (morphParam.isDiscrete)
			{
				slider->SetUpperLimit (morphParam.discreteRange, notify);
				slider->SetValue (floatToDataValue (morphParam.current, morphParam.discreteRange), notify);
			}
			else
			{
				slider->SetUpperLimit (100, notify);
				slider->SetValue (floatToDataValue (morphParam.current, static_cast<float>(morphParam.discreteRange)), notify);
			}
		}
	}
 
	//----------------------------------------------------------------------

	bool s_debugPrintCustomizationGroups = false;
	bool s_debugShowCustomizationPaths = false;
}

using namespace SwgCuiAvatarCustomizationBaseNamespace;

// ----------------------------------------------------------------------

SwgCuiAvatarCustomizationBase::SwgCuiAvatarCustomizationBase (std::string const & className, UIPage & newPage) :
CuiMediator            (className.c_str(), newPage),
UIEventCallback        (),
m_hairVolumePage       (0),
m_compositeGroups      (0),
m_sampleGroupButton    (0),
m_pageMorph            (0),
m_compositeColorPicker (0),
m_pageHair             (0),
m_objectViewer         (0),
m_dataFolder           (),
m_hairStyleVector      (new HairStyleVector),
m_patternVar           (0),
m_cdata                (0),
m_customizationGroups  (new CustomizationGroupVector),
m_morphSliderPagesH    (new PageVector),
m_morphSlidersH        (new SliderVector),
m_morphSliderPagesV    (new PageVector),
m_morphSlidersV        (new SliderVector),
m_morphParameterModificationMap (new MorphParameterModificationMap),
m_morphInProgress      (0),
m_ignoreHairChange     (0),
m_currentGroup         (),
m_curMorphParameter    (0),
m_lastModifiedMorphParameter (0),
m_sampleHairBox        (0),
m_defaultCameraYaw     (0.0f),
m_textCurrentGroup     (0),
m_allowBald            (true),
m_defaultHairIndex     (0),
m_ownedHairSet         (new ClientObjectSet),
m_transitionFinished   (false),
m_lastShouldUpdateYawFromColorIndex (-1),
m_customizationGroupNameVisible(false),
m_disableSubPagesWhenNotSelected(false),
m_groupButtons(new ButtonVector),
m_creation(true),
m_yawOnCustomizationSelection(false)
{
	//-- the composite color picker must be sized prior to setting a palette in it
	getPage ().ForcePackChildren ();
	DebugFlags::registerFlag(s_debugPrintCustomizationGroups,       "SwgClientUserInterface", "customizationGroups");
	DebugFlags::registerFlag(s_debugShowCustomizationPaths,         "SwgClientUserInterface", "customizationPaths");

	const UIData * codeData = NON_NULL (getCodeData ());

	if (!codeData->GetProperty (UILowerString ("parameterDataFolder"), m_dataFolder))
	{
		DEBUG_FATAL (true, ("Unable to load parameter data folder for avatar cust.\n"));
	}

	if (!m_dataFolder.empty () && m_dataFolder [m_dataFolder.size () - 1] != '.')
		IGNORE_RETURN (m_dataFolder.append (1, '.'));

	getCodeDataObject (TUIComposite,  m_compositeGroups,   "compositeGroups");
	getCodeDataObject (TUIButton,     m_sampleGroupButton, "sampleGroupButton");
	getCodeDataObject (TUIVolumePage, m_hairVolumePage,    "hairVolumePage");
	
	getCodeDataObject (TUIPage,       m_pageMorph,       "pageMorph");
	getCodeDataObject (TUIPage,       m_pageHair,        "pageHair");
	
	getCodeDataObject (TUIComposite,  m_compositeColorPicker,  "compositeColorPicker");

	getCodeDataObject (TUIText,       m_textCurrentGroup,      "textCurrentGroup");
	getCodeDataObject (TUIWidget,     m_sampleHairBox,         "sampleHairBox");

	UIPage * const pageColorPicker [2] =
	{
		NON_NULL (GET_UI_OBJ ((*m_compositeColorPicker), UIPage, "picker0")),
		NON_NULL (GET_UI_OBJ ((*m_compositeColorPicker), UIPage, "picker1"))
	};

	UIWidget * viewer = 0;
	getCodeDataObject (TUIWidget,     viewer,            "viewer");
	m_objectViewer = NON_NULL (dynamic_cast<CuiWidget3dObjectListViewer *>(viewer));
	m_objectViewer->SetLocalTooltip (CuiStringIds::tooltip_viewer_3d_controls.localize ());
	m_objectViewer->SetPropertyFloat (Properties::DefaultViewerPitch, m_objectViewer->getCameraPitch ());
	m_objectViewer->SetPropertyFloat (Properties::DefaultViewerYaw,   m_objectViewer->getCameraYaw   ());

	m_objectViewer->setCameraLodBias (2.0f);
	m_objectViewer->setAutoZoomOutOnly       (false);
	m_objectViewer->setCameraZoomInWhileTurn (true);
	m_objectViewer->setAlterObjects          (false);
	m_objectViewer->setCameraLookAtCenter    (false);
	m_objectViewer->setDragYawOk             (true);
	m_objectViewer->setPaused                (false);
	m_objectViewer->SetDragable              (false);	
	m_objectViewer->SetContextCapable        (true, false);
	m_objectViewer->setRotateSpeed           (0.0f);
	m_objectViewer->setCameraForceTarget     (false);
	m_objectViewer->setCameraTransformToObj  (true);
	m_objectViewer->setCameraLodBias         (3.0f);
	m_objectViewer->setCameraLodBiasOverride (true);
	m_objectViewer->setRotationSlowsToStop (true);

	m_objectViewer->setCameraForceTarget   (false);
	m_objectViewer->setCameraAutoZoom      (true);

	pageColorPicker [0]->SetProperty (CuiColorPicker::Properties::AutoSizePaletteCells, Unicode::narrowToWide ("true"));
	m_colorPicker   [0]= new CuiColorPicker (*pageColorPicker [0]);
	m_colorPicker   [0]->setText (Unicode::emptyString);
	m_colorPicker   [0]->setAutoForceColumns (true);
	m_colorPicker   [0]->fetch ();

	pageColorPicker [1]->SetProperty (CuiColorPicker::Properties::AutoSizePaletteCells, Unicode::narrowToWide ("true"));
	m_colorPicker   [1]= new CuiColorPicker (*pageColorPicker [1]);
	m_colorPicker   [1]->setText (Unicode::emptyString);
	m_colorPicker   [1]->setAutoForceColumns (true);

	m_colorPicker   [1]->fetch ();

	m_sampleGroupButton->SetVisible (false);

	m_sampleHairBox->SetVisible   (false);

	m_defaultCameraYaw = m_objectViewer->getCameraYaw ();

	UIPage * const page_h = NON_NULL (dynamic_cast<UIPage *>(m_pageMorph->GetChild ("h")));
	UIPage * const page_v = NON_NULL (dynamic_cast<UIPage *>(m_pageMorph->GetChild ("v")));

	attachToMorphWidgets (*m_morphSliderPagesH, *m_morphSlidersH, *page_h);
	attachToMorphWidgets (*m_morphSliderPagesV, *m_morphSlidersV, *page_v);

	CuiColorPicker::setupPaletteColumnData(CustomizationManager::getPaletteColumnData());

	//-----------------------------------------------------------------
	//-- setup the customization data

	setupCustomizationData (0, 0);

	registerMediatorObject (*m_hairVolumePage,    true);
}

// ----------------------------------------------------------------------

SwgCuiAvatarCustomizationBase::~SwgCuiAvatarCustomizationBase ()
{
	detachFromMorphWidgets ();

	delete m_morphSliderPagesH;
	m_morphSliderPagesH = 0;
	delete m_morphSlidersH;
	m_morphSlidersH = 0;
	delete m_morphSliderPagesV;
	m_morphSliderPagesV = 0;
	delete m_morphSlidersV;
	m_morphSlidersV = 0;

	delete m_morphParameterModificationMap;
	m_morphParameterModificationMap  = 0;

	delete m_customizationGroups;
	m_customizationGroups = 0;

	delete m_hairStyleVector;
	m_hairStyleVector = 0;

	m_colorPicker [0]->release ();
	m_colorPicker [0] = 0;

	m_colorPicker [1]->release ();
	m_colorPicker [1] = 0;

	m_curMorphParameter = 0;

	m_objectViewer     = 0;

	delete m_ownedHairSet;
	m_ownedHairSet     = 0;

	delete m_groupButtons;
	m_groupButtons = 0;

	m_hairVolumePage->Clear ();
	m_hairVolumePage = NULL;
}

//-----------------------------------------------------------------

void SwgCuiAvatarCustomizationBase::performActivate   ()
{
	setGroup (std::string ());

	manageMorphSliderCallbacks (true);

	{
		float f = 0.0f;
		if (m_objectViewer->GetPropertyFloat (Properties::DefaultViewerPitch, f))
			m_objectViewer->setCameraPitch (f);
		if (m_objectViewer->GetPropertyFloat (Properties::DefaultViewerYaw, f))
			m_objectViewer->setCameraYaw (f, true);
	}

	m_objectViewer->setPaused (false);
	m_colorPicker [0]->activate ();
	m_colorPicker [1]->activate ();

	if(!m_disableSubPagesWhenNotSelected)
		m_compositeColorPicker->SetVisible (false);

	if (m_cdata)
	{
		m_cdata->release ();
		m_cdata = 0;
	}

	setIsUpdating (true);

	if (!m_customizationGroups->empty ())
	{
		const CustomizationGroup & group = m_customizationGroups->front ();
		setGroup (Unicode::wideToNarrow (group.name));
	}

	m_transitionFinished = false;
}

//-----------------------------------------------------------------

void SwgCuiAvatarCustomizationBase::performDeactivate ()
{
	std::for_each (m_ownedHairSet->begin (), m_ownedHairSet->end (), PointerDeleter ());
	m_ownedHairSet->clear ();

	m_colorPicker [0]->deactivate ();
	m_colorPicker [1]->deactivate ();

	manageMorphSliderCallbacks (false);

	m_objectViewer->setPaused (true);

	setIsUpdating (false);

	m_objectViewer->clearObjects ();

	m_hairStyleVector->clear ();
	m_compositeGroups->Clear ();

	if (m_cdata)
	{
		m_cdata->release ();
		m_cdata = 0;
	}

	m_patternVar = 0;

	setObject (0, 0);
}

// ----------------------------------------------------------------------

void SwgCuiAvatarCustomizationBase::OnButtonPressed(UIWidget * const context)
{
	//-- a group button (tab)
	if (context->GetParent () == m_compositeGroups)
	{
		const std::string & groupName = context->GetName ();

		UIButton * button = NULL;
		for(ButtonVector::const_iterator i = m_groupButtons->begin(); i != m_groupButtons->end(); ++i)
		{
			button = *i;
			if(button)
				button->SetTextCapital(false);
		}

		button = dynamic_cast<UIButton *>(context);
		if(button)
		{
			button->SetTextCapital(true);
		}

		setGroup (groupName);
	}
}

// ----------------------------------------------------------------------
/**
 * Setup customization data for the appearance associated with the
 * current object.
 *
 * This function setups up any object-specific customization data.
 * It may be called multiple times during the object's lifetime.
 * In particular, it will be called each time a new ObjectTemplate
 * is specified by the user.
 *
 * This function will handle a NULL current object properly.
 */

void SwgCuiAvatarCustomizationBase::setupCustomizationData (ClientObject * const object, int const hairSkillModValue)
{
	m_patternVar = 0;

	if (m_cdata)
	{
		m_cdata->release ();
		m_cdata         = 0;
	}

	m_objectViewer->clearObjects ();
	m_customizationGroups->clear ();

	if (object)
	{
		CreatureObject * const creature = dynamic_cast<CreatureObject *>(object);

		if (creature)
		{
			m_cdata = creature->fetchCustomizationData ();
			creature->clientSetMood (0);
			creature->setAnimationMood ("ui");
		}

		//- the creatures in the pool may have been moved around by other screens
		object->resetRotateTranslate_o2p ();

		TangibleObject * const pedestal = SwgCuiAvatarCreationHelper::getPedestal ();

		if (pedestal)
		{
			pedestal->resetRotateTranslate_o2p ();
			m_objectViewer->addObject (*pedestal);

			const BoxExtent * const box = dynamic_cast<const BoxExtent *>(pedestal->getAppearance ()->getExtent ());

			if (box)
				object->move_o (Vector::unitY * (box->getTop () + SwgCuiAvatarCreationHelper::getPedestalOffsetFromTop ()));
		}
		else
			WARNING (true, ("No pedestal for SwgCuiAvatarCustomizationBase"));

		//do this in imagedesigner, not character creation
		m_objectViewer->setCameraTransformToObj (true);

		m_objectViewer->addObject (*object);
		m_objectViewer->setCameraZoomInWhileTurn(true);

		m_objectViewer->setViewDirty             (true);
		m_objectViewer->setCameraForceTarget     (true);
		m_objectViewer->recomputeZoom            ();
		m_objectViewer->setCameraForceTarget     (false);

		setupCustomizationGroups (*object);
		setupCustomizationGroupButtons ();
	}

	//----------------------------------------------------------------------
	//-- setup hairstyles

	setupHairStyles (object ? object->getObjectTemplateName () : "", hairSkillModValue);
}

//-----------------------------------------------------------------

void SwgCuiAvatarCustomizationBase::OnSliderbarChanged            ( UIWidget * const context)
{
	NOT_NULL (context);

	if (!context->IsA (TUISliderbar))
		return;

	UISliderbar * const slider = NON_NULL (safe_cast<UISliderbar *>(context));

	bool isMorphH = false;
	bool isMorphV = false;

	CustomizationManager::MorphParameter * morphParam = 0;

	if (findMorphSlider (*safe_cast<UISliderbar *>(context), isMorphH, isMorphV, morphParam))
	{
		if (morphParam)
		{
			float value = 0.0f;
			
			value = dataValueToFloat (slider->GetValue (), static_cast<float>(morphParam->discreteRange));

			(*m_morphParameterModificationMap) [morphParam] = value;
		}
	}
} //lint !e818

//----------------------------------------------------------------------

void SwgCuiAvatarCustomizationBase::OnVolumePageSelectionChanged (UIWidget * const context)
{
	if (context == m_hairVolumePage)
	{
		if (m_ignoreHairChange || !m_hairVolumePage->IsVisible ())
			return;

		ClientObject * const player     = safe_cast<ClientObject *>(m_objectViewer->getLastObject ());

		if (!player)
			return;

		if (!m_cdata)
			return;

		TangibleObject * const oldHair     = safe_cast<TangibleObject *>(getCurrentHair ());
		SlottedContainer * const slotted = ContainerInterface::getSlottedContainer (*player);

		if (!slotted)
		{
			WARNING (true, ("no slotted container for player"));
			return;
		}
		Container::ContainerErrorCode tmp = Container::CEC_Success;

		if (oldHair && !slotted->remove (*oldHair, tmp))
		{
			WARNING (true, ("Failed to remove hair from player model\n"));
			return;
		}

		//-- it probably shouldn't be necessary to do this.
		//-- This is required to force TangibleObject's m_containerChangeMap to be correct for
		//-- the next alter of creature
		if (oldHair)
		{
			if (oldHair->alter(1.0f))
				oldHair->conclude();
		}

		int index = 0;
		CuiWidget3dObjectListViewer * const oldViewer = findViewer (m_hairVolumePage, oldHair, index);
		if (oldViewer)
			m_ownedHairSet->insert (oldHair);

		CuiWidget3dObjectListViewer * const viewer = dynamic_cast<CuiWidget3dObjectListViewer *>(m_hairVolumePage->GetLastSelectedChild ());

		if (viewer)
		{
			ClientObject * const hairToWear = safe_cast<ClientObject *>(viewer->getLastObject ());
			m_ownedHairSet->erase (hairToWear);

			if (hairToWear)
			{
				int arrangementIndex = 0;
				Container::ContainerErrorCode tmp = Container::CEC_Success;

				if (slotted->getFirstUnoccupiedArrangement (*hairToWear, arrangementIndex, tmp))
				{
					slotted->add(*hairToWear, arrangementIndex, tmp);
				}
			}
		}

		//-- we removed some hair.  It needs to be re-linked to the player model
		//-- or possibly destroyed , 
		if (oldHair)
		{
			//-- The TangibleObject m_containerChangeMap refers to the removal of objects
			//-- before deleting or re-linking customization on the objects, we must force
			//-- the TangibleObject creature to process the changes

			if (player->alter(0.0f))
				player->conclude();

			createCustomizationLink (*oldHair);

			//-- none of the hair widgets own it, so destroy it
			//-- it should now be safe to delete the previously worn objects
			if (!oldViewer)
				delete oldHair;
		}

		updateSelectedHairColorState ();
	}
}  //lint !e818 //stfu

//----------------------------------------------------------------------

void SwgCuiAvatarCustomizationBase::updateSelectedHairColorState ()
{
	const CustomizationGroup * const group = findCustomizationGroup (m_currentGroup);

	if (group && group->isHair)
	{
		TangibleObject * hair            = dynamic_cast<TangibleObject *>(getCurrentHair ());

		if (!hair)
		{
			ClientObjectVector cov;
			findHairObjects      (cov);

			if (!cov.empty ())
				hair = safe_cast<TangibleObject *>(cov.front ());
		}

		CustomizationData * const cdata        = hair ? hair->fetchCustomizationData () : 0;

		if (hair && cdata && !group->varColors [0].empty ())
		{
			SharedImageDesignerManager::SkillMods const & designerSkills = ClientImageDesignerManager::getSkillModsForDesigner(Game::getPlayerNetworkId());

			if (group->isVarColorHair[0])
			{
				const CuiColorPicker::ObjectWatcherVector idv (m_colorPicker [0]->getLinkedObjects ());

				CustomizationVariable * const var0 = CustomizationManager::findVariable (*cdata, group->varColors [0]);
				m_colorPicker [0]->getPage ().SetVisible (var0 != 0);
				m_colorPicker [0]->getPage ().SetPropertyBoolean(Properties::IsOnHair, true);

				if (var0)
				{
					PaletteArgb const * const palette = m_colorPicker[0]->getPalette();
					if(palette)
					{
						SharedImageDesignerManager::PaletteValues const & paletteValues = SharedImageDesignerManager::getPaletteValuesForPaletteAndSkillMods(palette, designerSkills);
						m_colorPicker[0]->setForceColumns(paletteValues.columns);
						m_colorPicker[0]->setTarget(hair->getNetworkId(), group->varColors[0], 0, paletteValues.maxIndex);
					}
					else
					{
						m_colorPicker[0]->setTarget(hair->getNetworkId(), group->varColors[0], 0, 256);
					}


					Unicode::String const colorPickerName = cms_textPre + group->varColorNames [0];
					m_colorPicker [0]->setText          (colorPickerName);
					m_colorPicker [0]->setLinkedObjects (idv, true);
				}
			}

			if (group->isVarColorHair[1] && !group->varColors[1].empty())
			{
				const CuiColorPicker::ObjectWatcherVector idv (m_colorPicker [1]->getLinkedObjects ());

				CustomizationVariable * const var1 = CustomizationManager::findVariable (*cdata, group->varColors [1]);
				m_colorPicker [1]->getPage ().SetVisible (var1 != 0);
				m_colorPicker [1]->getPage ().SetPropertyBoolean(Properties::IsOnHair, true);

				if (var1)
				{
					PaletteArgb const * const palette = m_colorPicker[1]->getPalette();
					if(palette)
					{
						SharedImageDesignerManager::PaletteValues const & paletteValues = SharedImageDesignerManager::getPaletteValuesForPaletteAndSkillMods(palette, designerSkills);
						m_colorPicker[1]->setForceColumns(paletteValues.columns);
						m_colorPicker[1]->setTarget(hair->getNetworkId(), group->varColors[1], 0, paletteValues.maxIndex);
					}
					else
					{
						m_colorPicker [1]->setTarget(hair->getNetworkId(), group->varColors[1], 0, 256);
					}

					Unicode::String const colorPickerName = cms_textPre + group->varColorNames [1];
					m_colorPicker [1]->setText          (colorPickerName);
					m_colorPicker [1]->setLinkedObjects (idv, true);
				}
			}

			if (m_colorPicker [0]->getPage ().IsVisible () || m_colorPicker [1]->getPage ().IsVisible ())
			{
				m_compositeColorPicker->SetVisible   (true);
				m_compositeColorPicker->Pack ();
			}
			else
				m_compositeColorPicker->SetVisible   (false);
		}

		if (cdata)
			cdata->release ();

		if (m_colorPicker [0])
			m_colorPicker [0]->updateCellSizes ();

		if (m_colorPicker [1] && m_colorPicker [1]->getPage ().IsVisible ())
			m_colorPicker [1]->updateCellSizes ();
	}
}

//----------------------------------------------------------------------

void SwgCuiAvatarCustomizationBase::OnHoverIn (UIWidget * const context)
{
	NOT_NULL (context);

	if (context->GetParent () == m_hairVolumePage)
	{
	}

	else if (context->IsA (TUIPage))
	{
		bool isMorphH = false;
		bool isMorphV = false;

		if (findMorphSlider (*context, isMorphH, isMorphV, m_curMorphParameter))
		{
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiAvatarCustomizationBase::OnHoverOut (UIWidget * const context)
{
	NOT_NULL (context);

	if (context->GetParent () == m_hairVolumePage)
	{
	}

	if (context->IsA (TUIPage))
	{
		bool isMorphH = false;
		bool isMorphV = false;
		
		if (findMorphSlider (*context, isMorphH, isMorphV, m_curMorphParameter))
		{
			m_curMorphParameter = 0;
		}
	}
}

//-----------------------------------------------------------------

void SwgCuiAvatarCustomizationBase::update (float const deltaTimeSecs)
{
	CuiMediator::update (deltaTimeSecs);

	Object * const player     = m_objectViewer->getLastObject ();

	if (!player)
		return;

	if (!m_transitionFinished)
	{
		bool finished = true;

		for (ClientObjectSet::const_iterator it = m_ownedHairSet->begin (); it != m_ownedHairSet->end (); ++it)
		{
			const ClientObject * const hair = *it;
			
			const Appearance * const app = hair->getAppearance ();
			if (app)
			{
				const SkeletalAppearance2 * const skelApp = app->asSkeletalAppearance2 ();
				if (skelApp)
				{
					//-- preload all lod assets
					const int count = skelApp->getDetailLevelCount ();
					for (int i = 0; i < count; ++i)
						finished == skelApp->isDetailLevelAvailable (i) && finished;
				}
			}
		}

		{
			Appearance * const app = player->getAppearance ();
			if (app)
			{
				SkeletalAppearance2 * const skelApp = app->asSkeletalAppearance2 ();
				if (skelApp)
				{
					//-- preload all lod assets
					const int count = skelApp->getDetailLevelCount ();
					for (int i = 0; i < count; ++i)
						finished == skelApp->isDetailLevelAvailable (i) && finished;
					//make sure to set the highest lod level, otherwise zooming won't work when certain bones aren't availabe to zoom in on
					skelApp->setDetailLevel(0);
				}
			}
		}

		if (finished)
		{
			m_transitionFinished = true;
			CuiTransition::signalTransitionReady (CuiMediatorTypes::AvatarCustomize);
		}
	}

	bool viewDirty = false;

	if (m_morphInProgress)
	{
		bool somethingChanged = false;

		const float frameTime = Clock::frameTime ();

		int changeCount = 0;

		PerformanceTimer timer;
		timer.start ();

		for (CustomizationGroupVector::iterator it = m_customizationGroups->begin (); it != m_customizationGroups->end (); ++it)
		{
			CustomizationGroup & group = *it;			
			changeCount += group.incrementTarget (*player, frameTime);
		}

		timer.stop ();

		somethingChanged = (changeCount != 0);
		m_morphInProgress = somethingChanged;

		const CustomizationGroup * const group = findCustomizationGroup (m_currentGroup);

		if (group)
		{
			updateSliderVector (*m_morphSlidersH, group->horizontal, false);
			updateSliderVector (*m_morphSlidersV, group->vertical,   false);
		}

		viewDirty = somethingChanged;
	}

	if (!m_morphParameterModificationMap->empty ())
	{
		CustomizationManager::MorphParameter * last_param = 0;

		for (MorphParameterModificationMap::const_iterator it = m_morphParameterModificationMap->begin (); it != m_morphParameterModificationMap->end (); ++it)
		{
			CustomizationManager::MorphParameter * const param = (*it).first;
			const float            value = (*it).second;

			param->update (*player, value);
			param->target = param->current;
			
			last_param = param;

			if (param->isScale)
				viewDirty = true;
		}

		if (m_curMorphParameter && m_curMorphParameter == last_param)
		{
			if (m_lastModifiedMorphParameter != m_curMorphParameter)
			{
				if (m_curMorphParameter->cameraYawValid && m_yawOnCustomizationSelection)
					m_objectViewer->setCameraYawTarget        (m_curMorphParameter->cameraYaw);
				else
					m_objectViewer->setCameraYawTarget        (m_defaultCameraYaw);

				m_lastModifiedMorphParameter = m_curMorphParameter;
				m_lastShouldUpdateYawFromColorIndex = -1;
			}
		}

		m_morphParameterModificationMap->clear ();
	}
	else
	{
		for (int i = 0; i < 2; ++i)
		{
			if (m_colorPicker [i]->isActive () && m_colorPicker [i]->checkAndResetUSerChanged ())
			{
				if (m_lastShouldUpdateYawFromColorIndex != i)
				{
					//-- don't stop here, we want to check all color pickers
					m_objectViewer->setCameraYawTarget        (m_defaultCameraYaw);
					m_lastShouldUpdateYawFromColorIndex = i;
					m_lastModifiedMorphParameter = 0;
				}
			}
		}
	}


	if (viewDirty && m_objectViewer->getCameraLookAtBone () != "root")
	{
		m_objectViewer->setViewDirty             (true);
		m_objectViewer->setCameraLookAtBoneDirty (true);
		m_objectViewer->setAutoZoomOutOnly       (false);
		m_objectViewer->setCameraForceTarget     (true);
		m_objectViewer->setCameraZoomInWhileTurn (true);
		m_objectViewer->recomputeZoom            ();
		m_objectViewer->setCameraForceTarget     (false);
	}

}

// ----------------------------------------------------------------------

void SwgCuiAvatarCustomizationBase::setObject (ClientObject * const obj, int const hairSkillModValue)
{
	if(obj)
	{
		setupCustomizationData (obj, hairSkillModValue);

		if(m_groupButtons && m_groupButtons->size() > 0)
		{
			OnButtonPressed((*m_groupButtons)[0]);
		}
	}
}

// ----------------------------------------------------------------------

CreatureObject * SwgCuiAvatarCustomizationBase::getObject ()
{
	return dynamic_cast<CreatureObject *>(m_objectViewer->getLastObject ());
}

//----------------------------------------------------------------------

void SwgCuiAvatarCustomizationBase::findHairObjects      (ClientObjectVector & cov) const
{
	const UIBaseObject::UIObjectList & olist = m_hairVolumePage->GetChildrenRef ();

	cov.clear ();
	cov.reserve (olist.size ());

	for (UIBaseObject::UIObjectList::const_iterator it = olist.begin (); it != olist.end (); ++it)
	{
		CuiWidget3dObjectListViewer * const viewer = dynamic_cast<CuiWidget3dObjectListViewer *>(*it);
		if (viewer && viewer->getLastObject ())
			cov.push_back (viewer->getLastObject ()->asClientObject ());
	}
}

//----------------------------------------------------------------------

void SwgCuiAvatarCustomizationBase::setupHairStyles (std::string const & templateName, int const hairSkillModValue)
{
	NOT_NULL (m_hairStyleVector);
	m_hairStyleVector->clear ();

	m_ignoreHairChange = true;
	m_hairVolumePage->Clear ();
	m_ignoreHairChange = false;

	std::string dummyDefaultHair;

	if (!CuiCharacterHairManager::loadHairStyles (templateName, *m_hairStyleVector, dummyDefaultHair, hairSkillModValue))
	{
		WARNING_STRICT_FATAL (!templateName.empty (), ("Error loading hairstyles for template %s", templateName.c_str ()));
		return;
	}

	if (!m_cdata)
		return;

	CuiWidget3dObjectListViewer * selectedViewer = 0;

	if (m_allowBald)
	{
		//-- add the bald hairstyle
		CuiWidget3dObjectListViewer * const defaultViewer = new CuiWidget3dObjectListViewer;
		defaultViewer->CopyPropertiesFrom (*m_sampleHairBox);
		defaultViewer->SetVisible (true);
		defaultViewer->SetTooltip (CuiStringIds::hairstyle_bald.localize ());
		defaultViewer->AddCallback (this);
		defaultViewer->SetSelectable (true);
		
		m_hairVolumePage->AddChild   (defaultViewer);
		defaultViewer->Link ();

		selectedViewer = defaultViewer;
	}

	TangibleObject * const currentHair = getCurrentHair ();

	for (HairStyleVector::const_iterator it = m_hairStyleVector->begin (); it != m_hairStyleVector->end (); ++it)
	{
		const std::string & hairTemplateName = *it;

		TangibleObject * hair = 0;

		if (currentHair && hairTemplateName == currentHair->getObjectTemplateName ())
		{
			hair = currentHair;
		}
		else
		{
			hair = safe_cast<TangibleObject *>(ObjectTemplate::createObject (hairTemplateName.c_str ()));
			
			if (!hair)
				WARNING (true, ("Invalid hair object template '%s' specified for player template '%s'.", hairTemplateName.c_str (), templateName.c_str ()));
			else
			{
				hair->setNetworkId(ClientImageDesignerManager::getNextClientNetworkId());
				hair->endBaselines ();
			}
		}
		
		if (hair)
		{
			createCustomizationLink (*hair);

			CuiWidget3dObjectListViewer * const viewer = CuiIconManager::createObjectIcon (*hair, 0);
			//the UI owns this, so it needs to alter it
			viewer->setAlterObjects(true);
			viewer->setCameraLodBias         (100.0f);
			viewer->setCameraLodBiasOverride (true);
			viewer->CopyPropertiesFrom       (*m_sampleHairBox);
			
			Unicode::String str = hair->getLocalizedName ();

			if (s_debugShowCustomizationPaths)
			{
				str.append (1, '\n');
				str += Unicode::narrowToWide (hair->getObjectTemplateName ());

				str.append (1, '\n');

				const Appearance * const app = hair->getAppearance ();
				if (app)
				{
					const char * const atn = app->getAppearanceTemplateName ();

					if (atn)
					{
						str += Unicode::narrowToWide (atn);
					}
					else
					{
						static const Unicode::String errstr = Unicode::narrowToWide ("No appearance template name");
						str += errstr;
					}
				}
				else
				{
					static const Unicode::String errstr = Unicode::narrowToWide ("No appearance");
					str += errstr;
				}
			}

			viewer->SetTooltip                            (str);
			viewer->setPaused                             (false);
			viewer->SetVisible                            (true);
			viewer->AddCallback                           (this);
			viewer->SetSelectable                         (true);

			m_hairVolumePage->AddChild                    (viewer);
			viewer->Link ();

			if (currentHair == hair)
				selectedViewer = viewer;
			else 
				m_ownedHairSet->insert (hair);
		}
	}

	if (selectedViewer)
	{
		m_ignoreHairChange = true;
		m_hairVolumePage->SetSelection (selectedViewer);
		m_hairVolumePage->Pack ();
		m_ignoreHairChange = false;
		m_hairVolumePage->CenterChild  (*selectedViewer);
	}

	updateSelectedHairColorState  ();
}

//----------------------------------------------------------------------

TangibleObject * SwgCuiAvatarCustomizationBase::getCurrentHair ()
{
	CreatureObject * const player     = dynamic_cast<CreatureObject*>(m_objectViewer->getLastObject ());

	if (!player)
		return 0;

	return safe_cast<TangibleObject *>(ContainerInterface::getObjectInSlot (*player, SlotNames::s_hair.c_str ()));
}

//----------------------------------------------------------------------

void SwgCuiAvatarCustomizationBase::populatePatterns ()
{
	m_patternVar = 0;

	if (!m_cdata)
		return;
}

//----------------------------------------------------------------------

void SwgCuiAvatarCustomizationBase::updatePlayerPattern ()
{
} //lint !e1762 //not now

//----------------------------------------------------------------------

void SwgCuiAvatarCustomizationBase::setupCustomizationGroups (ClientObject const & object)
{
	Unicode::String species (Unicode::narrowToWide (object.getObjectTemplateName ()));

	std::string species_gender;

	// @todo: replace species hack code below with real code
	Unicode::String::size_type sharedPos = species.find(Unicode::narrowToWide("shared_"), 0);
	const Unicode::String::size_type iffPos = species.find(Unicode::narrowToWide(".iff"), 0);
	if (sharedPos != Unicode::String::npos && iffPos != Unicode::String::npos)
	{
		sharedPos += strlen("shared_");
		species = species.substr(sharedPos, iffPos - sharedPos);
		species_gender = Unicode::wideToNarrow(species);
		species = Unicode::narrowToWide("creature.") + species;
	}

	std::string debugString;

	if (s_debugPrintCustomizationGroups)
		debugString += Unicode::wideToNarrow (species) + "\n";

	const Unicode::String dataPath    (m_dataFolder + species);

	m_allowBald = CustomizationManager::getAllowBald(species_gender);

	CustomizationManager::CustomizationSpeciesMap const & customizationMap = CustomizationManager::getCustomizationSpeciesMap(species_gender);


	for (CustomizationManager::CustomizationSpeciesMap::const_iterator i = customizationMap.begin(); i != customizationMap.end(); ++i)
	{
		std::string const & customizationGroupName = i->first;
		CustomizationManager::CustomizationVector const & customizationVector = i->second;
		CustomizationGroup const group(object, *m_cdata, species_gender, customizationGroupName, customizationVector);
		m_customizationGroups->push_back(group);
	}

	REPORT_LOG_PRINT (s_debugPrintCustomizationGroups, ("%s\n", debugString.c_str ()));

}

//----------------------------------------------------------------------

void SwgCuiAvatarCustomizationBase::setupCustomizationGroupButtons ()
{
	m_compositeGroups->Clear ();
	m_groupButtons->clear();

	NOT_NULL (m_sampleGroupButton);

	for (CustomizationGroupVector::const_iterator it = m_customizationGroups->begin (); it != m_customizationGroups->end (); ++it)
	{
		const CustomizationGroup & group = *it;

		UIButton * const dupe   = safe_cast<UIButton *>(m_sampleGroupButton->DuplicateObject ());

		Unicode::String buttonName = cms_textPre + group.name;
		dupe->SetText     (buttonName);
		dupe->AddCallback (this);

		IGNORE_RETURN (dupe->SetProperty   (UIWidget::PropertyName::PackSize, Unicode::narrowToWide ("1,f")));

		dupe->SetName       (Unicode::wideToNarrow (group.name));
		dupe->SetVisible    (true);
		IGNORE_RETURN (m_compositeGroups->AddChild (dupe));
		dupe->Link          ();

		m_groupButtons->push_back(dupe);
	}
}

//----------------------------------------------------------------------

SwgCuiAvatarCustomizationBase::CustomizationGroup * SwgCuiAvatarCustomizationBase::findCustomizationGroup (std::string const & groupName)
{
	for (CustomizationGroupVector::iterator it = m_customizationGroups->begin (); it != m_customizationGroups->end (); ++it)
	{
		CustomizationGroup & group = *it;
		if (Unicode::caseInsensitiveCompare (group.name, groupName))
			return &group;
	}
	return 0;
}

//----------------------------------------------------------------------

const SwgCuiAvatarCustomizationBase::CustomizationGroup * SwgCuiAvatarCustomizationBase::findCustomizationGroup (std::string const & groupName) const
{
	return const_cast<SwgCuiAvatarCustomizationBase *>(this)->findCustomizationGroup (groupName);
}

//----------------------------------------------------------------------

SwgCuiAvatarCustomizationBase::CustomizationGroup * SwgCuiAvatarCustomizationBase::findHairCustomizationGroup ()
{
	for (CustomizationGroupVector::iterator it = m_customizationGroups->begin (); it != m_customizationGroups->end (); ++it)
	{
		CustomizationGroup & group = *it;
		if(group.isHair)
			return &group;
	}
	return NULL;
}

//----------------------------------------------------------------------

const SwgCuiAvatarCustomizationBase::CustomizationGroup * SwgCuiAvatarCustomizationBase::findHairCustomizationGroup () const
{
	return const_cast<SwgCuiAvatarCustomizationBase *>(this)->findHairCustomizationGroup ();
}

//----------------------------------------------------------------------

void SwgCuiAvatarCustomizationBase::setupPageMorph ()
{
	if (!m_pageMorph->IsVisible ())
		return;

	const CustomizationGroup * const group = findCustomizationGroup (m_currentGroup);

	if (group)
	{
		setupPageMorphWidgets (*m_morphSliderPagesH, group->horizontal);
		setupPageMorphWidgets (*m_morphSliderPagesV, group->vertical);
	}

	updateMorphSliders  (false);
}

//----------------------------------------------------------------------

void SwgCuiAvatarCustomizationBase::updateMorphSliders (bool const notify)
{
	if (!m_pageMorph->IsVisible ())
		return;

	const CustomizationGroup * const group = findCustomizationGroup (m_currentGroup);

	if (group)
	{
		updateSliderVector (*m_morphSlidersH, group->horizontal, notify);
		updateSliderVector (*m_morphSlidersV, group->vertical, notify);
	}
}

//----------------------------------------------------------------------

std::string const & SwgCuiAvatarCustomizationBase::getGroup()
{
	return m_currentGroup;
}

//----------------------------------------------------------------------

void SwgCuiAvatarCustomizationBase::setGroup (std::string const & groupName)
{
	const CustomizationGroup * const group = findCustomizationGroup (groupName);

	const bool groupChanged = m_currentGroup != groupName;

	if (group)
	{
		if(m_customizationGroupNameVisible)
		{
			m_textCurrentGroup->SetText    (group->name);
			m_textCurrentGroup->SetVisible (true);
		}

		m_currentGroup = Unicode::wideToNarrow (group->name);
		m_curMorphParameter = 0;

		if (group->isHair)
		{
			m_pageHair->SetVisible (true);
			m_pageHair->SetEnabled (true);
		}
		else
		{
			if(m_disableSubPagesWhenNotSelected)
				m_pageHair->SetEnabled (false);
			else
				m_pageHair->SetVisible (false);
		}

		if (!group->horizontal.empty () || !group->vertical.empty ())
		{
			m_pageMorph->SetEnabled (true);
			m_pageMorph->SetVisible (true);
			setupPageMorph ();
		}
		else
		{
			setupPageMorph ();
			if(m_disableSubPagesWhenNotSelected)
				m_pageMorph->SetEnabled (false);
			else
				m_pageMorph->SetVisible (false);
		}

		if (!group->varColors [0].empty ())
		{
			//-- color picker for hair is activated later
			m_compositeColorPicker->SetVisible       (true);
			m_colorPicker [0]->getPage ().SetVisible (false);
			m_colorPicker [1]->getPage ().SetVisible (false);
			m_colorPicker [0]->getPage ().SetPropertyBoolean(Properties::IsOnHair, false);
			m_colorPicker [1]->getPage ().SetPropertyBoolean(Properties::IsOnHair, false);

			ClientObject * target     = 0;
			ClientObject * hairTarget = 0;

			CuiColorPicker::ObjectWatcherVector idv;

			ClientObjectVector cov;
			findHairObjects      (cov);

			if (group->isHair)
			{
				ClientObject * const currentHair = getCurrentHair ();

				if (currentHair)
					hairTarget = currentHair;
				else if (!cov.empty ())
					hairTarget = cov.front ();
			}

			target = getObject ();

			if (group->isHair)
			{
				idv.reserve (cov.size ());
				for (ClientObjectVector::const_iterator it = cov.begin (); it != cov.end (); ++it)
				{
					idv.push_back (Watcher<TangibleObject>(safe_cast<TangibleObject *>(*it)));
				}
			}

			ClientObject * const targets [2] =
			{
				group->isVarColorHair [0] ? hairTarget : target,
				group->isVarColorHair [1] ? hairTarget : target
			};

			if (targets [0])
			{
				SharedImageDesignerManager::SkillMods const & designerSkills = ClientImageDesignerManager::getSkillModsForDesigner(Game::getPlayerNetworkId());

				if (!group->isVarColorHair [0])
				{
					m_colorPicker [0]->getPage().SetVisible (true);

					PaletteArgb const * const palette = m_colorPicker[0]->getPalette();
					if(palette)
					{
						SharedImageDesignerManager::PaletteValues const & paletteValues = SharedImageDesignerManager::getPaletteValuesForPaletteAndSkillMods(palette, designerSkills);
						m_colorPicker[0]->setForceColumns(paletteValues.columns);
						m_colorPicker[0]->setTarget(targets[0]->getNetworkId(), group->varColors[0], 0, paletteValues.maxIndex);
					}
					else
					{
						m_colorPicker[0]->setTarget(targets[0]->getNetworkId(), group->varColors[0], 0, 256);
					}

					Unicode::String const colorPickerName = cms_textPre + group->varColorNames [0];
					m_colorPicker [0]->setText(colorPickerName);
				}
				else
					m_colorPicker [0]->setLinkedObjects (idv, false);

				if (!group->varColors [1].empty () && targets [1])
				{
					if (!group->isVarColorHair [1])
					{
						m_colorPicker [1]->getPage().SetVisible (true);

						PaletteArgb const * const palette = m_colorPicker[1]->getPalette();
						if(palette)
						{
							SharedImageDesignerManager::PaletteValues const & paletteValues = SharedImageDesignerManager::getPaletteValuesForPaletteAndSkillMods(palette, designerSkills);
							m_colorPicker[1]->setForceColumns(paletteValues.columns);
							m_colorPicker[1]->setTarget(targets[1]->getNetworkId(), group->varColors[1], 0, paletteValues.maxIndex);
						}
						else
						{
							m_colorPicker[1]->setTarget(targets[1]->getNetworkId(), group->varColors[1], 0, 256);
						}

						Unicode::String const colorPickerName = cms_textPre + group->varColorNames [1];
						m_colorPicker [1]->setText(colorPickerName);
					}
					else
						m_colorPicker [1]->setLinkedObjects (idv, false);
				}

				m_compositeColorPicker->ForcePackChildren ();

				if (m_colorPicker [0])
					m_colorPicker [0]->updateCellSizes ();

				if (m_colorPicker [1] && m_colorPicker [1]->getPage ().IsVisible ())
					m_colorPicker [1]->updateCellSizes ();
			}

			if (group->isHair)
			{
				updateSelectedHairColorState ();
			}
		}
		else
		{
			m_compositeColorPicker->SetVisible (false);
		}

		if (groupChanged)
		{
			m_objectViewer->setCameraLookAtBone  (group->cameraLookAtBone);
			m_objectViewer->setFitDistanceFactor (group->cameraZoomFactor);

			const bool isRoot = group->cameraLookAtBone.empty () || !_stricmp (group->cameraLookAtBone.c_str (), "root");

			if (isRoot)
				m_objectViewer->setCameraCompensateScale (true);
			else
				m_objectViewer->setCameraCompensateScale (false);

			if (group->cameraYawValid && m_yawOnCustomizationSelection)
				m_objectViewer->setCameraYawTarget        (group->cameraYaw);
			else
				m_objectViewer->setCameraYawTarget        (m_defaultCameraYaw);

			m_objectViewer->setAutoZoomOutOnly (false);
			m_objectViewer->recomputeZoom ();

		}
	}
	else
	{
		m_textCurrentGroup->Clear ();
		m_textCurrentGroup->SetVisible (false);

		m_currentGroup = groupName;
		m_curMorphParameter = 0;

		WARNING (!m_currentGroup.empty (), ("No group: %s", groupName.c_str ()));

		if(!m_disableSubPagesWhenNotSelected)
		{
			m_pageHair->SetVisible (false);
			m_pageMorph->SetVisible (false);
			m_compositeColorPicker->SetVisible (false);
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiAvatarCustomizationBase::attachToMorphWidgets (PageVector & pages, SliderVector & sliders, UIPage const & pageMorph) const
{
	const UIBaseObject::UIObjectList & olist = pageMorph.GetChildrenRef ();

	sliders.reserve (olist.size ());
	pages.reserve (olist.size ());

	for (UIBaseObject::UIObjectList::const_iterator it = olist.begin (); it != olist.end (); ++it)
	{
		UIPage * const page = dynamic_cast<UIPage *>(*it);
		
		if (page)
		{
			page->Attach (0);
			pages.push_back (page);
			
			UISliderbar * const bar = NON_NULL (dynamic_cast<UISliderbar *>(page->GetChild ("slider")));
			
			if (bar)
			{
				bar->Attach (0);
				sliders.push_back (bar);
			}
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiAvatarCustomizationBase::manageMorphSliderCallbacks (bool const onOff)
{
	manageMorphSliderCallbacks (*m_morphSlidersH, *m_morphSliderPagesH, onOff);
	manageMorphSliderCallbacks (*m_morphSlidersV, *m_morphSliderPagesV, onOff);
}

//----------------------------------------------------------------------

void SwgCuiAvatarCustomizationBase::manageMorphSliderCallbacks (SliderVector & sliders, PageVector & pages, bool const onOff)
{
	{
		for (SliderVector::iterator it = sliders.begin (); it != sliders.end (); ++it)
		{
			UISliderbar * const slider = *it;
			if (onOff)
				slider->AddCallback (this);
			else
				slider->RemoveCallback (this);
		}
	}
	
	for (PageVector::iterator it = pages.begin (); it != pages.end (); ++it)
	{
		UIPage * const page = *it;
		if (onOff)
			page->AddCallback (this);
		else
			page->RemoveCallback (this);
	}
}

//----------------------------------------------------------------------

namespace
{
	template <typename T> void detachFromMorphWidgetVector (T & v)
	{
		for (T::iterator it = v.begin (); it != v.end (); ++it)
		{
			UIWidget * const widget = *it;
			widget->Detach (0);
		}
		v.clear ();
	}
}

//----------------------------------------------------------------------

void SwgCuiAvatarCustomizationBase::detachFromMorphWidgets ()
{
	detachFromMorphWidgetVector (*m_morphSliderPagesH);
	detachFromMorphWidgetVector (*m_morphSliderPagesV);
	detachFromMorphWidgetVector (*m_morphSlidersH);
	detachFromMorphWidgetVector (*m_morphSlidersV);
}

//----------------------------------------------------------------------

UISliderbar * SwgCuiAvatarCustomizationBase::findMorphSlider (UIWidget const & widget, bool & isMorphH, bool & isMorphV, CustomizationManager::MorphParameter *& morphParam, bool outputWarning)
{
	UISliderbar * slider = 0;
	
	if (widget.IsA (TUISliderbar))
	{
		const SliderVector::iterator slider_h_it = std::find (m_morphSlidersH->begin (), m_morphSlidersH->end (), &widget);
		const SliderVector::iterator slider_v_it = std::find (m_morphSlidersV->begin (), m_morphSlidersV->end (), &widget);
			
		isMorphH = slider_h_it != m_morphSlidersH->end ();
		isMorphV = slider_v_it != m_morphSlidersV->end ();
		
		if (isMorphH || isMorphV)
		{	
			//-- it is a morph slider
			if (widget.GetParent ())
			{		
				CustomizationGroup * const group = findCustomizationGroup (m_currentGroup);
				
				if (group)
				{				
					const std::string & paramName = widget.GetParent ()->GetName ();
					
					if (isMorphH)
					{
						morphParam = group->findMorphParam (paramName, group->horizontal);
						slider = *slider_h_it;
					}
					else
					{
						morphParam = group->findMorphParam (paramName, group->vertical);
						slider = *slider_v_it;
					}
					
					WARNING (!morphParam && outputWarning, ("Unable to find morph param '%s' for '%s'", paramName.c_str (), m_currentGroup.c_str ()));
					return slider;
				}
			}
		}
	}
	
	else if (widget.IsA (TUIWidget))
	{
		const PageVector::iterator page_h_it = std::find (m_morphSliderPagesH->begin (), m_morphSliderPagesH->end (), &widget);
		const PageVector::iterator page_v_it = std::find (m_morphSliderPagesV->begin (), m_morphSliderPagesV->end (), &widget);
			
		isMorphH = page_h_it != m_morphSliderPagesH->end ();
		isMorphV = page_v_it != m_morphSliderPagesV->end ();
		
		if (isMorphH)
			slider = (*m_morphSlidersH) [std::distance (m_morphSliderPagesH->begin (), page_h_it)];
		else if (isMorphV)
			slider = (*m_morphSlidersV) [std::distance (m_morphSliderPagesV->begin (), page_v_it)];

		if (slider)
		{
			return findMorphSlider (*slider, isMorphH, isMorphV, morphParam);
		}
	}

	morphParam = 0;
	return 0;
}

//----------------------------------------------------------------------

void SwgCuiAvatarCustomizationBase::randomizeHairColors (std::string const & varName, int const value)
{
	if (varName.empty ())
		return;

	ClientObjectVector cov;
	findHairObjects      (cov);

	bool done = false;
	int v = value;

	for (ClientObjectVector::iterator it = cov.begin (); it != cov.end () && !done; ++it)
	{
		TangibleObject * const hair     = safe_cast<TangibleObject *>(*it);
		CustomizationData * const cdata = hair->fetchCustomizationData ();
		if (cdata)
		{
			CustomizationManager::PathType type = CustomizationManager::PT_none;
			PaletteColorCustomizationVariable * const var = dynamic_cast<PaletteColorCustomizationVariable *>(CustomizationManager::findVariable (*cdata, varName, type));

			if (var)
			{
				PaletteArgb const * const palette = var->fetchPalette();
				if(palette)
				{
					SharedImageDesignerManager::PaletteValues const paletteValues = SharedImageDesignerManager::getPaletteValuesForPaletteAndLevel(palette, SharedImageDesignerManager::IDPL_NOT_A_DESIGNER);
					int range_begin = 0, range_end = 0;
					var->getRange (range_begin, range_end);
					if(paletteValues.maxIndex < range_end)
						range_end = paletteValues.maxIndex;

					if (v < 0)
						v = Random::random (range_begin, range_end - 1);
					var->setValue (v);
					palette->release();
				}

				//-- shared variables are handled by the linking of customization datas
				if (type == CustomizationManager::PT_shared)
					done = true;
			}
			//-- Release local reference.
			cdata->release();
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiAvatarCustomizationBase::createCustomizationLink (TangibleObject & wearable)
{
	if (m_cdata)
	{
		CustomizationData * const wearableCd = wearable.fetchCustomizationData ();
		if (wearableCd)
		{
			wearableCd->mountRemoteCustomizationData (*m_cdata, CustomizationManager::cms_shared_owner, CustomizationManager::cms_shared_owner_no_slash);
			wearableCd->release ();
		}
	}
}

//-----------------------------------------------------------------------

void SwgCuiAvatarCustomizationBase::randomize (bool const all)
{
	CuiSoundManager::play (CuiSounds::button_random);

	const Unicode::String & wide_currentGroup = Unicode::narrowToWide (m_currentGroup);

	typedef std::map<std::string, int>   StringIntMap;
	StringIntMap                         colorValues;
	StringIntMap                         colorValuesLinked;
	StringIntMap                         hairColorValues;
	std::string                          hairColorVarNames [2];
	
	bool currentGroupChanged = false;

	NOT_NULL (m_cdata);
	{
		for (CustomizationGroupVector::iterator it = m_customizationGroups->begin (); it != m_customizationGroups->end (); ++it)
		{
			CustomizationGroup & group = *it;
			
			if (!all && group.name != wide_currentGroup)
				continue;

			group.randomize (!all);
			
			//-- randomize the hairstyle

			if (group.isHair)
			{
				if (m_hairVolumePage)
				{
					const int index = Random::random (0, m_hairVolumePage->GetChildrenRef ().size ());
					m_hairVolumePage->SetSelectionIndex (index);
				}
			}
			
			//-- store the hair color variables names

			if (group.isHair || group.isColorLinkedToHair)
			{
				hairColorVarNames [0] = group.varColorLinkedToHair [0];
				hairColorVarNames [1] = group.varColorLinkedToHair [1];
			}

			// && !group.isColorLinkedToBody

			//-- randomize any non-hair colors
//			if (!group.isHair)
			{
				bool found = false;

				for (int i = 0; i < 2; ++i)
				{
					if (group.varColors [i].empty ())
						continue;

					if (group.isVarColorHair [i])
						continue;

					found = true;

					if (!group.isColorRandomizable [i] && (all || !group.isColorRandomizableGroup [i]))
						continue;

					//-- don't randomize the color multiple times
					const StringIntMap::const_iterator it = colorValues.find (group.varColors [i]);

					int value = 0;

					if (it == colorValues.end ())
					{
						PaletteColorCustomizationVariable * const var = dynamic_cast<PaletteColorCustomizationVariable *>(CustomizationManager::findVariable (*m_cdata, group.varColors [i]));

						if (var)
						{
							PaletteArgb const * const palette = var->fetchPalette();
							if(palette)
							{
								SharedImageDesignerManager::PaletteValues const paletteValues = SharedImageDesignerManager::getPaletteValuesForPaletteAndLevel(palette, SharedImageDesignerManager::IDPL_NOT_A_DESIGNER);
								int range_begin = 0;
								int range_end = 0;
								var->getRange (range_begin, range_end);
								if(paletteValues.maxIndex < range_end)
									range_end = paletteValues.maxIndex;
								value = Random::random (range_begin, range_end - 1);
								var->setValue (value);
								colorValues.insert (std::make_pair (group.varColors [i], value));
								if (!group.varColorLinkedToSelf [i][0].empty ())
									colorValuesLinked.insert (std::make_pair (group.varColorLinkedToSelf [i][0], value));
								if (!group.varColorLinkedToSelf [i][1].empty ())
									colorValuesLinked.insert (std::make_pair (group.varColorLinkedToSelf [i][1], value));
								palette->release();
							}
						}
					}
					else
						value = (*it).second;

					if (group.isColorLinkedToHair)
						hairColorValues [group.varColorLinkedToHair [i]] = value;
				}

				if (found && group.name == wide_currentGroup)
				{
					currentGroupChanged = true;
				}
			}
		}
	}

	//-- bring the linked variables up to spec
	{
		for (StringIntMap::const_iterator it = colorValuesLinked.begin (); it != colorValuesLinked.end (); ++it)
		{
			const std::string & varName = (*it).first;
			const int value            = (*it).second;
			PaletteColorCustomizationVariable * const var = dynamic_cast<PaletteColorCustomizationVariable *>(CustomizationManager::findVariable (*m_cdata, varName));

			if (var)
				var->setValue (value);
		}
	}

	//-- don't randomize the hair color twice
	if (hairColorVarNames [1] == hairColorVarNames [0])
		hairColorVarNames [1].clear ();

	//-- update or randomize hair colors as needed
	for (int i = 0; i < 2; ++i)
	{
		if (!hairColorVarNames [i].empty ())
		{
			const StringIntMap::const_iterator it = hairColorValues.find (hairColorVarNames [i]);

			if (it == hairColorValues.end ())
				randomizeHairColors (hairColorVarNames [i]);
			else
				randomizeHairColors (hairColorVarNames [i], (*it).second);
		}
	}

	if (currentGroupChanged)
		setGroup (m_currentGroup);

	updateSelectedHairColorState ();

	m_morphInProgress = true;
}

//-----------------------------------------------------------------------

void SwgCuiAvatarCustomizationBase::setCustomizationGroupNameVisible(bool const visible)
{
	m_customizationGroupNameVisible = visible;
}

//-----------------------------------------------------------------------

void SwgCuiAvatarCustomizationBase::SetDisableSubPagesWhenNotSelected(bool const disableSubPagesWhenNotSelected)
{
	m_disableSubPagesWhenNotSelected = disableSubPagesWhenNotSelected;
}

//-----------------------------------------------------------------------

float SwgCuiAvatarCustomizationBase::dataValueToFloat (long const l, float const range)
{
	return static_cast<float>(l) / range;
}

//-----------------------------------------------------------------------

void SwgCuiAvatarCustomizationBase::setIsCreation(bool const isCreation)
{
	m_creation = isCreation;
}

//-----------------------------------------------------------------------

void SwgCuiAvatarCustomizationBase::setYawOnCustomizationSelection (bool const yawOnCustomizationSelection)
{
	m_yawOnCustomizationSelection = yawOnCustomizationSelection;
}

//-----------------------------------------------------------------------

void SwgCuiAvatarCustomizationBase::resetDefaults()
{
	for (CustomizationGroupVector::iterator it = m_customizationGroups->begin (); it != m_customizationGroups->end (); ++it)
	{
		CustomizationGroup & group = *it;

		for(MorphParameterVector::iterator it2 = group.horizontal.begin(); it2 != group.horizontal.end(); ++it2)
		{
			CustomizationManager::MorphParameter & param = *it2;
			param.current = param.theDefault;
		}
		for(MorphParameterVector::iterator it3 = group.vertical.begin(); it3 != group.vertical.end(); ++it3)
		{
			CustomizationManager::MorphParameter & param = *it3;
			param.current = param.theDefault;
		}
	}

	std::string const currentGroup = getGroup();
	setGroup(std::string());
	setGroup(currentGroup);
}

// ======================================================================

