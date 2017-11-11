/*.T PIC.H  Include file for Pegasus Routines
 * PICTools ProductVersion 2.00.564
 * $Header: /PEGASUS/INCLUDE/pic.h 550   10/08/08 11:21a Sbucholtz $
 * $Nokeywords: $
 */

/***************************************************************************\
*       Copyright (C) 1996-2008 Pegasus Imaging Corporation                 *
*       All rights reserved.                                                *
*****************************************************************************
*       Revision History:                                                   *
* modified 2/16/96 -- jweber                                                *
*       -- changed Eof in QUEUE to a dword of flags                         *
*       -- changed names of OP80, 81, 82 structures to D2F, F2D, UTL        *
* modified 2/2/96 -- jweber                                                 *
*       -- added QBIT_COMMENT, QBIT_PALETTE, BI_RLE4, BI_RLE8               *
* modified 1/26/96 -- jweber                                                *
*       -- added YieldEvery to opcode 81                                    *
*       -- modified QBIT_IMAGESIZE comment                                  *
* modified 12/26/95+ -- jweber                                              *
*       -- added PicFlags to opcode 80                                      *
*       -- added BI_PCX, BI_TIF, BI_TGA (generic) definitions               *
*       -- added PF_MultiImage flag                                         *
*       -- added PicFlags to opcode 82                                      *
*       -- renamed Histogram/ColorMap to ptr/ptr2, opcode 82                *
*       -- added NumColors to opcode 82; removed references to subcode 7    *
*       -- added NumImages and ImageNum to PEGQUERY, and associated         *
*          QBIT_NUMIMAGES                                                   *
*       -- changed PEGQUERY to individual expansion DWORDs                  *
* modified 2/22/96 -- smann                                                 *
*       -- support reverse queue                                            *
*       -- ParmVer is now 11, ParmVerMinor is now 1                         *
* modified 3/12/96 -- jweber                                                *
*       -- opcode 82: "Reserverd3" changed to "ptr3"                        *
* modified 3/27/96 -- jweber                                                *
*       -- incorporates John's changes for opcode 15 (D2J)                  *
* modified 4/4/96 -- jweber                                                 *
*       -- more of John's changes                                           *
* modified 4/17/96 -- jweber                                                *
*       -- QUEUE structure changed to new pointers                          *
* modified 4/21/96 -- SSM                                                   *
*       -- LOSSLESS and LAYER structures added as well as support for the   *
*          LIME and LIMP functions.                                         *
* modified 4/23/96 -- jrb                                                   *
*       -- ParmVer is 13                                                    *
* modified 5/9/96 -- jweber                                                 *
*       -- Added BI_TIFJ, QBIT_SOIMARKER, and SOIMarker field to query      *
*          private u structure in support for TIFF JPEG                     *
* modified 8/31/96 -- Els                                                   *
*       -- Added support for ZOOM operation                                 *
* modified 10/16/96 -- SSM                                                  *
*       -- Added support for cropping                                       *
* modified 3/2/96 -- jweber                                                 *
*       -- Reversing optimization                                           *
* modified 3/10/97 -- jweber                                                *
*       -- Several changes to D2F_STRUC to support Printhouse requirements  *
*          (32-bit "DIB"s; CMYK indication; DotRange support; Moto byte     *
*          order; X & Y resolutions                                         *
* modified 6/12/97 -- SSM                                                   *
*       -- Changes to allow LIP/LIE support                                 *
* modified 8/2/97  -- Jim                                                   *
*       -- John and Charles changes so pc/mac use a single set of includes  *
* modified 9/18/97 -- SSM                                                   *
*       -- Changes to allow LL3 and PIC2List support                        *
* modified 12/8/97 -- SSM                                                   *
*       -- Changes to allow Regions                                         *
* modified 4/28/98 -- SSM                                                   *
*       -- Changes to allow support for JPEG restart markers                *
* modified 9/14/98 -- SSM                                                   *
*       -- Added WSQ support, YIELD flag to correspond to RES_YIELD         *
\***************************************************************************/

#if !defined(_PIC)
#define _PIC

#define CURRENT_PARMVER         20

/* DLG added to detect Windows environment from compiler directives */
  #if !defined(WINDOWS)
    #if defined(_Windows) || defined(_WINDOWS) || defined(_WINDOWS_) || \
        defined(__WINDOWS_386__) || defined(_WINDOWS_16_)
            #define WINDOWS
    #endif
  #endif

  #if !defined(__FLAT__) && defined(WIN32)
    #define __FLAT__
  #endif


#include    "stdtypes.h"
#include    "pcd.h"

#if !defined(USEPICSETJMP)
#include    <setjmp.h>
#endif

/* DLG - only include if not doing windows */
  #if !defined(WINDOWS) && !defined(PIC_DONT_DEFINE_STDTYPES)
        #include  "bmp.h"
  #endif

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
#else /* assume Microsoft or Watcom */
    #if defined(__FLAT__) || defined(__WATCOMC__)
        #pragma pack(push)
    #endif
    /* Microsoft 16-bit doesn't support pack(push) */
    #pragma pack(1)
#endif

/*#### must be packed from here on ####*/

#if !defined(BI_BITFIELDS)
/* this is needed in 16-bit windows where bmp.h isn't
    included, but windows.h doesn't define BI_BITFIELDS */
#define BI_BITFIELDS    (3L)
#endif

/*.P*/
/***************************************************************************\
*   The following typedefs declare extensions for the file types and for    *
*   PIC1 and PIC4 compression types similar to JPEGINFOHEADER.              *
\***************************************************************************/
#define FC(ch0, ch1, ch2, ch3)                               \
        ((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) |    \
        ((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24))

#define BI_picJPEG FC('J','P','E','G') // 'JPEG' compressed (raw)

#if !defined(BI_JPEG)
// MS started defining BI_JPEG in the header files
#define BI_JPEG    BI_picJPEG          // 'JPEG' compressed (raw)
#endif

#define BI_JPGE    FC('J','P','G','E') // 'JPEG-ELS' compressed
#define BI_JPGL    FC('J','P','G','L') // 'JPEG Lossless
#define BI_LJPG    FC('L','J','P','G') // PIC2 JPEG Lossless
#define BI_PJPG    FC('P','J','P','G') // 'PJPG' progressive compressed (raw)
#define BI_TGA1    FC('T','G','A','1') // 'TGA1' Targa type 1 (cm) uncompressed
#define BI_TGA2    FC('T','G','A','2') // 'TGA2' Targa type 2 (bgr) uncompressed
#define BI_TGA3    FC('T','G','A','3') // 'TGA3' Targa type 3 (gray) uncompressed
#define BI_TGA9    FC('T','G','A','9') // 'TGA9' Targa type 9 (cm) RLE
#define BI_TGAA    FC('T','G','A','A') // 'TGAA' Targa type 10 (bgr) RLE
#define BI_TGAB    FC('T','G','A','B') // 'TGAB' Targa type 11 (gray) RLE
#define BI_PICJ    FC('P','I','C','J') // 'PICJ' compressed (PIC JPEG)
#define BI_PICL    FC('P','I','C','L') // 'PICL' compressed
#define BI_PICP    FC('P','I','C','P') // 'PICP' compressed (PIC prog. JPEG)
#define BI_PIC1    FC('P','I','C','1') // 'PIC1' compressed
#define BI_PIC4    FC('P','I','C','4') // 'PIC4' compressed
#define BI_PIC8    FC('P','I','C','8') // 'PIC8' compressed
#define BI_PICG    FC('P','I','C','G') // 'PICG' compressed
#define BI_PICX    FC('P','I','C','X') // 'PICX' compressed
#define BI_PC2J    FC('P','C','2','J') // 'PC2J' compressed (PIC2 JPEG)
#define BI_PC2E    FC('P','C','2','E') // 'PC2E' compressed (PIC2 JPEG Els)
#define BI_PC2L    FC('P','C','2','L') // 'PC2L' compressed
#define BI_PC2P    FC('P','C','2','P') // 'PC2P' compressed (PIC2 prog. JPEG)
#define BI_PC2N    FC('P','C','2','N') // 'PC2N' compressed (PIC2 prog. JPEG Els)
#define BI_PC21    FC('P','C','2','1') // 'PC21' compressed
#define BI_PC24    FC('P','C','2','4') // 'PC24' compressed
#define BI_PC28    FC('P','C','2','8') // 'PC28' compressed
#define BI_PC2G    FC('P','C','2','G') // 'PC2G' compressed
#define BI_PC2X    FC('P','C','2','X') // 'PC2X' compressed
// note BI_RGB is defined in bmp.h
#define BI_RGBA    FC('R','G','B','A') // 'RGBA' ordered B/G/R/A
#define BI_RGB565  FC('R','5','6','5') // to recognize rgb 565 format
#define BI_BMPO    FC('B','M','P','O') // 'BMPO' old-style BMP uncompressed
                                       // (OS/2 1.1 and 1.2)
#define BI_BMPR    FC('B','M','P','R') // 'BMPR' new-style BMP RLE
#define BI_PCX1    FC('P','C','X','1') // 'PCX1' PCX 1-bit
#define BI_PCX2    FC('P','C','X','2') // 'PCX2' PCX 2-bit ('CGA') (cm)
#define BI_PCX3    FC('P','C','X','3') // 'PCX3' PCX 3-bit ('EGA') (cm)
#define BI_PCX4    FC('P','C','X','4') // 'PCX4' PCX 4-bit (cm)
#define BI_PCX8    FC('P','C','X','8') // 'PCX8' PCX 8-bit (cm)
#define BI_PCXT    FC('P','C','X','T') // 'PCXT' PCX 24-bit
#define BI_DCXZ    FC('D','C','X','Z') // 'DCXZ' DCX multi-image
#define BI_TIF1    FC('T','I','F','1') // 'TIF1' TIFF 1-bit
#define BI_TIFM    FC('T','I','F','M') // 'TIFM' TIFF color-map (up to 8-bit)
#define BI_TIFG    FC('T','I','F','G') // 'TIFG' TIFF gray-scale (up to 16-bit)
#define BI_TIFC    FC('T','I','F','C') // 'TIFC' TIFF rgb (16, 24, 32-bit)
#define BI_TIFZ    FC('T','I','F','Z') // 'TIFZ' TIFF multi-image
#define BI_TIFJ    FC('T','I','F','J') // 'TIFJ' TIFF JPEG
#define BI_TIFL    FC('T','I','F','L') // 'TIFL' TIFF LZW
#define BI_TIFK    FC('T','I','F','K') // 'TIFK' TIFF CMYK
#define BI_TIFu    FC('T','I','F','u') // 'TIFu' TIFF unknown
#define BI_GIFN    FC('G','I','F','N') // 'GIFN' GIF non-interlaced
#define BI_GIFI    FC('G','I','F','I') // 'GIFI' GIF interlaced
#define BI_GIFu    FC('G','I','F','u') // 'GIFu' GIF unknown
#define BI_picPNG  FC('P','N','G',' ') // 'PNG ' PNG all flavors

#if !defined(BI_PNG)
// MS started defining BI_PNG in the header files
#define BI_PNG     BI_picPNG           // 'PNG ' PNG all flavors
#endif

#define BI_OS2     FC('O','S','2','U') // 'OS2U' OS2 2.x+ uncompressed
#define BI_TGA     FC('T','G','A',' ') // 'TGA ' Targa generic
#define BI_PCX     FC('P','C','X',' ') // 'PCX ' PCX generic
#define BI_TIF     FC('T','I','F',' ') // 'TIF ' TIFF generic
#define BI_PCD     FC('P','C','D',' ') // 'PCD ' PhotoCD generic
#define BI_G3      FC('G','3',' ',' ') // 'G3  ' Raw G3 fax -- 1D
#define BI_G32D    FC('G','3','2','D') // 'G32D' Raw G3 fax -- 2D
#define BI_UYVY    FC('U','Y','V','Y') // 'UYVY' Microsoft uyvy video subtype
#define BI_YUY2    FC('Y','U','Y','2') // 'YUY2' Microsoft yuy2 video subtype
#define BI_YV12    FC('Y','V','1','2') // 'YV12' Microsoft yv12 video subtype (planar)
#define BI_IYUV    FC('I','Y','U','V') // 'IYUV' Microsoft iyuv video subtype (planar)
#define BI_I420    FC('I','4','2','0') // 'I420' Microsoft i420 video subtype (planar -- identical to IYUV)
#define BI_CMYK    FC('C','M','Y','K') // 'CMYK' 4-byte interlaced
#define BI_WAVE    FC('W','A','V','E') // 'WAVE' wavelet
#define BI_WAVP    FC('W','A','V','P') // 'WAVP' progressive wavelet
#define BI_GR12    FC('G','R','1','2') // 'GR12' 12-bit gray uncompressed, low-order bits in 16-bit field
#define BI_DJVU    FC('D','J','V','U') // 'DJVU' AT&T DJVU(tm)
#define BI_WSQ     FC('W','S','Q',' ') // 'WSQ '
#define BI_IOCA    FC('I','O','C','A') // 'IOCA'
#define BI_MDCA    FC('M','D','C','A') // 'MDCA' is MO:DCA
#define BI_CALS    FC('C','A','L','S') // 'CALS'
#define BI_PDF     FC('P','D','F',' ') // PDF wrapper around G4 image
#define BI_J2K     FC('J','2','K',' ') // 'J2K ' JPEG 2000
#define BI_WMF     FC('W','M','F',' ') // 'WMF ' Windows MetaFile
#define BI_EMF     FC('E','M','F',' ') // 'EMF ' Enhanced Windows MetaFile
#define BI_JLS     FC('J','L','S',' ') // 'JLS ' JPEG LS
#define BI_ICON    FC('I','C','O','N') // 'ICON' Icon image
#define BI_WBMP    FC('W','B','M','P') // 'WBMP' Wireless BitMaP
#define BI_PBM     FC('P','B','M',' ') // 'PBM ' Portable Bit Map (1 bpp)
#define BI_PGM     FC('P','G','M',' ') // 'PGM ' Portable Bit Map (up to 8 bpp - gray)
#define BI_PPM     FC('P','P','M',' ') // 'PPM ' Portable Bit Map (24 bpp - RGB)
#define BI_PNM     FC('P','N','M',' ') // 'PNM ' Portable Anymap (generic name)
#define BI_PBMA    FC('P','B','M','A') // 'PBMA' ASCII Portable Bit Map (1 bpp) (unimplemented feature)
#define BI_PGMA    FC('P','G','M','A') // 'PGMA' ASCII Portable Bit Map (up to 8 bpp - gray) (unimplemented feature)
#define BI_PPMA    FC('P','P','M','A') // 'PPMA' ASCII Portable Bit Map (24 bpp - RGB) (unimplemented feature)
#define BI_PNMA    FC('P','N','M','A') // 'PNM ' ASCII Portable Anymap (generic name) (unimplemented feature)
#define BI_DCX     FC('D','C','X',' ') // 'DCX ' generic
#define BI_JB2     FC('J','B','2',' ') // 'JB2 ' JBIG2
#define BI_GRAY    FC('G','R','A','Y') // 'GRAY' 9- to 16-bit high resolution gray
#define BI_DWG     FC('D','W','G',' ') // 'DWG' AutoCad Drawing file
#define BI_DWF     FC('D','W','F',' ') // 'DWF' AutoCad DWF file
#define BI_DXF     FC('D','X','F',' ') // 'DXF' AutoCad DXF file
#define BI_DXFB    FC('D','X','F','B') // 'DXFB' AutoCad Binary DXF file
#define BI_HDP     FC('H','D','P',' ') // 'HDP'  MS HD Photo file
#define BI_C4      FC('C','4',' ',' ') // 'C4' JEDMICS C4 Compressed Image file



/***************************************************************************\
*   This structure is used to convert to 16-bit DIBs.  Only r5:g5:b5 and    *
*   r5:g6:b5 are supported.                                                 *
\***************************************************************************/

#if 0
The following will not work in 32-bit environment because UINT is 4bytes
rather than the two bytes required. This is intended to be a two-byte
field for 16-bit video modes. If we use WORD here rather than UINT, we are
not ANSI compatible.  So for now we are just going to make RGB555 and
RGB565 be simply words rather than bitfields.
typedef struct {
        UINT    blue  : 5;      /* 00000000 000bbbbb */
        UINT    green : 5;      /* 000000gg ggg00000 */
        UINT    red   : 5;      /* 0rrrrr00 00000000 */
    } RGB555;

typedef struct {
        UINT    blue  : 5;      /* 00000000 000bbbbb */
        UINT    green : 6;      /* 00000ggg ggg00000 */
        UINT    red   : 5;      /* rrrrr000 00000000 */
    } RGB565;
#endif
typedef WORD    RGB555;
typedef WORD    RGB565;
typedef struct {
        BYTE    C;
        BYTE    M;
        BYTE    Y;
        BYTE    K;
    } CMYKQUAD;

/*.P*/
/* PixType determines physical bits per pixel. */
typedef BYTE    PIXEL_TYPE;
/* standard definition:  Sig = 0 */
#define PT_NONE      (0x00)     /* undefined or compressed pixels */
#define PT_CM        (0x01)     /* color mapped up to 256 colors */
#define PT_GRAY      (0x02)     /* gray scale up to 16 bits (Intel format) */
#define PT_GRAYM     (0x03)     /* gray scale 9 to 16 bits (Motorola format */
#define PT_RGB       (0x04)     /* RGB 24 or 48 bit (blue is low, red is high) */
#define PT_RGBM      (0x05)     /* RGB 48 bit (blue is low, red is high) (Motorola) */
#define PT_RGB555    (0x06)     /* RGB 16 bit (xrrrrrgggggbbbbb) */
#define PT_RGB565    (0x07)     /* RGB 16 bit (rrrrrggggggbbbbb) */
#define PT_CMYK      (0x08)     /* CMYK 32 bit (cyan is low, black is high) */
#define PT_GRAYA     (0x12)     /* gray scale up to 16 bits with alpha (Intel) */
#define PT_GRAYAM    (0x13)     /* gray scale 9 to 16 bits with alpha (Motorola) */
#define PT_RGBA      (0x14)     /* RGBA 32 bit (blue is low, alpha is high) */
#define PT_RGBAM     (0x15)     /* RGBA 64 bit (blue is low, alpha is high) (Motorola) */

#define PT_YUV       (0x09)     /* YUV 24 bit (Y is low byte, V is high bit) */
#define PT_YUYV      (0x0A)     /* 4 8-bit samples, U&V subsampled 2:1 */
#define PT_YUY2      (0x0A)     /* same as PT_YUYV */
#define PT_UYVY      (0x0B)     /* 4 8-bit samples, U&V subsampled 2:1 order differs from PT_YUYV */
#define PT_YCBCR     (0x0E)     /* YCbCr 24 bit (Y is low byte, Cr is high byte) */
                                /* 16 <= Y <= 235,  16 <= U, V <= 240 */
#define PT_RGB101010 (0x17)     /* Microsoft RGB 10/10/10 BGR in 32-bit DWORD */
#define PT_RGBE      (0x18)     /* RADIANCE HDR format */
#define PT_CMYKA     (0x19)     /* CMYK plus alpha */

// following flags can be or'ed into some of the above to define the (S)ample (R)epresentation
// as MS fixed point or as floating point instead of the default integer
#define PTSR_MASK    (0xc0)
#define PTSR_MSFIXED (0x80)
#define PTSR_FLOAT   (0xc0)

// presently only the combinations allowed by HD Photo are defined
// and use RF_BigEndian for big-endian ordering rather than, for example, PT_GRAYM
#define PT_GRAY_MSFIXED (PTSR_MSFIXED|PT_GRAY)
#define PT_GRAY_FLOAT   (PTSR_FLOAT|PT_GRAY)
#define PT_RGB_MSFIXED  (PTSR_MSFIXED|PT_RGB)
#define PT_RGB_FLOAT    (PTSR_FLOAT|PT_RGB)
#define PT_RGBA_MSFIXED (PTSR_MSFIXED|PT_RGBA)
#define PT_RGBA_FLOAT   (PTSR_FLOAT|PT_RGBA)

/* following are reserved, not used */
#define PT_INT       (0x0C)     /* 32-bit signed 2's complement integer */
#define PT_FLOAT     (0x0D)     /* 32-bit IEEE floating point value */
#define PT_RGBI      (0x16)     /* (use PT_RGB) RGB 48 bit (blue is low, red is high) (Intel = little endian) */
/* end reserved, not used */


typedef BYTE    REGION_FLAGS;
/* standard definition:  Sig = 0 */
#define RF_TopDown   0x01       /* set if image stored top line first, bottom last */
#define RF_NonInter  0x02       /* set if image is non-interleaved (obsolete) */
/*  If PixType is PT_RGB or RF_CM2RGB set then the following apply */
#define RF_MakeGray  0x04       /* set if region to be treated as gray scale */
#define RF_SwapRB    0x08       /* set if Red and Blue components to be exchanged (RGB only)) */
#define RF_WhiteIsZero (RF_SwapRB) /* set if bitmap uses zero for white */
/*  If PixType is PT_GRAY or PT_GRAYM or RF_MakeGray set then the following apply */
#define RF_Signed    0x10       /* set if bits in pixel are to be treated as signed int */
#define RF_2Byte     0x20       /* use 2-byte field for samples, ignored if > 8 bits/sample */
#define RF_BigEndian 0x40       /* when unpacking 2-byte pixels, make them big endian */
#define RF_Packed    0x80       /* packed pixels if PT_CM or PT_GRAY and 1-bpp/2-bpp/4-bpp pixels */
/*  If PixType is PT_CM then the following apply */
#define RF_CM2RGB    0x10       /* color mapped pixels are to be converted to RGB pixels */
#define RF_Channel   0x20       /* set if a single color channel is to be used */
#define RF_ChLo      0x40       /* if RF_Channel is set, ChHi,ChLo are two bits whose value, */
#define RF_ChHi      0x80       /* 0..3, indicates the color channel.  Channel 0 is the index.
                                   Channels 1..3 are the first..third ColorTable members. */

/*  This structure is used to reference general-purpose image regions. */
typedef struct {
        BYTE         Sig;       /* 0 for REGION */
        BYTE         Interlace; /* 0 => None, 1 => PNG, 2 => GIF, 3 => line */
                                /* If PixType is PT_GRAY or PT_GRAYM then this also specifies */
                                /* subsampling; bits 7:5 specify horizontal and bits 4:2 */
                                /* specify vertical subsampling counts.  000..111 corresponds to */
                                /* skipping 0...7 samples.  I.e., 001 corresponds to 2:1 subsampling. */
                                /* This subsampling is ignored if any is implied by transformations. */
        BYTE         BitErr;    /* low-order bits to be dropped or treated as 0 (Grayscale only) */
        REGION_FLAGS Flags;     /* attributes of region */
        PIXEL_TYPE   PixType;   /* type of pixel which comprise this region */
        BYTE         Bpp;       /* meaningful Bits per pixel.  If bpp <= 8 then one byte per
                                   sample, else 2 bytes per sample (see RF_2Byte) */
        PICXWORD     Width;     /* width of region in pixels > 0 */
        PICXWORD     Height;    /* height of region in lines > 0 */
        PICXSHORT    Stride;    /* width of area containing the region in bytes */
        PICSIZET     Offset;    /* byte offset of logical first line of region */
    } REGION;

/*  This structure is used to reference general-purpose image regions.
    if width or height might be greater than 65535 or stride limits width
    In opcodes:
        OP_LIP3PLUS, OP_LIP3, OP_LIE3PLUS, OP_LIE3 (lossless JPEG)
        set ParmVerMinor to 4 and use the casting macros below
        to set/get region fields */
typedef struct {
        BYTE         Sig;       /* 2 for REGION2 */
        REGION_FLAGS Flags;     /* attributes of region */
        PIXEL_TYPE   PixType;   /* type of pixel which comprise this region */
        BYTE         Bpp;       /* meaningful Bits per pixel.  If bpp <= 8 then one byte per
                                   sample, else 2 bytes per sample (see RF_2Byte) */
        DWORD        Width;     /* width of region in pixels > 0 */
        DWORD        Height;    /* height of region in lines > 0 */
        DWORD        Stride;    /* width of area containing the region in bytes */
    } REGION2;

/* casting macros with a little bit of type checking */
#define Region2(r)          (*(REGION2*)(&(r).Interlace - 1))
#define Region2Ptr(pr)      ((REGION2*)(&(pr)->Interlace - 1))
/* for example, for OP_LIP3PLUS, set the input Stride as:
    Region2(p->u.LL3.Region).Stride = InputStride;
        or
    Region2Ptr(&p->u.LL3.Region)->Stride = InputStride;
*/



/* A pointer to this structure is passed to PegasusLoadFromRes or set in
    PIC_PARM.LoadPath in order to load the opcode SSM from a list of
    buffered SSM's,
    Signature points to the string "\..\PicSsmList",
    Count is the number of PICSSM structures
    Ssms points to an array of PICSSM structures */
#define PICSSM_SIGNATURE    "\\..\\PicSsmList"
typedef struct
{
    char* Name;
    unsigned char* Buffer;
    unsigned long Length;
} PICSSM;
typedef struct
{
    char Signature[16];
    unsigned long Count;
    PICSSM* Ssms;
    char* LoadPath;
} PICSSMLIST;

/***************************************************************************\
*   These typedefs are used to represent non-callback states.               *
\***************************************************************************/

typedef LONG OPERATION;


// Offsets 0..9 are reserved for standard opcodes; all opcodes are offset 10.
#define STD_OPCODE_OFFSET (10)

// MKOP macro allows the DLL loader to take opcode/1000 to construct a DLL name.
#define MKOP(x) ((x) * 1000L + STD_OPCODE_OFFSET)

////00 reserved
#define OP_SHELL     (MKOP(  1)) // Opcode shell (internal use only)
////02 reserved
////03, 04, 05, 06, 07, 08, 09

#define OP_D2S       (MKOP( 10)) // DIB to Huffman Sequential JPEG
#define OP_S2D       (MKOP( 11)) // Huffman Sequential JPEG to DIB
#define OP_D2SE      (MKOP( 12)) // DIB to Huffman or Els Sequential JPEG
#define OP_P2D       (MKOP( 13)) // Huffman Progressive JPEG to DIB
#define OP_S2P       (MKOP( 14)) // Sequential JPEG to Progressive JPEG
#define OP_D2J       (MKOP( 15)) // DIB to Huffman Progressive or Sequential JPEG
#define OP_P2S       (MKOP( 16)) // Progressive JPEG to Sequential JPEG
////17 reserved
#define OP_SE2D      (MKOP( 18)) // Huffman or Els Sequential JPEG to DIB
////19 reserved

////30 reserved
////31 reserved
////32, 33, 34, 35, 36, 37, 38, 39

////20 reserved
#define OP_EXP4      (MKOP( 21)) // IM4 to DIB
#define OP_ROR       (MKOP( 22)) // Reorient/requantize to Huffman JPEG
#define OP_RORE      (MKOP( 23)) // Reorient/requantize to Els or Huffman JPEG
#define OP_REQUANT   (MKOP( 24)) // Requantize to Huffman JPEG
////25
#define OP_HDPHOTOP  (MKOP( 26)) // Microsoft HD Photo Pack
#define OP_HDPHOTOE  (MKOP( 27)) // Microsoft HD Photo Expand
#define OP_J2KTRANSCODE (MKOP( 28)) // JPEG2000 transcoder
#define OP_ADJUST    (MKOP( 29)) // Various adjustment operations

////40 reserved
////41 reserved
#define OP_LIMP      (MKOP( 42)) // Lossless IMage Pack
#define OP_LIME      (MKOP( 43)) // Lossless IMage Expand
#define OP_LIP       (MKOP( 44)) // Lossless IMage Pack   (version 2)
#define OP_LIE       (MKOP( 45)) // Lossless IMage Expand (version 2)
#define OP_LIP3      (MKOP( 46)) // Lossless IMage Pack   (version 3)
#define OP_LIE3      (MKOP( 47)) // Lossless IMage Expand (version 3)
#define OP_PNGP      (MKOP( 48)) // Portable Network Graphics Pack
#define OP_PNGE      (MKOP( 49)) // Portable Network Graphics Expand

#define OP_CLEAN     (MKOP( 50)) // Various cleanup operations
#define OP_SCANFIX   (MKOP( 51)) // Various document-imaging enhancements
#define OP_TIDP      (MKOP( 52)) // IDP Transmitter
#define OP_RIDP      (MKOP( 53)) // IDP Receiver
#define OP_BINARIZE  (MKOP( 54)) // Binarization
#define OP_CAMERARAWE (MKOP(55)) // Expand digital camera RAW files
#define OP_TIDP2     (MKOP( 56)) // IDP Transmitter (version 2: enhanced and J2K only)
#define OP_RIDP2     (MKOP( 57)) // IDP Receiver    (version 2: enhanced and J2K only)
#define OP_JPIPSERVER (MKOP( 58)) // JPIP Server
#define OP_JPIPCLIENT (MKOP( 59)) // JPIP Client

#define OP_JLSP      (MKOP( 60)) // JPEG-LS Pack
#define OP_JLSE      (MKOP( 61)) // JPEG-LS Expand
#define OP_LIP3PLUS  (MKOP( 62)) // Lossless Image Pack   (version 3) + 9-16 bit lossless JPEG
#define OP_LIE3PLUS  (MKOP( 63)) // Lossless Image Expand (version 3) + 9-16 bit lossless JPEG
#define OP_D2SEPLUS  (MKOP( 64)) // Lossy JPEG Pack,   8/12-bit gray or 24-bit RGB
#define OP_SE2DPLUS  (MKOP( 65)) // Lossy JPEG Expand, 8/12-bit gray or 24-bit RGB
#define OP_J2KPRGB   (MKOP( 66)) // JPEG 2000 RGB Pack,   8-bit gray/color-map or 24-bit RGB
#define OP_J2KERGB   (MKOP( 67)) // JPEG 2000 RGB Expand, 8-bit gray/color-map or 24-bit RGB
#define OP_J2KP      (MKOP( 68)) // JPEG 2000 Pack
#define OP_J2KE      (MKOP( 69)) // JPEG 2000 Expand

#define OP_D2DJVU    (MKOP( 70)) // AT&T DJVU(tm) Pack
#define OP_DJVU2D    (MKOP( 71)) // AT&T DJVU(tm) Expand
#define OP_D2MDCA    (MKOP( 72)) // MO:DCA and CALS Pack
#define OP_MDCA2D    (MKOP( 73)) // MO:DCA and CALS Expand
#define OP_JBIG2P    (MKOP( 74)) // JBIG2 Pack
#define OP_JBIG2E    (MKOP( 75)) // JBIG2 Expand
////76
#define OP_CAD2D     (MKOP( 77)) // CAD to DIB
#define OP_J2KP3D    (MKOP( 78)) // JPEG 2000 Pack with 3D
#define OP_J2KE3D    (MKOP( 79)) // JPEG 2000 Expand with 3D

#define OP_D2F       (MKOP( 80)) // DIB to supported file format
#define OP_F2D       (MKOP( 81)) // Supported file format to DIB
#define OP_UTL       (MKOP( 82)) // Utility functions (DIB, palette, histogram)
#define OP_TIFEDIT   (MKOP( 83)) // Multi-image TIFF editing features
#define OP_D2FPLUS   (MKOP( 84)) // OP_D2F + GIF + LZW/TIFF
#define OP_F2DPLUS   (MKOP( 85)) // OP_F2D + GIF + LZW/TIFF
#define OP_D2PDF     (MKOP( 86)) // Embed DIB in created PDF or append in existing PDF
#define OP_PDF2D     (MKOP( 87)) // Extract DIB from PDF
#define OP_ZOOM2     (MKOP( 88)) // Change image dimensions
#define OP_ZOOM      (MKOP( 89)) // Change image dimensions

#define OP_D2W       (MKOP( 90)) // DIB to wavelet
#define OP_W2D       (MKOP( 91)) // Wavelet to DIB
#define OP_WSQP      (MKOP( 92)) // Wavelet Scalar Quantization Pack
#define OP_WSQE      (MKOP( 93)) // Wavelet Scalar Quantization Expand
#define OP_D2WV      (MKOP( 94)) // DIB to wavelet video
#define OP_WV2D      (MKOP( 95)) // Wavelet video to DIB
#define OP_D2WOLD    (MKOP( 96)) // 2.00.66 version of OP_D2W using fwrz 1.08f library
////97, 98
////99 reserved as opcode numbering escape

#define OP_DEBUG     (MKOP(100)) // Tracing/logging functions (internal use only)

/* reserved >> */
#define OP_DISPATCH  (MKOP(  0))
#define OP_QRY       (MKOP(  1)) // Query a buffer for extensive information
#define OP_CMAP      (MKOP(  2)) // Create a Color Map from a buffer
#define OP_PACKJ     (MKOP( 10)) // IMJ - Pack a DIB to Huffman Sequential JPEG
#define OP_EXPJ      (MKOP( 11)) // IMJ - Expand Huffman Sequential JPEG to a DIB
#define OP_D2JE      (MKOP( 17)) // DIB to Huffman or Els Progressive or Sequential JPEG
#define OP_JE2D      (MKOP( 19)) // Huffman or Els Sequential or Progressive JPEG to DIB
#define OP_PACK4     (MKOP( 20)) // IM4 - Pack a DIB to IM4
#define OP_PACK1     (MKOP( 30)) // IM1 - Pack a DIB to IM1
#define OP_EXP1      (MKOP( 31)) // IM1 - Expand IM1 to a DIB
#define OP_PACK8     (MKOP( 40)) // IM8 - Pack a DIB to IM8
#define OP_EXP8      (MKOP( 41)) // IM8 - Expand IM8 to a DIB
#define OP_D2LJ      (MKOP( 62)) // Lossless JPEG Pack
#define OP_LJ2D      (MKOP( 63)) // Lossless JPEG Expand

/*.P*/
/***************************************************************************\
*                                                                           *
*   This structure represents the visual orientation of the image.  It      *
*   is independent of the file's storage orientation (bottom-to-top         *
*   with the bottom of the image stored first in the file or top-to-        *
*   bottom with the top of the image stored first in the file).  Let        *
*   TOP_DOWN be TRUE iff the image is stored top-to-bottom, i.e., the       *
*   biHeight of the image is < 0.  Let VO be the Visual Orientation as      *
*   represented below.  Finally let FO be the File Orientation - the        *
*   orientation field as stored within the PIC file (FO should equal        *
*   VO but for historical reasons it actually indicates whether the         *
*   file should be inverted or not).  Then have:  FO = VO ^ O_inverted.     *
*                                                                           *
*   +--+--+--+--+--+--+--+--+                                               *
*   | 0| 0| x| x| 0| x| x| x|    visual ORIENTATION values                  *
*   +--+--+--+--+--+--+--+--+                                               *
*     7  6  5  4  3  2  1  0                                                *
*           |  |     |  |  +- set if reflected across horizontal line       *
*           |  |     |  +---- set if rotated right 90ø                      *
*           |  |     +------- set if rotated right 180ø                     *
*           |  +------------- set if appears white on black                 *
*           +---------------- set if bit order is reversed                  *
*   Any rotating is done before the reflection.                             *
*                                                                           *
*   If Õ is the standard image then the visual orientations are:            *
*     Õ   +   +   ½   ¾   ¸   Ó   Ö   (if displayed without adjustment)     *
*                                                                           *
*   This structure is also used to indicate the visual characteristics      *
*   of 1-bit images, i.e., IMG (Group 3 and Group 4 fax).                   *
*                                                                           *
\***************************************************************************/
typedef LONG    ORIENTATION;
#define O_normal    0
#define O_inverted  1
#define O_r90       2
#define O_r90_in    3
#define O_r180      4
#define O_r180_in   5
#define O_r270      6
#define O_r270_in   7
#define O_w_on_b    0x10
#define O_bit_rev   0x20


typedef LONG    SUBSAMPLING;
#define SS_111      0
#define SS_211      1   /* Cb and Cr are 2-to-1 subsamp. horiz., not vert. */
#define SS_411      2   /* Cb and Cr are 2-to-1 subsamp. horiz and vert. */
#define SS_211v     3   /* Cb and Cr are 2-to-1 subsampled vert., not horiz. */
                        /* For all other cases subsampling is defined as: */
                        /* HY<<27 | VY<<22 | HCb<<17 | VCb<<12 | HCr<<7 | VCr<<2, */
                        /* where H and V are the subsampling factors defined */
                        /* in JPEG spec. */

typedef DWORD   PICFLAGS;

#define PF_IsGray                   0x00000001L
        /* (Pack) Set if image is gray-scale */
#define PF_TopDown                  0x00000002L
        /* (Exp and Pack) Set if image is top_down (DIB buffers then run backwards) */
#define PF_OptimizeHuff             0x00000004L
        /* (Pack, not Els coded) Set if huff codes are to be optimized (meaningless if ELS coded */
#define PF_IsTransparency           0x00000004L
        /* (GIF) image has a transparent color index */
#define PF_IncludeBMPHead           0x00000008L
        /* Include BITMPAINFOHEADER and palette in output stream if appropriate */
#define PF_UsedP3                  (0x00000008L)
        /* Set for S2D/SE2D/SEP2D if P3 was detected and P3 extensions were used during decompress */
#define PF_ReturnYIfFirst           0x00000010L
        /* (Seq Exp) If image is multiscan sequential color lossy JPEG and */
        /* first scan is Y only, return gray image first and defer (RES_Y_DONE_FIRST). */
        /* App can then reset Put buffer before continuing to get color image. */
#define PF_YuvOutput                0x00000020L
        /* (Seq Expand). Causes uyvy or yuy2 output (type chosen in DIB_OUTPUT below) */
#define PF_BigEndian                0x00000040L
        /* (ROR/D2J Exif Motorola byte-ordered tags instead of Intel */
#define PF_ExpandThumbnail          0x00000080L
        /* (J2D) expand exif thumbnail instead of primary image */
#define PF_NoImageChanges           0x00000080L
        /* (ROR) changing Exif tags only -- no change to other image data */
#define PF_FastZoom                 0x00000080L
        /* (ZOOM)  Set if fast mode is to be used for zooming */
#define PF_ConvertGray              0x00000100L
        /* (Exp, ZOOM) Set if image is to be gray-scale */
#define PF_IsProtected              0x00000100L
        /* set if PegasusQuery detects that the file is protected */
#define PF_NoDibPad                 0x00000200L
        /* (Exp and Pack)  Set if DIB is not (or not to be) DWORD aligned */
#define PF_PreserveBlack            0x00000200L
        /* (ZOOM2)  Set to preserve black when shrinking 1-bit to 1-bit */
#define PF_Dither                   0x00000400L
        /* (Exp)  Set if image is to be dithered */
#define PF_MakeColors               0x00000800L
        /* (Exp and Pack)  Set if palette is to be made */
#define PF_CreateDibWithMadeColors  0x00001000L
        /* (Exp)  Set if a DIB is to be filled with made colors */
#define PF_OnlyUseMadeColors        0x00002000L
        /* (Exp)  Set if ONLY made colors are to be used (no DIB output on "first */
        /* pass" using image or user colors) */
#define PF_App1Pal                  0x00004000L
        /* (Pack) Set if palette is to be in APP1 marker code */
#define PF_IsColorMapped            0x00004000L
        /* (Exp) Set if image was color-mapped */
#define PF_DibKnown                 0x00008000L    /* obs. name */
#define PF_WidthPadKnown            0x00008000L
        /* (exp and pack) Set if WidthPad is set by app before */
        /* init, NOT to be calc. by level 2 during init. */
#define PF_Yield                    0x00010000L
        /* (WSQ)  Set if Yield should be called during operation */
#define PF_YieldGet                 0x00010000L
        /* (P&E)  Set if Yield during data get (CB only) */
#define PF_YieldPut                 0x00020000L
        /* (P&E)  Set if Yield during data put (CB only) */
#define PF_Poll                     0x00040000L
        /* (Comm) Set if return instantly when nothing received */
#define PF_D2FSwapRB                0x00040000L
        /* (D2F) TIFF swap R/B in input */
#define PF_F2DSwapRB                0x00040000L
        /* (F2D) TIFF swap R/B in output */
#define PF_NoGets                   0x00080000L
        /* (Exp prog.) Set if no data to be read - just use what is in buffer now. */
#define PF_AutoIgnoreBadSuffix      0x00080000L
        /* (LL) Determine if invalid suffix after -32768, set PF_IgnoreBadSuffix */
#define PF_EOIFound                 0x00100000L
        /* (Exp prog.) Set if JPEG EOI marker reached in progressive. */
#define PF_NonInterleaved           0x00100000L
        /* (LL) The image should be planar packed: all reds, then greens, then blues */
#define PF_NoCrossBlockSmoothing    0x00200000L
        /* (Exp prog. and seq) */
#define PF_IgnoreBadSuffix          0x00200000L
        /* (LL) Ignore invalid suffix after -32768  */
#define PF_LIPUseYCbCrForColors     0x00200000L
        /* (Pack only) Use YCbCr space if making colors - often chooses better
             colors. If not making colors, it is ignored. This flag is not
             used by JPEG expand, as it makes colors from YCbCr always. */
#define PF_DecodeToDataEnd          0x00400000L
        /* (Exp prog.) */
#define PF_SynchRestart             0x00400000L
        /* (LL) If a restart error occurs, resynch at next marker */
#define PF_MultiImage               0x00800000L
        /* (OP81) -- At RES_DONE, if PF_EOIFound is 0, then: 0 means next DIB */
        /* is a progressive image; 1 means next DIB is a new image from the */
        /* file (e.g., multi-image TIFF or GIF files) */
#define PF_JPEGFileOnly             0x00800000L
        /* (LL) Do not make PIC2 file for mode J, make JPEG only,
           (F2D) for TIFF/JPEG input, output JPEG image instead of DIB */
#define PF_ElsCoder                 0x00800000L
        /* (Pack and ROR and Exp) Use ELS rather than Huffman entropy coding */
        /* For Pack and ROR, this is set by app to determine output. For Exp, it is merely returned by Pegasus describing input. */
#define PF_SwapRB                   0x01000000L
                /* Exp and Pack. Swap RB in uncompressed out and in, if color > 8 bpp */
#define PF_ApplyTransparency        0x01000000L
        /* (LL) Don't write transparent color index pixels */
#define PF_Quickview12              0x02000000L
                /* Seq. Exp. Decode a 12-bit per component JPEG by keeping only high 8 bits. */
#define PF_UserDelay                0x02000000L
        /* (LL) Mouse or keyboard allows image replacement, also F2D/D2F GIF User Input Flag in graphics extension block */
#define PF_OptimalFilter            0x02000000L
        /* (PNG) the best of filters 0..4 is used for each line */
#define PF_ReverseInputByteOrder    0x02000000L
        /* (JPEG pack, 12-bit grayscale case only) Use this if the word input is in big-endian (Motorola) order. */
#define PF_AllocateComment          0x04000000L
        /* (LL) RES_ALLOCATE_COMMENT_BUF on every comment */
        /* (LL) RES_ALLOCATE_APP2_BUF on every app */
        /* (LL) RES_ALLOCATE_OVERTEXT_BUF on every overtext */
#define PF_IsBMP                    0x04000000L
        /* (PNG) the source file is BMP */
#define PF_ColorTableProvided       0x08000000L
        /* (LL) Use provided ColorTable, not one in file */
#define PF_UsedMMX                  0x08000000L
        /* (SJPEG) Expand MMX was detected and used */
#define PF_MakeBMP                  0x08000000L
        /* (do not use) convert output to a BMP image (set PF_SwapColors, */
        /*  do 16->8 bit conversion, DWORD padding, etc.) */
#define PF_ZoomToSize               0x10000000L
        /* (LL) Zoom all layers to MinLayerWanted size */
#define PF_SwapColors               0x10000000L
        /* (do not use) convert RGB to BGR for 24-, 32, 48-, and 64-bit colors */
#define PF_ConvertToColor           0x10000000L
        /* (ROR) convert Grayscale to Color (for lossy jpegs) */
#define PF_ContextInterpolation     0x20000000L
        /* (LL) Use best guess for interpolation */
#define PF_DidPolish                0x20000000L
        /* (LL) Able to do all PIC2 forward updates */
#define PF_BlendWithBuffer          0x20000000L
        /* (PNG) if alpha or transparency, blend color with buffer vs background color */
#define PF_WordQtbl                 0x20000000L
        /* (JPEG pack, 12-bit grayscale case only) The user-inputted qtbl is words, values up to 32767.  */
#define PF_SkipHuffman              0x40000000L
        /* (LL) Don't calculate optimal Huffman table */
#define PF_HaveColorMap             0x40000000L
        /* (P&E) Must have ColorMap != NULL and colors in color table */
#define PF_HaveTransparency         0x40000000L
        /* (PNG) Transparency value is available */
#define PF_Log                      0x40000000L
        /* (WSQP-PIC internal use only) Create a log file for tracing */
#define PF_UseYCbCrForColors        0x80000000L
        /* (Pack only) Use YCbCr space if making colors - often chooses better
             colors. If not making colors, it is ignored. This flag is not
             used by JPEG expand, as it makes colors from YCbCr always. */
#define PF_HaveWatermark            0x80000000L
        /* (LL) OverText points to watermark not CT */
#define PF_HaveBackground           0x80000000L
        /* (PNG) Background value is available */

/* following PF2_ flag values reserved (can't be overloaded by any opcode) */
/*#define PF2_P2AndP3OptDisable   (0x20L)        \\ disable P2, P3, P4 optimizations */
/*#define PF2_P3OptDisable        (0x40L)        \\ disable P3, P4 optimizations */
/*#define PF2_UseResPoke          (0x80L)        \\ disable RES_POKE */
/*#define PF2_MMXDisable         (0x100L)        \\ disable MMX, P2, P3, P4 optimizations */
/*#define PF2_OmitPICMetaData    (0x200L)        \\ JBIG2 - suppress writing PIC metadata to compressed file */
/*#define PF2_UseResSeek         (0x400L)        \\ J2K,F2DPLUS - allow opcode to use RES_SEEK's */
/*#define PF2_AllowResSeek       PF2_UseResSeek  \\ J2K,F2DPLUS - allow opcode to use RES_SEEK's */
/*#define PF2_P4OptDisable       (0x800L)        \\ disable P4 optimizations */
/*#define PF2_RetrieveAlpha                (0x00001000L) */
/*#define PF2_SaveAlpha                    (0x00001000L) */
/*#define PF2_HaveAlpha                    (0x00002000L) */
/*#define PF2_RGBAIsPremultiplied          (0x00004000L) */
/*#define PF2_RetrieveRGBANonPremultiplied (0x00008000L) */
/*#define PF2_SSSE3OptDisable              (0x00010000L) \\ disable SSSE3 optimizations */

typedef DWORD   THUMBNAIL;
#define THUMB_NONE      0
#define THUMB_4         1
#define THUMB_16        2
#define THUMB_64        3
#define THUMB_4F        4 /* use all dct coeffs in making 1/4 area thumbnail */
#define THUMB_256       4

/***************************************************************************\
*  This typedef designates the currently supported input file types.        *
\***************************************************************************/
typedef LONG    JPEG_TYPE;
#define JT_PIC          0
#define JT_BMP          1
#define JT_RAW          2
#define JT_PIC2         3
#define JT_EXIF         4

typedef LONG    REQUEST;
#define REQ_INIT        1
#define REQ_EXEC        2
#define REQ_CONT        3
#define REQ_TERM        4

/*.P*/
typedef LONG    RESPONSE;
#define RES_DONE                    1
#define RES_EXEC_DONE               1
#define RES_ERR                     2
#define RES_GET_DATA_YIELD          3
#define RES_PUT_DATA_YIELD          4
#define RES_YIELD                   5
#define RES_COLORS_MADE             6
#define RES_PUT_NEED_SPACE          7
#define RES_GET_NEED_DATA           8
#define RES_NULL_PICPARM_PTR        9
#define RES_SEEK                    10
#define RES_AUX_NEEDED              11
#define RES_ALLOCATE_APP2_BUF       12
#define RES_ALLOCATE_COMMENT_BUF    13
#define RES_ALLOCATE_OVERTEXT_BUF   14
#define RES_HAVE_COMMENT            15
#define RES_POKE                    16
#define RES_EXTEND_PIC2LIST         17
#define RES_Y_DONE_FIRST            18
#define RES_PUTQ_GET_NEED_DATA      19
#define RES_INIT_DONE               20
#define RES_PIC_PARM_INIT_DONE      21
#define RES_EXTEND_RCVRSTATE        22
#define RES_ERR_THREADS_LEAKED      23
#define RES_NEED_EXTERNAL_OVERLAY   24



/***************************************************************************\
*   Instance specific work area used by level 1 and level 2.                *
\***************************************************************************/

#if defined(PIC64)
  #define SIZE_PIC_STACK  16384
#else
  #define SIZE_PIC_STACK  8192
#endif

#ifdef __cplusplus
  typedef struct {
        BYTE        reserved;
  } WORK_AREA;
#elif defined(PIC64) && !defined(__SPARC__)
  #pragma pack(push, 8)
  typedef struct {
        jmp_buf     AppState;
        jmp_buf     PicState;
        BYTE        PicStack[SIZE_PIC_STACK];
        BYTE        InstState[564]; /* see internal.h */
        BYTE        Padding[4];
        BYTE        WorkArea[1];
            /* other stuff - should never allocate sizeof(WORK_AREA) bytes! */
  } WORK_AREA;
  #pragma pack(pop)
#elif defined(MACINTOSH) && defined(__ppc__) && defined(__DISPATCHER__)
  /*
   * This is a special case for the Mac/PowerPC Dispatcher.  In this case, the PPC opcodes
   * are built as Code Fragment Manager (CFM) executables, while the dispatcher is a
   * Mach-O executable.  Due to differences in the size of the jump buffer in these two
   * formats, the Work Area could be sized differently between the Dispatcher
   * and the Opcode.  In order to prevent this, the Work Area for the dispatcher
   * is sized so that the size of the jump buffer matches the size defined for
   * CFM executables.
   */
  typedef long* cfm_jmp_buf[70];
  typedef struct {
        cfm_jmp_buf  AppState;
        cfm_jmp_buf  PicState;
        BYTE         PicStack[SIZE_PIC_STACK];
        BYTE         InstState[564]; /* see internal.h */
        BYTE         Padding[4];
        BYTE         WorkArea[1];
            /* other stuff - should never allocate sizeof(WORK_AREA) bytes! */
  } WORK_AREA;
#else
  typedef struct {
        jmp_buf     AppState;
        jmp_buf     PicState;
        BYTE        PicStack[SIZE_PIC_STACK];
        BYTE        InstState[564]; /* see internal.h */
        BYTE        Padding[4];
        BYTE        WorkArea[1];
            /* other stuff - should never allocate sizeof(WORK_AREA) bytes! */
  } WORK_AREA;
#endif

typedef struct {
        BYTE PICHUGE *FrontEnd; /* Auxiliary, points just beyond contiguous data */
        BYTE PICHUGE *Start;    /* Points to start of queue */
        BYTE PICHUGE *Front;    /* Points to start of data in the queue */
        BYTE PICHUGE *Rear;     /* Points to start of empty space in the queue */
        BYTE PICHUGE *End;      /* Points just beyond queue buffer */
        BYTE PICHUGE *RearEnd;  /* Auxiliary, points just beyond contiguous space */
        DWORD        QFlags;    /* Flags for queue operation */
    } QUEUE;

#define Q_EOF       1           /* Set iff End-of-file reached on input */
#define Q_REVERSE   2           /* Set iff queue is reversed */
#define Q_IO_ERR    4           /* Set iff input or output error occurred */
#define Q_READ_WRAP 8           /* Set iff the Rear pointer has wrapped to Start */
#define Q_DID_WRITE 16          /* Set for Put iff RES_PUT_NEED_SPACE defer has been issued */
#define Q_DID_READ  16          /* Set for Get iff RES_GET_NEED_DATA defer has been issued */
#define Q_READING   32          /* set while level 2 is reading from Put queue, else clear */
#define Q_INIT      64          /* indicates initialization should be done in the queue */
#define Q_READFORSEEK  128      /* set if reads should be used instead of seeks to advance
                                    to a file offset past the largest file offset read so far
                                    (beta F2D only) */
#define Q_WRITEFORSEEK 256      /* set if writes should be used instead of seeks to advance
                                    to a file offset past the largest file offset written so far
                                    (beta F2D only) */
#define Q_RELAXED      512      /* strict queue or relaxed queue for F2D, D2F, UTL
                                    (not implemented yet) */

typedef struct {
        LONG         Width;     /* Width of image in pixels for given layer */
        LONG         Height;    /* Height of image in pixels for given layer */
        LONG         Stride;    /* Width of image buffer in bytes for given layer */
        LONG         Size;      /* Compressed size of layer (BYTES) */
        BYTE PICFAR *Handle;    /* Level 2 pointer to a buffer */
    } LAYER;

/*.P*/
/***************************************************************************\
*   Variables to/from PegasusQuery                                          *
\***************************************************************************/
typedef struct {
        DWORD        Reserved0;
        /* NOTE: this is used by PegasusQuery -- there is NO OPCODE for query */
#if defined(_MSC_VER)
        CDInfo PICHUGE *CDInfo;     /* PCD Information */
#else
        CDInfo PICHUGE* CDInfoData; /* GNU doesn't seem to like the name space collision
                                        (neither do we but we can't break the MSC compile) */
#endif
        BYTE PICHUGE *Reserved2;    /* Must be NULL! */
        BYTE PICHUGE *Reserved3;    /* Must be NULL! */
        BYTE PICHUGE *Reserved4;    /* Must be NULL! */
        BYTE PICHUGE *Reserved5;    /* Must be NULL! */
        BYTE PICHUGE *Reserved6;    /* Must be NULL! */
        BYTE PICHUGE *Reserved7;    /* Must be NULL! */
        BYTE PICHUGE *Reserved8;    /* Must be NULL! */
        PICFLAGS     PicFlags;      /* Only PF_IsGray is relevant */
        PICFLAGS     PicFlags2;     /* PF2_ flags above when implemented */
        LONG         NumOfPages;    /* # of pages in document (G3/G4) */
        LONG         PageNum;       /* Page # of 1st (or only) page (G3/G4) */
        DWORD        BitFlagsReq;   /* Bitmapped flags to indicate which */
                                    /*   values PegasusQuery should try to */
                                    /*   ascertain.  Set by the user. See */
                                    /* QBITxxx below. */
        DWORD        BitFlagsAck;   /* Bitmapped flags to indicate which */
                                    /*   of the requested values PegasusQuery */
                                    /*   was able to ascertain.  Note that */
                                    /*   some fields may be determined, and */
                                    /*   the resulting bits set, even if not */
                                    /*   specifically requested. */
                                    /* See QBITxxx below. */
        DWORD        ImageSize;     /* Size of image.  See QBIT_IMAGESIZE below. */
        DWORD        AuxSize;       /* Size required to hold all auxiliary */
                                    /*   data blocks. */
        DWORD        ImageNum;      /* Image number we're requesting query on */
                                    /*   (1 is 1st image) -- this is ignored */
                                    /*   for files that only contain 1 image */
                                    /*   (BMP, PCX, etc. and single-image */
                                    /*   TIFF, GIF, etc.) */
        DWORD        NumImages;     /* Number of images in file, if known */
        DWORD        SOIMarker;     /* Offset in file of SOI marker if TIFF JPEG */
        SUBSAMPLING  SubSampling;   /* if determinable and JPEG */
        REGION       Region;
        BYTE         LumFactor;     /* if present and JPEG */
        BYTE         ChromFactor;   /* if present and JPEG */
        BYTE         AllowedBitErr; /* where significant */
        BYTE         TIFFFirstByte; /* first byte of TIFF file when the first byte of Get is a
                                    TIFF IFD instead of being the first byte of the TIFF file
                                    also set TIFFIFDOffset to IFD offset within the file */
        WORD         ClusterHeaderVersion;
        WORD         DjVuImageType; /* see ImageType in DJVU_UNION */
        DWORD        TIFFCompression;   /* value of TIFF Compression tag except -3 for G32D */
        DWORD        TIFFPhotometricInterpretation; /* value of TIFF PhotometricInterpretation tag */
        DWORD        TIFFIFDOffset; /* when TIFFFirstByte is 'I' or 'M', this is the offset within
                                    the file of the first byte of the IFD -- the first byte in Get */
        WORD         PDFMajorVersion; /* the major version number of the PDF specification this file is built against */
        WORD         PDFMinorVersion; /* the minor version number of the PDF specification this file is built against */
        DWORD        PixelDataOffset; /* if a one strip TIFF (else 0), this is strip offset within the TIFF file */
        DWORD        PixelDataSize;   /* if a one strip TIFF (else 0), this is size of the strip in bytes */
        BYTE         TIFFOrientation; /* value of TIFF Orientation tag */
        BYTE         TIFFPredictor;   /* value of TIFF Predictor tag */
        BYTE         TIFFFillOrder;   /* value of TIFF FillOrder tag */
        BYTE         TIFFT4Options;   /* value of TIFF T4Options tag */
        WORD         NumPartitions; /* Number of slices if PF2_3D_Slices is set in PicFlags2, else not used */
        WORD         Reserved1;     /* don't forget alignment */
    } PEGQUERY;

#define PF2_IsExif                  (0x00000001L)
#define PF2_IsDCX                   (0x00000002L)
        /* (PegasusQuery) file has APP1 marker with Exif signature */
////#define PF2_SpecialTIFF     (0x00000004L)
        /* (shared with F2D_STRUC) process TIFF files with "special" rules */
////#define PF2_GIFAllow1and4Bpp (0x00000008L)
        /* (shared with F2D_STRUC) GIF files with <= 2 colors reported as 1bpp,
            else GIF files with <= 16 colors reported as 4bpp, else GIF files reported as 8bpp */
////#define PF2_3D_Slices        (0x02000000L)
        /* (shared with J2K_UNION) set if J2K image has 3D slices */

#define QBIT_BISIZE (0x1L)
        /* Head.biSize -- this is the size of the bitmap info header.  It */
        /* will generally be sizeof(BITMAPINFOHEADER), except in the */
        /* case of .BMP files, where it may be larger. */
#define QBIT_BIWIDTH (0x2L)
        /* Head.biWidth -- this is the width of the image in pixels. */
#define QBIT_BIHEIGHT (0x4L)
        /* Head.biHeight -- this is the height of the image in pixels. */
        /* This value may be negative.  If so, it indicates that if the */
        /* image is converted to a DIB via opcode 81, it will be upside */
        /* down.  (Opcode 81 may generate upside-down DIBS.) */
#define QBIT_BIPLANES (0x8L)
        /* Head.biPlanes -- this will always be 1. */
#define QBIT_BIBITCOUNT (0x10L)
        /* Head.biBitCount -- total number of bits per pixel for the */
        /* image.  May be weird (e.g., 7).  When opcode 81 generates a */
        /* DIB, it will create only 1, 4, 8, 16, or 24 bpp images. */
#define QBIT_BICOMPRESSION (0x20L)
        /* Head.biCompression -- this is the basic file type.  See the */
        /* BI_???? manifest constants near the top of this file. */
#define QBIT_BISIZEIMAGE (0x40L)
        /* Head.biSizeImage -- this is the size of the image data if */
        /* converted to a DIB.  Pixels will be padded out to 1, 4, 8, */
        /* 16, or 24 bpp, and scanlines will be padded out to a 4-byte */
        /* boundary. */
#define QBIT_BIXPELSPERMETER (0x80L)
        /* Head.biXPelsPerMeter -- generally of no use */
#define QBIT_BIYPELSPERMETER (0x100L)
        /* Head.biYPelsPerMeter -- generally of no use */
#define QBIT_BICLRUSED (0x200L)
        /* Head.biClrUsed -- a RGB image may have any value here, or */
        /* 0 (if non-zero, this is presumably some sort of "suggested" */
        /* palette).  For palettized images, this is the number of colors */
        /* in the palette and MUST be >=2 and <= (1 << bpp). */
#define QBIT_BICLRIMPORTANT (0x400L)
        /* Head.biClrImportant -- generally of no use. */
#define QBIT_IMAGESIZE (0x800L)
        /* ImageSize -- size of the input "file".  This is the number of */
        /* bytes, starting from the beginning of the "file", which must be */
        /* accessed in order to get all header information and image data */
        /* in order to display an image.  Therefore, other auxiliary */
        /* information that may be present in the file, but not required */
        /* for image display (e.g., author, comments, etc) will not be */
        /* included in this size.  Note that for 'random' formats such as */
        /* TIFF, it is possible that this space could contain information */
        /* for other images. */
#define QBIT_AUXSIZE (0x1000L)
        /* AuxSize -- size of the buffer required to read in all */
        /* 'auxiliary' information (comments, gamma curves, etc.) in */
        /* preparation for an opcode 81 call.  Note that opcode 81 does */
        /* not require that this be known in advance, but things are more */
        /* efficient if it is known. */
#define QBIT_NUMIMAGES (0x2000L)
        /* NumImages -- number of images in the file.  Most often, 1. */
        /* This count may not be obtainable with a small amount of data. */
#define QBIT_COMMENT (0x4000L)
        /* Iff Comment field set in PIC_PARM */
#define QBIT_PALETTE (0x08000L)
        /* Set this if you want the palette returned in ColorTable */
        /* if possible. */
#define QBIT_SOIMARKER (0x10000L)
                /* Set if we found SOIMarker in a TIFF JPEG file */
#define QBIT_PICJPEG (0x20000L)
        /* set if a JPEG image has Pegasus APP1 marker data and PicVer,
            VisualOrient, LumFactor, ChromFactor and possibly a
            color table are returned */
#define QBIT_REGION  (0x40000L)
        /* set if u.QRY.Region is filled in */
#define QBIT_PCDINFO (0x80000L)
        /* Located and recorded PCD Information (Kodak PhotoCD) */


/* for progressive JPEG */
typedef struct {
        /* NOTE: Co = 3 (interleaved yCbCr) is allowed ONLY for dc scans. */
        /* Also, for now, the dc scans MUST be interleaved for color image) */
        LONG   Co; /* Component indicator (0=y, 1=Cb, 2=Cr, 3=interleaved YCbCr) */
        LONG   Ss; /* dct coeff. index of start of scan */
        LONG   Se; /* dct coeff. index of end of scan (0 indicates dc) */
        LONG   Ah; /* (see JPEG spec) */
        LONG   Al; /* (see JPEG spec) */
        } SCAN_PARM;


/***************************************************************************\
*   Variables for reading DIBS (Operation D2S)                              *
\***************************************************************************/
typedef struct {
        DWORD        Reserved0;
        /*  Information prior to REQ_INIT */
        BYTE PICHUGE *AppField;     /* points to user info of AppLen bytes */
        BYTE PICHUGE *QTable;       /* NULL or points to Q-table values;
                                     Not in zig_zag order! There only need be   64 bytes for
                                     grayscale input.  For color,   first 64 bytes are for
                                     luminance (Y) and the next 64 for Cb. If this is
                                     followed by a zero byte, the same 64 are used for Cr
                                     as were for Cb.  If not, it is assumed that the 64
                                     bytes following the 64 Cr quant.   bytes are used for Cb.
                                     Thus in the color case, this table must be at least 129
                                     bytes, and 192 bytes if there are 3 different quant. tables. */
        SCAN_PARM PICHUGE *ScanParms;    /* For progressive JPEG */
        BYTE PICHUGE *ExifThumbnail; /* pointer to Jfif-format thumbnail image for Exif output if
                                     desired.  Error unless JpegType == JT_EXIF &&
                                     ExifThumbnailToMake == THUMB_NONE */
        BYTE PICHUGE *Reserved5;    /* Must be NULL! */
        BYTE PICHUGE *Reserved6;    /* Must be NULL! */
        BYTE PICHUGE *Reserved7;    /* Must be NULL! */
        BYTE PICHUGE *Reserved8;    /* Must be NULL! */
        PICFLAGS     PicFlags;      /* PF_IsGray, PF_IsNotDibPad, PF_OptimizeHuff, etc. */
        PICFLAGS     PicFlags2;     /* PF2_ flags above when implemented */
        LONG         PrimClrToMake; /* Desired number of colors in primary palette */
        LONG         SecClrToMake;  /* Desired number of colors in secondary palette */
        LONG         LumFactor;     /* Luminance compression factor 0..255.
                                    Ignored if QTable above is not NULL. */
        LONG         ChromFactor;   /* Chrominance compression factor 0..255 */
        SUBSAMPLING  SubSampling;   /* SS_field, 111/211/211v/411 subsampling */
        JPEG_TYPE    JpegType;      /* JT_ fields, desired PIC/BMP/RAW/PIC2 JPEG file type. JT_BMP not supported. */
        LONG         AppFieldSize;  /* Size of AppField in bytes. */
        LONG         AppFieldLen;   /* 0 or length of data within AppField */
        LONG         NumOfPages;    /* Number of pages in document */
        LONG         PageNum;       /* Page Number of first (or only) page */
        LONG         Context;       /* number of context pts (IM1, IM4, & IM8) */
        /*  Derived information about the Uncompressed Image after REQ_INIT */
        LONG         StripSize;     /* Minimum size of buffer to hold a strip */
        LONG         WidthPad;      /* Actual byte size of UI line (with any pad) */
        LONG         NumProgScans;  /* for progressive JPEG */
        LONG         ExifAppsToKeep;/* use AppsToOmit */
        DWORD        ExifThumbnailLen;
            /* length of jiff thumbnail provided at ExifThumbail.  Error unless
                JpegType == JT_EXIF && ExifThumbnailToMake == THUMB_NONE */
        THUMBNAIL    ExifThumbnailToMake;
            /* != THUMB_NONE to create a thumbnail in Exif output.
                (requires and uses OP_S2D or OP_SE2D and OP_D2S or OP_D2SE)
                error unless JpegType == JT_EXIF && ExifThumbnailLen == 0 */
        LONG         AppsToOmit;
            /* bit 0 on -> do not put a JFIF App0 marker in JPEG output stream (recommended for JT_EXIF)
               bit 1 on -> do not put a PIC App1 marker in JPEG output stream (recommended for JT_EXIF)
               bit n on -> do not put APP<n> marker */
        LONG         RightShift12; /* only used for compressing 2 byte-per-pixel gray
                images to 12-bit JPEG format.  This is how much the data needs to
                be right-shifted so that the high 4 bits of the word are guaranteed to
                be 0. It can take values from   0 through 4. For example if the
                data were left-justified in the word, this parameter would
                be set to 4. */
        DWORD        ResolutionUnit;
                                    /* 0 = none (X/Y specify aspect ratio if != 0)
                                        else if X or Y are 0 then biX/YPelsPerMeter
                                        in PIC_PARM.Head are used */
        DWORD        XResolution;   /* horizontal pixels/ResolutionUnit */
        DWORD        YResolution;   /* vertical pixels/ResolutionUnit */
        /* next 3 for adjusting brightness and contrast on input */
        LONG         Brightness;   /* -4095...4095, with 0 -> no change */
        LONG         Contrast;     /* -4095...4095, with 0 -> no change */
        LONG         ContrastOfs;  /* -4095...4095, with 0 -> no change */
        LONG         RestartInterval;  /* number of MCUs between restart markers */
        DWORD        Planar1Offset;/* 0 if "simple" compression from planar yuv */
        DWORD        Planar2Offset;/* 0 if "simple" compression from planar yuv */
        /* else not simple means the compressed image is de-interlacing lines from the
           input image or a subrectangle of the input image is being compressed
           (for planar yuv, the Get queue must hold the entire input image)
           if Q_REVERSE is not set
             if Get.Front is offset N samples from the actual start of the entire image Y plane,
             set Planar1Offset to <size Y plane> + N / 2 where <size Y plane> is
             <width> * <height> for the <width> and <height> of the entire image.
             Set Planar2Offset to Planar1Offset + <size Y plane> / 4
             Thus Planar1/2Offset are the offsets from Get.Front of the U or V samples
             corresponding to the Y sample pointed to by Get.Front.
           else error if Q_REVERSE is set
        */
        } DIB_INPUT;


/* defined above */
/* #define PF2_Cosited                 0x00000001L */
        /* (JPEG pack and exp) Applies for now to 211 (horizontal 2 to 1) Chrom. sampling only.  */
        /* For exp, must also set PF2_SmoothUpsample to have any effect. */
#define PF2_UseResPoke  (0x00000080L)   /* for PF_MakeColors, Sequential JPEG pack used to require that
                                        you set the colors in the JPEG header yourself following
                                        compression.  If your Put queue is not large enough to hold
                                        the entire compressed image, setting this flag allows JPEG
                                        pack to use RES_POKE responses to prompt you through setting
                                        these colors.  The Put queue must be at least
                                        (PrimClrToMake + SecClrToMake) * sizeof(RGBTRIPLE) bytes.
                                        For a PIC2 file, you will get a RES_POKE even if this flag
                                        is not set and your Put queue < compressed size */
/* defined for ROR */
/* #define PF2_OmitISOStandardHuffmanTbls 0x00000008L */
    /* If PF_OptimizeHuff and PF_ElsCoder are clear, then do not write
        the Huffman code markers (FFC4) to the output JPEG stream; this is primarily
        for MJPEG streams because the result doesn't conform to the still-image JPEG standard */
#define PF2_CompressedYUV           (0x00000010L)
        /* (JPEG pack and exp) yuv is compressed to 16-235 lum and 16-240 chrom.  */
/* See PF2_RgbJpeg    defined after DIB_OUTPUT; this flag applies here also. */

/*.P*/
/***************************************************************************\
*   Variables for writing DIBs (Operations S2D, P2D)                   *
\***************************************************************************/
typedef struct {
        DWORD        Reserved0;
        /*  Information prior to REQ_INIT */
        BYTE PICHUGE *AppField;     /* points to user info of AppLen bytes */
        BYTE PICHUGE *MadeColorTable;
        BYTE PICHUGE *PrecisionReq; /* PJPEG - 192 byte array of precision requested */
        BYTE PICHUGE *ColorMap;     /* points to pre-allocated 32K area for ColorMap */
        BYTE PICHUGE *ExifThumbnail;/* exif thumbnail buffer of ExifThumbnailLen */
        BYTE PICHUGE *GrayMap12Bit; /* If not NULL, Remaps 12-bit grayscale jpgs; will be assumed to be
             words rather than bytes is DibSize is 16 rather than 8. */
        BYTE PICHUGE *Reserved7;    /* Must be NULL! */
        BYTE PICHUGE *Reserved8;    /* Must be NULL! */
        PICFLAGS     PicFlags;      /* PF_MakeGray, PF_NoDibPad, PF_Dither etc. */
        PICFLAGS     PicFlags2;     /* PF2_ flags above when implemented */
        LONG         DibSize;       /* 1/4/8/16/24/32, may change if gray image */
        THUMBNAIL    Thumbnail;     /* 0/1/2/3 => none / 1/4 / 1/16 / 1/64 thumbnail */
        LONG         NumScansReq;   /* PJPEG User's requested number of scans */
                                    /*   (0 means do all) */
        LONG         NumScansDone;  /* PJPEG Scans actually done upon return */
        LONG         NumBytesDone;  /* PJPEG Bytes comprising the scans returned */
        LONG         GraysToMake;   /* if > 0 then number of gray levels to make */
        LONG         PrimClrToMake; /* Desired number of colors in primary palette */
        LONG         SecClrToMake;  /* Desired number of colors in secondary palette */
        /*  Derived information about the Uncompressed Image after REQ_INIT */
        LONG         StripSize;     /* Minimum size of buffer to hold a strip */
        LONG         WidthPad;      /* Actual byte size of UI line (with any pad) */
        LONG         LumFactor;     /* Luminance compression factor 0..255 */
        LONG         ChromFactor;   /* Chrominance compression factor 0..255 */
        SUBSAMPLING  SubSampling;   /* SS_field, 111/211/211v/411 subsampling */
                                    /*   (returned by Pegasus) */
        JPEG_TYPE    JpegType;      /* (returned by Pegasus). JT_ fields, PIC/BMP/RAW/PIC2 JPEG file type (JT_BMP not supported) */
        LONG         AppFieldSize;  /* Size of AppField in bytes. */
        LONG         AppFieldLen;   /* 0 or logical length of Application data */
        LONG         NumOfPages;    /* Number of pages in document */
        LONG         PageNum;       /* Page Number of first (or only) page */
        LONG         Context;       /* number of context pts (IM1, IM4, & IM8) */
        LONG         DitherType;    /* 0 = Floyd-Steinberg, 1 = fast-error-diffusion */
        LONG         YuvOutputType; /* BI_UYVY or BI_YUY2.  Ignored unless PF_YuvOutput is set. */
        LONG         ExifThumbnailLen; /* size of ExifThumbnail buffer */
        DWORD        ResolutionUnit;
                                    /* 0 = none (X/Y specify aspect ratio if != 0)
                                        else if X or Y are 0 then biX/YPelsPerMeter
                                        in PIC_PARM.Head are used */
        DWORD        XResolution;   /* horizontal pixels/ResolutionUnit */
        DWORD        YResolution;   /* vertical pixels/ResolutionUnit */
        /* next 3 for adjusting brightness and contrast on output */
        LONG         Brightness;   /* -4095...4095, with 0 -> no change */
        LONG         Contrast;     /* -4095...4095, with 0 -> no change */
        LONG         ContrastOfs;  /* -4095...4095, with 0 -> no change */
        DWORD        EnhanceMethod; /* Only if PF_NoCrossBlockSmoothing is NOT set,
                                        OR one or more EM_ flags below into this field */
        DWORD        Planar1Offset;/* 0 if "simple" decompression to planar yuv */
        DWORD        Planar2Offset;/* 0 if "simple" decompression to planar yuv */
        /* else not simple means the output image is re-interleaving lines or
           compositing the decompressed image into a larger image subrectangle,
           (for planar yuv, the Get queue must hold the entire input image)
           if Q_REVERSE is not set
             if Put.Front is offset N samples from the actual start of the entire image Y plane,
             set Planar1Offset to <size Y plane> + N / 2 where <size Y plane> is
             <width> * <height> for the <width> and <height> of the entire image.
             Set Planar2Offset to Planar1Offset + <size Y plane> / 4
             Thus Planar1/2Offset are the offsets from Put.Front of the U or V samples
             corresponding to the Y sample pointed to by Put.Front.
           if error if Q_REVERSE is set
        */
        } DIB_OUTPUT;

#define EM_CrossBlockSmoothing      (0x0L)
#define EM_ReduceContouring         (0x1L)
#define EM_ReduceArtifacts          (0x2L)
#define EM_ReduceBlockEdgeArtifacts (0x4L)

#define PF2_Cosited                 0x00000001L
        /* (JPEG pack and exp) Applies for now to 211 (horizontal 2 to 1) Chrom. sampling only.  */
        /* For exp, must also set PF2_SmoothUpsample to have any effect. */
#define PF2_SmoothUpsample          0x00000002L
        /* (JPEG exp) Applies for now to 211 (horizontal 2 to 1) Chrom. subsampling only.  */
#define PF2_565Output               0x00000004L
        /* (Seq Expand). Set for 565 rather than 555 rgb output if 16-bit output is chosen for color images. */
#define PF2_ClippedYUV             (0x00000008L)
    /* (JPEG exp only) yuv is clipped (not compressed) to 16-235 lum and 16-240 chrom.  */
    /* PF2_CompressedYUV and PF2_ClippedYUV should not both be set, but if they are,
        PF2_CompressedYUV takes precedence. */
/* The following was defined above after DIB_INPUT, used by both JPEG pack and expand */
/* #define PF2_CompressedYUV           (0x00000010L)  */
        /* (JPEG pack and exp) yuv is compressed to 16-235 lum and 16-240 chrom.  */
/* following used by lossy JPEG for 12-bit images, */
/*#define PF2_P2AndP3OptDisable   (0x20L)     \\ disable P2 optimizations */
/*#define PF2_P3OptDisable        (0x40L)     \\ disable P3 optimizations */
#define PF2_RgbJpeg             (0x80000000L)
#define PF2_NoYccTransform      (0x80000000L)
#define PF2_RgbaOutput          (0x40000000L)   // for 4-component compressed instead of CMYK output
        /* (JPEG exp and pack - 3-component image components are compressed RGB
            so it's decompressed without the YCbCr->RGB color conversion */
        /*                    - 4-component CMYK is not transformed to YCCK */
#define PF2_AcrobatCmyk         (0x20000000L)
        /* (JPEG exp and pack) for Acrobat-style CMYK instead of Photoshop-style CMYK */
/* following value is reserved (same value as ROR for future enhancement returning the
    same status from S2D
#define PF2_ExifInputIsBigEndian (0x10000000L)
*/

/******************  For Wizard ***********************************/

#define MAX_REGIONS 16
/* This structure is used for regions in wizard (see REORIENT structure). */
typedef struct {
                WORD         LumFactor; /* 0-255 gives quality as in compress, can go beyond 255 here. */
                WORD         ChromFactor;
                LONG         NumTbls; /* Number of quant. tables to be supplied, or a code.
                                         255-> do not change quantization from existing,
                                         0->use luminance and chrominance values instead of supplying q-tables,
                                         1->grayscale, 2 or 3 -> color. */
                WORD         QTbls[1]; /* If NumTbls != 0 or 255, there should actually be
                                         64*NumTbls words here. The 1 is a dummy. */
        } REGION_INFO;


/***************************************************************************\
*   Variables for re-orienting JPEG files (Operation ROR)                   *
\***************************************************************************/
typedef struct {
        DWORD         Reserved0;
        BYTE PICHUGE *RegionInfo; /* Null unless doing regions.
                                     Put in data according to REGION_INFO structure above,
                                     for each of NumRegions regions (concatenated). After that, put in
                                     RegionMap, at offset RegionMapOffset from beginning of RegionInfo.
                                     RegionMap is an array of bytes, corresponding to the 8x8 blocks in
                                     the image.  The value in each byte, in row scan order, tells
                                     which region number the corresponding 8x8 block is in.  The
                                     region numbers are from 0 through NumRegions - 1. RegionMap
                                     has width and height given below in RegionMapWidth and
                                     RegionMapHeight. */
        BYTE PICHUGE *QTableReq;    /* (optional) Ignored unless Requantize
                                     below is set to 1, or JoinFlags & JF_DoJoin != 0 and
                                     JoinFlags & JF_UseRequestedQuantization != 0. Allocated
                                     and set by user.   Points to Q-table values, if used.
                                     Not in zig_zag order! There only need be   64 bytes for
                                     grayscale input.  For color,   first 64 bytes are for
                                     luminance (Y) and the next 64 for Cb. If this is followed
                                     by a   zero byte, the same 64 are used for Cr as   were for Cb.
                                     If not, it is assumed that the 64 bytes following the 64
                                     Cr quant. bytes are used for Cb.  Thus in the color case,
                                     this table must be at least 129 bytes, and 192 bytes if
                                     there are 3 different quant. tables. */
        BYTE PICHUGE *ExifThumbnail; /* pointer to Jfif-format thumbnail image for Exif output if
                                     desired.  Error unless JpegType == JT_EXIF &&
                                     ExifThumbnailToMake == THUMB_NONE */
        BYTE PICHUGE *MapY; /* If not NULL, Remaps Y component (used for gamma correction, takes precedence over Yscale and Yshift). */
        BYTE PICHUGE *MapCb; /* If not NULL, Remaps Cb component (used for gamma correction, takes precedence over Yscale and Yshift). */
        BYTE PICHUGE *MapCr; /* If not NULL, Remaps Cr component (used for gamma correction, takes precedence over Yscale and Yshift). */
        BYTE PICHUGE *Reserved7;    /* Must be NULL! */
        BYTE PICHUGE *Reserved8;    /* Must be NULL! */
        PICFLAGS     PicFlags;
        PICFLAGS     PicFlags2;
        LONG         Pad;           /* 1-> pad to fill mcu if needed, 0->trim ,
                                     2-> pad width only, 3-> pad height only (in case of rotation,
                                     width and height here refer to original image). */
        LONG         KeepColors;    /* 1->put colors from pic file in APP1, 0->discard */
        LONG         LumFactorReq;  /* (optional) Ignored unless QTableReq is
                                     NULL and: Requantize   below is set to 1, or    JoinFlags &
                                     JF_DoJoin != 0 and JoinFlags & JF_UseRequestedQuantization
                                     != 0. Luminance compression factor 0..255 */
        LONG         ChromFactorReq; /* Same comment as above. Chrominance compression factor 0..255 */
        LONG         Requantize;    /* Set by User. 1-> change the quantization interval size
                                    for each DCT coefficient to the odd multiple
                                    of the existing value which is nearest to
                                    the value in QTableReq if this is present,
                                    or to the one implied by LumFactorReq and
                                    ChromFactorReq otherwise. This produces the
                                    same quantized DCT coefficients that would
                                    have been obtained starting from the raw
                                    image using this quantization.
                                    2-> for each DCT coefficient, use requested
                                    quantization value when larger than one in
                                    input file, otherwise use one in input file.
                                    NOTE: this does not produce same JPEG file
                                    as if we had started with the raw image
                                    using this quantization! Use at your own risk.
                                    3-> for each DCT coefficient, use requested
                                    quantization value, period.
                                    0-> do not requantize  */
        LONG         RequantizationDone;  /* set by Pegasus if Requantize is
                                    1. 1->some change was made to quantization,
                                    0-> no change was made. */
        LONG         AppsToKeep;    /* The default is to remove all App data segments
                                    except JFIF and PIC. The bit positions on in
                                    AppsToKeep, numbered from least significant to
                                    most, tell which App data segments to keep,
                                    other than JFIF and PIC, for positions 0,1,...15.
                                    For example, if AppsToKeep = 0xC (1100 binary),
                                    this means keep App2 and App3 data segments.
                                    JFIF is removed only if bit position 16 is ON,
                                    and PIC is removed only if bit position 17 is ON. */
        LONG         RemoveComments; /* if the 0th bit is on, remove the existing
                                    comments (if not, keep all comments).
                                    If the 1st bit is on, do NOT add the comment
                                    Pegasus Imaging Corp (if not, add this
                                    comment, even if bit 0 is on, unless a comment
                                    whose first 15 characters are Pegasus Imaging
                                    already remains in the file). */
        JPEG_TYPE    JpegType;      /* Output type (set by application). Only JT_RAW and JT_PIC2 are supported */
        BYTE         OutputKeyField[8];
                /* ^ Output encode key -- currently only for JpegType == JT_PIC2 &&
                                if OutputKeyField[0] != 0.  Eventually also for JpegType ==
                                JT_RAW && OutputKeyField[N] != 0 for any N = 0, .., 7 */
        LONG         JoinOffset; /* offset from Get.Start to the start of
                    a second image in Get buffer to be joined to first (see JoinFlags
                    below). If this is 0,   Pegasus will assume offset is unknown and
                    will scan to attempt to find it. The Get buffer must be large
                    enough to hold ALL of the   first image and up through the first scan
                    header of the second image. */
        DWORD   JoinFlags;  /* These flags are set to cause two images to be joined
                     in manner desired. See the defined values for JoinFlags below. */
        DWORD   InsertTransparencyLum; /* A value between 0 and 256, inclusive, which
                    determines how much of the luminance comes from the pixels of the original
                    image when inserting an image.  A value of 0 means the INSERTED
                    image determines the luminance in its   region.  A value of 256 means the
                    the original image completely determines the luminance.
                    This has meaning only when JF_Insert is set (see below). */
        DWORD   InsertTransparencyChrom; /* A value between 0 and 256, inclusive, which
                    determines how much of the chrominance comes from the pixels of the original
                    image when inserting an image. See above.
                    This has meaning only when JF_Insert is set (see below). */
        LONG    NumRegions; /* 1 or greater to have effect; <= MAX_REGIONS */
        DWORD   RegionMapOffset; /* offset from RegionInfo to RegionMap. */
        WORD    RegionMapWidth; /* Must be >= (image_width + 7)/8.  Any extra columns skipped over */
        WORD    RegionMapHeight; /* Must be >= (image_height + 7)/8. Any extra rows ignored */
        /* The following color space transforms are not allowed if doing requantize, join, or regions. */
        SHORT   YShift; /* 0 if not transforming luminance (Y); -256 ... 255 otherwise. */
        /* YShift represents increase (decrease if < 0) in brightness. */
        SHORT   YScale; /* 0 if not transforming luminance (Y); -128 ... 127 otherwise. */
        /* YScale represents increase (decrease if <0) in contrast.  Usually the
        contrast needs to be increased if the brightness is increased, so these two
        are often used together. */
        SHORT   CbShift; /* 0 if not transforming chrominance; -256 ... 255 otherwise. */
        SHORT   CbScale; /* 0 if not transforming chrominance; -128 ... 127 otherwise. */
        SHORT   CrShift; /* 0 if not transforming chrominance; -256 ... 255 otherwise. */
        SHORT   CrScale; /* 0 if not transforming chrominance; -128 ... 127 otherwise. */
        DWORD     ExifThumbnailLen;
            /* length of jiff thumbnail provided at ExifThumbail.  Error unless
                JpegType == JT_EXIF && ExifThumbnailToMake == THUMB_NONE */
        THUMBNAIL ExifThumbnailToMake;
            /* != THUMB_NONE to create a thumbnail in Exif output.
                (requires and uses OP_S2D or OP_SE2D and OP_D2S or OP_D2SE)
                error unless JpegType == JT_EXIF && ExifThumbnailTomake == THUMB_NONE */
        LONG    MaxRqtblSize; /* only applies to Flash */
        } REORIENT;

        /* settings for JoinFlags for join operation above (these are "ORed with JoinFlags). */
#define JF_DoJoin    0x00000001  /* Set if two images are to be joined. All
        other join flags are ignored if this is not set.  This is NOT allowed
        unless: Requantize is zero, F_Crop is NOT set in PicParm Flags, and
        VisualOrient is zero in PicParm. */
#define JF_LeftRight 0x00000002  /* Set if images are to be joined left-right
        rather than top-bottom (the default is top-bottom) */
#define JF_SecondOnTopLeftInsert 0x00000004  /* Set if second image in Get buffer
     goes on top or left or inserted; default is first is top or left or inserted. */
#define JF_UseSecondSubsampling 0x00000008  /* Set if subsampling is to be
        taken from second image in Get buffer; default is from first image.
        WARNING: If the two images do not   have the same subsampling, some inverse
        and forward DCT operations are required on the chrominance components. */
#define JF_UseRequestedQuantization 0x00000010  /* Set if quantization of
        joined image is to be input by app, either by supplying a non-null
        pointer for QTableReq above; if that is null, Pegasus will use
        LumFactorReq and ChromFactorReq above. */
#define JF_UseSecondQuantization 0x00000020  /* This is ignored if
        JoinFlags & JF_UseRequestedQuantization != 0.  Set this if quantization
        is to be taken from second image in get buffer; the default is to take
        quantization from the first. */

        /* WARNING: if the joined image quantization is different from the input
        quantization for either image, some new error may be introduced, larger
        than would have been expected from the indicated quantization table.
        Even when an output quantization interval is larger than the input one,
        the error in a coefficient is no longer guaranteed to be no more than half
        of the output quantization interval size.   This has to do with rounding. */

#define JF_Insert    0x00000040  /* Set if one image is to be inserted into
        the other rather than placed side-by-side. NOTE: This is considered a
        version of joining two images, so JF_DoJoin must also be set in order for
        this to have meaning.  The first image in the Get   buffer will be
        inserted in the second image unless the flag JF_SecondOnTopLeftInsert
        is set.  If JF_Insert is set, then CropXoff and CropYoff in the PIC_PARM
        structure must be set;  this gives the location of where to put the upper
        left corner of the inserted image.  This will be rounded down to the nearest
        multiple of MCU width and height of the surrounding image.  The width and
        height of the inserted image will be rounded down to a multiple of MCU width
        and height, and made to fit in the surrounding image if too large. Also
        if JF_Insert is set, InsertTransparencyLum and InsertTransparencyChrom
        must be given a value between 0 and 256 inclusive (see above). */

        /* Settings for PicFlags2 above */
#define PF2_ClipDc  0x00000001L  /* not operational at this time */
#define PF2_ClipAc  0x00000002L  /* not operational at this time */
#define PF2_UseISOStandardHuffmanTbls    0x00000004L  /* (ROR) The default is to compute optimal Huffman codes */
#define PF2_OmitISOStandardHuffmanTbls 0x00000008L
    /* (ROR) If PF2_USEISOStandardHuffmanTbls is also set, then do not write
    the Huffman code markers (FFC4) to the output JPEG stream; this is primarily
    for MJPEG streams because the result doesn't conform to the still-image JPEG
    standard */
#define PF2_ExifInputIsBigEndian (0x10000000L) /* (ROR) set if input exif tags are big-endian */

/***************************************************************************\
*   Variables for converting Seq to Progressive JPEG (Operations S2P)       *
\***************************************************************************/
typedef struct {
        DWORD        Reserved0;
        /*  Buffer information */
        SCAN_PARM PICHUGE *ScanParms; /* For progressive JPEG */
        BYTE PICHUGE *Reserved2;    /* Must be NULL */
        BYTE PICHUGE *Reserved3;    /* Must be NULL */
        BYTE PICHUGE *Reserved4;    /* Must be NULL */
        BYTE PICHUGE *Reserved5;    /* Must be NULL! */
        BYTE PICHUGE *Reserved6;    /* Must be NULL! */
        BYTE PICHUGE *Reserved7;    /* Must be NULL! */
        BYTE PICHUGE *Reserved8;    /* Must be NULL! */
        PICFLAGS     PicFlags;
        PICFLAGS     PicFlags2;     /* PF2_ flags above when implemented */
        LONG         NumProgScans;  /* for progressive JPEG */
        } TRANS2P;

/***************************************************************************\
*   Variables for converting Progressive to Seq JPEG (Operation P2S)        *
\***************************************************************************/
typedef struct {
        DWORD        Reserved0;
        /*  Buffer information */
        BYTE PICHUGE *Reserved1;    /* Must be NULL! */
        BYTE PICHUGE *Reserved2;    /* Must be NULL */
        BYTE PICHUGE *Reserved3;    /* Must be NULL */
        BYTE PICHUGE *Reserved4;    /* Must be NULL */
        BYTE PICHUGE *Reserved5;    /* Must be NULL! */
        BYTE PICHUGE *Reserved6;    /* Must be NULL! */
        BYTE PICHUGE *Reserved7;    /* Must be NULL! */
        BYTE PICHUGE *Reserved8;    /* Must be NULL! */
        PICFLAGS     PicFlags;
        PICFLAGS     PicFlags2;     /* PF2_ flags above when implemented */
        LONG         NumScansReq;   /* num progressive JPEG scans to decode
                                        (0 means do all) */
        LONG         NumScansDone;  /* PJPEG Scans actually done upon return */
        PICPTRDIFFT  NumBytesDone;  /* PJPEG Bytes comprising the scans returned */
        SUBSAMPLING  SubSampling;   /* SS_field, 111/211/211v/411 subsampling
                                        (returned by Pegasus) */
    } TRANP2S;

/*.P*/
/***************************************************************************\
*   Info is already in the PIC_PARM structure                               *
\***************************************************************************/
/* INTERNAL must be the largest union variant */
typedef struct {
        DWORD        Reserved0;
        BYTE PICHUGE *ReservedPtrs[8]; /* Must be NULL! */
        PICFLAGS     PicFlags;
        PICFLAGS     PicFlags2;
        PICSIZET     Reserveds[64];
    } INTERNAL;

/*.P*/
/***************************************************************************\
*   Converting supported file formats to DIBs (F2D) (opcode 81)             *
\***************************************************************************/

#define OP8X_NPOINTERS  20          /* way more than enough */
#define OP8X_NVARS      20          /* way more than enough */
typedef struct {
        BYTE PICFAR *ptr8X[OP8X_NPOINTERS];
        DWORD vars[OP8X_NVARS];
        } OP8X_WORK_AREA;
/* This is the work area (in p->Reserved) for opcodes 80, 81, 82.  It basically */
/* consists of a number of malloced pointers that all get released */
/* at PegasusTerm time.  Currently, vars[0] is a flag which, if set, */
/* tells REQ_EXEC to do a REQ_CONT (but only for opcode 81) -- this is so */
/* the app that calls Pegasus can call REQ_EXEC for multiple dibs, but */
/* REQ_CONT will actually be executed.  The area must obviously initially */
/* be nulled out. */

typedef struct {
        DWORD        Reserved0;
        BYTE PICHUGE *AuxSpace;
                /* If auxiliary data (e.g., Author, etc) is to be retained, this is a */
                /* pointer to the area to contain these chunks.  If the application */
                /* is not interested in these chunks, this pointer must be NULL.  The */
                /* application is responsible for allocating this space.  If there is */
                /* not enough room to hold the data, the response RES_SPACE_NEEDED */
                /* will result, which means that AuxNeeded (below) additional bytes */
                /* need to be allocated.  The current allocated size of this buffer */
                /* must be set in AuxSize below, and maintained each time it is */
                /* increased by a response to RES_SPACE_NEEDED. */
        BYTE PICHUGE *Ptr2;    /* Must be NULL!, except for PhotoCD, in which */
                /* case it must point to the filename for the PhotoCD file */
        BYTE PICHUGE *GIFHead;      /* -> 781 byte buffer for GIF header if you
                                        want to process animated GIF's */
        DWORD PICHUGE *TIFFIFDOffsetArray; /* != 0 and then is a pointer to
                an array of PIC_PARM->ImageNumber DWORDS and is filled in with TIFF IFD
                offsets from 1 .. p->ImageNumber */
        BYTE PICHUGE *MaskBuffer;   /* BI_ICO returns ths icon AND mask here.  If this is NZ and
                                        MaskBufferSize is large enough, then the opcode will use your buffer
                                        and you will free the buffer when you're finished with it.
                                        Otherwise the opcode will allocate a buffer to return the mask and
                                        then the opcode will freea the buffer during REQ_TERM */
        BYTE PICHUGE *Reserved6;    /* Must be NULL! */
        BYTE PICHUGE *Reserved7;    /* Must be NULL! */
        BYTE PICHUGE *Reserved8;    /* Must be NULL! */
                /* NOTE: if any of the following fields are not applicable, */
                /* they should be set to 0 */
        PICFLAGS     PicFlags;      /* PF_EOIFound, PF_IsGray, PF_IncludeBMPHead */
        PICFLAGS     PicFlags2;     /* PF2_ flags above when implemented */
        DWORD        AllocType;
                /* 0 if input is to be processed in chunks, 1 if input will be */
                /* available in its entirety (avoids possible need for seeking) */
        DWORD        AuxSize;
                /* current allocated size of AuxSpace; set by the application */
        DWORD        AuxUsed;
                /* how many bytes of AuxSpace are used up.  Should be set to 0 by */
                /* the application before the opcode is "called". */
        DWORD        AuxNeeded;
                /* needed additional space; set by opcode before returning with */
                /* RES_AUX_NEEDED.  If the application continues (REQ_CONT) and */
                /* AuxSize is unchanged, it is assumed that the application requests */
                /* that any other auxiliary chunks be tossed. */
        DWORD        ApplyResponse;
                /* ignored unless PNG or TIFF.  0=don't apply gamma correction or */
                /* gray response to resulting DIB; 1=do */
        DWORD        ProgressiveMode;
                /* only applies if input file is progressive: */
                /* 0=create increasingly larger DIBs */
                /* 1=create same size DIBS, increasing clarity (just pixel */
                /*   replication) -- with only data from current pass */
                /* 2=return only final DIB */
                /* 3=create same size DIBS, increasing clarity (just pixel replica- */
                /*   tion) -- merging data from each pass.  Seeking of the output */
                /*   queue may be required if the output queue is not large enough */
                /*   to hold the entire image. */
                /* 4=create same size DIBS, smoothing, with only data from current pass */
                /* 5 = create same size DIBS, smoothing, merging data from each pass. */
                /*   Seeking of the output queue may be required if the output queue */
                /*   is not large enough to hold the entire image. */
        BITMAPINFOHEADER BiOut;     /* this is the bi header of the output DIB */
        DWORD        YieldEvery;    /* If PF_Yield is set, yield after putting this many scanlines in the
                                        output queue */
        DWORD        PhotoCDResolution; /* unused and should be 0, except for PhotoCD,
                                        in which case it is: 0: 64x96; 1: 128x192; 2: 256x384; 3: 512x768;
                                        4: 1024x1536; 5: 2048x3072; 6: 4096x6144 */
        LONG         Compression;   /* TIFF (0, 1-none, 2-Modified G3, 3-G31D, -3-G32D, 4-G4, 5-LZW, 6-JPEG, 32773-PackBits) */
        BYTE         TransparentColorIndex;/* if GIF && PF_IsTransparency */
        BYTE         RawG3FillOrder; /* != 1 for raw fax TIFF byte FillOrder == 2, otherwise FillOrder == 1 */
        BYTE         RawG3PhotometricInterpretation; /* != 0 for raw fax BlackIsZero, otherwise WhiteIsZero */
        BYTE         DisposalMethod;/* GIF graphics extension field */
        WORD         DelayTime;     /* GIF graphics extension field */
        BYTE         BackgroundColor;/* GIF header background color index */
        BYTE         AspectRatio;   /* GIF header aspect ratio = floor(((pixelwidth/pixelheight)+15)/64) */
        DWORD        WidthPad;      /* currently only used for GIF -- set PF_WidthPadKnown to override DIB padding */
        WORD         LogicalScreenWidth;    /* from GIF header */
        WORD         LogicalScreenHeight;   /* from GIF header */
        WORD         ImageLeftPosition;     /* from GIF image header */
        WORD         ImageTopPosition;      /* from GIF image header */
        DWORD        TIFFPhotometricInterpretation; /* if TIFF, value of PhotoMetricInterpretation Tag */
        DWORD        TIFFIFDOffset; /* if TIFFFirstByte == 'I' or 'M' then set to offset of IFD to start scanning
                                    for PIC_PARM.ImageNumber'th image or 0 to start scanning at first IFD --
                                    returned set to the offset of the IFD for the requested PIC_PARM.ImageNumber'th image */
        BYTE         TIFFFirstByte; /* 0 and read TIFF header else 'I' and 'M' and don't have to read TIFF header
                                        when TIFFIFDOffset is also set */
        BYTE         OutBpp;        /* for TGA 15, 16, 24, or 32 bit images, this can be 24 or 32 */
        BYTE         Expansion10c;  /* unused and should be 0 */
        BYTE         Expansion10d;  /* unused and should be 0 */
        DWORD        MaskBufferSize;/* bytes in MaskBuffer above for BI_ICO */
    } F2D_STRUC;

    /* set if input GIF is interlaced */
#define PF2_IsInterlacedGIF (0x00000001L)
#define PF2_GIFColorsSorted (0x00000002L)
#define PF2_SpecialTIFF     (0x00000004L)
#define PF2_GIFAllow1and4Bpp (0x00000008L)
#define PF2_PCXGetResolution (0x00000010L) /* if set, PCX resolution information, if present, in biX/YPelsPerMeter */
////shared with S2D for decompressing TIFF/JPEG compressed in RGB color space
////#define PF2_RgbJpeg     (0x80000000L)
#define PF2_UseAlphaMask    (0x40000000L) /* reserved */
#define PF2_RetrieveRGBANonPremultiplied (0x00008000L)



/***************************************************************************\
*   Converting DIBs to supported file formats (D2F) (opcode 80)             *
\***************************************************************************/
typedef struct {
        DWORD        Reserved0;
        BYTE PICHUGE *AuxSpace;
                /* If auxiliary data (e.g., Author, etc) has been retained from an */
                /* opcode 81 call, this is a pointer to the area that contains these */
                /* chunks.  The size of this buffer must be set in AuxSize below. */
        BYTE PICHUGE *Reserved2;    /* Must be NULL! */
        BYTE PICHUGE *Reserved3;    /* Must be NULL! */
        BYTE PICHUGE *Reserved4;    /* Must be NULL! */
        BYTE PICHUGE *MaskBuffer;   /* alpha channel when PF2_UseAlphaMask set */
        BYTE PICHUGE *Reserved6;    /* Must be NULL! */
        BYTE PICHUGE *Reserved7;    /* Must be NULL! */
        BYTE PICHUGE *Reserved8;    /* Must be NULL! */
                /* NOTE: if any of the following fields are not applicable, they */
                /* should be set to 0 */
        PICFLAGS     PicFlags;      /* only PF_IncludeBMPHead */
        PICFLAGS     PicFlags2;     /* PF2_ flags above when implemented */
        DWORD        AllocType;
                /* 0 if input is to be processed in chunks, 1 if input will be */
                /* available in its entirety (avoids possible need for seeking) */
        DWORD        AuxSize;
                /* allocated size of AuxSpace; set by the application */
        DWORD        ImageType;
                /* output image type requested: must be in one of these families: */
                /* BI_BMPx, BI_TGAx, BI_PCXx, BI_TIFx, BI_PNGx */
        DWORD        Compression;
                /* type of compression:
                    0 = none (BMP, TGA, TIFF)
                    1 = rle (BMP, PCX) none TIFF)
                    2 = lzw (PNG), Modified G3 (TIFF)
                    3 = G3 (TIFF) 1-D & 2-D
                    4 = G4 (TIFF),
                    6 = JPEG */
        DWORD        PNG_Progressive;
                /* PNG only: 0=sequential, 1=adam7 progressive */
        DWORD        PNG_Filter;
                /* PNG filter option: 0=none, 1=sub, 2=up, 3=average, 4=paeth, */
                /* 1000="optimal" (computed optimally for each scanline) */
        DWORD        ApplyAux;
                /* 0=discard AuxSpace data; 1=map chunks only if they map exactly to */
                /* the output file; 2=map chunks to most reasonable output chunks */
        DWORD        OutBpp;
                /* only used when creating Targa files and the input DIB size is 24; */
                /* in that case, OutBpp may be either 16 or 24 */
        DWORD        Expansion1;    /* unused and should be 0, except for TIFF
                                        (Expansion1 & 0xff000000) == 0 for Intel byte order output
                                        (Expansion1 & 0xff000000) == 1 for Motorola byte order output
                                        if the input is 32-bpp CMYK:
                                            (Expansion1 & 0xff) == 0 for CMYK output
                                            (Expansion1 & 0xff) == 1 for K output only from input CMYK
                                        if the input is 32-bpp CMYK and (Expansion1 & 0xff) == 0
                                            (Expansion1 & 0x0000ff00) is output as DotRange[0]
                                            (Expansion1 & 0x00ff0000) is output as DotRange[1] */
        DWORD        Expansion2;     /* unused and should be 0, except for TIFF
                                        (Expansion2 & 0xff) == 0 don't output X/Y resolution tags
                                                             == 1 output in inches
                                                             == 2 output in centimeters */
        DWORD        MultiImageSize; /* for TIFF, optional input image size for appending to an existing image */
        BYTE         TransparentColorIndex; /* if GIF && PF_ApplyTransparency */
        BYTE         NegateImage;   /* for TIFF, G3 or G4, output complement of image if != 0 */
        BYTE         Dimension;     /* for TIFF with Compression=3, 0=1-D, 2=1-D */
        BYTE         DisposalMethod;/* GIF graphics extension block field */
        DWORD        RowsPerStrip;  /* # rows in each strip, 0 or >= abs(biHeight) for 1 strip with all rows */
        LONG         LumFactor;     /* Luminance compression factor 0..255 if TIFF JPEG */
        LONG         ChromFactor;   /* Chrominance compression factor 0..255 if TIFF JPEG */
        SUBSAMPLING  SubSampling;   /* SS_field, 111/211/211v/411 subsampling if TIFF JPEG */
        LONG         Predictor;     /* Predictor to be used for TIFF/LZW 0, 1=no prediction, 2=TIFF Horizontal differencing */
        WORD         DelayTime;     /* GIF graphics extension block field */
        BYTE         BackgroundColor;/* GIF header background color index */
        BYTE         AspectRatio;   /* GIF header aspect ratio = floor(((pixelwidth/pixelheight)+15)/64) */
        DWORD        WidthPad;      /* currently only used for GIF -- set PF_WidthPadKnown to override DIB padding */
        WORD         LogicalScreenWidth;    /* if != 0, GIF logical screen width for GIF header */
        WORD         LogicalScreenHeight;   /* if != 0, GIF logical screen height for GIF header */
        WORD         ImageLeftPosition;     /* from GIF image header */
        WORD         ImageTopPosition;      /* from GIF image header */
        DWORD        YieldEvery;    /* If PF_Yield is set, yield after getting this many scanlines from the
                                        input queue */
        DWORD        IFDOffset;     /* set by the opcode to the file offset of the TIFF IFD written */
        DWORD        InsertAfterIFDOffset; /* file offset to start of IFD to insert new image after */
        BYTE         TIFFFirstByte; /* when adding an image to an existing TIFF file, set this to the first byte for more efficient operation */
    } D2F_STRUC;

#define PF2_MakeCorrectTiffJPEG (0x00000001L)
    /* else it makes a TIFF JPEG which is incorrect according to the TIFF
        spec but which Wang Imaging can read */
#define PF2_OmitGIFTrailer      (0x00000002L)
    /* set for all frames except the last frame of a multi-image GIF */
#define PF2_LocalGIFColors      (0x00000004L)
    /* if PF_MultiImage is set, then input colors are written to output as a local color table */
#define PF2_RGBAIsPremultiplied (0x00004000L)
    /* if set, RGB values in the get queue are premultiplied by alpha */
//#define PF2_UseAlphaMask        (0x40000000L) /* same meaning as F2D */

#define TIFF_INSERT_AS_FIRST (4)
    /* place in InsertAfterIFDOffset to insert the new image as the first in the file */
#define TIFF_INSERT_AS_LAST (0)
    /* place in InsertAfterIFDOffset to append the new image */

/*.P*/
/***************************************************************************\
*   Utility functions (DIBs, colormaps, histograms) (opcode 82)             *
\***************************************************************************/
typedef struct {
        DWORD        Reserved0;
        BYTE PICHUGE *ptr1;
                /* subcode 0,3,4,5,6 -- unreferenced */
                /* subcode 1,2 -- pointer to 32k/64K WORD histogram buffer */
        BYTE PICHUGE *ptr2;
                /* subcode 3,4 -- pointer to 32K BYTE colormap buffer */
                /* subcode 0,1,2,5,6 -- unreferenced */
        BYTE PICHUGE *ptr3;         /* subcode 8 -- external RGBQUAD color table */
        BYTE PICHUGE *Reserved4;    /* Must be NULL! */
        BYTE PICHUGE *Reserved5;    /* Must be NULL! */
        BYTE PICHUGE *Reserved6;    /* Must be NULL! */
        BYTE PICHUGE *Reserved7;    /* Must be NULL! */
        BYTE PICHUGE *Reserved8;    /* Must be NULL! */
                /* NOTE: if a subcode does not use one of the following members, */
                /* it should be set to 0. */
        PICFLAGS     PicFlags;      /* PF_IncludeBMPHead, PF_Dither */
        PICFLAGS     PicFlags2;     /* PF2_ flags above when implemented */
        DWORD        AllocType;
                /* 0 if input is to be processed in chunks, 1 if input will be */
                /* available in its entirety (avoids possible need for seeking) */
        DWORD        Subcode;       /* 0,1,2,3,4,5,6 */
        DWORD        HistogramSize;
                /* subcode 0,4,5,6 -- unused */
                /* subcode 1,2 -- size of histogram buffer (Histogram) in WORDS -- */
                /* must be either 32768 or 65536 */
        BITMAPINFOHEADER BiOut;     /* bi header for output dibs */
        DWORD        RetainPalette;
                /* subcode 0 -- 1 to retain palette info, 0 to discard */
                /* subcodes 1,2,3,4,5,6 -- unused */
        DWORD        Orient;
                /* subcode 0,1,2,3,4,6 -- unused */
                /* subcode 5 -- 3 lsbs indicate: */
                /*   bit 0: reflect horizontally */
                /*   bit 1: rotate right 90 */
                /*   bit 2: rotate 180 */
                /* all other bits should be 0 rotations are performed first */
        DWORD        OutBpp;
                /* subcode 0,4,7,8: output bits per pixel */
                /* subcode 1,2,3,5,6: unused */
        DWORD        NumColors;
                /* subcode 2,3,7,8: number of colors */
                /* subcode 0,1,4,5,6: unused */
        DWORD        OutWidthPad;   /* used to pass in output width on Mac */
        DWORD        TransparentColorIndex; /* if PF_ApplyTransparency is set and <= 8 bit:
                                                subcode 1:      this index is ignored when constructing the color histogram
                                                subcode 2:      optimum palette of NumColors-1 created for opaque input
                                                                colors and the last output palette entry is reserved as
                                                                the new transparent color
                                                subcode 3:      color map won't map to last palette entry
                                                subcode 4,7,8:  input transparent pixels are mapped to last palette entry and
                                                                opaque pixels aren't mapped to last palette entry
                                            */

        DWORD        ReduceError;   /* unused and should be 0 */
        BYTE         BackgroundColorIndex; /* subcode 4: input as input palette index, output as re-mapped index
                                                in output palette */
        BYTE         DitherType;    /* 0 for default dither, 1 for faster, less accurate dithering */
        WORD         Expansion4c;   /* unused and should be 0 */
        DWORD        Expansion5;    /* unused and should be 0 */
        DWORD        Expansion6;    /* unused and should be 0 */
        DWORD        Expansion7;    /* unused and should be 0 */
        DWORD        Expansion8;    /* unused and should be 0 */
        DWORD        Expansion9;    /* unused and should be 0 */
        DWORD        Expansion10;   /* unused and should be 0 */
    } UTL_STRUC;

/* set PF2_ShowReduceError to get color error returned in ReduceError
    and set ptr1 to point to a 32K histogram for input rgb555 colors
    (color-mapped input only) */
#define PF2_ShowReduceError             (0x00000001L)
/* set PF2_ApplyReduceErrorThreshold to use the ReduceError value as
    an error threshold and abort subcode 4 with ERR_THRESHOLD_EXCEEDED
    if applicable */
#define PF2_ApplyReduceErrorThreshold   (0x00000002L)
/* set PF2_ColorMap444 to create/use a 4K (SIZE_COLORMAP444) RGB444 ColorMap instead of
    a 32K (SIZE_COLORMAP555) RGB555 ColorMap trading speed for accuracy */
#define PF2_ColorMap444                 (0x00000004L)
#define UTL_SIZE_COLORMAP444            (0x00001000L)
#define UTL_SIZE_COLORMAP555            (0x00008000L)



/*.P*/
/***************************************************************************\
*   Variables for Lossless DIBS (Operation LIMP, LIME)                      *
\***************************************************************************/
typedef struct {
        DWORD        Reserved0;
        LAYER PICHUGE *Layer;       /* points to an 8-element array of layers */
        BYTE PICHUGE *AppField;     /* points to user info of AppFieldLen bytes */
        BYTE PICHUGE *OverText;     /* NULL or points to overlaying text */
        RGBQUAD PICHUGE *UniversalCT;  /* Universal Color Table, replaces CT in image */
        BYTE PICHUGE *Reserved5;    /* Must be NULL! */
        BYTE PICHUGE *Reserved6;    /* Must be NULL! */
        BYTE PICHUGE *Reserved7;    /* Must be NULL! */
        BYTE PICHUGE *Reserved8;    /* Must be NULL! */
        PICFLAGS     PicFlags;      /* See the PF_ flags above */
        PICFLAGS     PicFlags2;     /* PF2_ flags above when implemented */
        LONG         AppFieldSize;  /* Allocated size of AppField in bytes. */
        LONG         AppFieldLen;   /* 0 or length of data within AppField */
        LONG         OverTextSize;  /* Allocated size of OverText in bytes. */
        LONG         OverTextLen;   /* 0 or length of data within OverText */
        LONG         StripSize;     /* Minimum size of buffer to hold a strip - after */
                                    /* INIT it is the size of the largest strip, upon */
                                    /* RES_PUT_NEED_SPACE it is the size for the */
                                    /* current layer */
        SHORT        NumUC;         /* Number of Universal Colors in UniversalCT */
        SBYTE        MinLayerWanted;/* When expanding, smallest layer needed */
        SBYTE        CurrentLayer;  /* Current layer being packed/expanded */
        BYTE         RestartLo;     /* Low range of restart intervals to be expanded */
        BYTE         RestartHi;     /* High range of restart intervals to be expanded */
        /* the next 15 items (ProgHeight..SymbolSize) should remain in the same order */
        BYTE         ProgHeight;    /* Progression height */
        BYTE         NumRestarts;   /* Number of restart intervals (for level 0) */
        DWORD        RestartOff[4]; /* Offset of restart interval from layer 0 */
        PICXSHORT    NumOfPages;    /* Number of pages/images in document/file */
        PICXSHORT    PageNum;       /* Page/image number of current page/image */
        SHORT        Transparent;   /* Transparent color index, -1 means none */
        SHORT        UserDelay;     /* User delay (seconds/100) before image replacement */
        PICXSHORT    XOff;          /* Image X-offset relative to logical screen */
        PICXSHORT    YOff;          /* Image Y-offset relative to logical screen */
        BYTE         DispMethod;    /* Disposition method */
        BYTE         ErrLimit;      /* Last ErrLimit bits can be in error */
        BYTE         CompMethod;    /* Lossless compression 'L', or 'P', 0 => automatic */
        BYTE         CompOrder;     /* Order of method if method if 'P' (1..4), 0=>3 */
        /* The remaining should be invisible to the app. */
        LONG         TableSize;     /* Hash table size */
        LONG         SymbolSize;    /* Size in bits of Symbol */
        DWORD        Symbol;        /* Symbol to be encoded/decoded */
        DWORD        HashedContext; /* Context for Symbol to be encoded/decoded */
        DWORD        HashStep;      /* Aid to help Els-coder traverse hash table */
                                    /* must be < TableSize */
        DWORD        TotalWritten;  /* Accumulative count of bytes written so far */
        WORD         internal_buffer;   /* for internal use by Els coder only */
        SHORT        jot_count;                     /* " */
        BYTE PICHUGE *context_table;                /* " */
        BYTE PICHUGE *backup_table;                 /* " */
        BYTE PICHUGE *table_end;                    /* " */
        DWORD        minimum;                       /* " */
        LONG         backlog;                       /* " */
    } LOSSLESS;


/*.P*/
/***************************************************************************\
*   Variables for Lossless DIBS (Operation LIP3, LIE3), have ParmVerMin>=2  *
\***************************************************************************/
typedef struct {
        DWORD        Reserved0;
        LAYER PICHUGE *Layer;       /* points to an 8-element array of layers */
        BYTE PICHUGE *AHT;          /* points to optional Abbreviated Huffman Table */
        BYTE PICHUGE *Reserved3;    /* Must be NULL! */
        RGBQUAD PICHUGE *UniversalCT;  /* Universal Color Table, replaces CT in image */
        BYTE PICHUGE *Reserved5;    /* Must be NULL! */
        BYTE PICHUGE *Reserved6;    /* Must be NULL! */
        BYTE PICHUGE *Reserved7;    /* Must be NULL! */
        BYTE PICHUGE *Reserved8;    /* Must be NULL! */
        PICFLAGS     PicFlags;      /* See the PF_ flags above */
        PICFLAGS     PicFlags2;     /* See the PF2_ flags above */
        REGION       Region;        /* Source or Destination region (replaces Head) */
        LONG         StripSize;     /* Minimum size of buffer to hold a strip - after */
                                    /* INIT it is the size of the largest strip, upon */
                                    /* RES_PUT_NEED_SPACE it is the size for the */
                                    /* current layer */
        SHORT        NumUC;         /* superseded by NumUC2 below (must be 0) */
        SBYTE        MinLayerWanted;/* When expanding, smallest layer needed */
        SBYTE        CurrentLayer;  /* Current layer being packed/expanded */
        BYTE         RestartLo;     /* Low range of restart intervals to be expanded */
        BYTE         RestartHi;     /* High range of restart intervals to be expanded */
        /* the next 13 items (ProgHeight..SymbolSize) should remain in the same order */
        BYTE         ProgHeight;    /* Progression height */
        BYTE         NumRestarts;   /* Number of restart intervals (for level 0) */
        DWORD        RestartOff[4]; /* Offset of restart interval from layer 0 */
        PICXSHORT    NumOfPages;    /* Number of pages/images in document/file */
        PICXSHORT    PageNum;       /* Page/image number of current page/image */
        SHORT        Transparent;   /* Transparent color index, -1 means none */
        SHORT        UserDelay;     /* User delay (seconds/100) before image replacement */
        PICXSHORT    XOff;          /* Image X-offset relative to logical screen */
        PICXSHORT    YOff;          /* Image Y-offset relative to logical screen */
        BYTE         DispMethod;    /* Disposition method */
        BYTE         AllowedBitErr; /* Last AllowedBitErr bits can be in error */
        BYTE         CompMethod;    /* Lossless compression (see #define METHOD_xxx below) */
        /* Configuration parameters for PPMD or JPEG method: */
        BYTE         CompOrder;     /* Order of method PPMD=>0..4, JPEG=>1..7 */
        BYTE         PTuning;       /* Index to set of tuned parameters for PPMD method */
        SBYTE        Channel;       /* Color channel to be treated as gray (-1 if composite) */
        PIXEL_TYPE   IOPixType;     /* Convert from/to this pixel type , 0 = default */
        BYTE         NativeBpp;     /* Bpp prior to any conversion from/to IOPixType */
        DWORD        RestartCount;  /* Number of pixels between restart markers */
        DWORD        SetBits;       /* (internal) After UnpkLine this has pixel's used bits set */
        DWORD PICHUGE *PixelMap;     /* (internal) Maps 2^EffectiveBpp values to 2^RegionBpp values, */
                                    /* includes UniversalCT corrections and any byte swap */
        BYTE         EffectiveBpp;  /* (internal) NativeBpp - AllowedBitErr after INIT */
        BYTE         PrecisionUCT;  /* (internal) 0 <= Each element in UniversalCT < 2^PrecisionUCT */
        BYTE         Reserved9[2];
        DWORD        AHTLen;        /* Byte length of AHT buffer (if any) */
        DWORD        AppsToOmit;
        DWORD        NumUC2;        /* Number of Universal Colors in UniversalCT (if NumUC above is 0,
                                        replaces NumUC so up to a 65536-element UniversalCT can be specified)
                                        UniversalCT is ignored if NumUC2 and NumUC are less than 2 */
        LONG         ImageWidth;     /* True width before cropping */
        LONG         ImageHeight;    /* True height before cropping */
    } LOSSLESS3;

#define PF2_FoundDicom              (0x00000001L)
        /* (LL JPEG) file was found to be DICOM, may have multiple images */
#define PF2_NonInterleaved          (0x00000002L)
        /* (LL JPEG) compressed code is non-interleaved */
#define PF2_FastDecompress          (0x00000004L)
        /* (LL JPEG) high-speed AVI decompression used */
#define PF2_ReachedEOI              (0x00000008L)
        /* (LL JPEG) 0xff/0xd9 encountered in image */
#define PF2_AHTCodeChanged          (0x00000010L)
        /* (LL JPEG pack) AHT specified a single 15-bit Huffman code and
            zero, one or two 16-bit Huffman codes.  The code(s) were changed
            to avoid having to generate a 0xffff code to code a prefix
            not coded by the AHT. */
#define PF2_AllowOutOfRangeBits     (0x40000000L)
        /*  This is for improperly compressed ljpeg files which produce pixels with bits beyond those
            advertised in the SOF marker as the sample precision.  The default behavior is to mask
            them off; this flag overrides that. */

#define METHOD_AUTO 0
#define METHOD_LOCO 'J'
#define METHOD_PPMD 'P'
#define METHOD_JPEG 'S'


/***************************************************************************\
*   Variables for Digital Camera Raw Formats (Operation CAMERARAWE)                      *
\***************************************************************************/
typedef struct {
        DWORD        Reserved0;
        BYTE PICHUGE *Reserved1;    /* Must be NULL! */
        BYTE PICHUGE *Reserved2;    /* Must be NULL! */
        BYTE PICHUGE *Reserved3;    /* Must be NULL! */
        BYTE PICHUGE *Reserved4;    /* Must be NULL! */
        BYTE PICHUGE *Reserved5;    /* Must be NULL! */
        BYTE PICHUGE *Reserved6;    /* Must be NULL! */
        BYTE PICHUGE *Reserved7;    /* Must be NULL! */
        BYTE PICHUGE *Reserved8;    /* Must be NULL! */
        PICFLAGS     PicFlags;      /* See the PF_ flags above */
        PICFLAGS     PicFlags2;     /* PF2_ flags above when implemented */
        REGION2      Region;
        LONG         WhiteBalanceMethod;
        double       RedFactor;
        double       BlueFactor;
        double       BrightnessFactor;
        double       Gamma;
        BOOL         DoNotClipColors;
        BOOL         DoNotConvertTosRGB;
        LONG         InterpolationMethod;
        BOOL         QuarterSizeImage;
        BOOL         FourColorRGB;
        BOOL         DoNotStretchColors;
        BOOL         DoNotRemoveRotation;
        BOOL         UseSecondaryPixels;
        PICSIZET     FileLength;
        DWORD        ThumbnailType;
        DWORD        ThumbnailLength;
        BYTE PICHUGE *ThumbnailBuffer;
    } CAMERARAWE;



/***************************************************************************\
*   Variables for Portable Network Graphics, have ParmVerMin>=2             *
\***************************************************************************/

/* Flags for the PNG filter to say which filters to use.  The flags
 * are chosen so that they don't conflict with real filter types below.
 * These values should NOT be changed.
 */
#define F_PNG_NO_FILTERS     0x00
#define F_PNG_FILTER_NONE    0x08
#define F_PNG_FILTER_SUB     0x10
#define F_PNG_FILTER_UP      0x20
#define F_PNG_FILTER_AVG     0x40
#define F_PNG_FILTER_PAETH   0x80
#define F_PNG_ALL_FILTERS (PNG_FILTER_NONE | PNG_FILTER_SUB | PNG_FILTER_UP | \
                         PNG_FILTER_AVG | PNG_FILTER_PAETH)

/* F_PNG_ALL_FILTERS is the typical default
 * and will be used if PF_OptimalFilter is set.
 */

/* Filter values
 * These defines should NOT be mixed with filter flags. Either use
 * the flags or 1 of the following filter values.
 * These values should NOT be changed.
 */
#define V_PNG_FILTER_VALUE_NONE  0
#define V_PNG_FILTER_VALUE_SUB   1
#define V_PNG_FILTER_VALUE_UP    2
#define V_PNG_FILTER_VALUE_AVG   3
#define V_PNG_FILTER_VALUE_PAETH 4
#define V_PNG_FILTER_VALUE_LAST  5


#define rgbAlpha     rgbReserved    /* replaces 4th component of RGBQUAD array */
typedef struct {
        DWORD        Reserved0;
        BYTE PICHUGE *Reserved1;           /* Must be NULL! */
        BYTE PICHUGE *Reserved2;           /* Must be NULL! */
        BYTE PICHUGE *Reserved3;           /* Must be NULL! */
        BYTE PICHUGE *Reserved4;           /* Must be NULL! */
        BYTE PICHUGE *Reserved5;           /* Must be NULL! */
        BYTE PICHUGE *Reserved6;           /* Must be NULL! */
        BYTE PICHUGE *Reserved7;           /* Must be NULL! */
        BYTE PICHUGE *Reserved8;           /* Must be NULL! */
        PICFLAGS     PicFlags;             /* See the PF_ flags above */
        PICFLAGS     PicFlags2;            /* PF2_ flags above when implemented */
        REGION       Region;               /* Source or Destination region (replaces Head) */
        LONG         StripSize;            /* Minimum size of buffer to hold a strip - after */
                                           /* INIT it is the size of the largest strip */
        DWORD        FilterSampleSize;     /* Byte size for filter offset */
        DWORD        AlphaBpp;             /* Bits per pixel, as indicated by the input file */
        DWORD        AlphaStride;          /* Reserved for opcode use */
        DWORD        BytesRem;             /* Reserved for opcode use */
        BYTE         Background[6];        /* Reserved for opcode use */
        BYTE         Trans[6];             /* Reserved for opcode use */
        BYTE         LastTrans;            /* Reserved for opcode use */
        BYTE         Action;               /* Reserved for opcode use */
        BYTE         Filter;               /* valid if PF_OptimalFilter is not set. Use filter 0..4 but values */
        BYTE         BitDepth;             /* number of bits per component */
        DWORD        MinReadData;          /* Reserved for opcode use */
        WORD         Palette_Entries;      /* Reserved for opcode use */
        WORD         ResolutionUnit;       /* 0 = none (X/Y specify aspect ratio if != 0)
                                               else if X or Y are 0 then biX/YPelsPerMeter
                                               in PIC_PARM.Head are used */
        DWORD        XResolution;          /* horizontal pixels/ResolutionUnit */
        DWORD        YResolution;          /* vertical pixels/ResolutionUnit */
        RGBQUAD16    BackgroundColorRGB;   /* the background color of the PNG file */
        RGBQUAD16    TransparentColorRGB;  /* the transparent color of the PNG file */
        LONG         BackgroundColorIndex; /* the background palette index of the PNG file */
        BYTE         Interlace;            /* 0 for no interlacing, 1 for "Adam7" interlacing */
    } PNG_UNION;
// grandfather misspelling just in case
#define Palette_Enteries Palette_Entries

#define PF2_InvertAlpha             0x00000001L /* reserved */
#define PF2_IndexedAlpha            0x00000002L /* reserved */

#define PF2_RetrieveAlpha           0x00001000L
#define PF2_SaveAlpha               0x00001000L
/* PNG Read - place alpha channel into the PUT queue */
#define PF2_HaveAlpha               0x00002000L
/* PNG Read - set if the input file has alpha information */



/***************************************************************************\
*   Variables for Zooming (Operation ZOOM)                                  *
\***************************************************************************/
typedef struct {
        DWORD        Reserved0;
        BYTE PICHUGE *Reserved1;    /* Must be NULL! */
        BYTE PICHUGE *Reserved2;    /* Must be NULL! */
        BYTE PICHUGE *Reserved3;    /* Must be NULL! */
        BYTE PICHUGE *Reserved4;    /* Must be NULL! */
        BYTE PICHUGE *Reserved5;    /* Must be NULL! */
        BYTE PICHUGE *Reserved6;    /* Must be NULL! */
        BYTE PICHUGE *Reserved7;    /* Must be NULL! */
        BYTE PICHUGE *Reserved8;    /* Must be NULL! */
        PICFLAGS     PicFlags;      /* See the PF_ flags above */
        PICFLAGS     PicFlags2;     /* See the PF_ flags above */
        LONG         NewWidth;      /* width of new image */
        LONG         NewHeight;     /* height of new image */
        WORD         Mode;          /* if 1, then scaling factor is NewWidth/NewHeight */
        WORD         NewBitCount;   /* bitcount for new image */
        void PICHUGE *WorkArea;     /* For internal use */
        BITMAPINFOHEADER BiOut;     /* Header for output image */
        DWORD        Reserved;      /* Should be zero */
        REGION       RegionOut;
    } ZOOM_PARMS;



/***************************************************************************\
*   Variables for wavelet expand/compress (Operations W2D, D2W)             *
\***************************************************************************/
typedef struct {
        WORD          QuantizationThreshold;    /* 0 .. 32767 */
        CHAR          XTransformFilter[9];      /* ASCIIZ file name */
        CHAR          YTransformFilter[9];      /* ASCIIZ file name */
        BYTE          MinTransformLevel;        /* 0 .. 10   */
        BYTE          MaxTransformLevel;        /* 0 .. 10   */
        CHAR          Qthr[6];                  /* QLUTLIB ASCIIZ Quantization table name */
        } WAVECOMPONENT;

typedef struct {
        DWORD        Reserved0;
        /* Information prior to REQ_INIT */
        BYTE PICHUGE* QlutlibPath;
        BYTE PICHUGE* SliceFactors; /* reserved */
        BYTE PICHUGE* UpdatedTiles; /* if <> 0, points to a bitmap of updated tiles in order
                                        ordinarily this would be set by OP_RIDP */
        BYTE PICHUGE* Reserved4;
        BYTE PICHUGE* Reserved5;    /* Must be NULL! */
        BYTE PICHUGE* Reserved6;    /* Must be NULL! */
        BYTE PICHUGE* Reserved7;    /* Must be NULL! */
        BYTE PICHUGE* Reserved8;    /* Must be NULL! */
        PICFLAGS     PicFlags;
        PICFLAGS     PicFlags2;     /* PF2_ flags above when implemented */
        LONG         Status;
        /* file names below specify files with extension .ASC in the same
            directory (QlutlibPath) as QLUTLIB.ASC */
        WAVECOMPONENT Y;    /* also gray scale */
        WAVECOMPONENT I;
        WAVECOMPONENT Q;
        REGION       Region;
        /* ChunkSize .. ChunkHeight must appear in this order and adjacent
            to each other so op91.c/op90.c can define a pic_parm offset to them */
        BYTE         ChunkSize;     /* if ChunkType != 3 tile size in megabytes (0 for no tiles),
                                        else ignored if ChunkType == 3 */
        BYTE         ChunkType;     /* 0 horizontal strips, 1 vertical strips, 2 rectangles,
                                        3 to use ChunkWidth/ChunkHeight and ignore
                                        ChunkSize */
        WORD         ChunkWidth;    /* tile width for ChunkType == 3, but at least 4 */
        WORD         ChunkHeight;   /* tile height for ChunkType == 3, but at least 4 */
        BYTE         TolerancePercentOutputSize; /* if PF2_AutoQuantizationThreshold is set and MaxOutputSize is not 0
                                        and not Chunks and TolerancePercentOutputSize is not 0,
                                        then a Quantization Threshold is selected such that the output
                                        +/-TolerancePercentOutputSize percent (default 5%).  Then if the output size
                                        would be larger than MaxOutputSize, the output is limited to MaxOutputSize */
        BYTE         NumSlices;     /* reserved */
        DWORD        KeyFrameRate;  /* key frame at least once every KeyFrameRate frames */
        DWORD        MaxOutputSize; /* != 0 to limit output size if compression settings would otherwise
                                        result in a larger output */
        BYTE         ChunkProgSlices; /* if >= 2, and there are chunks, then this specifies the rearrangement
                                        to be done for progressive display -- the first 1/ChunkProgSlices'th of each
                                        chunk is output, then the next 1/ChunkProgSlices'th of each chunk is output,
                                        and so on,
                                        if >= 1, and there are chunks, and MaxOutputSize is not 0, then each chunk's
                                        size is limited according to the ratio of each chunk's unlimited size to the
                                        total unlimited size for all chunks
                                        otherwise if < 1 and MaxOutputSize is not 0, then each chunk's size is limited
                                        according to the ratio of the chunk's pixels to the total pixels for all chunks */
        BYTE         Padding1;
        WORD         ClusterHeaderVersion;
        THUMBNAIL    Thumbnail;     /* expand to thumbnail size if != THUMB_NONE */
        THUMBNAIL    Resolution;    /* level of detail to include in decompressed image */
        } WAVELET;

#define PF2_IsKeyFrame      (0x00000001L) /* returned if packed/expanded frame were a key frame */
#define PF2_ForceKeyFrame   (0x00000002L) /* set to force packed frame to be a key frame */
#define PF2_Hurry           (0x00000004L) /* set to force wavelet delta encoding to hurry up */
#define PF2_AutoQuantizationThreshold (0x8L) /* set to automatically determine a quantization threshold to
                                                achieve MaxOutputSize to a tolerance of ToleranceOutputSize bytes */
#define PF2_FrameByFrameQthr    (0x10L)     /* set to adjust qthr frame by frame */
#define PF2_P2AndP3OptDisable   (0x20L)     /* disable P2 and P3 optimizations */
#define PF2_P3OptDisable        (0x40L)     /* disable P3 optimizations */
#define PF2_MMXDisable          (0x100L)    /* disable MMX, P2 and P3 optimizations */


/***************************************************************************\
*   Variables for Wavelet Scalar Quantization (Operations WSQP, WSQE)       *
\***************************************************************************/
typedef struct {
        DWORD        Reserved0;
        BYTE PICHUGE* Reserved1;
        BYTE PICHUGE* Reserved2;
        BYTE PICHUGE* Reserved3;
        BYTE PICHUGE* Reserved4;
        BYTE PICHUGE* Reserved5;    /* Must be NULL! */
        BYTE PICHUGE* Reserved6;    /* Must be NULL! */
        BYTE PICHUGE* Reserved7;    /* Must be NULL! */
        BYTE PICHUGE* Reserved8;    /* Must be NULL! */
        PICFLAGS     PicFlags;      /* PF_Yield is used */
        PICFLAGS     PicFlags2;     /* PF2_ flags above when implemented */
        LONG         Status;
        REGION       Region;
        LONG         StripSize;     /* Minimum size of buffer to hold a strip */
        LONG         Black;         /* Scanner black calibration value. */
        LONG         White;         /* Scanner white calibration value. */
        double       Quant;         /* Desired total bit rate per pixel. */
    } WSQ_UNION;



/***************************************************************************\
*   Variables for AT&T DjVu(tm) (Operations D2DJVU, DJVU2D)                 *
\***************************************************************************/
#define DJVUTYPE_DOCUMENT       (0)
#define DJVUTYPE_BW             (1)
#define DJVUTYPE_PHOTO_COLOR    (2)
#define DJVUTYPE_PHOTO_GRAY     (3)

#define DJVULAYER_FULL          (0)
#define DJVULAYER_MASK          (1)
#define DJVULAYER_FOREGROUND    (2)
#define DJVULAYER_BACKGROUND    (3)

#define DJVUFILTER_COLOR        (1)
#define DJVUFILTER_SIZE         (2)
#define DJVUFILTER_INVERSION    (4)
#define DJVUFILTER_ALL          (DJVUFILTER_COLOR | DJVUFILTER_SIZE | DJVUFILTER_INVERSION)

#define DJVUQUALITY_LOSSY       (0)
#define DJVUQUALITY_LOSSLESS    (1)
#define DJVUQUALITY_AGGRESSIVE  (2)

#define DJVUSUBSAMPLING_LESSDETAIL  (1)
#define DJVUSUBSAMPLING_MOREDETAIL  (0)
#define DJVUSUBSAMPLING_FULLDETAIL  (2)

typedef struct {
        DWORD        Reserved0;
        BYTE PICHUGE* StatusText;   /* expand and pack: you set a pointer to a buffer for error text (ERR_DJVU_*) and
                                        status text (RES_PUT_DATA_YIELD) returned by DjVu */
        BYTE PICHUGE* Reserved2;
        BYTE PICHUGE* Reserved3;
        BYTE PICHUGE* Reserved4;
        BYTE PICHUGE* Reserved5;    /* Must be NULL! */
        BYTE PICHUGE* Reserved6;    /* Must be NULL! */
        BYTE PICHUGE* Reserved7;    /* Must be NULL! */
        BYTE PICHUGE* Reserved8;    /* Must be NULL! */
        PICFLAGS      PicFlags;     /* expand: PF_YieldGet and/or PF_YieldPut
                                       pack:   PF_YieldPut */
        PICFLAGS      PicFlags2;    /* PF2_ flags (none) */
        DWORD         StatusSize;   /* expand and pack: you set the size of the StatusText buffer */
        DWORD         WidthPad;     /* expand and pack: returned width of one line */
        DWORD         StripSize;    /* expand: returned minimum size for Put queue,
                                       pack:   returned minimum size for Get queue */
        WORD          Version;      /* expand only: returned DjVu encoder version */
        WORD          ImageDpi;     /* expand: returned image dpi
                                       pack:   you set to the overall image dpi (not needed for DJVUTYPE_PHOTO_*)
                                               (25..6000, default according to rounded Head.biXPelsPerMeter
                                                or 200 if Head.biXPelsPerMeter is 0) */
        WORD          GammaX10;     /* expand: returned image target gamma
                                       pack:   you set to target gamma (multiplied by 10) to be stored with DjVu image
                                               (4..49, default 22 for gammas 0.4..4.9, default 2.2) */
        BYTE          ImageType;    /* expand: returned DJVUTYPE_* above
                                       pack:   you set to DJVUTYPE_BW if input is bilevel (biBitCount == 1)
                                               otherwise if 24-bit,
                                                   DJVUTYPE_PHOTO_COLOR,
                                                   DJVUTYPE_PHOTO_GRAY converts to gray
                                                   DJVUTYPE_DOCUMENT and DJVUTYPE_BW separate the image into:
                                                        BW mask
                                                        foreground colors (colors under BW mask)
                                                        background image (remainder of image)
                                                   then DJVUTYPE_BW discards foreground colors and background image
                                               otherwise if 8-bit with uniform grayscale palette
                                                   DJVUTYPE_PHOTO_COLOR (invalid),
                                                   DJVUTYPE_PHOTO_GRAY
                                                   DJVUTYPE_DOCUMENT and DJVUTYPE_BW separate the image into:
                                                        BW mask
                                                        foreground gray levels (gray levels under BW mask)
                                                        background image (remainder of image)
                                                   then DJVUTYPE_BW discards foreground gray levels and background image
                                               otherwise don't do that (convert it first to one of the above formats) */
        BYTE          RenderLayer;  /* expand only: you set to DJVULAYER_* above
                                       DJVULAYER_FULL is ok for all ImageTypes
                                       DJVULAYER_MASK is ok for DJVUTYPE_BW or
                                           DJVUTYPE_DOCUMENT
                                       other values are ok for DJVUTYPE_DOCUMENT only
                                       ignored if not ok (same as full) */

        /* additional pack only parameters you set are: */

        /* the next two only matter for DJVUTYPE_BW and DJVU_DOCUMENT */
        BYTE          TextQuality;  /* DJVUQUALITY_LOSSLESS, DJVUQUALITY_LOSSY (default), DJVUQUALITY_AGGRESSIVE */
        BYTE          TextDontThicken; /* (default 0) if 0, the normal character thickening algorithm is used which is
                                        designed to increase the readability of text.  != 0 may improve the
                                        readability of small text (small fonts or low-resolution image) */

        /* the next two matter for DJVUTYPE_PHOTO_COLOR */
        BYTE          PhotoSubsampling; /* control the amount of CbCr detail retained
                                           DJVUSUBSAMPLING_LESSDETAIL  (less color detail)
                                           DJVUSUBSAMPLING_MOREDETAIL  (more color detail)
                                           DJVUSUBSAMPLING_FULLDETAIL  (full color detail)
                                           (default is DJVUSUBSAMPLING_MOREDETAIL) */
        /* this one also matters for DJVUTYPE_PHOTO_GRAY */
        BYTE          PhotoQuality;      /* image compression quality setting (20..100 default is 75) */

        /* the remaining fields only matter for DJVUTYPE_DOCUMENT (TextQuality and TextDontThicken above)
            If you need to be compatible with prior DjVu versions, ImageDpi must be 100, 150 or 300 */
        WORD          DocTextColorDpi; /* (25..6000, default 25) 25-50 recommended */
        WORD          DocBackgroundDpi; /* (25.6000, default 100) 100 recommended
                                        100 if you need to be compatible with prior DjVu decoders */
        BYTE          DocBackgroundQuality; /* background compression quality setting (20..100 default is 75) */
        BYTE          Padding; /* padding to dword boundary */

        /* the remaining fields only matter for DJVUTYPE_DOCUMENT
           **** these fields will ordinarily be left at their default values ****
           they are parameters to DjVu's mask creation during which text-like information is separated
           from the other image content

           If you _really_ need to change these, I suggest you look at the help file which comes with AT&T's
           DjVuShop application or look at AT&T's DjVu SDK documentation
        */
        LONG          MaskFilterLevel;  /* (-200..200, default 0)
                                            SDK says "connected components with a score less than this ... are kept" */
        BYTE          MaskMultiForeBack;/* (0..255, default 0)
                                            if != 0, used as a luminance threshold to divide into two ranges
                                            a mask is determined for each, then the two masks are "stitched"
                                            together into one mask */
        BYTE          MaskDisableFilters;/* turn off one or more of the following mask-separation filters:
                                            DJVUFILTER_COLOR
                                            DJVUFILTER_SIZE
                                            DJVUFILTER_INVERSION
                                            DJVUFILTER_ALL (invalid if MaskFilterLevel != 0) */
    } DJVU_UNION;



/***************************************************************************\
*   Variables for TIFF Edit  (OP_TIFEDIT)             *
\***************************************************************************/

typedef struct {
            DWORD        Reserved0;
            /*  Buffer information */
            BYTE PICHUGE    *Reserved1;    /* Must be NULL */
            BYTE PICHUGE    *Reserved2;    /* Must be NULL */
            BYTE PICHUGE    *Reserved3;    /* Must be NULL */
            BYTE PICHUGE    *Reserved4;    /* Must be NULL */
            BYTE PICHUGE    *Reserved5;    /* Must be NULL */
            BYTE PICHUGE    *Reserved6;    /* Must be NULL */
            BYTE PICHUGE    *Reserved7;    /* Must be NULL */
            BYTE PICHUGE    *Reserved8;    /* Must be NULL */
            PICFLAGS        PicFlags;
            PICFLAGS        PicFlags2;
            LONG            TiffReq_Op;     /* TIFF Command Operation */
            PICXWORD        Get_Page;       /* Requested IFD page in Get Queue */
            PICXWORD        Put_Page;       /* Requested IFD page in Put Queue */
            DWORD           ImageSize;      /* Hopefully set by user and/or opcode */
} TIFF_EDIT;

#define TF_DELETE_PAGE      0x001
#define TF_INSERT_PAGE      0x002
#define TF_REPLACE_PAGE     0x004
#define TF_EXTRACT_PAGE     0x008
#define TF_LOCATE_PAGE      0x010
#define TF_COMPACT_TIFF     0x020
#define TF_EDIT_TAGS        0x100



/***************************************************************************\
*   Variables for MO:DCA/CALS Expand                                        *
\***************************************************************************/

#define PF2_HasThumbnail (0x00000001L) /* returned if thumbnail preview present in C4 input */

typedef struct {
        DWORD         Reserved0;
        CHAR PICHUGE* ExternalOverlayName; /* expand: the overlay requested, ASCIIZ */
        BYTE PICHUGE* ExternalOverlay;     /* expand: the overlay supplied */
        CHAR PICHUGE* DocumentName;        /* pack: Used as the FQN in the Begin Document SFI */
        BYTE PICHUGE* Reserved4;
        BYTE PICHUGE* Reserved5;    /* Must be NULL! */
        BYTE PICHUGE* Reserved6;    /* Must be NULL! */
        BYTE PICHUGE* Reserved7;    /* Must be NULL! */
        BYTE PICHUGE* Reserved8;    /* Must be NULL! */
        PICFLAGS      PicFlags;     /* expand: PF_YieldPut
                                       expand BI_C4: set PF_ExpandThumbnail to expand the thumbnail preview
                                       pack:   PF_YieldPut */
        PICFLAGS      PicFlags2;    /* PF2_ flags (see above) */
        DWORD         WidthPad;     /* expand and pack: returned width of one line */
        DWORD         StripSize;    /* expand: returned minimum size for Put queue,
                                       pack:   returned minimum size for Get queue */
        DWORD         ImageType;    /* pack: BI_CALS or BI_MODCA */
        DWORD         NegateImage;  /* pack: != 0 to compress negative of input image */
        // following returned by BI_MODCA expand according to the selected page's
        // Page Descriptor structured field. Some fields, as noted, returned by BI_C4
        WORD          PageXRes;     // BI_MODCA expand: x units per 10 inches/cm; BI_C4 expand: pixels per inch
        WORD          PageYRes;     // BI_MODCA expand: y units per 10 inches/cm; BI_C4 expand: pixels per inch
        DWORD         PageWidth;    // BI_MODCA expand: page width in x units
        DWORD         PageHeight;   // BI_MODCA expand: page height in y units
        WORD          PageResUnits; // BI_MODCA expand: 0 -- 10 inches, else 10 centimeters
        // following set by you before REQ_INIT and/or before REQ_EXEC according
        // to the output you desire.  Set either OutXRes plus OutYRes or set OutWidth plus OutHeight.
        // The default is the page width and height in inches times a resolution such that
        // the first ioca object doesn't have to be enlarged or shrunk.  If there is no ioca
        // object, then the default resolution is 200 pixels per inch.
        WORD          OutResUnits;  // BI_MODCA expand: 0 -- 10 inches, else 10 centimeters (default 10 inches)
        WORD          OutXRes;      // BI_MODCA expand: pixels per 10 inches/cm
        WORD          OutYRes;      // BI_MODCA expand: pixels per 10 inches/cm
        DWORD         OutWidth;     // BI_MODCA expand: page width in pixels
        DWORD         OutHeight;    // BI_MODCA expand: page height in pixels
        DWORD         OutBpp;       // BI_MODCA expand: output bit depth (default 1 else 8)
        DWORD         ExternalOverlayLength; // BI_MODCA expand: the length in bytes of ExternalOverlay
        // following returned from REQ_INIT for BI_C4 expand only.  Thumbnail dimensions, if present.
        DWORD         ThumbWidth;  // BI_C4 expand: width of embedded preview image
        DWORD         ThumbHeight; // BI_C4 expand: height of embedded preview image
    } MODCA_UNION;



/***************************************************************************\
*   Variables for cleanup, such as dust removal (OP_CLEAN)
\***************************************************************************/

typedef struct {
    LONG          RectX;        /* x coordinate of upper left corner of rect. */
    LONG          RectY;        /* y coordinate of upper left corner of rect. */
    LONG          RectWidth;
    LONG          RectHeight;
    LONG          Confidence;
    } REDEYE_RECTANGLE;   /* for subcode 18 */

typedef struct {
        DWORD         Reserved0;
        BYTE PICHUGE* YMap;         /* reserved */
        BYTE PICHUGE* CbCrMap;      /* reserved */
        BYTE PICHUGE* PixelMask;    /* for Subcode 0, PixelMask is a 1 bit per pixel mask for pixels
                                        in the rectangle specified by RectX/RectY/RectWidth/RectHeight
                                        specifying pixels that are allowed to change during scratch
                                        or speck removal.  A pixel at offset x,y within the rectangle
                                        is allowed to change as follows:
                                        int pixelofs = y * RectWidth + x;
                                        int byteofs = pixelofs / 8;
                                        int bitofs = pixelofs % 8;
                                        PixelMask[byteofs] |= 1 << bitofs; */
        BYTE PICHUGE* LookupTables; /* (for Subcode 2) to save/load tables */
        REDEYE_RECTANGLE PICHUGE* RedeyeRectangleList;    /* for subcode 18 */
        BYTE PICHUGE* Reserved6;    /* Must be NULL! */
        BYTE PICHUGE* Reserved7;    /* Must be NULL! */
        BYTE PICHUGE* Reserved8;    /* Must be NULL! */
        PICFLAGS      PicFlags;
        PICFLAGS      PicFlags2;
        LONG          Subcode;      /* 0 = speck/streak removal */
                                    /* 1 = streak removal */
                                    /* 2 = brightness/contrast/gamma enhance by histogram modification */
                                    /* 6 = change file orientation */
                                    /* 8 = deskew */
                                    /* 16 = red eye removal on JPEG or 24 bit BGR files  */
                                    /* 32 = apply sharpen, unsharp mask, or noise reduction filter */
        LONG          Stride;
        LONG          RectX;        /* x coordinate of upper left corner of rect. */
        LONG          RectY;        /* y coordinate of upper left corner of rect. */
        LONG          RectWidth;
        LONG          RectHeight;
        LONG          ThreshAdjust; /* (for subcode 0) for method 0 only, between -255 and 255 (default 0) */
        LONG          FiltAdjust;   /* (for subcode 0) for method 0 only, between -6 and 2 (default 0) */
        LONG          IterAdjust;   /* (for subcode 0) for method 0 only, between -5 and 100 (default 0) */
        LONG          Method;       /* (for subcode 0) 0 */
                                    /* (for subcode 2) one of:                                 */
                                    /*    0: Manual setting of Brightness, Contrast, Gamma     */
                                    /*    1: AutoLevels (Enhance Per Channel)                  */
                                    /*    2: AutoContrast (Enhance All Channels Together)      */
                                    /*    3: Equalize (internal use only)                      */
                                    /*    4: Read and use three tables                         */
                                    /*    5: Read and use one table                            */
                                    /* (for subcode 16, 18) 0 , 1, or 2 (how redeye is found)      */
        LONG          Darkness;     /* (for subcode 16, 18)  - degree of making eye dark */
                                    /* Last 4 bits for eye darkness 0x3 - light, 0 - normal, 1 dark */
                                    /* (Next 4 bits) Reduce Eye Glare: 0x10 -slightly reduce, 0x20 Reduce eye glare */
        LONG          Alignment;    /* (for subcode 16, 18) and JPEG , set none zero */
                                    /* may try 0 if you know JPEG is 111 SubSampled */
        LONG          Brightness;   /* (for subcode 2) -100 to +100 with 0 as default */
        LONG          Contrast;     /* (for subcode 2) -100 to +100 with 0 as default */
        double        Gamma;        /* (for subcode 2) 0.1 (bright) to 10.0 (dark) with 1.0 as default */
        LONG          Angle;        /* (for subcode 8) -450 to +450 where 1 = 0.1 degree */
        BITMAPINFOHEADER BiOut;     /* (for subcode 8) Header for output image */
        LONG          AutoClip;     /* reserved */
        LONG          AutoShift;    /* reserved */
        THUMBNAIL     Thumbnail;    /* (expand only) power of 2 reduced size */
        double        PctClippingLow;    /* (for subcode 2) real number 0.0 (none) to 100.0 (all), percent of lowest-value pixels to clip */
        double        PctClippingHigh;   /* (for subcode 2) real number 0.0 (none) to 100.0 (all), percent of highest-value pixels to clip */
        DWORD         CurveType;         /* (for subcode 2, method 0) see CT_ values below */
        DWORD         LookupTablesSize;  /* (for subcode 2) size in bytes of LookupTables */
        double        ContrastFixedPoint;/* (for subcode 2) fixed point for manual contrast adjustment */
        LONG          RedeyeRectListLength;  /* (for subcode 18)Max number of rects that can be put in list. */
        LONG          NumRedeyeRectsFound;   /* (for subcode 18) Returned by program */
        } CLEAN;

#define PF2_DarkSpeck           (0x00000001L)
#define PF2_AutoColor           (0x00000002L)   /* reserved */
#define PF2_AutoLum             (0x00000004L)   /* reserved */
#define PF2_ComputeContrastFixedPoint (0x00000008L) /* compute and use fixed point, else use default */
#define PF2_UseContrastFixedPoint     (0x00000010L) /* use fixed point either supplied or computed, else use default */
#define CT_Linear  (0x0L)  /* use linear relation (straight-line) to map original values to new values; may cause clipping */
#define CT_Arctan  (0x1L)  /* use arctan relation to map original values to new values; will not cause clipping */
                           /* for now, CT_Arctan can only be used with subcode 2, method 0 */


/***************************************************************************\
*   Variables for adjustment such as color and highlight adjustment (OP_ADJUST)
\***************************************************************************/
typedef struct {
        DWORD         Reserved0;
        BYTE PICHUGE* Reserved1;    /* reserved */
        BYTE PICHUGE* Reserved2;    /* reserved */
        BYTE PICHUGE* Reserved3;    /* reserved */
        BYTE PICHUGE* Reserved4;    /* reserved */
        BYTE PICHUGE* Reserved5;    /* reserved */
        BYTE PICHUGE* Reserved6;    /* Must be NULL! */
        BYTE PICHUGE* Reserved7;    /* Must be NULL! */
        BYTE PICHUGE* Reserved8;    /* Must be NULL! */
        PICFLAGS      PicFlags;
        PICFLAGS      PicFlags2;
        LONG          Stride;
        LONG          Method;       /* one of:                                  */
                                    /*    0: Manual Color correction method     */
                                    /*    1: Manual Lightness correction method */
                                    /*    2: Auto Color correction method       */
                                    /*    3: Auto Lightness correction method   */

        BITMAPINFOHEADER BiOut;     /*  header for output image */
        LONG          Color;        /*  color correction strength (0...100, default 0)    */
        LONG          Shadows;      /*  shadow lightening strength (0...100, default 0)   */
        LONG          Highlights;   /*  highlights darkening strength (0...100, default 0)*/
        LONG          Red;
        LONG          Green;
        LONG          Blue;          /* temp values for testing */
        } ADJUST;

/****************************************************************************************\
*   Variables for ScanFix image enhancement, such as deskew and line removal (OP_SCANFIX)
\****************************************************************************************/

typedef struct {
    double        DetectedHorizontalAngle; /* Angle needed to correct skew of horizontal lines, in degrees counter-clockwise */
    double        DetectedVerticalAngle; /* Angle needed to correct skew of vertical lines, in degrees counter-clockwise */
    LONG          DetectedVariation; /* Amount of variation from detected result, between 0 and 100 */
    } SCANFIX_SC0;

typedef struct {
    LONG          SpeckWidth; /* Maximum width of specks, in pixels */
    LONG          SpeckHeight; /* Maximum height of specks, in pixels */
    } SCANFIX_SC1;

typedef struct
{
    double MinimumAspectRatio; /* Minimum aspect ratio */
    LONG MinimumLength; /* Minimum length of lines, in pixels */
    LONG MaximumThickness; /* Maximum thickness of lines, in pixels */
    LONG MaximumGap; /* Maximum gap inside lines, in pixels */
    LONG MaximumCharacterRepairSize; /* Maximum thickness of stroke repairs, in pixels */
    LONG EdgeCleaning; /* Amount of additional edge cleaning, in pixels */
    LONG Reserved1; /* Amount of additional edge cleaning, in pixels */
    LONG Reserved2; /* Amount of additional edge cleaning, in pixels */
    LONG Reserved3; /* Amount of additional edge cleaning, in pixels */
    LONG Reserved4; /* Amount of additional edge cleaning, in pixels */
    LONG Reserved5; /* Amount of additional edge cleaning, in pixels */
    LONG Reserved6; /* Amount of additional edge cleaning, in pixels */
    LONG Reserved7; /* Amount of additional edge cleaning, in pixels */
    } SC2_LINE_PARAMS;

typedef struct {
    SC2_LINE_PARAMS HorizontalLine;
    SC2_LINE_PARAMS VerticalLine;
    } SCANFIX_SC2;

typedef struct {
    double        HorizontalRotationAngle; /* Angle of rotation for horizontal lines, in degrees counter-clockwise between -360 and +360 */
    double        VerticalRotationAngle; /* Angle of rotation for vertical lines, in degrees counter-clockwise between -360 and +360 */
    BOOL          MaintainOriginalSize; /* TRUE if the output image should be identical in size to the input image. */
    } SCANFIX_SC3;

typedef struct {
    LONG          AmountOfEffectLeft; /* Amount of effect on left edge of objects, in pixels */
    LONG          AmountOfEffectRight; /* Amount of effect on right edge of objects, in pixels */
    LONG          AmountOfEffectUp; /* Amount of effect on top edge of objects, in pixels */
    LONG          AmountOfEffectDown; /* Amount of effect on bottom edge of objects, in pixels */
    } SCANFIX_SC7;

typedef struct {
    LONG          BlankDetectMinContrast; /* Minimum contrast for data */
    LONG          BlankDetectMinObjectDimension; /* Minimum size for data */
    LONG          BlankDetectGapFill; /* Size of close operation before objectization */
    BOOL          BlankDetectIgnoreBorder; /* Ignore overscan border */
    BOOL          BlankDetectIsBlank; /* Result of blank detection */
    LONG          MarginTop; /* Margin in pixels */
    LONG          MarginLeft; /* Margin in pixels */
    LONG          MarginRight; /* Margin in pixels */
    LONG          MarginBottom; /* Margin in pixels */
    } SCANFIX_SC13;

typedef struct {
    LONG          BlankDetectMinContrast; /* Minimum contrast for data */
    LONG          BlankDetectMinObjectDimension; /* Minimum size for data */
    LONG          BlankDetectGapFill; /* Size of close operation before objectization */
    BOOL          BlankDetectIgnoreBorder; /* Ignore overscan border */
    BOOL          BlankDetectIsBlank; /* Result of blank detection */
    } SCANFIX_SC14;

typedef struct {
    LONG          MinimumPixelCount; /* Minimum black pixels in blob */
    LONG          MaximumPixelCount; /* Maximum black pixels in blob */
    LONG          MinimumDensity; /* Minimum black pixel density in blob */
    } SCANFIX_SC15;

typedef struct {
    double        HorizontalMagnification; /* Amount of horizontal magnification (1=none) */
    double        VerticalMagnification; /* Amount of vertical magnification (1=none) */
    } SCANFIX_SC16;

typedef struct {
    LONG          MinimumAreaWidth; /* Minimum width of an area containing dot shading, in pixels */
    LONG          MinimumAreaHeight; /* Minimum height of an area containing dot shading, in pixels */
    LONG          VerticalSizeAdjustment; /*  */
    LONG          HorizontalSizeAdjustment; /*  */
    LONG          DensityAdjustment; /*  */
    LONG          MaximumDotSize; /*  */
    LONG          MaximumVerificationHeight; /*  */
    } SCANFIX_SC17;

typedef struct {
    LONG          MinimumAreaWidth; /* Minimum width of an area containing inverse text, in pixels */
    LONG          MinimumAreaHeight; /* Minimum height of an area containing inverse text, in pixels */
    LONG          MinimumBlackOnEdges; /* , in pixels */
    LONG          Density; /* , in pixels */
    } SCANFIX_SC18;

typedef struct {
    LONG          Amount; /* , in pixels */
    } SCANFIX_SC19;

typedef struct {
    LONG          ImageIsNormal; /* FALSE if the whole image was determined to be inverse (white on black), TRUE otherwise. */
    } SCANFIX_SC20;

typedef struct {
    LONG          BorderSpeckSize; /* Maximum width or height (in pixels) of specks within the border */
    LONG          PageSpeckSize; /* Maximum width or height (in pixels) of specks within the page */
    LONG          MinimumPageWidth; /* Minimum expected page width */
    LONG          MaximumPageWidth; /* Maximum expected page width */
    LONG          MinimumPageHeight; /* Minimum expected page height */
    LONG          MaximumPageHeight; /* Maximum expected page height */
    double        DetectedHorizontalAngle; /* Angle of rotation for horizontal lines, in degrees counter-clockwise between -360 and +360 */
    double        DetectedVerticalAngle; /* Angle of rotation for vertical lines, in degrees counter-clockwise between -360 and +360 */
    LONG          PageX; /* X coordinate of the top-left corner of the detected page */
    LONG          PageY; /* Y coordinate of the top-left corner of the detected page */
    LONG          PageWidth; /* Width, in pixels, of the detected page */
    LONG          PageHeight; /* Height, in pixels, of the detected page */
    LONG          AmountToExpandPage; /* Number of pixels to expand each each edge (or reduce if this value is negative) */
    BOOL          DeskewBorder; /* TRUE if the image should be rotated so the detected border is not skewed */
    BOOL          ReplaceBorder; /* TRUE if the detected border pixels should be replaced with pixels of PadColor */
    BOOL          RemoveBorder; /* TRUE if the detected border pixels should be removed from the output image */
    } SCANFIX_SC21;

typedef struct {
    LONG          MinimumCombLength; /* the minimum distance (in pixels) from the leftmost comb mark to the rightmost comb mark */
    LONG          CombHeight; /* The typical height (in pixels) of a comb mark (measured from the top of the horizontal base line to the top of the mark) */
    LONG          CombSpacing; /* The typical distance (in pixels) between comb marks */
    LONG          HorizontalLineThickness; /* The typical thickness (in pixels) of the horizontal lines that comprise the comb */
    LONG          VerticalLineThickness; /* The typical thickness (in pixels) of the vertical lines that comprise the comb */
    LONG          MinimumConfidence; /* the confidence of an individual comb before it will be removed */
    } SCANFIX_SC23;

typedef struct
{
    LONG FeedbackLinesRemoved;
    LONG ResultantMargin; /* Desired left/top margin */
    LONG SkipDistance;
    LONG MinBackground;
    LONG MinForeground;
    LONG MinActivity;
    LONG ToLineMinLineLength;
    LONG ToLineMaxLineGap;
    LONG ToLineMaxLineThickness;
    BOOL Active;
    BOOL CentralFocus;
    BOOL AddOnly;
    BOOL IgnoreHoles;
    BOOL SkipActive;
    BOOL ToLineActive;
    BOOL ToLineCheckThinLinesActive;
    LONG Reserved1;
} SC24_REGISTER_PARAMS;

typedef struct {
    SC24_REGISTER_PARAMS HorizontalRegister;
    SC24_REGISTER_PARAMS VerticalRegister;
    } SCANFIX_SC24;

typedef struct {
        DWORD         Reserved0;
        BYTE PICHUGE* Reserved1;
        BYTE PICHUGE* Reserved2;
        BYTE PICHUGE* Reserved3;
        BYTE PICHUGE* Reserved4;
        BYTE PICHUGE* Reserved5;    /* Must be NULL! */
        BYTE PICHUGE* Reserved6;    /* Must be NULL! */
        BYTE PICHUGE* Reserved7;    /* Must be NULL! */
        BYTE PICHUGE* Reserved8;    /* Must be NULL! */
        PICFLAGS      PicFlags;
        PICFLAGS      PicFlags2;
        LONG          Subcode;      /* 0 = detect skew */
                                    /* 1 = despeckle */
                                    /* 2 = line removal */
                                    /* 3 = rotation */
                                    /* 4 = mirror */
                                    /* 5 = flip */
                                    /* 6 = negate */
                                    /* 7 = dilate */
                                    /* 8 = erode */
                                    /* 9 = diagonal dilate */
                                    /* 10 = diagonal erode */
                                    /* 11 = get rectangle */
                                    /* 12 = smooth zoom */
                                    /* 13 = blank page detect */
                                    /* 14 = blank rectangle detect */
                                    /* 15 = blob removal */
                                    /* 16 = scale */
                                    /* 17 = remove dot shading */
                                    /* 18 = correct inverse text */
                                    /* 19 = smooth objects */
                                    /* 20 = detect negative page */
                                    /* 21 = remove border */
                                    /* 23 = remove combs */
                                    /* 24 = registration (margin adjustment) */
        LONG          Stride; /* Input stride */
        BITMAPINFOHEADER BiOut; /* Output Image Width, Height, etc. */
        RGBQUAD       PadColor; /* Color to be used for pixels beyond the bounds of the source image (subcodes in: 3, 11) */
        LONG          OutputStride; /* Output stride (subcodes out: all) */
        LONG          Confidence; /* Confidence that the detection process produced accurate results, between 0 and 100 (subcodes out: 0, 20) */
        LONG          CountOfFeaturesFound; /* Number of objects detected (subcodes out: 1, 2, 17, 18) */
        LONG          DetectionQuality; /* Desired quality of the detection process, between 0 and 100 (subcodes in: 0, 20) */
        LONG          RectX; /* x coordinate of upper left corner of rect. */
        LONG          RectY; /* y coordinate of upper left corner of rect. */
        LONG          RectWidth; /* width of the rect. */
        LONG          RectHeight; /* height of the rect. */
        LONG          ReservedA;
        LONG          ReservedB;
        LONG          ReservedC;
        LONG          ReservedD;
        LONG          ReservedE;
        LONG          ReservedF;
        LONG          ReservedG;
        LONG          ReservedH;

        /*  Subcode-specific information */
        union {
            SCANFIX_SC0  SC0;
            SCANFIX_SC1  SC1;
            SCANFIX_SC2  SC2;
            SCANFIX_SC3  SC3;
            SCANFIX_SC7  SC7;
            SCANFIX_SC7  SC8;
            SCANFIX_SC7  SC9;
            SCANFIX_SC7  SC10;
            SCANFIX_SC13 SC13;
            SCANFIX_SC14 SC14;
            SCANFIX_SC15 SC15;
            SCANFIX_SC16 SC16;
            SCANFIX_SC17 SC17;
            SCANFIX_SC18 SC18;
            SCANFIX_SC19 SC19;
            SCANFIX_SC20 SC20;
            SCANFIX_SC21 SC21;
            SCANFIX_SC23 SC23;
            SCANFIX_SC24 SC24;
            } u;
        } SCANFIX;

#define PF2_ActOnRectangle          (0x00000001L)
        /* limit operation of a subcode to the rectangle specified in
           RectX/Y/Width/Height (subcodes 15, 23) */

/* The following constants define values that can be placed into SCANFIX.Subcode */
#define SF_SUBCODE_DETECT_SKEW             (0)
#define SF_SUBCODE_DESPECKLE               (1)
#define SF_SUBCODE_REMOVE_LINES            (2)
#define SF_SUBCODE_ROTATE                  (3)
#define SF_SUBCODE_MIRROR                  (4)
#define SF_SUBCODE_FLIP                    (5)
#define SF_SUBCODE_NEGATE                  (6)
#define SF_SUBCODE_DILATE                  (7)
#define SF_SUBCODE_ERODE                   (8)
#define SF_SUBCODE_DIAGONAL_DILATE         (9)
#define SF_SUBCODE_DIAGONAL_ERODE         (10)
#define SF_SUBCODE_GET_RECTANGLE          (11)
#define SF_SUBCODE_SMOOTH_ZOOM            (12)
#define SF_SUBCODE_DETECT_BLANK_PAGE      (13)
#define SF_SUBCODE_DETECT_BLANK_RECTANGLE (14)
#define SF_SUBCODE_REMOVE_BLOBS           (15)
#define SF_SUBCODE_SCALE                  (16)
#define SF_SUBCODE_REMOVE_DOT_SHADING     (17)
#define SF_SUBCODE_CORRECT_INVERSE_TEXT   (18)
#define SF_SUBCODE_SMOOTH_OBJECTS         (19)
#define SF_SUBCODE_DETECT_NEGATIVE_PAGE   (20)
#define SF_SUBCODE_REMOVE_BORDER          (21)
#define SF_SUBCODE_REMOVE_COMBS           (23)
#define SF_SUBCODE_REGISTER               (24)

/***************************************************************************\
*   Variables for Image Delivery Protocol (OP_RIDP, OP_TIDP)
\***************************************************************************/

typedef struct {
        DWORD         Priority;     /* 1 is lowest, 0 is the same as 1,
                                        wavelet max is ULONG_MAX, J2K max is 255 */
        DWORD         XOfs;         /* enclosing rectangle horizontal pixel offset */
        DWORD         YOfs;         /* enclosing rectangle vertical pixel offset */
        DWORD         Width;        /* enclosing rectangle width in pixels */
        DWORD         Height;       /* enclosing rectangle height in pixels */
        DWORD         XBox;         /* horizontal pixels per bitmap box, 0 is the same as 1 */
        DWORD         YBox;         /* vertical pixels per bitmap box, 0 is the same as 1 */
        BYTE*         BoxBitmap;    /* ROI boxes within enclosing rectangle */
        } IDPREGION;

typedef struct {
        DWORD         Reserved0;
        IDPREGION PICHUGE* RegionsOfInterest;
        BYTE PICHUGE* RcvrState;
        BYTE PICHUGE* UpdatedTiles; /* bitmap of changed tiles for OP_W2D */
        BYTE PICHUGE* Reserved4;    /* Must be NULL! */
        BYTE PICHUGE* Reserved5;    /* Must be NULL! */
        BYTE PICHUGE* Reserved6;    /* Must be NULL! */
        BYTE PICHUGE* Reserved7;    /* Must be NULL! */
        BYTE PICHUGE* Reserved8;    /* Must be NULL! */
        PICFLAGS      PicFlags;
        PICFLAGS      PicFlags2;    /* PF2_ flags when implemented */
        LONG          NumRegions;   /* # regions in RegionsOfInterest */
        LONG          RcvrStateSize;/* total size of RcvrState buffer */
        LONG          RcvrStateLen; /* data length in RcvrState buffer */
        THUMBNAIL     OutputResolution; /* don't bother with output at finer resolution than this */
        DWORD         OutputSize;   /* Data length to be output,
                                        0 = unlimited output according to
                                        parameters */

        LONG          XmitAppData;  /* set bit 0 to transmit comments
                                       set bit 1 to transmit raw data packets
                                       set bit 2 to transmit color table
                                       set bit 3 to transmit JPEG 2000 .JP2 boxes */
        DWORD         StateStamp;   /* output by OP_RIDP to be input to OP_TIDP as a verification
                                        that their states are synchronized */
        DWORD         NumTiles;     /* returned by OP_RIDP after REQ_INIT so DeltaTileBitmap's
                                        size is known so it can be allocated */
        DWORD         OutputLayer;  /* (J2K only) don't bother with output at a more detailed
                                        bitslice layer than this (0 = all layers, 1 = most detailed
                                        layer only, 2 = two most detailed layers only, etc.) */
        } IDP_UNION;

// shared with lossless JPEG
//#define PF2_ReachedEOI              0x00000008L   // server has nothing more to deliver

/* bit flags for XmitAppData field above */
#define IDPXMIT_Comment         (1)   /* else Wavelet Comment packets removed (n/a for JP2) */
#define IDPXMIT_RawData         (2)   /* else Wavelet RawData packets and JP2 UUID/XML boxes removed */
#define IDPXMIT_ColorPalette    (4)   /* else Wavelet ColorPalette packets removed (n/a for JP2) */
#define IDPXMIT_JP2Boxes        (8)   /* else all JP2 boxes are stripped */


/***************************************************************************\
*   Variables for JPEG 2000 operations (Operations J2KP, J2KE)              *
\***************************************************************************/

#define WT_9_7  0                   /* irreversible wavelet transform (lossy) */
#define WT_5_3  1                   /* reversible wavelet transform (lossless) */

#define TCT_Default     0           /* apply YCbCr <-> RGB transform to first 3 components,
                                        component transform depends on wavelet transform */
#define TCT_None        1           /* do not apply component transform */
#define TCT_YCbCr       2           /* apply YCbCr <-> RGB transform to first 3 components */

#define PO_LRCP         1           /* Layer-Resolution level-Component-Position progression */
#define PO_RLCP         2           /* Resolution level-Layer-Component-Position progression */
#define PO_RPCL         3           /* Resolution level-Position-Component-Layer progression */
#define PO_PCRL         4           /* Position-Component-Resolution level-Layer progression */
#define PO_CPRL         5           /* Component-Position-Resolution level-Layer progression */

#define JPEG2000FileFormat_JPX  3  // Part-2 JPX file format
#define JPEG2000Profile_Part2_Full 4  // requires a full Part-2 decoder
#define JPEG2000Profile_Part2_JPXBaseline 5  // requires a Part-2 JPX Baseline decoder

typedef struct {
        DWORD    ComponentNumber;     /* numbered 0..M as implied by partition ordering */
        DWORD    WaveletTransform;    /* WT_9_7 (default, irreversible) or WT_5_3 (reversible) */
        DWORD    CompressionProfile;  /* Reserved: Best quality = 0 <= CompressionProfile <= 10 = best compression */
                                      /* ignored if CompFileSize (below) is not 0. */
                                      /* (it's currently just mapped to Rate presets, so it's not very useful yet */
        DWORD    DecompositionLevels; /* (0 = default) actual wavelet decomposition levels (up to 65535) */
        DWORD    SkippedLevels;       /* Reserved: # highest-res levels skipped (Total = Skipped+Decomposition) */
        WORD     PrecinctWidth;       /* must be a power of 2 from 2 to 32768 -- (default 0 is 32768,
                                          128 is optimum for images for IDP) */
        WORD     PrecinctHeight;      /* must be a power of 2 from 2 to 32768 -- (default 0 is 32768,
                                          128 is optimum for images for IDP) */
        DWORD    Flags;               /* see TCF_* flags following */
        DWORD    Reserved[6];
    } TILECOMP;
#define TCF_Bypass              (0x1L) /* use faster bypass mode for encoding */

typedef struct {
        DWORD    TileNumber;        /* numbered 0..N in left-right/top-down order */
        DWORD    TileCompTransform; /* TCT_Default, TCT_None, TCT_YCbCr */
        TILECOMP DftTileComp;       /* these settings apply to all this tile's components not in OtherTileComps array */
        DWORD    NumOtherTileComps;
        TILECOMP PICHUGE* OtherTileComps;
        DWORD    NumLayers;         /* 0 = default used (which is 1) bit slice layers */
        DWORD    ProgressionOrder;  /* 0 = default used (which is PO_LRCP) */
        DWORD    Flags;             /* see TF_* flags following */
        double*  LayerRates;        /* to target an explicit byte size for each bitslice layer.  If TF_LayerRateTargetPSNR is set
                                        in Flags then this instead targets an explicit PSNR through each layer.  The array has
                                        NumLayers (see above) elements unless an earlier element is 0.0 and then that layer's
                                        size and following layers' sizes are selected by the opcode.  When PF2_ForceLossless is
                                        specified, the final layer will always have all remaining compressed data regardless of
                                        this setting.  If there are more values here than there are layers in the compressed image,
                                        any extra values are ignored. */
        DWORD    SliceDecompositionLevels; /* (0 = default) wavelet decomposition levels for 3rd (slice) dimension */
        DWORD    Reserved[5];
    } TILE;
#define TF_TilePartResolution   (0x1L)  /* tile parts consist of packets from one resolution level only */
#define TF_TilePartLayer        (0x2L)  /* tile parts consist of packets from one quality layer only */
#define TF_TilePartComponent    (0x4L)  /* tile parts consist of packets from one component only */
#define TF_InsertPLTMarker      (0x8L)  /* include Packet-Length-Tile-Header Marker facilitating random-access */
#define TF_LayerRatesTargetPSNR (0x10L) /* target PSNR instead of byte size for layer rate see LayerRates above */

typedef struct {
        QUEUE    Queue;
        DWORD    StripSize;         /* Minimum size of buffer to hold a strip */
        REGION   Region;
#if defined(__SPARC__) && defined(PIC64)
        WORD     Pad[3];            /* An array of partitions is used.  Force each to start on 8-byte boundary so Queue pointers
                                        remain 8-byte aligned */
#endif
    } PARTITION;

typedef struct {
        DWORD         Reserved0;
        BYTE PICHUGE* UpdatedTiles; /* if <> 0, points to a bitmap of updated 64x64 pixel boxes in raster order
                                        ordinarily this would be set by OP_RIDP */
        BYTE PICHUGE* Reserved2;    /* Must be NULL! */
        BYTE PICHUGE* Reserved3;    /* Must be NULL! */
        BYTE PICHUGE* Reserved4;    /* Must be NULL! */
        BYTE PICHUGE* Reserved5;    /* Must be NULL! */
        BYTE PICHUGE* Reserved6;    /* Must be NULL! */
        BYTE PICHUGE* Reserved7;    /* Must be NULL! */
        BYTE PICHUGE* Reserved8;    /* Must be NULL! */
        PICFLAGS    PicFlags;
        PICFLAGS    PicFlags2;      /* PF2_SkipJP2Header, PF2_ForceLossless, ... */

        /* following in reference grid coordinates */
        DWORD       ImageWidth;     /* Xsiz - XOsiz */
        DWORD       ImageHeight;    /* Ysiz - YOsiz */
        DWORD       ImageXOff;      /* XOsiz */
        DWORD       ImageYOff;      /* YOsiz */

        DWORD       StripSize;      /* Minimum size of buffer to hold a strip */
        REGION      Region;
        DWORD       NumOtherPartitions;
        PARTITION PICHUGE* OtherPartitions;
        DWORD       PartitionNum;   /* Used by Defer to determine which partition */

        /* following in reference grid coordinates */
        DWORD       TileWidth;      /* XTsiz, 0 is the same as TileWidth = ImageWidth */
        DWORD       TileHeight;     /* YTsiz, 0 is the same as TileHeight = ImageHeight */
        DWORD       TileXOff;       /* XTOsiz, 0th tile origin X/Y offset from reference grid origin */
        DWORD       TileYOff;       /* YTOsiz */

        TILE        DftTile;        /* these settings apply to all components for all tiles not in OtherTiles array */
        DWORD       NumOtherTiles;
        TILE PICHUGE* OtherTiles;

        DWORD       Rate;           /* Bits per pixel * 1000, or compressed size */
        THUMBNAIL   Thumbnail;      /* (expand only) power of 2 reduced size */
        DWORD       Resolution;     /* (reserved) */
        DWORD       CompFileSize;   /* Desired compressed file size; if 0, use Compression profile instead. */
        DWORD       NumPartitions;  /* (expand only returned after REQ_INIT/RES_DONE) number of partitions needed */
        DWORD       ExpandLayers;   /* (expand only) limit expansion by discarding higher details coding-pass bitslice layers */
        double      TargetPSNR;     /* compress to desired target PSNR ignoring Rate, CompFileSize and CompressionProfile */
        DWORD       ExpandResolution; /* (expand only) limit expansion by discarding higher wavelet resolution level detail */

        DWORD       SliceOff;             /* (expand only) offset to first slice returned */
        DWORD       JPEG2000FileFormat;   /* see JPEG2000FileFormat_* constants */
        DWORD       JPEG2000Profile;      /* see JPEG2000Profile_* constants */

    } J2K_UNION;

#define PF2_3D_Slices               (0x02000000L)
        /* (JPEG 2000) set for 3D compression/decompression */
#define PF2_SkipJP2Header           (0x00000001L)
        /* (JPEG 2000) create minimal JPEG 2000 stream file, not full JP2 file */
#define PF2_ForceLossless           (0x00000002L)
        /* (JPEG 2000) Override CompressionProfile, Rate, and Transforms to ensure lossless. */
#define PF2_KeepColorTable          (0x00000004L)
        /* (JPEG 2000) For Color Mapped files only, optionally keep the table with the
            compressed image.  This option is ignored if PF2_SkipJP2Header is set. */
#define PF2_SkipUpsampling          (0x00000008L)
        /* (JPEG 2000) don't upsample the image, leave components downsampled. */
#define PF2_AlternateRateDistortion (0x00000010L)
        /* */
#define PF2_Use32BitPrecision       (0x80000000L)
        /* (JPEG 2000) use more precise internal representations/conversions */
        /*             (this really means use floating pointing instead of fixpoint) */
#define PF2_UseFloatingPoint        (0x80000000L)
        /* (JPEG 2000) use floating point instead of fixpoint */
        /*             (must have same value as PF2_Use32BitPrecision for backwards compatibility) */
#define PF2_InsertTLMMarker         (0x40000000L)
        /* (JPEG 2000) include Tile-Part-Length-Main-Header Marker facilitating random-access */
#define PF2_InsertPPMMarker         (0x04000000L)
        /* (JPEG 2000) include Packed-Packet-Headers-Main-Header Marker facilitating random-access */
        /*             (improves JPIP server stateless performance) */
#define PF2_EarlyOutRateControl     (0x20000000L)
/* If clear, compression is about the same speed regardless of the requested rate, but the requested rate
   is hit very closely for all rates for all images.
   If set, compression is faster for more compression than for less compression, but for high-gray the
   compressor may not hit the desired rate as closely for some images when the requested rate is
   from around 1 bpp up to the rate for the highest lossy quality. */

// Note: make sure additional PF2_* flags don't conflict with JPIP_UNION PF2_* flags in case we need to share at some point

#define PF2_UseResSeek              (0x400L)
#define PF2_AllowResSeek            PF2_UseResSeek
#define PF2_P4OptDisable            (0x800L)    // disable P4 optimizations
#define PF2_SSSE3OptDisable    (0x00010000L)    // disable SSSE3 optimization


/***************************************************************************\
*   Variables for JPEG 2000 transcoder (OP_J2KTRANSCODE)                    *
\***************************************************************************/

#define JPEG2000FileFormat_J2K  1  // raw codestream
#define JPEG2000FileFormat_JP2  2  // Part-1 JP2 file format

#define JPEG2000Profile_None  1
#define JPEG2000Profile_0     2
#define JPEG2000Profile_1     3
#define JPEG2000Profile_2     JPEG2000Profile_None

//#define TCF_Bypass              (0x1L) /* use faster bypass mode for encoding */
#define TCF_NoBypass            (0x2L) /* turn off bypass mode */

typedef struct {
        DWORD         Reserved0;
        BYTE PICHUGE* Reserved1;    /* Must be NULL! */
        BYTE PICHUGE* Reserved2;    /* Must be NULL! */
        BYTE PICHUGE* Reserved3;    /* Must be NULL! */
        BYTE PICHUGE* Reserved4;    /* Must be NULL! */
        BYTE PICHUGE* Reserved5;    /* Must be NULL! */
        BYTE PICHUGE* Reserved6;    /* Must be NULL! */
        BYTE PICHUGE* Reserved7;    /* Must be NULL! */
        BYTE PICHUGE* Reserved8;    /* Must be NULL! */
        PICFLAGS    PicFlags;
        PICFLAGS    PicFlags2;

        /* JPEG2000 image info, in reference canvas coordinates */
        /* the following are returned values (by REQ_INIT) */
        DWORD       ImageWidth;           /* Image width on reference canvas */
        DWORD       ImageHeight;          /* Image height on reference canvas */
        DWORD       ImageXOff;            /* Horizontal offset of top left corner of image on reference canvas */
        DWORD       ImageYOff;            /* Vertical offset of top left corner of image on reference canvas */
        DWORD       TileWidth;            /* Width of tiles on reference canvas */
        DWORD       TileHeight;           /* Height of tiles on reference canvas */
        DWORD       TileXOff;             /* Horizontal offset of top left corner of first tile on reference canvas */
        DWORD       TileYOff;             /* Vertical offset of top left corner of first tile on reference canvas */
        DWORD       NumComponents;        /* Number of components */
        DWORD       TileCompTransform;    /* Multiple component transformation - TCT_None or TCT_YCbCr, (0) means "other" */
        DWORD       DecompositionLevels;  /* Number of wavelet decomposition levels */

        /* the following are returned by REQ_INIT and then input to REQ_EXEC */
        /* leave each unchanged or set to new (nonzero) value */
        DWORD       JPEG2000FileFormat;   /* see JPEG2000FileFormat_* constants */
        DWORD       JPEG2000Profile;      /* see JPEG2000Profile_* constants */
        DWORD       TileCompFlags;        /* see TCF_* flags */
        DWORD       PrecinctWidth;        /* must be zero or a power of 2 from 2 to 32768 */
        DWORD       PrecinctHeight;       /* must be zero or a power of 2 from 2 to 32768 */
        DWORD       ProgressionOrder;     /* see PO_* constants */
        DWORD       NumLayers;            /* Note: if setting DiscardLayers nonzero, then leave this field unchanged */

        /* the following are input values (to REQ_EXEC) */
        DWORD       DiscardLayers;  /* if nonzero, limits some things */
        THUMBNAIL   Thumbnail;      /* power of 2 reduced size */
        DWORD       TileFlags;      /* see TF_* flags */
                                    /*   Note: TF_LayerRatesTargetPSNR is currently not allowed for this opcode */
        DWORD       Rate;           /* Bits per pixel * 1000 */
        DWORD       CompFileSize;   /* Desired compressed file size */
        double*     LayerRates;     /* array of values to target an explicit byte size for each bitslice layer */
                                    /* supply one value per layer to be created; can terminate list early by setting a value to 0.0 */
                                    /* last array value must be 0.0 */
    } J2KT_UNION;

// Note: make sure additional PF2_* flags don't conflict with JPIP_UNION PF2_* flags in case we need to share at some point

//#define PF_ConvertGray              0x00000100L    // if three components and TCT_YCbCr, set to drop chrominances
//#define PF2_OmitPICMetaData         (0x200L)       // must set to avoid adding Pegasus UUID
//#define PF2_InsertTLMMarker         (0x40000000L)  // must set if want this marker
//#define PF2_InsertPPMMarker         (0x04000000L)  // must set if want this marker


/***************************************************************************\
*   Variables for Smart Binarization (OP_BINARIZE)
\***************************************************************************/

/*
*/

/*
Input is 24-bit-per-pixel or 8-bit-per-pixel colormapped
BMP format.

Output is 1 bit-per-pixel BMP format. The color palette is not modified.
*/

typedef struct {
        DWORD         Reserved0;
        BYTE PICHUGE* Reserved1;    /* Must be NULL! */
        BYTE PICHUGE* Reserved2;    /* Must be NULL! */
        BYTE PICHUGE* Reserved3;    /* Must be NULL! */
        BYTE PICHUGE* Reserved4;    /* Must be NULL! */
        BYTE PICHUGE* Reserved5;    /* Must be NULL! */
        BYTE PICHUGE* Reserved6;    /* Must be NULL! */
        BYTE PICHUGE* Reserved7;    /* Must be NULL! */
        BYTE PICHUGE* Reserved8;    /* Must be NULL! */
        PICFLAGS      PicFlags;
        PICFLAGS      PicFlags2;    /* PF2_ flags above when implemented */
        LONG          Subcode;      /* 0 = quick mode) */
        LONG          LoThreshold;  /* From 0 to 255 */
        LONG          HiThreshold;  /* From LoThreshold to 255 */
        LONG          GridAngle;
        LONG          GridPitch;
        LONG          Eccentricity;
        LONG          Mode;         /* quick text / photo halftone */
        LONG          LCEFactor;
        LONG          Blur;
        } SB_PARMS;

#define BIN_MODE_QUICK_TEXT     0
#define BIN_MODE_PHOTO_HALFTONE 1
#define BIN_MODE_GRAY           2

#define BIN_BLUR_GAUSSIAN       1
#define BIN_BLUR_SMART          2


/***************************************************************************\
*   Variables for PDF creation, modification, and reading                   *
\***************************************************************************/
typedef struct {
        DWORD         Reserved0;
        BYTE PICHUGE* Reserved1;    /* Must be NULL! */
        BYTE PICHUGE* Reserved2;    /* Must be NULL! */
        BYTE PICHUGE* Reserved3;    /* Must be NULL! */
        BYTE PICHUGE* Reserved4;    /* Must be NULL! */
        BYTE PICHUGE* Reserved5;    /* Must be NULL! */
        BYTE PICHUGE* Reserved6;    /* Must be NULL! */
        BYTE PICHUGE* Reserved7;    /* Must be NULL! */
        BYTE PICHUGE* Reserved8;    /* Must be NULL! */
        PICFLAGS      PicFlags;
        PICFLAGS      PicFlags2;    /* PF2_ flags above when implemented */
        DWORD         Compression;  /* type of compression: one of possible PDFCompress values defined below */
        DWORD         WidthPad;
        DWORD         StripSize;
        DWORD         NumOfPages;
        DWORD         LumFactor;
        DWORD         ChromFactor;
        DWORD         SubSampling;
        DWORD         EncodeModeJBIG2;  /* one of the JBIG2 Encode Modes listed as part of the JBIG2 struct */
        DWORD         PageFlagsJBIG2;   /* JBIG2_PageFlags_ flags of the JBIG2 struct */
        DWORD         RegionFlagsJBIG2; /* JBIG2_RegionFlags_ flags of the JBIG2 struct */
        DWORD         Looseness;        /* JBIG2 Parameter */
        DWORD         Rate;             /* J2K Parameter */
        DWORD         CompFileSize;     /* J2K Parameter */
        double        TargetPSNR;       /* J2K Parameter */
} PDF_UNION;

#define PDFCompress_Default         0
#define PDFCompress_None            1
#define PDFCompress_G31D            2
#define PDFCompress_G32D            3
#define PDFCompress_G4              4   /* G4 must be 4 for backward compatibility issues */
#define PDFCompress_JBIG2           5
#define PDFCompress_SequentialJPEG  6
#define PDFCompress_J2K				7

#define PF2_SwapBW                  (0x80000000L) /* 1-bit images, default is Black=0 */

/***************************************************************************\
*   Variables for JPEG-LS (Operations JLSP, JLSE)                           *
\***************************************************************************/

typedef struct {
        LONG        ScanNum;        /* reserved for future enhancement */
        LONG        NumComponents;  /* reserved for future enhancement */
        LONG        NEARparam;
        LONG        ILVparam;
        LONG        Pt;
        LONG        MAXVAL;
        LONG        T1;
        LONG        T2;
        LONG        T3;
        LONG        RESET;
        LONG        RestartInterval;
        LONG        Reserved[10];
    } LSSCAN;

typedef struct {
        DWORD       Reserved0;
        BYTE PICHUGE* Reserved1;
        BYTE PICHUGE* Reserved2;
        BYTE PICHUGE* Reserved3;
        BYTE PICHUGE* Reserved4;
        BYTE PICHUGE* Reserved5;    /* Must be NULL! */
        BYTE PICHUGE* Reserved6;    /* Must be NULL! */
        BYTE PICHUGE* Reserved7;    /* Must be NULL! */
        BYTE PICHUGE* Reserved8;    /* Must be NULL! */
        PICFLAGS    PicFlags;
        PICFLAGS    PicFlags2;
        REGION      Region;
        LONG        ImageWidth;     /* True width before cropping */
        LONG        ImageHeight;    /* True height before cropping */
        LONG        StripSize;      /* Minimum size of buffer to hold a strip */
        LSSCAN      DftScan;
    } JLS_UNION;

#define JLS_NONINTERLEAVED    0
#define JLS_LINEINTERLEAVED   1
#define JLS_SAMPLEINTERLEAVED 2


/***************************************************************************\
*   Variables for JBIG2 (Operations JBIG2P, JBIG2E)                           *
\***************************************************************************/

typedef struct {
        DWORD             XOff;
        DWORD             YOff;
        DWORD             Width;
        DWORD             Height;
        DWORD             EncodeMode;
        DWORD             RegionFlags;
        void PICHUGE*     Reserved0;  /* Must be NULL! */
        DWORD             Looseness;
        LONG              Reserved[9];
    } JBIG2_REGION;

typedef struct {
        DWORD         Reserved0;
        BYTE PICHUGE* Reserved1;
        BYTE PICHUGE* Reserved2;
        BYTE PICHUGE* Reserved3;
        BYTE PICHUGE* Reserved4;
        BYTE PICHUGE* Reserved5;    /* Must be NULL! */
        BYTE PICHUGE* Reserved6;    /* Must be NULL! */
        BYTE PICHUGE* Reserved7;    /* Must be NULL! */
        BYTE PICHUGE* Reserved8;    /* Must be NULL! */
        PICFLAGS      PicFlags;
        PICFLAGS      PicFlags2;
        REGION        Region;
        DWORD         StripSize;      /* Minimum size of buffer to hold a strip */
        DWORD         ImageWidth;     /* True width before cropping */
        DWORD         ImageHeight;    /* True height before cropping */
        DWORD         FileOrg;
        DWORD         NumOfPages;
        DWORD         PageNum;
        DWORD         PageFlags;
        DWORD         MaxStripeSize;
        JBIG2_REGION  BaseJBIG2Region;
        DWORD         NumOtherJBIG2Regions;
        JBIG2_REGION PICHUGE* OtherJBIG2Regions;
        DWORD         CommentsToOmit;
        PICSIZET      JBIG2GlobalsLen;
    } JBIG2_UNION;

/* defined for PDF */
/* #define PF2_SwapBW                  (0x80000000L) */

/* suppress writing PIC informational metadata to the compressed file */
#define PF2_OmitPICMetaData                     (0x200L)

#define PF2_JBIG2Transcode_ForEmbeddedPDF       (0x001L)

/* the following is only valid with PF2_JBIG2Transcode_ForEmbeddedPDF */
#define PF2_AllowJBIG2Globals                   (0x002L)

/* CommentsToOmit in JBIG2_UNION (can be OR'd together) */
/* the following is only valid with PF2_JBIG2Transcode_ForEmbeddedPDF */
#define JBIG2_CommentsToOmit_Page0              0x01
#define JBIG2_CommentsToOmit_PageN              0x02

/* FileOrg in JBIG2_UNION */
#define JBIG2_FileOrg_Default                   0x00
#define JBIG2_FileOrg_Sequential                0x01
#define JBIG2_FileOrg_RandomAccess              0x02
#define JBIG2_FileOrg_ForEmbeddedPDF            0x03 /* reserved, for internal use only */

/* EncodeMode in JBIG2_REGION; list is likely to grow */
#define JBIG2_EncodeMode_AutoDetect             0x00
#define JBIG2_EncodeMode_Lossless_Generic_MQ    0x01
#define JBIG2_EncodeMode_Lossless_Generic_MMR   0x02
#define JBIG2_EncodeMode_Lossless_Text_MQ       0x03
#define JBIG2_EncodeMode_Lossless_Text_MMR      0x04
#define JBIG2_EncodeMode_Lossless_Text_SPM_MQ   0x05
#define JBIG2_EncodeMode_Lossless_Text_SPM_MMR  0x06
#define JBIG2_EncodeMode_Lossy_Text_MQ          0x07
#define JBIG2_EncodeMode_Lossy_Text_MMR         0x08
#define JBIG2_EncodeMode_Lossy_Halftone_MQ      0x09
#define JBIG2_EncodeMode_Lossy_Halftone_MMR     0x0A

#define JBIG2_PageFlags_IsLossless              (0x00000001L) /* JBIG2 file says page is lossless */
#define JBIG2_PageFlags_IsStriped               (0x00000002L) /* JBIG2 file says page is striped */

#define JBIG2_RegionFlags_InvertedRegion        (0x00000001L) /* invert bits prior to symbol analysis in encoder */


/***************************************************************************\
*   Variables for JPIP (Operations JPIPSERVER, JPIPCLIENT)                  *
\***************************************************************************/

typedef struct {
        DWORD         Reserved0;
        CHAR PICHUGE* JPIPRequestString;    /* (client) output by JPIP_Action_Create_Request
                                               (server) may be input to REQ_INIT and/or JPIP_Action_Input_Request
                                                        as received from client */
        CHAR PICHUGE* JPIPResponseString;   /* (client) may be input to JPIP_Action_Input_Response or JPIP_Action_Input_Output
                                                        as received from server
                                               (server) output by JPIP_Action_Output_Response */
        BYTE PICHUGE* EORMessageBody;       /* (client) output by JPIP_Action_Input_Response or JPIP_Action_Input_Output
                                                        if server's response EOR has a message body
                                                        and EORMessageBodyLen is the nonzero length of the message body */
        CHAR PICHUGE* Target;               /* specific Target requested by client */
        CHAR PICHUGE* TargetID;             /* a server-assigned ID for desired target; put into or taken out of a JPIP request */
        CHAR PICHUGE* GrantedTargetID;      /* a server-assigned ID for desired target; feedback to client for subsequent use as TargetID */
        CHAR PICHUGE* JPIPChannelID;        /* a server-assigned ID for desired channel; put into or taken out of a JPIP request */
        CHAR PICHUGE* JPIPChannelIDsClose;  /* ID's of channel(s) to close or "*" to close all channels */
        PICFLAGS      PicFlags;             /* not used */
        PICFLAGS      PicFlags2;            /* see PF2_ flag below */
        /* JPEG2000 image info, in reference canvas coordinates */
        DWORD         ImageWidth;           /* Image width on reference canvas */
        DWORD         ImageHeight;          /* Image height on reference canvas */
        DWORD         ImageXOff;            /* Horizontal offset of top left corner of image on reference canvas */
        DWORD         ImageYOff;            /* Vertical offset of top left corner of image on reference canvas */
        DWORD         TileWidth;            /* Width of tiles on reference canvas */
        DWORD         TileHeight;           /* Height of tiles on reference canvas */
        DWORD         TileXOff;             /* Horizontal offset of top left corner of first tile on reference canvas */
        DWORD         TileYOff;             /* Vertical offset of top left corner of first tile on reference canvas */
        WORD          NumComponents;        /* Number of components */
        /* JPIP specific fields */
        WORD          Action;               /* see JPIP_Action_* constants below */
        DWORD         EORMessageBodyLen;    /* (client) length of data in EORMessageBody; zero means none */
        DWORD         JPIPStringFlags;      /* see JSF_ bit flags below */
        DWORD         JPIPStringFlags2;     /* reserved */
        DWORD         JPIPServerStatusCode; /* (server) output by REQ_INIT and/or JPIP_Action_Input_Request to report the status
                                                        of input request processsing and can be used by server app in response to client */
        DWORD         JPIPChannelNumber;    /* (server) output by JPIP_Action_Input_Request and input to JPIP_Action_Output_Response
                                                        to identify the channel to perform the next action on */
        DWORD         JPIPServerBlocksize;  /* (server) input to JPIP_Action_Output_Response to set the maximum byte count output to the
                                                        Put queue before the opcode returns, at which time the server app is allowed to
                                                        call JPIP_Action_Input_Request, to input a newly arrived request, or
                                                        JPIP_Action_Output_Response, to continue output for this channel */
        WORD          AllowedJPIPChannelTransportsFlags;    /* see JCF_ flags below;
                                               (server) input to REQ_INIT and JPIP_Action_Input_Request to pre-authorize denying or creating
                                                        a new channel using one of these transport protocol(s) if requested by the client
                                               (server) output by REQ_INIT and JPIP_Action_Input_Request to indicate whether request is
                                                        stateless or not */
        WORD          RequestedJPIPChannelTransportsFlags;  /* see JCF_ flags below;
                                                               new channel transport protocol(s) put into or taken out of a JPIP request */
        /* JPIP fields for setting view window size, region of interest, components of interest, and maximum quality layer desired;
            (client) input to JPIP_Action_Create_Request to put into the JPIP request to the server
            (client) output by JPIP_Action_Input_Response or JPIP_Action_Input_Output from the JPIP response from the server
                     (except ViewWindowFrameRoundDirection which is never put into a JPIP response)
            (server) input to JPIP_Action_Input_Request if JPIPRequestString is NULL
            (server) output by REQ_INIT and JPIP_Action_Input_Request if JPIPRequestString is not NULL
            (server) output by JPIP_Action_Output_Response for the JPIP response to the client
                     (except ViewWindowFrameRoundDirection which is never put into a JPIP response) */
        DWORD         ViewWindowFrameWidth;
        DWORD         ViewWindowFrameHeight;
        DWORD         ViewWindowFrameRoundDirection;    /* JPIP_RoundUp, JPIP_RoundDown, JPIP_RoundClosest -- how to round
                                                ViewWindowFrameWidth and ViewWindowFrameHeight to find a matching decomposition level
                                                in the JPEG2000 image */
        DWORD         ViewWindowRegionOffsetX;
        DWORD         ViewWindowRegionOffsetY;
        DWORD         ViewWindowRegionWidth;
        DWORD         ViewWindowRegionHeight;
        WORD          ViewWindowMinComponent;
        WORD          ViewWindowMaxComponent;
        WORD          ViewWindowLayers;
        /* other JPIP fields */
        BYTE          JPIPServerResponseAlignment;  /* client-requested alignment for server responses (see progref.pdf) */
        BYTE          JPIPServerRequestWait;        /* TRUE:  client requests that the server wait to process this new request until
                                                              all other requests are processed, or
                                                       FALSE: client requests that the server preempt all other requests to process
                                                              this new request now */
        WORD          DefaultMediaType;             /* takes exactly one of the JTF_ flags below;
                                                       (server) input to REQ_INIT to pre-authorize (to allow if client requests a media type)
                                                                and to set a default (to use if client does not request a media type)
                                                       (server) output by REQ_INIT set to opcode's default if server had not provided one */
        WORD          MediaTypeFlags;               /* see JTF_ flags below -- put into or taken out of a JPIP request or JPIP response */
        DWORD         JPIPServerResponseMaxLength;  /* put into or taken out of a JPIP request or JPIP response
                                                       to indicate the maximum byte length of the response to a request */
        CHAR PICHUGE* Model;                        /* reserved for internal and future use */
        /* for the following GrantedJPIP* fields, zero means don't use or not found, nonzero means:
            (server) input to JPIP_Action_Output_Response to supply desired responses when JSF_CHANNEL_NEW returned by JPIP_Action_Input_Request
                     (the client has requested a new channel)
            (client) output by JPIP_Action_Input_Response or JPIP_Action_Input_Output from the JPIP response from the server */
        CHAR PICHUGE* GrantedJPIPChannelID;         /* (required) channel ID to use for new channel */
        CHAR PICHUGE* GrantedJPIPChannelHost;       /* (optional) name or IP of host to use for future requests if need to change */
        CHAR PICHUGE* GrantedJPIPChannelPath;       /* (optional) path component of URL to use for future requests if need to change */
        DWORD         GrantedJPIPChannelPort;       /* (optional) port to use for future requests if need to change */
        DWORD         GrantedJPIPChannelAuxport;    /* (optional) second channel port, if applicable, to use for new channel */
        WORD          GrantedJPIPChannelTransport;  /* (optional) transport protocol for new channel (see JCF_ flags below) if server had to select one */
        BYTE          EORReasonCode;                /* end-of-response reason code (see JPIP_EOR_* constants below) */
        BYTE          Reserved;                     /* for DWORD-alignment, this may be renamed in future releases so it should not be referenced */
        BYTE PICHUGE* UpdatedTiles;                 /* points to a bitmap of updated 64x64 pixel boxes in raster order */
        DWORD         UpdatedTilesSize;             /* size (in bytes) needed for UpdatedTiles */

        /* additional JPEG2000 image info */
        DWORD         JPEG2000FileFormat;           /* see JPEG2000FileFormat_* constants */
        DWORD         JPEG2000Profile;              /* see JPEG2000Profile_* constants */
        } JPIP_UNION;

#define PF2_AnotherJPIPResponseAvailable (0x10000000L)  // returned by JPIP_Action_Input_Response or JPIP_Action_Input_Output
                                                        // to signal that there's internally buffered
                                                        // data still to use, so call again to use it
#define PF2_HexHexEncode                 (0x08000000L)  // instruct JPIP_Action_Create_Request to
                                                        // %-hex-hex encode those characters in the Target string
                                                        // that could cause URI problems if left unencoded

// Note: make sure additional PF2_* flags don't conflict with J2K_UNION PF2_* flags in case we need to share at some point

// Actions
#define JPIP_Action_Create_Request        (0x01)  // (client) create and output a JPIP request string to send to the server
#define JPIP_Action_Input_Response        (0x02)  // (client) input and process the JPIP response from the server
#define JPIP_Action_Output_Bitstream      (0x03)  // (client) output the JPEG2000 image at the client
#define JPIP_Action_Input_Output          (0x04)  // (client) JPIP_Action_Input_Response plus JPIP_Action_Output_Bitstream
#define JPIP_Action_Input_Request         (0x05)  // (server) input a JPIP request received from the client
                                                  //          either as a nonzero JPIPRequestString or as individual values (JPIPRequestString == 0)
#define JPIP_Action_Output_Response       (0x06)  // (server) output the JPIP response to send to the client

// JPIP Server Status Codes for request processing status
#define JPIP_SSCode_OK                     200
#define JPIP_SSCode_BadRequest             400
#define JPIP_SSCode_NotAcceptable          406
#define JPIP_SSCode_UnsupportedMediaType   415
#define JPIP_SSCode_InternalServerError    500
#define JPIP_SSCode_NotImplemented         501
#define JPIP_SSCode_ServiceUnavailable     503

// EOR Reason Codes for response processing status or to indicate incomplete when set to JPIP_EOR_None
#define JPIP_EOR_None                      0
#define JPIP_EOR_ImageDone                 1
#define JPIP_EOR_WindowDone                2
#define JPIP_EOR_WindowChange              3
#define JPIP_EOR_ByteLimitReached          4
#define JPIP_EOR_QualityLimitReached       5
// server app determines these
#define JPIP_EOR_SessionLimitReached       6
#define JPIP_EOR_ResponseLimitReached      7
// if no other
#define JPIP_EOR_UnspecifiedReason       255

// Round Direction set in ViewWindowFrameRoundDirection controlling determination of served ViewWindowFrameWidth and ViewWindowFrameHeight
// as a function of client request and decomposition levels available in the JPEG2000 image
#define JPIP_RoundDown          (0x00L)
#define JPIP_RoundUp            (0x01L)
#define JPIP_RoundClosest       (0x02L)

// JPIP Channel (Transport Protocols) Flags
#define JCF_NONE                (0x00L)
#define JCF_DENY JCF_NONE
#define JCF_HTTP                (0x01L)
#define JCF_HTTPS               (0x02L)
#define JCF_HTTP_TCP            (0x04L)

// JPIP (Media) Type Flags (see progref.pdf)
#define JTF_NONE                (0x00L)
#define JTF_JPP                 (0x01L)
#define JTF_JPP_EXT             (0x02L)
#define JTF_JPT                 (0x04L)
#define JTF_JPT_EXT             (0x08L)
#define JTF_RAW                 (0x10L)

// JPIP String Flags signalling that corresponding field values or request elements are present
// see progref.pdf -- at server or client these are set by the app or by the opcode depending
// on the Action being performed and what is found or produced by request or response or
// what is supplied by the app
#define JSF_NONE                (0x00000000L)
#define JSF_TARGET              (0x00000001L)
#define JSF_SUB_TARGET          (0x00000002L)
#define JSF_TARGET_ID           (0x00000004L)
#define JSF_CHANNEL_ID          (0x00000008L)
#define JSF_CHANNEL_NEW         (0x00000010L)
#define JSF_CHANNEL_CLOSE       (0x00000020L)
#define JSF_REQUEST_ID          (0x00000040L)
#define JSF_FRAME_SIZE          (0x00000080L)
#define JSF_REGION_OFFSET       (0x00000100L)
#define JSF_REGION_SIZE         (0x00000200L)
#define JSF_COMPONENTS          (0x00000400L)
#define JSF_CODESTREAM          (0x00000800L)
#define JSF_CONTEXT             (0x00001000L)
#define JSF_SAMPLING_RATE       (0x00002000L)
#define JSF_ROI                 (0x00004000L)
#define JSF_LAYERS              (0x00008000L)
#define JSF_META_REQUEST        (0x00010000L)
#define JSF_MAX_LENGTH          (0x00020000L)
#define JSF_QUALITY             (0x00040000L)
#define JSF_ALIGN               (0x00080000L)
#define JSF_WAIT                (0x00100000L)
#define JSF_MEDIA_TYPE          (0x00200000L)
#define JSF_DELIVERY_RATE       (0x00400000L)
#define JSF_MODEL               (0x00800000L)   /* reserved for internal and future use */
#define JSF_TPMODEL             (0x01000000L)   /* reserved for internal and future use */
#define JSF_NEED                (0x02000000L)   /* reserved for internal and future use */
#define JSF_TPNEED              (0x04000000L)   /* reserved for internal and future use */
#define JSF_MSET                (0x08000000L)
#define JSF_UPLOAD              (0x10000000L)
#define JSF_CAPABILITY          (0x20000000L)
#define JSF_PREFERENCES         (0x40000000L)
#define JSF_CONTRAST            (0x80000000L)


/***************************************************************************\
*   Variables for CAD (Operation CAD2D)                                    *
\***************************************************************************/
/* The maximum size of string fields in the CAD structures. */
#define CAD_MAX_STRING (256)

/* 
 * Use this value in the CAD structure, LayoutToRender field in order to specify 
 * that the Model Space layout should be rendered.
 */
#define RENDER_MODEL_SPACE (0)

/* These are the allowable units values for the Units field of the CAD structure. */
#define CAD_UNITS_UNDEFINED    (0)
#define CAD_UNITS_INCHES       (1)
#define CAD_UNITS_FEET         (2)
#define CAD_UNITS_MILES        (3)
#define CAD_UNITS_MILLIMETERS  (4)
#define CAD_UNITS_CENTIMETERS  (5)
#define CAD_UNITS_METERS       (6)
#define CAD_UNITS_KILOMETERS   (7)
#define CAD_UNITS_MICROINCHES  (8)
#define CAD_UNITS_MILS         (9)
#define CAD_UNITS_YARDS        (10)
#define CAD_UNITS_ANGSTROMS    (11)
#define CAD_UNITS_NANOMETERS   (12)
#define CAD_UNITS_MICRONS      (13)
#define CAD_UNITS_DECIMETERS   (14)
#define CAD_UNITS_DEKAMETERS   (15)
#define CAD_UNITS_HECTOMETERS  (16)
#define CAD_UNITS_GIGAMETERS   (17)
#define CAD_UNITS_ASTRONOMICAL (18)
#define CAD_UNITS_LIGHTYEARS   (19)
#define CAD_UNITS_PARSECS      (20)

/* These are the allowable values for the Filetype field of the CAD stucture */
#define CAD_FILETYPE_DWG        (0) /* DWG File        */
#define CAD_FILETYPE_DXF        (1) /* Ascii DXF File  */
#define CAD_FILETYPE_DXB        (2) /* Binary DXF File */
#define CAD_FILETYPE_DWF        (3) /* DWF File        */
#define CAD_FILETYPE_UNKNOWN    (-1)

/* These are the allowable values for the FileVersion field of the CAD structure */
/* The comments show the corresponding AutoCad Application version               */
#define CAD_VERSION_0_0     (0)
#define CAD_VERSION_1_2     (1)
#define CAD_VERSION_1_40    (2)
#define CAD_VERSION_1_50    (3)
#define CAD_VERSION_2_20    (4)    /* Yes, this is correct, 2_20 comes before 2_10 */  
#define CAD_VERSION_2_10    (5)
#define CAD_VERSION_2_21    (6)
#define CAD_VERSION_2_22    (7)
#define CAD_VERSION_1001    (8)
#define CAD_VERSION_1002    (9)     /* AutoCAD 2.5         */
#define CAD_VERSION_1003    (10)    /* AutoCAD 2.6         */
#define CAD_VERSION_1004    (11)    /* Release 9           */
#define CAD_VERSION_1005    (12)
#define CAD_VERSION_1006    (13)    /* Release 10          */
#define CAD_VERSION_1007    (14)
#define CAD_VERSION_1008    (15)
#define CAD_VERSION_1009    (16)    /* R11 and R12         */
#define CAD_VERSION_1010    (17)
#define CAD_VERSION_1011    (18)
#define CAD_VERSION_1012    (19)     /* R13                */
#define CAD_VERSION_1013    (20)     /* R14 beta           */
#define CAD_VERSION_1014    (21)     /* R14 release        */
#define CAD_VERSION_1500    (22)     /* R15 (2000) beta, yes 1500 before 1015 is correct */
#define CAD_VERSION_1015    (23)     /* R15 (2000) release */
#define CAD_VERSION_1800a   (24)     /* R18 (2004) beta    */
#define CAD_VERSION_1800    (25)     /* R18 (2004) release */
#define CAD_VERSION_2100a   (26)     /* R21 (2007) beta, yes 2100 before 1021 is correct */
#define CAD_VERSION_1021    (27)     /* R21 (2007) release */
#define CAD_VERSION_UNKNOWN (32766)


typedef struct {
    CHAR          Name[CAD_MAX_STRING]; /* The null-terminated name of the layout. The model space layout is always named Model   */
                                        /*   and will always be the first entry in array of layouts.                              */                                                  
    double        Width;                /* Width of the layout in the units associated with the drawing.                          */
    double        Height;               /* Height of the layout in the units associated with the drawing.                         */
    DWORD         Reserved[8];          /* Reserved for possible future use.                                                      */
    } CAD_LAYOUT;  

typedef struct {
    CHAR          Name[CAD_MAX_STRING]; /* The name of the layer.                                               */
    CHAR          Desc[CAD_MAX_STRING]; /* The first CAD_MAX_STRING characters of the layer description.        */
    DWORD         Frozen;               /* Value of 1 indicates the layer is frozen.                            */
    DWORD         InUse;                /* Value of 1 indicates the layer is in use.                            */
    DWORD         Locked;               /* Value of 1 indicates the layer is locked.                            */
    DWORD         Plottable;            /* Value of 1 indicates the layer is plottable.                         */
    DWORD         Off;                  /* Value of 1 indicates the layer is off.  The client may set this to   */
                                        /* include or exclude the layer from the output. It is initially        */ 
                                        /* set by the opcode to the value specified in the input drawing.       */
    DWORD         Reserved[8];          /* Reserved for possible future use.                                    */
    } CAD_LAYER;  



typedef struct {
        DWORD        Reserved0;
        CAD_LAYOUT   PICHUGE *Layouts; /* Array of named layouts contained in the drawing.                             */
                                       /* The model space layout will always be the first entry in the array.          */
                                       /* Memory for the array is allocated by the opcode and freed by the opcode      */
                                       /* during REQ_TERM or before RES_ERR is returned.                               */
        CAD_LAYER    PICHUGE *Layers;  /* Array of layers contained in the drawing.                                    */
                                       /* Memory for the array is allocated by the opcode and freed by the opcode      */
                                       /* during REQ_TERM or before RES_ERR is returned.                               */
        BYTE PICHUGE *Reserved3;       /* Must be NULL! */
        BYTE PICHUGE *Reserved4;       /* Must be NULL! */
        BYTE PICHUGE *Reserved5;       /* Must be NULL! */
        BYTE PICHUGE *Reserved6;       /* Must be NULL! */
        BYTE PICHUGE *Reserved7;       /* Must be NULL! */
        BYTE PICHUGE *Reserved8;       /* Must be NULL! */
        PICFLAGS     PicFlags;         /* See the PF_ flags above.                                                       */
        PICFLAGS     PicFlags2;        /* PF2_ flags above when implemented.                                             */
        LONG         FileType;         /* Type of the input drawing file. See the #define's above for values.            */
        DWORD        FileVersion;      /* Version of the input drawing file.  See the #define's above for values.        */
        DWORD        NumLayouts;       /* Number of entries in the Layouts list.                                         */
        DWORD        NumLayers;        /* Number of entries in the Layers list.                                          */
        DWORD        StripSize;        /* Minimum size of buffer to hold a strip.                                        */
        DWORD        WidthPad;         /* Width of one output line.                                                      */
        DWORD        Units;            /* The units specified for the drawing. See the #define's above for values.       */
        DWORD        RenderWidth;      /* Set by the client to specify the width, in pixels, of the output bitmap.       */
        DWORD        RenderHeight;     /* Set by the client to specify the height, in pixels, of the output bitmap.      */
        DWORD        RenderBitDepth;   /* Set by the client to specify the output bit depth. Defaults to 8 bpp.          */
        RGBQUAD      BackgroundColor;  /* The background color of the rendered drawing, defaults to white (255,255,255). */
        DWORD        LayoutToRender;   /* Specifies the layout to render.  This is an index into the array of            */
                                       /* layouts contained in the Layouts field.  Will be set to the active             */
                                       /* layout by the opcode during REQ_INIT processing.  This field can be set to the */
                                       /* #define RENDER_MODEL_SPACE in order to render the Model Space layout.          */
    } CAD_UNION;




/***************************************************************************\
*   Variables for Microsoft HD HPhoto (Operations HDPHOTOP, HDHPOTOP)       *
\***************************************************************************/

/* These are the allowable values for the ChromaSubsampling field of the HDP_UNION stucture */
#define HDP_SS_444 (4)
#define HDP_SS_422 (3)
#define HDP_SS_420 (2)
#define HDP_SS_400 (1)

/* These are the allowable values for the Overlapping field of the HDP_UNION stucture */
#define HDP_OL_NONE   (1)
#define HDP_OL_SINGLE (2)
#define HDP_OL_DOUBLE (3)

/* These are the allowable values for the SkipSubbands field of the HDP_UNION stucture */
#define HDP_SB_ALL         (1)
#define HDP_SB_SKIPFLEX    (2)
#define HDP_SB_SKIPHIGH    (3)
#define HDP_SB_SKIPHIGHLOW (4)

/* These are the allowable values for the AlphaChannelStructure field of the HDP_UNION stucture */
#define HDP_ALPHA_NONE        (0)
#define HDP_ALPHA_PLANAR      (2)
#define HDP_ALPHA_INTERLEAVE  (3)

typedef struct {
        DWORD       Reserved0;
        DWORD PICHUGE* TileWidths;   /* Array of non-uniform tile widths, NumHorizontalTiles contains size of array */ 
        DWORD PICHUGE* TileHeights;  /* Array of non-uniform tile heights, NumVerticalTiles contains size of array */ 
        BYTE PICHUGE* Reserved3;
        BYTE PICHUGE* Reserved4;
        BYTE PICHUGE* Reserved5;    /* Must be NULL! */
        BYTE PICHUGE* Reserved6;    /* Must be NULL! */
        BYTE PICHUGE* Reserved7;    /* Must be NULL! */
        BYTE PICHUGE* Reserved8;    /* Must be NULL! */
        PICFLAGS      PicFlags;
        PICFLAGS      PicFlags2;
        REGION2       Region;
        DWORD         OrigWidth;            /* True width before cropping and thumbnailing */
        DWORD         OrigHeight;           /* True height before cropping and thumbnailing */
        double        WidthResolution;      /* pixels per inch */
        double        HeightResolution;     /* pixels per inch */
        LONG          StripSize;            /* Minimum size of buffer to hold a strip */
        LONG          LibraryError;         /* error reported by HD Photo library when ERR_LIBRARY_ERROR is returned in Status */
        THUMBNAIL     Thumbnail;            /* THUMB_NONE, .. THUMB_256, if greater than THUMB_256, then width and height are each
                                                reduced by 2^Thumbnail rounded up (i.e. 1x1 is the smallest no matter Thumbnail) */
        ORIENTATION   Reorient;             /* see ORIENTATION values above and value below */
        DWORD         PostProcessingFilter; /* 0 (none) to 4 (very strong) */
        BYTE PICHUGE* PixelFormat;          /* MS HD Photo GUID for image */
        DWORD          Quantization;          /* Quantization level for encoding, 1 = lossless */
        DWORD          ChromaSubsampling;     /* Subsampling level for encoding, see #defines above */
        DWORD          Overlapping;           /* Overlapping level for encoding, see #defines above */
        DWORD          FrequencyOrdering;     /* Encoder ordering, 0 = spatial ordering, 1 = frequency ordering */
        DWORD          TrimmedFlexbits;       /* Encoder flexbit trimming, 0 = no trimming, 15 = trim all */
        DWORD          SkipSubbands;          /* Specify which subbands for encoder to skip, see #defines above */
        DWORD          NumHorizontalTiles;    /* Number of non-uniform TileWidth entries, or number of uniform tiles */
        DWORD          NumVerticalTiles;      /* Number of non-uniform TileHeight entries, or number of uniform tiles */ 
        DWORD          AlphaQuantization;     /* Quantization level of alpha planar channel, 1 = lossless */
        DWORD          AlphaChannelStructure; /* Specify either interleaved or planar alpha channel for encoding, see #defines above */
    } HDP_UNION;

#define REORIENT_FROM_IMAGE ((ORIENTATION)-1)   /* Set u.HDP.Reorient to this value to rotate the output image
                                               according to the orientation recorded in the input image bitstream
                                               header.  Otherwise the default (0, O_normal) does no rotation,
                                               or you can set an ORIENTATION value to force a specific rotation */



/*.P*/
/***************************************************************************\
*  This structure is allocated by the user and its address passed to        *
*  the Pegasus routines and to any callback routines.                       *
*  The values which need to be specified in the PIC_PARM structure vary     *
*  depending on which operation is performed, and on whether the            *
*  REQ_INIT or REQ_EXEC request is made.  See the documentation for         *
*  specific details.                                                        *
\***************************************************************************/

typedef struct PIC_PARM_TAG {
        PICINTPTRT   Reserved0;
        /*  Elements shared by most operations */
        LONG         ParmSize;      /* Size of this structure (bytes) */
        BYTE         ParmVer;       /* Version of parameters for given Op (11..99) */
        BYTE         ParmVerMinor;  /* Minor version number of parameter structure */
        WORD         Reserver1;     /* reserved for shell/opmain.c */
        LONG         Status;        /* 0 or error code (see ERRORS.H) */
        OPERATION    Op;            /* OP_ fields, specific operation to be performed */
        BITMAPINFOHEADER Head;      /* Uncompressed Image (UI) Width, Height, etc. */
        RGBQUAD      ColorTable[272]; /* Holds primary (ó256) & secondary (ó16) palettes */
#if 0
#if defined(MACINTOSH)
        CTabHandle  MacCTable;      /* Color Table used on the Macintosh */
#endif
#endif
        LONG         PicVer;        /* PIC image version */
        ORIENTATION  VisualOrient;  /* O_ fields, displayed orientation of UI */
        LONG         CommentSize;   /* Size of comment buffer (bytes) */
        LONG         CommentLen;    /* Length of comment (0 if no comment) */
        CHAR PICHUGE *Comment;      /* Points to comment up to CommentSize bytes */
        PICINTPTRT   App;           /* any user info needed by the callbacks */
        LONG         PercentDone;   /* Strip progress monitor, 100 iff done */
#if defined(__SPARC__) && defined(PIC64)
        DWORD         Pad1; // so following is 8-aligned
#endif
        WORK_AREA PICHUGE *Reserved;/* Allocated by Pegasus routines, init NULL */
        /*  Buffer information. Can be used as circular buffers. */
        QUEUE        Get;
#if defined(__SPARC__) && defined(PIC64)
        DWORD         Pad2; // so following is 8-aligned
#endif
        QUEUE        Put;
        BYTE         KeyField[8];   /* Used to hold encode/decode key */
        /* The following function is called in the non-coroutine case by Pegasus */
        /* to ask for more data, space, etc. from the application.  DeferFn is an */
        /* application function that returns non-zero if Pegasus is to terminate */
        LONG         (PICFARFN *DeferFn)(struct PIC_PARM_TAG PICFAR *, RESPONSE);
        DWORD        Flags;         /* See F_ defines */
        DWORD        Flags2;        /* See F2_ defines when implemented */
        PICXWORD     CropWidth;     /* These next four fields have meaning only */
        PICXWORD     CropHeight;    /* if the Flags F_Crop bit is set.  The */
        PICXWORD     CropXoff;      /* pixel position are relative to the */
        PICXWORD     CropYoff;      /* image's logical upper-left corner */
        DWORD        ImageNumber;   /* Which image of a multi-image file (0 = next) */
        DWORD        PacketType;    /* Type of PIC2 packet for which space is to be reserved */
#if defined(__SPARC__) && defined(PIC64)
        DWORD         Pad3; // so following is 8-aligned
#endif
        PICSIZET     SeekInfo;      /* see components of SeekInfo below */

        CHAR PICHUGE* PIC2List;
        LONG         PIC2ListSize;
        LONG         PIC2ListLen;
        REGION       RegionIn;
        REGION       RegionOut;
        WORD         OpVersion0;    /* lsw of 64-bit opcode FileVersion */
        WORD         OpVersion1;    /* word 1 of 64-bit opcode FileVersion */
        WORD         OpVersion2;    /* word 2 of 64-bit opcode FileVersion */
        WORD         OpVersion3;    /* msw of 64-bit opcode FileVersion */
#if defined(__SPARC__) && defined(PIC64)
        DWORD         Pad4; // so following is 8-aligned
#endif
        BYTE PICHUGE* stkReserved; // reserved opmain.c
        BYTE PICHUGE* ReservedPtr2;
        BYTE PICHUGE* ReservedPtr3;
        BYTE PICHUGE* ReservedPtr4;
        BYTE PICHUGE* ReservedPtr5;
        BYTE PICHUGE* ReservedPtr6;
        BYTE PICHUGE* ReservedPtr7;
        BYTE PICHUGE* ReservedPtr8;

        CHAR PICHUGE* LoadPath;     /* Path to directory with opcode DLL */
        PICINTPTRT    LoadResInstance; /* Module instance handle of an
                                       EXE or DLL from whose resource data
                                       the opcode DLL is to be loaded */
            /* if LoadPath == 0 && LoadResInstance == 0
                    then the opcode DLL is loaded from the same directory as
                    the dispatcher DLL.  If not found there then the opcode
                    DLL is loaded using the directory order Windows uses
                    when loading DLLs
               if LoadPath != 0 && LoadResInstance == 0,
                    then the opcode DLL is loaded from the specified directory
               if LoadPath == 0 && LoadResInstance != 0
                    then the opcode DLL is loaded from the specified module's
                    resource data
               if LoadPath != 0 && LoadResInstance != 0
                    then the opcode DLL is loaded from the specified module's
                    resource data.  If not found there, then the opcode DLL
                    is loaded from the specified path. If LoadPath is "",
                    and the opcode DLL is not found in the specified module's
                    resource data, then the opcode DLL is loaded as though
                    LoadPath == 0 && LoadResInstance == 0
            */
        struct PIC_PARM_TAG PICHUGE* NestPP;    /* reserved picnclib.c */
        REQUEST       NestReq;                  /* reserved picnclib.c */
#if defined(__SPARC__) && defined(PIC64)
        DWORD         Pad5; // so following is 8-aligned
#endif
        PICINTPTRT    tlsReserved;              /* reserved picdisp.c */
        PICINTPTRT    tlsReserved2;             /* reserved picdisp.c */
        PICINTPTRT    memReserved;              /* reserved opmem.c */
        PICINTPTRT    wrapReserved;             /* reserved opmain.c */
        PICINTPTRT    wrapReserved2;            /* reserved opmain.c */
        WORD          DispVersion0;    /* lsw of 64-bit dispatcher FileVersion */
        WORD          DispVersion1;    /* word 1 of 64-bit dispatcher FileVersion */
        WORD          DispVersion2;    /* word 2 of 64-bit dispatcher FileVersion */
        WORD          DispVersion3;    /* msw of 64-bit dispatcher FileVersion */
        PICINTPTRT    DispExports;     /* reserved picnclib.c, picmacos.c, picunixos.c, etc. */
        PICINTPTRT    tlsReserved3;             /* reserved picdisp.c */
        PICINTPTRT    tlsReserved4;             /* reserved picdisp.c */
        /* if F_InputCrop or F_OutputCrop set in Flags */
        PICSIZET      IOCropXoff;       /* left edge of crop rectangle */
        PICSIZET      IOCropYoff;       /* top edge of crop rectangle */
        PICSIZET      IOCropWidth;      /* width of crop rectangle */
        PICSIZET      IOCropHeight;     /* height of crop rectangle */
        PICINTPTRT    tlsReserved5;             /* reserved picdisp.c */
        PICINTPTRT    tlsReserved6;             /* reserved picdisp.c */
        PICINTPTRT    NumberOfThreadsAllowed;   /* The max number of requested threads for a multithreaded opcode to use. */
        PICINTPTRT    NumberOfThreadsUsed;      /* The number of threads used by the opcode. */
        PICINTPTRT    SeekOffsetLow;    /* if SeekOffsetLow and SeekOffsetHigh are 0, then use SeekInfo as before, otherwise
                                            you can use SeekOffsetLow with SeekOffsetHigh for the seek offset and still use
                                            SeekInfo for the seek direction and for the queue.  On 64-bit platforms
                                            SeekOffsetHigh will always be 0.  On 32-bit platforms, SeekOffsetHigh is the
                                            upper 32-bits of a full 64-bit seek offset. */
        PICINTPTRT    SeekOffsetHigh;
        PICINTPTRT    Reserveds[88];
#if defined(__SPARC__) && defined(PIC64)
        DWORD         Pad6;      /* This forces 8-byte alignment of reserved pointer fields of all unions. */
#endif
        /* see also: Internal Use Only comments in internal.h */
        /*  Operator specific information */
        union {
            PEGQUERY    QRY;      // PegasusQuery (NOT opcode)
            DIB_INPUT   D2J;      // OP_D2J (progressive or sequential)
            DIB_OUTPUT  J2D;      // OP_JE2D, OP_S2D, OP_SE2D
            TRANS2P     S2P;      // OP_S2P
            TRANP2S     P2S;      // OP_P2S
            D2F_STRUC   D2F;      // OP_D2F
            F2D_STRUC   F2D;      // OP_F2D
            UTL_STRUC   UTL;      // OP_UTL
            LOSSLESS    LL;       // OP_LIMP, OP_LIME
            LOSSLESS3   LL3;      // OP_LIP3, OP_LIE3
            CAMERARAWE  CRE;      // OP_CAMERARAWE
            REORIENT    ROR;      // OP_ROR
            ZOOM_PARMS  ZOOM;     // OP_ZOOM
            WAVELET     WAVE;     // OP_W2D, OP_D2W
            PNG_UNION   PNG;      // OP_PNGP, OP_PNGE
            WSQ_UNION   WSQ;      // OP_WSQP, OP_WSQE
            DJVU_UNION  DJVU;     // OP_DJVU2D, OP_D2DJVU
            TIFF_EDIT   TED;      // OP_TIFEDIT
            MODCA_UNION Modca;    // OP_D2MDCA, OP_MDCA2D
            CLEAN       CLN;      // OP_CLEAN
            SCANFIX     SF;       // OP_SCANFIX
            IDP_UNION   IDP;      // OP_RIDP, OP_TIDP
            J2K_UNION   J2K;      // OP_J2KP, OP_J2KE
            J2KT_UNION  J2KT;     // OP_J2KTRANSCODE
            SB_PARMS    SB;       // OP_BINARIZE
            JLS_UNION   JLS;      // OP_JLSP, OP_JLSE
            PDF_UNION   PDF;      // OP_D2PDF, OP_PDF2D
            JBIG2_UNION JBIG2;    // OP_JBIG2P, OP_JBIG2E
            JPIP_UNION  JPIP;     // OP_JPIPCLIENT, OP_JPIPSERVER
            ADJUST      ADJ;      // OP_ADJUST
            CAD_UNION   CAD;      // OP_CAD2D
            HDP_UNION   HDP;      // OP_HDPHOTOP, OP_HDPHOTOE
            DIB_INPUT   D2S;      // ### DEPRECATED: please use the D2J member instead ###
            DIB_OUTPUT  S2D;      // ### DEPRECATED: please use the J2D member instead ###
            DIB_OUTPUT  P2D;      // ### DEPRECATED: please use the J2D member instead ###
            INTERNAL    Reserved; // Always reserved as the largest variant
        } u;
#if defined(__SPARC__) && defined(PIC64)
    } PIC_PARM __attribute__((aligned(8))); //Ensure 8-byte alignment of PIC_PARM variables
#elif defined(__SPARC__)
    } PIC_PARM __attribute__((aligned(4))); //Ensure 4-byte alignment of PIC_PARM variables
#else
    } PIC_PARM;
#endif


        /* The components of SeekInfo xx is 64 or 32 bits */
#define SEEK_OFFSET     (((PICSIZET)~0)>>2) /* bits 0..xx-3: unsigned offset in file */
#define SEEK_DIRECTION  ((~SEEK_OFFSET)<<1) /* bit xx-1: 0/1 => 0 seek beg plus offset or 1 seek end minus offset */
#define SEEK_FILE       (SEEK_DIRECTION>>1) /* bit xx-2: 0/1 => seek input/output queue */

#define SEEKINFO_BACK_FROM_END  SEEK_DIRECTION /* set if seeking to end-of-data minus the seek offset,
                                                  otherwise seeking to seek offset from the beginning of data */
#define SEEKINFO_PUT_QUEUE      SEEK_FILE      /* set if seeking in Put queue else seeking in Get queue */

        /* Flag bit fields for general image information */
#define F_Crop          (0x00000001L)  /* Set if image is to be cropped using Crop*'s fields */
#define F_Raw           (0x00000002L)  /* Set if pack source or unpack dest. is RAW */
#define F_Bmp           (0x00000004L)  /* Set if pack source or unpack dest. is BMP */
#define F_XOut          (0x00000008L)  /* reserved, for internal use only */
#define F_XIn           (0x00000010L)  /* reserved, for internal use only */
#define F_DoReqExecNext (0x00000020L)  /* used by KSAPI */
#define F_DidReqExec    (0x00000040L)  /* used by KSAPI */
#define F_AsyncAbort    (0x00000080L)  /* used by MTAPI */
#define F_UseDeferFn    (0x00000100L)  /* allows use of DeferFn when DeferFn != 0
                                        and opcode would otherwise return the
                                        response from Pegasus */
#define F_Reserved1     (0x00000200L)  /* reserved for dispatcher (picdisp.c) */
#define F_Reserved2     (0x00000400L)  /* reserved for dispatcher (picdisp.c) */
#define F_Reserved3     (0x00000800L)  /* reserved for dispatcher (picdisp.c) */


#define F_ReservedPtr5  (0x00001000L)  /* set to enable ReservedPtr5/ReservedPtr4
                                            processing else they are ignored */
#define F_RelaxedQueue  (0x00002000L)  /* set for OP_F2D(IX) to enable queue processing the same as all other opcodes
                                            (except OP_D2F/OP_UTL)-- otherwise OP_F2D queues are strict +1 sentinel
                                            queues that don't allow Q_REVERSE */

#define F_UseAlternateCoroutine (0x00004000L) /* set to use the alternate coroutine implementation, otherwise, the
                                                 legacy coroutine implementation is used.  This is ignored if
                                                 F_UseDeferFn is set */
#define F_UseStackSwitchCoroutine (0x00008000L) /* set to use the stack-switch coroutine on a platform for which
                                                 that is available but isn't the default (linux).  It is the default
                                                 on win32, win64, and solaris.  This is ignored if either F_UseDeferFn
                                                 or F_UseAlternateCoroutine are set */
#define F_InputCrop     (0x00010000L)  /* Set to crop using IOCrop*'s fields in input image coordinates */
#define F_OutputCrop    (0x00020000L)  /* Set to crop using IOCrop*'s fields in output image coordinates */


/*.P*/
/***************************************************************************\
*   void  Verify (BOOL x, PIC_PARM *p, LONG err);                           *
*                                                                           *
*   This is a macro which tests the value of x.  Iff x is false, Verify     *
*   will return control back to the application from any nested depth.      *
*   p->status is set to err, the operation is terminated, the Pegasus       *
*   routine is terminated, and the Reserved is freed.                       *
*                                                                           *
*   void  ChkDefer (BOOL x, PIC_PARM *p);                                   *
*                                                                           *
*   This is a macro which tests the value of x.  Iff x is false, it         *
*   will return control back to the application from any nested depth.      *
*   The Pegasus routine is terminated, and the Reserved is freed.           *
*                                                                           *
*   void  Defer (PIC_PARM *p, RESPONSE res);                                *
*                                                                           *
*   This is a macro which will pass control back to the application         *
*   with the result res.  It is assumed that control will resume after      *
*   the Defer statement when the application continues with the next        *
*   call to the Pegasus routine. Note that res should not equal RES_ERR.    *
\***************************************************************************/
#define Verify(x,p,err)     while (!(x)) (p)->Status = (err), \
                                longjmp((p)->Reserved->AppState, RES_ERR)

#define VerifyFALSE(p,err)  (p)->Status = (err), longjmp((p)->Reserved->AppState, RES_ERR)

/***************************************************************************\
*  Prototypes for Pegasus functions.                                        *
\***************************************************************************/
#ifdef __cplusplus
    extern "C" {
#endif

PUBLIC  BOOL DLLEXPORTWINAPI PegasusQuery(PIC_PARM PICHUGE *p);

#if !defined(MACINTOSH)
PUBLIC  BOOL DLLEXPORTWINAPI PegasusCB(PIC_PARM PICHUGE *p,
                 void (PICFARFN *GetData) (PIC_PARM PICHUGE *p),
                 void (PICFARFN *PutData) (PIC_PARM PICHUGE *p));
#endif

PUBLIC  RESPONSE DLLEXPORTWINAPI Pegasus(PIC_PARM PICHUGE *p, REQUEST req);

PUBLIC  LONG DLLEXPORTWINAPI PegasusLoad(
    OPERATION Op,
    LONG ParmVer,
    char PICFAR *Path);

PUBLIC  LONG DLLEXPORTWINAPI PegasusLoadFromRes(
    OPERATION Op,
    LONG ParmVer,
    char PICFAR *Path,
    PICINTPTRT hInstance);

PUBLIC  void DLLEXPORTWINAPI PegasusUnload(OPERATION Op, LONG ParmVer);

PUBLIC  void DLLEXPORTWINAPI PegasusTrace(const char PICFAR *pszMsg);

PUBLIC  BOOL DLLEXPORTWINAPI PegasusLibInit(PICINTPTRT hInstance);

PUBLIC  void DLLEXPORTWINAPI PegasusLibTerm(void);

PUBLIC  BOOL DLLEXPORTWINAPI PegasusLibInitNest(PICINTPTRT hInstance);

PUBLIC  void DLLEXPORTWINAPI PegasusLibTermNest(void);

PUBLIC  void DLLEXPORTWINAPI PegasusLibThreadInit(void);

PUBLIC  void DLLEXPORTWINAPI PegasusLibThreadTerm(void);

PUBLIC  void DLLEXPORTWINAPI PegasusLibThreadInitNest(void);

PUBLIC  void DLLEXPORTWINAPI PegasusLibThreadTermNest(void);

/* This include must go here, because internal.h uses some definitions from pic.h */
#if defined(PIC_INTERNAL)
#include "internal.h"
#endif

#ifdef __cplusplus
    }
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
#else /* assume Microsoft or Watcom */
    #if defined(__FLAT__) || defined(__WATCOMC__)
        #pragma pack(pop)
    #else
        /* in MS 16-bit, best we can do is to restore to the command-line state */
        #pragma pack()
    #endif
#endif

#endif  /* #if !defined(_PIC) */
