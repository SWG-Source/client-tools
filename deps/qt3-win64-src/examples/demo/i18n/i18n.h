#ifndef I18N_H
#define I18N_H

#include <qmainwindow.h>

class QWorkspace;
class QAction;
class QPopupMenu;
class Wrapper;


class I18nDemo : public QMainWindow
{
    Q_OBJECT

public:
    I18nDemo(QWidget *, const char * = 0);
    ~I18nDemo();

    void initActions();
    void initMenuBar();

    void showEvent(QShowEvent *);
    void hideEvent(QHideEvent *);

    QWorkspace *workspace;
    QAction *actionClose, *actionCloseAll, *actionTile, *actionCascade;
    QPopupMenu *windowMenu, *newMenu;
    Wrapper *lastwrapper;


public slots:
    void newSlot(int);
    void windowSlot(int);
    void windowActivated(QWidget *);
    void closeSlot();
    void closeAllSlot();
    void tileSlot();
    void cascadeSlot();
    void wrapperDead();
};


#endif // I18N_H
