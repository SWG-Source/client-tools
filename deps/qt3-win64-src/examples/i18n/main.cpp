/****************************************************************************
** $Id: main.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qapplication.h>
#include <qtranslator.h>
#include <qfileinfo.h>
#include <qmessagebox.h>
#include <qcheckbox.h>
#include <qvbox.h>
#include <qlayout.h>
#include <qbuttongroup.h>
#include <qpushbutton.h>
#include <qsignalmapper.h>
#include <qtextcodec.h>
#include <stdlib.h>

#if defined(Q_OS_UNIX)
#include <unistd.h>
#endif

#include "mywidget.h"

//#define USE_I18N_FONT

class QVDialog : public QDialog {
public:
    QVDialog(QWidget *parent=0, const char *name=0, bool modal=FALSE,
             WFlags f=0) : QDialog(parent,name,modal,f)
    {
	QVBoxLayout* vb = new QVBoxLayout(this,8);
	vb->setAutoAdd(TRUE);
	hb = 0;
	sm = new QSignalMapper(this);
	connect(sm,SIGNAL(mapped(int)),this,SLOT(done(int)));
    }
    void addButtons( const QString& cancel=QString::null,
		    const QString& ok=QString::null,
		    const QString& mid1=QString::null,
		    const QString& mid2=QString::null,
		    const QString& mid3=QString::null)
    {
	addButton(ok.isNull() ? QObject::tr("OK") : ok, 1);
	if ( !mid1.isNull() ) addButton(mid1,2);
	if ( !mid2.isNull() ) addButton(mid2,3);
	if ( !mid3.isNull() ) addButton(mid3,4);
	addButton(cancel.isNull() ? QObject::tr("Cancel") : cancel, 0);
    }

    void addButton( const QString& text, int result )
    {
	if ( !hb )
	    hb = new QHBox(this);
	QPushButton *c = new QPushButton(text, hb);
	sm->setMapping(c,result);
	connect(c,SIGNAL(clicked()),sm,SLOT(map()));
    }

private:
    QSignalMapper *sm;
    QHBox *hb;
};

MyWidget* showLang(QString lang)
{

    static QTranslator *translator = 0;

    qApp->setPalette(QPalette(QColor(220-rand()%64,220-rand()%64,220-rand()%64)));

    lang = "mywidget_" + lang + ".qm";
    QFileInfo fi( lang );

    if ( !fi.exists() ) {
	QMessageBox::warning( 0, "File error",
			      QString("Cannot find translation for language: "+lang+
				      "\n(try eg. 'de', 'ko' or 'no')") );
	return 0;
    }
    if ( translator ) {
	qApp->removeTranslator( translator );
	delete translator;
    }
    translator = new QTranslator( 0 );
    translator->load( lang, "." );
    qApp->installTranslator( translator );
    MyWidget *m = new MyWidget;
    m->setCaption("Qt Example - i18n - " + m->caption() );
    return m;
}

int main( int argc, char** argv )
{
    QApplication app( argc, argv );

    const char* qm[]=
	{ "ar", "cs", "de", "el", "en", "eo", "fr", "it", "jp", "ko", "no", "ru", "zh", 0 };

#if defined(Q_OS_UNIX)
    srand( getpid() << 2 );
#endif

    QString lang;
    if ( argc == 2 )
        lang = argv[1];

    if ( argc != 2 || lang == "all" ) {
	QVDialog dlg(0,0,TRUE);
	QCheckBox* qmb[sizeof(qm)/sizeof(qm[0])];
	int r;
	if ( lang == "all" ) {
	    r = 2;
	} else {
	    QButtonGroup *bg = new QButtonGroup(4,Qt::Vertical,"Choose Locales",&dlg);
	    QString loc = QTextCodec::locale();
	    for ( int i=0; qm[i]; i++ ) {
		qmb[i] = new QCheckBox((const char*)qm[i],bg);
		qmb[i]->setChecked( loc == qm[i] );
	    }
	    dlg.addButtons("Cancel","OK","All");
	    r = dlg.exec();
	}
	if ( r ) {
	    QRect screen = qApp->desktop()->availableGeometry();
	    bool tight = screen.width() < 1024;
	    int x=screen.left()+5;
	    int y=screen.top()+25;
	    for ( int i=0; qm[i]; i++ ) {
		if ( r == 2 || qmb[i]->isChecked() ) {
		    MyWidget* w = showLang((const char*)qm[i]);

		    if( w == 0 ) exit( 0 );
		    QObject::connect(w, SIGNAL(closed()), qApp, SLOT(quit()));
		    w->setGeometry(x,y,197,356);
		    w->show();
		    if ( tight ) {
			x += 8;
			y += 8;
		    } else {
			x += 205;
			if ( x > 1000 ) {
			    x = 5;
			    y += 384;
			}
		    }
		}
	    }
	} else {
            exit( 0 );
        }
    } else {
	QString lang = argv[1];
	QWidget* m = showLang(lang);
	app.setMainWidget( m );
	m->setCaption("Qt Example - i18n");
	m->show();
    }

#ifdef USE_I18N_FONT
    memorymanager->savePrerenderedFont(font.handle(),FALSE);
#endif

    // While we run "all", kill them all
    return app.exec();

}
