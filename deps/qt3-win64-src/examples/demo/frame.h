/****************************************************************************
** $Id: frame.h 2051 2007-02-21 10:04:20Z chehrlic $
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qmainwindow.h>
#include <qintdict.h>
#include "categoryinterface.h"

class QToolBox;
class QStyle;
class QWidgetStack;

class Frame : public QMainWindow
{
    Q_OBJECT

public:
    Frame( QWidget *parent=0, const char *name=0 );
    void setCategories( const QPtrList<CategoryInterface> &l );

    static void updateTranslators();

    QWidgetStack *widgetStack() const { return stack; }

private slots:
    void setStyle( const QString& );

protected:
    bool event( QEvent *e );

private:
    QWidget *createCategoryPage( CategoryInterface *c );

private:
    QToolBox *toolBox;
    QWidgetStack *stack;
    QIntDict<QWidget> categoryPages;
    QPtrList<CategoryInterface> categories;

};
