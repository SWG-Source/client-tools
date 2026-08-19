/****************************************************************************
** $Id: qpicture.h 2051 2007-02-21 10:04:20Z chehrlic $
**
** Definition of QPicture class
**
** Created : 940729
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech ASA of Norway and appearing in the file
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

#ifndef QPICTURE_H
#define QPICTURE_H

#ifndef QT_H
#include "qpaintdevice.h"
#include "qbuffer.h"
#endif // QT_H

#ifndef QT_NO_PICTURE

class Q_EXPORT QPicture : public QPaintDevice		// picture class
{
public:
    QPicture( int formatVersion = -1 );
    QPicture( const QPicture & );
   ~QPicture();

    bool	isNull() const;

    uint	size() const;
    const char* data() const;
    virtual void setData( const char* data, uint size );

    bool	play( QPainter * );

    bool	load( QIODevice *dev, const char *format = 0 );
    bool	load( const QString &fileName, const char *format = 0 );
    bool	save( QIODevice *dev, const char *format = 0 );
    bool	save( const QString &fileName, const char *format = 0 );

    QRect boundingRect() const;
    void setBoundingRect( const QRect &r );

    QPicture& operator= (const QPicture&);

    friend Q_EXPORT QDataStream &operator<<( QDataStream &, const QPicture & );
    friend Q_EXPORT QDataStream &operator>>( QDataStream &, QPicture & );

protected:
    bool	cmd( int, QPainter *, QPDevCmdParam * );
    int		metric( int ) const;
    void	detach();
    QPicture	copy() const;

private:
    bool	exec( QPainter *, QDataStream &, int );

    struct QPicturePrivate : public QShared {
	bool	cmd( int, QPainter *, QPDevCmdParam * );
	bool	checkFormat();
	void	resetFormat();

	QBuffer	pictb;
	int	trecs;
	bool	formatOk;
	int	formatMajor;
	int	formatMinor;
	QRect	brect;
    } *d;
};


inline bool QPicture::isNull() const
{
    return d->pictb.buffer().isNull();
}

inline uint QPicture::size() const
{
    return d->pictb.buffer().size();
}

inline const char* QPicture::data() const
{
    return d->pictb.buffer().data();
}

/*****************************************************************************
  QPicture stream functions
 *****************************************************************************/

Q_EXPORT QDataStream &operator<<( QDataStream &, const QPicture & );
Q_EXPORT QDataStream &operator>>( QDataStream &, QPicture & );

#endif // QT_NO_PICTURE

#endif // QPICTURE_H
