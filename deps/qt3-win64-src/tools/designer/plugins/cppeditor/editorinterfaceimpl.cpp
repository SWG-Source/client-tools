/**********************************************************************
**
** Copyright (C) 2005-2007 Trolltech ASA.  All rights reserved.
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

#include "editorinterfaceimpl.h"
#include <viewmanager.h>
#include "cppeditor.h"
#include <private/qrichtext_p.h>
#include <qapplication.h>
#include "completion.h"
#include <designerinterface.h>
#include <qtimer.h>

EditorInterfaceImpl::EditorInterfaceImpl()
    : EditorInterface(), viewManager( 0 ), dIface( 0 )
{
    updateTimer = new QTimer( this );
    connect( updateTimer, SIGNAL( timeout() ),
	     this, SLOT( update() ) );
}

EditorInterfaceImpl::~EditorInterfaceImpl()
{
    updateTimer->stop();
    delete (ViewManager*)viewManager;
    if ( dIface )
	dIface->release();
}

QRESULT EditorInterfaceImpl::queryInterface( const QUuid &uuid, QUnknownInterface** iface )
{
    *iface = 0;
    if ( uuid == IID_QUnknown )
	*iface = (QUnknownInterface*)this;
    else if ( uuid == IID_Editor )
	*iface = (EditorInterface*)this;
    else
	return QE_NOINTERFACE;

    (*iface)->addRef();
    return QS_OK;
}

QWidget *EditorInterfaceImpl::editor( bool readonly,
				      QWidget *parent, QUnknownInterface *iface )
{
    if ( !viewManager ) {
	( (EditorInterfaceImpl*)this )->viewManager = new ViewManager( parent, 0 );
	( (EditorInterfaceImpl*)this )->viewManager->showMarkerWidget( FALSE );
	if ( iface )
	    iface->queryInterface( IID_Designer, (QUnknownInterface**) &dIface );
	CppEditor *e = new CppEditor( QString::null, viewManager, "editor", dIface );
	e->setEditable( !readonly );
	e->installEventFilter( this );
	connect( e, SIGNAL( intervalChanged() ), this, SLOT( intervalChanged() ) );
	QApplication::sendPostedEvents( viewManager, QEvent::ChildInserted );
    }
    return viewManager->currentView();
}

void EditorInterfaceImpl::setText( const QString &txt )
{
    if ( !viewManager || !viewManager->currentView() )
	return;
    CppEditor *e = (CppEditor*)viewManager->currentView();
    disconnect( e, SIGNAL( modificationChanged( bool ) ), this, SLOT( modificationChanged( bool ) ) );
    e->setText( txt );
    e->setModified( FALSE );
    connect( e, SIGNAL( modificationChanged( bool ) ), this, SLOT( modificationChanged( bool ) ) );
}

QString EditorInterfaceImpl::text() const
{
    if ( !viewManager || !viewManager->currentView() )
	return QString::null;
    QString txt = ( (CppEditor*)viewManager->currentView() )->text();
    if ( !txt.isEmpty() && !txt.endsWith("\n") )
	txt += "\n";
    return txt;
}

bool EditorInterfaceImpl::isUndoAvailable() const
{
    if ( !viewManager || !viewManager->currentView() )
	return FALSE;
    return ( (CppEditor*)viewManager->currentView() )->isUndoAvailable();
}

bool EditorInterfaceImpl::isRedoAvailable() const
{
    if ( !viewManager || !viewManager->currentView() )
	return FALSE;
    return ( (CppEditor*)viewManager->currentView() )->isRedoAvailable();
}

void EditorInterfaceImpl::undo()
{
    if ( !viewManager || !viewManager->currentView() )
	return;
    ( (CppEditor*)viewManager->currentView() )->undo();
}

void EditorInterfaceImpl::redo()
{
    if ( !viewManager || !viewManager->currentView() )
	return;
    ( (CppEditor*)viewManager->currentView() )->redo();
}

void EditorInterfaceImpl::cut()
{
    if ( !viewManager || !viewManager->currentView() )
	return;
    ( (CppEditor*)viewManager->currentView() )->cut();
}

void EditorInterfaceImpl::copy()
{
    if ( !viewManager || !viewManager->currentView() )
	return;
    ( (CppEditor*)viewManager->currentView() )->copy();
}

void EditorInterfaceImpl::paste()
{
    if ( !viewManager || !viewManager->currentView() )
	return;
    ( (CppEditor*)viewManager->currentView() )->paste();
}

void EditorInterfaceImpl::selectAll()
{
    if ( !viewManager || !viewManager->currentView() )
	return;
    ( (CppEditor*)viewManager->currentView() )->selectAll();
}

bool EditorInterfaceImpl::find( const QString &expr, bool cs, bool wo, bool forward, bool startAtCursor )
{
    if ( !viewManager || !viewManager->currentView() )
	return FALSE;
    CppEditor *e = (CppEditor*)viewManager->currentView();
    if ( startAtCursor )
	return e->find( expr, cs, wo, forward );
    int dummy = 0;
    return e->find( expr, cs, wo, forward, &dummy, &dummy );
}

bool EditorInterfaceImpl::replace( const QString &find, const QString &replace, bool cs, bool wo,
				   bool forward, bool startAtCursor, bool replaceAll )
{
    if ( !viewManager || !viewManager->currentView() )
	return FALSE;
    CppEditor *e = (CppEditor*)viewManager->currentView();
    bool ok = FALSE;
    if ( startAtCursor ) {
	ok = e->find( find, cs, wo, forward );
    } else {
	int dummy = 0;
	ok =  e->find( find, cs, wo, forward, &dummy, &dummy );
    }

    if ( ok ) {
	e->removeSelectedText();
	e->insert( replace, FALSE, FALSE );
    }

    if ( !replaceAll || !ok ) {
	if ( ok )
	    e->setSelection( e->textCursor()->paragraph()->paragId(),
			     e->textCursor()->index() - replace.length(),
			     e->textCursor()->paragraph()->paragId(),
			     e->textCursor()->index() );
	return ok;
    }

    bool ok2 = TRUE;
    while ( ok2 ) {
	ok2 = e->find( find, cs, wo, forward );
	if ( ok2 ) {
	    e->removeSelectedText();
	    e->insert( replace, FALSE, FALSE );
	}
    }

    return TRUE;
}

void EditorInterfaceImpl::gotoLine( int line )
{
    if ( !viewManager || !viewManager->currentView() )
	return;
    ( (CppEditor*)viewManager->currentView() )->setCursorPosition( line, 0 );
}

void EditorInterfaceImpl::indent()
{
    if ( !viewManager || !viewManager->currentView() )
	return;
    ( (CppEditor*)viewManager->currentView() )->QTextEdit::indent();
}

void EditorInterfaceImpl::splitView()
{
#if 0
    if ( !viewManager || !viewManager->currentView() )
	return;
    QTextDocument *doc = ( (CppEditor*)viewManager->currentView() )->document();
    CppEditor *editor = new CppEditor( QString::null, viewManager, "editor" );
    editor->setDocument( doc );
#endif
}

void EditorInterfaceImpl::scrollTo( const QString &txt, const QString & )
{
    if ( !viewManager || !viewManager->currentView() )
	return;
    ( (CppEditor*)viewManager->currentView() )->sync();
    QTextDocument *doc = ( (CppEditor*)viewManager->currentView() )->document();
    QTextParagraph *p = doc->firstParagraph();
    while ( p ) {
	if ( p->string()->toString().find( txt ) != -1 ) {
	    ( (CppEditor*)viewManager->currentView() )->setCursorPosition( p->paragId() + 2, 0 );
	    break;
	}
	p = p->next();
    }
    ( (CppEditor*)viewManager->currentView() )->setFocus();
}

void EditorInterfaceImpl::setContext( QObject *this_ )
{
    if ( !viewManager || !viewManager->currentView() )
	return;
    ( (CppEditor*)viewManager->currentView() )->completionManager()->setContext( this_ );
}

void EditorInterfaceImpl::setError( int line )
{
    if ( !viewManager )
	return;
    viewManager->setError( line );
}

void EditorInterfaceImpl::readSettings()
{
    if ( !viewManager )
	return;
    ( (CppEditor*)viewManager->currentView() )->configChanged();
}

void EditorInterfaceImpl::modificationChanged( bool m )
{
    if ( viewManager && dIface )
	dIface->setModified( m, viewManager->currentView() );
}

void EditorInterfaceImpl::setModified( bool m )
{
    if ( !viewManager )
	return;
    ( (CppEditor*)viewManager->currentView() )->setModified( m );
}

bool EditorInterfaceImpl::isModified() const
{
    if ( !viewManager )
	return FALSE;
    return ( (CppEditor*)viewManager->currentView() )->isModified();
}

bool EditorInterfaceImpl::eventFilter( QObject *o, QEvent *e )
{
    if ( e->type() == QEvent::KeyPress )
	updateTimer->stop();
    else if ( e->type() == QEvent::FocusOut )
	update();

    return QObject::eventFilter( o, e );
}

int EditorInterfaceImpl::numLines() const
{
    if ( !viewManager || !viewManager->currentView() )
	return 0;
    return ( (CppEditor*)viewManager->currentView() )->paragraphs();
}

void EditorInterfaceImpl::intervalChanged()
{
    if ( !dIface )
	return;
    updateTimer->start( 2000, TRUE );
}

void EditorInterfaceImpl::update()
{
    if ( !dIface )
	return;
    dIface->updateFunctionList();
}

void EditorInterfaceImpl::onBreakPointChange( QObject *receiver, const char *slot )
{
    if ( !viewManager )
	return;
    connect( viewManager, SIGNAL( markersChanged() ), receiver, slot );
}
