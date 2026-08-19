/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions use Qt Designer which will
** update this file, preserving your code. Create an init() function in place
** of a constructor, and a destroy() function in place of a destructor.
*****************************************************************************/

void DialogForm::init()
{
    extensionShown = FALSE;
    setExtension( new Extension( this ) );
    setOrientation( Vertical );
}


void DialogForm::toggleDetails()
{
    extensionShown = !extensionShown;
    showExtension( extensionShown );
    QString text = tr( "&Details " );
    text += QString( extensionShown ? "<<<" : ">>>" );
    detailsPushButton->setText( text );
}
