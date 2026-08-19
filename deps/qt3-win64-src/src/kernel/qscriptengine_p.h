/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSCRIPTENGINE_P_H
#define QSCRIPTENGINE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qtextengine_p.h"

class QString;
struct QCharAttributes;

struct QShaperItem {
    int script;
    const QString *string;
    int from;
    int length;
    QFontEngine *font;
    glyph_t *glyphs;
    advance_t *advances;
    qoffset_t *offsets;
    GlyphAttributes *attributes;
    int num_glyphs; // in: available glyphs out: glyphs used/needed
    unsigned short *log_clusters;
    int flags;
    bool has_positioning;
};

// return true if ok.
typedef bool (*ShapeFunction)(QShaperItem *item);
typedef void (*AttributeFunction)(int script, const QString &, int, int, QCharAttributes *);

struct q_scriptEngine {
    ShapeFunction shape;
    AttributeFunction charAttributes;
};

extern const q_scriptEngine scriptEngines[];

#endif // QSCRIPTENGINE_P_H
