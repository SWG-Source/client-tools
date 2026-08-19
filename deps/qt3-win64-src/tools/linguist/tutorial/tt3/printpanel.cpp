/****************************************************************
**
** Implementation of PrintPanel class, translation tutorial 3
**
****************************************************************/

#include "printpanel.h"

#include <qlabel.h>
#include <qradiobutton.h>
#include <qhbuttongroup.h>

PrintPanel::PrintPanel( QWidget *parent, const char *name )
    : QVBox( parent, name )
{
    setMargin( 10 );
    setSpacing( 10 );

/*
    QLabel *lab = new QLabel( tr("<b>TROLL PRINT</b>"), this );
    lab->setAlignment( AlignCenter );
*/

    QRadioButton *but;

    QHButtonGroup *twoSided = new QHButtonGroup( this );
    twoSided->setTitle( tr("2-sided") );
    but = new QRadioButton( tr("Enabled"), twoSided );
    but = new QRadioButton( tr("Disabled"), twoSided );
    but->toggle();

    QHButtonGroup *colors = new QHButtonGroup( this );
    colors->setTitle( tr("Colors") );
    but = new QRadioButton( tr("Enabled"), colors );
    but = new QRadioButton( tr("Disabled"), colors );
    but->toggle();
}
