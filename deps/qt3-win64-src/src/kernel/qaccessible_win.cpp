/****************************************************************************
** $Id: qaccessible_win.cpp 1964 2006-12-15 12:14:12Z chehrlic $
**
** Implementation of QAccessible classes for Windows
**
** Created : 20040807
**
** Copyright (C) 2004 Christian Ehrlicher
** Copyright (C) 2004 Andreas Hausladen
**
** This file is part of the dialogs module of the Qt GUI Toolkit.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about Qt Commercial License Agreements.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** Contact kde-cygwin@kde.org if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qaccessible.h"

#ifndef QT_NO_ACCESSIBILITY

#include "qapplication.h"
#include "qlibrary.h"
#include "qmessagebox.h" // ### dependency
#include "qt_windows.h"
#include "qwidget.h"

#include <mmsystem.h>

void QAccessible::initialize()
{}

void QAccessible::cleanup()
{}

void QAccessible::updateAccessibility( QObject * o, int who, QAccessible::Event reason )
{
    Q_ASSERT(o);

    QString soundName;
    switch (reason) {
    case PopupMenuStart:
        soundName = "MenuPopup";
        break;

    case MenuCommand:
        soundName = "MenuCommand";
        break;

    case Alert:
        {
#ifndef QT_NO_MESSAGEBOX
            QMessageBox *mb = (QMessageBox*)(o);
            if (mb) {
                switch (mb->icon()) {
                case QMessageBox::Warning:
                    soundName = "SystemExclamation";
                    break;
                case QMessageBox::Critical:
                    soundName = "SystemHand";
                    break;
                case QMessageBox::Information:
                    soundName = "SystemAsterisk";
                    break;
                default:
                    break;
                }
            } else
#endif // QT_NO_MESSAGEBOX
            {
                soundName = "SystemAsterisk";
            }
        }
        break;
    default:
        break;
    }

    if (soundName.length()) {
        QT_WA({
            PlaySoundW((TCHAR*)soundName.ucs2(), 0, SND_ALIAS | SND_ASYNC | SND_NODEFAULT | SND_NOWAIT);
        } , {
            PlaySoundA(soundName.local8Bit(), 0, SND_ALIAS | SND_ASYNC | SND_NODEFAULT | SND_NOWAIT );
        });
    }

    if (!isActive())
        return;

    typedef void (WINAPI *PtrNotifyWinEvent)(DWORD, HWND, LONG, LONG);

    static PtrNotifyWinEvent ptrNotifyWinEvent = 0;
    static bool resolvedNWE = false;
    if (!resolvedNWE) {
        resolvedNWE = true;
        ptrNotifyWinEvent = (PtrNotifyWinEvent)QLibrary::resolve("user32", "NotifyWinEvent");
    }
    if (!ptrNotifyWinEvent)
        return;

    // An event has to be associated with a window,
    // so find the first parent that is a widget.
    QWidget *w = 0;
    if (o->isWidgetType()) {
        w = (QWidget*)o;
    } else {
        QObject *p = o;
        while ((p = p->parent()) != 0) {
            if (p->isWidgetType()) {
                w = (QWidget*)p;
                break;
            }
        }
    }

    if (!w) {
        if (reason != QAccessible::ContextHelpStart &&
             reason != QAccessible::ContextHelpEnd)
            w = qApp->focusWidget();
        if (!w) {
            w = qApp->activeWindow();

            if (!w)
                return;

// ### Fixme
//             if (!w) {
//                 w = qApp->mainWidget();
//                 if (!w)
//                     return;
//             }
        }
    }

    if (reason != MenuCommand) // MenuCommand is faked
        ptrNotifyWinEvent(reason, w->winId(), OBJID_CLIENT, who);
}

#endif // QT_NO_ACCESSIBILITY
