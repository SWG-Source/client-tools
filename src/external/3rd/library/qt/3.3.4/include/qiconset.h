/****************************************************************************
** $Id: qt/qiconset.h   3.3.4   edited Jul 22 2003 $
**
** Definition of QIconSet class
**
** Created : 980318
**
** Copyright (C) 1992-2001 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QICONSET_H
#define QICONSET_H

#ifndef QT_H
#include "qobject.h"
#include "qpixmap.h"
#endif // QT_H

#ifndef QT_NO_ICONSET

class QIconFactory;
class QIconSetPrivate;

// ### Remove all 'virtual' functions in QIconSet (but not QIconFactory) in Qt 4.0
class Q_EXPORT QIconSet
{
public:
    // the implementation makes assumptions about the value of these
    enum Size { Automatic, Small, Large };
    enum Mode { Normal, Disabled, Active };
    enum State { On, Off };

    QIconSet();
    QIconSet( const QPixmap& pixmap, Size size = Automatic );
    QIconSet( const QPixmap& smallPix, const QPixmap& largePix );
    QIconSet( const QIconSet& other );
    virtual ~QIconSet();

    void reset( const QPixmap& pixmap, Size size );

    virtual void setPixmap( const QPixmap& pixmap, Size size,
			    Mode mode = Normal, State state = Off );
    virtual void setPixmap( const QString& fileName, Size size,
			    Mode mode = Normal, State state = Off );
    QPixmap pixmap( Size size, Mode mode, State state = Off ) const;
    QPixmap pixmap( Size size, bool enabled, State state = Off ) const;
    QPixmap pixmap() const;
    bool isGenerated( Size size, Mode mode, State state = Off ) const;
    void clearGenerated();
    void installIconFactory( QIconFactory *factory );

    bool isNull() const;

    void detach();

    QIconSet& operator=( const QIconSet& other );

    // static functions
    static void setIconSize( Size which, const QSize& size );
    static const QSize& iconSize( Size which );

#ifndef Q_QDOC
    Q_DUMMY_COMPARISON_OPERATOR(QIconSet)
#endif

private:
    void normalize( Size& which, const QSize& pixSize );
    QPixmap *createScaled( Size size, const QPixmap *suppliedPix ) const;
    QPixmap *createDisabled( Size size, State state ) const;

    QIconSetPrivate *d;
};

class Q_EXPORT QIconFactory : private QShared
{
public:
    QIconFactory();
    virtual ~QIconFactory();

    virtual QPixmap *createPixmap( const QIconSet& iconSet, QIconSet::Size size,
				   QIconSet::Mode mode, QIconSet::State state );
    void setAutoDelete( bool autoDelete ) { autoDel = autoDelete; }
    bool autoDelete() const { return autoDel; }

    static QIconFactory *defaultFactory();
    static void installDefaultFactory( QIconFactory *factory );

private:
#if defined(Q_DISABLE_COPY)
    QIconFactory( const QIconFactory & );
    QIconFactory &operator=( const QIconFactory & );
#endif

    friend class QIconSet;
    friend class QIconSetPrivate;

    uint autoDel : 1;
    uint unused : 31;
};

#endif // QT_NO_ICONSET
#endif
