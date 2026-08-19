/****************************************************************
**
** Implementation of ArrowPad class, translation tutorial 2
**
****************************************************************/

#include "arrowpad.h"

#include <qpushbutton.h>

ArrowPad::ArrowPad( QWidget *parent, const char *name )
    : QGrid( 3, Horizontal, parent, name )
{
    setMargin( 10 );
    setSpacing( 10 );

    skip();
    (void) new QPushButton( tr("&Up"), this );
    skip();
    (void) new QPushButton( tr("&Left"), this );
    skip();
    (void) new QPushButton( tr("&Right"), this );
    skip();
    (void) new QPushButton( tr("&Down"), this );
    skip();
}

void ArrowPad::skip()
{
    (void) new QWidget( this );
}
