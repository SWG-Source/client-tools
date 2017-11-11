// tiff.h
// PICTools ProductVersion 2.00.567
// Copyright (C) 2004-2008 Pegasus Imaging Corporation

#if !defined(_TIFFTAGS)
#define _TIFFTAGS

/*Baseline Tiff:*/
#define TAG_Artist 0x13b
#define TAG_BitsPerSample 0x102
#define TAG_CellLength 0x109
#define TAG_CellWidth 0x108
#define TAG_ColorMap 0x140
#define TAG_Compression 0x103
#define TAG_Copyright 0x8298
#define TAG_DateTime 0x132
#define TAG_ExtraSamples 0x152
#define TAG_FillOrder 0x10a
#define TAG_FreeByteCounts 0x121
#define TAG_FreeOffsets 0x120
#define TAG_GrayResponseCurve 0x123
#define TAG_GrayResponseUnit 0x122
#define TAG_HostComputer 0x13c
#define TAG_ImageDescription 0x10e
#define TAG_ImageLength 0x101
#define TAG_ImageWidth 0x100
#define TAG_Make 0x10f
#define TAG_MaxSampleValue 0x119
#define TAG_MinSampleValue 0x118
#define TAG_Model 0x110
#define TAG_NewSubfileType 0xfe
#define TAG_Orientation 0x112
#define TAG_PhotometricInterpretation 0x106
#define TAG_PlanarConfiguration 0x11c
#define TAG_ResolutionUnit 0x128
#define TAG_RowsPerStrip 0x116
#define TAG_SamplesPerPixel 0x115
#define TAG_Software 0x131
#define TAG_StripByteCounts 0x117
#define TAG_StripOffsets 0x111
#define TAG_SubfileType 0xff
#define TAG_Threshholding 0x107
#define TAG_XResolution 0x11a
#define TAG_YResolution 0x11b

/*Exif*/
#define TAG_ISOSpeedRatings 0x8827
#define TAG_ShutterSpeedValue 0x9201
#define TAG_ApertureValue 0x9202
#define TAG_FocalLength 0x920A
#define TAG_DNGVersion 0xc612

/*CCITT BiLevel Encodings:*/
#define TAG_T4Options 0x124
#define TAG_T6Options 0x125

/*Document Storage and Retrieval:*/
#define TAG_DocumentName 0x10d
#define TAG_PageName 0x11d
#define TAG_PageNumber 0x129
#define TAG_XPosition 0x11e
#define TAG_YPosition 0x11f

/*Differencing Predictor:*/
#define TAG_Predictor 0x13d

/*Tiled Images:*/
#define TAG_TileWidth 0x142
#define TAG_TileLength 0x143
#define TAG_TileOffsets 0x144
#define TAG_TileByteCounts 0x145

/*CMYK Images:*/
#define TAG_InkSet 0x14c
#define TAG_NumberOfInks 0x14e
#define TAG_InkNames 0x14d
#define TAG_DotRange 0x150
#define TAG_TargetPrinter 0x151

/*Halftone Hints:*/
#define TAG_HalftoneHints 0x141

/*Data Sample Format:*/
#define TAG_SampleFormat 0x153
#define TAG_SMinSampleValue 0x154
#define TAG_SMaxSampleValue 0x155

/*RGB Image Colorimetry:*/
#define TAG_WhitePoint 0x13e
#define TAG_PrimaryChromaticities 0x13f
#define TAG_TransferFunction 0x12d
#define TAG_TransferRange 0x156
#define TAG_ReferenceBlackWhite 0x214

/*YCbCr Images:*/
#define TAG_YCbCrCoefficients 0x211
#define TAG_YCbCrSubSampling 0x212
#define TAG_YCbCrPositioning 0x213

/*JPEG Compression:*/
#define TAG_JPEGTables 0x15b
#define TAG_JPEGProc 0x200
#define TAG_JPEGInterchangeFormat 0x201
#define TAG_JPEGInterchangeFormatLength 0x202
#define TAG_JPEGRestartInterval 0x203
#define TAG_JPEGLosslessPredictors 0x205
#define TAG_JPEGPointTransforms 0x206
#define TAG_JPEGQTables 0x207
#define TAG_JPEGDCTTables 0x208
#define TAG_JPEGDCTables 0x208
#define TAG_JPEGACTables 0x209

/*IPTC*/
#define TAG_IPTC 0x83BB     // == TAG_IPTCNAA below

// photoshop and exif tags
#define TAG_XMP 0x2bc
#define TAG_IccProfile 0x8773
#define TAG_EXIF 0x8769
#define TAG_GPSInfo 0x8825
#define TAG_IPTCNAA 0x83bb
#define TAG_Photoshop 0x8649
#define TAG_InteroperabilityIFD 0xa005

// MS HD Photo tags
#define TAG_HDP_PixelFormat 0xbc01
#define TAG_HDP_Transformation 0xbc02
#define TAG_HDP_Compression 0xbc03
#define TAG_HDP_ImageType 0xbc04
#define TAG_HDP_ImageWidth 0xbc80
#define TAG_HDP_ImageHeight 0xbc81
#define TAG_HDP_WidthResolution 0xbc82
#define TAG_HDP_HeightResolution 0xbc83
#define TAG_HDP_ImageOffset 0xbcc0
#define TAG_HDP_ImageByteCount 0xbcc1
#define TAG_HDP_AlphaOffset 0xbcc2
#define TAG_HDP_AlphaByteCount 0xbcc3
#define TAG_HDP_ImageDataDiscard 0xbcc4
#define TAG_HDP_AlphaDataDiscard 0xbcc5

// MS Photo tags
#define TAG_MSP_RatingStars          0x4746
#define TAG_MSP_RatingValue          0x4749
#define TAG_MSP_Caption              0x9c9b



#define TIF_BYTE        (1)
#define TIF_ASCII       (2)
#define TIF_SHORT       (3)
#define TIF_LONG        (4)
#define TIF_RATIONAL    (5)
#define TIF_SBYTE       (6)
#define TIF_UNDEFINED   (7)
#define TIF_SSHORT      (8)
#define TIF_SLONG       (9)
#define TIF_SRATIONAL   (10)
#define TIF_FLOAT       (11)
#define TIF_DOUBLE      (12)

typedef struct {
    BYTE Format[2];     /*II or MM*/
    WORD Zaphod;        /*42*/
    DWORD IFDOffset;    /*where the IFD is*/
} TIFF_HEAD;

typedef struct {
    WORD tag;
    WORD type;
    DWORD count;
    DWORD val_offset;
} DIR_ENTRY;
    /* a directory entry in the ifd*/

#endif
