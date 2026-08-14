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

#include "main.h"
#include <qapplication.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qlistview.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qmessagebox.h>
#include <qpopupmenu.h>
#include <qmenubar.h>
#include <qtoolbutton.h>
#include <qstatusbar.h>
#include <qsplitter.h>
#include <qregexp.h>
#if defined(Q_OS_WIN32)
#include <direct.h>
#else
#include <unistd.h>
#endif
#include <stdlib.h>


/* XPM */
static const char * const logo_xpm[] = {
/* width height ncolors chars_per_pixel */
"50 50 17 1",
/* colors */
"  c #000000",
". c #495808",
"X c #2A3304",
"o c #242B04",
"O c #030401",
"+ c #9EC011",
"@ c #93B310",
"# c #748E0C",
"$ c #A2C511",
"% c #8BA90E",
"& c #99BA10",
"* c #060701",
"= c #181D02",
"- c #212804",
"; c #61770A",
": c #0B0D01",
"/ c None",
/* pixels */
"/$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$/",
"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$+++$$$$$$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$@;.o=::=o.;@$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$+#X*         **X#+$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$#oO*         O  **o#+$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$&.* OO              O*.&$$$$$$$$$$$$$",
"$$$$$$$$$$$$@XOO            * OO    X&$$$$$$$$$$$$",
"$$$$$$$$$$$@XO OO  O  **:::OOO OOO   X@$$$$$$$$$$$",
"$$$$$$$$$$&XO      O-;#@++@%.oOO      X&$$$$$$$$$$",
"$$$$$$$$$$.O  :  *-#+$$$$$$$$+#- : O O*.$$$$$$$$$$",
"$$$$$$$$$#*OO  O*.&$$$$$$$$$$$$+.OOOO **#$$$$$$$$$",
"$$$$$$$$+-OO O *;$$$$$$$$$$$&$$$$;*     o+$$$$$$$$",
"$$$$$$$$#O*  O .+$$$$$$$$$$@X;$$$+.O    *#$$$$$$$$",
"$$$$$$$$X*    -&$$$$$$$$$$@- :;$$$&-    OX$$$$$$$$",
"$$$$$$$@*O  *O#$$$$$$$$$$@oOO**;$$$#    O*%$$$$$$$",
"$$$$$$$;     -+$$$$$$$$$@o O OO ;+$$-O   *;$$$$$$$",
"$$$$$$$.     ;$$$$$$$$$@-OO OO  X&$$;O    .$$$$$$$",
"$$$$$$$o    *#$$$$$$$$@o  O O O-@$$$#O   *o$$$$$$$",
"$$$$$$+=    *@$$$$$$$@o* OO   -@$$$$&:    =$$$$$$$",
"$$$$$$+:    :+$$$$$$@-      *-@$$$$$$:    :+$$$$$$",
"$$$$$$+:    :+$$$$$@o* O    *-@$$$$$$:    :+$$$$$$",
"$$$$$$$=    :@$$$$@o*OOO      -@$$$$@:    =+$$$$$$",
"$$$$$$$-    O%$$$@o* O O    O O-@$$$#*   OX$$$$$$$",
"$$$$$$$. O *O;$$&o O*O* *O      -@$$;    O.$$$$$$$",
"$$$$$$$;*   Oo+$$;O*O:OO--      Oo@+=    *;$$$$$$$",
"$$$$$$$@*  O O#$$$;*OOOo@@-O     Oo;O*  **@$$$$$$$",
"$$$$$$$$X* OOO-+$$$;O o@$$@-    O O     OX$$$$$$$$",
"$$$$$$$$#*  * O.$$$$;X@$$$$@-O O        O#$$$$$$$$",
"$$$$$$$$+oO O OO.+$$+&$$$$$$@-O         o+$$$$$$$$",
"$$$$$$$$$#*    **.&$$$$$$$$$$@o      OO:#$$$$$$$$$",
"$$$$$$$$$+.   O* O-#+$$$$$$$$+;O    OOO:@$$$$$$$$$",
"$$$$$$$$$$&X  *O    -;#@++@#;=O    O    -@$$$$$$$$",
"$$$$$$$$$$$&X O     O*O::::O      OO    Oo@$$$$$$$",
"$$$$$$$$$$$$@XOO                  OO    O*X+$$$$$$",
"$$$$$$$$$$$$$&.*       **  O      ::    *:#$$$$$$$",
"$$$$$$$$$$$$$$$#o*OO       O    Oo#@-OOO=#$$$$$$$$",
"$$$$$$$$$$$$$$$$+#X:* *     O**X#+$$@-*:#$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$%;.o=::=o.#@$$$$$$@X#$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$$+++$$$$$$$$$$$+$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$",
"/$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$/",
};


#define FIXED_LAYOUT

static const char*back_xpm[]={
"16 16 5 1",
"# c #000000",
"a c #ffffff",
"c c #808080",
"b c #c0c0c0",
". c None",
"................",
".......#........",
"......##........",
".....#a#........",
"....#aa########.",
"...#aabaaaaaaa#.",
"..#aabbbbbbbbb#.",
"...#abbbbbbbbb#.",
"...c#ab########.",
"....c#a#ccccccc.",
".....c##c.......",
"......c#c.......",
".......cc.......",
"........c.......",
"................",
"......................"};

static const char*forward_xpm[]={
"16 16 5 1",
"# c #000000",
"a c #ffffff",
"c c #808080",
"b c #c0c0c0",
". c None",
"................",
"................",
".........#......",
".........##.....",
".........#a#....",
"..########aa#...",
"..#aaaaaaabaa#..",
"..#bbbbbbbbbaa#.",
"..#bbbbbbbbba#..",
"..########ba#c..",
"..ccccccc#a#c...",
"........c##c....",
"........c#c.....",
"........cc......",
"........c.......",
"................",
"................"};

class ChoiceItem : public QCheckListItem {
public:
    QString id;
    ChoiceItem(const QString& i, QListViewItem* parent) :
	QCheckListItem(parent,
	    i.mid(6), // strip "QT_NO_" as we reverse the logic
	    CheckBox),
	id(i)
    {
	setOpen(TRUE);
	label = text(0);
	avl = TRUE;
    }

    // We reverse the logic
    void setDefined(bool y) { setOn(!y); }
    bool isDefined() const { return !isOn(); }

    void setAvailable(bool y)
    {
	if ( avl != y ) {
	    avl = y;
	    repaint();
	}
    }
    bool isAvailable() const { return avl; }

    virtual void setOn(bool y)
    {
	QCheckListItem::setOn(y);
	setOpen(y);
/*
	for (QListViewItem* i=firstChild(); i; i = i->nextSibling() ) {
	    ChoiceItem* ci = (ChoiceItem*)i; // all are ChoiceItem
	    if ( ci->isSelectable() != y ) {
		ci->setSelectable(y);
		listView()->repaintItem(ci);
	    }
	}
*/
    }

    void paintBranches( QPainter * p, const QColorGroup & cg,
                            int w, int y, int h)
    {
	QListViewItem::paintBranches(p,cg,w,y,h);
    }

    void paintCell( QPainter * p, const QColorGroup & cg,
                               int column, int width, int align )
    {
	if ( !isSelectable() || !isAvailable() ) {
	    QColorGroup c = cg;
	    c.setColor(QColorGroup::Text, lightGray);
	    QCheckListItem::paintCell(p,c,column,width,align);
	} else {
	    QCheckListItem::paintCell(p,cg,column,width,align);
	}
    }

    void setInfo(const QString& l, const QString& d)
    {
	label = l;
	doc = d;
	setText(0,label);
    }

    QString label;

    QString info() const
    {
	return "<h2>"+label+"</h2>"+doc;
    }

private:
    QString doc;
    bool avl;
};

Main::Main()
{
    setIcon( (const char**)logo_xpm );
#ifdef FIXED_LAYOUT
    QHBox* horizontal = new QHBox(this);
#else
    QSplitter* horizontal = new QSplitter(this);
#endif

    lv = new QListView(horizontal);
    lv->setSorting(-1);
    lv->setRootIsDecorated(TRUE);
    lv->addColumn("ID");

    info = new Info(horizontal);
    info->setBackgroundMode(PaletteBase);
    info->setMargin(10);
    info->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
    info->setAlignment(AlignTop);

    connect(info, SIGNAL(idClicked(const QString&)),
	    this, SLOT(selectId(const QString&)));

#ifdef FIXED_LAYOUT
    horizontal->setStretchFactor(info,2);
#endif

    connect(lv, SIGNAL(pressed(QListViewItem*)),
	    this, SLOT(updateAvailability(QListViewItem*)));
    connect(lv, SIGNAL(selectionChanged(QListViewItem*)),
	    this, SLOT(showInfo(QListViewItem*)));

    setCentralWidget(horizontal);

    QToolBar* tb = new QToolBar( this, "browser controls" );
    tb->setLabel( "Browser Controls" );
    (void)new QToolButton( QPixmap(back_xpm), "Back", QString::null,
                           info, SLOT(back()), tb, "back" );
    (void)new QToolButton( QPixmap(forward_xpm), "Forward", QString::null,
                           info, SLOT(forward()), tb, "forward" );

    QPopupMenu* file = new QPopupMenu( menuBar() );
    file->insertItem( "&Open",  this, SLOT(open()), CTRL+Key_O );
    file->insertItem( "&Save", this, SLOT(save()), CTRL+Key_S );
    file->insertSeparator();
    file->insertItem( "&Test all", this, SLOT(testAll()), CTRL+Key_T );
    file->insertSeparator();
    file->insertItem( "E&xit",  qApp, SLOT(quit()), CTRL+Key_Q );

    menuBar()->insertItem( "&File",file );

    menuBar()->insertSeparator();

    QPopupMenu *help = new QPopupMenu( menuBar() );
    help->insertItem( "&About", this, SLOT(about()) );
    help->insertItem( "About &Qt", this, SLOT(aboutQt()) );

    menuBar()->insertItem( "&Help", help );

    statusBar()->message( "Ready" );
}

void Main::open()
{
}

void Main::save()
{
}

void Main::testAll()
{
    QString qtdir = getenv("QTDIR");
    chdir((qtdir+"/src").ascii());
    QString c;
    for (QStringList::ConstIterator it = choices.begin(); it != choices.end(); ++it)
    {
	c += "Feature: ";
	c += *it;
	c += "\n";

	c += "Section: ";
	c += section[*it];
	c += "\n";

	c += "Requires: ";
	c += dependencies[*it].join(" ");
	c += "\n";

	c += "Name: ";
	c += label[*it];
	c += "\n";

	c += "SeeAlso: ???\n";

	c += "\n";
    }
    QFile f("features.txt");
    f.open(IO_WriteOnly);
    f.writeBlock(c.ascii(),c.length());
    f.close();
    // system("./feature_size_calculator");

#if 0
    system("mv ../include/qconfig.h ../include/qconfig.h-orig");
    for (QStringList::ConstIterator it = choices.begin(); it != choices.end(); ++it)
    {
	QString choice = *it;
	QFile f("../include/qconfig.h");
	f.open(IO_WriteOnly);
	QCString s = "#define ";
	s += choice.latin1();
	s += "\n";
	f.writeBlock(s,s.length());
	f.close();
	int err = system("make");
	if ( err != 0 )
	    break;
    }
    system("mv ../include/qconfig.h-orig ../include/qconfig.h");
#endif
}


// ##### should be in QMap?
template <class K, class D>
QValueList<K> keys(QMap<K,D> map)
{
    QValueList<K> result;
    for (Q_TYPENAME QMap<K,D>::ConstIterator it = map.begin(); it!=map.end(); ++it)
	result.append(it.key());
    return result;
}

void Main::loadFeatures(const QString& filename)
{
    QFile file(filename);
    if ( !file.open(IO_ReadOnly) ) {
	QMessageBox::warning(this,"Warning",
			     "Cannot open file " + filename);
	return;
    }
    QTextStream s(&file);
    QRegExp qt_no_xxx("QT_NO_[A-Z_0-9]*");
    QStringList sections;

#if 1
    QString line = s.readLine();
    QString feature, lab, sec;
    QStringList deps, seealso;
    QMap<QString,QStringList> sectioncontents;
    while (!s.atEnd()) {
	if ( line.length() <= 1 ) {
	    if ( !feature.isEmpty() ) {
		dependencies[feature] = deps;
		for (QStringList::ConstIterator it = deps.begin(); it!=deps.end(); ++it)
		    rdependencies[*it].append(feature);
		label[feature] = lab;
		links[feature] = seealso;
		section[feature] = sec;
		sectioncontents[sec].append(feature);
		choices.append(feature);
	    } else {
		qDebug("Unparsed text");
	    }

	    feature = lab = sec = QString::null;
	    deps.clear(); seealso.clear();
	    line = s.readLine();
	    continue;
	}

	QString nextline = s.readLine();
	while ( nextline[0] == ' ' ) {
	    line += nextline;
	    nextline = s.readLine();
	}

	int colon = line.find(':');
	if ( colon < 0 ) {
	    qDebug("Cannot parse: %s",line.ascii());
	} else {
	    QString tag = line.left(colon);
	    QString value = line.mid(colon+1).stripWhiteSpace();
	    if ( tag == "Feature" )
		feature = value;
	    else if ( tag == "Requires" )
		deps = QStringList::split(QChar(' '),value);
	    else if ( tag == "Name" )
		lab = value;
	    else if ( tag == "Section" )
		sec = value;
	    else if ( tag == "SeeAlso" )
		seealso = QStringList::split(QChar(' '),value);
	}

	line = nextline;
    }
    sections = keys(sectioncontents);

#else
    QString sec;
    QString lab;
    QString doc;
    bool on = FALSE;
    bool docmode = FALSE;
    QStringList deps;

    do {
	QString line = s.readLine();
	line.replace(QRegExp("# *define"),"#define");

	QStringList token = QStringList::split(QChar(' '),line);
	if ( on ) {
	    if ( docmode ) {
		if ( token[0] == "*/" )
		    docmode = FALSE;
		else if ( lab.isEmpty() )
		    lab = line.stripWhiteSpace();
		else
		    doc += line.simplifyWhiteSpace() + "\n";
	    } else if ( token[0] == "//#define" || token[0] == "#define" ) {
		dependencies[token[1]] = deps;
		for (QStringList::ConstIterator it = deps.begin(); it!=deps.end(); ++it)
		    rdependencies[*it].append(token[1]);
		section[token[1]] = sec;
		documentation[token[1]] = doc;
		label[token[1]] = lab;
		choices.append(token[1]);
		doc = "";
		lab = "";
	    } else if ( token[0] == "/*!" ) {
		docmode = TRUE;
	    } else if ( token[0] == "//" ) {
		token.remove(token.begin());
		sec = token.join(" ");
		sections.append(sec);
	    } else if ( token[0] == "#if" ) {
		Q_ASSERT(deps.isEmpty());
		for (int i=1; i<(int)token.count(); i++) {
		    if ( token[i][0] == 'd' ) {
			int index;
			int len;
			index = qt_no_xxx.match(token[i],0,&len);
			if ( index >= 0 ) {
			    QString d = token[i].mid(index,len);
			    deps.append(d);
			}
		    }
		}
	    } else if ( token[0] == "#endif" ) {
		deps.clear();
	    } else if ( token[0].isEmpty() ) {
	    } else {
		qDebug("Cannot parse: %s",token.join(" ").ascii());
	    }
	} else if ( token[0] == "#include" ) {
	    on = TRUE;
	}
    } while (!s.atEnd());
#endif

    lv->clear();
    sections.sort();
    // ##### QListView default sort order is reverse of insertion order
    for (QStringList::Iterator se = sections.fromLast(); se != sections.end(); --se) {
	sectionitem[*se] = new QListViewItem(lv,*se);
    }
    for (QStringList::Iterator ch = choices.begin(); ch != choices.end(); ++ch) {
	createItem(*ch);
    }

#ifdef FIXED_LAYOUT
    lv->setFixedWidth(lv->sizeHint().width());
#endif
}

void Main::createItem(const QString& ch)
{
    if ( !item[ch] ) {
	QStringList deps = dependencies[ch];
	QString sec = section[ch];
	QListViewItem* parent = 0;
	for (QStringList::Iterator dp = deps.begin(); dp != deps.end(); ++dp) {
	    QString dsec = section[*dp];
	    if ( dsec.isEmpty() )
		qDebug("No section for %s",(*dp).latin1());
	    if ( !parent && dsec == sec ) {
		createItem(*dp);
		parent = item[*dp];
	    }
	}
	if ( !parent )
	    parent = sectionitem[section[ch]];
	ChoiceItem* ci = new ChoiceItem(ch,parent);
	item[ch] = ci;
	if ( !label[ch].isEmpty() )
	    ci->setInfo(label[ch],documentation[ch]);
    }
}

void Main::loadConfig(const QString& filename)
{
    QFile file(filename);
    if ( !file.open(IO_ReadOnly) ) {
	QMessageBox::warning(this,"Warning",
			     "Cannot open file " + filename);
	return;
    }
    QTextStream s(&file);
    QRegExp qt_no_xxx("QT_NO_[A-Z_0-9]*");

    for (QStringList::Iterator ch = choices.begin(); ch != choices.end(); ++ch) {
	item[*ch]->setDefined(FALSE);
    }
    do {
	QString line = s.readLine();
	QStringList token = QStringList::split(QChar(' '),line);
	if ( token[0] == "#define" ) {
	    ChoiceItem* i = item[token[1]];
	    if ( i )
		i->setDefined(TRUE);
	    else
		qDebug("The item %s is not used by qfeatures.h", token[1].latin1());
	}
    } while (!s.atEnd());
}

void Main::updateAvailability(QListViewItem* i)
{
    if ( !i || !i->parent() ) {
        // section. do nothing for now
    } else {
        ChoiceItem* choice = (ChoiceItem*)i;
	QStringList deps = rdependencies[choice->id];
	for (QStringList::ConstIterator it = deps.begin();
		it != deps.end(); ++it)
	{
	    ChoiceItem* d = item[*it];
	    QStringList ddeps = dependencies[d->id];
	    bool av = TRUE;
	    for (QStringList::ConstIterator dit = ddeps.begin();
		    av && dit != ddeps.end(); ++dit)
	    {
		ChoiceItem* dd = item[*dit];
		if ( dd ) {
		    if ( dd->isDefined() || !dd->isAvailable() )
			av = FALSE;
		} else
		    qDebug("%s ???",(*dit).latin1());
	    }
	    if ( d->isAvailable() != av ) {
		d->setAvailable(av);
		updateAvailability(d);
	    }
	}
qDebug("%s: %d",choice->id.latin1(),choice->isAvailable());
    }
}

void Main::showInfo(QListViewItem* i)
{
    if ( !i )
	return;
    if ( !i->parent() ) {
	// section. do nothing for now
    } else {
	ChoiceItem* choice = (ChoiceItem*)i;
	QString i = choice->info();
	QStringList deps = dependencies[choice->id];
	if ( !deps.isEmpty() ) {
	    i += "<h3>Requires:</h3><ul>";
	    for (QStringList::ConstIterator it = deps.begin();
		    it != deps.end(); ++it)
	    {
		ChoiceItem* d = item[*it];
		if ( d ) {
		    bool got = d->isAvailable() && !d->isDefined();
		    i += "<li>";
		    if ( !got ) i += "<font color=red>";
		    i += "<a href=id://"+d->id+">"+d->label+"</a>";
		    if ( !got ) i += "</font>";
		}
	    }
	    i += "</ul>";
	}
	QStringList rdeps = rdependencies[choice->id];
	if ( !rdeps.isEmpty() ) {
	    i += "<h3>Required for:</h3><ul>";
	    for (QStringList::ConstIterator it = rdeps.begin();
		    it != rdeps.end(); ++it)
	    {
		ChoiceItem* d = item[*it];
		if ( d )
		    i += "<li><a href=id://"+d->id+">"+d->label+"</a>";
	    }
	    i += "</ul>";
	}
	info->setText(i);
    }
}

void Main::selectId(const QString& id)
{
    QListViewItem* it = item[id];
    if ( it ) {
	lv->setSelected(it,TRUE);
	lv->ensureItemVisible(it);
    }
}

Info::Info( QWidget* parent, const char* name ) : 
    QTextBrowser(parent, name)
{
}

void Info::setSource(const QString& name)
{
    if ( name.left(5) == "id://" ) {
	emit idClicked(name.mid(5,name.length()-6)); // skip trailing "/" too
    } else {
	QTextBrowser::setSource(name);
    }
}

void Main::about()
{
    QMessageBox::about(this, "About qconfig",
	"<p><b><font size=+2>Qt/Embedded build configuration</font></b></p>"
	"<p></p>"
	"<p>Version 1.0</p>"
	"<p>Copyright (C) 2001-2007 Trolltech ASA. All rights reserved.</p>"
	"<p></p>"
	"<p>This program is licensed to you under the terms of the GNU General "
	"Public License Version 2 as published by the Free Software Foundation. This "
	"gives you legal permission to copy, distribute and/or modify this software "
	"under certain conditions. For details, see the file 'LICENSE.GPL' that came with "
	"this software distribution. If you did not get the file, send email to "
	"info@trolltech.com.</p>\n\n<p>The program is provided AS IS with NO WARRANTY "
	"OF ANY KIND, INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS "
	"FOR A PARTICULAR PURPOSE.</p>"
    );
}

void Main::aboutQt()
{
    QMessageBox::aboutQt( this, tr("qconfig") );
}

int main(int argc, char** argv)
{
    QApplication app(argc,argv);
    Main m;
    QString qtdir = getenv("QTDIR");
    QString qfeatures = qtdir + "/src/tools/qfeatures.txt";
    //QString qfeatures = qtdir + "/include/qfeatures.h";
    QString qconfig = qtdir + "/include/qconfig.h";
    for (int i=1; i<argc; i++) {
	QString arg = argv[i];
	if ( arg == "-f" && i+i<argc ) {
	    qfeatures = argv[++i];
	} else {
	    qconfig = argv[i];
	}
    }
    m.loadFeatures(qfeatures);
    m.loadConfig(qconfig);
    m.resize(m.sizeHint()+QSize(500,300));
    app.setMainWidget(&m);
    m.show();
    return app.exec();
}
