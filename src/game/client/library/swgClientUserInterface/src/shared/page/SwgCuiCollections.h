// ======================================================================
//
// SwgCuiCollections.h
// copyright (c) 2007 Sony Online Entertainment LLC
//
// ======================================================================

#ifndef INCLUDED_SwgCuiCollections_H
#define INCLUDED_SwgCuiCollections_H

// ======================================================================

#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"

// ----------------------------------------------------------------------

namespace MessageDispatch
{
	class Callback;
}

// ----------------------------------------------------------------------

class CreatureObject;
class PlayerObject;
class UICheckbox;
class UIComposite;
class UIImageStyle;
class UINamespace;
class UIPage;
class UIText;
class UIVolumePage;

template <typename T> class Watcher;

// ----------------------------------------------------------------------

class SwgCuiCollections : public CuiMediator, public UIEventCallback
{
public:
	SwgCuiCollections(UIPage & page);
	~SwgCuiCollections();

	virtual void OnVolumePageSelectionChanged(UIWidget * context);
	virtual void OnGenericSelectionChanged(UIWidget * context);

	void setTarget(CreatureObject * const target);

	void onCollectionsChanged(PlayerObject const & player);
	void onCollectionServerFirstChanged(std::string const & collectionServerFirstListVersion);
	void onCollectionShowServerFirstOptionChanged(bool const & enabled);

protected:
	virtual void performActivate();
	virtual void performDeactivate();
	virtual void update(float deltaTimeSecs);

	void updateBooks(bool const restoreSelectedBook);
	void updatePages(bool const restoreScrollLocation);

	void addBook(std::string const & bookName);
	void addPage(std::string const & bookName, std::string const & pageName);
	void addCollection(std::string const & bookName, std::string const & pageName, std::string const & collectionName, UIComposite * const page);

private:
	PlayerObject const * getPlayerObject() const;
	UIImageStyle * getImageStyle(std::string const & path, bool gray);

private: // disabled
	SwgCuiCollections();
	SwgCuiCollections(SwgCuiCollections const & rhs);
	SwgCuiCollections &operator=(SwgCuiCollections const & rhs);

private:
	MessageDispatch::Callback * m_callback;
	UIVolumePage * m_books;
	UIComposite * m_pages;
	UIPage * m_sampleCollection;
	UIComposite * m_samplePage;
	UIPage * m_sampleIcon;
	UIText * m_bookNameText;
	UIText * m_captionText;
	UICheckbox * m_showCompleted;
	UIImageStyle * m_imageStyleMissing;
	UIImageStyle * m_imageStyleMissingGray;
	UIImageStyle * m_imageStyleUnknown;
	UINamespace * m_imageStyleNamespace;
	Watcher<CreatureObject> * m_target;
};

// ======================================================================

#endif
