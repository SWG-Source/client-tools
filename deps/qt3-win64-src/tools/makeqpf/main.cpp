/**********************************************************************
** Copyright (C) 1999-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of Qt/Embedded.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include <qapplication.h>
#include <qtextstream.h>
#include <qscrollview.h>
#include <qfile.h>
#include <qfont.h>
#include <qvbox.h>
#include <qlabel.h>
#include <qstringlist.h>
#include <qlistview.h>
#include <qmainwindow.h>
#include <qmessagebox.h>
#include <qpainter.h>
#ifdef Q_WS_QWS
#include <qmemorymanager_qws.h>
#endif

#include <stdlib.h>


class FontViewItem : public QListViewItem {
    QString family;
    int pointSize;
    int weight;
    bool italic;
    QFont font;

public:
    FontViewItem(const QString& f, int pt, int w, bool ital, QListView* parent) :
	QListViewItem(parent),
	family(f), pointSize(pt), weight(w), italic(ital)
    {
    }

    void renderAndSave()
    {
	font = QFont(family,pointSize,weight,italic);
#ifdef Q_WS_QWS
	memorymanager->savePrerenderedFont((QMemoryManager::FontID)font.handle());
#endif
	setHeight(QFontMetrics(font).lineSpacing());
	repaint();
    }

    void render(int from, int to)
    {
	font = QFont(family,pointSize,weight,italic);
#ifdef Q_WS_QWS
	for (int i=from; i<=to; i++) {
	    if ( memorymanager->inFont((QMemoryManager::FontID)font.handle(),QChar(i))) {
		memorymanager->lockGlyph((QMemoryManager::FontID)font.handle(),QChar(i));
		memorymanager->unlockGlyph((QMemoryManager::FontID)font.handle(),QChar(i));
	    }
	}
#endif
    }

    void save()
    {
	font = QFont(family,pointSize,weight,italic);
#ifdef Q_WS_QWS
	memorymanager->savePrerenderedFont((QMemoryManager::FontID)font.handle(),FALSE);
#endif
	setHeight(QFontMetrics(font).lineSpacing());
	repaint();
    }

    void paintCell( QPainter *p, const QColorGroup & cg,
                            int column, int width, int alignment )
    {
	p->setFont(font);
	QListViewItem::paintCell(p,cg,column,width,alignment);
    }

    int width( const QFontMetrics&,
                       const QListView* lv, int column) const
    {
	QFontMetrics fm(font);
	return QListViewItem::width(fm,lv,column);
    }

    QString text(int col) const
    {
	switch (col) {
	    case 0:
		return family;
	    case 1:
		return QString::number(pointSize)+"pt";
	    case 2:
		if ( weight < QFont::Normal ) {
		    return "Light";
		} else if ( weight >= QFont::Black ) {
		    return "Black";
		} else if ( weight >= QFont::Bold ) {
		    return "Bold";
		} else if ( weight >= QFont::DemiBold ) {
		    return "DemiBold";
		} else {
		    return "Normal";
		}
	    case 3:
		return italic ? "Italic" : "Roman";
	}
	return QString::null;
    }
};

extern QString qws_topdir();

class MakeQPF : public QMainWindow
{
    Q_OBJECT
    QListView* view;
public:
    MakeQPF()
    {
	view = new QListView(this);
	view->addColumn("Family");
	view->addColumn("Size");
	view->addColumn("Weight");
	view->addColumn("Style");
	setCentralWidget(view);
	QString fontdir = qws_topdir() + "/lib/fonts";
	readFontDir(fontdir);

	connect(view,SIGNAL(selectionChanged(QListViewItem*)),
	    this,SLOT(renderAndSave(QListViewItem*)));
    }

    void readFontDir(const QString& fntd)
    {
	QString fontdir = fntd + "/fontdir";
	QFile fd(fontdir);
	if ( !fd.open(IO_ReadOnly) ) {
	    QMessageBox::warning(this, "Read Error",
		"<p>Cannot read "+fontdir);
	    return;
	}
	while ( !fd.atEnd() ) {
	    QString line;
	    fd.readLine(line,9999);
	    if ( line[0] != '#' ) {
		QStringList attr = QStringList::split(" ",line);
		if ( attr.count() >= 7 ) {
		    QString family = attr[0];
		    int weight = QString(attr[4]).toInt();
		    bool italic = QString(attr[3]) == "y";
		    QStringList sizes = attr[5];
		    if ( sizes[0] == "0" ) {
			if ( attr[7].isNull() )
			    sizes = QStringList::split(',',attr[6]);
			else
			    sizes = QStringList::split(',',attr[7]);
		    }
		    for (QStringList::Iterator it = sizes.begin(); it != sizes.end(); ++it) {
			int pointSize = (*it).toInt()/10;
			if ( pointSize )
			    new FontViewItem(
				family, pointSize, weight, italic, view);
		    }
		}
	    }
	}
    }

    void renderAndSaveAll()
    {
	QListViewItem* c = view->firstChild();
	while ( c ) {
	    renderAndSave(c);
	    qApp->processEvents();
	    c = c->nextSibling();
	}
    }

    void renderAndSave(const QString& family)
    {
	QListViewItem* c = view->firstChild();
	while ( c ) {
	    if ( c->text(0).lower() == family.lower() )
		renderAndSave(c);
	    c = c->nextSibling();
	}
    }

    void render(const QString& family, int from, int to)
    {
	QListViewItem* c = view->firstChild();
	while ( c ) {
	    if ( c->text(0).lower() == family.lower() )
		((FontViewItem*)c)->render(from,to);
	    c = c->nextSibling();
	}
    }

    void save(const QString& family)
    {
	QListViewItem* c = view->firstChild();
	while ( c ) {
	    if ( c->text(0).lower() == family.lower() )
		((FontViewItem*)c)->save();
	    c = c->nextSibling();
	}
    }

private slots:
    void renderAndSave(QListViewItem* i)
    {
	((FontViewItem*)i)->renderAndSave();
    }
};

static void usage()
{
    qWarning("Usage: makeqpf [-A] [-f spec-file] [font ...]");
    qWarning("");
    qWarning("   Saves QPF font files by rendering and saving fonts.");
    qWarning("");
    qWarning("         -A = Render and save all fonts in fontdir");
    qWarning("         -f = File of lines:");
    qWarning("                fontname character-ranges");
    qWarning("              eg.");
    qWarning("                smoothtimes 0-ff,20a0-20af");
    qWarning("       font = Font to render and save");
}

int main(int argc, char** argv)
{
    QApplication app(argc, argv, QApplication::GuiServer);
    MakeQPF m;
    if ( argc > 1 ) {
	argv++;
	QString specfile;
	while ( *argv && argv[0][0] == '-' ) {
	    if (argv[0][1] == 'A' )
		m.renderAndSaveAll();
	    else if (argv[0][1] == 'f' )
		specfile = *++argv;
	    else {
		usage();
		specfile=QString::null;
		*argv=0;
		break;
	    }
	    argv++;
	}
	if ( !specfile.isNull() ) {
	    QFile f(specfile);
	    if ( f.open(IO_ReadOnly) ) {
		QTextStream s(&f);
		while (!s.atEnd()) {
		    QString family,ranges;
		    s >> family >> ranges;
		    QStringList r = QStringList::split(',',ranges);
		    for (QStringList::Iterator it=r.begin(); it!=r.end(); ++it) {
			QString rng = *it;
			int dash = rng.find('-');
			int from,to;
			if ( dash==0 ) {
			    from=0;
			    to=rng.mid(1).toInt(0,16);
			} else if ( dash==(int)rng.length()-1 ) {
			    from=rng.left(dash).toInt(0,16);
			    to=65535;
			} else if ( dash<0 ) {
			    from=to=rng.toInt(0,16);
			} else {
			    from=rng.left(dash).toInt(0,16);
			    to=rng.mid(dash+1).toInt(0,16);
			}
			m.render(family,from,to);
		    }
		    m.save(family);
		}
	    } else {
		qWarning("Cannot open %s",specfile.latin1());
	    }
	} else {
	    while (*argv)
		m.renderAndSave(*argv++);
	}
    } else {
	// Interactive
	app.setMainWidget(&m);
	m.show();
	return app.exec();
    }
}

#include "main.moc"
