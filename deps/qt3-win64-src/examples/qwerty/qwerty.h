/****************************************************************************
** $Id: qwerty.h 2051 2007-02-21 10:04:20Z chehrlic $
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef QWERTY_H
#define QWERTY_H

#include <qwidget.h>
#include <qmenubar.h>
#include <qmultilineedit.h>
#include <qprinter.h>

class Editor : public QWidget
{
    Q_OBJECT
public:
    Editor( QWidget *parent=0, const char *name="qwerty" );
   ~Editor();

    void load( const QString& fileName, int code=-1 );

public slots:
    void newDoc();
    void load();
    bool save();
    void print();
    void addEncoding();
    void toUpper();
    void toLower();
    void font();
protected:
    void resizeEvent( QResizeEvent * );
    void closeEvent( QCloseEvent * );

private slots:
    void saveAsEncoding( int );
    void openAsEncoding( int );
    void textChanged();

private:
    bool saveAs( const QString& fileName, int code=-1 );
    void rebuildCodecList();
    QMenuBar 	   *m;
    QMultiLineEdit *e;
#ifndef QT_NO_PRINTER
    QPrinter        printer;
#endif
    QPopupMenu	   *save_as;
    QPopupMenu	   *open_as;
    bool changed;
};

#endif // QWERTY_H
