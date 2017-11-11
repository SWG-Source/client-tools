//======================================================================
//
// SwgCuiCraftExperiment.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiCraftExperiment_Attrib.h"

#include "UIButton.h"
#include "UIColorEffector.h"
#include "UIComposite.h"
#include "UIData.h"
#include "UIRunner.h"
#include "UIManager.h"
#include "UIMessage.h"
#include "UIPage.h"
#include "UISliderbar.h"
#include "UIText.h"
#include "UIUtils.h"
#include "clientGame/Game.h"
#include "clientGame/ManufactureSchematicObject.h"
#include "clientUserInterface/CuiCraftManager.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiStringIdsCraft.h"
#include "clientUserInterface/CuiStringVariablesData.h"
#include "clientUserInterface/CuiStringVariablesManager.h"
#include "clientUserInterface/CuiWidget3dObjectListViewer.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedGame/SharedDraftSchematicObjectTemplate.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedObject/ObjectList.h"
#include "sharedObject/ObjectTemplateList.h"
#include "swgClientUserInterface/SwgCuiInventoryInfo.h"
#include "swgClientUserInterface/SwgCuiMediatorTypes.h"

#include <algorithm>
#include <list>

//======================================================================

namespace
{
	namespace Properties
	{
		const UILowerString marginValue = UILowerString ("marginValue");
	}
}

//----------------------------------------------------------------------

SwgCuiCraftExperiment::SwgCuiCraftExperiment (UIPage & page) :
CuiMediator             ("SwgCuiCraftExperiment", page),
UIEventCallback         (),
m_buttonNext            (0),
m_buttonExperiment      (0),
m_transition            (false),
m_callback              (new MessageDispatch::Callback),
m_attribMediators       (new AttribMediatorVector),
m_attribSample          (0),
m_compositeAttribs      (0),
m_compositePoints       (0),
m_samplePoint           (0),
m_pagePointsCurParent   (0),
m_pageComplexityCur     (0),
m_textPointsCur         (0),
m_textComplexityCur     (0),
m_textRiskPercent       (0),
m_runnerRisk            (0),
m_remainingPoints       (0),
m_updatePoints          (false),
m_effectorRisk          (new UIColorEffector (UIColor::white, 1.0f, false, true, false)),
m_effectorComplexity    (new UIColorEffector (UIColor::white, 1.0f, false, true, false)),
m_mediatorInfo          (0),
m_viewer                (0),
m_sessionEnded          (false),
m_coreLevelSlider       (0),
m_minCoreLevel          (0),
m_maxCoreLevel          (0),
m_experimentPage        (0),
m_coreLevelPage         (0),
m_coreLevelSet          (false),
m_experimentDefaultSize (),
m_initializePage        (true)
{
	m_effectorRisk->Attach (0);
	m_effectorComplexity->Attach (0);

	getCodeDataObject (TUIButton,    m_buttonNext,        "buttonNext");
	getCodeDataObject (TUIButton,    m_buttonExperiment,  "buttonExperiment");
	getCodeDataObject (TUIPage,      m_attribSample,      "attribSample");
	getCodeDataObject (TUIComposite, m_compositeAttribs,  "compositeAttribs");
	getCodeDataObject (TUIRunner,    m_runnerRisk,        "runnerRisk");
	getCodeDataObject (TUIText,      m_textRiskPercent,   "textRiskPercent");
	getCodeDataObject (TUIPage,      m_pageComplexityCur, "ComplexityCur");
	getCodeDataObject (TUIText,      m_textPointsCur,     "textPointsCur");
	getCodeDataObject (TUIText,      m_textComplexityCur, "textComplexityCur");

	getCodeDataObject (TUIComposite, m_compositePoints,   "compositePoints");
	getCodeDataObject (TUIPage,      m_samplePoint,       "sampleExpPoint");

	getCodeDataObject (TUISliderbar, m_coreLevelSlider,   "corelevel");
	getCodeDataObject (TUIText,      m_minCoreLevel,      "mincorelevel");
	getCodeDataObject (TUIText,      m_maxCoreLevel,      "maxcorelevel");

	getCodeDataObject (TUIPage,      m_experimentPage,    "exppage");
	getCodeDataObject (TUIPage,      m_coreLevelPage,     "corelevelpage");


	{
		UIPage * pageMediatorInfo = 0;
		getCodeDataObject (TUIPage,      pageMediatorInfo,      "pageMediatorInfo");		
		m_mediatorInfo = new SwgCuiInventoryInfo (*pageMediatorInfo);
		m_mediatorInfo->fetch ();
	}
	
	registerMediatorObject(*m_coreLevelSlider, true);

	m_textPointsCur->SetPreLocalized (true);
	m_textRiskPercent->SetPreLocalized (true);
	m_textComplexityCur->SetPreLocalized (true);
	m_minCoreLevel->SetPreLocalized(true);
	m_maxCoreLevel->SetPreLocalized(true);

	m_pagePointsCurParent = m_compositePoints->GetParentWidget ();

	registerMediatorObject (*m_pagePointsCurParent, true);
	registerMediatorObject (*m_buttonNext,          true);
	registerMediatorObject (*m_buttonExperiment,    true);

	m_attribSample->Attach (0);
	NON_NULL (m_attribSample->GetParent ())->RemoveChild (m_attribSample);

	m_attribSample->SetVisible (false);
	m_samplePoint->SetVisible (false);

	m_compositePoints->Clear ();

	setState    (MS_closeable);
}

//----------------------------------------------------------------------

SwgCuiCraftExperiment::~SwgCuiCraftExperiment ()
{
	m_mediatorInfo->release ();
	m_mediatorInfo      = 0;

	delete m_callback;
	m_callback          = 0;

	releaseMediators ();

	delete m_attribMediators;
	m_attribMediators      = 0;

	m_attribSample->Detach (0);
	m_attribSample         = 0;

	m_buttonNext           = 0;
	m_buttonExperiment     = 0;
	m_compositeAttribs     = 0;
	m_pageComplexityCur    = 0;
	m_remainingPoints      = 0;
	m_textPointsCur        = 0;
	m_textComplexityCur    = 0;
	m_pagePointsCurParent  = 0;

	m_effectorRisk->Detach (0);
	m_effectorComplexity->Detach (0);

	m_effectorRisk         = 0;
	m_effectorComplexity   = 0;

	m_coreLevelSlider = 0;
	m_minCoreLevel = 0;
	m_maxCoreLevel = 0;

	m_experimentPage = 0;
	m_coreLevelPage = 0;
}

//----------------------------------------------------------------------

void SwgCuiCraftExperiment::performActivate   ()
{
	m_sessionEnded = false;

	if (!CuiCraftManager::isCrafting ())
	{
		onSessionEnded (true);
		return;
	}

	bool coreCraftingWindow = false;

	// Grab the manufacturing schematic - this should never fail, but check to be sure.
	ManufactureSchematicObject * manuf = CuiCraftManager::getManufactureSchematic();

	if(manuf && m_initializePage)
	{
		// We need to grab the shared object template to grab the arguments we will be checking for.
		const SharedDraftSchematicObjectTemplate * sdsot = NULL;

		const ConstCharCrcString & schematicName = ObjectTemplateList::lookUp(manuf->getDraftSchematicSharedTemplate());
		if (!schematicName.isEmpty())
		{
			sdsot = dynamic_cast<const SharedDraftSchematicObjectTemplate *>(ObjectTemplateList::fetch(schematicName));
		}
		
		// If we found our shared object template.
		if(sdsot)
		{
			// Grab the number of attributes we have in the array.
			int attributeCount = sdsot->getAttributesCount();

			// Find out if we're using one of those new fancy weapon core schematics.
			StringId const corelevel("crafting", "coreLevel");

			for( int i = 0; i < attributeCount; ++i)
			{
				SharedDraftSchematicObjectTemplate::SchematicAttribute sattr;
				SharedDraftSchematicObjectTemplate::SchematicAttribute sattrMin;
				SharedDraftSchematicObjectTemplate::SchematicAttribute sattrMax;		
				
				// Attribute Name
				sdsot->getAttributes(sattr,       i);
				// Attribute Min Value
				sdsot->getAttributesMin(sattrMin, i);
				// Attribute Max Value
				sdsot->getAttributesMax(sattrMax, i);
				
				// If we found a core...
				if(sattr.name == corelevel)
				{
					// Set the Min - Max Local text
					char buffer[64];
					memset(buffer, 0, 64);
					sprintf(buffer, "%d", static_cast<int>(sattrMin.value));
					
					m_minCoreLevel->SetLocalText(Unicode::narrowToWide(buffer));
					m_maxCoreLevel->SetLocalText(Unicode::narrowToWide(buffer));
					
					// Set our Slider values.
					m_coreLevelSlider->SetLowerLimit(static_cast<long>(sattrMin.value));
					m_coreLevelSlider->SetUpperLimit(static_cast<long>(sattrMax.value));
					m_coreLevelSlider->SetIncrementValue(2);
					m_coreLevelSlider->SetValue(static_cast<long>(sattrMin.value), true);
					m_coreLevelSlider->SetEnabled(true);
					
					// We are indeed a core crafting window.
					coreCraftingWindow = true;
					m_coreLevelSet = false;

					m_coreLevelPage->SetVisible(true);

					break;

				}
			}
			// Release our reference to the shared object template.
			sdsot->releaseReference();
		}
	}

	// If we're not a core crafting window.
	if(!coreCraftingWindow && m_initializePage)
	{
		// Turn off the Core page, enlarge the experimentation page.
		m_coreLevelPage->SetVisible(false);
		m_coreLevelSlider->SetEnabled(false);
		m_coreLevelSet = true;
	}

	m_coreLevelSlider->AddCallback(this);

	m_buttonNext->SetEnabled(true);

	CuiManager::requestPointer(true);

	m_mediatorInfo->activate ();

	m_runnerRisk->ToggleAll (false);

	releaseMediators ();

	m_remainingPoints = CuiCraftManager::findPlayerCurrentExperimentPoints (true);

	populateAttribs ();
	updateAttribs   ();
	updatePoints    ();

	m_transition    = false;

	m_callback->connect (*this, &SwgCuiCraftExperiment::onCustomize,          static_cast<CuiCraftManager::Messages::Customize*>     (0));
	m_callback->connect (*this, &SwgCuiCraftExperiment::onExperiment,         static_cast<CuiCraftManager::Messages::Experiment*>    (0));
	m_callback->connect (*this, &SwgCuiCraftExperiment::onSessionEnded,       static_cast<CuiCraftManager::Messages::SessionEnded *>     (0));
	m_callback->connect (*this, &SwgCuiCraftExperiment::onComplexityChange,   static_cast<ClientObject::Messages::ComplexityChange*>     (0));

	setIsUpdating (true);

	m_initializePage = false;
}

//----------------------------------------------------------------------

void SwgCuiCraftExperiment::performDeactivate ()
{
	CuiManager::requestPointer (false);

	setIsUpdating (false);

	m_callback->disconnect (*this, &SwgCuiCraftExperiment::onCustomize,          static_cast<CuiCraftManager::Messages::Customize*>     (0));
	m_callback->disconnect (*this, &SwgCuiCraftExperiment::onExperiment,         static_cast<CuiCraftManager::Messages::Experiment*>    (0));
	m_callback->disconnect (*this, &SwgCuiCraftExperiment::onSessionEnded,       static_cast<CuiCraftManager::Messages::SessionEnded *>     (0));
	m_callback->disconnect (*this, &SwgCuiCraftExperiment::onComplexityChange,   static_cast<ClientObject::Messages::ComplexityChange*>     (0));

	releaseMediators ();
	m_mediatorInfo->deactivate ();

	m_compositeAttribs->Clear ();

	m_coreLevelSlider->RemoveCallback(this);
}

//----------------------------------------------------------------------

void SwgCuiCraftExperiment::OnButtonPressed( UIWidget *context )
{
	if (context == m_buttonNext)
	{
		if (m_remainingPoints)
		{
			CuiStringVariablesData csvd;
			csvd.digit_i = m_remainingPoints;

			Unicode::String str;
			if(m_coreLevelSlider->IsEnabled())
				CuiStringVariablesManager::process (CuiStringIdsCraft::confirm_exp_done_core_level, csvd, str);
			else
				CuiStringVariablesManager::process (CuiStringIdsCraft::confirm_exp_done_prose, csvd, str);
			CuiMessageBox * const box = CuiMessageBox::createYesNoBox (str);
			m_callback->connect (box->getTransceiverClosed (), *this, &SwgCuiCraftExperiment::onMessageBoxClosedNext);
			m_buttonNext->SetEnabled(false);
			return;
		}

		m_buttonNext->SetEnabled(false);
		CuiCraftManager::customize ();
	}
	else if (context == m_buttonExperiment)
	{
		//-- we might need some sort of specialized confirmation dialog here
		if(!m_coreLevelSet)
		{
			CuiStringVariablesData csvd;
			csvd.digit_i = static_cast<int>(m_coreLevelSlider->GetValue());

			Unicode::String message;
			CuiStringVariablesManager::process (CuiStringIdsCraft::confirm_core_level_setting, csvd, message);
			CuiMessageBox * const box = CuiMessageBox::createYesNoBox (message);
			m_callback->connect (box->getTransceiverClosed (), *this, &SwgCuiCraftExperiment::onExperimentBoxClosedNext);

			return;
		}

		CuiCraftManager::IntPairVector v;
		v.reserve (m_attribMediators->size ());

		int i = 0;
		for (AttribMediatorVector::iterator it = m_attribMediators->begin (); it != m_attribMediators->end (); ++it, ++i)
			v.push_back (std::make_pair (i, (*it)->getExp ()));

		int coreValue = static_cast<int>(m_coreLevelSlider->GetValue());

		CuiCraftManager::experiment (v, coreValue);
	}
}

//----------------------------------------------------------------------

bool SwgCuiCraftExperiment::OnMessage (UIWidget * context, const UIMessage & msg)
{
	if (context == m_pagePointsCurParent)
	{
		if (msg.Type == UIMessage::LeftMouseDown)
		{
			recoverAllExperimentPoints ();
			return false;
		}
	}

	return true;
}

//----------------------------------------------------------------------

void SwgCuiCraftExperiment::onMessageBoxClosedNext (const CuiMessageBox & box)
{
	if (box.completedAffirmative ())
		CuiCraftManager::customize ();
	else
		m_buttonNext->SetEnabled(true);
}


void SwgCuiCraftExperiment::onExperimentBoxClosedNext(const CuiMessageBox & box)
{
	if(box.completedAffirmative())
	{
		CuiCraftManager::IntPairVector v;
		v.reserve (m_attribMediators->size ());

		int i = 0;
		for (AttribMediatorVector::iterator it = m_attribMediators->begin (); it != m_attribMediators->end (); ++it, ++i)
			v.push_back (std::make_pair (i, (*it)->getExp ()));

		int coreValue = static_cast<int>(m_coreLevelSlider->GetValue());

		CuiCraftManager::experiment (v, coreValue);

		m_coreLevelSlider->SetEnabled(false);

		m_coreLevelSet = true;
	}
}
//----------------------------------------------------------------------

void SwgCuiCraftExperiment::update (float deltaTimeSecs)
{
	CuiMediator::update (deltaTimeSecs);

	if (m_transition)
	{
		m_transition = false;
		deactivate ();
		
		if (CuiCraftManager::canManufacture())
			CuiMediatorFactory::activateInWorkspace (CuiMediatorTypes::WS_CraftOption);
		else
		{
			CuiCraftManager::setFinalState (CuiCraftManager::FS_proto);
			CuiMediatorFactory::activateInWorkspace (CuiMediatorTypes::WS_CraftCustomize);
		}
	}

	else if (m_updatePoints)
	{
		m_updatePoints = false;
		updatePoints ();
	}
}

//----------------------------------------------------------------------

void SwgCuiCraftExperiment::recoverAllExperimentPoints ()
{	
	for (AttribMediatorVector::iterator it = m_attribMediators->begin (); it != m_attribMediators->end (); ++it)
	{
		Attrib * const attrib = *it;
		attrib->setExp (0);
	}
}

//----------------------------------------------------------------------

void SwgCuiCraftExperiment::onCustomize (const int & )
{
	m_transition = true;
};

//----------------------------------------------------------------------

void SwgCuiCraftExperiment::onExperiment (const int &)
{
	//-- force an attribute re-fetch for info
	m_mediatorInfo->setInfoObject (m_mediatorInfo->getInfoObject (), true);

	updateAttribs ();
	updatePoints  ();
};

//----------------------------------------------------------------------

void SwgCuiCraftExperiment::releaseMediators ()
{
	std::for_each (m_attribMediators->begin (), m_attribMediators->end (), ReleaserFunctor ());
	m_attribMediators->clear ();

	UIBaseObject::UIObjectList olist;
	m_compositeAttribs->GetChildren (olist);

	for (UIBaseObject::UIObjectList::const_iterator it = olist.begin (); it != olist.end (); ++it)
		m_compositeAttribs->RemoveChild (*it);
}

//-----------------------------------------------------------------

void SwgCuiCraftExperiment::updatePoints ()
{
	const int curPoints       = CuiCraftManager::findPlayerCurrentExperimentPoints ();
	
	m_buttonExperiment->SetEnabled (m_remainingPoints < curPoints);

	{
		Unicode::String str;
		UIUtils::FormatLong (str, m_remainingPoints);
		m_textPointsCur->SetLocalText (str);
	}

	{		
		const UIBaseObject::UIObjectList & olist = m_compositePoints->GetChildrenRef ();

		while (static_cast<int>(olist.size ()) < curPoints)
		{
			UIBaseObject * const dupe = m_samplePoint->DuplicateObject ();
			m_compositePoints->AddChild (dupe);
			dupe->Link ();
		}

		int count = 0;
		for (UIBaseObject::UIObjectList::const_reverse_iterator it = olist.rbegin (); it != olist.rend (); ++it, ++count)
		{
			UIWidget * const widget = safe_cast<UIWidget *>(*it);
			if (count < curPoints)
			{
				widget->SetVisible (true);

				if (count < m_remainingPoints)
					widget->SetEnabled (true);
				else
					widget->SetEnabled (false);
			}
			else
				widget->SetVisible (false);
		}

		m_compositePoints->Pack ();
	}

	//-----------------------------------------------------------------
	//-- compute the risk
	
	
	int riskMod = 0;
	int numAttributesModified = 0;
	int maxAttributeRisk      = 0;
	
	const ManufactureSchematicObject * const manf_schem = CuiCraftManager::getManufactureSchematic ();
	if (manf_schem)
	{
		riskMod = static_cast<int>(manf_schem->getExperimentMod());
	}

	for (AttribMediatorVector::iterator it = m_attribMediators->begin (); it != m_attribMediators->end (); ++it)
	{
		Attrib * const attrib = *it;
		attrib->setExpPointsAvailable (m_remainingPoints);
		
		if (attrib->getExp ())
		{
			++numAttributesModified;
			const int risk = 100 - ((100 - attrib->getExp () * 10) / 2 + riskMod);
			maxAttributeRisk = std::max (maxAttributeRisk, risk);
		}
	}
	
	if (numAttributesModified > 0)
		maxAttributeRisk += (numAttributesModified - 1) * 5;
	
	{
		char buf [64];
		snprintf (buf, sizeof (buf), "%d%%", maxAttributeRisk);
		m_textRiskPercent->SetLocalText (Unicode::narrowToWide (buf));

		if (maxAttributeRisk)
		{
			m_runnerRisk->SetRunDelayAdvance (std::max (0, 100 - maxAttributeRisk) * 4);
			m_runnerRisk->SetEnabled (true);
			m_runnerRisk->SetPropertyInteger (UIRunner::PropertyName::RunLoops, 0);
			m_runnerRisk->SetPropertyInteger (UIRunner::PropertyName::RunAdvance, 1);
		}
		else
		{
			m_runnerRisk->SetPropertyInteger (UIRunner::PropertyName::RunAdvance, -1);
			m_runnerRisk->SetPropertyInteger (UIRunner::PropertyName::RunLoops, 1);
			m_runnerRisk->SetEnabled (false);
		}

		m_runnerRisk->SetPaused (false);
	}
}

//-----------------------------------------------------------------

int SwgCuiCraftExperiment::fetchPoints   (int requestedCount)
{
	const int retval = std::min (m_remainingPoints, requestedCount);

	if (retval)
	{
		m_remainingPoints -= retval;
		m_updatePoints = true;
	}

	return retval;
}

//-----------------------------------------------------------------

int SwgCuiCraftExperiment::releasePoints (int requestedCount)
{
	const int curPoints       = CuiCraftManager::findPlayerCurrentExperimentPoints ();
	const int retval = std::min (curPoints - m_remainingPoints, requestedCount);

	if (retval)
	{
		m_remainingPoints += retval;
		m_updatePoints = true;
	}

	return retval;
}

//----------------------------------------------------------------------

SwgCuiCraftExperiment::Attrib * SwgCuiCraftExperiment::createAttribMediator (int attribIndex)
{
	//-- DuplicateObject requires a parent

	m_attribSample->SetParent (&getPage ());
	UIPage * const attribDupe = dynamic_cast<UIPage *>(m_attribSample->DuplicateObject ());		
	m_attribSample->SetParent (0);

	NOT_NULL (attribDupe);
	attribDupe->SetVisible        (true);
	attribDupe->SetHeight         (m_compositeAttribs->GetHeight ());
	attribDupe->SetLocation       (0L, 0L);
	m_compositeAttribs->AddChild  (attribDupe);
	attribDupe->Link              ();
	attribDupe->ForcePackChildren ();
	
	Attrib * const attribMediator = new Attrib (*attribDupe, *this);
	attribMediator->fetch        ();
	attribMediator->activate     ();
	
	StringId id;
	float min = 0, max = 0, cur = 0, resourceMax = 0;
	CuiCraftManager::findManufactureSchematicAttributeValues (attribIndex, id, min, max, cur, resourceMax);
	attribMediator->setAttrib(attribIndex, id, min, max, cur, resourceMax);

	attribMediator->setExp       (0);
	m_attribMediators->push_back (attribMediator);
	
	return attribMediator;
}


//----------------------------------------------------------------------

void SwgCuiCraftExperiment::updateAttribs ()
{
	for (AttribMediatorVector::iterator it = m_attribMediators->begin (); it != m_attribMediators->end (); ++it)
	{
		Attrib * const attribMediator = *it;
		int attribIndex = attribMediator->getIndex();
		StringId attribName;
		float min = 0, max = 0, cur = 0, resourceMax = 0;
		CuiCraftManager::findManufactureSchematicAttributeValues (attribIndex, attribName, min, max, cur, resourceMax);
		attribMediator->setAttrib(attribIndex, attribName, min, max, cur, resourceMax);
	}

	updateComplexity ();

	{
		ClientObject * const test_proto = CuiCraftManager::getSchematicPrototype ();
		m_mediatorInfo->setInfoObject (test_proto);
	}
}

//----------------------------------------------------------------------

void SwgCuiCraftExperiment::updateComplexity ()
{
	ClientObject * const test_proto = CuiCraftManager::getSchematicPrototype ();

	if (!test_proto)
		return;

	const int complexity = static_cast<int>(test_proto->getComplexity ());
	
	const UIWidget * const parent = NON_NULL (safe_cast<UIWidget *>(m_pageComplexityCur->GetParent ()));
	const long parentHeight    = parent->GetHeight ();
	UIRect margin;
	parent->GetPropertyRect (Properties::marginValue, margin);
	const long usableHeight = parentHeight - margin.top - margin.bottom;
	
	m_pageComplexityCur->SetHeight (complexity * usableHeight / 100);
	m_pageComplexityCur->SetLocation (m_pageComplexityCur->GetLocation ().x, parentHeight - m_pageComplexityCur->GetHeight () - margin.bottom); 
	m_effectorComplexity->SetTargetColor (UIColor::lerp (UIColor::cyan, UIColor::violet, static_cast<float>(complexity) * 0.01f));
	
	UIManager::gUIManager ().CancelEffector ( m_effectorComplexity, m_pageComplexityCur );
	UIManager::gUIManager ().ExecuteEffector( m_effectorComplexity, m_pageComplexityCur );
	
	Unicode::String str;
	UIUtils::FormatLong (str, complexity);
	m_textComplexityCur->SetLocalText (str);
}

//----------------------------------------------------------------------

void SwgCuiCraftExperiment::populateAttribs ()
{
	// We assume that releaseMediators () has already been called
	
	if (!Game::getSinglePlayer ())
	{
		ManufactureSchematicObject * const manf_schem = CuiCraftManager::getManufactureSchematic ();
		if (!manf_schem)
			return;
		
		int attributeCount = manf_schem->getExperimentAttribCount();
		
		for (int i = 0; i < attributeCount; ++i)
		{			
			NON_NULL (createAttribMediator (i));
		}
	}
	else
	{
		{
			const StringId id ("attr", "one");
			Attrib * const attribMediator = NON_NULL (createAttribMediator (0));
			attribMediator->setAttrib (0, id, 0.05f, 1.00f, 0.20f, 0.90f);

		}
		{
			const StringId id ("attr", "two");
			Attrib * const attribMediator = NON_NULL (createAttribMediator (1));
			attribMediator->setAttrib (1, id, 0.0f, 0.50f, 0.05f, 0.30f);
		}
		{
			const StringId id ("attr", "three");
			Attrib * const attribMediator = NON_NULL (createAttribMediator (2));
			attribMediator->setAttrib (2, id, 0.05f, 0.70f, 0.30f, 0.60f);

		}
	}

	m_compositeAttribs->ForcePackChildren ();
}

//----------------------------------------------------------------------

void SwgCuiCraftExperiment::onSessionEnded (const bool &)
{
	m_sessionEnded = true;
	closeNextFrame ();
}

//----------------------------------------------------------------------

bool SwgCuiCraftExperiment::close ()
{
	CuiCraftManager::stopCrafting (m_sessionEnded);
	return true;
}

//----------------------------------------------------------------------

void SwgCuiCraftExperiment::onComplexityChange         (const ClientObject & obj)
{
	ClientObject * const test_proto = CuiCraftManager::getSchematicPrototype ();

	if (test_proto == &obj)
		updateComplexity ();
}

void SwgCuiCraftExperiment::OnSliderbarChanged(UIWidget *context)
{
	if(context == m_coreLevelSlider)
	{
		long value = m_coreLevelSlider->GetValue();
		
		char buffer[32];
		memset(buffer, 32, 0);
		sprintf(buffer, "%d", value);

		m_maxCoreLevel->SetLocalText(Unicode::narrowToWide(buffer));
	}
}

void SwgCuiCraftExperiment::setInitializePage(bool init)
{
	m_initializePage = init;
}
//======================================================================

