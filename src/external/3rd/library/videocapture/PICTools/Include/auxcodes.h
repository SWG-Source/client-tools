/*auxcodes.h*/
// PICTools ProductVersion 2.00.567
// Copyright (C) 2004-2008 Pegasus Imaging Corporation

/*codes for storing chunks in the Auxillary buffer*/

#if !defined(_AUXCODESTUFF)
#define _AUXCODESTUFF

/*  Version 1.1 -- 2/6/96 -- jweber*/
/*0.92 -- 1/24/96 -- jweber*/
/*  -- changed bases of mainifest groups*/
/*0.91 -- 1/23/96 -- jweber*/
/*  -- changed names of manifests*/

/*for bmp files:*/

#define AUX_BMP_EXTRABI 0x1000
    /*this identifies a chunk that has all extra, non-standard data*/
    /* past the normal 0x28 bytes of the bitmapinfoheader which*/
    /* were in the bitmapinfoheader.  Note that an OS/2 2.x+ bitmap*/
    /* file is exactly like a normal .bmp file, except the bitmapinfoheader*/
    /* is 24 bytes longer, and that data can all be 0's.*/

/*for targa files:*/

#define AUX_TGA_COMMENT 0x2001
    /*comment*/
#define AUX_TGA_EXTENSION 0x2002
    /*extension area*/
#define AUX_TGA_DEVELOPER_COUNT 0x2003
    /*developer area count*/
#define AUX_TGA_DEVELOPER 0x2004
    /*developer area field*/

#define AUX_GIF_1 0x3001
    /*graphics control extension*/
#define AUX_GIF_2 0x3001
    /*plain text extension*/
#define AUX_GIF_3 0x3001
    /*application extentions*/
#define AUX_GIF_4 0x3001
    /*comment extension*/

#include "tiff.h"

#define AUX_TIFF_Base 0x4000
#define AUX_TIFF_NEWSUBFILETYPE (AUX_TIFF_Base + TAG_NewSubfileType)
#define AUX_TIFF_SUBFILETYPE (AUX_TIFF_Base + TAG_SubfileType)
#define AUX_TIFF_THRESHHOLDING (AUX_TIFF_Base + TAG_Threshholding)
#define AUX_TIFF_CELLWIDTH (AUX_TIFF_Base + TAG_CellWidth)
#define AUX_TIFF_CELLLENGTH (AUX_TIFF_Base + TAG_CellLength)
#define AUX_TIFF_FILLORDER (AUX_TIFF_Base + TAG_FillOrder)
#define AUX_TIFF_DOCUMENTNAME (AUX_TIFF_Base + TAG_DocumentName)
#define AUX_TIFF_IMAGEDESCRIPTION (AUX_TIFF_Base + TAG_ImageDescription)
#define AUX_TIFF_MAKE (AUX_TIFF_Base + TAG_Make)
#define AUX_TIFF_MODEL (AUX_TIFF_Base + TAG_Model)
#define AUX_TIFF_MINSAMPLEVALUE (AUX_TIFF_Base + TAG_MinSampleValue)
#define AUX_TIFF_MAXSAMPLEVALUE (AUX_TIFF_Base + TAG_MaxSampleValue)
#define AUX_TIFF_XRESOLUTION (AUX_TIFF_Base + TAG_XResolution)
#define AUX_TIFF_YRESOLUTION (AUX_TIFF_Base + TAG_YResolution)
#define AUX_TIFF_PAGENAME (AUX_TIFF_Base + TAG_PageName)
#define AUX_TIFF_XPOSITION (AUX_TIFF_Base + TAG_XPosition)
#define AUX_TIFF_YPOSITION (AUX_TIFF_Base + TAG_YPosition)
#define AUX_TIFF_GRAYRESPONSEUNIT (AUX_TIFF_Base + TAG_GrayResponseUnit)
#define AUX_TIFF_GRAYRESPONSECURVE (AUX_TIFF_Base + TAG_GrayResponseCurve)
#define AUX_TIFF_T4OPTIONS (AUX_TIFF_Base + TAG_T4Options)
#define AUX_TIFF_T6OPTIONS (AUX_TIFF_Base + TAG_T6Options)
#define AUX_TIFF_RESOLUTIONUNIT (AUX_TIFF_Base + TAG_ResolutionUnit)
#define AUX_TIFF_PAGENUMBER (AUX_TIFF_Base + TAG_PageNumber)
#define AUX_TIFF_TRANSFERFUNCTION (AUX_TIFF_Base + TAG_TransferFunction)
#define AUX_TIFF_SOFTWARE (AUX_TIFF_Base + TAG_Software)
#define AUX_TIFF_DATETIME (AUX_TIFF_Base + TAG_DateTime)
#define AUX_TIFF_ARTIST (AUX_TIFF_Base + TAG_Artist)
#define AUX_TIFF_HOSTCOMPUTER (AUX_TIFF_Base + TAG_HostComputer)
#define AUX_TIFF_PREDICTOR (AUX_TIFF_Base + TAG_Predictor)
#define AUX_TIFF_WHITEPOINT (AUX_TIFF_Base + TAG_WhitePoint)
#define AUX_TIFF_PRIMARYCHROMATICITIES (AUX_TIFF_Base + TAG_PrimaryChromaticities)
#define AUX_TIFF_HALFTONEHINTS (AUX_TIFF_Base + TAG_HalftoneHints)
#define AUX_TIFF_INKSET (AUX_TIFF_Base + TAG_InkSet)
#define AUX_TIFF_INKNAMES (AUX_TIFF_Base + TAG_InkNames)
#define AUX_TIFF_NUMBEROFINKS (AUX_TIFF_Base + TAG_NumberOfInks)
#define AUX_TIFF_DOTRANGE (AUX_TIFF_Base + TAG_DotRange)
#define AUX_TIFF_TARGETPRINTER (AUX_TIFF_Base + TAG_TargetPrinter)
#define AUX_TIFF_EXTRASAMPLES (AUX_TIFF_Base + TAG_ExtraSamples)
#define AUX_TIFF_SAMPLEFORMAT (AUX_TIFF_Base + TAG_SampleFormat)
#define AUX_TIFF_SMINSAMPLEVALUE (AUX_TIFF_Base + TAG_SMinSampleValue)
#define AUX_TIFF_SMAXSAMPLEVALUE (AUX_TIFF_Base + TAG_SMaxSampleValue)
#define AUX_TIFF_TRANSFERRANGE (AUX_TIFF_Base + TAG_TransferRange)
#define AUX_TIFF_REFERENCEBLACKWHITE (AUX_TIFF_Base + TAG_ReferenceBlackWhite)
#define AUX_TIFF_COPYRIGHT (AUX_TIFF_Base + TAG_Copyright)

#endif
