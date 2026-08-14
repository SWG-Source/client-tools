/**********************************************************************
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of Qt/Embedded virtual framebuffer.
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

#include "qvfb.h"
#include "qvfbview.h"
#include "qvfbratedlg.h"
#include "config.h"
#include "skin.h"

#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qapplication.h>
#include <qmessagebox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qfiledialog.h>
#include <qslider.h>
#include <qspinbox.h>
#include <qradiobutton.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qcursor.h>
#include <qdragobject.h>
#include <qcheckbox.h>

QVFb::QVFb( int display_id, int w, int h, int d, const QString &skin, QWidget *parent,
	    const char *name, uint flags )
    : QMainWindow( parent, name, flags )
{
    const QMimeSource *m = QMimeSourceFactory::defaultFactory()->data( "logo.png" );
    if ( m ) {
	QPixmap pix;
	QImageDrag::decode( m, pix );
	setIcon( pix );
    }

    imagesave = new QFileDialog( this, 0, TRUE );

    rateDlg = 0;
    view = 0;
    init( display_id, w, h, d, skin );
    createMenu( menuBar() );
    adjustSize();
}

QVFb::~QVFb()
{
}

void QVFb::popupMenu()
{
    QPopupMenu *pm = new QPopupMenu( this );
    createMenu( pm );
    pm->exec(QCursor::pos());
}

void QVFb::init( int display_id, int w, int h, int d, const QString &skin_name )
{
    setCaption( QString("Virtual framebuffer %1x%2 %3bpp Display :%4")
		    .arg(w).arg(h).arg(d).arg(display_id) );
    delete view;

    if ( !skin_name.isEmpty() && QFile::exists(skin_name) ) {
	bool vis = isVisible();
	if ( vis ) hide();
	menuBar()->hide();
	Skin *skin = new Skin( this, skin_name, w, h );
	view = new QVFbView( display_id, w, h, d, skin );
	skin->setView( view );
	view->setMargin( 0 );
	view->setFrameStyle( QFrame::NoFrame );
	view->setFixedSize( w, h );
	setCentralWidget( skin );
	adjustSize();
	view->show();
	if ( vis ) show();
    } else {
	if ( !currentSkin.isEmpty() ) {
	    clearMask();
	    reparent( 0, 0, pos(), TRUE );
	}
	menuBar()->show();
	view = new QVFbView( display_id, w, h, d, this );
	view->setMargin( 0 );
	view->setFrameStyle( QFrame::NoFrame );
	setCentralWidget( view );
	resize( sizeHint() );
	view->show();
    }

    currentSkin = skin_name;
}

void QVFb::enableCursor( bool e )
{
    view->viewport()->setCursor( e ? ArrowCursor : BlankCursor );
    viewMenu->setItemChecked( cursorId, e );
}

void QVFb::createMenu(QMenuData *menu)
{
    QPopupMenu *file = new QPopupMenu( this );
    file->insertItem( "&Configure...", this, SLOT(configure()), ALT+CTRL+Key_C );
    file->insertSeparator();
    file->insertItem( "&Save image...", this, SLOT(saveImage()), ALT+CTRL+Key_S );
    file->insertItem( "&Animation...", this, SLOT(toggleAnimation()), ALT+CTRL+Key_A );
    file->insertSeparator();
    file->insertItem( "&Quit", qApp, SLOT(quit()) );

    menu->insertItem( "&File", file );

    viewMenu = new QPopupMenu( this );
    viewMenu->setCheckable( TRUE );
    cursorId = viewMenu->insertItem( "Show &Cursor", this, SLOT(toggleCursor()) );
    enableCursor(TRUE);
    viewMenu->insertItem( "&Refresh Rate...", this, SLOT(changeRate()) );
    viewMenu->insertSeparator();
    viewMenu->insertItem( "Zoom scale &1", this, SLOT(setZoom1()) );
    viewMenu->insertItem( "Zoom scale &2", this, SLOT(setZoom2()) );
    viewMenu->insertItem( "Zoom scale &3", this, SLOT(setZoom3()) );
    viewMenu->insertItem( "Zoom scale &4", this, SLOT(setZoom4()) );
    viewMenu->insertItem( "Zoom scale &0.5", this, SLOT(setZoomHalf()) );

    menu->insertItem( "&View", viewMenu );

    QPopupMenu *help = new QPopupMenu( this );
    help->insertItem("&About", this, SLOT(about()));
    help->insertItem("About &Qt", this, SLOT(aboutQt()));
    menu->insertSeparator();
    menu->insertItem( "&Help", help );
}

void QVFb::setZoom(double z)
{
    view->setZoom(z);
}

void QVFb::setZoomHalf()
{
    setZoom(0.5);
}

void QVFb::setZoom1()
{
    setZoom(1);
}

void QVFb::setZoom2()
{
    setZoom(2);
}

void QVFb::setZoom3()
{
    setZoom(3);
}

void QVFb::setZoom4()
{
    setZoom(4);
}

void QVFb::saveImage()
{
    QImage img = view->image();
    QString filename = imagesave->getSaveFileName("snapshot.png", "*.png", this, "", "Save Image");
    if ( !!filename )
	img.save(filename,"PNG");
}

void QVFb::toggleAnimation()
{
    if ( view->animating() ) {
	view->stopAnimation();
    } else {
	QString filename = imagesave->getSaveFileName("animation.mng", "*.mng", this, "", "Save animation");
	if ( !filename ) {
	    view->stopAnimation();
	} else {
	    view->startAnimation(filename);
	}
    }
}

void QVFb::toggleCursor()
{
    enableCursor( !viewMenu->isItemChecked( cursorId ) );
}

void QVFb::changeRate()
{
    if ( !rateDlg ) {
	rateDlg = new QVFbRateDialog( view->rate(), this );
	connect( rateDlg, SIGNAL(updateRate(int)), view, SLOT(setRate(int)) );
    }

    rateDlg->show();
}

void QVFb::about()
{
#if defined( Q_WS_MAC )
    QString platform("Mac OS X");
    QString qt("Mac");
#elif defined( Q_WS_WIN )
    QString platform("Windows");
    QString qt("Windows");
#else
    QString platform("X11");
    QString qt("X11");
#endif
    QMessageBox::about(this, "About QVFB",
	"<p><b><font size=+2>Qt/Embedded Virtual " + platform + " Framebuffer</font></b></p>"
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

void QVFb::aboutQt()
{
    QMessageBox::aboutQt( this, tr("QVFB") );
}

void QVFb::configure()
{
    config = new Config(this,0,TRUE);

    int w = view->displayWidth();
    int h = view->displayHeight();
    QString skin;
    config->size_width->setValue(w);
    config->size_height->setValue(h);
    config->size_custom->setChecked(TRUE); // unless changed by settings below
    config->size_240_320->setChecked(w==240&&h==320);
    config->size_320_240->setChecked(w==320&&h==240);
    config->size_640_480->setChecked(w==640&&h==480);
    config->skin->setEditable(TRUE);
    if (!currentSkin.isNull()) {
	config->size_skin->setChecked(TRUE);
	config->skin->setEditText(currentSkin);
    }
    config->touchScreen->setChecked(view->touchScreenEmulation());
    config->depth_1->setChecked(view->displayDepth()==1);
    config->depth_4gray->setChecked(view->displayDepth()==4);
    config->depth_8->setChecked(view->displayDepth()==8);
    config->depth_12->setChecked(view->displayDepth()==12);
    config->depth_16->setChecked(view->displayDepth()==16);
    config->depth_32->setChecked(view->displayDepth()==32);
    if ( view->gammaRed() == view->gammaGreen() && view->gammaGreen() == view->gammaBlue() ) {
	config->gammaslider->setValue(int(view->gammaRed()*400));
	config->rslider->setValue(100);
	config->gslider->setValue(100);
	config->bslider->setValue(100);
    } else {
	config->gammaslider->setValue(100);
	config->rslider->setValue(int(view->gammaRed()*400));
	config->gslider->setValue(int(view->gammaGreen()*400));
	config->bslider->setValue(int(view->gammaBlue()*400));
    }
    connect(config->gammaslider, SIGNAL(valueChanged(int)), this, SLOT(setGamma400(int)));
    connect(config->rslider, SIGNAL(valueChanged(int)), this, SLOT(setR400(int)));
    connect(config->gslider, SIGNAL(valueChanged(int)), this, SLOT(setG400(int)));
    connect(config->bslider, SIGNAL(valueChanged(int)), this, SLOT(setB400(int)));
    updateGammaLabels();

    double ogr=view->gammaRed(), ogg=view->gammaGreen(), ogb=view->gammaBlue();

    if ( config->exec() ) {
	int id = view->displayId(); // not settable yet
	if ( config->size_240_320->isChecked() ) {
	    w=240; h=320;
	} else if ( config->size_320_240->isChecked() ) {
	    w=320; h=240;
	} else if ( config->size_640_480->isChecked() ) {
	    w=640; h=480;
	} else if ( config->size_skin->isChecked() ) {
	    skin = config->skin->currentText();
	} else {
	    w=config->size_width->value();
	    h=config->size_height->value();
	}
	int d;
	if ( config->depth_1->isChecked() )
	    d=1;
	else if ( config->depth_4gray->isChecked() )
	    d=4;
	else if ( config->depth_8->isChecked() )
	    d=8;
	else if ( config->depth_12->isChecked() )
	    d=12;
	else if ( config->depth_16->isChecked() )
	    d=16;
	else
	    d=32;

	if ( w != view->displayWidth() || h != view->displayHeight()
		|| d != view->displayDepth() || skin != currentSkin )
	    init( id, w, h, d, skin );
	view->setTouchscreenEmulation( config->touchScreen->isChecked() );
    } else {
	view->setGamma(ogr, ogg, ogb);
    }

    delete config;
    config=0;
}

void QVFb::setGamma400(int n)
{
    double g = n/100.0;
    view->setGamma(config->rslider->value()/100.0*g,
                   config->gslider->value()/100.0*g,
                   config->bslider->value()/100.0*g);
    updateGammaLabels();
}

void QVFb::setR400(int n)
{
    double g = n/100.0;
    view->setGamma(config->rslider->value()/100.0*g,
                   view->gammaGreen(),
                   view->gammaBlue());
    updateGammaLabels();
}

void QVFb::setG400(int n)
{
    double g = n/100.0;
    view->setGamma(view->gammaRed(),
                   config->gslider->value()/100.0*g,
                   view->gammaBlue());
    updateGammaLabels();
}

void QVFb::setB400(int n)
{
    double g = n/100.0;
    view->setGamma(view->gammaRed(),
                   view->gammaGreen(),
                   config->bslider->value()/100.0*g);
    updateGammaLabels();
}

void QVFb::updateGammaLabels()
{
    config->rlabel->setText(QString::number(view->gammaRed(),'g',2));
    config->glabel->setText(QString::number(view->gammaGreen(),'g',2));
    config->blabel->setText(QString::number(view->gammaBlue(),'g',2));
}
