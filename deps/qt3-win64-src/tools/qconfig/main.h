/**********************************************************************
** Copyright (C) 1999-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of Qt/Embedded.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include <qmainwindow.h>
#include <qmap.h>
#include <qstringlist.h>
#include <qtextbrowser.h>

class ChoiceItem;
class QListViewItem;
class QListView;
class QLabel;

class Info : public QTextBrowser {
    Q_OBJECT
public:
    Info( QWidget* parent, const char* name=0 );

signals:
    void idClicked(const QString& name);

public slots:
    void setSource(const QString& name);
};

class Main : public QMainWindow {
    Q_OBJECT
public:
    Main();
    void loadFeatures(const QString& filename);
    void loadConfig(const QString& filename);

private:
    void createItem(const QString& ch);
    QMap<QString,QString> label;
    QMap<QString,QString> documentation;
    QMap<QString,QStringList> links;
    QMap<QString,QStringList> dependencies;
    QMap<QString,QStringList> rdependencies;
    QMap<QString,QString> section;
    QMap<QString,ChoiceItem*> item;
    QMap<QString,QListViewItem*> sectionitem;
    QStringList choices;
    QListView* lv;
    QTextBrowser* info;

private slots:
    void updateAvailability(QListViewItem* i);
    void showInfo(QListViewItem* i);
    void selectId(const QString&);
    void open();
    void save();
    void testAll();
    void about();
    void aboutQt();
};
