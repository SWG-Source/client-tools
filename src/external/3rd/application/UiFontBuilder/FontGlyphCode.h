// ======================================================================
//
// FontGlpyhCode.h
//
// This code was retrieved from a public Microsoft web page at 
// http://support.microsoft.com/default.aspx?scid=kb;EN-US;241020
//
// ======================================================================

#ifndef FONT_GLYPH_CODE_H
#define FONT_GLYPH_CODE_H

//Given a device context that has a font selected, and a unicode character,
// return the Glyph Index into the font.  Returns 0 if that font does not
// contain the given character.
USHORT GetTTUnicodeGlyphIndex(HDC hdc, USHORT ch);

//Given a device context that has a font selected, return the number of unicode
// glyphs contained in the font.
USHORT GetTTUnicodeCharCount(HDC hdc);

#endif