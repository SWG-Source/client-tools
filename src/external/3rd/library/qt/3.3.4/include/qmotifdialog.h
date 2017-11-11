/****************************************************************************
** $Id: qt/qmotifdialog.h   3.3.4   edited May 27 2003 $
**
** Definition of Qt extension classes for Xt/Motif support.
**
** Copyright (C) 1992-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qt extension for Xt/Motif support.
**
** Licensees holding valid Qt Enterprise Edition licenses for X11 may use
** this file in accordance with the Qt Commercial License Agreement provided
** with the Software.
**
** This file is not available for use under any other license without
** express written permission from the copyright holder.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QMOTIFDIALOG_H
#define QMOTIFDIALOG_H

#include <qdialog.h>

#include <X11/Intrinsic.h>
#include <Xm/Xm.h>

class QMotifWidget;
class QMotifDialogPrivate;

class QMotifDialog : public QDialog
{
    Q_OBJECT

public:
    // obsolete
    enum DialogType {
	Prompt,
	Selection,
	Command,
	FileSelection,
	Template,
	Error,
	Information,
	Message,
	Question,
	Warning,
	Working
    };
    // obsolete
    QMotifDialog( DialogType dialogtype,
		  Widget parent, ArgList args = NULL, Cardinal argcount = 0,
		  const char *name = 0, bool modal = FALSE, WFlags flags = 0 );
    // obsolete
    QMotifDialog( Widget parent, ArgList args = NULL, Cardinal argcount = 0,
		  const char *name = 0, bool modal = FALSE, WFlags flags = 0 );

    QMotifDialog( Widget parent, const char *name = 0,
		  bool modal = FALSE, WFlags flags = 0 );
    QMotifDialog( QWidget *parent, const char *name = 0,
		  bool modal = FALSE, WFlags flags = 0 );

    virtual ~QMotifDialog();

    Widget shell() const;
    Widget dialog() const;

    void show();
    void hide();

    static void acceptCallback( Widget, XtPointer, XtPointer );
    static void rejectCallback( Widget, XtPointer, XtPointer );

public slots:
    void accept();
    void reject();

protected:
    bool event( QEvent * );

#if !defined(Q_NO_USING_KEYWORD)
    using QObject::insertChild;
#endif

private:
    QMotifDialogPrivate *d;

    void init( Widget parent = NULL, ArgList args = NULL, Cardinal argcount = 0);

    void realize( Widget w );
    void insertChild( Widget w );
    void deleteChild( Widget w );

    friend void qmotif_dialog_realize( Widget, XtValueMask *, XSetWindowAttributes *);
    friend void qmotif_dialog_insert_child( Widget );
    friend void qmotif_dialog_delete_child( Widget );
    friend void qmotif_dialog_change_managed( Widget );
};

#endif // QMOTIFDIALOG_H
