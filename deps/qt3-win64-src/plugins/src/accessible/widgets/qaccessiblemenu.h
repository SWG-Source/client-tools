#ifndef QACCESSIBLEMENU_H
#define QACCESSIBLEMENU_H

#include "qaccessiblewidget.h"

class QPopupMenu;
class QMenuBar;

class QAccessiblePopup : public QAccessibleWidget
{
public:
    QAccessiblePopup( QObject *o );

    int		childCount() const;
    QRESULT	queryChild( int control, QAccessibleInterface ** ) const;

    QRect	rect( int control ) const;
    int		controlAt( int x, int y ) const;
    int		navigate( NavDirection direction, int control ) const;

    QString	text( Text t, int control ) const;
    Role	role( int control ) const;
    State	state( int control ) const;

    bool	doDefaultAction( int control );
    bool	setFocus( int control );

protected:
    QPopupMenu *popupMenu() const;
};

class QAccessibleMenuBar : public QAccessibleWidget
{
public:
    QAccessibleMenuBar( QObject *o );

    int		childCount() const;
    QRESULT	queryChild( int control, QAccessibleInterface ** ) const;

    QRect	rect( int control ) const;
    int		controlAt( int x, int y ) const;
    int		navigate( NavDirection direction, int control ) const;

    QString	text( Text t, int control ) const;
    Role	role( int control ) const;
    State	state( int control ) const;

    bool	doDefaultAction( int control );
    bool	setFocus( int control );

protected:
    QMenuBar *menuBar() const;
};

#endif // QACCESSIBLEMENU_H
