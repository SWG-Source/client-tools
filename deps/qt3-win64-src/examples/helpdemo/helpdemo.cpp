#include <qassistantclient.h>
#include <qmessagebox.h>
#include <qlineedit.h>
#include <qaction.h>
#include <qpopupmenu.h>
#include <qcheckbox.h>
#include <qprocess.h>
#include <qpushbutton.h>
#include <qdir.h>

#include "helpdemo.h"

HelpDemo::HelpDemo( QWidget *parent, const char *name )
    : HelpDemoBase( parent, name )
{
    leFileName->setText( "./doc/index.html" );
    assistant = new QAssistantClient( QDir( "../../bin" ).absPath(), this );
    widgets.insert( (QWidget*)openQAButton, "./doc/manual.html#openqabutton" );
    widgets.insert( (QWidget*)closeQAButton, "./doc/manual.html#closeqabutton" );
    widgets.insert( (QWidget*)checkOnlyExampleDoc, "./doc/manual.html#onlydoc" );
    widgets.insert( (QWidget*)checkHide, "./doc/manual.html#hide" );
    widgets.insert( (QWidget*)leFileName, "./doc/manual.html#lineedit" );
    widgets.insert( (QWidget*)displayButton, "./doc/manual.html#displaybutton" );
    widgets.insert( (QWidget*)closeButton, "./doc/manual.html#closebutton" );

    menu = new QPopupMenu( this );

    QAction *helpAction = new QAction( "Show Help", QKeySequence(tr("F1")), this );
    helpAction->addTo( menu );

    connect( helpAction, SIGNAL(activated()), this, SLOT(showHelp()) );
    connect( assistant, SIGNAL(assistantOpened()), this, SLOT(assistantOpened()) );
    connect( assistant, SIGNAL(assistantClosed()), this, SLOT(assistantClosed()));
    connect( assistant, SIGNAL(error(const QString&)),
	     this, SLOT(showAssistantErrors(const QString&)) );
    closeQAButton->setEnabled(FALSE);
}

HelpDemo::~HelpDemo()
{
}

void HelpDemo::contextMenuEvent( QContextMenuEvent *e )
{
    QWidget *w = lookForWidget();
    if ( menu->exec( e->globalPos() ) != -1 )
	showHelp( w );
}

QWidget* HelpDemo::lookForWidget()
{
    QPtrDictIterator<char> it( widgets );
    QWidget *w;
    while ( (w = (QWidget*)(it.currentKey())) != 0 ) {
	++it;
	if ( w->hasMouse() )
	    return w;
    }
    return 0;
}

void HelpDemo::showHelp()
{
    showHelp( lookForWidget() );  
}

void HelpDemo::showHelp( QWidget *w )
{
    if ( w )
	assistant->showPage( QString( widgets[w] ) );
    else
	assistant->showPage( "./doc/index.html" );
}

void HelpDemo::setAssistantArguments()
{
    QStringList cmdLst;
    if ( checkHide->isChecked() )
	cmdLst << "-hideSidebar";
    if ( checkOnlyExampleDoc->isChecked() )
        cmdLst << "-profile"
	       << QString("doc") + QDir::separator() + QString("helpdemo.adp");
    assistant->setArguments( cmdLst );
}

void HelpDemo::openAssistant()
{
    if ( !assistant->isOpen() )
	assistant->openAssistant();
}

void HelpDemo::closeAssistant()
{
    if ( assistant->isOpen() )
	assistant->closeAssistant();
}

void HelpDemo::displayPage()
{
    assistant->showPage( leFileName->text() );
}

void HelpDemo::showAssistantErrors( const QString &err )
{
    QMessageBox::critical( this, "Assistant Error", err );

}

void HelpDemo::assistantOpened()
{
    closeQAButton->setEnabled( TRUE );
    openQAButton->setEnabled( FALSE );
}

void HelpDemo::assistantClosed()
{
    closeQAButton->setEnabled( FALSE );
    openQAButton->setEnabled( TRUE );
}
