/****************************************************************
**
** Definition of PrintPanel class, translation tutorial 3
**
****************************************************************/

#ifndef PRINTPANEL_H
#define PRINTPANEL_H

#include <qvbox.h>

class PrintPanel : public QVBox
{
    Q_OBJECT
public:
    PrintPanel( QWidget *parent = 0, const char *name = 0 );
};

#endif
