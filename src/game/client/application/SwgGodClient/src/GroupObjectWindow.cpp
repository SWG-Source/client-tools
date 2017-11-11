// ======================================================================
//
// GroupObjectWindow.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgGodClient/FirstSwgGodClient.h"

#include "GroupObjectWindow.h"
#include "GroupObjectWindow.moc"

#include "BrushData.h"
#include "BrushListView.h"

#include <qlistview.h>

// ======================================================================

GroupObjectWindow::GroupObjectWindow(QWidget *theParent, const char *theName)
: BaseGroupObjectWindow (theParent, theName),
  MessageDispatch::Receiver ()
{
	connectToMessage (BrushData::Messages::BRUSHES_CHANGED);

	IGNORE_RETURN (connect (m_brushesList,  SIGNAL (selectionChanged ()), this, SLOT (onBrushListSelectionChanged ())));

}

// ======================================================================

GroupObjectWindow::~GroupObjectWindow()
{
}

//-----------------------------------------------------------------

/**
 * Todo move this to the brush list view, since it's a custom widget now and can handle this itself (connectToMessage too)
 */
void GroupObjectWindow::receiveMessage(const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message)
{
	UNREF(source);

	const BrushData & bd = BrushData::getInstance();

	if (message.isType (BrushData::Messages::BRUSHES_CHANGED))
	{
		m_brushesList->clear();
		const size_t numBrushes = bd.getNumBrushes();
		
		for (size_t i = 0; i < numBrushes; ++i)
		{
			const BrushData::BrushStruct & brush = bd.getBrush((numBrushes - i) - 1);
			QListViewItem* item = new QListViewItem(m_brushesList, brush.name.c_str());
			item->setDragEnabled(true);
		}
	}
}

//-----------------------------------------------------------------

void GroupObjectWindow::onBrushListSelectionChanged()
{
	const QListViewItem * item = m_brushesList->selectedItem();

	if(item == 0)
	{
		brushSelectionChanged("");
		return;
	}

	std::string name = item->text(0).latin1();

	BrushData::getInstance().setSelectedBrush(name);
}

// ======================================================================
