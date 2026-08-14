/****************************************************************************
** $Id: qprogressdialog.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of QProgressDialog class
**
** Created : 970521
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of the dialogs module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech ASA of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qprogressdialog.h"

#ifndef QT_NO_PROGRESSDIALOG

#include "qaccel.h"
#include "qpainter.h"
#include "qdrawutil.h"
#include "qdatetime.h"
#include "qapplication.h"
#include "qstyle.h"
#include "qpushbutton.h"
#include "qcursor.h"
#include "qtimer.h"
#include <limits.h>

// If the operation is expected to take this long (as predicted by
// progress time), show the progress dialog.
static const int defaultShowTime    = 4000;
// Wait at least this long before attempting to make a prediction.
static const int minWaitTime = 50;

// Various layout values
static const int margin_lr   = 10;
static const int margin_tb   = 10;
static const int spacing     = 4;


class QProgressDialogData
{
public:
    QProgressDialogData( QProgressDialog* that, QWidget* parent,
		   const QString& labelText,
		   int totalSteps ) :
	creator( parent ),
	label( new QLabel(labelText,that,"label") ),
	cancel( 0 ),
	bar( new QProgressBar(totalSteps,that,"bar") ),
	shown_once( FALSE ),
	cancellation_flag( FALSE ),
	showTime( defaultShowTime )
    {
	label->setAlignment(that->style().styleHint(QStyle::SH_ProgressDialog_TextLabelAlignment, that));
    }

    QWidget	 *creator;
    QLabel	 *label;
    QPushButton	 *cancel;
    QProgressBar *bar;
    bool	  shown_once;
    bool	  cancellation_flag;
    QTime	  starttime;
#ifndef QT_NO_CURSOR
    QCursor	  parentCursor;
#endif
    int		  showTime;
    bool autoClose;
    bool autoReset;
    bool forceHide;
};


/*!
  \class QProgressDialog qprogressdialog.h
  \brief The QProgressDialog class provides feedback on the progress of a slow operation.
  \ingroup dialogs
  \mainclass

  A progress dialog is used to give the user an indication of how long
  an operation is going to take, and to demonstrate that the
  application has not frozen. It can also give the user an opportunity
  to abort the operation.

  A common problem with progress dialogs is that it is difficult to know
  when to use them; operations take different amounts of time on different
  hardware.  QProgressDialog offers a solution to this problem:
  it estimates the time the operation will take (based on time for
  steps), and only shows itself if that estimate is beyond minimumDuration()
  (4 seconds by default).

  Use setTotalSteps() (or the constructor) to set the number of
  "steps" in the operation and call setProgress() as the operation
  progresses. The step value can be chosen arbitrarily. It can be the
  number of files copied, the number of bytes received, the number of
  iterations through the main loop of your algorithm, or some other
  suitable unit.  Progress starts at 0, and the progress dialog shows
  that the operation has finished when you call setProgress() with
  totalSteps() as its argument.

  The dialog automatically resets and hides itself at the end of the
  operation. Use setAutoReset() and setAutoClose() to change this
  behavior.

  There are two ways of using QProgressDialog: modal and modeless.

  Using a modal QProgressDialog is simpler for the programmer, but you
  must call QApplication::processEvents() or
  QEventLoop::processEvents(ExcludeUserInput) to keep the event loop
  running to ensure that the application doesn't freeze. Do the
  operation in a loop, call \l setProgress() at intervals, and check
  for cancellation with wasCanceled(). For example:
\code
QProgressDialog progress( "Copying files...", "Abort Copy", numFiles,
                          this, "progress", TRUE );
for ( int i = 0; i < numFiles; i++ ) {
    progress.setProgress( i );
    qApp->processEvents();

    if ( progress.wasCanceled() )
        break;
    //... copy one file
}
progress.setProgress( numFiles );
\endcode

  A modeless progress dialog is suitable for operations that take
  place in the background, where the user is able to interact with the
  application. Such operations are typically based on QTimer (or
  QObject::timerEvent()), QSocketNotifier, or QUrlOperator; or performed
  in a separate thread. A QProgressBar in the status bar of your main window
  is often an alternative to a modeless progress dialog.

  You need to have an event loop to be running, connect the
  canceled() signal to a slot that stops the operation, and call \l
  setProgress() at intervals. For example:
\code
Operation::Operation( QObject *parent = 0 )
    : QObject( parent ), steps( 0 )
{
    pd = new QProgressDialog( "Operation in progress.", "Cancel", 100 );
    connect( pd, SIGNAL(canceled()), this, SLOT(cancel()) );
    t = new QTimer( this );
    connect( t, SIGNAL(timeout()), this, SLOT(perform()) );
    t->start( 0 );
}

void Operation::perform()
{
    pd->setProgress( steps );
    //... perform one percent of the operation
    steps++;
    if ( steps > pd->totalSteps() )
        t->stop();
}

void Operation::cancel()
{
    t->stop();
    //... cleanup
}
\endcode


  In both modes the progress dialog may be customized by
  replacing the child widgets with custom widgets by using setLabel(),
  setBar(), and setCancelButton().
  The functions setLabelText() and setCancelButtonText()
  set the texts shown.

  <img src=qprogdlg-m.png> <img src=qprogdlg-w.png>

  \sa QDialog QProgressBar
  \link guibooks.html#fowler GUI Design Handbook: Progress Indicator\endlink
*/


/*!
  Returns the QLabel currently being displayed above the progress bar.
  This QLabel is owned by the QProgressDialog.

  \sa setLabel()
*/
QLabel *QProgressDialog::label() const
{
    return d->label;
}

/*!
  Returns the QProgressBar currently being used to display progress.
  This QProgressBar is owned by the QProgressDialog.

  \sa setBar()
*/
QProgressBar *QProgressDialog::bar() const
{
    return d->bar;
}


/*!
  Constructs a progress dialog.

  Default settings:
  \list
  \i The label text is empty.
  \i The cancel button text is (translated) "Cancel".
  \i The total number of steps is 100.
  \endlist

  The \a creator argument is the widget to use as the dialog's parent.
  The \a name, \a modal, and the widget flags, \a f, are
  passed to the QDialog::QDialog() constructor. If \a modal is FALSE (the
  default), you must have an event loop proceeding for any redrawing
  of the dialog to occur. If \a modal is TRUE, the dialog ensures that
  events are processed when needed.

  \sa setLabelText(), setLabel(), setCancelButtonText(), setCancelButton(),
  setTotalSteps()
*/

QProgressDialog::QProgressDialog( QWidget *creator, const char *name,
				  bool modal, WFlags f )
    : QDialog( creator, name, modal, f)
{
    init( creator, QString::fromLatin1(""), tr("Cancel"), 100 );
}

/*!
  Constructs a progress dialog.

   The \a labelText is text used to remind the user what is progressing.

   The \a cancelButtonText is the text to display on the cancel button,
	    or 0 if no cancel button is to be shown.

   The \a totalSteps is the total number of steps in the operation for
   which this progress dialog shows progress.  For example, if the
   operation is to examine 50 files, this value would be 50. Before
   examining the first file, call setProgress(0). As each file is
   processed call setProgress(1), setProgress(2), etc., finally
   calling setProgress(50) after examining the last file.

   The \a creator argument is the widget to use as the dialog's parent.
   The \a name, \a modal, and widget flags, \a f, are passed to the
   QDialog::QDialog() constructor. If \a modal is FALSE (the default),
   you will must have an event loop proceeding for any redrawing of
   the dialog to occur. If \a modal is TRUE, the dialog ensures that
   events are processed when needed.


  \sa setLabelText(), setLabel(), setCancelButtonText(), setCancelButton(),
  setTotalSteps()
*/

QProgressDialog::QProgressDialog( const QString &labelText,
				  const QString &cancelButtonText,
				  int totalSteps,
				  QWidget *creator, const char *name,
				  bool modal, WFlags f )
    : QDialog( creator, name, modal, f)
{
    init( creator, labelText, cancelButtonText, totalSteps );
}


/*!
  Destroys the progress dialog.
*/

QProgressDialog::~QProgressDialog()
{
#ifndef QT_NO_CURSOR
    if ( d->creator )
	d->creator->setCursor( d->parentCursor );
#endif
    delete d;
}

void QProgressDialog::init( QWidget *creator,
			    const QString& lbl, const QString& canc,
			    int totstps)
{
    d = new QProgressDialogData(this, creator, lbl, totstps);
    d->autoClose = TRUE;
    d->autoReset = TRUE;
    d->forceHide = FALSE;
    setCancelButtonText( canc );
    connect( this, SIGNAL(canceled()), this, SIGNAL(cancelled()) );
    connect( this, SIGNAL(canceled()), this, SLOT(cancel()) );
    forceTimer = new QTimer( this );
    connect( forceTimer, SIGNAL(timeout()), this, SLOT(forceShow()) );
    layout();
}

/*!
  \fn void QProgressDialog::canceled()

  This signal is emitted when the cancel button is clicked.
  It is connected to the cancel() slot by default.

  \sa wasCanceled()
*/

/*!
  \fn void QProgressDialog::cancelled()

  \obsolete

  Use canceled() instead.
*/


/*!
  Sets the label to \a label. The progress dialog resizes to fit. The
  label becomes owned by the progress dialog and will be deleted when
  necessary, so do not pass the address of an object on the stack.

  \sa setLabelText()
*/

void QProgressDialog::setLabel( QLabel *label )
{
    delete d->label;
    d->label = label;
    if (label) {
	if ( label->parentWidget() == this ) {
	    label->hide(); // until we resize
	} else {
	    label->reparent( this, 0, QPoint(0,0), FALSE );
	}
    }
    int w = QMAX( isVisible() ? width() : 0, sizeHint().width() );
    int h = QMAX( isVisible() ? height() : 0, sizeHint().height() );
    resize( w, h );
    if (label)
	label->show();
}


/*!
  \property QProgressDialog::labelText
  \brief the label's text

  The default text is QString::null.
*/

QString QProgressDialog::labelText() const
{
    if ( label() )
	return label()->text();
    return QString::null;
}

void QProgressDialog::setLabelText( const QString &text )
{
    if ( label() ) {
	label()->setText( text );
	int w = QMAX( isVisible() ? width() : 0, sizeHint().width() );
	int h = QMAX( isVisible() ? height() : 0, sizeHint().height() );
	resize( w, h );
    }
}


/*!
  Sets the cancel button to the push button, \a cancelButton. The
  progress dialog takes ownership of this button which will be deleted
  when necessary, so do not pass the address of an object that is on
  the stack, i.e. use new() to create the button.

  \sa setCancelButtonText()
*/

void QProgressDialog::setCancelButton( QPushButton *cancelButton )
{
    delete d->cancel;
    d->cancel = cancelButton;
    if (cancelButton) {
	if ( cancelButton->parentWidget() == this ) {
	    cancelButton->hide(); // until we resize
	} else {
	    cancelButton->reparent( this, 0, QPoint(0,0), FALSE );
	}
	connect( d->cancel, SIGNAL(clicked()), this, SIGNAL(canceled()) );
#ifndef QT_NO_ACCEL
	QAccel *accel = new QAccel( this );
	accel->connectItem( accel->insertItem(Key_Escape),
			    d->cancel, SIGNAL(clicked()) );
#endif
    }
    int w = QMAX( isVisible() ? width() : 0, sizeHint().width() );
    int h = QMAX( isVisible() ? height() : 0, sizeHint().height() );
    resize( w, h );
    if (cancelButton)
	cancelButton->show();
}

/*!
  Sets the cancel button's text to \a cancelButtonText.
  \sa setCancelButton()
*/

void QProgressDialog::setCancelButtonText( const QString &cancelButtonText )
{
    if ( !cancelButtonText.isNull() ) {
	if ( d->cancel )
	    d->cancel->setText(cancelButtonText);
	else
	    setCancelButton(new QPushButton(cancelButtonText, this, "cancel"));
    } else {
	setCancelButton(0);
    }
    int w = QMAX( isVisible() ? width() : 0, sizeHint().width() );
    int h = QMAX( isVisible() ? height() : 0, sizeHint().height() );
    resize( w, h );
}


/*!
  Sets the progress bar widget to \a bar. The progress dialog resizes to
  fit. The progress dialog takes ownership of the progress \a bar which
  will be deleted when necessary, so do not use a progress bar
  allocated on the stack.
*/

void QProgressDialog::setBar( QProgressBar *bar )
{
    if ( progress() > 0 ) {
#if defined(QT_CHECK_STATE)
	qWarning( "QProgrssDialog::setBar: Cannot set a new progress bar "
		 "while the old one is active" );
#endif
    }
    delete d->bar;
    d->bar = bar;
    int w = QMAX( isVisible() ? width() : 0, sizeHint().width() );
    int h = QMAX( isVisible() ? height() : 0, sizeHint().height() );
    resize( w, h );
}


/*!
  \property QProgressDialog::wasCancelled
  \brief whether the dialog was canceled

  \obsolete

  Use \l wasCanceled instead.
*/

/*!
  \property QProgressDialog::wasCanceled
  \brief whether the dialog was canceled

  \sa setProgress()
*/

bool QProgressDialog::wasCancelled() const
{
    return d->cancellation_flag;
}


/*!
  \property QProgressDialog::totalSteps
  \brief the total number of steps

  The default is 0.
*/

int QProgressDialog::totalSteps() const
{
    if ( d && d->bar )
	return bar()->totalSteps();
    return 0;
}

void QProgressDialog::setTotalSteps( int totalSteps )
{
    bar()->setTotalSteps( totalSteps );
}


/*!
  Resets the progress dialog.
  The progress dialog becomes hidden if autoClose() is TRUE.

  \sa setAutoClose(), setAutoReset()
*/

void QProgressDialog::reset()
{
#ifndef QT_NO_CURSOR
    if ( progress() >= 0 ) {
	if ( d->creator )
	    d->creator->setCursor( d->parentCursor );
    }
#endif
    if ( d->autoClose || d->forceHide )
	hide();
    bar()->reset();
    d->cancellation_flag = FALSE;
    d->shown_once = FALSE;
    forceTimer->stop();
}

/*!
  Resets the progress dialog. wasCanceled() becomes TRUE until
  the progress dialog is reset.
  The progress dialog becomes hidden.
*/

void QProgressDialog::cancel()
{
    d->forceHide = TRUE;
    reset();
    d->forceHide = FALSE;
    d->cancellation_flag = TRUE;
}

/*!
  \property QProgressDialog::progress
  \brief the current amount of progress made.

  For the progress dialog to work as expected, you should initially set
  this property to 0 and finally set it to
  QProgressDialog::totalSteps(); you can call setProgress() any number of times
  in-between.

  \warning If the progress dialog is modal
    (see QProgressDialog::QProgressDialog()),
    this function calls QApplication::processEvents(), so take care that
    this does not cause undesirable re-entrancy in your code. For example,
    don't use a QProgressDialog inside a paintEvent()!

  \sa totalSteps
*/

int QProgressDialog::progress() const
{
    return bar()->progress();
}

void QProgressDialog::setProgress( int progress )
{
    if ( progress == bar()->progress() ||
	 bar()->progress() == -1 && progress == bar()->totalSteps() )
	return;

    bar()->setProgress(progress);

    if ( d->shown_once ) {
	if (testWFlags(WShowModal))
	    qApp->processEvents();
    } else {
	if ( progress == 0 ) {
#ifndef QT_NO_CURSOR
	    if ( d->creator ) {
		d->parentCursor = d->creator->cursor();
		d->creator->setCursor( waitCursor );
	    }
#endif
	    d->starttime.start();
	    forceTimer->start( d->showTime );
	    return;
	} else {
	    bool need_show;
	    int elapsed = d->starttime.elapsed();
	    if ( elapsed >= d->showTime ) {
		need_show = TRUE;
	    } else {
		if ( elapsed > minWaitTime ) {
		    int estimate;
		    if ( (totalSteps() - progress) >= INT_MAX / elapsed )
			estimate = (totalSteps() - progress) / progress * elapsed;
		    else
			estimate = elapsed * (totalSteps() - progress) / progress;
		    need_show = estimate >= d->showTime;
		} else {
		    need_show = FALSE;
		}
	    }
	    if ( need_show ) {
		int w = QMAX( isVisible() ? width() : 0, sizeHint().width() );
		int h = QMAX( isVisible() ? height() : 0, sizeHint().height() );
		resize( w, h );
		show();
		d->shown_once = TRUE;
	    }
	}
#ifdef Q_WS_MACX
	QApplication::flush();
#endif
    }

    if ( progress == bar()->totalSteps() && d->autoReset )
	reset();
}

/*!
  \overload

  Sets the current amount of progress to \a progress and the total number of
  steps to \a totalSteps.

  \sa setTotalSteps()
*/

void QProgressDialog::setProgress( int progress, int totalSteps )
{
    setTotalSteps( totalSteps );
    setProgress( progress );
}

/*!
  Returns a size that fits the contents of the progress dialog.
  The progress dialog resizes itself as required, so you should not
  need to call this yourself.
*/

QSize QProgressDialog::sizeHint() const
{
    QSize sh = label()->sizeHint();
    QSize bh = bar()->sizeHint();
    int h = margin_tb*2 + bh.height() + sh.height() + spacing;
    if ( d->cancel )
	h += d->cancel->sizeHint().height() + spacing;
    return QSize( QMAX(200, sh.width() + 2*margin_lr), h );
}

/*!\reimp
*/
void QProgressDialog::resizeEvent( QResizeEvent * )
{
    layout();
}

/*!
  \reimp
*/
void QProgressDialog::styleChange(QStyle& s)
{
    QDialog::styleChange(s);
    layout();
}

void QProgressDialog::layout()
{
    int sp = spacing;
    int mtb = margin_tb;
    int mlr = QMIN(width()/10, margin_lr);
    const bool centered =
	bool(style().styleHint(QStyle::SH_ProgressDialog_CenterCancelButton, this));

    QSize cs = d->cancel ? d->cancel->sizeHint() : QSize(0,0);
    QSize bh = bar()->sizeHint();
    int cspc;
    int lh = 0;

    // Find spacing and sizes that fit.  It is important that a progress
    // dialog can be made very small if the user demands it so.
    for (int attempt=5; attempt--; ) {
	cspc = d->cancel ? cs.height() + sp : 0;
	lh = QMAX(0, height() - mtb - bh.height() - sp - cspc);

	if ( lh < height()/4 ) {
	    // Getting cramped
	    sp /= 2;
	    mtb /= 2;
	    if ( d->cancel ) {
		cs.setHeight(QMAX(4,cs.height()-sp-2));
	    }
	    bh.setHeight(QMAX(4,bh.height()-sp-1));
	} else {
	    break;
	}
    }

    if ( d->cancel ) {
	d->cancel->setGeometry(
	    centered ? width()/2 - cs.width()/2 : width() - mlr - cs.width(),
	    height() - mtb - cs.height() + sp,
	    cs.width(), cs.height() );
    }

    label()->setGeometry( mlr, 0, width()-mlr*2, lh );
    bar()->setGeometry( mlr, lh+sp, width()-mlr*2, bh.height() );
}

/*!
    \property QProgressDialog::minimumDuration
    \brief the time that must pass before the dialog appears

    If the expected duration of the task is less than the
    minimumDuration, the dialog will not appear at all. This prevents
    the dialog popping up for tasks that are quickly over. For tasks
    that are expected to exceed the minimumDuration, the dialog will
    pop up after the minimumDuration time or as soon as any progress
    is set.

    If set to 0, the dialog is always shown as soon as any progress is
    set. The default is 4000 milliseconds.
*/
void QProgressDialog::setMinimumDuration( int ms )
{
    d->showTime = ms;
    if ( bar()->progress() == 0 ) {
	forceTimer->stop();
	forceTimer->start( ms );
    }
}

int QProgressDialog::minimumDuration() const
{
    return d->showTime;
}


/*!
  \reimp
*/

void QProgressDialog::closeEvent( QCloseEvent *e )
{
    emit canceled();
    QDialog::closeEvent( e );
}

/*!
  \property QProgressDialog::autoReset
  \brief whether the progress dialog calls reset() as soon as progress() equals totalSteps()

  The default is TRUE.

  \sa setAutoClose()
*/

void QProgressDialog::setAutoReset( bool b )
{
    d->autoReset = b;
}

bool QProgressDialog::autoReset() const
{
    return d->autoReset;
}

/*!
  \property QProgressDialog::autoClose
  \brief whether the dialog gets hidden by reset()

  The default is TRUE.

  \sa setAutoReset()
*/

void QProgressDialog::setAutoClose( bool b )
{
    d->autoClose = b;
}

bool QProgressDialog::autoClose() const
{
    return d->autoClose;
}

/*!
  \reimp
*/

void QProgressDialog::showEvent( QShowEvent *e )
{
    QDialog::showEvent( e );
    int w = QMAX( isVisible() ? width() : 0, sizeHint().width() );
    int h = QMAX( isVisible() ? height() : 0, sizeHint().height() );
    resize( w, h );
    forceTimer->stop();
}

/*!
  Shows the dialog if it is still hidden after the algorithm has been started
  and minimumDuration milliseconds have passed.

  \sa setMinimumDuration()
*/

void QProgressDialog::forceShow()
{
    if ( d->shown_once || d->cancellation_flag )
	return;

    show();
    d->shown_once = TRUE;
}


#endif
