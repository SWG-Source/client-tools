/****************************************************************************
** $Id: qscriptengine_win.cpp 1964 2006-12-15 12:14:12Z chehrlic $
**
** Continuation of middle eastern languages
**
** Created 20040828
**
** Copyright (C) 2004 Peter Kuemmel
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about Qt Commercial License Agreements.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** Contact kde-cygwin@kde.org if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

const q_scriptEngine scriptEngines[] =
    {
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
        { basic_shape, 0 },
        // Thaana,
        { basic_shape, 0 },

        // // South and Southeast Asian Scripts
        // Devanagari,
        { basic_shape, 0 },
        // Bengali,
        { basic_shape, 0 },
        // Gurmukhi,
        { basic_shape, 0 },
        // Gujarati,
        { basic_shape, 0 },
        // Oriya,
        { basic_shape, 0 },
        // Tamil,
        { basic_shape, 0 },
        // Telugu,
        { basic_shape, 0 },
        // Kannada,
        { basic_shape, 0 },
        // Malayalam,
        { basic_shape, 0 },
        // Sinhala,
        { basic_shape, 0 },
        // Thai,
        { basic_shape, 0 },
        // Lao,
        { basic_shape, 0 },
        // Tibetan,
        { basic_shape, 0 },
        // Myanmar,
        { basic_shape, 0 },
        // Khmer,
        { basic_shape, 0 },

        // // East Asian Scripts
        // Han,
        { basic_shape, 0 },
        // Hiragana,
        { basic_shape, 0 },
        // Katakana,
        { basic_shape, 0 },
        // Hangul,
        { basic_shape, 0 },
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
        { basic_shape, 0 }
    };
