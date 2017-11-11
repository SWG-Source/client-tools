
#include "stdafx.h"

#include <windows.h>
#include "Unicode.h"
#include <vector>
#include <set>

#pragma pack(1)     // for byte alignment
// We need byte alignment to be structure compatible with the
// contents of a TrueType font file

// Macros to swap from Big Endian to Little Endian
#define SWAPWORD(x) MAKEWORD( \
    HIBYTE(x), \
    LOBYTE(x) \
    )
#define SWAPLONG(x) MAKELONG( \
    SWAPWORD(HIWORD(x)), \
    SWAPWORD(LOWORD(x)) \
    )


typedef struct _CMap4   // From the TrueType Spec. revision 1.66
{
    USHORT format;          // Format number is set to 4.
    USHORT length;          // Length in bytes.
    USHORT version;         // Version number (starts at 0).
    USHORT segCountX2;      // 2 x segCount.
    USHORT searchRange;     // 2 x (2**floor(log2(segCount)))
    USHORT entrySelector;   // log2(searchRange/2)
    USHORT rangeShift;      // 2 x segCount - searchRange

    USHORT Arrays[1];       // Placeholder symbol for address of arrays following
} CMAP4, *LPCMAP4;


/*  CMAP table Data
    From the TrueType Spec revision 1.66

    USHORT  Table Version #
    USHORT  Number of encoding tables
*/
#define     CMAPHEADERSIZE  (sizeof(USHORT)*2)


/*  ENCODING entry Data aka CMAPENCODING
    From the TrueType Spec revision 1.66

    USHORT  Platform Id
    USHORT  Platform Specific Encoding Id
    ULONG   Byte Offset from beginning of table
*/
#define     ENCODINGSIZE    (sizeof(USHORT)*2 + sizeof(ULONG))

typedef struct _CMapEncoding
{
    USHORT  PlatformId;
    USHORT  EncodingId;
    ULONG   Offset;
} CMAPENCODING;


// Macro to pack a TrueType table name into a DWORD
#define     MAKETABLENAME(ch1, ch2, ch3, ch4) (\
    (((DWORD)(ch4)) << 24) | \
    (((DWORD)(ch3)) << 16) | \
    (((DWORD)(ch2)) << 8) | \
    ((DWORD)(ch1)) \
    )



// DWORD packed four letter table name for each GetFontData()
// function call when working with the CMAP TrueType table
DWORD dwCmapName = MAKETABLENAME( 'c','m','a','p' );

USHORT *GetEndCountArray(LPBYTE pBuff)
{
    return (USHORT *)(pBuff + 7 * sizeof(USHORT));  // Per TT spec
}

USHORT *GetStartCountArray(LPBYTE pBuff)
{
    DWORD   segCount = ((LPCMAP4)pBuff)->segCountX2/2;
    return (USHORT *)( pBuff +
        8 * sizeof(USHORT) +        // 7 header + 1 reserved USHORT
        segCount*sizeof(USHORT) );  // Per TT spec
}

USHORT *GetIdDeltaArray(LPBYTE pBuff)
{
    DWORD   segCount = ((LPCMAP4)pBuff)->segCountX2/2;
    return (USHORT *)( pBuff +
        8 * sizeof(USHORT) +        // 7 header + 1 reserved USHORT
        segCount * 2 * sizeof(USHORT) );    // Per TT spec
}

USHORT *GetIdRangeOffsetArray(LPBYTE pBuff)
{
    DWORD   segCount = ((LPCMAP4)pBuff)->segCountX2/2;
    return (USHORT *)( pBuff +
        8 * sizeof(USHORT) +        // 7 header + 1 reserved USHORT
        segCount * 3 * sizeof(USHORT) );    // Per TT spec
}


void SwapArrays( LPCMAP4 pFormat4 )
{
    DWORD   segCount = pFormat4->segCountX2/2;  // Per TT Spec
    DWORD   i;
    USHORT  *pGlyphId,
            *pEndOfBuffer,
            *pstartCount    = GetStartCountArray( (LPBYTE)pFormat4 ),
            *pidDelta       = GetIdDeltaArray( (LPBYTE)pFormat4 ),
            *pidRangeOffset = GetIdRangeOffsetArray( (LPBYTE)pFormat4 ),
            *pendCount      = GetEndCountArray( (LPBYTE)pFormat4 );

    // Swap the array elements for Intel.
    for (i=0; i < segCount; i++)
    {
        pendCount[i] = SWAPWORD(pendCount[i]);
        pstartCount[i] = SWAPWORD(pstartCount[i]);
        pidDelta[i] = SWAPWORD(pidDelta[i]);
        pidRangeOffset[i] = SWAPWORD(pidRangeOffset[i]);
    }

    // Swap the Glyph Id array
    pGlyphId = pidRangeOffset + segCount;   // Per TT spec
    pEndOfBuffer = (USHORT*)((LPBYTE)pFormat4 + pFormat4->length);
    for (;pGlyphId < pEndOfBuffer; pGlyphId++)
    {
        *pGlyphId = SWAPWORD(*pGlyphId);
    }
} /* end of function SwapArrays */


BOOL GetFontEncoding (
    HDC hdc,
    CMAPENCODING * pEncoding,
    int iEncoding
    )
/*
    Note for this function to work correctly, structures must
    have byte alignment.
*/
{
    DWORD   dwResult;
    BOOL    fSuccess = TRUE;

    // Get the structure data from the TrueType font
    dwResult = GetFontData (
        hdc,
        dwCmapName,
        CMAPHEADERSIZE + ENCODINGSIZE*iEncoding,
        pEncoding,
        sizeof(CMAPENCODING) );
    fSuccess = (dwResult == sizeof(CMAPENCODING));

    // swap the Platform Id for Intel
    pEncoding->PlatformId = SWAPWORD(pEncoding->PlatformId);

    // swap the Specific Id for Intel
    pEncoding->EncodingId = SWAPWORD(pEncoding->EncodingId);

    // swap the subtable offset for Intel
    pEncoding->Offset = SWAPLONG(pEncoding->Offset);

    return fSuccess;

} /* end of function GetFontEncoding */

BOOL GetFontFormat4Header (
    HDC hdc,
    LPCMAP4 pFormat4,
    DWORD dwOffset
    )
/*
    Note for this function to work correctly, structures must
    have byte alignment.
*/
{
    BOOL    fSuccess = TRUE;
    DWORD   dwResult;
    int     i;
    USHORT  *pField;

    // Loop and Alias a writeable pointer to the field of interest
    pField = (USHORT *)pFormat4;

    for (i=0; i < 7; i++)
    {
        // Get the field from the subtable
        dwResult = GetFontData (
            hdc,
            dwCmapName,
            dwOffset + sizeof(USHORT)*i,
            pField,
            sizeof(USHORT) );

        // swap it to make it right for Intel.
        *pField = SWAPWORD(*pField);
        // move on to the next
        pField++;
        // accumulate our success
        fSuccess = (dwResult == sizeof(USHORT)) && fSuccess;
    }

    return fSuccess;

} /* end of function GetFontFormat4Header */

BOOL GetFontFormat4Subtable (
    HDC hdc,                    // DC with TrueType font
    LPCMAP4 pFormat4Subtable,   // destination buffer
    DWORD   dwOffset            // Offset within font
    )
{
    DWORD   dwResult;
    USHORT  length;


    // Retrieve the header values in swapped order
    if (!GetFontFormat4Header ( hdc,
        pFormat4Subtable,
        dwOffset ))
    {
        return FALSE;
    }

    // Get the rest of the table
    length = (USHORT)(pFormat4Subtable->length - (7 * sizeof(USHORT)));
    dwResult = GetFontData( hdc,
        dwCmapName,
        dwOffset + 7 * sizeof(USHORT),      // pos of arrays
        (LPBYTE)pFormat4Subtable->Arrays,   // destination
        length );

    if ( dwResult != length)
    {
        // We really shouldn't ever get here
        return FALSE;
    }

    // Swamp the arrays
    SwapArrays( pFormat4Subtable );

    return TRUE;
}

USHORT GetFontFormat4CharCount (
    LPCMAP4 pFormat4    // pointer to a valid Format4 subtable
    )
{
    USHORT  i,
            *pendCount = GetEndCountArray((LPBYTE) pFormat4),
            *pstartCount = GetStartCountArray((LPBYTE) pFormat4),
            *idRangeOffset = GetIdRangeOffsetArray( (LPBYTE) pFormat4 );

    // Count the # of glyphs
    USHORT nGlyphs = 0;

    if ( pFormat4 == NULL )
        return 0;

    // by adding up the coverage of each segment
    for (i=0; i < (pFormat4->segCountX2/2); i++)
    {

        if ( idRangeOffset[i] == 0)
        {
            // if per the TT spec, the idRangeOffset element is zero,
            // all of the characters in this segment exist.
            nGlyphs = (USHORT)(nGlyphs + (USHORT)(pendCount[i] - pstartCount[i] + ((USHORT)1)));
        }
        else
        {
            // otherwise we have to test for glyph existence for
            // each character in the segment.
            USHORT idResult;    //Intermediate id calc.
            USHORT ch;

            for (ch = pstartCount[i]; ch <= pendCount[i]; ch++)
            {
                // determine if a glyph exists
                idResult = *(
                    idRangeOffset[i]/2 +
                    (ch - pstartCount[i]) +
                    &idRangeOffset[i]
                    );  // indexing equation from TT spec
                if (idResult != 0)
                    // Yep, count it.
                    nGlyphs++;
            }
        }
    }

    return nGlyphs;
} /* end of function GetFontFormat4CharCount */

BOOL GetTTUnicodeCoverage (
    HDC hdc,            // DC with TT font
    LPCMAP4 pBuffer,    // Properly allocated buffer
    DWORD cbSize,       // Size of properly allocated buffer
    DWORD *pcbNeeded    // size of buffer needed
    )
/*
    if cbSize is to small or zero, or if pBuffer is NULL the function
    will fail and return the required buffer size in *pcbNeeded.

    if another error occurs, the function will fail and *pcbNeeded will
    be zero.

    When the function succeeds, *pcbNeeded contains the number of bytes
    copied to pBuffer.
*/
{
    USHORT          nEncodings;     // # of encoding in the TT font
    CMAPENCODING    Encoding;       // The current encoding
    DWORD           dwResult;
    DWORD           i,
                    iUnicode;       // The Unicode encoding
    CMAP4           Format4;        // Unicode subtable format
    LPCMAP4         pFormat4Subtable;   // Working buffer for subtable	

	Encoding.Offset = 0;

    // Get the number of subtables in the CMAP table from the CMAP header
    // The # of subtables is the second USHORT in the CMAP table, per the TT Spec.
    dwResult = GetFontData ( hdc, dwCmapName, sizeof(USHORT), &nEncodings, sizeof(USHORT) );
    nEncodings = SWAPWORD(nEncodings);

    if ( dwResult != sizeof(USHORT) )
    {
        // Something is wrong, we probably got GDI_ERROR back
        // Probably this means that the Device Context does not have
        // a TrueType font selected into it.
        return FALSE;
    }

    // Get the encodings and look for a Unicode Encoding
    iUnicode = nEncodings;
    for (i=0; i < nEncodings; i++)
    {
        // Get the encoding entry for each encoding
        if (!GetFontEncoding ( hdc, &Encoding, i ))
        {
            *pcbNeeded = 0;
            return FALSE;
        }

        // Take note of the Unicode encoding.
        //
        // A Unicode encoding per the TrueType specification has a
        // Platform Id of 3 and a Platform specific encoding id of 1
        // Note that Symbol fonts are supposed to have a Platform Id of 3
        // and a specific id of 0. If the TrueType spec. suggestions were
        // followed then the Symbol font's Format 4 encoding could also
        // be considered Unicode because the mapping would be in the
        // Private Use Area of Unicode. We assume this here and allow
        // Symbol fonts to be interpreted. If they do not contain a
        // Format 4, we bail later. If they do not have a Unicode
        // character mapping, we'll get wrong results.
        // Code could infer from the coverage whether 3-0 fonts are
        // Unicode or not by examining the segments for placement within
        // the Private Use Area Subrange.
        if (Encoding.PlatformId == 3 &&
            (Encoding.EncodingId == 1 || Encoding.EncodingId == 0) )
        {
            iUnicode = i;       // Set the index to the Unicode encoding
        }
    }

    // index out of range means failure to find a Unicode mapping
    if (iUnicode >= nEncodings)
    {
        // No Unicode encoding found.
        *pcbNeeded = 0;
        return FALSE;
    }

    // Get the header entries(first 7 USHORTs) for the Unicode encoding.
    if ( !GetFontFormat4Header ( hdc, &Format4, Encoding.Offset ) )
    {
        *pcbNeeded = 0;
        return FALSE;
    }

    // Check to see if we retrieved a Format 4 table
    if ( Format4.format != 4 )
    {
        // Bad, subtable is not format 4, bail.
        // This could happen if the font is corrupt
        // It could also happen if there is a new font format we
        // don't understand.
        *pcbNeeded = 0;
        return FALSE;
    }

    // Figure buffer size and tell caller if buffer to small
    *pcbNeeded = Format4.length;
    if (*pcbNeeded > cbSize || pBuffer == NULL)
    {
        // Either test indicates caller needs to know
        // the buffer size and the parameters are not setup
        // to continue.
        return FALSE;
    }

    // allocate a full working buffer
    pFormat4Subtable = (LPCMAP4)malloc ( Format4.length );
    if ( pFormat4Subtable == NULL)
    {
        // Bad things happening if we can't allocate memory
        *pcbNeeded = 0;
        return FALSE;
    }

    // get the entire subtable
    if (!GetFontFormat4Subtable ( hdc, pFormat4Subtable, Encoding.Offset ))
    {
        // Bad things happening if we can't allocate memory
        *pcbNeeded = 0;
        return FALSE;
    }

    // Copy the retrieved table into the buffer
    CopyMemory( pBuffer,
        pFormat4Subtable,
        pFormat4Subtable->length );

    free ( pFormat4Subtable );
    return TRUE;
} /* end of function GetTTUnicodeCoverage */

BOOL FindFormat4Segment (
    LPCMAP4 pTable,     // a valid Format4 subtable buffer
    USHORT ch,          // Unicode character to search for
    USHORT *piSeg       // out: index of segment containing ch
    )
/*
    if the Unicode character ch is not contained in one of the
    segments the function returns FALSE.

    if the Unicode character ch is found in a segment, the index
    of the segment is placed in*piSeg and the function returns
    TRUE.
*/
{
    USHORT  i,
            segCount = (USHORT)(pTable->segCountX2/2);
    USHORT  *pendCount = GetEndCountArray((LPBYTE) pTable);
    USHORT  *pstartCount = GetStartCountArray((LPBYTE) pTable);

    // Find segment that could contain the Unicode character code
    for (i=0; i < segCount && pendCount[i] < ch; i++);

    // We looked in them all, ch not there
    if (i >= segCount)
        return FALSE;

    // character code not within the range of the segment
    if (pstartCount[i] > ch)
        return FALSE;

    // this segment contains the character code
    *piSeg = i;
    return TRUE;
} /* end of function FindFormat4Segment */

USHORT GetTTUnicodeCharCount (
    HDC hdc
    )
/*
    Returns the number of Unicode character glyphs that
    are in the TrueType font that is selected into the hdc.
*/
{
    LPCMAP4 pUnicodeCMapTable;
    USHORT  cChar;
    DWORD   dwSize;

    // Get the Unicode CMAP table from the TT font
    GetTTUnicodeCoverage( hdc, NULL, 0, &dwSize );
    pUnicodeCMapTable = (LPCMAP4)malloc( dwSize );
    if (!GetTTUnicodeCoverage( hdc, pUnicodeCMapTable, dwSize, &dwSize ))
    {
        // possibly no Unicode cmap, not a TT font selected,...
        free( pUnicodeCMapTable );
        return 0;
    }

    cChar = GetFontFormat4CharCount( pUnicodeCMapTable );
    free( pUnicodeCMapTable );

    return cChar;
} /* end of function GetTTUnicodeCharCount */


USHORT GetTTUnicodeGlyphIndex (
    HDC hdc,        // DC with a TrueType font selected
    USHORT ch       // Unicode character to convert to Index
    )
/*
    When the TrueType font contains a glyph for ch, the
    function returns the glyph index for that character.

    If an error occurs, or there is no glyph for ch, the
    function will return the missing glyph index of zero.
*/
{
    LPCMAP4 pUnicodeCMapTable;
    DWORD   dwSize;
    USHORT  iSegment;
    USHORT  *idRangeOffset;
    USHORT  *idDelta;
    USHORT  *startCount;
    USHORT  GlyphIndex = 0;     // Initialize to missing glyph

    // How big a buffer do we need for Unicode CMAP?
    GetTTUnicodeCoverage( hdc, NULL, 0, &dwSize );
    pUnicodeCMapTable = (LPCMAP4)malloc( dwSize );
    if (!GetTTUnicodeCoverage( hdc, pUnicodeCMapTable, dwSize, &dwSize ))
    {
        // Either no Unicode cmap, or some other error occurred
        // like font in DC is not TT.
        free( pUnicodeCMapTable );
        return 0;       // return missing glyph on error
    }

    // Find the cmap segment that has the character code.
    if (!FindFormat4Segment( pUnicodeCMapTable, ch, &iSegment ))
    {
        free( pUnicodeCMapTable );
        return 0;       // ch not in cmap, return missing glyph
    }

    // Get pointers to the cmap data
    idRangeOffset = GetIdRangeOffsetArray( (LPBYTE) pUnicodeCMapTable );
    idDelta = GetIdDeltaArray( (LPBYTE) pUnicodeCMapTable );
    startCount = GetStartCountArray( (LPBYTE) pUnicodeCMapTable );

    // Per TT spec, if the RangeOffset is zero,
    if ( idRangeOffset[iSegment] == 0)
    {
        // calculate the glyph index directly
        GlyphIndex = (USHORT)((idDelta[iSegment] + ch) % 65536);
    }
    else
    {
        // otherwise, use the glyph id array to get the index
        USHORT idResult;    //Intermediate id calc.

        idResult = *(
            idRangeOffset[iSegment]/2 +
            (ch - startCount[iSegment]) +
            &idRangeOffset[iSegment]
            );  // indexing equation from TT spec
        if (idResult)
            // Per TT spec, nonzero means there is a glyph
            GlyphIndex = (USHORT)((idDelta[iSegment] + idResult) % 65536);
        else
            // otherwise, return the missing glyph
            GlyphIndex = 0;
    }

    free( pUnicodeCMapTable );
    return GlyphIndex;
} /* end of function GetTTUnicodeGlyphIndex */