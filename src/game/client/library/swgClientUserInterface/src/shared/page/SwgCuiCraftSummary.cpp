//======================================================================
//
// SwgCuiCraftExperiment.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiCraftSummary.h"

//======================================================================

#include "UIButton.h"
#include "UIClock.h"
#include "UIData.h"
#include "UIDataSource.h"
#include "UIPage.h"
#include "UITable.h"
#include "UITableModelDefault.h"
#include "UIText.h"
#include "UIUtils.h"
#include "clientGame/DraftSchematicInfo.h"
#include "clientGame/DraftSchematicManager.h"
#include "clientGame/Game.h"
#include "clientGame/ManufactureSchematicObject.h"
#include "clientUserInterface/CuiCraftManager.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedRandom/Random.h"
#include "swgClientUserInterface/SwgCuiInventoryInfo.h"
#include "swgClientUserInterface/SwgCuiMediatorTypes.h"
#include <cstdio>

//======================================================================

SwgCuiCraftSummary::SwgCuiCraftSummary (UIPage & page) :
CuiMediator     ("SwgCuiCraftSummary", page),
UIEventCallback (),
UINotification  (),
m_buttonNext    (0),
m_transition    (false),
m_mediatorInfo  (0),
m_table         (0),
m_pageSampleBar (0),
m_bars          (new PageVector),
m_callback      (new MessageDispatch::Callback),
m_textSuccess   (0),
m_sessionEnded  (false)
{
	getCodeDataObject (TUIButton,  m_buttonNext,        "buttonNext");
	getCodeDataObject (TUITable,   m_table,             "table");
	getCodeDataObject (TUIPage,    m_pageSampleBar,     "pageSampleBar");
	getCodeDataObject (TUIText,    m_textSuccess,       "textSuccess");

	m_textSuccess->SetPreLocalized (true);
	m_textSuccess->Clear ();

	{
		UIPage * page = 0;
		getCodeDataObject (TUIPage,    page,        "pageMediatorInfo");
		m_mediatorInfo = new SwgCuiInventoryInfo (*page);
		m_mediatorInfo->fetch ();
	}

	m_pageSampleBar->SetVisible (false);

	setState    (MS_closeable);
}

//----------------------------------------------------------------------

SwgCuiCraftSummary::~SwgCuiCraftSummary ()
{
	delete m_callback;
	m_callback = 0;

	delete m_bars;
	m_bars = 0;

	m_mediatorInfo->release ();
	m_mediatorInfo = 0;

	m_buttonNext = 0;
	m_table      = 0;
}

//----------------------------------------------------------------------

void SwgCuiCraftSummary::performActivate   ()
{
	m_sessionEnded = false;

	if (!CuiCraftManager::isCrafting ())
	{
		onSessionEnded (true);
		return;
	}

	m_textSuccess->SetLocalText (CuiCraftManager::getLocalizedAssemblyResult ());

	CuiManager::requestPointer (true);

	m_mediatorInfo->activate ();

	m_transition = false;
	m_buttonNext->AddCallback (this);

	UITableModelDefault * const model = dynamic_cast<UITableModelDefault *>(m_table->GetTableModel ());
	if (model)
	{
		model->ClearData ();
		Unicode::String tooltip;

		ManufactureSchematicObject * const manf_schem = CuiCraftManager::getManufactureSchematic ();
		if (manf_schem)
		{
			const std::pair<uint32, uint32> & draftCrc = CuiCraftManager::getCurrentDraftSchematicCrc ();

			const DraftSchematicInfo * const dsi = DraftSchematicManager::findDraftSchematic (draftCrc);

			UIDataSource * const ds [3] =
			{
				NON_NULL (model->GetColumnDataSource (0)),
				NON_NULL (model->GetColumnDataSource (1)),
				NON_NULL (model->GetColumnDataSource (2))
			};

			int attributeCount = manf_schem->getExperimentAttribCount();

			if (Game::getSinglePlayer ())
				attributeCount = 5;

			m_table->SetVisible (attributeCount > 0);

			int bar_index = 0;
			m_bars->reserve (attributeCount);

			for (int i = 0; i < attributeCount; ++i, ++bar_index)
			{
				StringId id;
				float min = 0, max = 0, cur = 0, resourceMax = 0;
				if (Game::getSinglePlayer ())
				{
					char buf [32];
					_snprintf (buf, sizeof (buf), "string_%d", i);
					id  = StringId ("client_test_table", buf);
					min         = 10;
					max         = 120;
					resourceMax = 90;
					if (i == 0)
						cur = min;
					else
						cur         = Random::randomReal (min, resourceMax);
				}
				else
					CuiCraftManager::findManufactureSchematicAttributeValues (i, id, min, max, cur, resourceMax);

				tooltip.clear ();
				if (dsi)
					dsi->formatDraftAttribWeights (tooltip, id.localize (), false);

				const float range   = max - min;
				const float percent = range ? ((cur - min) * 100.0f / range) : 0;

				const std::string debugString (id.getDebugString ());

				{
					UIData * const data = new UIData;
					Unicode::String str;

					//-- hack to get around upper case attribute names
					const StringId lowerId (id.getTable (), Unicode::toLower (id.getText ()));
					lowerId.localize (str);
					data->SetProperty (UITableModelDefault::DataProperties::Value, str);
					data->SetProperty (UITableModelDefault::DataProperties::LocalTooltip, tooltip);

					ds[0]->AddChild (data);
				}

				{
					UIData * const data = new UIData;
					Unicode::String str;
					UIUtils::FormatLong (str, static_cast<long>(percent));
					data->SetProperty (UITableModelDefault::DataProperties::Value, str);
					data->SetProperty (UITableModelDefault::DataProperties::LocalTooltip, tooltip);

					ds[1]->AddChild (data);
				}


				{
					UIData * const data = new UIData;

					UIPage * bar = 0;

					if (static_cast<int>(m_bars->size ()) > bar_index)
					{
						bar = (*m_bars) [bar_index];
					}
					else
					{
						bar = dynamic_cast<UIPage *>(m_pageSampleBar->DuplicateObject ());
						bar->SetVisible (false);
						bar->Attach (0);
						m_bars->push_back (bar);
						getPage ().AddChild (bar);
						bar->Link ();
					}

					bar->SetName (debugString.c_str ());
					UIWidget * const valueWidget = dynamic_cast<UIWidget *>(bar->GetObjectFromPath ("value", TUIWidget));
					bar->SetWidth (100 + valueWidget->GetLocation ().x * 2L);
					bar->SetLocation (UIPoint::zero);
					//-- always show a little bit of the bar, even at 0%
					valueWidget->SetWidth (5 + static_cast<int>(percent * 0.95f));
					ds[2]->AddChild (data);

					Unicode::String str;
					m_table->GetPathTo (str, bar);
					data->SetProperty        (UITableModelDefault::DataProperties::Value, str);
					data->SetProperty        (UITableModelDefault::DataProperties::LocalTooltip, tooltip);
					bar->SetPropertyInteger  (UITableModel::DataProperties::WidgetValue, static_cast<int>(percent));
				}
			}
		}

		model->fireColumnsChanged ();
		model->fireDataChanged ();
	}

	ClientObject * const test_proto = CuiCraftManager::getSchematicPrototype ();

	m_mediatorInfo->setInfoObject (test_proto);

	m_callback->connect (*this, &SwgCuiCraftSummary::onSessionEnded,      static_cast<CuiCraftManager::Messages::SessionEnded *>     (0));

}

//----------------------------------------------------------------------

void SwgCuiCraftSummary::performDeactivate ()
{
	CuiManager::requestPointer (false);

	m_callback->disconnect (*this, &SwgCuiCraftSummary::onSessionEnded,      static_cast<CuiCraftManager::Messages::SessionEnded *>     (0));

	UIClock::gUIClock ().StopListening (this);
	m_buttonNext->RemoveCallback (this);

	m_mediatorInfo->deactivate ();

	for (PageVector::iterator it = m_bars->begin (); it != m_bars->end (); ++it)
	{
		getPage ().RemoveChild (*it);
		(*it)->Detach (0);
	}

	m_bars->clear ();
}

//----------------------------------------------------------------------

void SwgCuiCraftSummary::OnButtonPressed( UIWidget *context )
{
	if (context == m_buttonNext)
	{
		deactivate ();

		if (CuiCraftManager::findCraftingLevel() >= 2 &&
			(CuiCraftManager::canManufacture() || CuiCraftManager::canExperiment()))
		{
			CuiMediatorFactory::activateInWorkspace (CuiMediatorTypes::WS_CraftOption);
		}
		else
		{
			CuiCraftManager::setFinalState (CuiCraftManager::FS_proto);
			CuiMediatorFactory::activateInWorkspace (CuiMediatorTypes::WS_CraftCustomize);
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiCraftSummary::Notify( UINotificationServer *, UIBaseObject *, Code )
{
	if (m_transition)
	{
		UIClock::gUIClock ().StopListening (this);
		m_transition = false;
		deactivate ();
	}
}

//----------------------------------------------------------------------

void SwgCuiCraftSummary::onSessionEnded (const bool &)
{
	m_sessionEnded = true;
	closeNextFrame ();
}

//----------------------------------------------------------------------

bool SwgCuiCraftSummary::close ()
{
	CuiCraftManager::stopCrafting (m_sessionEnded);
	return true;
}

//======================================================================
