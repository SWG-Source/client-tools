/****************************************************************************
** $Id: wizard.h 2051 2007-02-21 10:04:20Z chehrlic $
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef WIZARD_H
#define WIZARD_H

#include <qwizard.h>

class QWidget;
class QHBox;
class QLineEdit;
class QLabel;

class Wizard : public QWizard
{
    Q_OBJECT

public:
    Wizard( QWidget *parent = 0, const char *name = 0 );

    void showPage(QWidget* page);

protected:
    void setupPage1();
    void setupPage2();
    void setupPage3();

    QHBox *page1, *page2, *page3;
    QLineEdit *key, *firstName, *lastName, *address, *phone, *email;
    QLabel *lKey, *lFirstName, *lLastName, *lAddress, *lPhone, *lEmail;

protected slots:
    void keyChanged( const QString & );
    void dataChanged( const QString & );

};

#endif
