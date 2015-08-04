/****************************************************************************
** $Id: qt/qfontinfo.h   3.3.4   edited May 27 2003 $
**
** Definition of QFontInfo class
**
** Created : 950131
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
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

#ifndef QFONTINFO_H
#define QFONTINFO_H

#ifndef QT_H
#include "qfont.h"
#endif // QT_H


class Q_EXPORT QFontInfo
{
public:
    QFontInfo( const QFont & );
    QFontInfo( const QFont &, QFont::Script );
    QFontInfo( const QFontInfo & );
    ~QFontInfo();

    QFontInfo	       &operator=( const QFontInfo & );

    QString   	        family()	const;
    int			pixelSize()	const;
    int			pointSize()	const;
    bool		italic()	const;
    int			weight()	const;
    bool		bold()		const;
    bool		underline()	const;
    bool                overline()      const;
    bool		strikeOut()	const;
    bool		fixedPitch()	const;
    QFont::StyleHint	styleHint()	const;
    bool		rawMode()	const;

    bool		exactMatch()	const;


private:
    QFontInfo( const QPainter * );

    QFontPrivate *d;
    QPainter *painter;
    int fscript;

    friend class QWidget;
    friend class QPainter;
};


inline bool QFontInfo::bold() const
{ return weight() > QFont::Normal; }


#endif // QFONTINFO_H
