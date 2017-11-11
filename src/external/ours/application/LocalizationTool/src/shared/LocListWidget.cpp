// ======================================================================
//
// LocListWidget.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "StdAfx.h"
#include "LocListWidget.h"
#include "LocListWidget.moc"

#include "LocEditingWidget.h"
#include "LocalizationData.h"
#include "LocalizationToolXpms.h"
#include "LocalizedStringPair.h" 
#include "StringUtils.h"
#include <QListView.h>
#include <cassert>
#include <qpixmap.h>

//----------------------------------------------------------------------

namespace
{
	//----------------------------------------------------------------------	
	/**
	* Filter to allow keystroke commands in list view
	*/
	class MyKeyFilter : public QObject
	{
	public:
		MyKeyFilter (LocListWidget & listWidget) : QObject (), m_listWidget (listWidget) {}
	private:
		inline bool eventFilter( QObject *o, QEvent *e )
		{
			if ( e->type() == QEvent::KeyPress) {  // key press
				QKeyEvent *k = (QKeyEvent*)e;
				
				if (k->key () == Qt::Key_Delete)
				{
					m_listWidget.deleteSelection ();
					return true;                        // eat event
				}
			}
			return o->eventFilter( o, e );    // standard event processing
		}
		
		LocListWidget & m_listWidget;
		
	private:	// Disabled copy constructor and operator=
		MyKeyFilter( const MyKeyFilter & );
		MyKeyFilter &operator=( const MyKeyFilter & );
	};
	
	//-----------------------------------------------------------------
	
	const QColor s_upToDateEditingColor           ( 55, 155, 255);
	const QColor s_upToDateEditingHighlightColor  (  0,  55, 155);
	const QColor s_outOfDateEditingColor          (255, 155, 255);
	const QColor s_outOfDateEditingHighlightColor (135, 35,  155);
	const QColor s_outOfDateColor                 (255, 200, 155);
	const QColor s_outOfDateHighlightColor        (135, 35,   50);
	
	//----------------------------------------------------------------------
	
	class MyListViewItem : public QListViewItem
	{
	public:
		
		enum Columns
		{
			ID_COL        = 0,
			NAME_COL      = 1,
			FIX_COL       = 2,
			MOD_SRC_COL   = 3,
			SRC_COL       = 4,
			MOD_TRANS_COL = 5,
			TRANS_COL     = 6,
			NUM_COLS      = 7
		};
		
		//----------------------------------------------------------------------

		MyListViewItem (QListView * listView, const LocalizedStringPair & sp, const QPixmap & outPixmap, const QPixmap & modPixmap) :
			QListViewItem (listView,
				QString::number (sp.getId ()),
				sp.getName ().c_str (),
				"",
				"",
				StringUtils::convertUnicode (sp.getSourceString ().getString ()),
				"",
				sp.getHasTranslatedString () ? StringUtils::convertUnicode (sp.getTranslatedString ().getString ()) : ""),
				m_outOfDate (sp.isOutOfDate ()),
				m_entryId (static_cast<int>(sp.getId()))
		{
			
			m_modified [0] = sp.isModified (0);
			m_modified [1] = sp.isModified (1);
			
			if (m_outOfDate)
				setPixmap (FIX_COL, outPixmap);
			
			if (sp.isModified (0))
				setPixmap (MOD_SRC_COL, modPixmap);
			
			if (sp.isModified (1))
				setPixmap (MOD_TRANS_COL, modPixmap);
			
		}
		
		//----------------------------------------------------------------------

		inline void paintCell ( QPainter * p, const QColorGroup & cg, int column, int width, int align ) 
		{
			
			if (m_outOfDate == false)
			{
				if (m_entryId != LocalizationData::getData ().getCurrentEditId ())
				{
					QListViewItem::paintCell (p, cg, column, width, align);
				}
				else
				{
					QColorGroup g (cg);
					g.setColor (QColorGroup::Base, s_upToDateEditingColor);
					g.setColor (QColorGroup::Highlight, s_upToDateEditingHighlightColor);
					QListViewItem::paintCell (p, g, column, width, align);	
				}
			}
			else
			{
				QColorGroup g (cg);
				
				if (m_entryId != LocalizationData::getData ().getCurrentEditId ())
				{
					g.setColor (QColorGroup::Base, s_outOfDateColor);
					g.setColor (QColorGroup::Highlight, s_outOfDateHighlightColor);
				}
				else
				{
					g.setColor (QColorGroup::Base, s_outOfDateEditingColor);
					g.setColor (QColorGroup::Highlight, s_outOfDateEditingHighlightColor);
				}
				QListViewItem::paintCell (p, g, column, width, align);
			}
		}
		
		//----------------------------------------------------------------------

		inline QString key (int column, bool ascending ) const 
		{
			if (column == MOD_SRC_COL)
			{
				return (m_modified [0]) ? "1" : "";
			}
			
			if (column == MOD_TRANS_COL)
			{
				return (m_modified [1]) ? "1" : "";
			}
			
			if (column != FIX_COL)
			{
				if (column != ID_COL)
					return QListViewItem::key (column, ascending);
				
				return text (column).rightJustify (10, '0');
			}
			
			if (m_outOfDate)
				return "1";
			else
				return "";
		}
		
		//----------------------------------------------------------------------

		inline int getEntryId () const { return m_entryId; }
			
	private:
		
		bool m_outOfDate;
		int  m_entryId;
		
		bool m_modified [2];
	};
}

// ======================================================================

LocListWidget::LocListWidget (LocEditingWidget * editor, QWidget * parent) :
ListWidget              (parent, "ListWidget"),
m_editingWidget         (editor),
m_outOfDatePixmap       (0),
m_modifiedPixmap        (0),
m_ignoreDataChange      (false)
{
	assert (m_editingWidget);

	m_outOfDatePixmap = new QPixmap (LocalizationToolXpms::out_of_date);
	m_modifiedPixmap  = new QPixmap (LocalizationToolXpms::modified);

	updateData ();

	ListStrings->setSorting (1, true);

	LocalizationData::getData ().addListener (this);

	MyKeyFilter * filter = new MyKeyFilter (*this);

	ListStrings->installEventFilter (filter);
}

//-----------------------------------------------------------------

LocListWidget::~LocListWidget ()
{
	LocalizationData::getData ().removeListener (this);
}

//-----------------------------------------------------------------


void LocListWidget::resizeEvent (QResizeEvent * evt)
{
	ListWidget::resizeEvent (evt);

	fitColumnWidths ();

	ListStrings->triggerUpdate ();
}

//-----------------------------------------------------------------

void LocListWidget::fitColumnWidths ()
{
	int rightMargin = ListStrings->visibleWidth ();
	
	for (size_t i = 0; i < MyListViewItem::NUM_COLS; ++i)
		rightMargin -= ListStrings->columnWidth (i);

	const int srcWidth = ListStrings->columnWidth (MyListViewItem::SRC_COL);
	rightMargin += srcWidth;

	// all columns are present
	if (ListStrings->columns () == MyListViewItem::NUM_COLS)
	{ 
		const int transWidth = ListStrings->columnWidth (MyListViewItem::TRANS_COL);
		rightMargin += transWidth;

		int transMargin;

		if (transWidth)
		{
			transMargin = rightMargin / 2;
		}
		else
		{
			transMargin = (rightMargin * transWidth / srcWidth) / 2;
		}

		rightMargin -= transMargin;

		if (transMargin > 0)
			ListStrings->setColumnWidth (MyListViewItem::TRANS_COL, transMargin);
	}

	if (rightMargin > 0)
		ListStrings->setColumnWidth (MyListViewItem::SRC_COL, rightMargin);
}

//-----------------------------------------------------------------

void LocListWidget::updateData ()
{
	const MyListViewItem * prevSelectedItem = static_cast<const MyListViewItem *>(ListStrings->selectedItem ());

	const int prevSelected = prevSelectedItem ? prevSelectedItem->getEntryId () : -1;

	const QPoint prevScrollPt (ListStrings->viewportToContents (QPoint (0,0)));

	ListStrings->clear ();

	ListStrings->setColumnAlignment (MyListViewItem::FIX_COL,     Qt::AlignHCenter);
	ListStrings->setColumnAlignment (MyListViewItem::MOD_SRC_COL, Qt::AlignRight);
	ListStrings->setColumnAlignment (MyListViewItem::SRC_COL,     Qt::SingleLine);
	ListStrings->setColumnWidthMode (MyListViewItem::SRC_COL,     QListView::Manual);

	//-- don't use the "Translated String" column if nothing is being translated
	if (LocalizationData::getData ().getHasTranslatedData ())
	{
		if (ListStrings->columns () < MyListViewItem::NUM_COLS)
		{
			ListStrings->addColumn ("TMod");
			ListStrings->setColumnAlignment (MyListViewItem::MOD_TRANS_COL, Qt::SingleLine);
			ListStrings->setColumnWidthMode (MyListViewItem::MOD_TRANS_COL, QListView::Manual);
			ListStrings->setColumnAlignment (MyListViewItem::MOD_TRANS_COL, Qt::AlignRight);

			ListStrings->addColumn ("Translated String");
			ListStrings->setColumnAlignment (MyListViewItem::TRANS_COL, Qt::SingleLine);
			ListStrings->setColumnWidthMode (MyListViewItem::TRANS_COL, QListView::Manual);

			fitColumnWidths ();
		}
	}
	else if (ListStrings->columns () == MyListViewItem::NUM_COLS)
	{
		ListStrings->removeColumn (MyListViewItem::TRANS_COL);
		ListStrings->removeColumn (MyListViewItem::MOD_TRANS_COL);
		
		//-- resize the src col
		fitColumnWidths ();
	}

	LocalizationData::const_iterator end;
	LocalizationData::const_iterator iter = LocalizationData::getData ().getConstIterators (end);

	for ( ; iter != end; ++iter)
	{
		const LocalizedStringPair & sp = (*iter).second;

		MyListViewItem * item = new MyListViewItem (ListStrings, sp, *m_outOfDatePixmap, *m_modifiedPixmap);

		ListStrings->insertItem (item);

		if (item->getEntryId () == prevSelected)
			ListStrings->setSelected (item, true);
	} //lint -e322

	ListStrings->setContentsPos (prevScrollPt.x(), prevScrollPt.y());
}
	
//-----------------------------------------------------------------

void LocListWidget::startEditingItem (QListViewItem * item)
{

	const int id = static_cast<MyListViewItem *>(item)->getEntryId ();

	const LocalizedStringPair * strPair = LocalizationData::getData ().getEntryById (id);

	assert (strPair);

	m_editingWidget->startEditingString (*strPair);

}

//-----------------------------------------------------------------

void LocListWidget::dataChanged ()
{
	if (!m_ignoreDataChange)
	{
		updateData ();
		ListStrings->triggerUpdate ();
	}
}

//-----------------------------------------------------------------

void LocListWidget::currentEditingChanged ()
{
	//todo: this might need to be optimized to update fewer rows
	ListStrings->triggerUpdate ();
}

//-----------------------------------------------------------------

void LocListWidget::readOnlyChanged ()
{
	m_editingWidget->readOnlyChanged ();
}

//-----------------------------------------------------------------

void LocListWidget::deleteSelection ()
{
	MyListViewItem * item = static_cast<MyListViewItem *> (ListStrings->firstChild ());

	if (!item)
		return;

	typedef std::vector<int> IdVector;
	IdVector idv;

	while (item)
	{
		if (ListStrings->isSelected (item))
		{
			idv.push_back (item->getEntryId ());
		}

		item = static_cast<MyListViewItem *> (item->nextSibling ());
	}

	m_ignoreDataChange = true;

	for (IdVector::const_iterator it = idv.begin (); it != idv.end (); ++it)
	{
		const int id = *it;
		if (id == LocalizationData::getData ().getCurrentEditId ())
			m_editingWidget->deleteEntry ();
		else
		{
			LocalizationData::getData ().deleteEntryById (id);
			//@todo: maintain selected index
		}
	}

	m_ignoreDataChange = false;

	dataChanged ();
}

//-----------------------------------------------------------------

const QFont LocListWidget::getListFont () const
{
	return ListStrings->font ();
}

//-----------------------------------------------------------------

void LocListWidget::setListFont (const QFont & font)
{
	ListStrings->setFont (font);
}

// ======================================================================

