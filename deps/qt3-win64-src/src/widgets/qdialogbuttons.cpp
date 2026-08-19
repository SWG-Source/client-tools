/****************************************************************************
** $Id: qdialogbuttons.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of QDialogButtons class
**
** Copyright (C) 2005-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
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

#include "qdialogbuttons_p.h"
#ifndef QT_NO_DIALOGBUTTONS

#include <qapplication.h>
#include <qpushbutton.h>
#include <qguardedptr.h>
#include <qmap.h>
#include <qvariant.h>
#ifndef QT_NO_DIALOG
#include <qdialog.h>
#endif // QT_NO_DIALOG
#include <qlayout.h>
#include <qstyle.h>
#include <qmap.h>

struct QDialogButtonsPrivate
{
    QMap<int, QString> text;
    QMap<QDialogButtons::Button, QWidget *> buttons;
    QGuardedPtr<QWidget> custom;
    Q_UINT32 enabled, visible;
    QDialogButtons::Button def;
    Qt::Orientation orient;
    bool questionMode;
};

#ifndef QT_NO_DIALOG
QDialogButtons::QDialogButtons(QDialog *parent, bool autoConnect, Q_UINT32 buttons,
			       Orientation orient, const char *name ) : QWidget(parent, name)
{
    init(buttons, orient);
    if(parent && autoConnect) {
	QObject::connect(this, SIGNAL(acceptClicked()), parent, SLOT(accept()));
	QObject::connect(this, SIGNAL(rejectClicked()), parent, SLOT(reject()));
    }
}
#endif // QT_NO_DIALOG

QDialogButtons::QDialogButtons(QWidget *parent, Q_UINT32 buttons, 
			       Orientation orient, const char *name ) : QWidget(parent, name)
{
    init(buttons, orient);
}

void
QDialogButtons::init(Q_UINT32 buttons, Orientation orient)
{
    if(buttons == All) {
	qWarning("QDialogButtons: cannot specify All by itself!");
	buttons = None;
    }
    d = new QDialogButtonsPrivate;
    d->questionMode = FALSE;
    d->orient = orient;
    d->def = (Button)style().styleHint(QStyle::SH_DialogButtons_DefaultButton, this);
    d->enabled = d->visible = buttons;
}

QDialogButtons::~QDialogButtons()
{
    delete (QWidget *)d->custom;
    delete d;
}

void
QDialogButtons::setQuestionMode(bool b)
{
    d->questionMode = b;
}

bool
QDialogButtons::questionMode() const
{
    return d->questionMode;
}

void
QDialogButtons::setButtonEnabled(Button button, bool enabled)
{
    if(enabled) 
	d->enabled |= button;
    else
	d->enabled ^= button;
    if(d->buttons.contains(button))
	d->buttons[button]->setEnabled(enabled);
}

bool
QDialogButtons::isButtonEnabled(Button button) const
{
    return ((int)(d->enabled & button)) == button;
}

void
QDialogButtons::setButtonVisible(Button button, bool visible)
{
    if(visible) {
	if(d->buttons.contains(button))
	    d->buttons[button]->show();
	d->visible |= button;
    } else {
	if(d->buttons.contains(button))
	    d->buttons[button]->hide();
	d->visible ^= button;
    }
    layoutButtons();
}

bool
QDialogButtons::isButtonVisible(Button button) const
{
    return ((int)(d->visible & button)) == button;
}

void
QDialogButtons::addWidget(QWidget *w)
{
    QBoxLayout *lay = NULL;
    if(!d->custom) {
	d->custom = new QWidget(this, "dialog_custom_area");
	if(orientation() == Horizontal)
	    lay = new QHBoxLayout(d->custom);
	else
	    lay = new QVBoxLayout(d->custom);
	layoutButtons();
    } else {
	lay = (QBoxLayout*)d->custom->layout();
    }
    if(w->parent() != d->custom)
	w->reparent(d->custom, 0, QPoint(0, 0), TRUE);
    lay->addWidget(w);
}

void
QDialogButtons::setDefaultButton(Button button)
{
    if(!((int)(d->visible & button) == button)) {
	qWarning("QDialogButtons: Button '%d' is not visible (so cannot be default)", button);
	return;
    }
    if(d->def != button) {
#ifndef QT_NO_PROPERTIES
	if(d->buttons.contains(d->def))
	    d->buttons[d->def]->setProperty("default", QVariant(FALSE,0));
#endif
	d->def = button;
#ifndef QT_NO_PROPERTIES
	if(d->buttons.contains(d->def))
	    d->buttons[d->def]->setProperty("default", QVariant(FALSE,0));
#endif
    }
}

QDialogButtons::Button
QDialogButtons::defaultButton() const
{
    return d->def;
}

void
QDialogButtons::setButtonText(Button button, const QString &str)
{
    d->text[button] = str;
#ifndef QT_NO_PROPERTIES
    if(d->buttons.contains(button))
	d->buttons[button]->setProperty("text", QVariant(str));
#endif
    layoutButtons();
}

QString
QDialogButtons::buttonText(Button b) const
{
    if(d->text.contains(b))
	return d->text[b];
    return QString(); //null if it is default..
}

void
QDialogButtons::setOrientation(Orientation orient)
{
    if(d->orient != orient) {
	d->orient = orient;
	if(d->custom && d->custom->layout())
	    ((QBoxLayout*)d->custom->layout())->setDirection(orient == Horizontal ? QBoxLayout::LeftToRight : 
							     QBoxLayout::TopToBottom);
	layoutButtons();
    }
}

Qt::Orientation
QDialogButtons::orientation() const
{
    return d->orient;
}

QWidget *
QDialogButtons::createButton(Button b)
{
    QPushButton *ret = new QPushButton(this, "qdialog_button");
    QObject::connect(ret, SIGNAL(clicked()), this, SLOT(handleClicked()));
    if(d->text.contains(b)) {
	ret->setText(d->text[b]);
    } else {
	switch(b) {
	case All: {
	    QString txt = buttonText(defaultButton());
	    if(txt.isNull()) {
		if(defaultButton() == Accept) {
		    if(questionMode())
			txt = tr("Yes to All");
		    else
		        txt = tr("OK to All");
		} else {
		    if(questionMode())
		        txt = tr("No to All");
		    else
			txt = tr("Cancel All");
		}
	    } else {
		txt += tr(" to All"); //ick, I can't really do this!!
	    }
	    ret->setText(txt);
	    break; }
	case Accept:
	    if(questionMode())
		ret->setText(tr("Yes"));
	    else
		ret->setText(tr("OK"));
	    break;
	case Reject:
	    if(questionMode())
		ret->setText(tr("No"));
	    else
		ret->setText(tr("Cancel"));
	    break;
	case Apply:
	    ret->setText(tr("Apply"));
	    break;
	case Ignore:
	    ret->setText(tr("Ignore"));
	    break;
	case Retry:
	    ret->setText(tr("Retry"));
	    break;
	case Abort:
	    ret->setText(tr("Abort"));
	    break;
	case Help:
	    ret->setText(tr("Help"));
	    break;
	default:
	    break;
	}
    }
    return ret;
}

void
QDialogButtons::handleClicked()
{
    const QObject *s = sender();
    if(!s)
	return;

    for(QMapIterator<QDialogButtons::Button, QWidget *> it = d->buttons.begin(); it != d->buttons.end(); ++it) {
	if(it.data() == s) {
	    emit clicked((QDialogButtons::Button)it.key());
	    switch(it.key()) {
	    case Retry:
		emit retryClicked();
		break;
	    case Ignore:
		emit ignoreClicked();
		break;
	    case Abort:
		emit abortClicked();
		break;
	    case All:
		emit allClicked();
		break;
	    case Accept:
		emit acceptClicked();
		break;
	    case Reject:
		emit rejectClicked();
		break;
	    case Apply:
		emit applyClicked();
		break;
	    case Help:
		emit helpClicked();
		break;
	    default:
		break;
	    }
	    return;
	}
    }
}

void
QDialogButtons::resizeEvent(QResizeEvent *)
{
    layoutButtons();
}

void
QDialogButtons::showEvent(QShowEvent *)
{
    layoutButtons();
}

void
QDialogButtons::styleChanged(QStyle &old)
{
    layoutButtons();
    QWidget::styleChange(old);
}

void
QDialogButtons::layoutButtons()
{
    if(!isVisible()) //nah..
	return;

    QStyle::SubRect rects[] = {
    	QStyle::SR_DialogButtonAccept, QStyle::SR_DialogButtonReject,
	QStyle::SR_DialogButtonApply,  QStyle::SR_DialogButtonHelp, 
	QStyle::SR_DialogButtonCustom, QStyle::SR_DialogButtonAll,
        QStyle::SR_DialogButtonRetry,  QStyle::SR_DialogButtonIgnore,
        QStyle::SR_DialogButtonAbort };
    for(unsigned int i = 0; i < (sizeof(rects) / sizeof(rects[0])); i++) {
	QWidget *w = NULL;
	if(rects[i] == QStyle::SR_DialogButtonCustom) {
	    w = d->custom;
	} else {
	    Button b = None;
	    if(rects[i] == QStyle::SR_DialogButtonApply)
		b = Apply;
	    else if(rects[i] == QStyle::SR_DialogButtonAll)
		b = All;
	    else if(rects[i] == QStyle::SR_DialogButtonAccept)
		b = Accept;
	    else if(rects[i] == QStyle::SR_DialogButtonReject)
		b = Reject;
	    else if(rects[i] == QStyle::SR_DialogButtonHelp)
		b = Help;
	    else if(rects[i] == QStyle::SR_DialogButtonRetry)
		b = Retry;
	    else if(rects[i] == QStyle::SR_DialogButtonAbort)
		b = Abort;
	    else if(rects[i] == QStyle::SR_DialogButtonIgnore)
		b = Ignore;
	    if(b != None) {
		if(d->buttons.contains(b)) {
		    w = d->buttons[b];
		    if(!(d->visible & b)) {
			w->hide();
			continue;
		    }
		} else if(d->visible & b) {
		    w = createButton(b);
		    d->buttons.insert(b, w);
		} else {
		    continue;
		}
		if(w) {
		    if(b == d->def) {
			w->setFocus();
#ifndef QT_NO_PROPERTIES
			w->setProperty("default", QVariant(TRUE,0));
#endif
		    }
		    w->setEnabled(d->enabled & b);
		}
	    }
	}
	if(w) {
	    w->show();
	    w->setGeometry(style().subRect(rects[i], this));
	}
    }
}

QSize
QDialogButtons::sizeHint() const
{
    constPolish();
    QSize s;
    if(d->custom)
	s = d->custom->sizeHint();
    return (style().sizeFromContents(QStyle::CT_DialogButtons, this, s.
				     expandedTo(QApplication::globalStrut())));
}

QSize
QDialogButtons::sizeHint(QDialogButtons::Button button) const
{
    QWidget *w = NULL;
    if(d->visible & button) {
	if(!d->buttons.contains(button)) {
	    QDialogButtons *that = (QDialogButtons*)this; //ick, constness..
	    w = that->createButton(button);
	    that->d->buttons.insert(button, w);
	} else {
	    w = d->buttons[button];
	}
    }
    return w->sizeHint();
}

QSize
QDialogButtons::minimumSizeHint() const
{
    return sizeHint();
}
#endif
