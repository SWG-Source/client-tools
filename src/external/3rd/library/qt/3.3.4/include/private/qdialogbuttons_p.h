/****************************************************************************
** $Id: qt/qdialogbuttons_p.h   3.3.4   edited Oct 13 2003 $
**
** Definition of QDialogButtons class.
**
** Copyright (C) 2003 Trolltech AS.  All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
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

#ifndef QDIALOGBUTTONS_P_H
#define QDIALOGBUTTONS_P_H

#ifndef QT_H
#ifndef QT_H
#include "qwidget.h"
#endif // QT_H
#endif

#ifndef QT_NO_DIALOGBUTTONS
struct QDialogButtonsPrivate;

class
QDialogButtons : public QWidget
{
    Q_OBJECT
public:
    enum Button { None=0, Accept=0x01, Reject=0x02, Help=0x04, Apply=0x08, All=0x10, Abort=0x20, Retry=0x40, Ignore=0x80 };
#ifndef QT_NO_DIALOG
    QDialogButtons(QDialog *parent, bool autoConnect = TRUE, Q_UINT32 buttons = Accept | Reject,
		   Orientation orient = Horizontal, const char *name = NULL);
#endif // QT_NO_DIALOG
    QDialogButtons(QWidget *parent, Q_UINT32 buttons = Accept | Reject, 
		   Orientation orient = Horizontal, const char *name = NULL);
    ~QDialogButtons();

    void setQuestionMode(bool);
    bool questionMode() const;

    void setButtonEnabled(Button button, bool enabled);
    bool isButtonEnabled(Button) const;

    inline void showButton(Button b) { setButtonVisible(b, TRUE) ; }
    inline void hideButton(Button b) { setButtonVisible(b, FALSE); }
    virtual void setButtonVisible(Button, bool visible);
    bool isButtonVisible(Button) const;

    void addWidget(QWidget *);

    virtual void setDefaultButton(Button);
    Button defaultButton() const;

    virtual void setButtonText(Button, const QString &);
    QString buttonText(Button) const;

    void setOrientation(Orientation);
    Orientation orientation() const;

    virtual QSize sizeHint(Button) const;
    QSize minimumSizeHint() const;
    QSize sizeHint() const;

protected:
    void layoutButtons();
    virtual QWidget *createButton(Button);

    void showEvent(QShowEvent *);
    void resizeEvent(QResizeEvent *);
    void styleChanged(QStyle &);

private slots:
    void handleClicked();

signals:
    void clicked(Button);
    void acceptClicked();
    void rejectClicked();
    void helpClicked();
    void applyClicked();
    void allClicked();
    void retryClicked();
    void ignoreClicked();
    void abortClicked();

private:
    QDialogButtonsPrivate *d;
    void init(Q_UINT32, Orientation);
};
#endif //QT_NO_DIALOGBUTTONS
#endif //QDIALOGBUTTONS_P_H
