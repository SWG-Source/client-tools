/**********************************************************************
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of Qt/Embedded virtual framebuffer.
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

class QVFbView;
class QVFbRateDialog;
class QPopupMenu;
class QMenuData;
class QFileDialog;
class Config;

class QVFb: public QMainWindow
{
    Q_OBJECT
public:
    QVFb( int display_id, int w, int h, int d, const QString &skin, QWidget *parent = 0,
		const char *name = 0, uint wflags = 0 );
    ~QVFb();

    void enableCursor( bool e );
    void popupMenu();

protected slots:
    void saveImage();
    void toggleAnimation();
    void toggleCursor();
    void changeRate();
    void about();
    void aboutQt();

    void configure();

    void setZoom(double);
    void setZoom1();
    void setZoom2();
    void setZoom3();
    void setZoom4();
    void setZoomHalf();

protected:
    void createMenu(QMenuData *menu);

private:
    void init( int display_id, int w, int h, int d, const QString &skin );
    QVFbView *view;
    QVFbRateDialog *rateDlg;
    QFileDialog* imagesave;
    QPopupMenu *viewMenu;
    int cursorId;
    Config* config;
    QString currentSkin;

private slots:
    void setGamma400(int n);
    void setR400(int n);
    void setG400(int n);
    void setB400(int n);
    void updateGammaLabels();
};

