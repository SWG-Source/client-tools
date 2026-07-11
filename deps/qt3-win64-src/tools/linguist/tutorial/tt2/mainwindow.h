/****************************************************************
**
** Definition of MainWindow class, translation tutorial 2
**
****************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <qmainwindow.h>

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow( QWidget *parent = 0, const char *name = 0 );
};

#endif
