/**********************************************************************
** Copyright (C) 2000-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of Qt Configuration.
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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "mainwindowbase.h"


class MainWindow : public MainWindowBase
{
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow();

    void closeEvent(QCloseEvent *);


public slots:
    virtual void buildPalette();
    virtual void buildFont();
    virtual void tunePalette();
    virtual void paletteSelected(int);
    virtual void styleSelected(const QString &);
    virtual void familySelected(const QString &);
    virtual void substituteSelected(const QString &);
    virtual void removeSubstitute();
    virtual void addSubstitute();
    virtual void downSubstitute();
    virtual void upSubstitute();
    virtual void removeLibpath();
    virtual void addLibpath();
    virtual void downLibpath();
    virtual void upLibpath();
    virtual void browseLibpath();
    virtual void removeFontpath();
    virtual void addFontpath();
    virtual void downFontpath();
    virtual void upFontpath();
    virtual void browseFontpath();
    virtual void fileSave();
    virtual void fileExit();
    virtual void somethingModified();
    virtual void helpAbout();
    virtual void helpAboutQt();
    virtual void pageChanged(QWidget *);


private:
    void buildActive();
    void buildActiveEffect();
    void buildInactive();
    void buildInactiveEffect();
    void buildDisabled();
    void buildDisabledEffect();

    void updateColorButtons();
    void updateFontSample();

    void setPreviewPalette(const QPalette &);

    void setModified(bool);

    QPalette editPalette, previewPalette;
    QStyle *previewstyle;
    QStringList fontpaths;
    bool modified;
};


#endif // MAINWINDOW_H
