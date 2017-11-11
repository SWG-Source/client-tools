/*.T(32)  BMP.H  Include file for BMP support     */
// PICTools ProductVersion 2.00.567

/*  +---------------------------------------------------------------------------+*/
/*  | Copyright (C) 1993-2008 Pegasus Imaging Corporation  All rights reserved. |*/
/*  +---------------------------------------------------------------------------+*/

#if !defined(_BMP)
#define _BMP

#ifdef __MWERKS__
    /*#if PRAGMA_ALIGN_SUPPORTED*/
    #pragma options align=packed
    /*#endif*/
#elif defined(__IBMC__) || defined(__IBMCPP__)
    #pragma pack(1)
#elif defined(__GNUC__)
        #pragma pack(1)
#elif defined(__unix__)
        #pragma pack(1)
#elif defined(__BORLANDC__)
    #pragma option -a1
#else /* assume microsoft or watcom */
    #if defined(__FLAT__) || defined(__WATCOMC__)
        #pragma pack(push)
    #endif
    /* microsoft 16-bit doesn't support pack(push) */
    #pragma pack(1)
#endif

#include    "stdtypes.h"

/*  +-----------------------------------------------------------------------+*/
/*  |   The following typedef declarations are exactly as in WINDOWS.       |*/
/*  |   They will be removed if WINDOWS.H is included.                      |*/
/*  +-----------------------------------------------------------------------+*/
#if !defined(WINDOWS)

typedef struct {
        BYTE    peRed;
        BYTE    peGreen;
        BYTE    peBlue;
        BYTE    peFlags;
    } PALETTEENTRY;

typedef struct {
        WORD         palVersion;        /* 0x310*/
        WORD         palNumEntries;     /* total number of enttries*/
        PALETTEENTRY palPalEntry[1];
    } LOGPALETTE;

typedef struct {
        BYTE    rgbtBlue;
        BYTE    rgbtGreen;
        BYTE    rgbtRed;
    } RGBTRIPLE;

typedef struct {
        BYTE    rgbReserved;
        BYTE    rgbBlue;
        BYTE    rgbGreen;
        BYTE    rgbRed;
    } MACRGBQUAD;

typedef struct {
        BYTE    rgbBlue;
        BYTE    rgbGreen;
        BYTE    rgbRed;
        BYTE    rgbReserved;
    } RGBQUAD;

typedef struct {
        WORD    bfType;             /* 0x4D42 = 'BM'*/
        DWORD   bfSize;
        WORD    bfReserved1;
        WORD    bfReserved2;
        DWORD   bfOffBits;
    } BITMAPFILEHEADER;

/* The following is only used to read old-style BMP files */
typedef struct {
        DWORD   bcSize;             /* 12 = sizeof(BITMAPCOREHEADER)*/
        WORD    bcWidth;            /* in pixels*/
        WORD    bcHeight;           /* in pixels*/
        WORD    bcPlanes;           /* 1*/
        WORD    bcBitCount;         /* 1/4/8/24,  bcBitCount < 24 => RGBTRIPLEs*/
    } BITMAPCOREHEADER;

/* constants for the BITMAPINFOHEAER biCompression field */
#define BI_RGB      0L              /* uncompressed*/
#define BI_RLE8     1L              /*8-bit rle*/
#define BI_RLE4     2L              /*4-bit rle*/
#define BI_BITFIELDS 3L             /* first 3 dwords in bmiColors specify the pixel masks */

typedef struct {
        DWORD   biSize;             /* 68 = sizeof(BITMAPINFOHEADER)*/
        LONG    biWidth;            /* in pixels*/
        LONG    biHeight;           /* in pixels, negative if top-to-bottom*/
        WORD    biPlanes;           /* 1*/
        WORD    biBitCount;         /* 4, 8, 16, 24 or 32*/
        DWORD   biCompression;      /* BI_JPEG = 'JPEG' = 0x4745504A*/
        DWORD   biSizeImage;        /* compressed image size [SOI..EOI]*/
        LONG    biXPelsPerMeter;    /* 0 if none*/
        LONG    biYPelsPerMeter;    /* 0 if none*/
        DWORD   biClrUsed;          /* 2 <= ncolors <= 256  (typically 236)*/
        DWORD   biClrImportant;     /* 0*/
    } BITMAPINFOHEADER;

typedef struct {
        BITMAPINFOHEADER bmiHeader;
        RGBQUAD          bmiColors[1];
    } BITMAPINFO;

#endif

/*  +-----------------------------------------------------------------------+*/
/*  |   The following typedefs declare Windoes extensions for BMP files.    |*/
/*  |   They are not currently defined in Windows 3.10.                     |*/
/*  +-----------------------------------------------------------------------+*/

typedef struct {        /* used to extend BITMAPINFOHEADER fields  */
        BITMAPINFOHEADER    bmi;
        DWORD   biExtDataOffset;    /*  44 = offset of JPEGINFOHEADER*/
        /* Other stuff will go here */
        /* Format-specific information follows EXBMINFOHEADER */
    } EXBMINFOHEADER;

/*  +-----------------------------------------------------------------------+*/
/*  |   This structure is used to convert to 16-bit DIBs.  Only r5:g5:b5    |*/
/*  |   is suported.                                                        |*/
/*  +-----------------------------------------------------------------------+*/

#if 0
typedef struct {
        WORD    blue  : 5;       /* 00000000 000bbbbb*/
        WORD    green : 5;       /* 000000gg ggg00000*/
        WORD    red   : 5;       /* 0rrrrr00 00000000*/
    } RGB16;

#endif

#ifdef __MWERKS__
    /*#### find out how to save/restore the alignment */
#elif defined(__IBMC__) || defined(__IBMCPP__)
    #pragma pack(pop)
#elif defined(__GNUC__)
        #pragma pack()
#elif defined(__unix__)
        #pragma pack()
#elif defined(__BORLANDC__)
    /*#### find out how to save/restore the alignment
            "#pragma option -a" doesn't do it in BC 4.5 */
#else /* assume microsoft or watcom */
    #if defined(__FLAT__) || defined(__WATCOMC__)
        #pragma pack(pop)
    #else
        /* in MS 16-bit, best we can do is to restore to the command-line state */
        #pragma pack()
    #endif
#endif

typedef BITMAPINFO * LPBITMAPINFO;
typedef BITMAPINFOHEADER * LPBITMAPINFOHEADER;
typedef BITMAPFILEHEADER * LPBITMAPFILEHEADER;

#endif  /* #if !defined(_BMP) */

