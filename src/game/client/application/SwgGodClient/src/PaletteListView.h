// ======================================================================
//
// PaletteListView.h
// copyright(c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_PaletteListView_H
#define INCLUDED_PaletteListView_H

// ======================================================================

#include "sharedMessageDispatch/Receiver.h" // in the precompiled header, but included by QT-generated code that doesn't use our precompiled headers

#include <qlistview.h>

//-----------------------------------------------------------------

class QDragObject;

//-----------------------------------------------------------------

/**
*/

class PaletteListView : public QListView, public MessageDispatch::Receiver
{
	Q_OBJECT; //lint !e1516 !e19 !e1924 !e1762 various deficiencies in the Qt macro

private:
	class PaletteListViewItem : public QListViewItem
	{
		public:
			void setIsPalette(bool isPalette) {m_isPalette = isPalette;}
			bool isPalette() const            {return m_isPalette;}

			explicit PaletteListViewItem(QListView* parent);
			explicit PaletteListViewItem(PaletteListViewItem * parent);
			PaletteListViewItem(QListView* parent, PaletteListViewItem * after);
			PaletteListViewItem(PaletteListViewItem * parent, PaletteListViewItem * after);
			PaletteListViewItem(QListView* parent,
			                    const QString& label1,
			                    const QString& label2 = QString::null,
			                    const QString& label3 = QString::null,
			                    const QString& label4 = QString::null,
			                    const QString& label5 = QString::null,
			                    const QString& label6 = QString::null,
			                    const QString& label7 = QString::null,
			                    const QString& label8 = QString::null);
			PaletteListViewItem(PaletteListViewItem* parent,
			                    const QString& label1,
			                    const QString& label2 = QString::null,
			                    const QString& label3 = QString::null,
			                    const QString& label4 = QString::null,
			                    const QString& label5 = QString::null,
			                    const QString& label6 = QString::null,
			                    const QString& label7 = QString::null,
			                    const QString& label8 = QString::null);
			PaletteListViewItem(QListView* parent,
			                    PaletteListViewItem* after,
			                    const QString& label1,
			                    const QString& label2 = QString::null,
			                    const QString& label3 = QString::null,
			                    const QString& label4 = QString::null,
			                    const QString& label5 = QString::null,
			                    const QString& label6 = QString::null,
			                    const QString& label7 = QString::null,
			                    const QString& label8 = QString::null);
			PaletteListViewItem(PaletteListViewItem* parent,
			                    PaletteListViewItem * after,
			                    const QString& label1,
			                    const QString& label2 = QString::null,
			                    const QString& label3 = QString::null,
			                    const QString& label4 = QString::null,
			                    const QString& label5 = QString::null,
			                    const QString& label6 = QString::null,
			                    const QString& label7 = QString::null,
			                    const QString& label8 = QString::null);

		private:
			//disabled
			PaletteListViewItem();

		private:
			bool m_isPalette;
	};

public:
	PaletteListView(QWidget* parent, const char* name);
	void receiveMessage(const MessageDispatch::Emitter& source, const MessageDispatch::MessageBase& message);

public slots:
	void onRefreshList() const;

protected:
	QDragObject* dragObject();
	void dragEnterEvent    (QDragEnterEvent* event);
	void dropEvent         (QDropEvent* evt);

protected slots:
	void onContextMenuRequested(QListViewItem* item, const QPoint& pos, int col);

private slots:
	void onSelectionChanged() const;

private:
	//disabled
	PaletteListView();
	PaletteListView(const PaletteListView& rhs);
	PaletteListView& operator=(const PaletteListView& rhs);
};

// ======================================================================

#endif
