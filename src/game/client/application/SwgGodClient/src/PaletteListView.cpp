// ======================================================================
//
// PaletteListView.cpp
// copyright(c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgGodClient/FirstSwgGodClient.h"

#include "PaletteListView.h"
#include "PaletteListView.moc"

#include "ActionHack.h"
#include "ActionsEdit.h"
#include "ActionsTool.h"
#include "GodClientData.h"
#include "IconLoader.h"
#include "ObjectTemplateData.h"

#include <qdragobject.h>
#include <qpopupmenu.h>

//-----------------------------------------------------------------

PaletteListView::PaletteListViewItem::PaletteListViewItem(QListView* parent)
: QListViewItem(parent),
  m_isPalette(false)
{
}

//-----------------------------------------------------------------

PaletteListView::PaletteListViewItem::PaletteListViewItem(PaletteListViewItem* parent)
: QListViewItem(parent),
  m_isPalette(false)
{
}

//-----------------------------------------------------------------

PaletteListView::PaletteListViewItem::PaletteListViewItem(QListView* parent, PaletteListViewItem* after)
: QListViewItem(parent, after),
  m_isPalette(false)
{
}

//-----------------------------------------------------------------

PaletteListView::PaletteListViewItem::PaletteListViewItem(PaletteListViewItem* parent, PaletteListViewItem* after)
: QListViewItem(parent, after),
  m_isPalette(false)
{
}

//-----------------------------------------------------------------

PaletteListView::PaletteListViewItem::PaletteListViewItem(QListView* parent, const QString& label1, const QString& label2, const QString& label3, const QString& label4, const QString& label5, const QString& label6, const QString& label7, const QString& label8)
: QListViewItem(parent, label1, label2, label3, label4, label5, label6, label7, label8),
  m_isPalette(false)
{
}

//-----------------------------------------------------------------

PaletteListView::PaletteListViewItem::PaletteListViewItem(PaletteListViewItem* parent, const QString& label1, const QString& label2, const QString& label3, const QString& label4, const QString& label5, const QString& label6, const QString& label7, const QString& label8)
: QListViewItem(parent, label1, label2, label3, label4, label5, label6, label7, label8),
  m_isPalette(false)
{
}

//-----------------------------------------------------------------

PaletteListView::PaletteListViewItem::PaletteListViewItem(QListView* parent, PaletteListViewItem* after, const QString& label1, const QString& label2, const QString& label3, const QString& label4, const QString& label5, const QString& label6, const QString& label7, const QString& label8)
: QListViewItem(parent, after, label1, label2, label3, label4, label5, label6, label7, label8),
  m_isPalette(false)
{
}

//-----------------------------------------------------------------

PaletteListView::PaletteListViewItem::PaletteListViewItem(PaletteListViewItem* parent, PaletteListViewItem* after, const QString& label1, const QString& label2, const QString& label3, const QString& label4, const QString& label5, const QString& label6, const QString& label7, const QString& label8)
: QListViewItem(parent, after, label1, label2, label3, label4, label5, label6, label7, label8),
  m_isPalette(false)
{
}

//-----------------------------------------------------------------

PaletteListView::PaletteListView(QWidget* theParent, const char* theName)
: QListView(theParent, theName),
  MessageDispatch::Receiver()
{
	IGNORE_RETURN(QListView::addColumn("Palette"));
	QListView::setResizeMode(QListView::AllColumns);
	QListView::setRootIsDecorated(true);

	IGNORE_RETURN(connect(this, SIGNAL(contextMenuRequested(QListViewItem*, const QPoint&, int)), this, SLOT(onContextMenuRequested(QListViewItem*, const QPoint&, int))));
	IGNORE_RETURN(connect(this, SIGNAL(selectionChanged()), this, SLOT(onSelectionChanged())));

	connectToMessage (GodClientData::Messages::PALETTES_CHANGED);
}

//-----------------------------------------------------------------

QDragObject* PaletteListView::dragObject()
{
	return new QTextDrag(ActionsEdit::DragMessages::PALETTE_DRAGGED_INTO_GAME, this, "menu");
}

//-----------------------------------------------------------------

void PaletteListView::onSelectionChanged() const
{
	const PaletteListViewItem* item = dynamic_cast<PaletteListViewItem*>(selectedItem());

	if(item == 0)
	{
		return;
	}
	
	if(!item->isPalette())
		return;

	std::string name = item->text(0).latin1();
	GodClientData::getInstance().setSelectedPalette(name);
}

//-----------------------------------------------------------------

void PaletteListView::onRefreshList() const
{
}

//-----------------------------------------------------------------

void PaletteListView::onContextMenuRequested(QListViewItem* item, const QPoint& p, int)
{
	QPopupMenu* const m_pop = new QPopupMenu(this, "menu");
	ActionsTool* at         = &ActionsTool::getInstance();

	if(item == 0)
	{
		IGNORE_RETURN(at->m_createPalette->addTo(m_pop));
	}
	else
	{
		IGNORE_RETURN(at->m_deletePalette->addTo(m_pop));
	}

	m_pop->popup(p);
} //lint !e818 item "could" be const, but Qt allows us to change it

//-----------------------------------------------------------------

void PaletteListView::receiveMessage(const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message)
{
	UNREF(source);

	GodClientData& gcd = GodClientData::getInstance();
	const QPixmap pix       = IL_PIXMAP(hi16_mime_document);
	const QPixmap folderPix = IL_PIXMAP(hi16_filesys_folder_green);

	if (message.isType (GodClientData::Messages::PALETTES_CHANGED))
	{
		clear();
		const size_t numPalettes = gcd.getNumPalettes();
		
		for (size_t i = 0; i < numPalettes; ++i)
		{
			std::string name;
			GodClientData::Palette_t palette = gcd.getPalette(static_cast<int>(i), name);
			PaletteListViewItem* paletteItem = new PaletteListViewItem(this, name.c_str());
			paletteItem->setDragEnabled(false);
			paletteItem->setDropEnabled(true);
			paletteItem->setPixmap(0, folderPix);
			paletteItem->setIsPalette(true);
			for(GodClientData::Palette_t::iterator it = palette.begin(); it != palette.end(); ++it)
			{
				PaletteListViewItem* item = new PaletteListViewItem(paletteItem, (*it).c_str());
				item->setDragEnabled(true);
				item->setPixmap(0, pix);
				item->setIsPalette(false);
			}
		}
	}
}

//-----------------------------------------------------------------

void PaletteListView::dragEnterEvent (QDragEnterEvent * evt)
{
	QString text;
	if (QTextDrag::decode (evt, text))
	{
		if (text == ObjectTemplateData::DragMessages::SERVER_TEMPLATE_DRAGGED)
			evt->accept ();
	}
}

//-----------------------------------------------------------------

void PaletteListView::dropEvent(QDropEvent* evt)
{
	QString text;

	if (QTextDrag::decode(evt, text))
	{
		if (text == ObjectTemplateData::DragMessages::SERVER_TEMPLATE_DRAGGED)
		{
			GodClientData::getInstance().addTemplateToPalette(ActionsEdit::getInstance().getSelectedServerTemplate());
			evt->accept();
		}
	}
}

// ======================================================================
