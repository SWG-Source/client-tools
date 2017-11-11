/****************************************************************************
** $Id: qt/qerrormessage.h   3.3.4   edited May 27 2003 $
**
** Definition of a nice qInstallErrorMessage() handler
**
** Created : 000527, after Kalle Dalheimer's birthday
**
** Copyright (C) 1992-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the dialogs module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
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

#ifndef QERRORMESSAGE_H
#define QERRORMESSAGE_H

#ifndef QT_H
#include "qdialog.h"
#endif // QT_H

#ifndef QT_NO_ERRORMESSAGE
class QPushButton;
class QCheckBox;
class QLabel;
class QTextView;
class QStringList;
template<class type> class QDict;


class Q_EXPORT QErrorMessage: public QDialog {
    Q_OBJECT
public:
    QErrorMessage( QWidget* parent, const char* name=0 );
    ~QErrorMessage();

    static QErrorMessage * qtHandler();

public slots:
    void message( const QString & );

protected:
    void done( int );

private:
    QPushButton * ok;
    QCheckBox * again;
    QTextView * errors;
    QLabel * icon;
    QStringList * pending;
    QDict<int> * doNotShow;

    bool nextPending();

#if defined(Q_DISABLE_COPY) // Disabled copy constructor and operator=
    QErrorMessage( const QErrorMessage & );
    QErrorMessage &operator=( const QErrorMessage & );
#endif
};

#endif //QT_NO_ERRORMESSAGE

#endif
