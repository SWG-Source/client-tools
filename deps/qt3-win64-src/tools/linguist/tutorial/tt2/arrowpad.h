/****************************************************************
**
** Definition of ArrowPad class, translation tutorial 2
**
****************************************************************/

#ifndef ARROWPAD_H
#define ARROWPAD_H

#include <qgrid.h>

class ArrowPad : public QGrid
{
    Q_OBJECT
public:
    ArrowPad( QWidget *parent = 0, const char *name = 0 );

private:
    void skip();
};

#endif
