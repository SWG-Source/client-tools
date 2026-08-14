/****************************************************************************
** $Id: qscriptengine_x11.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Copyright (C) 2003-2007 Trolltech ASA.  All rights reserved.
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

// ------------------------------------------------------------------------------------------------------------------
//
// Continuation of middle eastern languages
//
// ------------------------------------------------------------------------------------------------------------------

// #### stil missing: identify invalid character combinations
static bool syriac_shape(QShaperItem *item)
{
    Q_ASSERT(item->script == QFont::Syriac);

#ifndef QT_NO_XFTFREETYPE
    QOpenType *openType = item->font->openType();
    if (openType && openType->supportsScript(QFont::Syriac)) {
        bool ot_ok;
        if (arabicSyriacOpenTypeShape(openType, item, &ot_ok))
            return true;
        if (ot_ok)
            return false;
        // fall through to the non OT code
    }
#endif
    return basic_shape(item);
}


static bool thaana_shape(QShaperItem *item)
{
    Q_ASSERT(item->script == QFont::Thaana);

#ifndef QT_NO_XFTFREETYPE
    QOpenType *openType = item->font->openType();

    if (openType && openType->supportsScript(item->script)) {
        openType->selectScript(QFont::Thaana);
        if (item->font->stringToCMap(item->string->unicode()+item->from, item->length, item->glyphs, item->advances,
                                     &item->num_glyphs, item->flags & QTextEngine::RightToLeft) != QFontEngine::NoError)
            return FALSE;
        heuristicSetGlyphAttributes(item);
        openType->shape(item);
        return openType->positionAndAdd(item);
    }
#endif
    return basic_shape(item);
}

// --------------------------------------------------------------------------------------------------------------------------------------------
//
// Indic languages
//
// --------------------------------------------------------------------------------------------------------------------------------------------

enum Form {
    Invalid = 0x0,
    Unknown = Invalid,
    Consonant,
    Nukta,
    Halant,
    Matra,
    VowelMark,
    StressMark,
    IndependentVowel,
    LengthMark,
    Control,
    Other
};

static const unsigned char indicForms[0xe00-0x900] = {
    // Devangari
    Invalid, VowelMark, VowelMark, VowelMark,
    Invalid, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, IndependentVowel, IndependentVowel, IndependentVowel,

    IndependentVowel, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Unknown, Unknown,
    Nukta, Other, Matra, Matra,

    Matra, Matra, Matra, Matra,
    Matra, Matra, Matra, Matra,
    Matra, Matra, Matra, Matra,
    Matra, Halant, Unknown, Unknown,

    Other, StressMark, StressMark, StressMark,
    StressMark, Unknown, Unknown, Unknown,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    IndependentVowel, IndependentVowel, VowelMark, VowelMark,
    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    // Bengali
    Invalid, VowelMark, VowelMark, VowelMark,
    Invalid, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, Invalid, Invalid, IndependentVowel,

    IndependentVowel, Invalid, Invalid, IndependentVowel,
    IndependentVowel, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Invalid, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Invalid, Consonant, Invalid,
    Invalid, Invalid, Consonant, Consonant,
    Consonant, Consonant, Unknown, Unknown,
    Nukta, Other, Matra, Matra,

    Matra, Matra, Matra, Matra,
    Matra, Invalid, Invalid, Matra,
    Matra, Invalid, Invalid, Matra,
    Matra, Halant, Consonant, Unknown,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, VowelMark,
    Invalid, Invalid, Invalid, Invalid,
    Consonant, Consonant, Invalid, Consonant,

    IndependentVowel, IndependentVowel, VowelMark, VowelMark,
    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    Consonant, Consonant, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    // Gurmukhi
    Invalid, Invalid, VowelMark, Invalid,
    Invalid, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, IndependentVowel, IndependentVowel, Invalid,
    Invalid, Invalid, Invalid, IndependentVowel,

    IndependentVowel, Invalid, Invalid, IndependentVowel,
    IndependentVowel, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Invalid, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Invalid, Consonant, Consonant,
    Invalid, Consonant, Consonant, Invalid,
    Consonant, Consonant, Unknown, Unknown,
    Nukta, Other, Matra, Matra,

    Matra, Matra, Matra, Invalid,
    Invalid, Invalid, Invalid, Matra,
    Matra, Invalid, Invalid, Matra,
    Matra, Halant, Unknown, Unknown,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Unknown, Unknown, Unknown,
    Invalid, Consonant, Consonant, Consonant,
    Consonant, Invalid, Consonant, Invalid,

    Other, Other, Invalid, Invalid,
    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    StressMark, StressMark, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    // Gujarati
    Invalid, VowelMark, VowelMark, VowelMark,
    Invalid, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, IndependentVowel, IndependentVowel, IndependentVowel,
    Invalid, IndependentVowel, Invalid, IndependentVowel,

    IndependentVowel, IndependentVowel, Invalid, IndependentVowel,
    IndependentVowel, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Invalid, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Invalid, Consonant, Consonant,
    Invalid, Consonant, Consonant, Consonant,
    Consonant, Consonant, Unknown, Unknown,
    Nukta, Other, Matra, Matra,

    Matra, Matra, Matra, Matra,
    Matra, Matra, Invalid, Matra,
    Matra, Matra, Invalid, Matra,
    Matra, Halant, Unknown, Unknown,

    Other, Unknown, Unknown, Unknown,
    Unknown, Unknown, Unknown, Unknown,
    Unknown, Unknown, Unknown, Unknown,
    Unknown, Unknown, Unknown, Unknown,

    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    // Oriya
    Invalid, VowelMark, VowelMark, VowelMark,
    Invalid, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, Invalid, Invalid, IndependentVowel,

    IndependentVowel, Invalid, Invalid, IndependentVowel,
    IndependentVowel, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Invalid, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Invalid, Consonant, Consonant,
    Invalid, Invalid, Consonant, Consonant,
    Consonant, Consonant, Unknown, Unknown,
    Nukta, Other, Matra, Matra,

    Matra, Matra, Matra, Matra,
    Invalid, Invalid, Invalid, Matra,
    Matra, Invalid, Invalid, Matra,
    Matra, Halant, Unknown, Unknown,

    Other, Invalid, Invalid, Invalid,
    Invalid, Unknown, LengthMark, LengthMark,
    Invalid, Invalid, Invalid, Invalid,
    Consonant, Consonant, Invalid, Consonant,

    IndependentVowel, IndependentVowel, Invalid, Invalid,
    Invalid, Invalid, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    //Tamil
    Invalid, Invalid, VowelMark, Other,
    Invalid, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, IndependentVowel, IndependentVowel, Invalid,
    Invalid, Invalid, IndependentVowel, IndependentVowel,

    IndependentVowel, Invalid, IndependentVowel, IndependentVowel,
    IndependentVowel, Consonant, Invalid, Invalid,
    Invalid, Consonant, Consonant, Invalid,
    Consonant, Invalid, Consonant, Consonant,

    Invalid, Invalid, Invalid, Consonant,
    Consonant, Invalid, Invalid, Invalid,
    Consonant, Consonant, Consonant, Invalid,
    Invalid, Invalid, Consonant, Consonant,

    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Invalid, Consonant,
    Consonant, Consonant, Unknown, Unknown,
    Invalid, Invalid, Matra, Matra,

    Matra, Matra, Matra, Invalid,
    Invalid, Invalid, Matra, Matra,
    Matra, Invalid, Matra, Matra,
    Matra, Halant, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, LengthMark,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    // Telugu
    Invalid, VowelMark, VowelMark, VowelMark,
    Invalid, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, Invalid, IndependentVowel, IndependentVowel,

    IndependentVowel, Invalid, IndependentVowel, IndependentVowel,
    IndependentVowel, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Invalid, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Consonant, Consonant, Consonant,
    Invalid, Consonant, Consonant, Consonant,
    Consonant, Consonant, Unknown, Unknown,
    Invalid, Invalid, Matra, Matra,

    Matra, Matra, Matra, Matra,
    Matra, Invalid, Matra, Matra,
    Matra, Invalid, Matra, Matra,
    Matra, Halant, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, LengthMark, Matra, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    IndependentVowel, IndependentVowel, Invalid, Invalid,
    Invalid, Invalid, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    // Kannada
    Invalid, Invalid, VowelMark, VowelMark,
    Invalid, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, Invalid, IndependentVowel, IndependentVowel,

    IndependentVowel, Invalid, IndependentVowel, IndependentVowel,
    IndependentVowel, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Invalid, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Consonant, Consonant, Consonant,
    Invalid, Consonant, Consonant, Consonant,
    Consonant, Consonant, Unknown, Unknown,
    Invalid, Invalid, Matra, Matra,

    Matra, Matra, Matra, Matra,
    Matra, Invalid, Matra, Matra,
    Matra, Invalid, Matra, Matra,
    Matra, Halant, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, LengthMark, LengthMark, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Consonant, Invalid,

    IndependentVowel, IndependentVowel, Invalid, Invalid,
    Invalid, Invalid, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    // Malayalam
    Invalid, Invalid, VowelMark, VowelMark,
    Invalid, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, Invalid, IndependentVowel, IndependentVowel,

    IndependentVowel, Invalid, IndependentVowel, IndependentVowel,
    IndependentVowel, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Invalid, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Unknown, Unknown,
    Invalid, Invalid, Matra, Matra,

    Matra, Matra, Matra, Matra,
    Invalid, Invalid, Matra, Matra,
    Matra, Invalid, Matra, Matra,
    Matra, Halant, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, LengthMark,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    IndependentVowel, IndependentVowel, Invalid, Invalid,
    Invalid, Invalid, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    // Sinhala
    Invalid, Invalid, VowelMark, VowelMark,
    Invalid, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, IndependentVowel, IndependentVowel, IndependentVowel,

    IndependentVowel, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, IndependentVowel, IndependentVowel, Invalid,
    Invalid, Invalid, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Consonant, Invalid, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Invalid, Consonant, Invalid, Invalid,

    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Invalid,
    Invalid, Invalid, Halant, Invalid,
    Invalid, Invalid, Invalid, Matra,

    Matra, Matra, Matra, Matra,
    Matra, Invalid, Matra, Invalid,
    Matra, Matra, Matra, Matra,
    Matra, Matra, Matra, Matra,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    Invalid, Invalid, Matra, Matra,
    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,
};

enum Position {
    None,
    Pre,
    Above,
    Below,
    Post,
    Split,
    Base,
    Reph,
    Vattu,
    Inherit
};

static const unsigned char indicPosition[0xe00-0x900] = {
    // Devanagari
    None, Above, Above, Post,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    Below, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, Post, Pre,

    Post, Below, Below, Below,
    Below, Above, Above, Above,
    Above, Post, Post, Post,
    Post, None, None, None,

    None, Above, Below, Above,
    Above, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, Below, Below,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    // Bengali
    None, Above, Post, Post,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    Below, None, None, Post,

    Below, None, None, None,
    None, None, None, None,
    None, None, None, None,
    Below, None, Post, Pre,

    Post, Below, Below, Below,
    Below, None, None, Pre,
    Pre, None, None, Split,
    Split, Below, None, None,

    None, None, None, None,
    None, None, None, Post,
    None, None, None, None,
    None, None, None, None,

    None, None, Below, Below,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    // Gurmukhi
    None, None, Above, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, Post,

    Below, None, None, None,
    None, Below, None, None,
    None, Below, None, None,
    Below, None, Post, Pre,

    Post, Below, Below, None,
    None, None, None, Above,
    Above, None, None, Above,
    Above, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    Above, Above, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    // Gujarati
    None, Above, Above, Post,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    Below, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, Post, Pre,

    Post, Below, Below, Below,
    Below, Above, None, Above,
    Above, Post, None, Post,
    Post, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    // Oriya
    None, Above, Post, Post,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    Below, None, None, None,
    Below, None, None, None,
    Below, Below, Below, Post,

    Below, None, Below, Below,
    None, None, None, None,
    None, None, None, None,
    None, None, Post, Above,

    Post, Below, Below, Below,
    None, None, None, Pre,
    Split, None, None, Split,
    Split, None, None, None,

    None, None, None, None,
    None, None, Above, Post,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    // Tamil
    None, None, Above, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, Post, Post,

    Above, Below, Below, None,
    None, None, Pre, Pre,
    Pre, None, Split, Split,
    Split, Halant, None, None,

    None, None, None, None,
    None, None, None, Post,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    // Telugu
    None, Post, Post, Post,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, Below, Below, Below,
    Below, Below, Below, Below,
    Below, Below, Below, Below,

    Below, Below, Below, Below,
    Below, Below, Below, Below,
    Below, None, Below, Below,
    Below, Below, Below, Below,

    Below, None, Below, Below,
    None, Below, Below, Below,
    Below, Below, None, None,
    None, None, Post, Above,

    Above, Post, Post, Post,
    Post, None, Above, Above,
    Split, None, Post, Above,
    Above, Halant, None, None,

    None, None, None, None,
    None, Above, Below, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    // Kannada
    None, None, Post, Post,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, Below, Below, Below,
    Below, Below, Below, Below,
    Below, Below, Below, Below,

    Below, Below, Below, Below,
    Below, Below, Below, Below,
    Below, Below, Below, Below,
    Below, Below, Below, Below,

    Below, None, Below, Below,
    None, Below, Below, Below,
    Below, Below, None, None,
    None, None, Post, Above,

    Split, Post, Post, Post,
    Post, None, Above, Split,
    Split, None, Split, Split,
    Above, Halant, None, None,

    None, None, None, None,
    None, Post, Post, None,
    None, None, None, None,
    None, None, Below, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    // Malayalam
    None, None, Post, Post,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, Post,

    Post, None, Below, None,
    None, Post, None, None,
    None, None, None, None,
    None, None, Post, Post,

    Post, Post, Post, Post,
    None, None, Pre, Pre,
    Pre, None, Split, Split,
    Split, Halant, None, None,

    None, None, None, None,
    None, None, None, Post,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    // Sinhala
    None, None, Post, Post,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, Post,

    Post, Post, Above, Above,
    Below, None, Below, None,
    Post, Pre, Split, Pre,
    Split, Split, Split, Post,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, Post, Post,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None
};

static inline Form form(unsigned short uc) {
    if (uc < 0x900 || uc > 0xdff) {
        if (uc == 0x25cc)
            return Consonant;
        if (uc == 0x200c || uc == 0x200d)
            return Control;
        return Other;
    }
    return (Form)indicForms[uc-0x900];
}

static inline Position indic_position(unsigned short uc) {
    if (uc < 0x900 || uc > 0xdff)
        return None;
    return (Position) indicPosition[uc-0x900];
}


enum IndicScriptProperties {
    HasReph = 0x01,
    HasSplit = 0x02
};

const uchar scriptProperties[10] = {
    // Devanagari,
    HasReph,
    // Bengali,
    HasReph|HasSplit,
    // Gurmukhi,
    0,
    // Gujarati,
    HasReph,
    // Oriya,
    HasReph|HasSplit,
    // Tamil,
    HasSplit,
    // Telugu,
    HasSplit,
    // Kannada,
    HasSplit|HasReph,
    // Malayalam,
    HasSplit,
    // Sinhala,
    HasSplit
};

struct IndicOrdering {
    Form form;
    Position position;
};

static const IndicOrdering devanagari_order [] = {
    { Consonant, Below },
    { Matra, Below },
    { VowelMark, Below },
    { StressMark, Below },
    { Matra, Above },
    { Matra, Post },
    { Consonant, Reph },
    { VowelMark, Above },
    { StressMark, Above },
    { VowelMark, Post },
    { (Form)0, None }
};

static const IndicOrdering bengali_order [] = {
    { Consonant, Below },
    { Matra, Below },
    { Matra, Above },
    { Consonant, Reph },
    { VowelMark, Above },
    { Consonant, Post },
    { Matra, Post },
    { VowelMark, Post },
    { (Form)0, None }
};

static const IndicOrdering gurmukhi_order [] = {
    { Consonant, Below },
    { Matra, Below },
    { Matra, Above },
    { Consonant, Post },
    { Matra, Post },
    { VowelMark, Above },
    { (Form)0, None }
};

static const IndicOrdering tamil_order [] = {
    { Matra, Above },
    { Matra, Post },
    { VowelMark, Post },
    { (Form)0, None }
};

static const IndicOrdering telugu_order [] = {
    { Matra, Above },
    { Matra, Below },
    { Matra, Post },
    { Consonant, Below },
    { Consonant, Post },
    { VowelMark, Post },
    { (Form)0, None }
};

static const IndicOrdering kannada_order [] = {
    { Matra, Above },
    { Matra, Post },
    { Consonant, Below },
    { Consonant, Post },
    { LengthMark, Post },
    { Consonant, Reph },
    { VowelMark, Post },
    { (Form)0, None }
};

static const IndicOrdering malayalam_order [] = {
    { Consonant, Below },
    { Matra, Below },
    { Consonant, Reph },
    { Consonant, Post },
    { Matra, Post },
    { VowelMark, Post },
    { (Form)0, None }
};

static const IndicOrdering sinhala_order [] = {
    { Matra, Below },
    { Matra, Above },
    { Matra, Post },
    { VowelMark, Post },
    { (Form)0, None }
};

static const IndicOrdering * const indic_order[] = {
    devanagari_order, // Devanagari
    bengali_order, // Bengali
    gurmukhi_order, // Gurmukhi
    devanagari_order, // Gujarati
    bengali_order, // Oriya
    tamil_order, // Tamil
    telugu_order, // Telugu
    kannada_order, // Kannada
    malayalam_order, // Malayalam
    sinhala_order // Sinhala
};



// vowel matras that have to be split into two parts.
static const unsigned short split_matras[]  = {
    //  matra, split1, split2

    // bengalis
    0x9cb, 0x9c7, 0x9be,
    0x9cc, 0x9c7, 0x9d7,
    // oriya
    0xb48, 0xb47, 0xb56,
    0xb4b, 0xb47, 0xb3e,
    0xb4c, 0xb47, 0xb57,
    // tamil
    0xbca, 0xbc6, 0xbbe,
    0xbcb, 0xbc7, 0xbbe,
    0xbcc, 0xbc6, 0xbd7,
    // telugu
    0xc48, 0xc46, 0xc56,
    // kannada
    0xcc0, 0xcbf, 0xcd5,
    0xcc7, 0xcc6, 0xcd5,
    0xcc8, 0xcc6, 0xcd6,
    0xcca, 0xcc6, 0xcc2,
    0xccb, 0xcca, 0xcd5,
    // malayalam
    0xd4a, 0xd46, 0xd3e,
    0xd4b, 0xd47, 0xd3e,
    0xd4c, 0xd46, 0xd57,
    // sinhala
    0xdda, 0xdd9, 0xdca,
    0xddc, 0xdd9, 0xdcf,
    0xddd, 0xddc, 0xdca,
    0xdde, 0xdd9, 0xddf,
    0xffff
};

static inline void splitMatra(unsigned short *reordered, int matra, int &len, int &base)
{
    unsigned short matra_uc = reordered[matra];
    //qDebug("matra=%d, reordered[matra]=%x", matra, reordered[matra]);

    const unsigned short *split = split_matras;
    while (split[0] < matra_uc)
        split += 3;

    assert(*split == matra_uc);
    ++split;

    if (indic_position(*split) == Pre) {
        reordered[matra] = split[1];
        memmove(reordered + 1, reordered, len*sizeof(unsigned short));
        reordered[0] = split[0];
        base++;
    } else {
        memmove(reordered + matra + 1, reordered + matra, (len-matra)*sizeof(unsigned short));
        reordered[matra] = split[0];
        reordered[matra+1] = split[1];
    }
    len++;
}

enum IndicProperties {
    // these two are already defined
//     CcmpProperty = 0x1,
//     InitProperty = 0x2,
    NuktaProperty = 0x4,
    AkhantProperty = 0x8,
    RephProperty = 0x10,
    PreFormProperty = 0x20,
    BelowFormProperty = 0x40,
    AboveFormProperty = 0x80,
    HalfFormProperty = 0x100,
    PostFormProperty = 0x200,
    VattuProperty = 0x400,
    PreSubstProperty = 0x800,
    BelowSubstProperty = 0x1000,
    AboveSubstProperty = 0x2000,
    PostSubstProperty = 0x4000,
    HalantProperty = 0x8000,
    CligProperty = 0x10000
};

#ifndef QT_NO_XFTFREETYPE
static const QOpenType::Features indic_features[] = {
    { FT_MAKE_TAG('c', 'c', 'm', 'p'), CcmpProperty },
    { FT_MAKE_TAG('i', 'n', 'i', 't'), InitProperty },
    { FT_MAKE_TAG('n', 'u', 'k', 't'), NuktaProperty },
    { FT_MAKE_TAG('a', 'k', 'h', 'n'), AkhantProperty },
    { FT_MAKE_TAG('r', 'p', 'h', 'f'), RephProperty },
    { FT_MAKE_TAG('b', 'l', 'w', 'f'), BelowFormProperty },
    { FT_MAKE_TAG('h', 'a', 'l', 'f'), HalfFormProperty },
    { FT_MAKE_TAG('p', 's', 't', 'f'), PostFormProperty },
    { FT_MAKE_TAG('v', 'a', 't', 'u'), VattuProperty },
    { FT_MAKE_TAG('p', 'r', 'e', 's'), PreSubstProperty },
    { FT_MAKE_TAG('b', 'l', 'w', 's'), BelowSubstProperty },
    { FT_MAKE_TAG('a', 'b', 'v', 's'), AboveSubstProperty },
    { FT_MAKE_TAG('p', 's', 't', 's'), PostSubstProperty },
    { FT_MAKE_TAG('h', 'a', 'l', 'n'), HalantProperty },
    { 0, 0 }
};
#endif

// #define INDIC_DEBUG
#ifdef INDIC_DEBUG
#define IDEBUG qDebug
#else
#define IDEBUG if(0) qDebug
#endif

#ifdef INDIC_DEBUG
static QString propertiesToString(int properties)
{
    QString res;
    properties = ~properties;
    if (properties & CcmpProperty)
        res += "Ccmp ";
    if (properties & InitProperty)
        res += "Init ";
    if (properties & NuktaProperty)
        res += "Nukta ";
    if (properties & AkhantProperty)
        res += "Akhant ";
    if (properties & RephProperty)
        res += "Reph ";
    if (properties & PreFormProperty)
        res += "PreForm ";
    if (properties & BelowFormProperty)
        res += "BelowForm ";
    if (properties & AboveFormProperty)
        res += "AboveForm ";
    if (properties & HalfFormProperty)
        res += "HalfForm ";
    if (properties & PostFormProperty)
        res += "PostForm ";
    if (properties & VattuProperty)
        res += "Vattu ";
    if (properties & PreSubstProperty)
        res += "PreSubst ";
    if (properties & BelowSubstProperty)
        res += "BelowSubst ";
    if (properties & AboveSubstProperty)
        res += "AboveSubst ";
    if (properties & PostSubstProperty)
        res += "PostSubst ";
    if (properties & HalantProperty)
        res += "Halant ";
    if (properties & CligProperty)
        res += "Clig ";
    return res;
}
#endif

static bool indic_shape_syllable(QOpenType *openType, QShaperItem *item, bool invalid)
{
    Q_UNUSED(openType)
    int script = item->script;
    Q_ASSERT(script >= QFont::Devanagari && script <= QFont::Sinhala);
    const unsigned short script_base = 0x0900 + 0x80*(script-QFont::Devanagari);
    const unsigned short ra = script_base + 0x30;
    const unsigned short halant = script_base + 0x4d;
    const unsigned short nukta = script_base + 0x3c;

    int len = item->length;
    IDEBUG(">>>>> indic shape: from=%d, len=%d invalid=%d", item->from, item->length, invalid);

    if (item->num_glyphs < len+4) {
        item->num_glyphs = len+4;
        return FALSE;
    }

    QVarLengthArray<unsigned short> reordered(len+4);
    QVarLengthArray<unsigned char> position(len+4);

    unsigned char properties = scriptProperties[script-QFont::Devanagari];

    if (invalid) {
        *reordered.data() = 0x25cc;
        memcpy(reordered.data()+1, item->string->unicode() + item->from, len*sizeof(QChar));
        len++;
    } else {
        memcpy(reordered.data(), item->string->unicode() + item->from, len*sizeof(QChar));
    }
    if (reordered[len-1] == 0x200c) // zero width non joiner
        len--;

    int i;
    int base = 0;
    int reph = -1;

#ifdef INDIC_DEBUG
    IDEBUG("original:");
    for (i = 0; i < len; i++) {
        IDEBUG("    %d: %4x", i, reordered[i]);
    }
#endif

    if (len != 1) {
        unsigned short *uc = reordered.data();
        bool beginsWithRa = FALSE;

        // Rule 1: find base consonant
        //
        // The shaping engine finds the base consonant of the
        // syllable, using the following algorithm: starting from the
        // end of the syllable, move backwards until a consonant is
        // found that does not have a below-base or post-base form
        // (post-base forms have to follow below-base forms), or
        // arrive at the first consonant. The consonant stopped at
        // will be the base.
        //
        //  * If the syllable starts with Ra + H (in a script that has
        //    'Reph'), Ra is excluded from candidates for base
        //    consonants.
        //
        // * In Kannada and Telugu, the base consonant cannot be
        //   farther than 3 consonants from the end of the syllable.
        // #### replace the HasReph property by testing if the feature exists in the font!
        if (form(*uc) == Consonant || (script == QFont::Bengali && form(*uc) == IndependentVowel)) {
            beginsWithRa = (properties & HasReph) && ((len > 2) && *uc == ra && *(uc+1) == halant);

            if (beginsWithRa && form(*(uc+2)) == Control)
                beginsWithRa = FALSE;

            base = (beginsWithRa ? 2 : 0);
            IDEBUG("    length = %d, beginsWithRa = %d, base=%d", len, beginsWithRa, base);

            int lastConsonant = 0;
            int matra = -1;
            // we remember:
            // * the last consonant since we need it for rule 2
            // * the matras position for rule 3 and 4

            // figure out possible base glyphs
            memset(position.data(), 0, len);
            if (script == QFont::Devanagari || script == QFont::Gujarati) {
                bool vattu = FALSE;
                for (i = base; i < len; ++i) {
                    position[i] = form(uc[i]);
                    if (position[i] == Consonant) {
                        lastConsonant = i;
                        vattu = (!vattu && uc[i] == ra);
                        if (vattu) {
                            IDEBUG("excluding vattu glyph at %d from base candidates", i);
                            position[i] = Vattu;
                        }
                    } else if (position[i] == Matra) {
                        matra = i;
                    }
                }
            } else {
                for (i = base; i < len; ++i) {
                    position[i] = form(uc[i]);
                    if (position[i] == Consonant)
                        lastConsonant = i;
                    else if (matra < 0 && position[i] == Matra)
                        matra = i;
                }
            }
            int skipped = 0;
            Position pos = Post;
            for (i = len-1; i > base; i--) {
                if (position[i] != Consonant && (position[i] != Control || script == QFont::Kannada))
                    continue;

                Position charPosition = indic_position(uc[i]);
                if (pos == Post && charPosition == Post) {
                    pos = Post;
                } else if ((pos == Post || pos == Below) && charPosition == Below) {
                    if (script == QFont::Devanagari || script == QFont::Gujarati)
                        base = i;
                    pos = Below;
                } else {
                    base = i;
                    break;
                }
                if (skipped == 2 && (script == QFont::Kannada || script == QFont::Telugu)) {
                    base = i;
                    break;
                }
                ++skipped;
            }

            IDEBUG("    base consonant at %d skipped=%d, lastConsonant=%d", base, skipped, lastConsonant);

            // Rule 2:
            //
            // If the base consonant is not the last one, Uniscribe
            // moves the halant from the base consonant to the last
            // one.
            if (lastConsonant > base) {
                int halantPos = 0;
                if (uc[base+1] == halant)
                    halantPos = base + 1;
                else if (uc[base+1] == nukta && uc[base+2] == halant)
                    halantPos = base + 2;
                if (halantPos > 0) {
                    IDEBUG("    moving halant from %d to %d!", base+1, lastConsonant);
                    for (i = halantPos; i < lastConsonant; i++)
                        uc[i] = uc[i+1];
                    uc[lastConsonant] = halant;
                }
            }

            // Rule 3:
            //
            // If the syllable starts with Ra + H, Uniscribe moves
            // this combination so that it follows either:

            // * the post-base 'matra' (if any) or the base consonant
            //   (in scripts that show similarity to Devanagari, i.e.,
            //   Devanagari, Gujarati, Bengali)
            // * the base consonant (other scripts)
            // * the end of the syllable (Kannada)

            Position matra_position = None;
            if (matra > 0)
                matra_position = indic_position(uc[matra]);
            IDEBUG("    matra at %d with form %d, base=%d", matra, matra_position, base);

            if (beginsWithRa && base != 0) {
                int toPos = base+1;
                if (toPos < len && uc[toPos] == nukta)
                    toPos++;
                if (toPos < len && uc[toPos] == halant)
                    toPos++;
                if (toPos < len && uc[toPos] == 0x200d)
                    toPos++;
                if (toPos < len-1 && uc[toPos] == ra && uc[toPos+1] == halant)
                    toPos += 2;
                if (script == QFont::Devanagari || script == QFont::Gujarati || script == QFont::Bengali) {
                    if (matra_position == Post || matra_position == Split) {
                        toPos = matra+1;
                        matra -= 2;
                    }
                } else if (script == QFont::Kannada) {
                    toPos = len;
                    matra -= 2;
                }

                IDEBUG("moving leading ra+halant to position %d", toPos);
                for (i = 2; i < toPos; i++)
                    uc[i-2] = uc[i];
                uc[toPos-2] = ra;
                uc[toPos-1] = halant;
                base -= 2;
                if (properties & HasReph)
                    reph = toPos-2;
            }

            // Rule 4:

            // Uniscribe splits two- or three-part matras into their
            // parts. This splitting is a character-to-character
            // operation).
            //
            //      Uniscribe describes some moving operations for these
            //      matras here. For shaping however all pre matras need
            //      to be at the begining of the syllable, so we just move
            //      them there now.
            if (matra_position == Split) {
                splitMatra(uc, matra, len, base);
                // Handle three-part matras (0xccb in Kannada)
                matra_position = indic_position(uc[matra]);
                if (matra_position == Split)
                    splitMatra(uc, matra, len, base);
            } else if (matra_position == Pre) {
                unsigned short m = uc[matra];
                while (matra--)
                    uc[matra+1] = uc[matra];
                uc[0] = m;
                base++;
            }
        }

        // Rule 5:
        //
        // Uniscribe classifies consonants and 'matra' parts as
        // pre-base, above-base (Reph), below-base or post-base. This
        // classification exists on the character code level and is
        // language-dependent, not font-dependent.
        for (i = 0; i < base; ++i)
            position[i] = Pre;
        position[base] = Base;
        for (i = base+1; i < len; ++i) {
            position[i] = indic_position(uc[i]);
            // #### replace by adjusting table
            if (uc[i] == nukta || uc[i] == halant)
                position[i] = Inherit;
        }
        if (reph > 0) {
            // recalculate reph, it might have changed.
            for (i = base+1; i < len; ++i)
                if (uc[i] == ra)
                    reph = i;
            position[reph] = Reph;
            position[reph+1] = Inherit;
        }

        // all reordering happens now to the chars after the base
        int fixed = base+1;
        if (fixed < len && uc[fixed] == nukta)
            fixed++;
        if (fixed < len && uc[fixed] == halant)
            fixed++;
        if (fixed < len && uc[fixed] == 0x200d)
            fixed++;

#ifdef INDIC_DEBUG
        for (i = fixed; i < len; ++i)
            IDEBUG("position[%d] = %d, form=%d", i, position[i], form(uc[i]));
#endif
        // we continuosly position the matras and vowel marks and increase the fixed
        // until we reached the end.
        const IndicOrdering *finalOrder = indic_order[script-QFont::Devanagari];

        IDEBUG("    reordering pass:");
        //IDEBUG("        base=%d fixed=%d", base, fixed);
        int toMove = 0;
        while (finalOrder[toMove].form && fixed < len-1) {
            //IDEBUG("        fixed = %d, moving form %d with pos %d", fixed, finalOrder[toMove].form, finalOrder[toMove].position);
            for (i = fixed; i < len; i++) {
                if (form(uc[i]) == finalOrder[toMove].form &&
                     position[i] == finalOrder[toMove].position) {
                    // need to move this glyph
                    int to = fixed;
                    if (i < len-1 && position[i+1] == Inherit) {
                        IDEBUG("         moving two chars from %d to %d", i, to);
                        unsigned short ch = uc[i];
                        unsigned short ch2 = uc[i+1];
                        unsigned char pos = position[i];
                        for (int j = i+1; j > to+1; j--) {
                            uc[j] = uc[j-2];
                            position[j] = position[j-2];
                        }
                        uc[to] = ch;
                        uc[to+1] = ch2;
                        position[to] = pos;
                        position[to+1] = pos;
                        fixed += 2;
                    } else {
                        IDEBUG("         moving one char from %d to %d", i, to);
                        unsigned short ch = uc[i];
                        unsigned char pos = position[i];
                        for (int j = i; j > to; j--) {
                            uc[j] = uc[j-1];
                            position[j] = position[j-1];
                        }
                        uc[to] = ch;
                        position[to] = pos;
                        fixed++;
                    }
                }
            }
            toMove++;
        }

    }

    if (reph > 0) {
        // recalculate reph, it might have changed.
        for (i = base+1; i < len; ++i)
            if (reordered[i] == ra)
                reph = i;
    }

    if (item->font->stringToCMap((const QChar *)reordered.data(), len, item->glyphs, item->advances,
                                 &item->num_glyphs, item->flags & QTextEngine::RightToLeft) != QFontEngine::NoError)
        return FALSE;


    IDEBUG("  base=%d, reph=%d", base, reph);
    IDEBUG("reordered:");
    for (i = 0; i < len; i++) {
        item->attributes[i].mark = FALSE;
        item->attributes[i].clusterStart = FALSE;
        item->attributes[i].justification = 0;
        item->attributes[i].zeroWidth = FALSE;
        IDEBUG("    %d: %4x", i, reordered[i]);
    }

    // now we have the syllable in the right order, and can start running it through open type.

    bool control = FALSE;
    for (i = 0; i < len; ++i)
        control |= (form(reordered[i]) == Control);

#ifndef QT_NO_XFTFREETYPE
    if (openType) {

        // we need to keep track of where the base glyph is for some
        // scripts and use the cluster feature for this.  This
        // also means we have to correct the logCluster output from
        // the open type engine manually afterwards.  for indic this
        // is rather simple, as all chars just point to the first
        // glyph in the syllable.
        QVarLengthArray<unsigned short> clusters(len);
        QVarLengthArray<unsigned int> properties(len);

        for (i = 0; i < len; ++i)
            clusters[i] = i;

        // features we should always apply
        for (i = 0; i < len; ++i)
            properties[i] = ~(CcmpProperty
                              | NuktaProperty
                              | VattuProperty
                              | PreSubstProperty
                              | BelowSubstProperty
                              | AboveSubstProperty
                              | HalantProperty
                              | PositioningProperties);

        // Ccmp always applies
        // Init
        if (item->from == 0
            || !(item->string->unicode()[item->from-1].isLetter() ||  item->string->unicode()[item->from-1].isMark()))
            properties[0] &= ~InitProperty;

        // Nukta always applies
        // Akhant
        for (i = 0; i <= base; ++i)
            properties[i] &= ~AkhantProperty;
        // Reph
        if (reph >= 0) {
            properties[reph] &= ~RephProperty;
            properties[reph+1] &= ~RephProperty;
        }
        // BelowForm
        for (i = base+1; i < len; ++i)
            properties[i] &= ~BelowFormProperty;

        if (script == QFont::Devanagari || script == QFont::Gujarati) {
            // vattu glyphs need this aswell
            bool vattu = FALSE;
            for (i = base-2; i > 1; --i) {
                if (form(reordered[i]) == Consonant) {
                    vattu = (!vattu && reordered[i] == ra);
                    if (vattu) {
                        IDEBUG("forming vattu ligature at %d", i);
                        properties[i] &= ~BelowFormProperty;
                        properties[i+1] &= ~BelowFormProperty;
                    }
                }
            }
        }
        // HalfFormProperty
        for (i = 0; i < base; ++i)
            properties[i] &= ~HalfFormProperty;
        if (control) {
            for (i = 2; i < len; ++i) {
                if (reordered[i] == 0x200d /* ZWJ */) {
                    properties[i-1] &= ~HalfFormProperty;
                    properties[i-2] &= ~HalfFormProperty;
                } else if (reordered[i] == 0x200c /* ZWNJ */) {
                    properties[i-1] &= ~HalfFormProperty;
                    properties[i-2] &= ~HalfFormProperty;
                }
            }
        }
        // PostFormProperty
        for (i = base+1; i < len; ++i)
            properties[i] &= ~PostFormProperty;
        // vattu always applies
        // pres always applies
        // blws always applies
        // abvs always applies

        // psts
        // ### this looks slightly different from before, but I believe it's correct
        if (reordered[len-1] != halant || base != len-2)
            properties[base] &= ~PostSubstProperty;
        for (i = base+1; i < len; ++i)
            properties[i] &= ~PostSubstProperty;

        // halant always applies

#ifdef INDIC_DEBUG
        {
            IDEBUG("OT properties:");
            for (int i = 0; i < len; ++i)
                qDebug("    i: %s", ::propertiesToString(properties[i]).toLatin1().data());
        }
#endif

        // initialize
        item->log_clusters = clusters.data();
        openType->shape(item, properties.data());

        int newLen = openType->len();
        OTL_GlyphItem otl_glyphs = openType->glyphs();

        // move the left matra back to it's correct position in malayalam and tamil
        if ((script == QFont::Malayalam || script == QFont::Tamil) && (form(reordered[0]) == Matra)) {
//             qDebug("reordering matra, len=%d", newLen);
            // need to find the base in the shaped string and move the matra there
            int basePos = 0;
            while (basePos < newLen && (int)otl_glyphs[basePos].cluster <= base)
                basePos++;
            --basePos;
            if (basePos < newLen && basePos > 1) {
//                 qDebug("moving prebase matra to position %d in syllable newlen=%d", basePos, newLen);
                OTL_GlyphItemRec m = otl_glyphs[0];
                --basePos;
                for (i = 0; i < basePos; ++i)
                    otl_glyphs[i] = otl_glyphs[i+1];
                otl_glyphs[basePos] = m;
            }
        }

        if (!openType->positionAndAdd(item, FALSE))
            return FALSE;

        if (control) {
            IDEBUG("found a control char in the syllable");
            int i = 0, j = 0;
            while (i < item->num_glyphs) {
                if (form(reordered[otl_glyphs[i].cluster]) == Control) {
                    ++i;
                    if (i >= item->num_glyphs)
                        break;
                }
                item->glyphs[j] = item->glyphs[i];
                ++i;
                ++j;
            }
            item->num_glyphs = j;
        }

    }
#endif

    item->attributes[0].clusterStart = TRUE;
    IDEBUG("<<<<<<");
    return TRUE;
}


/* syllables are of the form:

   (Consonant Nukta? Halant)* Consonant Matra? VowelMark? StressMark?
   (Consonant Nukta? Halant)* Consonant Halant
   IndependentVowel VowelMark? StressMark?

   We return syllable boundaries on invalid combinations aswell
*/
static int indic_nextSyllableBoundary(int script, const QString &s, int start, int end, bool *invalid)
{
    *invalid = FALSE;
    IDEBUG("indic_nextSyllableBoundary: start=%d, end=%d", start, end);
    const QChar *uc = s.unicode()+start;

    int pos = 0;
    Form state = form(uc[pos].unicode());
    IDEBUG("state[%d]=%d (uc=%4x)", pos, state, uc[pos].unicode());
    pos++;

    if (state != Consonant && state != IndependentVowel) {
        if (state != Other)
            *invalid = TRUE;
        goto finish;
    }

    while (pos < end - start) {
        Form newState = form(uc[pos].unicode());
        IDEBUG("state[%d]=%d (uc=%4x)", pos, newState, uc[pos].unicode());
        switch(newState) {
        case Control:
            newState = state;
 	    if (state == Halant && uc[pos].unicode() == 0x200d /* ZWJ */)
  		break;
            // the control character should be the last char in the item
            ++pos;
            goto finish;
        case Consonant:
	    if (state == Halant && (script != QFont::Sinhala || uc[pos-1].unicode() == 0x200d /* ZWJ */))
                break;
            goto finish;
        case Halant:
            if (state == Nukta || state == Consonant)
                break;
            // Bengali has a special exception allowing the combination Vowel_A/E + Halant + Ya
            if (script == QFont::Bengali && pos == 1 &&
                 (uc[0].unicode() == 0x0985 || uc[0].unicode() == 0x098f))
                break;
            goto finish;
        case Nukta:
            if (state == Consonant)
                break;
            goto finish;
        case StressMark:
            if (state == VowelMark)
                break;
            // fall through
        case VowelMark:
            if (state == Matra || state == IndependentVowel)
                break;
            // fall through
        case Matra:
            if (state == Consonant || state == Nukta)
                break;
            // ### not sure if this is correct. If it is, does it apply only to Bengali or should
            // it work for all Indic languages?
            // the combination Independent_A + Vowel Sign AA is allowed.
            if (script == QFont::Bengali && uc[pos].unicode() == 0x9be && uc[pos-1].unicode() == 0x985)
                break;
            if (script == QFont::Tamil && state == Matra) {
                if (uc[pos-1].unicode() == 0x0bc6 &&
                     (uc[pos].unicode() == 0xbbe || uc[pos].unicode() == 0xbd7))
                    break;
                if (uc[pos-1].unicode() == 0x0bc7 && uc[pos].unicode() == 0xbbe)
                    break;
            }
            goto finish;

        case LengthMark:
        case IndependentVowel:
        case Invalid:
        case Other:
            goto finish;
        }
        state = newState;
        pos++;
    }
 finish:
    return pos+start;
}

static bool indic_shape(QShaperItem *item)
{
    Q_ASSERT(item->script >= QFont::Devanagari && item->script <= QFont::Sinhala);

#ifndef QT_NO_XFTFREETYPE
    QOpenType *openType = item->font->openType();
    if (openType)
        openType->selectScript(item->script, indic_features);
#else
    QOpenType *openType = 0;
#endif
    unsigned short *logClusters = item->log_clusters;

    QShaperItem syllable = *item;
    int first_glyph = 0;

    int sstart = item->from;
    int end = sstart + item->length;
    IDEBUG("indic_shape: from %d length %d", item->from, item->length);
    while (sstart < end) {
        bool invalid;
        int send = indic_nextSyllableBoundary(item->script, *item->string, sstart, end, &invalid);
        IDEBUG("syllable from %d, length %d, invalid=%s", sstart, send-sstart,
               invalid ? "TRUE" : "FALSE");
        syllable.from = sstart;
        syllable.length = send-sstart;
        syllable.glyphs = item->glyphs + first_glyph;
        syllable.offsets = item->offsets + first_glyph;
        syllable.advances = item->advances + first_glyph;
        syllable.attributes = item->attributes + first_glyph;
        syllable.num_glyphs = item->num_glyphs - first_glyph;
        if (!indic_shape_syllable(openType, &syllable, invalid)) {
            IDEBUG("syllable shaping failed, syllable requests %d glyphs", syllable.num_glyphs);
            item->num_glyphs += syllable.num_glyphs;
            return FALSE;
        }
        item->has_positioning |= syllable.has_positioning;

        // fix logcluster array
        IDEBUG("syllable:");
	int i;
        for (i = first_glyph; i < first_glyph + syllable.num_glyphs; ++i)
            IDEBUG("        %d -> glyph %x", i, item->glyphs[i]);
        IDEBUG("    logclusters:");
        for (i = sstart; i < send; ++i) {
            IDEBUG("        %d -> glyph %d", i, first_glyph);
            logClusters[i-item->from] = first_glyph;
        }
        sstart = send;
        first_glyph += syllable.num_glyphs;
    }
    item->num_glyphs = first_glyph;
    return TRUE;
}


static void indic_attributes(int script, const QString &text, int from, int len, QCharAttributes *attributes)
{
    int end = from + len;
    const QChar *uc = text.unicode() + from;
    attributes += from;
    int i = 0;
    while (i < len) {
        bool invalid;
        int boundary = indic_nextSyllableBoundary(script, text, from+i, end, &invalid) - from;
         attributes[i].charStop = TRUE;

        if (boundary > len-1) boundary = len;
        i++;
        while (i < boundary) {
            attributes[i].charStop = FALSE;
            ++uc;
            ++i;
        }
        assert(i == boundary);
    }


}


// --------------------------------------------------------------------------------------------------------------------------------------------
//
// Thai and Lao
//
// --------------------------------------------------------------------------------------------------------------------------------------------

#include <qtextcodec.h>
#include <qlibrary.h>


static void thaiWordBreaks(const QChar *string, const int len, QCharAttributes *attributes)
{
#ifndef QT_NO_TEXTCODEC
    typedef int (*th_brk_def)(const char*, int[], int);
    static QTextCodec *thaiCodec = QTextCodec::codecForMib(2259);
    static th_brk_def th_brk = 0;

#ifndef QT_NO_LIBRARY
    /* load libthai dynamically */
    if (!th_brk && thaiCodec) {
        th_brk = (th_brk_def)QLibrary::resolve("thai", "th_brk");
        if (!th_brk)
            thaiCodec = 0;
    }
#endif

    if (!th_brk)
        return;

    QCString cstr = thaiCodec->fromUnicode(QConstString(string, len).string());

    int brp[128];
    int *break_positions = brp;
    int numbreaks = th_brk(cstr.data(), break_positions, 128);
    if (numbreaks > 128) {
        break_positions = new int[numbreaks];
        numbreaks = th_brk(cstr.data(),break_positions, numbreaks);
    }

    attributes[0].softBreak = TRUE;
    int i;
    for (i = 1; i < len; ++i)
        attributes[i].softBreak = FALSE;

    for (i = 0; i < numbreaks; ++i)
        attributes[break_positions[i]].softBreak = TRUE;

    if (break_positions != brp)
        delete [] break_positions;
#endif
}


static void thai_attributes( int script, const QString &text, int from, int len, QCharAttributes *attributes )
{
    Q_UNUSED(script);
    Q_ASSERT(script == QFont::Thai);
    thaiWordBreaks(text.unicode() + from, len, attributes);
}



// --------------------------------------------------------------------------------------------------------------------------------------------
//
// Tibetan
//
// --------------------------------------------------------------------------------------------------------------------------------------------

// tibetan syllables are of the form:
//    head position consonant
//    first sub-joined consonant
//    ....intermediate sub-joined consonants (if any)
//    last sub-joined consonant
//    sub-joined vowel (a-chung U+0F71)
//    standard or compound vowel sign (or 'virama' for devanagari transliteration)

enum TibetanForm {
    TibetanOther,
    TibetanHeadConsonant,
    TibetanSubjoinedConsonant,
    TibetanSubjoinedVowel,
    TibetanVowel
};

// this table starts at U+0f40
static const unsigned char tibetanForm[0x80] = {
    TibetanHeadConsonant, TibetanHeadConsonant, TibetanHeadConsonant, TibetanHeadConsonant,
    TibetanHeadConsonant, TibetanHeadConsonant, TibetanHeadConsonant, TibetanHeadConsonant,
    TibetanHeadConsonant, TibetanHeadConsonant, TibetanHeadConsonant, TibetanHeadConsonant,
    TibetanHeadConsonant, TibetanHeadConsonant, TibetanHeadConsonant, TibetanHeadConsonant,

    TibetanHeadConsonant, TibetanHeadConsonant, TibetanHeadConsonant, TibetanHeadConsonant,
    TibetanHeadConsonant, TibetanHeadConsonant, TibetanHeadConsonant, TibetanHeadConsonant,
    TibetanHeadConsonant, TibetanHeadConsonant, TibetanHeadConsonant, TibetanHeadConsonant,
    TibetanHeadConsonant, TibetanHeadConsonant, TibetanHeadConsonant, TibetanHeadConsonant,

    TibetanHeadConsonant, TibetanHeadConsonant, TibetanHeadConsonant, TibetanHeadConsonant,
    TibetanHeadConsonant, TibetanHeadConsonant, TibetanHeadConsonant, TibetanHeadConsonant,
    TibetanHeadConsonant, TibetanHeadConsonant, TibetanHeadConsonant, TibetanHeadConsonant,
    TibetanOther, TibetanOther, TibetanOther, TibetanOther,

    TibetanOther, TibetanVowel, TibetanVowel, TibetanVowel,
    TibetanVowel, TibetanVowel, TibetanVowel, TibetanVowel,
    TibetanVowel, TibetanVowel, TibetanVowel, TibetanVowel,
    TibetanVowel, TibetanVowel, TibetanVowel, TibetanVowel,

    TibetanVowel, TibetanVowel, TibetanVowel, TibetanVowel,
    TibetanVowel, TibetanVowel, TibetanVowel, TibetanVowel,
    TibetanOther, TibetanOther, TibetanOther, TibetanOther,
    TibetanOther, TibetanOther, TibetanOther, TibetanOther,

    TibetanSubjoinedConsonant, TibetanSubjoinedConsonant, TibetanSubjoinedConsonant, TibetanSubjoinedConsonant,
    TibetanSubjoinedConsonant, TibetanSubjoinedConsonant, TibetanSubjoinedConsonant, TibetanSubjoinedConsonant,
    TibetanSubjoinedConsonant, TibetanSubjoinedConsonant, TibetanSubjoinedConsonant, TibetanSubjoinedConsonant,
    TibetanSubjoinedConsonant, TibetanSubjoinedConsonant, TibetanSubjoinedConsonant, TibetanSubjoinedConsonant,

    TibetanSubjoinedConsonant, TibetanSubjoinedConsonant, TibetanSubjoinedConsonant, TibetanSubjoinedConsonant,
    TibetanSubjoinedConsonant, TibetanSubjoinedConsonant, TibetanSubjoinedConsonant, TibetanSubjoinedConsonant,
    TibetanSubjoinedConsonant, TibetanSubjoinedConsonant, TibetanSubjoinedConsonant, TibetanSubjoinedConsonant,
    TibetanSubjoinedConsonant, TibetanSubjoinedConsonant, TibetanSubjoinedConsonant, TibetanSubjoinedConsonant,

    TibetanSubjoinedConsonant, TibetanSubjoinedConsonant, TibetanSubjoinedConsonant, TibetanSubjoinedConsonant,
    TibetanSubjoinedConsonant, TibetanSubjoinedConsonant, TibetanSubjoinedConsonant, TibetanSubjoinedConsonant,
    TibetanSubjoinedConsonant, TibetanSubjoinedConsonant, TibetanSubjoinedConsonant, TibetanSubjoinedConsonant,
    TibetanSubjoinedConsonant, TibetanOther, TibetanOther, TibetanOther
};


static inline TibetanForm tibetan_form(const QChar &c)
{
    return (TibetanForm)tibetanForm[c.unicode() - 0x0f40];
}

#ifndef QT_NO_XFTFREETYPE
static const QOpenType::Features tibetan_features[] = {
    { FT_MAKE_TAG('c', 'c', 'm', 'p'), CcmpProperty },
    { FT_MAKE_TAG('a', 'b', 'v', 's'), AboveSubstProperty },
    { FT_MAKE_TAG('b', 'l', 'w', 's'), BelowSubstProperty },
    {0, 0}
};
#endif

static bool tibetan_shape_syllable(QOpenType *openType, QShaperItem *item, bool invalid)
{
    Q_UNUSED(openType)
    int len = item->length;

    if (item->num_glyphs < item->length + 4) {
        item->num_glyphs = item->length + 4;
        return FALSE;
    }

    int i;
    QVarLengthArray<unsigned short> reordered(len+4);

    const QChar *str = item->string->unicode() + item->from;
    if (invalid) {
        *reordered.data() = 0x25cc;
        memcpy(reordered.data()+1, str, len*sizeof(QChar));
        len++;
        str = (QChar *)reordered.data();
    }

    if (item->font->stringToCMap(str, len, item->glyphs, item->advances,
                                 &item->num_glyphs, item->flags & QTextEngine::RightToLeft) != QFontEngine::NoError)
        return FALSE;

    for (i = 0; i < item->length; i++) {
        item->attributes[i].mark = FALSE;
        item->attributes[i].clusterStart = FALSE;
        item->attributes[i].justification = 0;
        item->attributes[i].zeroWidth = FALSE;
        IDEBUG("    %d: %4x", i, str[i].unicode());
    }

    // now we have the syllable in the right order, and can start running it through open type.

#ifndef QT_NO_XFTFREETYPE
    if (openType && openType->supportsScript(QFont::Tibetan)) {
        openType->selectScript(QFont::Tibetan, tibetan_features);

        openType->shape(item);
        if (!openType->positionAndAdd(item, FALSE))
            return FALSE;
    }
#endif

    item->attributes[0].clusterStart = TRUE;
    return TRUE;
}


static int tibetan_nextSyllableBoundary(const QString &s, int start, int end, bool *invalid)
{
    const QChar *uc = s.unicode() + start;

    int pos = 0;
    TibetanForm state = tibetan_form(*uc);

//     qDebug("state[%d]=%d (uc=%4x)", pos, state, uc[pos].unicode());
    pos++;

    if (state != TibetanHeadConsonant) {
        if (state != TibetanOther)
            *invalid = TRUE;
        goto finish;
    }

    while (pos < end - start) {
        TibetanForm newState = tibetan_form(uc[pos]);
        switch(newState) {
        case TibetanSubjoinedConsonant:
        case TibetanSubjoinedVowel:
            if (state != TibetanHeadConsonant &&
                 state != TibetanSubjoinedConsonant)
                goto finish;
            state = newState;
            break;
        case TibetanVowel:
            if (state != TibetanHeadConsonant &&
                 state != TibetanSubjoinedConsonant &&
                 state != TibetanSubjoinedVowel)
                goto finish;
            break;
        case TibetanOther:
        case TibetanHeadConsonant:
            goto finish;
        }
        pos++;
    }

finish:
    *invalid = FALSE;
    return start+pos;
}

static bool tibetan_shape(QShaperItem *item)
{
    Q_ASSERT(item->script == QFont::Tibetan);

#ifndef QT_NO_XFTFREETYPE
    QOpenType *openType = item->font->openType();
    if (openType && !openType->supportsScript(item->script))
        openType = 0;
#else
    QOpenType *openType = 0;
#endif
    unsigned short *logClusters = item->log_clusters;

    QShaperItem syllable = *item;
    int first_glyph = 0;

    int sstart = item->from;
    int end = sstart + item->length;
    while (sstart < end) {
        bool invalid;
        int send = tibetan_nextSyllableBoundary(*(item->string), sstart, end, &invalid);
        IDEBUG("syllable from %d, length %d, invalid=%s", sstart, send-sstart,
               invalid ? "TRUE" : "FALSE");
        syllable.from = sstart;
        syllable.length = send-sstart;
        syllable.glyphs = item->glyphs + first_glyph;
        syllable.offsets = item->offsets + first_glyph;
        syllable.advances = item->advances + first_glyph;
        syllable.attributes = item->attributes + first_glyph;
        syllable.num_glyphs = item->num_glyphs - first_glyph;
        if (!tibetan_shape_syllable(openType, &syllable, invalid)) {
            item->num_glyphs += syllable.num_glyphs;
            return FALSE;
        }
        item->has_positioning |= syllable.has_positioning;

        // fix logcluster array
        for (int i = sstart; i < send; ++i)
            logClusters[i-item->from] = first_glyph;
        sstart = send;
        first_glyph += syllable.num_glyphs;
    }
    item->num_glyphs = first_glyph;
    return TRUE;
}

static void tibetan_attributes(int script, const QString &text, int from, int len, QCharAttributes *attributes)
{
    Q_UNUSED(script);

    int end = from + len;
    const QChar *uc = text.unicode() + from;
    attributes += from;
    int i = 0;
    while (i < len) {
        bool invalid;
        int boundary = tibetan_nextSyllableBoundary(text, from+i, end, &invalid) - from;

        attributes[i].charStop = TRUE;

        if (boundary > len-1) boundary = len;
        i++;
        while (i < boundary) {
            attributes[i].charStop = FALSE;
            ++uc;
            ++i;
        }
        assert(i == boundary);
    }
}

// --------------------------------------------------------------------------------------------------------------------------------------------
//
// Khmer
//
// --------------------------------------------------------------------------------------------------------------------------------------------


//  Vocabulary
//      Base ->         A consonant or an independent vowel in its full (not subscript) form. It is the
//                      center of the syllable, it can be surrounded by coeng (subscript) consonants, vowels,
//                      split vowels, signs... but there is only one base in a syllable, it has to be coded as
//                      the first character of the syllable.
//      split vowel --> vowel that has two parts placed separately (e.g. Before and after the consonant).
//                      Khmer language has five of them. Khmer split vowels either have one part before the
//                      base and one after the base or they have a part before the base and a part above the base.
//                      The first part of all Khmer split vowels is the same character, identical to
//                      the glyph of Khmer dependent vowel SRA EI
//      coeng -->  modifier used in Khmer to construct coeng (subscript) consonants
//                 Differently than indian languages, the coeng modifies the consonant that follows it,
//                 not the one preceding it  Each consonant has two forms, the base form and the subscript form
//                 the base form is the normal one (using the consonants code-point), the subscript form is
//                 displayed when the combination coeng + consonant is encountered.
//      Consonant of type 1 -> A consonant which has subscript for that only occupies space under a base consonant
//      Consonant of type 2.-> Its subscript form occupies space under and before the base (only one, RO)
//      Consonant of Type 3 -> Its subscript form occupies space under and after the base (KHO, CHHO, THHO, BA, YO, SA)
//      Consonant shifter -> Khmer has to series of consonants. The same dependent vowel has different sounds
//                           if it is attached to a consonant of the first series or a consonant of the second series
//                           Most consonants have an equivalent in the other series, but some of theme exist only in
//                           one series (for example SA). If we want to use the consonant SA with a vowel sound that
//                           can only be done with a vowel sound that corresponds to a vowel accompanying a consonant
//                           of the other series, then we need to use a consonant shifter: TRIISAP or MUSIKATOAN
//                           x17C9 y x17CA. TRIISAP changes a first series consonant to second series sound and
//                           MUSIKATOAN a second series consonant to have a first series vowel sound.
//                           Consonant shifter are both normally supercript marks, but, when they are followed by a
//                           superscript, they change shape and take the form of subscript dependent vowel SRA U.
//                           If they are in the same syllable as a coeng consonant, Unicode 3.0 says that they
//                           should be typed before the coeng. Unicode 4.0 breaks the standard and says that it should
//                           be placed after the coeng consonant.
//      Dependent vowel ->   In khmer dependent vowels can be placed above, below, before or after the base
//                           Each vowel has its own position. Only one vowel per syllable is allowed.
//      Signs            ->  Khmer has above signs and post signs. Only one above sign and/or one post sign are
//                           Allowed in a syllable.
//
//
//   order is important here! This order must be the same that is found in each horizontal
//   line in the statetable for Khmer (see khmerStateTable) .
//
enum KhmerCharClassValues {
    CC_RESERVED             =  0,
    CC_CONSONANT            =  1, // Consonant of type 1 or independent vowel
    CC_CONSONANT2           =  2, // Consonant of type 2
    CC_CONSONANT3           =  3, // Consonant of type 3
    CC_ZERO_WIDTH_NJ_MARK   =  4, // Zero Width non joiner character (0x200C)
    CC_CONSONANT_SHIFTER    =  5,
    CC_ROBAT                =  6, // Khmer special diacritic accent -treated differently in state table
    CC_COENG                =  7, // Subscript consonant combining character
    CC_DEPENDENT_VOWEL      =  8,
    CC_SIGN_ABOVE           =  9,
    CC_SIGN_AFTER           = 10,
    CC_ZERO_WIDTH_J_MARK    = 11, // Zero width joiner character
    CC_COUNT                = 12  // This is the number of character classes
};


enum KhmerCharClassFlags {
    CF_CLASS_MASK    = 0x0000FFFF,

    CF_CONSONANT     = 0x01000000,  // flag to speed up comparing
    CF_SPLIT_VOWEL   = 0x02000000,  // flag for a split vowel -> the first part is added in front of the syllable
    CF_DOTTED_CIRCLE = 0x04000000,  // add a dotted circle if a character with this flag is the first in a syllable
    CF_COENG         = 0x08000000,  // flag to speed up comparing
    CF_SHIFTER       = 0x10000000,  // flag to speed up comparing
    CF_ABOVE_VOWEL   = 0x20000000,  // flag to speed up comparing

    // position flags
    CF_POS_BEFORE    = 0x00080000,
    CF_POS_BELOW     = 0x00040000,
    CF_POS_ABOVE     = 0x00020000,
    CF_POS_AFTER     = 0x00010000,
    CF_POS_MASK      = 0x000f0000
};


// Characters that get refered to by name
enum KhmerChar {
    C_SIGN_ZWNJ     = 0x200C,
    C_SIGN_ZWJ      = 0x200D,
    C_DOTTED_CIRCLE = 0x25CC,
    C_RO            = 0x179A,
    C_VOWEL_AA      = 0x17B6,
    C_SIGN_NIKAHIT  = 0x17C6,
    C_VOWEL_E       = 0x17C1,
    C_COENG         = 0x17D2
};


//  simple classes, they are used in the statetable (in this file) to control the length of a syllable
//  they are also used to know where a character should be placed (location in reference to the base character)
//  and also to know if a character, when independently displayed, should be displayed with a dotted-circle to
//  indicate error in syllable construction
//
enum {
    _xx = CC_RESERVED,
    _sa = CC_SIGN_ABOVE | CF_DOTTED_CIRCLE | CF_POS_ABOVE,
    _sp = CC_SIGN_AFTER | CF_DOTTED_CIRCLE| CF_POS_AFTER,
    _c1 = CC_CONSONANT | CF_CONSONANT,
    _c2 = CC_CONSONANT2 | CF_CONSONANT,
    _c3 = CC_CONSONANT3 | CF_CONSONANT,
    _rb = CC_ROBAT | CF_POS_ABOVE | CF_DOTTED_CIRCLE,
    _cs = CC_CONSONANT_SHIFTER | CF_DOTTED_CIRCLE | CF_SHIFTER,
    _dl = CC_DEPENDENT_VOWEL | CF_POS_BEFORE | CF_DOTTED_CIRCLE,
    _db = CC_DEPENDENT_VOWEL | CF_POS_BELOW | CF_DOTTED_CIRCLE,
    _da = CC_DEPENDENT_VOWEL | CF_POS_ABOVE | CF_DOTTED_CIRCLE | CF_ABOVE_VOWEL,
    _dr = CC_DEPENDENT_VOWEL | CF_POS_AFTER | CF_DOTTED_CIRCLE,
    _co = CC_COENG | CF_COENG | CF_DOTTED_CIRCLE,

    // split vowel
    _va = _da | CF_SPLIT_VOWEL,
    _vr = _dr | CF_SPLIT_VOWEL
};


//   Character class: a character class value
//   ORed with character class flags.
//
typedef unsigned long KhmerCharClass;


//  Character class tables
//  _xx character does not combine into syllable, such as numbers, puntuation marks, non-Khmer signs...
//  _sa Sign placed above the base
//  _sp Sign placed after the base
//  _c1 Consonant of type 1 or independent vowel (independent vowels behave as type 1 consonants)
//  _c2 Consonant of type 2 (only RO)
//  _c3 Consonant of type 3
//  _rb Khmer sign robat u17CC. combining mark for subscript consonants
//  _cd Consonant-shifter
//  _dl Dependent vowel placed before the base (left of the base)
//  _db Dependent vowel placed below the base
//  _da Dependent vowel placed above the base
//  _dr Dependent vowel placed behind the base (right of the base)
//  _co Khmer combining mark COENG u17D2, combines with the consonant or independent vowel following
//      it to create a subscript consonant or independent vowel
//  _va Khmer split vowel in wich the first part is before the base and the second one above the base
//  _vr Khmer split vowel in wich the first part is before the base and the second one behind (right of) the base
//
static const KhmerCharClass khmerCharClasses[] = {
    _c1, _c1, _c1, _c3, _c1, _c1, _c1, _c1, _c3, _c1, _c1, _c1, _c1, _c3, _c1, _c1, // 1780 - 178F
    _c1, _c1, _c1, _c1, _c3, _c1, _c1, _c1, _c1, _c3, _c2, _c1, _c1, _c1, _c3, _c3, // 1790 - 179F
    _c1, _c3, _c1, _c1, _c1, _c1, _c1, _c1, _c1, _c1, _c1, _c1, _c1, _c1, _c1, _c1, // 17A0 - 17AF
    _c1, _c1, _c1, _c1, _dr, _dr, _dr, _da, _da, _da, _da, _db, _db, _db, _va, _vr, // 17B0 - 17BF
    _vr, _dl, _dl, _dl, _vr, _vr, _sa, _sp, _sp, _cs, _cs, _sa, _rb, _sa, _sa, _sa, // 17C0 - 17CF
    _sa, _sa, _co, _sa, _xx, _xx, _xx, _xx, _xx, _xx, _xx, _xx, _xx, _sa, _xx, _xx  // 17D0 - 17DF
};

// this enum must reflect the range of khmerCharClasses
enum KhmerCharClassesRange {
    KhmerFirstChar = 0x1780,
    KhmerLastChar  = 0x17df
};

//  Below we define how a character in the input string is either in the khmerCharClasses table
//  (in which case we get its type back), a ZWJ or ZWNJ (two characters that may appear
//  within the syllable, but are not in the table) we also get their type back, or an unknown object
//  in which case we get _xx (CC_RESERVED) back
//
static inline KhmerCharClass getKhmerCharClass(const QChar &uc)
{
    if (uc.unicode() == C_SIGN_ZWJ) {
        return CC_ZERO_WIDTH_J_MARK;
    }

    if (uc.unicode() == C_SIGN_ZWNJ) {
        return CC_ZERO_WIDTH_NJ_MARK;
    }

    if (uc.unicode() < KhmerFirstChar || uc.unicode() > KhmerLastChar) {
        return CC_RESERVED;
    }

    return khmerCharClasses[uc.unicode() - KhmerFirstChar];
}


//  The stateTable is used to calculate the end (the length) of a well
//  formed Khmer Syllable.
//
//  Each horizontal line is ordered exactly the same way as the values in KhmerClassTable
//  CharClassValues. This coincidence of values allows the follow up of the table.
//
//  Each line corresponds to a state, which does not necessarily need to be a type
//  of component... for example, state 2 is a base, with is always a first character
//  in the syllable, but the state could be produced a consonant of any type when
//  it is the first character that is analysed (in ground state).
//
//  Differentiating 3 types of consonants is necessary in order to
//  forbid the use of certain combinations, such as having a second
//  coeng after a coeng RO,
//  The inexistent possibility of having a type 3 after another type 3 is permitted,
//  eliminating it would very much complicate the table, and it does not create typing
//  problems, as the case above.
//
//  The table is quite complex, in order to limit the number of coeng consonants
//  to 2 (by means of the table).
//
//  There a peculiarity, as far as Unicode is concerned:
//  - The consonant-shifter is considered in two possible different
//    locations, the one considered in Unicode 3.0 and the one considered in
//    Unicode 4.0. (there is a backwards compatibility problem in this standard).
//
//
//  xx    independent character, such as a number, punctuation sign or non-khmer char
//
//  c1    Khmer consonant of type 1 or an independent vowel
//        that is, a letter in which the subscript for is only under the
//        base, not taking any space to the right or to the left
//
//  c2    Khmer consonant of type 2, the coeng form takes space under
//        and to the left of the base (only RO is of this type)
//
//  c3    Khmer consonant of type 3. Its subscript form takes space under
//        and to the right of the base.
//
//  cs    Khmer consonant shifter
//
//  rb    Khmer robat
//
//  co    coeng character (u17D2)
//
//  dv    dependent vowel (including split vowels, they are treated in the same way).
//        even if dv is not defined above, the component that is really tested for is
//        KhmerClassTable::CC_DEPENDENT_VOWEL, which is common to all dependent vowels
//
//  zwj   Zero Width joiner
//
//  zwnj  Zero width non joiner
//
//  sa    above sign
//
//  sp    post sign
//
//  there are lines with equal content but for an easier understanding
//  (and maybe change in the future) we did not join them
//
static const signed char khmerStateTable[][CC_COUNT] =
{
   // xx  c1  c2  c3 zwnj cs  rb  co  dv  sa  sp zwj
    { 1,  2,  2,  2,  1,  1,  1,  6,  1,  1,  1,  2}, //  0 - ground state
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, //  1 - exit state (or sign to the right of the syllable)
    {-1, -1, -1, -1,  3,  4,  5,  6, 16, 17,  1, -1}, //  2 - Base consonant
    {-1, -1, -1, -1, -1,  4, -1, -1, 16, -1, -1, -1}, //  3 - First ZWNJ before a register shifter It can only be followed by a shifter or a vowel
    {-1, -1, -1, -1, 15, -1, -1,  6, 16, 17,  1, 14}, //  4 - First register shifter
    {-1, -1, -1, -1, -1, -1, -1, -1, 20, -1,  1, -1}, //  5 - Robat
    {-1,  7,  8,  9, -1, -1, -1, -1, -1, -1, -1, -1}, //  6 - First Coeng
    {-1, -1, -1, -1, 12, 13, -1, 10, 16, 17,  1, 14}, //  7 - First consonant of type 1 after coeng
    {-1, -1, -1, -1, 12, 13, -1, -1, 16, 17,  1, 14}, //  8 - First consonant of type 2 after coeng
    {-1, -1, -1, -1, 12, 13, -1, 10, 16, 17,  1, 14}, //  9 - First consonant or type 3 after ceong
    {-1, 11, 11, 11, -1, -1, -1, -1, -1, -1, -1, -1}, // 10 - Second Coeng (no register shifter before)
    {-1, -1, -1, -1, 15, -1, -1, -1, 16, 17,  1, 14}, // 11 - Second coeng consonant (or ind. vowel) no register shifter before
    {-1, -1, -1, -1, -1, 13, -1, -1, 16, -1, -1, -1}, // 12 - Second ZWNJ before a register shifter
    {-1, -1, -1, -1, 15, -1, -1, -1, 16, 17,  1, 14}, // 13 - Second register shifter
    {-1, -1, -1, -1, -1, -1, -1, -1, 16, -1, -1, -1}, // 14 - ZWJ before vowel
    {-1, -1, -1, -1, -1, -1, -1, -1, 16, -1, -1, -1}, // 15 - ZWNJ before vowel
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, 17,  1, 18}, // 16 - dependent vowel
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  1, 18}, // 17 - sign above
    {-1, -1, -1, -1, -1, -1, -1, 19, -1, -1, -1, -1}, // 18 - ZWJ after vowel
    {-1,  1, -1,  1, -1, -1, -1, -1, -1, -1, -1, -1}, // 19 - Third coeng
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  1, -1}, // 20 - dependent vowel after a Robat
};


//  #define KHMER_DEBUG
#ifdef KHMER_DEBUG
#define KHDEBUG qDebug
#else
#define KHDEBUG if(0) qDebug
#endif

//  Given an input string of characters and a location in which to start looking
//  calculate, using the state table, which one is the last character of the syllable
//  that starts in the starting position.
//
static inline int khmer_nextSyllableBoundary(const QString &s, int start, int end, bool *invalid)
{
    *invalid = FALSE;
    const QChar *uc = s.unicode() + start;
    int state = 0;
    int pos = start;

    while (pos < end) {
        KhmerCharClass charClass = getKhmerCharClass(*uc);
        if (pos == start) {
            *invalid = (charClass > 0) && ! (charClass & CF_CONSONANT);
        }
        state = khmerStateTable[state][charClass & CF_CLASS_MASK];

        KHDEBUG("state[%d]=%d class=%8lx (uc=%4x)", pos - start, state,
                charClass, uc->unicode() );

        if (state < 0) {
            break;
        }
        ++uc;
        ++pos;
    }
    return pos;
}


#ifndef QT_NO_XFTFREETYPE
static const QOpenType::Features khmer_features[] = {
    { FT_MAKE_TAG( 'p', 'r', 'e', 'f' ), PreFormProperty },
    { FT_MAKE_TAG( 'b', 'l', 'w', 'f' ), BelowFormProperty },
    { FT_MAKE_TAG( 'a', 'b', 'v', 'f' ), AboveFormProperty },
    { FT_MAKE_TAG( 'p', 's', 't', 'f' ), PostFormProperty },
    { FT_MAKE_TAG( 'p', 'r', 'e', 's' ), PreSubstProperty },
    { FT_MAKE_TAG( 'b', 'l', 'w', 's' ), BelowSubstProperty },
    { FT_MAKE_TAG( 'a', 'b', 'v', 's' ), AboveSubstProperty },
    { FT_MAKE_TAG( 'p', 's', 't', 's' ), PostSubstProperty },
    { FT_MAKE_TAG( 'c', 'l', 'i', 'g' ), CligProperty },
    { 0, 0 }
};
#endif


static bool khmer_shape_syllable(QOpenType *openType, QShaperItem *item)
{
#ifndef QT_NO_XFTFREETYPE
    if (openType)
        openType->selectScript(QFont::Khmer, khmer_features);
#endif
    // according to the specs this is the max length one can get
    // ### the real value should be smaller
    assert(item->length < 13);

    KHDEBUG("syllable from %d len %d, str='%s'", item->from, item->length,
	    item->string->mid(item->from, item->length).utf8().data());

    int len = 0;
    int syllableEnd = item->from + item->length;
    unsigned short reordered[16];
    unsigned char properties[16];
    enum {
	AboveForm = 0x01,
	PreForm = 0x02,
	PostForm = 0x04,
	BelowForm = 0x08
    };
    memset(properties, 0, 16*sizeof(unsigned char));

#ifdef KHMER_DEBUG
    qDebug("original:");
    for (int i = from; i < syllableEnd; i++) {
        qDebug("    %d: %4x", i, string[i].unicode());
    }
#endif

    // write a pre vowel or the pre part of a split vowel first
    // and look out for coeng + ro. RO is the only vowel of type 2, and
    // therefore the only one that requires saving space before the base.
    //
    int coengRo = -1;  // There is no Coeng Ro, if found this value will change
    int i;
    for (i = item->from; i < syllableEnd; i += 1) {
        KhmerCharClass charClass = getKhmerCharClass(item->string->at(i));

        // if a split vowel, write the pre part. In Khmer the pre part
        // is the same for all split vowels, same glyph as pre vowel C_VOWEL_E
        if (charClass & CF_SPLIT_VOWEL) {
            reordered[len] = C_VOWEL_E;
            properties[len] = PreForm;
            ++len;
            break; // there can be only one vowel
        }
        // if a vowel with pos before write it out
        if (charClass & CF_POS_BEFORE) {
            reordered[len] = item->string->at(i).unicode();
            properties[len] = PreForm;
            ++len;
            break; // there can be only one vowel
        }
        // look for coeng + ro and remember position
        // works because coeng + ro is always in front of a vowel (if there is a vowel)
        // and because CC_CONSONANT2 is enough to identify it, as it is the only consonant
        // with this flag
        if ( (charClass & CF_COENG) && (i + 1 < syllableEnd) &&
              ( (getKhmerCharClass(item->string->at(i+1)) & CF_CLASS_MASK) == CC_CONSONANT2) ) {
            coengRo = i;
        }
    }

    // write coeng + ro if found
    if (coengRo > -1) {
        reordered[len] = C_COENG;
        properties[len] = PreForm;
        ++len;
        reordered[len] = C_RO;
        properties[len] = PreForm;
        ++len;
    }

    // shall we add a dotted circle?
    // If in the position in which the base should be (first char in the string) there is
    // a character that has the Dotted circle flag (a character that cannot be a base)
    // then write a dotted circle
    if (getKhmerCharClass(item->string->at(item->from)) & CF_DOTTED_CIRCLE) {
        reordered[len] = C_DOTTED_CIRCLE;
        ++len;
    }

    // copy what is left to the output, skipping before vowels and
    // coeng Ro if they are present
    for (i = item->from; i < syllableEnd; i += 1) {
        QChar uc = item->string->at(i);
        KhmerCharClass charClass = getKhmerCharClass(uc);

        // skip a before vowel, it was already processed
        if (charClass & CF_POS_BEFORE) {
            continue;
        }

        // skip coeng + ro, it was already processed
        if (i == coengRo) {
            i += 1;
            continue;
        }

        switch (charClass & CF_POS_MASK)
        {
            case CF_POS_ABOVE :
                reordered[len] = uc.unicode();
                properties[len] = AboveForm;
                ++len;
                break;

            case CF_POS_AFTER :
                reordered[len] = uc.unicode();
                properties[len] = PostForm;
                ++len;
                break;

            case CF_POS_BELOW :
                reordered[len] = uc.unicode();
                properties[len] = BelowForm;
                ++len;
                break;

            default:
                // assign the correct flags to a coeng consonant
                // Consonants of type 3 are taged as Post forms and those type 1 as below forms
                if ( (charClass & CF_COENG) && i + 1 < syllableEnd ) {
                    unsigned char property = (getKhmerCharClass(item->string->at(i+1)) & CF_CLASS_MASK) == CC_CONSONANT3 ?
                                              PostForm : BelowForm;
                    reordered[len] = uc.unicode();
                    properties[len] = property;
                    ++len;
                    i += 1;
                    reordered[len] = item->string->at(i).unicode();
                    properties[len] = property;
                    ++len;
                    break;
                }

                // if a shifter is followed by an above vowel change the shifter to below form,
                // an above vowel can have two possible positions i + 1 or i + 3
                // (position i+1 corresponds to unicode 3, position i+3 to Unicode 4)
                // and there is an extra rule for C_VOWEL_AA + C_SIGN_NIKAHIT also for two
                // different positions, right after the shifter or after a vowel (Unicode 4)
                if ( (charClass & CF_SHIFTER) && (i + 1 < syllableEnd) ) {
                    if (getKhmerCharClass(item->string->at(i+1)) & CF_ABOVE_VOWEL ) {
                        reordered[len] = uc.unicode();
                        properties[len] = BelowForm;
                        ++len;
                        break;
                    }
                    if (i + 2 < syllableEnd &&
                        (item->string->at(i+1).unicode() == C_VOWEL_AA) &&
                        (item->string->at(i+2).unicode() == C_SIGN_NIKAHIT) )
                    {
                        reordered[len] = uc.unicode();
                        properties[len] = BelowForm;
                        ++len;
                        break;
                    }
                    if (i + 3 < syllableEnd && (getKhmerCharClass(item->string->at(i+3)) & CF_ABOVE_VOWEL) ) {
                        reordered[len] = uc.unicode();
                        properties[len] = BelowForm;
                        ++len;
                        break;
                    }
                    if (i + 4 < syllableEnd &&
                        (item->string->at(i+3).unicode() == C_VOWEL_AA) &&
                        (item->string->at(i+4).unicode() == C_SIGN_NIKAHIT) )
                    {
                        reordered[len] = uc.unicode();
                        properties[len] = BelowForm;
                        ++len;
                        break;
                    }
                }

                // default - any other characters
                reordered[len] = uc.unicode();
                ++len;
                break;
        } // switch
    } // for

    if (item->font->stringToCMap((const QChar *)reordered, len, item->glyphs, item->advances,
                                 &item->num_glyphs, item->flags & QTextEngine::RightToLeft) != QFontEngine::NoError)
        return FALSE;

    KHDEBUG("after shaping: len=%d", len);
    for (i = 0; i < len; i++) {
	item->attributes[i].mark = FALSE;
	item->attributes[i].clusterStart = FALSE;
	item->attributes[i].justification = 0;
	item->attributes[i].zeroWidth = FALSE;
	KHDEBUG("    %d: %4x property=%x", i, reordered[i], properties[i]);
    }

    // now we have the syllable in the right order, and can start running it through open type.

#ifndef QT_NO_XFTFREETYPE
    if (openType) {
	unsigned short logClusters[16];
	for (int i = 0; i < len; ++i)
	    logClusters[i] = i;

 	uint where[16];

        for (int i = 0; i < len; ++i) {
            where[i] = ~(PreSubstProperty
                         | BelowSubstProperty
                         | AboveSubstProperty
                         | PostSubstProperty
                         | CligProperty
                         | PositioningProperties);
            if (properties[i] == PreForm)
                where[i] &= ~PreFormProperty;
            else if (properties[i] == BelowForm)
                where[i] &= ~BelowFormProperty;
            else if (properties[i] == AboveForm)
                where[i] &= ~AboveFormProperty;
            else if (properties[i] == PostForm)
                where[i] &= ~PostFormProperty;
        }

        openType->shape(item, where);
        if (!openType->positionAndAdd(item, FALSE))
            return FALSE;
    } else
#endif
    {
	KHDEBUG("Not using openType");
	Q_UNUSED(openType);
    }

    item->attributes[0].clusterStart = TRUE;
    return TRUE;
}

static bool khmer_shape(QShaperItem *item)
{
    assert(item->script == QFont::Khmer);

#ifndef QT_NO_XFTFREETYPE
    QOpenType *openType = item->font->openType();
    if (openType && !openType->supportsScript(item->script))
        openType = 0;
#else
    QOpenType *openType = 0;
#endif
    unsigned short *logClusters = item->log_clusters;

    QShaperItem syllable = *item;
    int first_glyph = 0;

    int sstart = item->from;
    int end = sstart + item->length;
    KHDEBUG("khmer_shape: from %d length %d", item->from, item->length);
    while (sstart < end) {
        bool invalid;
        int send = khmer_nextSyllableBoundary(*item->string, sstart, end, &invalid);
        KHDEBUG("syllable from %d, length %d, invalid=%s", sstart, send-sstart,
               invalid ? "TRUE" : "FALSE");
        syllable.from = sstart;
        syllable.length = send-sstart;
        syllable.glyphs = item->glyphs + first_glyph;
        syllable.offsets = item->offsets + first_glyph;
        syllable.advances = item->advances + first_glyph;
        syllable.attributes = item->attributes + first_glyph;
        syllable.num_glyphs = item->num_glyphs - first_glyph;
        if (!khmer_shape_syllable(openType, &syllable)) {
            KHDEBUG("syllable shaping failed, syllable requests %d glyphs", syllable.num_glyphs);
            item->num_glyphs += syllable.num_glyphs;
            return FALSE;
        }
        item->has_positioning |= syllable.has_positioning;

        // fix logcluster array
        KHDEBUG("syllable:");
	int i;
        for (i = first_glyph; i < first_glyph + syllable.num_glyphs; ++i)
            KHDEBUG("        %d -> glyph %x", i, item->glyphs[i]);
        KHDEBUG("    logclusters:");
        for (i = sstart; i < send; ++i) {
            KHDEBUG("        %d -> glyph %d", i, first_glyph);
            logClusters[i-item->from] = first_glyph;
        }
        sstart = send;
        first_glyph += syllable.num_glyphs;
    }
    item->num_glyphs = first_glyph;
    return TRUE;
}

static void khmer_attributes( int script, const QString &text, int from, int len, QCharAttributes *attributes )
{
    Q_UNUSED(script);

    int end = from + len;
    const QChar *uc = text.unicode() + from;
    attributes += from;
    int i = 0;
    while ( i < len ) {
	bool invalid;
	int boundary = khmer_nextSyllableBoundary( text, from+i, end, &invalid ) - from;

	attributes[i].charStop = TRUE;

	if ( boundary > len-1 ) boundary = len;
	i++;
	while ( i < boundary ) {
	    attributes[i].charStop = FALSE;
	    ++uc;
	    ++i;
	}
	assert( i == boundary );
    }
}

// --------------------------------------------------------------------------------------------------------------------------------------------
//
// Myanmar
//
// --------------------------------------------------------------------------------------------------------------------------------------------

enum MymrCharClassValues
{
    Mymr_CC_RESERVED             =  0,
    Mymr_CC_CONSONANT            =  1, /* Consonant of type 1, that has subscript form */
    Mymr_CC_CONSONANT2           =  2, /* Consonant of type 2, that has no subscript form */
    Mymr_CC_NGA	          =  3, /* Consonant NGA */
    Mymr_CC_YA		          =  4, /* Consonant YA */
    Mymr_CC_RA		          =  5, /* Consonant RA */
    Mymr_CC_WA		          =  6, /* Consonant WA */
    Mymr_CC_HA		          =  7, /* Consonant HA */
    Mymr_CC_IND_VOWEL		  =  8, /* Independent vowel */
    Mymr_CC_ZERO_WIDTH_NJ_MARK   =  9, /* Zero Width non joiner character (0x200C) */
    Mymr_CC_VIRAMA               = 10, /* Subscript consonant combining character */
    Mymr_CC_PRE_VOWEL   	  = 11, /* Dependent vowel, prebase (Vowel e) */
    Mymr_CC_BELOW_VOWEL   	  = 12, /* Dependent vowel, prebase (Vowel u, uu) */
    Mymr_CC_ABOVE_VOWEL   	  = 13, /* Dependent vowel, prebase (Vowel i, ii, ai) */
    Mymr_CC_POST_VOWEL   	  = 14, /* Dependent vowel, prebase (Vowel aa) */
    Mymr_CC_SIGN_ABOVE           = 15,
    Mymr_CC_SIGN_BELOW           = 16,
    Mymr_CC_SIGN_AFTER           = 17,
    Mymr_CC_ZERO_WIDTH_J_MARK    = 18, /* Zero width joiner character */
    Mymr_CC_COUNT                = 19  /* This is the number of character classes */
};

enum MymrCharClassFlags
{
    Mymr_CF_CLASS_MASK    = 0x0000FFFF,

    Mymr_CF_CONSONANT     = 0x01000000,  /* flag to speed up comparing */
    Mymr_CF_MEDIAL	   = 0x02000000,  /* flag to speed up comparing */
    Mymr_CF_IND_VOWEL 	   = 0x04000000,  /* flag to speed up comparing */
    Mymr_CF_DEP_VOWEL 	   = 0x08000000,  /* flag to speed up comparing */
    Mymr_CF_DOTTED_CIRCLE = 0x10000000,  /* add a dotted circle if a character with this flag is the first in a syllable */
    Mymr_CF_VIRAMA        = 0x20000000,  /* flag to speed up comparing */

    /* position flags */
    Mymr_CF_POS_BEFORE    = 0x00080000,
    Mymr_CF_POS_BELOW     = 0x00040000,
    Mymr_CF_POS_ABOVE     = 0x00020000,
    Mymr_CF_POS_AFTER     = 0x00010000,
    Mymr_CF_POS_MASK      = 0x000f0000,

    Mymr_CF_AFTER_KINZI   = 0x00100000
};

/* Characters that get refrered to by name */
enum MymrChar
{
    Mymr_C_SIGN_ZWNJ     = 0x200C,
    Mymr_C_SIGN_ZWJ      = 0x200D,
    Mymr_C_DOTTED_CIRCLE = 0x25CC,
    Mymr_C_RA            = 0x101B,
    Mymr_C_YA            = 0x101A,
    Mymr_C_NGA           = 0x1004,
    Mymr_C_VOWEL_E       = 0x1031,
    Mymr_C_VIRAMA        = 0x1039
};

enum
{
    Mymr_xx = Mymr_CC_RESERVED,
    Mymr_c1 = Mymr_CC_CONSONANT | Mymr_CF_CONSONANT | Mymr_CF_POS_BELOW,
    Mymr_c2 = Mymr_CC_CONSONANT2 | Mymr_CF_CONSONANT,
    Mymr_ng = Mymr_CC_NGA | Mymr_CF_CONSONANT | Mymr_CF_POS_ABOVE,
    Mymr_ya = Mymr_CC_YA | Mymr_CF_CONSONANT | Mymr_CF_MEDIAL | Mymr_CF_POS_AFTER | Mymr_CF_AFTER_KINZI,
    Mymr_ra = Mymr_CC_RA | Mymr_CF_CONSONANT | Mymr_CF_MEDIAL | Mymr_CF_POS_BEFORE,
    Mymr_wa = Mymr_CC_WA | Mymr_CF_CONSONANT | Mymr_CF_MEDIAL | Mymr_CF_POS_BELOW,
    Mymr_ha = Mymr_CC_HA | Mymr_CF_CONSONANT | Mymr_CF_MEDIAL | Mymr_CF_POS_BELOW,
    Mymr_id = Mymr_CC_IND_VOWEL | Mymr_CF_IND_VOWEL,
    Mymr_vi = Mymr_CC_VIRAMA | Mymr_CF_VIRAMA | Mymr_CF_POS_ABOVE | Mymr_CF_DOTTED_CIRCLE,
    Mymr_dl = Mymr_CC_PRE_VOWEL | Mymr_CF_DEP_VOWEL | Mymr_CF_POS_BEFORE | Mymr_CF_DOTTED_CIRCLE | Mymr_CF_AFTER_KINZI,
    Mymr_db = Mymr_CC_BELOW_VOWEL | Mymr_CF_DEP_VOWEL | Mymr_CF_POS_BELOW | Mymr_CF_DOTTED_CIRCLE | Mymr_CF_AFTER_KINZI,
    Mymr_da = Mymr_CC_ABOVE_VOWEL | Mymr_CF_DEP_VOWEL | Mymr_CF_POS_ABOVE | Mymr_CF_DOTTED_CIRCLE | Mymr_CF_AFTER_KINZI,
    Mymr_dr = Mymr_CC_POST_VOWEL | Mymr_CF_DEP_VOWEL | Mymr_CF_POS_AFTER | Mymr_CF_DOTTED_CIRCLE | Mymr_CF_AFTER_KINZI,
    Mymr_sa = Mymr_CC_SIGN_ABOVE | Mymr_CF_DOTTED_CIRCLE | Mymr_CF_POS_ABOVE | Mymr_CF_AFTER_KINZI,
    Mymr_sb = Mymr_CC_SIGN_BELOW | Mymr_CF_DOTTED_CIRCLE | Mymr_CF_POS_BELOW | Mymr_CF_AFTER_KINZI,
    Mymr_sp = Mymr_CC_SIGN_AFTER | Mymr_CF_DOTTED_CIRCLE | Mymr_CF_AFTER_KINZI
};


typedef int MymrCharClass;


static const MymrCharClass mymrCharClasses[] =
{
    Mymr_c1, Mymr_c1, Mymr_c1, Mymr_c1, Mymr_ng, Mymr_c1, Mymr_c1, Mymr_c1,
    Mymr_c1, Mymr_c1, Mymr_c2, Mymr_c1, Mymr_c1, Mymr_c1, Mymr_c1, Mymr_c1, /* 1000 - 100F */
    Mymr_c1, Mymr_c1, Mymr_c1, Mymr_c1, Mymr_c1, Mymr_c1, Mymr_c1, Mymr_c1,
    Mymr_c1, Mymr_c1, Mymr_ya, Mymr_ra, Mymr_c1, Mymr_wa, Mymr_c1, Mymr_ha, /* 1010 - 101F */
    Mymr_c2, Mymr_c2, Mymr_xx, Mymr_id, Mymr_id, Mymr_id, Mymr_id, Mymr_id,
    Mymr_xx, Mymr_id, Mymr_id, Mymr_xx, Mymr_dr, Mymr_da, Mymr_da, Mymr_db, /* 1020 - 102F */
    Mymr_db, Mymr_dl, Mymr_da, Mymr_xx, Mymr_xx, Mymr_xx, Mymr_sa, Mymr_sb,
    Mymr_sp, Mymr_vi, Mymr_xx, Mymr_xx, Mymr_xx, Mymr_xx, Mymr_xx, Mymr_xx, /* 1030 - 103F */
    Mymr_xx, Mymr_xx, Mymr_xx, Mymr_xx, Mymr_xx, Mymr_xx, Mymr_xx, Mymr_xx,
    Mymr_xx, Mymr_xx, Mymr_xx, Mymr_xx, Mymr_xx, Mymr_xx, Mymr_xx, Mymr_xx, /* 1040 - 104F */
    Mymr_xx, Mymr_xx, Mymr_xx, Mymr_xx, Mymr_xx, Mymr_xx, Mymr_xx, Mymr_xx,
    Mymr_xx, Mymr_xx, Mymr_xx, Mymr_xx, Mymr_xx, Mymr_xx, Mymr_xx, Mymr_xx, /* 1050 - 105F */
};

static MymrCharClass
getMyanmarCharClass (const QChar &ch)
{
    if (ch.unicode() == Mymr_C_SIGN_ZWJ)
        return Mymr_CC_ZERO_WIDTH_J_MARK;

    if (ch.unicode() == Mymr_C_SIGN_ZWNJ)
        return Mymr_CC_ZERO_WIDTH_NJ_MARK;

    if (ch.unicode() < 0x1000 || ch.unicode() > 0x105f)
        return Mymr_CC_RESERVED;

    return mymrCharClasses[ch.unicode() - 0x1000];
}

static const signed char mymrStateTable[][Mymr_CC_COUNT] =
{
//   xx  c1, c2  ng  ya  ra  wa  ha  id zwnj vi  dl  db  da  dr  sa  sb  sp zwj
    { 1,  4,  4,  2,  4,  4,  4,  4, 24,  1, 27, 17, 18, 19, 20, 21,  1,  1,  4}, //  0 - ground state
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, //  1 - exit state (or sp to the right of the syllable)
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  3, 17, 18, 19, 20, 21, -1, -1,  4}, //  2 - NGA
    {-1,  4,  4,  4,  4,  4,  4,  4, -1, 23, -1, -1, -1, -1, -1, -1, -1, -1, -1}, //  3 - Virama after NGA
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  5, 17, 18, 19, 20, 21,  1,  1, -1}, //  4 - Base consonant
    {-2,  6, -2, -2,  7,  8,  9, 10, -2, 23, -2, -2, -2, -2, -2, -2, -2, -2, -2}, //  5 - First virama
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 25, 17, 18, 19, 20, 21, -1, -1, -1}, //  6 - c1 after virama
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 12, 17, 18, 19, 20, 21, -1, -1, -1}, //  7 - ya after virama
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 12, 17, 18, 19, 20, 21, -1, -1, -1}, //  8 - ra after virama
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 12, 17, 18, 19, 20, 21, -1, -1, -1}, //  9 - wa after virama
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 17, 18, 19, 20, 21, -1, -1, -1}, // 10 - ha after virama
    {-1, -1, -1, -1,  7,  8,  9, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, // 11 - Virama after NGA+zwj
    {-2, -2, -2, -2, -2, -2, 13, 14, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2}, // 12 - Second virama
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 15, 17, 18, 19, 20, 21, -1, -1, -1}, // 13 - wa after virama
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 17, 18, 19, 20, 21, -1, -1, -1}, // 14 - ha after virama
    {-2, -2, -2, -2, -2, -2, -2, 16, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2}, // 15 - Third virama
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 17, 18, 19, 20, 21, -1, -1, -1}, // 16 - ha after virama
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 20, 21,  1,  1, -1}, // 17 - dl, Dependent vowel e
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 19, -1, 21,  1,  1, -1}, // 18 - db, Dependent vowel u,uu
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  1,  1,  1, -1}, // 19 - da, Dependent vowel i,ii,ai
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 22, -1, -1, -1, -1, -1,  1,  1, -1}, // 20 - dr, Dependent vowel aa
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  1,  1, -1}, // 21 - sa, Sign anusvara
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, 23, -1, -1, -1, -1, -1, -1, -1, -1, -1}, // 22 - atha
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  1,  1, -1}, // 23 - zwnj for atha
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  1, -1}, // 24 - Independent vowel
    {-2, -2, -2, -2, 26, 26, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2}, // 25 - Virama after subscript consonant
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 12, 17, 18, 19, 20, 21, -1,  1, -1}, // 26 - ra/ya after subscript consonant + virama
    {-1,  6, -1, -1,  7,  8,  9, 10, -1, 23, -1, -1, -1, -1, -1, -1, -1, -1, -1}, // 27 - Virama after ground state
// exit state -2 is for invalid order of medials and combination of invalids
// with virama where virama should treat as start of next syllable
};



// #define MYANMAR_DEBUG
#ifdef MYANMAR_DEBUG
#define MMDEBUG qDebug
#else
#define MMDEBUG if(0) qDebug
#endif

//  Given an input string of characters and a location in which to start looking
//  calculate, using the state table, which one is the last character of the syllable
//  that starts in the starting position.
//
static inline int myanmar_nextSyllableBoundary(const QString &s, int start, int end, bool *invalid)
{
    *invalid = FALSE;
    const QChar *uc = s.unicode() + start;
    int state = 0;
    int pos = start;

    while (pos < end) {
        MymrCharClass charClass = getMyanmarCharClass(*uc);
        state = mymrStateTable[state][charClass & Mymr_CF_CLASS_MASK];
        if (pos == start)
            *invalid = charClass & Mymr_CF_DOTTED_CIRCLE;

        MMDEBUG("state[%d]=%d class=%8x (uc=%4x)", pos - start, state, charClass, uc->unicode() );

        if (state < 0) {
            if (state < -1)
                --pos;
            break;
        }
        ++uc;
        ++pos;
    }
    return pos;
}


#ifndef QT_NO_XFTFREETYPE
// ###### might have to change order of above and below forms and substitutions,
// but according to Unicode below comes before above
static const QOpenType::Features myanmar_features[] = {
    { FT_MAKE_TAG( 'p', 'r', 'e', 'f' ), PreFormProperty },
    { FT_MAKE_TAG( 'b', 'l', 'w', 'f' ), BelowFormProperty },
    { FT_MAKE_TAG( 'a', 'b', 'v', 'f' ), AboveFormProperty },
    { FT_MAKE_TAG( 'p', 's', 't', 'f' ), PostFormProperty },
    { FT_MAKE_TAG( 'p', 'r', 'e', 's' ), PreSubstProperty },
    { FT_MAKE_TAG( 'b', 'l', 'w', 's' ), BelowSubstProperty },
    { FT_MAKE_TAG( 'a', 'b', 'v', 's' ), AboveSubstProperty },
    { FT_MAKE_TAG( 'p', 's', 't', 's' ), PostSubstProperty },
    { FT_MAKE_TAG( 'r', 'l', 'i', 'g' ), CligProperty }, // Myanmar1 uses this instead of the other features
    { 0, 0 }
};
#endif


// Visual order before shaping should be:
//
//    [Vowel Mark E]
//    [Virama + Medial Ra]
//    [Base]
//    [Virama + Consonant]
//    [Nga + Virama] (Kinzi) ### should probably come before post forms (medial ya)
//    [Vowels]
//    [Marks]
//
// This means that we can keep the logical order apart from having to
// move the pre vowel, medial ra and kinzi

static bool myanmar_shape_syllable(QOpenType *openType, QShaperItem *item, bool invalid)
{
#ifndef QT_NO_XFTFREETYPE
    if (openType)
        openType->selectScript(QFont::Myanmar, myanmar_features);
#endif
    // according to the table the max length of a syllable should be around 14 chars
    assert(item->length < 32);

    MMDEBUG("\nsyllable from %d len %d, str='%s'", item->from, item->length,
	    item->string->mid(item->from, item->length).utf8().data());

    const QChar *uc = item->string->unicode() + item->from;
#ifdef MYANMAR_DEBUG
    qDebug("original:");
    for (int i = 0; i < item->length; i++) {
        qDebug("    %d: %4x", i, uc[i].unicode());
    }
#endif
    int vowel_e = -1;
    int kinzi = -1;
    int medial_ra = -1;
    int base = -1;
    int i;
    for (i = 0; i < item->length; ++i) {
        ushort chr = uc[i].unicode();

        if (chr == Mymr_C_VOWEL_E) {
            vowel_e = i;
            continue;
        }
        if (i == 0
            && chr == Mymr_C_NGA
            && i + 2 < item->length
            && uc[i+1].unicode() == Mymr_C_VIRAMA) {
            int mc = getMyanmarCharClass(uc[i+2]);
            //MMDEBUG("maybe kinzi: mc=%x", mc);
            if ((mc & Mymr_CF_CONSONANT) == Mymr_CF_CONSONANT) {
                kinzi = i;
                continue;
            }
        }
        if (base >= 0
            && chr == Mymr_C_VIRAMA
            && i + 1 < item->length
            && uc[i+1].unicode() == Mymr_C_RA) {
            medial_ra = i;
            continue;
        }
        if (base < 0)
            base = i;
    }

    MMDEBUG("\n  base=%d, vowel_e=%d, kinzi=%d, medial_ra=%d", base, vowel_e, kinzi, medial_ra);
    int len = 0;
    unsigned short reordered[32];
    unsigned char properties[32];
    enum {
	AboveForm = 0x01,
	PreForm = 0x02,
	PostForm = 0x04,
	BelowForm = 0x08
    };
    memset(properties, 0, 32*sizeof(unsigned char));

    // write vowel_e if found
    if (vowel_e >= 0) {
        reordered[0] = Mymr_C_VOWEL_E;
        len = 1;
    }
    // write medial_ra
    if (medial_ra >= 0) {
        reordered[len] = Mymr_C_VIRAMA;
        reordered[len+1] = Mymr_C_RA;
        properties[len] = PreForm;
        properties[len+1] = PreForm;
        len += 2;
    }

    // shall we add a dotted circle?
    // If in the position in which the base should be (first char in the string) there is
    // a character that has the Dotted circle flag (a character that cannot be a base)
    // then write a dotted circle
    if (invalid) {
        reordered[len] = C_DOTTED_CIRCLE;
        ++len;
    }

    bool lastWasVirama = FALSE;
    int basePos = -1;
    // copy the rest of the syllable to the output, inserting the kinzi
    // at the correct place
    for (i = 0; i < item->length; ++i) {
        if (i == vowel_e)
            continue;
        if (i == medial_ra || i == kinzi) {
            ++i;
            continue;
        }

        ushort chr = uc[i].unicode();
        MymrCharClass cc = getMyanmarCharClass(uc[i]);
        if (kinzi >= 0 && i > base && (cc & Mymr_CF_AFTER_KINZI)) {
            reordered[len] = Mymr_C_NGA;
            reordered[len+1] = Mymr_C_VIRAMA;
            properties[len-1] = AboveForm;
            properties[len] = AboveForm;
            len += 2;
            kinzi = -1;
        }

        if (lastWasVirama) {
            int prop = 0;
            switch(cc & Mymr_CF_POS_MASK) {
            case Mymr_CF_POS_BEFORE:
                prop = PreForm;
                break;
            case Mymr_CF_POS_BELOW:
                prop = BelowForm;
                break;
            case Mymr_CF_POS_ABOVE:
                prop = AboveForm;
                break;
            case Mymr_CF_POS_AFTER:
                prop = PostForm;
                break;
            default:
                break;
            }
            properties[len-1] = prop;
            properties[len] = prop;
            if(basePos >= 0 && basePos == len-2)
                properties[len-2] = prop;
        }
        lastWasVirama = (chr == Mymr_C_VIRAMA);
        if(i == base)
            basePos = len;

        if ((chr != Mymr_C_SIGN_ZWNJ && chr != Mymr_C_SIGN_ZWJ) || !len) {
            reordered[len] = chr;
            ++len;
        }
    }
    if (kinzi >= 0) {
        reordered[len] = Mymr_C_NGA;
        reordered[len+1] = Mymr_C_VIRAMA;
        properties[len] = AboveForm;
        properties[len+1] = AboveForm;
        len += 2;
    }

    if (item->font->stringToCMap((const QChar *)reordered, len, item->glyphs, item->advances,
                                 &item->num_glyphs, item->flags & QTextEngine::RightToLeft) != QFontEngine::NoError)
        return FALSE;

    MMDEBUG("after shaping: len=%d", len);
    for (i = 0; i < len; i++) {
	item->attributes[i].mark = FALSE;
	item->attributes[i].clusterStart = FALSE;
	item->attributes[i].justification = 0;
	item->attributes[i].zeroWidth = FALSE;
	MMDEBUG("    %d: %4x property=%x", i, reordered[i], properties[i]);
    }

    // now we have the syllable in the right order, and can start running it through open type.

#ifndef QT_NO_XFTFREETYPE
    if (openType) {
	unsigned short logClusters[32];
	for (int i = 0; i < len; ++i)
	    logClusters[i] = i;

 	uint where[32];

        for (int i = 0; i < len; ++i) {
            where[i] = ~(PreSubstProperty
                         | BelowSubstProperty
                         | AboveSubstProperty
                         | PostSubstProperty
                         | CligProperty
                         | PositioningProperties);
            if (properties[i] == PreForm)
                where[i] &= ~PreFormProperty;
            else if (properties[i] == BelowForm)
                where[i] &= ~BelowFormProperty;
            else if (properties[i] == AboveForm)
                where[i] &= ~AboveFormProperty;
            else if (properties[i] == PostForm)
                where[i] &= ~PostFormProperty;
        }

        openType->shape(item, where);
        if (!openType->positionAndAdd(item, FALSE))
            return FALSE;
    } else
#endif
    {
	MMDEBUG("Not using openType");
	Q_UNUSED(openType);
    }

    item->attributes[0].clusterStart = TRUE;
    return TRUE;
}

static bool myanmar_shape(QShaperItem *item)
{
    assert(item->script == QFont::Myanmar);

#ifndef QT_NO_XFTFREETYPE
    QOpenType *openType = item->font->openType();
    if (openType && !openType->supportsScript(item->script))
        openType = 0;
#else
    QOpenType *openType = 0;
#endif
    unsigned short *logClusters = item->log_clusters;

    QShaperItem syllable = *item;
    int first_glyph = 0;

    int sstart = item->from;
    int end = sstart + item->length;
    MMDEBUG("myanmar_shape: from %d length %d", item->from, item->length);
    while (sstart < end) {
        bool invalid;
        int send = myanmar_nextSyllableBoundary(*item->string, sstart, end, &invalid);
        MMDEBUG("syllable from %d, length %d, invalid=%s", sstart, send-sstart,
               invalid ? "TRUE" : "FALSE");
        syllable.from = sstart;
        syllable.length = send-sstart;
        syllable.glyphs = item->glyphs + first_glyph;
        syllable.offsets = item->offsets + first_glyph;
        syllable.advances = item->advances + first_glyph;
        syllable.attributes = item->attributes + first_glyph;
        syllable.num_glyphs = item->num_glyphs - first_glyph;
        if (!myanmar_shape_syllable(openType, &syllable, invalid)) {
            MMDEBUG("syllable shaping failed, syllable requests %d glyphs", syllable.num_glyphs);
            item->num_glyphs += syllable.num_glyphs;
            return FALSE;
        }
        item->has_positioning |= syllable.has_positioning;

        // fix logcluster array
        MMDEBUG("syllable:");
	int i;
        for (i = first_glyph; i < first_glyph + syllable.num_glyphs; ++i)
            MMDEBUG("        %d -> glyph %x", i, item->glyphs[i]);
        MMDEBUG("    logclusters:");
        for (i = sstart; i < send; ++i) {
            MMDEBUG("        %d -> glyph %d", i, first_glyph);
            logClusters[i-item->from] = first_glyph;
        }
        sstart = send;
        first_glyph += syllable.num_glyphs;
    }
    item->num_glyphs = first_glyph;
    return TRUE;
}

static void myanmar_attributes( int script, const QString &text, int from, int len, QCharAttributes *attributes )
{
    Q_UNUSED(script);

    int end = from + len;
    const QChar *uc = text.unicode() + from;
    attributes += from;
    int i = 0;
    while ( i < len ) {
	bool invalid;
	int boundary = myanmar_nextSyllableBoundary( text, from+i, end, &invalid ) - from;

	attributes[i].charStop = TRUE;
        attributes[i].softBreak = TRUE;

	if ( boundary > len-1 ) boundary = len;
	i++;
	while ( i < boundary ) {
	    attributes[i].charStop = FALSE;
            attributes[i].softBreak = FALSE;
	    ++uc;
	    ++i;
	}
	assert( i == boundary );
    }
}

// --------------------------------------------------------------------------------------------------------------------------------------------
//
// Hangul
//
// --------------------------------------------------------------------------------------------------------------------------------------------

// Hangul is a syllable based script. Unicode reserves a large range
// for precomposed hangul, where syllables are already precomposed to
// their final glyph shape. In addition, a so called jamo range is
// defined, that can be used to express old Hangul. Modern hangul
// syllables can also be expressed as jamo, and should be composed
// into syllables. The operation is rather simple and mathematical.

// Every hangul jamo is classified as being either a Leading consonant
// (L), and intermediat Vowel (V) or a trailing consonant (T). Modern
// hangul syllables (the ones in the precomposed area can be of type
// LV or LVT.
//
// Syllable breaks do _not_ occur between:
//
// L              L, V or precomposed
// V, LV          V, T
// LVT, T         T
//
// A standard syllable is of the form L+V+T*. The above rules allow
// nonstandard syllables L*V*T*. To transform them into standard
// syllables fill characers L_f and V_f can be inserted.

enum {
    Hangul_SBase = 0xac00,
    Hangul_LBase = 0x1100,
    Hangul_VBase = 0x1161,
    Hangul_TBase = 0x11a7,
    Hangul_SCount = 11172,
    Hangul_LCount = 19,
    Hangul_VCount = 21,
    Hangul_TCount = 28,
    Hangul_NCount = 21*28
};

static inline bool hangul_isPrecomposed(unsigned short uc) {
    return (uc >= Hangul_SBase && uc < Hangul_SBase + Hangul_SCount);
}

static inline bool hangul_isLV(unsigned short uc) {
    return ((uc - Hangul_SBase) % Hangul_TCount == 0);
}

enum HangulType {
    L,
    V,
    T,
    LV,
    LVT,
    X
};

static inline HangulType hangul_type(unsigned short uc) {
    if (uc > Hangul_SBase && uc < Hangul_SBase + Hangul_SCount)
        return hangul_isLV(uc) ? LV : LVT;
    if (uc < Hangul_LBase || uc > 0x11ff)
        return X;
    if (uc < Hangul_VBase)
        return L;
    if (uc < Hangul_TBase)
        return V;
    return T;
}

static int hangul_nextSyllableBoundary(const QString &s, int start, int end)
{
    const QChar *uc = s.unicode() + start;

    HangulType state = hangul_type(uc->unicode());
    int pos = 1;

    while (pos < end - start) {
        HangulType newState = hangul_type(uc[pos].unicode());
        switch(newState) {
        case X:
            goto finish;
        case L:
        case V:
        case T:
            if (state > newState)
                goto finish;
            state = newState;
            break;
        case LV:
            if (state > L)
                goto finish;
            state = V;
            break;
        case LVT:
            if (state > L)
                goto finish;
            state = T;
        }
        ++pos;
    }

 finish:
    return start+pos;
}

#ifndef QT_NO_XFTFREETYPE
static const QOpenType::Features hangul_features [] = {
    { FT_MAKE_TAG('c', 'c', 'm', 'p'), CcmpProperty },
    { FT_MAKE_TAG('l', 'j', 'm', 'o'), CcmpProperty },
    { FT_MAKE_TAG('j', 'j', 'm', 'o'), CcmpProperty },
    { FT_MAKE_TAG('t', 'j', 'm', 'o'), CcmpProperty },
    { 0, 0 }
};
#endif

static bool hangul_shape_syllable(QOpenType *openType, QShaperItem *item)
{
    Q_UNUSED(openType)
    const QChar *ch = item->string->unicode() + item->from;

    int i;
    unsigned short composed = 0;
    // see if we can compose the syllable into a modern hangul
    if (item->length == 2) {
        int LIndex = ch[0].unicode() - Hangul_LBase;
        int VIndex = ch[1].unicode() - Hangul_VBase;
        if (LIndex >= 0 && LIndex < Hangul_LCount &&
            VIndex >= 0 && VIndex < Hangul_VCount)
            composed = (LIndex * Hangul_VCount + VIndex) * Hangul_TCount + Hangul_SBase;
    } else if (item->length == 3) {
        int LIndex = ch[0].unicode() - Hangul_LBase;
        int VIndex = ch[1].unicode() - Hangul_VBase;
        int TIndex = ch[2].unicode() - Hangul_TBase;
        if (LIndex >= 0 && LIndex < Hangul_LCount &&
            VIndex >= 0 && VIndex < Hangul_VCount &&
            TIndex >= 0 && TIndex < Hangul_TCount)
            composed = (LIndex * Hangul_VCount + VIndex) * Hangul_TCount + TIndex + Hangul_SBase;
    }


    int len = item->length;
    QChar c(composed);

    // ### icc says 'chars' is unused
    // const QChar *chars = ch;

    // if we have a modern hangul use the composed form
    if (composed) {
        // chars = &c;
        len = 1;
    }

    if (item->font->stringToCMap(ch, len, item->glyphs, item->advances,
                                 &item->num_glyphs, item->flags & QTextEngine::RightToLeft) != QFontEngine::NoError)
        return FALSE;
    for (i = 0; i < len; i++) {
        item->attributes[i].mark = FALSE;
        item->attributes[i].clusterStart = FALSE;
        item->attributes[i].justification = 0;
        item->attributes[i].zeroWidth = FALSE;
        IDEBUG("    %d: %4x", i, ch[i].unicode());
    }

#ifndef QT_NO_XFTFREETYPE
    if (openType && !composed) {

        QVarLengthArray<unsigned short> logClusters(len);
        for (i = 0; i < len; ++i)
            logClusters[i] = i;
        item->log_clusters = logClusters.data();

        openType->shape(item);
        if (!openType->positionAndAdd(item, FALSE))
            return FALSE;

    }
#endif

    item->attributes[0].clusterStart = TRUE;
    return TRUE;
}

static bool hangul_shape(QShaperItem *item)
{
    Q_ASSERT(item->script == QFont::Hangul);

    const QChar *uc = item->string->unicode() + item->from;

    bool allPrecomposed = TRUE;
    for (int i = 0; i < item->length; ++i) {
        if (!hangul_isPrecomposed(uc[i].unicode())) {
            allPrecomposed = FALSE;
            break;
        }
    }

    if (!allPrecomposed) {
#ifndef QT_NO_XFTFREETYPE
        QOpenType *openType = item->font->openType();
        if (openType && !openType->supportsScript(item->script))
            openType = 0;
        if (openType)
            openType->selectScript(QFont::Hangul, hangul_features);
#else
        QOpenType *openType = 0;
#endif

        unsigned short *logClusters = item->log_clusters;

        QShaperItem syllable = *item;
        int first_glyph = 0;

        int sstart = item->from;
        int end = sstart + item->length;
        while (sstart < end) {
            int send = hangul_nextSyllableBoundary(*(item->string), sstart, end);

            syllable.from = sstart;
            syllable.length = send-sstart;
            syllable.glyphs = item->glyphs + first_glyph;
            syllable.offsets = item->offsets + first_glyph;
            syllable.advances = item->advances + first_glyph;
            syllable.attributes = item->attributes + first_glyph;
            syllable.num_glyphs = item->num_glyphs - first_glyph;
            if (!hangul_shape_syllable(openType, &syllable)) {
                item->num_glyphs += syllable.num_glyphs;
                return FALSE;
            }
            item->has_positioning |= syllable.has_positioning;
            // fix logcluster array
            for (int i = sstart; i < send; ++i)
                logClusters[i-item->from] = first_glyph;
            sstart = send;
            first_glyph += syllable.num_glyphs;
        }
        item->num_glyphs = first_glyph;
        return TRUE;
    }

    return basic_shape(item);
}

static void hangul_attributes(int script, const QString &text, int from, int len, QCharAttributes *attributes)
{
    Q_UNUSED(script);

    int end = from + len;
    const QChar *uc = text.unicode() + from;
    attributes += from;
    int i = 0;
    while (i < len) {
        int boundary = hangul_nextSyllableBoundary(text, from+i, end) - from;

        attributes[i].charStop = TRUE;

        if (boundary > len-1) boundary = len;
        i++;
        while (i < boundary) {
            attributes[i].charStop = FALSE;
            ++uc;
            ++i;
        }
        assert(i == boundary);
    }
}

// -----------------------------------------------------------------------------------------------
//
// The script engine jump table
//
// -----------------------------------------------------------------------------------------------

const q_scriptEngine scriptEngines[] = {
	// Latin,
    { basic_shape, 0 },
	// Greek,
    { basic_shape, 0 },
	// Cyrillic,
    { basic_shape, 0 },
	// Armenian,
    { basic_shape, 0 },
	// Georgian,
    { basic_shape, 0 },
	// Runic,
    { basic_shape, 0 },
	// Ogham,
    { basic_shape, 0 },
	// SpacingModifiers,
    { basic_shape, 0 },
	// CombiningMarks,
    { basic_shape, 0 },

	// // Middle Eastern Scripts
	// Hebrew,
    { hebrew_shape, 0 },
	// Arabic,
    { arabic_shape, 0 },
	// Syriac,
    { syriac_shape, 0 },
	// Thaana,
    { thaana_shape, 0 },

	// // South and Southeast Asian Scripts
	// Devanagari,
    { indic_shape, indic_attributes },
	// Bengali,
    { indic_shape, indic_attributes },
	// Gurmukhi,
    { indic_shape, indic_attributes },
	// Gujarati,
    { indic_shape, indic_attributes },
	// Oriya,
    { indic_shape, indic_attributes },
	// Tamil,
    { indic_shape, indic_attributes },
	// Telugu,
    { indic_shape, indic_attributes },
	// Kannada,
    { indic_shape, indic_attributes },
	// Malayalam,
    { indic_shape, indic_attributes },
	// Sinhala,
    { indic_shape, indic_attributes },
	// Thai,
    { basic_shape, thai_attributes },
	// Lao,
    { basic_shape, thai_attributes },
	// Tibetan,
    { tibetan_shape, tibetan_attributes },
	// Myanmar,
    { myanmar_shape, myanmar_attributes },
	// Khmer,
    { khmer_shape, khmer_attributes },

	// // East Asian Scripts
	// Han,
    { basic_shape, 0 },
	// Hiragana,
    { basic_shape, 0 },
	// Katakana,
    { basic_shape, 0 },
	// Hangul,
    { hangul_shape, hangul_attributes },
	// Bopomofo,
    { basic_shape, 0 },
	// Yi,
    { basic_shape, 0 },

	// // Additional Scripts
	// Ethiopic,
    { basic_shape, 0 },
	// Cherokee,
    { basic_shape, 0 },
	// CanadianAboriginal,
    { basic_shape, 0 },
	// Mongolian,
    { basic_shape, 0 },

	// // Symbols
	// CurrencySymbols,
    { basic_shape, 0 },
	// LetterlikeSymbols,
    { basic_shape, 0 },
	// NumberForms,
    { basic_shape, 0 },
	// MathematicalOperators,
    { basic_shape, 0 },
	// TechnicalSymbols,
    { basic_shape, 0 },
	// GeometricSymbols,
    { basic_shape, 0 },
	// MiscellaneousSymbols,
    { basic_shape, 0 },
	// EnclosedAndSquare,
    { basic_shape, 0 },
	// Braille,
    { basic_shape, 0 },

	// Unicode,
    { basic_shape, 0 },
    //Tagalog,
    { basic_shape, 0 },
    //Hanunoo,
    { basic_shape, 0 },
    //Buhid,
    { basic_shape, 0 },
    //Tagbanwa,
    { basic_shape, 0 },
    // KatakanaHalfWidth
    { basic_shape, 0 },
    // Limbu
    { basic_shape, 0 },
    // TaiLe
    { basic_shape, 0 }
};
