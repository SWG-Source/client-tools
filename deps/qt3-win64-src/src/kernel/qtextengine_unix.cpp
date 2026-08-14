/****************************************************************************
** $Id: qtextengine_unix.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Text engine classes
**
** Copyright (C) 2005-2007 Trolltech ASA.  All rights reserved.
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
** licenses for Unix/X11 or for Qt/Embedded may use this file in accordance
** with the Qt Commercial License Agreement provided with the Software.
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

#include <assert.h>


QScriptItemArray::~QScriptItemArray()
{
    clear();
    free( d );
}

void QScriptItemArray::clear()
{
    if ( d ) {
	for ( unsigned int i = 0; i < d->size; i++ ) {
	    QScriptItem &si = d->items[i];
	    if ( si.fontEngine )
		si.fontEngine->deref();
	}
	d->size = 0;
    }
}

void QScriptItemArray::resize( int s )
{
    int alloc = (s + 8) >> 3 << 3;
    d = (QScriptItemArrayPrivate *)realloc( d, sizeof( QScriptItemArrayPrivate ) +
		 sizeof( QScriptItem ) * alloc );
    d->alloc = alloc;
}

void QTextEngine::shape( int item ) const
{
    assert( item < items.size() );
    QScriptItem &si = items[item];

    if ( si.num_glyphs )
	return;

    QFont::Script script = (QFont::Script)si.analysis.script;
    si.glyph_data_offset = used;

    if ( !si.fontEngine )
	si.fontEngine = fnt->engineForScript( script );
    si.fontEngine->ref();

	si.ascent = si.fontEngine->ascent();
	si.descent = si.fontEngine->descent();
    si.num_glyphs = 0;

    if ( si.fontEngine && si.fontEngine != (QFontEngine*)-1 ) {
        QShaperItem shaper_item;
        shaper_item.script = si.analysis.script;
        shaper_item.string = &string;
        shaper_item.from = si.position;
        shaper_item.length = length(item);
        shaper_item.font = si.fontEngine;
        shaper_item.num_glyphs = QMAX(int(num_glyphs - used), shaper_item.length);
        shaper_item.flags = si.analysis.bidiLevel % 2 ? RightToLeft : 0;
        shaper_item.has_positioning = FALSE;

        while (1) {
//         qDebug("    . num_glyphs=%d, used=%d, item.num_glyphs=%d", num_glyphs, used, shaper_item.num_glyphs);
            ensureSpace(shaper_item.num_glyphs);
            shaper_item.num_glyphs = num_glyphs - used;
//          qDebug("    .. num_glyphs=%d, used=%d, item.num_glyphs=%d", num_glyphs, used, shaper_item.num_glyphs);
            shaper_item.glyphs = glyphs(&si);
            shaper_item.advances = advances(&si);
            shaper_item.offsets = offsets(&si);
            shaper_item.attributes = glyphAttributes(&si);
            shaper_item.log_clusters = logClusters(&si);
            if (scriptEngines[shaper_item.script].shape(&shaper_item))
                break;
        }

        si.num_glyphs = shaper_item.num_glyphs;
        si.hasPositioning = shaper_item.has_positioning;
    }
    ((QTextEngine *)this)->used += si.num_glyphs;

    si.width = 0;
    advance_t *advances = this->advances( &si );
    advance_t *end = advances + si.num_glyphs;
    while ( advances < end ) {
//         qDebug("advances[%d] = %d", advances - this->advances(&si), *advances);
	si.width += *(advances++);
    }

    return;
}

