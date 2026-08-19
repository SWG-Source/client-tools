/**********************************************************************
**
** Copyright (C) 2000-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of Qt Designer.
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
** See http://www.trolltech.com/gpl/ for GPL licensing information.
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef CPPEDITOR_H
#define CPPEDITOR_H

#include <editor.h>

class EditorCompletion;
class EditorBrowser;
struct DesignerInterface;
class CIndent;

class  CppEditor : public Editor
{
    Q_OBJECT

public:
    CppEditor( const QString &fn, QWidget *parent, const char *name, DesignerInterface *i );
    ~CppEditor();

    virtual EditorCompletion *completionManager() { return completion; }
    virtual EditorBrowser *browserManager() { return browser; }
    void configChanged();

    bool supportsBreakPoints() const { return FALSE; }
#if defined(Q_USING)
    using QTextEdit::createPopupMenu;
#endif
    QPopupMenu *createPopupMenu( const QPoint &p );

    void paste();

private slots:
    void addInclDecl();
    void addInclImpl();
    void addForward();

protected:
    EditorCompletion *completion;
    EditorBrowser *browser;
    DesignerInterface *dIface;
    CIndent *indent;

};

#endif
