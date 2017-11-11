// ============================================================================
//
// NpcCuiViewer.cpp
// copyright (c) 2004 Sony Online Entertainment
//
// ============================================================================

#include "FirstNpcEditor.h"
#include "NpcCuiViewer.h"

#include "UIMessage.h"
#include "UIPage.h"
#include "UIVolumePage.h"

#include "sharedFile/TreeFile.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/Fatal.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedGame/AssetCustomizationManager.h"
#include "sharedGame/CustomizationManager.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/CustomizationData.h"
#include "sharedObject/CustomizationDataProperty.h"
#include "sharedObject/MemoryBlockManagedObject.h"
#include "sharedObject/CustomizationData.h"
#include "sharedObject/CustomizationDataProperty.h"

#include "clientSkeletalAnimation/SkeletalAppearanceTemplate.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientSkeletalAnimation/SkeletalAppearanceTemplate.h"
#include "clientSkeletalAnimation/MeshGeneratorTemplate.h"
#include "clientSkeletalAnimation/MeshGeneratorTemplateList.h"
#include "clientSkeletalAnimation/BasicMeshGeneratorTemplate.h"
#include "clientSkeletalAnimation/BasicSkeletonTemplate.h"
#include "clientSkeletalAnimation/SkeletalMeshGenerator.h"
#include "clientGame/CreatureController.h"
#include "clientGame/CreatureObject.h"
#include "clientGraphics/Texture.h"
#include "clientGraphics/TextureList.h"
#include "clientUserInterface/CuiWidget3dObjectListViewer.h"
#include "clientUserInterface/CuiIconManager.h"

#include <algorithm>
#include <list>

// ----------------------------------------------------------------------------

namespace NpcCuiViewerNamespace
{
	const UILowerString s_wearable    = UILowerString ("wearable");
	const UILowerString s_wearableNum = UILowerString ("wearableNum");

	namespace Properties
	{
		const UILowerString DefaultViewerPitch = UILowerString ("DefaultViewerPitch");
		const UILowerString DefaultViewerYaw   = UILowerString ("DefaultViewerYaw");
	}

	Object * createWearableFromMeshGenerator(const char *filename, bool noSkt = false);
}

using namespace NpcCuiViewerNamespace;

// ----------------------------------------------------------------------------

NpcCuiViewer::NpcCuiViewer (UIPage & newPage)
 : CuiMediator("NpcCuiViewer", newPage)
 , UIEventCallback()
 , m_avatar(0)
 , m_selectedWearable(0)
 , m_objectViewer(0)
 , m_skeletalAppearanceTemplate(0)
 , m_skeletalAppearanceTemplateFilename()
 , m_pageWearables(0)
 , m_currentVolumePage(0)
 , m_wearableVolumePage(0)
 , m_wearables()
 , m_currentWearables()
 , m_sampleWearableBox(0)
 , m_dirty(false)
{
	UIWidget *widget = 0;
	getCodeDataObject (TUIWidget, widget, "ViewerWidget");
	m_objectViewer = NON_NULL (dynamic_cast<CuiWidget3dObjectListViewer *>(widget));

	IGNORE_RETURN(m_objectViewer->SetPropertyFloat (Properties::DefaultViewerPitch, m_objectViewer->getCameraPitch ()));
	IGNORE_RETURN(m_objectViewer->SetPropertyFloat (Properties::DefaultViewerYaw,   m_objectViewer->getCameraYaw   ()));

	//-- wearable UI
	getCodeDataObject (TUIPage, m_pageWearables, "pageWearables");
	getCodeDataObject (TUIVolumePage, m_currentVolumePage, "currentVolumePage");
	getCodeDataObject (TUIVolumePage, m_wearableVolumePage, "wearableVolumePage");
	getCodeDataObject (TUIWidget, m_sampleWearableBox, "sampleWearableBox");

	m_sampleWearableBox->SetVisible   (false);

	//-- register so messages can be received
	registerMediatorObject (*m_wearableVolumePage, true);
	registerMediatorObject (*m_currentVolumePage,  true);

	//-- configure the wearable control
	setupWearableControl();
}

// ----------------------------------------------------------------------------

void NpcCuiViewer::setupWearableControl ()
{
	m_wearableVolumePage->SetSelectionIndex(-1);
	m_wearableVolumePage->Clear();
	m_currentVolumePage->SetSelectionIndex(-1);
	m_currentVolumePage->Clear();
}

// ----------------------------------------------------------------------------

void NpcCuiViewer::addWearable (const char * wearableName)
{
	Object * wearable = createWearableFromMeshGenerator(wearableName);
	if(!wearable)
		return;

	m_wearables.push_back(wearable);

	CuiWidget3dObjectListViewer * const viewer = CuiIconManager::createObjectIcon (*wearable, 0);
	
	//-- the UI owns this, so it needs to alter it
	viewer->setAlterObjects(true);
	viewer->setCameraLodBias(3.0f);
	viewer->setCameraLodBiasOverride(true);
	viewer->setPaused(false);
	viewer->setForceDefaultClippingPlanes(true);

	viewer->CopyPropertiesFrom(*m_sampleWearableBox);
	viewer->SetTooltip(Unicode::narrowToWide(wearableName));
	viewer->SetVisible(true);
	viewer->AddCallback(this);
	viewer->SetSelectable(true);

	IGNORE_RETURN(viewer->SetProperty(s_wearable, Unicode::narrowToWide(wearableName)));

	IGNORE_RETURN(m_wearableVolumePage->AddChild(viewer));
	viewer->Link ();
}

// ----------------------------------------------------------------------------

static bool ignore = false;

// ----------------------------------------------------------------------------

void NpcCuiViewer::OnVolumePageSelectionChanged (UIWidget * const context)
{
	if (!ignore && context == m_wearableVolumePage)
	{
		CuiWidget3dObjectListViewer * const viewer = dynamic_cast<CuiWidget3dObjectListViewer *>(m_wearableVolumePage->GetLastSelectedChild ());

		UIString s;
		UIString t(Unicode::narrowToWide(""));

		if(viewer && viewer->GetProperty(s_wearable, s))
		{
			if(s != t)
			{
				std::string wearableName = Unicode::wideToNarrow(s).c_str();

				if(isWearing(wearableName.c_str()))
					unwear(wearableName.c_str());
				else
					wear(wearableName.c_str());

				ignore = true;
				//TTODO: m_wearableVolumePage->SetSelection (0);
				m_wearableVolumePage->SetSelectionIndex(-1);

				updateCurrentWearables();

				ignore = false;
			}
		}
	}
	else if (!ignore && context == m_currentVolumePage)
	{
		CuiWidget3dObjectListViewer * const viewer = dynamic_cast<CuiWidget3dObjectListViewer *>(m_currentVolumePage->GetLastSelectedChild ());

		if(viewer)
			setSelectedWearable(viewer->getLastObject());
	}

	return;
}

// ----------------------------------------------------------------------------

Object *  NpcCuiViewerNamespace::createWearableFromMeshGenerator(const char *filename, bool noSkt)
{
	Appearance *appearance = 0;

	if (!TreeFile::exists(filename))
	{
		WARNING(true, ("Mesh [%s] does not exist.", filename));
		return 0;
	}

	//-- Load the mesh generator.
	const MeshGeneratorTemplate *const mgTemplate = MeshGeneratorTemplateList::fetch(CrcLowerString(filename));
	if (!mgTemplate)
		return 0;

	const BasicMeshGeneratorTemplate *const basicMgTemplate = mgTemplate->fetchBasicMeshGeneratorTemplate(0);
	if (!basicMgTemplate)
	{
		mgTemplate->release();
		return 0;
	}

	const MeshGenerator *const meshGenerator = basicMgTemplate->createMeshGenerator();
	basicMgTemplate->release();
	mgTemplate->release();

	if (!meshGenerator)
		return 0;

	//-- get referenced skeleton template name from mesh generator
	const SkeletalMeshGenerator *const skeletalMeshGenerator = dynamic_cast<const SkeletalMeshGenerator*>(meshGenerator);
	if (!skeletalMeshGenerator || !skeletalMeshGenerator->getReferencedSkeletonTemplateCount())
	{
		meshGenerator->release();
		return 0;
	}

	//-- get name for skeleton template
	const CrcLowerString &crcSkeletonTemplateName = skeletalMeshGenerator->getReferencedSkeletonTemplateName(0);

	//-- create a suitable SkeletalAppearanceTemplate
	SkeletalAppearanceTemplate *const sat = new SkeletalAppearanceTemplate();
	IGNORE_RETURN(AppearanceTemplateList::fetchNew(sat));

	char const * const pathName = crcSkeletonTemplateName.getString();

	if(TreeFile::exists(pathName))
	{
		//-- create a SkeletalAppearance2 with the skeleton
		FatalSetThrowExceptions(true);
		try
		{
			//-- populate it
			if(!noSkt)
				IGNORE_RETURN(sat->addSkeletonTemplate(pathName, ""));
			IGNORE_RETURN(sat->addMeshGenerator(filename));

			//-- use createAppearance so that the default assets will get returning if anything is missing
			appearance = sat->createAppearance();
		}
		catch(...)
		{
			delete appearance;
			appearance = 0;

			DEBUG_REPORT_LOG(true, ("NpcCuiViewerNamespace::createAppearanceFromMeshGenerator() - %s\n",
				filename));
		}
		FatalSetThrowExceptions(false);
	}
	else
	{
		WARNING(true, ("Mesh [%s] has non-existing skeleton [%s], will generate default skeletal appearance.", filename, pathName));

		appearance = AppearanceTemplateList::createAppearance(AppearanceTemplateList::getDefaultAppearanceTemplateName());
		DEBUG_REPORT_LOG(false, ("NpcCuiViewerNamespace::createAppearanceFromMeshGenerator() - Missing Assets for [%s]\n",
			filename));
	}

	if (!appearance)
		return 0; //lint !e429
	
	Object *wearableObject = new MemoryBlockManagedObject;

	wearableObject->setAppearance(appearance);

	//-- Create the wearable customization data.
	CustomizationDataProperty *const wearableCdProperty = new CustomizationDataProperty(*wearableObject);
	wearableObject->addProperty(*wearableCdProperty);

	CustomizationData *wearableCustomizationData = wearableCdProperty->fetchCustomizationData();
	NOT_NULL(wearableCustomizationData);

	//-- Set wearable appearance's customization data.
	appearance->setCustomizationData(wearableCustomizationData);

	//-- Loop through all the mesh generators in the wearable template and add all the customization
	//   variables for them.
	NOT_NULL(sat);
	int const meshGeneratorCount = sat->getMeshGeneratorCount();
	for (int i = 0; i < meshGeneratorCount; ++i)
	{
		CrcString const &meshGeneratorName = sat->getMeshGeneratorName(i);

		IGNORE_RETURN(AssetCustomizationManager::addCustomizationVariablesForAsset(meshGeneratorName, *wearableCustomizationData, true));
	}

	wearableCustomizationData->release();
	skeletalMeshGenerator->release();
	AppearanceTemplateList::release(sat);

	return wearableObject; //lint !e429
} //lint !e429

// ----------------------------------------------------------------------------

NpcCuiViewer::~NpcCuiViewer ()
{
	delete m_avatar;
	m_avatar         = 0;
	m_objectViewer   = 0;
	m_pageWearables  = 0;

	clearWearables();
	m_wearableVolumePage = NULL;

	clearCurrentWearables();
	m_currentVolumePage = NULL;

	m_selectedWearable = 0;

	if (m_skeletalAppearanceTemplate)
	{
		AppearanceTemplateList::release(m_skeletalAppearanceTemplate);
		m_skeletalAppearanceTemplate = 0;
	}
}

// ----------------------------------------------------------------------------

void NpcCuiViewer::clearWearables ()
{
	ignore = true;
	m_wearableVolumePage->SetSelectionIndex(-1);
	m_wearableVolumePage->Clear ();
	ignore = false;

	std::vector<Object *>::iterator i = m_wearables.begin();

	for(; i != m_wearables.end(); ++i)
		delete *i;

	m_wearables.clear();
}

// ----------------------------------------------------------------------------

void NpcCuiViewer::clearCurrentWearables ()
{
	setSelectedWearable(0);

	m_currentVolumePage->SetSelectionIndex(-1);
	m_currentVolumePage->Clear ();

	std::vector<Object *>::iterator i = m_currentWearables.begin();

	for(; i != m_currentWearables.end(); ++i)
		delete *i;

	m_currentWearables.clear();
}

// ----------------------------------------------------------------------------

void NpcCuiViewer::performActivate ()
{
	setPointerInputActive  (true);
	setKeyboardInputActive (true);
	setInputToggleActive   (false);

	{
		float f = 0.0f;
		if (m_objectViewer->GetPropertyFloat (Properties::DefaultViewerPitch, f))
			m_objectViewer->setCameraPitch (f);
		if (m_objectViewer->GetPropertyFloat (Properties::DefaultViewerYaw, f))
			m_objectViewer->setCameraYaw (f, true);
	}

	m_objectViewer->setPaused (false);
}


// ----------------------------------------------------------------------------

void NpcCuiViewer::loadModel(const std::string & templateName)
{
	if(m_avatar)
	{
		m_objectViewer->clearObjects ();
		delete m_avatar;
	}

	m_avatar = createPlayerModel (templateName.c_str());

	setSelectedWearable(0);

	updateAvatarSelection ();
	setIsUpdating (true);
}

// ----------------------------------------------------------------------------

void NpcCuiViewer::loadSAT(const std::string & satFilename)
{
	//-- Attempt to just change the appear (don't re-create the avater)
	if(!m_avatar)
		return;

	//-- clear out old skeletal mesh appearance and any object appearances
	if (m_skeletalAppearanceTemplate)
	{
		AppearanceTemplateList::release(m_skeletalAppearanceTemplate);
		m_skeletalAppearanceTemplate = 0;
	}

	m_avatar->setAppearance(0);

	m_skeletalAppearanceTemplateFilename = "";

	const AppearanceTemplate *at = AppearanceTemplateList::fetch(satFilename.c_str());
	m_skeletalAppearanceTemplate = const_cast<SkeletalAppearanceTemplate*>(dynamic_cast<const SkeletalAppearanceTemplate*>(at));
	if (!m_skeletalAppearanceTemplate)
	{
		if (at)
		{
			AppearanceTemplateList::release(at);
			DEBUG_REPORT_LOG(true, ("[%s] was an appearance, but not a skeletal appearance template\n", satFilename.c_str()));
		}
		return;
	}

	//-- make an appearance from default instance if one is available
	//   gracefully handle a broken instance specification here.
	Appearance* appearance = m_skeletalAppearanceTemplate->createAppearance();

	if (!appearance)
		return;

	//-- Delete customization data if we already have it.
	{
		CustomizationDataProperty *const cdProperty = dynamic_cast<CustomizationDataProperty*>(m_avatar->getProperty(CustomizationDataProperty::getClassPropertyId()));
		if (cdProperty)
		{
			m_avatar->removeProperty(CustomizationDataProperty::getClassPropertyId());
			delete cdProperty;
		}
	}

	//-- create and fill the Object's CustomizationData property
	//-- get or create the CustomizationDataProperty
	CustomizationDataProperty *property = dynamic_cast<CustomizationDataProperty*>(m_avatar->getProperty( CustomizationDataProperty::getClassPropertyId()));
	if (!property)
	{
		// create the CustomizationDataProperty
		property = new CustomizationDataProperty(*m_avatar);
		m_avatar->addProperty(*property);
	}

	//-- get the CustomizationData for the object
	NOT_NULL(property);

	CustomizationData *const customizationData = property->fetchCustomizationData();
	NOT_NULL(customizationData);

	//-- add variables influencing the appearance to the CustomizationData
	bool const skipSharedOwnerVariables = false;
	IGNORE_RETURN(AssetCustomizationManager::addCustomizationVariablesForAsset(TemporaryCrcString(satFilename.c_str(), true), *customizationData, skipSharedOwnerVariables));

	//-- set the appearance's CustomizationData
	appearance->setCustomizationData(customizationData);

	//-- release local reference
	customizationData->release();

	//-- set the appearance
	m_avatar->setAppearance(appearance);

	// setup appearance template view name
	m_skeletalAppearanceTemplateFilename = satFilename;
}  //lint !e429 !e830

// ----------------------------------------------------------------------------

void NpcCuiViewer::performDeactivate ()
{
	setIsUpdating (false);

	m_objectViewer->setPaused (true);

	m_objectViewer->clearObjects ();
}

// ----------------------------------------------------------------------------

bool NpcCuiViewer::OnMessage( UIWidget *context, const UIMessage & msg )
{
	UNREF(context);
	UNREF(msg);

#if 0
	if(msg.IsMouseButtonMessage() && msg.Type == UIMessage::LeftMouseUp)
	{
		CuiWidget3dObjectListViewer * const viewer = dynamic_cast<CuiWidget3dObjectListViewer *>(context);

		UIString s;
		if(viewer && viewer->GetProperty(s_wearable, s))
		{
			if(s != Unicode::narrowToWide(""))
			{
				std::string wearableName = Unicode::wideToNarrow(s).c_str();

				if(isWearing(wearableName.c_str()))
					unwear(wearableName.c_str());
				else
					wear(wearableName.c_str());
			}
		}
		REPORT_LOG_PRINT(true, ("OnMessage()::IsMouseButtonMessage\n"));
	}
#endif

	return true;
}

// ----------------------------------------------------------------------------

void NpcCuiViewer::OnButtonPressed( UIWidget *Context )
{
	UNREF(Context);

	REPORT_LOG_PRINT(true, ("OnButtonPressed\n"));
}

// ---------------------------------------------------------------------------------

CreatureObject * NpcCuiViewer::createPlayerModel (const std::string & templateName) const
{
	CreatureObject * const creature = dynamic_cast<CreatureObject*>(ObjectTemplate::createObject (templateName.c_str()));
	if (!creature)
	{
		WARNING (true, ("NpcCuiViewer failed [%s]", templateName.c_str ()));
		return 0;
	}
	
	const SharedObjectTemplate * const tmpl = dynamic_cast<const SharedObjectTemplate *>(creature->getObjectTemplate ());
	
	if (tmpl)
	{
		//-- set avatar scale to the midpoint
		creature->setScaleFactor ((tmpl->getScaleMax () + tmpl->getScaleMin ()) * 0.5f);
	}
	
	CreatureController* const controller = new CreatureController (creature);
	creature->setController (controller);

	creature->endBaselines ();

	Appearance * const app = creature->getAppearance ();
	if (app)
	{
		SkeletalAppearance2 * const skelApp = app->asSkeletalAppearance2 ();
		if (skelApp)
		{
			//-- preload all lod assets
			const int count = skelApp->getDetailLevelCount();
			for (int i = 0; i < count; ++i)
				IGNORE_RETURN(skelApp->isDetailLevelAvailable(i));
		}
	}
	
	return creature; //lint !e429
} //lint !e429

// ----------------------------------------------------------------------------

void NpcCuiViewer::updateAvatarSelection ()
{
	if(!m_avatar)
		return;

	m_objectViewer->addObject (*m_avatar);

	m_objectViewer->setViewDirty             (true);
	m_objectViewer->setCameraForceTarget     (true);
	m_objectViewer->recomputeZoom            ();
	m_objectViewer->setCameraForceTarget     (false);
	m_objectViewer->setViewDirty             (true);
	m_objectViewer->setIgnoreMouseWheel      (true);
}

// ----------------------------------------------------------------------------

void NpcCuiViewer::update (float deltaTimeSecs)
{
	CuiMediator::update (deltaTimeSecs);
}

// ----------------------------------------------------------------------------

void NpcCuiViewer::wear(const char * mgName)
{
	if(!m_avatar)
		return;

	const char * relativeName = strstr(mgName, "appearance");
	NOT_NULL(relativeName);

	//-- Create the wearable object.
	Object *const wearableObject = createWearableFromMeshGenerator(relativeName, true);

	//-- Get wearer's appearance and customization data.
	Appearance *const baseAppearance = m_avatar->getAppearance();
	SkeletalAppearance2 *const wearerAppearance = (baseAppearance ? baseAppearance->asSkeletalAppearance2() : 0);
	NOT_NULL(wearerAppearance);

	//-- Add wearable object as child of wearer.  This child object will never have its appearance rendered.
	//   We add it as a child so it gets deleted when the parent is deleted.
	m_avatar->addChildObject_p(wearableObject);

	createCustomizationLink(*wearableObject, *m_avatar, false);

	//-- Cause the wearer appearance to wear the wearable object.
	wearerAppearance->unlockWearables();
	wearerAppearance->wear(wearableObject);
	wearerAppearance->lockWearables();
}

// ----------------------------------------------------------------------------

void NpcCuiViewer::unwear()
{
	if(!m_avatar)
		return;

	Appearance * const app = m_avatar->getAppearance ();
	if (app)
	{
		SkeletalAppearance2 * const skelApp = app->asSkeletalAppearance2 ();
		if (skelApp)
		{
			int i = skelApp->getWearableCount() - 1;

			if(i >= 0)
			{
				SkeletalAppearance2 * s = skelApp->getWearableAppearance(i);

				Object * o = s->getOwner();

				skelApp->unlockWearables();
				skelApp->stopWearing(o);
				skelApp->lockWearables();

				delete o;
			}
		}
	}
}

// ----------------------------------------------------------------------------

void NpcCuiViewer::unwear(const char * wearableName)
{
	if(!m_avatar)
		return;

	//-- Get the skeletal appearance template.
	Appearance * app = m_avatar->getAppearance();
	if (!app)
	{
		DEBUG_WARNING(true, ("Object has a NULL appearance."));
		return;
	}

	SkeletalAppearance2 * skelApp = app->asSkeletalAppearance2 ();

	//-- Add mesh generators.
	int const count = skelApp->getWearableCount(); 
	for (int i = 0; i < count; ++i)
	{
		SkeletalAppearance2 * s = skelApp->getWearableAppearance(i);
		SkeletalAppearanceTemplate const *const appearanceTemplate = dynamic_cast<SkeletalAppearanceTemplate const*>(s->getAppearanceTemplate());

		if (!appearanceTemplate)
			continue;

		char const *const mgnName = appearanceTemplate->getMeshGeneratorName(0).getString();

		REPORT_LOG_PRINT(false, ("%d - '%s'\n", count, mgnName));

		if (!_stricmp(mgnName, wearableName))
		{
			Object * o = s->getOwner();

			skelApp->unlockWearables();
			skelApp->stopWearing(o);
			skelApp->lockWearables();

			delete o;

			return;
		}
	}
}

// ----------------------------------------------------------------------------

bool NpcCuiViewer::isWearing(const char * wearableName)
{
	if (!m_avatar)
		return false;

	//-- Get the skeletal appearance template.
	Appearance const *const app = m_avatar->getAppearance();
	if (!app)
	{
		DEBUG_WARNING(true, ("Object has a NULL appearance."));
		return false;
	}

	const SkeletalAppearance2 * skelApp = app->asSkeletalAppearance2 ();

	//-- Add mesh generators.
	int const count = skelApp->getWearableCount(); 
	for (int i = 0; i < count; ++i)
	{
		const SkeletalAppearance2 * s = skelApp->getWearableAppearance(i);

		if (!s)
			continue;

		SkeletalAppearanceTemplate const *const appearanceTemplate = dynamic_cast<SkeletalAppearanceTemplate const*>(s->getAppearanceTemplate());

		if (!appearanceTemplate)
			continue;

		char const *const mgnName = appearanceTemplate->getMeshGeneratorName(0).getString();

		REPORT_LOG_PRINT(true, ("%d - '%s'\n", count, mgnName));

		if (!_stricmp(mgnName, wearableName))
			return true;
	}

	// Success.
	return false;
}

// ----------------------------------------------------------------------------

void NpcCuiViewer::doneAddingWearables()
{
	/*
	m_wearableVolumePage->ForcePackChildren ();
	UISize cellSize = m_wearableVolumePage->GetCellSize ();
	cellSize.y = cellSize.x;
	m_wearableVolumePage->SetCellSize (cellSize);
	*/

	m_wearableVolumePage->Pack ();

	//-- how many cells?
	UIPoint cellCount = m_wearableVolumePage->GetCellCount();

	REPORT_LOG_PRINT(true, ("NpcCuiViewer::doneAddingWearables() - Cell Count = %ld,%ld\n", cellCount.x, cellCount.y));
}

// ----------------------------------------------------------------------------

void NpcCuiViewer::updateCurrentWearables()
{
	if(!m_avatar)
		return;

	ignore = true;

	clearCurrentWearables();

	//-- Get the skeletal appearance template.
	Appearance const *const app = m_avatar->getAppearance();
	if (!app)
	{
		DEBUG_WARNING(false, ("Object has a NULL appearance."));
		return;
	}

	const SkeletalAppearance2 * skelApp = app->asSkeletalAppearance2 ();

	if (!skelApp)
	{
		DEBUG_WARNING(true, ("Failed to get skeletal appearance template."));
		return;
	}

	//-- Add mesh generators.
	int const count = skelApp->getWearableCount(); 
	for (int i = 0; i < count; ++i)
	{
		const SkeletalAppearance2 * s = skelApp->getWearableAppearance(i);
		SkeletalAppearanceTemplate const *const appearanceTemplate = dynamic_cast<SkeletalAppearanceTemplate const*>(s->getAppearanceTemplate());

		if(!appearanceTemplate)
			continue;

		char const *const mgnName = appearanceTemplate->getMeshGeneratorName(0).getString();

		Object *wearable = createWearableFromMeshGenerator(mgnName);
		NOT_NULL(wearable);

		m_currentWearables.push_back(wearable);

		Object * owner = const_cast<Object *>(s->getOwner());
		createCustomizationLink(*wearable, *owner, true);

		CuiWidget3dObjectListViewer * const viewer = CuiIconManager::createObjectIcon (*wearable, 0);
		
		//-- the UI owns this, so it needs to alter it
		viewer->setAlterObjects(true);
		viewer->setCameraLodBias(3.0f);
		viewer->setCameraLodBiasOverride(true);
		viewer->setPaused(false);
		viewer->setForceDefaultClippingPlanes(true);

		viewer->CopyPropertiesFrom(*m_sampleWearableBox);
		viewer->SetTooltip(Unicode::narrowToWide(mgnName));
		viewer->SetVisible(true);
		viewer->AddCallback(this);
		viewer->SetSelectable(true);

		IGNORE_RETURN(viewer->SetProperty(s_wearable, Unicode::narrowToWide(mgnName)));
		IGNORE_RETURN(viewer->SetPropertyInteger(s_wearableNum, i));

		IGNORE_RETURN(m_currentVolumePage->AddChild(viewer));

		viewer->Link ();
	}

	ignore = false;
}

// ----------------------------------------------------------------------------

void NpcCuiViewer::createCustomizationLink (Object & wearable, Object & original, bool privateParts) const
{
	CustomizationDataProperty *const wearableCdProperty = dynamic_cast<CustomizationDataProperty*>(wearable.getProperty(CustomizationDataProperty::getClassPropertyId()));
	NOT_NULL(wearableCdProperty);

	CustomizationData *wearableCd = wearableCdProperty->fetchCustomizationData();
	NOT_NULL(wearableCd);

	CustomizationDataProperty *property = dynamic_cast<CustomizationDataProperty*>(original.getProperty(CustomizationDataProperty::getClassPropertyId()));
	NOT_NULL(property);

	CustomizationData *const customizationData = property->fetchCustomizationData();
	NOT_NULL(customizationData);

	IGNORE_RETURN(wearableCd->mountRemoteCustomizationData(*customizationData, CustomizationManager::cms_shared_owner, CustomizationManager::cms_shared_owner_no_slash));

	if(privateParts)
		IGNORE_RETURN(wearableCd->mountRemoteCustomizationData(*customizationData, "/private/", "/private"));
	
	wearableCd->release ();
	customizationData->release();
}

// ----------------------------------------------------------------------------

void NpcCuiViewer::getSelectedWearableName(std::string & wearableName) const
{
	if(!m_selectedWearable)
	{
		wearableName = "";
		return;
	}

	CuiWidget3dObjectListViewer * const viewer = dynamic_cast<CuiWidget3dObjectListViewer *>(m_currentVolumePage->GetLastSelectedChild ());

	UIString s;

	if(viewer && viewer->GetProperty(s_wearable, s))
		wearableName = Unicode::wideToNarrow(s).c_str();
	else
		wearableName = "";
}

// ----------------------------------------------------------------------------

void NpcCuiViewer::getSelectedWearableNum(int & wearableNum) const
{
	if(!m_selectedWearable)
	{
		wearableNum = -1;
		return;
	}

	CuiWidget3dObjectListViewer * const viewer = dynamic_cast<CuiWidget3dObjectListViewer *>(m_currentVolumePage->GetLastSelectedChild ());

	if(viewer && !viewer->GetPropertyInteger(s_wearableNum, wearableNum))
		wearableNum = -1;
}

// ----------------------------------------------------------------------------

void NpcCuiViewer::zoomBone(const char * bone)
{
	m_objectViewer->setCameraLookAtBone(bone);
	m_objectViewer->recomputeZoom();
}

// ----------------------------------------------------------------------------

void NpcCuiViewer::setFitDistanceFactor(float val)
{
	if(val > 64.0f) val = 64.0f;
	if(val <  0.6f) val =  0.6f;

	m_objectViewer->setFitDistanceFactor(val);
	m_objectViewer->recomputeZoom();
}

// ----------------------------------------------------------------------------

float NpcCuiViewer::getFitDistanceFactor() const
{
	float factor;

	if(!m_objectViewer->GetPropertyFloat(CuiWidget3dObjectListViewer::PropertyName::FitDistanceFactor, factor))
		return 0;

	return factor;
}

// ----------------------------------------------------------------------------

void NpcCuiViewer::setEnvironmentTexture(char const *baseFilename)
{
	std::string fullpath = "texture/";
	IGNORE_RETURN(fullpath.append(baseFilename));
	IGNORE_RETURN(fullpath.append(".dds"));

	const Texture* const texture = TextureList::fetch (fullpath.c_str());

	m_objectViewer->setEnvironmentTexture (texture);

	//-- object viewer keeps a reference to the texture so release this one
	texture->release();
}

// ----------------------------------------------------------------------------

CreatureObject * NpcCuiViewer::getCreature()
{
	return dynamic_cast<CreatureObject *>(m_avatar);
}

// ============================================================================
