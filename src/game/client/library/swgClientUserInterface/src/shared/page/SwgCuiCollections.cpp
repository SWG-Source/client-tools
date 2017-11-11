// ======================================================================
//
// SwgCuiCollections.cpp
// copyright (c) 2007 Sony Online Entertainment LLC
//
// ======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiCollections.h"

#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/PlayerObject.h"
#include "clientGame/ProsePackageManagerClient.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiStringVariablesManager.h"
#include "sharedFoundation/CalendarTime.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedFoundation/Watcher.h"
#include "sharedGame/CollectionsDataTable.h"
#include "sharedGame/ProsePackage.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/ShaderCapability.h"

#include "UICheckbox.h"
#include "UIComposite.h"
#include "UIImage.h"
#include "UIImageFragment.h"
#include "UIImageStyle.h"
#include "UINamespace.h"
#include "UIPage.h"
#include "UIText.h"
#include "UIVolumePage.h"

// ======================================================================

namespace SwgCuiCollectionsNamespace
{
	void const * ms_lastTarget = 0;
	int ms_lastBookSelectedIndex = -1;
}

using namespace SwgCuiCollectionsNamespace;

// ======================================================================

SwgCuiCollections::SwgCuiCollections(UIPage & page)
: CuiMediator("SwgCuiCollections", page)
, UIEventCallback()
, m_callback(new MessageDispatch::Callback)
, m_books(0)
, m_pages(0)
, m_sampleCollection(0)
, m_samplePage(0)
, m_sampleIcon(0)
, m_bookNameText(0)
, m_captionText(0)
, m_showCompleted(0)
, m_imageStyleMissing(0)
, m_imageStyleMissingGray(0)
, m_imageStyleUnknown(0)
, m_imageStyleNamespace(0)
, m_target(new Watcher<CreatureObject>)
{
	getCodeDataObject(TUIVolumePage, m_books, "books");
	getCodeDataObject(TUIComposite, m_pages, "pages");
	getCodeDataObject(TUIComposite, m_samplePage, "samplePage");
	getCodeDataObject(TUIPage, m_sampleCollection, "sampleCollection");
	getCodeDataObject(TUIPage, m_sampleIcon, "sampleIcon");
	getCodeDataObject(TUIText, m_bookNameText, "bookNameText");
	getCodeDataObject(TUIText, m_captionText, "captionText");
	getCodeDataObject(TUICheckbox, m_showCompleted, "showCompleted");
	getCodeDataObject(TUIImageStyle, m_imageStyleMissing, "imageStyleMissing");
	getCodeDataObject(TUIImageStyle, m_imageStyleMissingGray, "imageStyleMissingGray");
	getCodeDataObject(TUIImageStyle, m_imageStyleUnknown, "imageStyleUnknown");
	getCodeDataObject(TUINamespace, m_imageStyleNamespace, "imageStyleNamespace");

	m_samplePage->SetVisible(false);
	m_sampleCollection->SetVisible(false);
	m_sampleIcon->SetVisible(false);

	m_samplePage = NON_NULL(safe_cast<UIComposite *>(m_samplePage->DuplicateObject()));
	m_sampleCollection = NON_NULL(safe_cast<UIPage *>(m_sampleCollection->DuplicateObject()));
	m_sampleIcon = NON_NULL(safe_cast<UIPage *>(m_sampleIcon->DuplicateObject()));

	m_books->Clear();
	m_pages->Clear();

	// this window cannot be user resized so do not save or load size settings
	setSettingsAutoSizeLocation(false, true);

	setState(MS_closeable);
	setState(MS_closeDeactivates);

	m_showCompleted->SetChecked(CuiPreferences::getShowCompletedCollections());
}

// ----------------------------------------------------------------------

SwgCuiCollections::~SwgCuiCollections()
{
	m_books = 0;
	m_pages = 0;

	m_samplePage->Destroy();
	m_samplePage = 0;

	m_sampleCollection->Destroy();
	m_sampleCollection = 0;

	m_sampleIcon->Destroy();
	m_sampleIcon = 0;

	m_bookNameText = 0;
	m_captionText = 0;
	m_showCompleted = 0;

	m_imageStyleMissing = 0;
	m_imageStyleMissingGray = 0;
	m_imageStyleUnknown = 0;
	m_imageStyleNamespace = 0;

	delete m_target;
	m_target = 0;

	delete m_callback;
	m_callback = 0;
}

// ----------------------------------------------------------------------

void SwgCuiCollections::performActivate()
{
	CuiManager::requestPointer(true);
	setIsUpdating(true);

	m_books->AddCallback(this);
	m_showCompleted->AddCallback(this);

	m_callback->connect(*this, &SwgCuiCollections::onCollectionsChanged, static_cast<PlayerObject::Messages::CollectionsChanged *>(0));
	m_callback->connect(*this, &SwgCuiCollections::onCollectionServerFirstChanged, static_cast<Game::Messages::CollectionServerFirstChanged *>(0));
	m_callback->connect(*this, &SwgCuiCollections::onCollectionShowServerFirstOptionChanged, static_cast<Game::Messages::CollectionShowServerFirstOptionChanged *>(0));

	// request for updated list of collection "server first" if the list has changed
	Game::requestCollectionServerFirstList();
}

// ----------------------------------------------------------------------

void SwgCuiCollections::performDeactivate ()
{
	CuiManager::requestPointer(false);
	setIsUpdating(false);

	m_books->RemoveCallback(this);
	m_showCompleted->RemoveCallback(this);

	m_callback->disconnect(*this, &SwgCuiCollections::onCollectionsChanged, static_cast<PlayerObject::Messages::CollectionsChanged *>(0));
	m_callback->disconnect(*this, &SwgCuiCollections::onCollectionServerFirstChanged, static_cast<Game::Messages::CollectionServerFirstChanged *>(0));
	m_callback->disconnect(*this, &SwgCuiCollections::onCollectionShowServerFirstOptionChanged, static_cast<Game::Messages::CollectionShowServerFirstOptionChanged *>(0));
}

// ----------------------------------------------------------------------

void SwgCuiCollections::OnVolumePageSelectionChanged(UIWidget * context)
{
	if (context == m_books && m_books->GetLastSelectedChild())
		updatePages(ms_lastBookSelectedIndex == m_books->GetLastSelectedIndex());
}

// ----------------------------------------------------------------------

void SwgCuiCollections::OnGenericSelectionChanged(UIWidget * context)
{
	if (context == m_showCompleted)
	{
		updatePages(false);
		CuiPreferences::setShowCompletedCollections(m_showCompleted->IsChecked());
	}
}

// ----------------------------------------------------------------------

void SwgCuiCollections::setTarget(CreatureObject * const target)
{
	*m_target = target;

	if (target)
	{
		Unicode::String result;

		CuiStringVariablesManager::process(StringId("ui", "cpt_collections"), Unicode::emptyString, target->getLocalizedName(), Unicode::emptyString, result);
		m_captionText->SetText(result);
	}
	else
		m_captionText->SetText(Unicode::emptyString);

	bool const sameTarget = ms_lastTarget == target;

	if (!sameTarget)
		m_pages->ScrollToPoint(UIPoint(0, 0));

	updateBooks(sameTarget);

	if (!getPage().GetParent())
		updatePages(false);

	ms_lastTarget = target;
}

// ----------------------------------------------------------------------

void SwgCuiCollections::update(float deltaTimeSecs)
{
	CuiMediator::update(deltaTimeSecs);

	if (!m_target->getPointer())
	{
		closeNextFrame();
		return;
	}
}

// ----------------------------------------------------------------------

void SwgCuiCollections::updateBooks(bool const restoreSelectedBook)
{
	std::string const lastBookSelected = m_books->GetLastSelectedChild() ? m_books->GetLastSelectedChild()->GetName() : "";

	m_books->Clear();
	m_bookNameText->SetLocalText(Unicode::emptyString);
	m_pages->Clear();

	PlayerObject const * const playerObject = getPlayerObject();

	if (playerObject)
	{
		std::vector<std::string> bookNames;
		playerObject->getActiveCollectionBooks(&bookNames);

		for (std::vector<std::string>::const_iterator bookIter = bookNames.begin(); bookIter != bookNames.end(); ++bookIter)
			addBook(*bookIter);

		if (!bookNames.empty())
		{
			UIWidget * const bookToSelect = safe_cast<UIWidget *>(m_books->GetChild(lastBookSelected.c_str()));

			if (bookToSelect && restoreSelectedBook)
				m_books->SetSelection(bookToSelect);
			else
				m_books->SetSelectionIndex(0);
		}
	}
}

// ----------------------------------------------------------------------

void SwgCuiCollections::updatePages(bool const restoreScrollLocation)
{
	m_bookNameText->SetLocalText(Unicode::emptyString);

	UIPoint const scrollLocation = m_pages->GetScrollLocation();

	m_pages->Clear();
	m_pages->Pack();

	PlayerObject const * const playerObject = getPlayerObject();

	if (playerObject && m_books->GetLastSelectedChild())
	{
		std::string const & bookName = m_books->GetLastSelectedChild()->GetName();
		Unicode::String localizedBookName = StringId("collection_n", bookName).localize();

		if (bookName == std::string("badge_book"))
		{
			// for badges, include the count of how many badges have been completed,
			// regardless if the player has chosen to show completed collections or not
			char buffer[16];
			snprintf(buffer, sizeof(buffer)-1, " (%d)", playerObject->getCompletedCollectionSlotCountInBook(bookName));
			buffer[sizeof(buffer)-1] = '\0';

			localizedBookName += Unicode::narrowToWide(buffer);
		}

		m_bookNameText->SetLocalText(localizedBookName);

		std::vector<std::string> pageNames;

		playerObject->getActiveCollectionPages(bookName, &pageNames);

		for (std::vector<std::string>::const_iterator pageIter = pageNames.begin(); pageIter != pageNames.end(); ++pageIter)
			if (m_showCompleted->IsChecked() || !playerObject->hasCompletedCollectionPage(*pageIter))
				addPage(bookName, *pageIter);

		m_pages->Pack();

		if (restoreScrollLocation && !pageNames.empty())
			m_pages->ScrollToPoint(scrollLocation);
	}

	ms_lastBookSelectedIndex = m_books->GetLastSelectedIndex();
}

// ----------------------------------------------------------------------

void SwgCuiCollections::addBook(std::string const & bookName)
{
	char const * const baseBookIconPath = "book.";

	CollectionsDataTable::CollectionInfoBook const * const bookInfo = CollectionsDataTable::getBookByName(bookName);

	if (!bookInfo || bookInfo->hidden)
		return;

	UIImageStyle * style = 0;
	
	if (!bookInfo->icon.empty())
		style = getImageStyle(bookInfo->icon, false);
	else
		style = getImageStyle(baseBookIconPath + bookName, false);

	if (style)
	{
		UIImage * const image = new UIImage();
		image->SetStyle(style);
		image->SetName(bookName);
		image->SetTooltip(StringId("collection_n", bookName).localize());

		m_books->AddChild(image);
		m_books->Link();
	}
}

// ----------------------------------------------------------------------

void SwgCuiCollections::addPage(std::string const & bookName, std::string const & pageName)
{
	CollectionsDataTable::CollectionInfoPage const * const pageInfo = CollectionsDataTable::getPageByName(pageName);

	if (!pageInfo || pageInfo->hidden)
		return;

	PlayerObject const * const playerObject = getPlayerObject();

	if (playerObject)
	{
		std::vector<std::string> collectionNames;

		playerObject->getActiveCollections(pageName, &collectionNames);

		if (!collectionNames.empty())
		{
			UIComposite * const page = NON_NULL(safe_cast<UIComposite *>(m_samplePage->DuplicateObject()));

			if (!page)
				return;

			m_pages->AddChild(page);

			page->Link();
			page->SetVisible(true);
			page->SetName(pageName);

			UIText * const pageNameText = safe_cast<UIText *>(page->GetChild("pageNameText"));
			if (pageNameText)
				pageNameText->SetLocalText(StringId("collection_n", pageName).localize());

			for (std::vector<std::string>::const_iterator collectionIter = collectionNames.begin(); collectionIter != collectionNames.end(); ++collectionIter)
				if (m_showCompleted->IsChecked() || !playerObject->hasCompletedCollection(*collectionIter))
					addCollection(bookName, pageName, *collectionIter, page);
		}
	}
}
// ----------------------------------------------------------------------

void SwgCuiCollections::addCollection(std::string const & bookName, std::string const & pageName, std::string const & collectionName, UIComposite * const page)
{
	CollectionsDataTable::CollectionInfoCollection const * const collectionInfo = CollectionsDataTable::getCollectionByName(collectionName);

	if (!collectionInfo || collectionInfo->hidden)
		return;

	PlayerObject const * const playerObject = getPlayerObject();

	if (playerObject)
	{
		UIPage * const collection = NON_NULL(safe_cast<UIPage *>(m_sampleCollection->DuplicateObject()));

		if (!collection)
			return;

		page->AddChild(collection);

		collection->Link();
		collection->SetVisible(true);
		collection->SetName(collectionName);

		UIText * const collectionNameText = safe_cast<UIText *>(collection->GetChild("collectionNameText"));
		if (collectionNameText)
		{
			Unicode::String text = StringId("collection_n", collectionName).localize();
			Unicode::String textTooltip;

			if (collectionInfo->trackServerFirst)
			{
				bool const showServerFirst = CuiPreferences::getCollectionShowServerFirst();

				if (showServerFirst)
					text += Unicode::narrowToWide(" \\#32CD32"); // lime green

				if (collectionInfo->serverFirstClaimTime <= 0)
				{	
					if (showServerFirst)
						text += StringId("ui_collection", "server_first_undiscovered").localize();

					textTooltip += StringId("ui_collection", "server_first_undiscovered_tooltip").localize();
				}
				else
				{
					if (showServerFirst)
						text += collectionInfo->serverFirstClaimantName;

					ProsePackage pp;
					pp.stringId = StringId("ui_collection", "server_first_discovered_tooltip");
					pp.target.str = collectionInfo->serverFirstClaimantName;
					pp.other.str = Unicode::narrowToWide(CalendarTime::convertEpochToTimeStringLocal(collectionInfo->serverFirstClaimTime));
					ProsePackageManagerClient::appendTranslation(pp, textTooltip);
				}
			}

			collectionNameText->SetLocalText(text);

			if (!textTooltip.empty())
				collectionNameText->SetTooltip(textTooltip);
		}

		UIImage * const reward = safe_cast<UIImage *>(collection->GetChild("reward"));
		bool const isMyCollection = getPlayerObject() == Game::getPlayerObject();

		if (reward)
		{
			if (collectionInfo->noReward || !isMyCollection)
				reward->SetVisible(false);
			else
			{
				char const * const baseRewardPath = "reward.";

				UIImageStyle * style = 0;

				bool const completed = playerObject->hasCompletedCollection(collectionName);

				if (!collectionInfo->icon.empty())
					style = getImageStyle(collectionInfo->icon, !completed);
				else
					style = getImageStyle(baseRewardPath + collectionName, !completed);

				if (style)
				{
					reward->SetStyle(style);
					reward->SetName(collectionName);
					reward->SetTooltip(StringId("collection_reward", collectionName).localize());
				}
			}
		}

		std::vector<CollectionsDataTable::CollectionInfoSlot const *> const & slots = CollectionsDataTable::getSlotsInCollection(collectionName);

		if (slots.empty())
			return;

		UIVolumePage * const icons = safe_cast<UIVolumePage *>(collection->GetChild("icons"));

		if (icons)
		{
			icons->Clear();

			std::string const baseCollectionPath = "slot." + bookName + "." + pageName + "." + collectionName + ".";

			for (std::vector<CollectionsDataTable::CollectionInfoSlot const *>::const_iterator slotIter = slots.begin(); slotIter != slots.end(); ++slotIter)
			{
				CollectionsDataTable::CollectionInfoSlot const * const slotInfo = *slotIter;

				if (!slotInfo || slotInfo->hidden)
					continue;

				std::string const & slotName = slotInfo->name;

				bool const completed = playerObject->hasCompletedCollectionSlot(**slotIter);

				int showIfNotYetEarned;
				
				if (collectionInfo->showIfNotYetEarned)
					showIfNotYetEarned = collectionInfo->showIfNotYetEarned;
				else
					showIfNotYetEarned = slotInfo->showIfNotYetEarned;

				std::string imageStylePath;

				if (slotInfo && !slotInfo->icon.empty())
					imageStylePath = slotInfo->icon;
				else
					imageStylePath = baseCollectionPath + slotName;

				UIImageStyle * style = 0;

				if (completed)
					style = getImageStyle(imageStylePath, false);
				else if (showIfNotYetEarned == CollectionsDataTable::SE_gray)
					style = getImageStyle(imageStylePath, true);
				else if (showIfNotYetEarned == CollectionsDataTable::SE_unknown)
					style = m_imageStyleUnknown;
				else // CollectionsDataTable::SE_none
					continue;

				if (style)
				{
					UIPage * const iconPage = NON_NULL(safe_cast<UIPage *>(m_sampleIcon->DuplicateObject()));
					UIImage * const icon = NON_NULL(safe_cast<UIImage *>(iconPage->GetChild("icon")));

					icon->SetStyle(style);
					iconPage->SetName(slotName);

					Unicode::String tooltip;
					
					if (showIfNotYetEarned == CollectionsDataTable::SE_gray || completed)
						tooltip = StringId("collection_n", slotName).localize();
					else
						tooltip = StringId("ui", "tooltip_collection_unknown").localize();

					UIPage * const bar = NON_NULL(safe_cast<UIPage *>(iconPage->GetChild("bar")));

					if (slotInfo->counterTypeSlot && isMyCollection && !completed && slotInfo->maxSlotValue)
					{
						UIPage * const value = NON_NULL(safe_cast<UIPage *>(bar->GetChild("value")));

						unsigned long collectionSlotValue;
						playerObject->getCollectionSlotValue(*slotInfo, collectionSlotValue);

						UIScalar const completionSizeX = std::max<UIScalar>(value->GetMaximumSize().x * collectionSlotValue / slotInfo->maxSlotValue, 1);

						value->SetSize(UISize(collectionSlotValue ? completionSizeX : 0, value->GetMaximumSize().y));

						FormattedString<64> fs;

						tooltip.append(Unicode::narrowToWide(fs.sprintf(" %d/%d", collectionSlotValue, slotInfo->maxSlotValue)));
					}
					else
						bar->SetVisible(false);

					if (isMyCollection && completed)
					{
						tooltip.append(Unicode::narrowToWide("\n\n"));
						tooltip.append(StringId("collection_d", slotName).localize());
					}

					iconPage->SetTooltip(tooltip);
					iconPage->SetVisible(true);

					icons->AddChild(iconPage);
				}
			}

			icons->Pack();

			UISize scollExtent;
			icons->GetScrollExtent(scollExtent);
			icons->SetSize(scollExtent);
		}
	}
}

// ----------------------------------------------------------------------

PlayerObject const * SwgCuiCollections::getPlayerObject() const
{
	if (m_target->getPointer())
		return m_target->getPointer()->getPlayerObject();

	return 0;
}

// ----------------------------------------------------------------------

void SwgCuiCollections::onCollectionsChanged(PlayerObject const & player)
{
	if (&player == getPlayerObject())
		updateBooks(true);
}

// ----------------------------------------------------------------------

void SwgCuiCollections::onCollectionServerFirstChanged(std::string const & collectionServerFirstListVersion)
{
	UNREF(collectionServerFirstListVersion);
	updateBooks(true);
}

// ----------------------------------------------------------------------

void SwgCuiCollections::onCollectionShowServerFirstOptionChanged(bool const & enabled)
{
	UNREF(enabled);
	updateBooks(true);
}

// ----------------------------------------------------------------------

UIImageStyle * SwgCuiCollections::getImageStyle(std::string const & path, bool gray)
{
	char const * const grayShaderSuffix = "_gray";
	char const * const grayShaderPrefix = "ui_grayscale:";
	char const * const grayShaderFF = "gray_fixed";

	UIImageStyle * style = 0;

	bool	isFixedFunction = true;

	if(Graphics::getShaderCapability() >= ShaderCapability (1, 1))
	{
		isFixedFunction = false;
	}

	if (gray)
		style = safe_cast<UIImageStyle *>(m_imageStyleNamespace->GetObjectFromPath((path + grayShaderSuffix).c_str()));
	else
		style = safe_cast<UIImageStyle *>(m_imageStyleNamespace->GetObjectFromPath(path.c_str()));

	if (!style && gray)
	{
		if(isFixedFunction)
		{
			style = safe_cast<UIImageStyle *>(m_imageStyleNamespace->GetObjectFromPath(grayShaderFF));
		}
		else
		{
			style = safe_cast<UIImageStyle *>(m_imageStyleNamespace->GetObjectFromPath(path.c_str()));
		}

		if (style && !isFixedFunction)
		{
			UIImageStyle * imageStyleGray = NON_NULL(safe_cast<UIImageStyle *>(style->DuplicateObject()));
			bool successfullyCreated = false;

			if (imageStyleGray && style->GetParent())
			{
				UIString value;

				if (imageStyleGray->GetProperty(UIImageFragment::PropertyName::SourceResource, value) &&
					imageStyleGray->SetProperty(UIImageFragment::PropertyName::SourceResource, Unicode::narrowToWide(grayShaderPrefix) + value))
				{
					imageStyleGray->SetName(imageStyleGray->GetName() + grayShaderSuffix);
					style->GetParent()->AddChild(imageStyleGray);
					successfullyCreated = true;
				}
			}

			if (!successfullyCreated && imageStyleGray)
			{
				imageStyleGray->Destroy();
				imageStyleGray = 0;
			}

			style = imageStyleGray;
		}
	}

	if (!style)
	{
		//DEBUG_WARNING(gray, ("Could not create gray ImageStyle for '%s'.", path.c_str()));
		//DEBUG_WARNING(!gray, ("Could not find ImageStyle '%s'.", path.c_str()));
		style = gray ? m_imageStyleMissingGray : m_imageStyleMissing;
	}

	return style;
}

// ======================================================================
