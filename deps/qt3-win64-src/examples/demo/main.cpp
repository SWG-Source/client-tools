/****************************************************************************
** $Id: main.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "frame.h"
#include "graph.h"
#include "display.h"
#include "icons.h"

#include "textdrawing/textedit.h"
#include "textdrawing/helpwindow.h"
#include "dnd/dnd.h"
#include "i18n/i18n.h"

#include <qmodules.h>

#if defined(QT_MODULE_OPENGL)
#include "opengl/glworkspace.h"
#include "opengl/gllandscapeviewer.h"
#include "opengl/glinfotext.h"
#endif

#if defined(QT_MODULE_CANVAS)
#include "qasteroids/toplevel.h"
#endif

#if defined(QT_MODULE_TABLE)
#include "widgets/widgetsbase.h"
#else
#include "widgets/widgetsbase_pro.h"
#endif

#include <stdlib.h>

#include <qapplication.h>
#include <qimage.h>

#include <qtabwidget.h>
#include <qfont.h>
#include <qworkspace.h>
#include <qwidgetstack.h>

#if defined(QT_MODULE_SQL)
#include <qsqldatabase.h>
#include "sql/sqlex.h"
#endif

#if defined(Q_OS_MACX)
#include <stdlib.h>
#include <qdir.h>
#endif

#include "categoryinterface.h"

static void qdemo_set_caption( CategoryInterface *c, int i )
{
    QWidget *w = qApp->mainWidget();
    if ( !w )
	return;
    QString title = Frame::tr( "Qt Demo Collection" );
    title += " - " + c->categoryName( i - c->categoryOffset() );
    w->setCaption( title );
}

class WidgetCategory : public CategoryInterface
{
public:
    WidgetCategory( QWidgetStack *s ) : CategoryInterface( s ), created( FALSE ) {}

    QString name() const { return "Widgets"; }
    QIconSet icon() const { return QPixmap( widgeticon ); }
    int numCategories() const { return 2; }
    QString categoryName( int i ) const {
	switch ( i ) {
	case 0:
	    return Frame::tr( "Widgets" );
	    break;
	case 1:
	    return Frame::tr( "Drag and Drop" );
	    break;
	}
	return QString::null;
    }
    QIconSet categoryIcon( int ) const { return QIconSet(); }
    void setCurrentCategory( int i ) {
	create();
	stack->raiseWidget( i );
	qdemo_set_caption( this, i );
    }
    void create() {
	if ( created )
	    return;
	created = TRUE;
	stack->addWidget( new WidgetsBase( stack ), categoryOffset() + 0 );
	stack->addWidget( new DnDDemo( stack ), categoryOffset() + 1 );
    }

    int categoryOffset() const { return 0; }

private:
    bool created;

};

#if defined(QT_MODULE_SQL)
class DatabaseCategory : public CategoryInterface
{
public:
    DatabaseCategory( QWidgetStack *s ) : CategoryInterface( s ), created( FALSE ) {}

    QString name() const { return "Database"; }
    QIconSet icon() const { return QPixmap( dbicon ); }
    int numCategories() const { return 1; }
    QString categoryName( int i ) const {
	switch ( i ) {
	case 0:
	    return Frame::tr( "SQL Explorer" );
	    break;
	}
	return QString::null;
    }
    QIconSet categoryIcon( int ) const { return QIconSet(); }
    void setCurrentCategory( int i ) {
	create();
	stack->raiseWidget( i );
	qdemo_set_caption( this, i );
    }
    void create() {
	if ( created )
	    return;
	created = TRUE;
	stack->addWidget( new SqlEx( stack ), categoryOffset() + 0 );
    }

    int categoryOffset() const { return 10; }

private:
    bool created;

};
#endif

#if defined(QT_MODULE_CANVAS)
class CanvasCategory : public CategoryInterface
{
public:
    CanvasCategory( QWidgetStack *s ) : CategoryInterface( s ), created( FALSE ) {}

    QString name() const { return "2D Graphics"; }
    QIconSet icon() const { return QPixmap( twodicon ); }
    int numCategories() const { return 2; }
    QString categoryName( int i ) const {
	switch ( i ) {
	case 0:
	    return Frame::tr( "Graph Drawing" );
	    break;
	case 1:
	    return Frame::tr( "Display" );
	    break;
	}
	return QString::null;
    }
    QIconSet categoryIcon( int ) const { return QIconSet(); }
    void setCurrentCategory( int i ) {
	create();
	stack->raiseWidget( i );
	qdemo_set_caption( this, i );
    }
    void create() {
	if ( created )
	    return;
	created = TRUE;
	stack->addWidget( new GraphWidget( stack ), categoryOffset() + 0 );
	stack->addWidget( new DisplayWidget( stack ), categoryOffset() + 1 );
    }

    int categoryOffset() const { return 100; }

private:
    bool created;

};
#endif

#if defined(QT_MODULE_OPENGL)
class OpenGLCategory : public CategoryInterface
{
public:
    OpenGLCategory( QWidgetStack *s ) : CategoryInterface( s ), created( FALSE ) {}

    QString name() const { return "3D Graphics"; }
    QIconSet icon() const { return QPixmap( threedicon ); }
    int numCategories() const { return 3; }
    QString categoryName( int i ) const {
	switch ( i ) {
	case 0:
	    return Frame::tr( "3D Demo" );
	    break;
	case 1:
	    return Frame::tr( "Fractal landscape" );
	    break;
	case 2:
	    return Frame::tr( "OpenGL info" );
	    break;
	}
	return QString::null;
    }
    QIconSet categoryIcon( int ) const { return QIconSet(); }
    void setCurrentCategory( int i ) {
	create();
	stack->raiseWidget( i );
	qdemo_set_caption( this, i );
    }
    void create() {
	if ( created )
	    return;
	created = TRUE;
	stack->addWidget( new GLWorkspace( stack ), categoryOffset() + 0 );
	stack->addWidget( new GLLandscapeViewer( stack ), categoryOffset() + 1 );
	stack->addWidget( new GLInfoText( stack ), categoryOffset() + 2 );
    }
    int categoryOffset() const { return 1000; }

private:
    bool created;

};
#endif

class TextCategory : public CategoryInterface
{
public:
    TextCategory( QWidgetStack *s ) : CategoryInterface( s ), created( FALSE ) {}

    QString name() const { return "Text Drawing/Editing"; }
    QIconSet icon() const { return QPixmap( texticon ); }
    int numCategories() const { return 2; }
    QString categoryName( int i ) const {
	switch ( i ) {
	case 0:
	    return Frame::tr( "Richtext Editor" );
	    break;
	case 1:
	    return Frame::tr( "Help Browser" );
	    break;
	}
	return QString::null;
    }
    QIconSet categoryIcon( int ) const { return QIconSet(); }
    void setCurrentCategory( int i ) {
	create();
	stack->raiseWidget( i );
	qdemo_set_caption( this, i );
    }
    void create() {
	if ( created )
	    return;
	created = TRUE;
	TextEdit *te = new TextEdit( stack );
	te->load( "textdrawing/example.html" );
	stack->addWidget( te, categoryOffset() + 0 );
	QString home = QDir( "../../doc/html/index.html" ).absPath();
	HelpWindow *w = new HelpWindow( home, ".", stack, "helpviewer" );
	stack->addWidget( w, categoryOffset() + 1 );
    }
    int categoryOffset() const { return 10000; }

private:
    bool created;

};

class I18NCategory : public CategoryInterface
{
public:
    I18NCategory( QWidgetStack *s ) : CategoryInterface( s ), created( FALSE ) {}

    QString name() const { return "Internationalization"; }
    QIconSet icon() const { return QPixmap( internicon ); }
    int numCategories() const { return 1; }
    QString categoryName( int i ) const {
	switch ( i ) {
	case 0:
	    return Frame::tr( "Internationalization" );
	    break;
	}
	return QString::null;
    }
    QIconSet categoryIcon( int ) const { return QIconSet(); }
    void setCurrentCategory( int i ) {
	create();
	stack->raiseWidget( i );
	qdemo_set_caption( this, i );
    }
    void create() {
	if ( created )
	    return;
	created = TRUE;
	stack->addWidget( new I18nDemo( stack ), categoryOffset() + 0 );
    }
    int categoryOffset() const { return 100000; }

private:
    bool created;

};

#if defined(QT_MODULE_CANVAS)
class GameCategory : public CategoryInterface
{
public:
    GameCategory( QWidgetStack *s ) : CategoryInterface( s ), created( FALSE ) {}

    QString name() const { return "Game"; }
    QIconSet icon() const { return QPixmap( joyicon ); }
    int numCategories() const { return 1; }
    QString categoryName( int i ) const {
	switch ( i ) {
	case 0:
	    return Frame::tr( "Asteroids" );
	    break;
	}
	return QString::null;
    }
    QIconSet categoryIcon( int ) const { return QIconSet(); }
    void setCurrentCategory( int i ) {
	create();
	stack->raiseWidget( i );
	qdemo_set_caption( this, i );
    }
    void create() {
	if ( created )
	    return;
	created = TRUE;
	stack->addWidget( new KAstTopLevel( stack ), categoryOffset() + 0 );
    }
    int categoryOffset() const { return 1000000; }

private:
    bool created;

};
#endif

int main( int argc, char **argv )
{
    QString category;
    QApplication a( argc, argv );

    Frame::updateTranslators();
    Frame frame;
    a.setMainWidget( &frame );

    QPtrList<CategoryInterface> categories;
    categories.append( new WidgetCategory( frame.widgetStack() ) );
#if defined(QT_MODULE_SQL)
    categories.append( new DatabaseCategory( frame.widgetStack() ) );
#endif
    categories.append( new CanvasCategory( frame.widgetStack() ) );
#if defined(QT_MODULE_OPENGL)
    categories.append( new OpenGLCategory( frame.widgetStack() ) );
#endif
    categories.append( new TextCategory( frame.widgetStack() ) );
    categories.append( new I18NCategory( frame.widgetStack() ) );
    categories.append( new GameCategory( frame.widgetStack() ) );
    frame.setCategories( categories );

    frame.resize( 1000, 700 );
    frame.show();

    return a.exec();
}
