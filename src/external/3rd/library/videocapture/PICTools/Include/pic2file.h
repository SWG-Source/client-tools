/*.T PIC2FILE.H  Include file describing PIC2 File Format
 * PICTools ProductVersion 2.00.567
 * $Header: /PEGASUS/INCLUDE/pic2file.h 146   10/10/08 1:11p Buildmaster $
 * $Nokeywords: $
 */

/***************************************************************************\
*       Copyright (C) 1997-2008 Pegasus Imaging Corporation                 *
*       All rights reserved.                                                *
*****************************************************************************
*       Revision History:                                                   *
* created 03/14/97 - jim                                                    *
\***************************************************************************/



#if !defined(_PIC2FILE)
#define _PIC2FILE

#include "pic.h"

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



#define P2_Signature            "PIC2"
#define P2_Version              100



/* bit flags for File Version Packet Flags field */
#define P2F_CRC                 0x0001
#define P2F_Protected           0x0002
#define P2F_VirtualImages       0x0004
#define P2F_TableOfClusters     0x0008
#define P2F_MultipleImages      0x0010
#define P2F_RecursiveImages     0x0020



/* bit flags for Cluster Header Packet Flags field */
#define P2C_Deleted     0x0001
#define P2C_Protected   0x0010
#define P2C_Virtual     0x0020
#define P2C_Relative    0x0040
#define P2C_Recursive   0x0080
#define P2C_Slice       0x0100
#define P2C_Truncated   0x0200



/* values for Packet Type field */
#define P2P_EOF                 0
#define P2P_FileVersion         1
#define P2P_Script              2
#define P2P_FileDescription     3
#define P2P_TableOfClusters     4
#define P2P_ClusterHeader       5
#define P2P_ManifestFile        6
#define P2P_VirtualFile         7
#define P2P_RelativeFile        8
#define P2P_Copyright           9
#define P2P_AccessInformation   10
#define P2P_Promotion           11
#define P2P_RecursiveFile       12
#define P2P_RecursiveSlice      13
#define P2P_TableOfRecursive    14
#define P2P_Offset              15
#define P2P_TileDescription     17
#define P2P_PhysicalDimensions  18
#define P2P_Region              19
#define P2P_BMPHeader           20
#define P2P_Orientation         21
#define P2P_Comment             22
#define P2P_RawData             23
#define P2P_Watermark           24
#define P2P_ColorPalette        25
#define P2P_GammaLuminanceTable 26
#define P2P_GammaRGBTable       27
#define P2P_Filter              28
#define P2P_UserDecryption      29
#define P2P_PageInfo            30
#define P2P_DisplayInfo         31
#define P2P_ProgressiveLayers   32
#define P2P_LosslessInfo        33
#define P2P_WaveletInfo         34
#define P2P_TiffTag             35
#define P2P_Registration        36
#define P2P_Modca               37
#define P2P_Ioca                38
#define P2P_Cals                39
#define P2P_ErrorText           40
#define P2P_PngText             41
#define P2P_JBIG2Comment        42
#define P2P_IPTCDataSet         43
#define P2P_Skip                254
#define P2P_Extended            255



/* P2DATE is used by the Access Information Packet */
typedef struct
{
    BYTE Day;
    BYTE Month;
    WORD Year;
} P2DATE;

typedef struct
{
    DWORD Width;
    DWORD Height;
    DWORD ComponentSize;
} P2LAYER;


/********************************************************************/
/* Packet data structures                                           */
/********************************************************************/

typedef struct
{
    BYTE  Type;
    DWORD Length;
    WORD  Version;
    WORD  Flags;
    DWORD PIC2Length;
} P2PktFileVersion;

typedef struct
{
    BYTE  Type;
    DWORD Length;
    CHAR  ScriptText[1];
} P2PktScript;

typedef struct
{
    BYTE  Type;
    DWORD Length;
    CHAR  FileDescription[1];
} P2PktFileDescription;

typedef struct
{
    BYTE  Type;
    DWORD Length;
    WORD  k;
    DWORD Offset[1];
} P2PktTableOfClusters;

typedef struct
{
    BYTE  Type;
    DWORD Length;
    DWORD Filetype;
    WORD  Version;
    WORD  Flags;
    CHAR  FileName[1];
} P2PktClusterHeader;

typedef struct
{
    BYTE  Type;
    DWORD Length;
    BYTE  ActualFileContents[1];
} P2PktManifestFile;

typedef struct
{
    BYTE  Type;
    DWORD Length;
    CHAR  PathName[1];
} P2PktVirtualFile;

typedef struct
{
    BYTE  Type;
    DWORD Length;
    DWORD PIC2FileOffset;
} P2PktRelativeFile;

typedef struct
{
    BYTE  Type;
    DWORD Length;
    CHAR  Copyright[1];
} P2PktCopyright;

typedef struct
{
    BYTE   Type;
    DWORD  Length;
    P2DATE DateCreated;
    P2DATE DateLastModified;
    P2DATE DateLastAccessed;
    DWORD  NumberAccesses;
} P2PktAccessInformation;

typedef struct
{
    BYTE  Type;
    DWORD Length;
    CHAR  Promotion[1];
} P2PktPromotion;

typedef struct
{
    BYTE  Type;
    DWORD Length;
    BYTE  Data[1];
} P2PktRecursiveFile;

typedef struct
{
    BYTE  Type;
    DWORD Length;
    WORD  Flags;
    WORD  BoxWidth;
    WORD  BoxHeight;
    WORD  TileWidth;
    WORD  TileHeight;
} P2PktTileDescription;

typedef struct
{
    BYTE  Type;
    DWORD Length;
    BYTE  Units;    /* 0 none, 1 inches, 2 cm */
    WORD  XNum;
    WORD  XDen;
    WORD  YNum;
    WORD  YDen;
} P2PktPhysicalDimensions;

typedef struct
{
    BYTE  Type;
    DWORD Length;
    REGION Region;
} P2PktRegion;

typedef struct
{
    BYTE  Type;
    DWORD Length;
    BITMAPINFOHEADER bmih;
} P2PktBMPHeader;

typedef struct
{
    BYTE  Type;
    DWORD Length;
    WORD  Orientation;
} P2PktOrientation;

typedef struct
{
    BYTE  Type;
    DWORD Length;
    CHAR  Comment[1];
} P2PktComment;

typedef struct
{
    BYTE  Type;
    DWORD Length;
    CHAR  RawDescription[4];
    DWORD RawLength;
    BYTE  RawData[1];
} P2PktRawData;

typedef struct
{
    BYTE  Type;
    DWORD Length;
    CHAR  Watermark[1];
} P2PktWatermark;

typedef struct
{
    BYTE      Type;
    DWORD     Length;
    WORD      FirstColor;
    WORD      NumColors;
    RGBTRIPLE Colors[1];
} P2PktColorPalette;

typedef struct
{
    BYTE  Type;
    DWORD Length;
    BYTE  GammaLuminanceTable[256];
    CHAR  Description[1];    /* (optional) */
} P2PktGammaLuminanceTable;

typedef struct
{
    BYTE  Type;
    DWORD Length;
    BYTE  BlueGammaRGBTable[256];
    BYTE  GreenGammaRGBTable[256];
    BYTE  RedGammaRGBTable[256];
    CHAR  Description[1];    /* (optional) */
} P2PktGammaRGBTable;

typedef struct
{
    BYTE  Type;
    DWORD Length;
    WORD  k;
    WORD  WeightTable[1];    /* index is actually k * k */
/*  WORD  Divisor; */        /* == &WeightTable[k * k]  */
/*  CHAR  Description; */    /* (optional) == &WeightTable[k * k + 1] */
} P2PktFilter;

typedef struct
{
    BYTE  Type;
    DWORD Length;
    WORD  Method;
    WORD  Version;
    BYTE  RelevantData[1];
} P2PktUserDecryption;

typedef struct
{
    BYTE  Type;
    DWORD Length;
    DWORD PageNum;
    DWORD NumOfPages;
} P2PktPageInfo;

typedef struct
{
    BYTE  Type;
    DWORD Length;
    WORD  TransparentColorIndex;
    WORD  DispositionMethod;
    WORD  Delay;
    DWORD XOffset;
    DWORD YOffset;
} P2PktDisplayInfo;

typedef struct
{
    BYTE    Type;
    DWORD   Length;
    BYTE    NumLayers;
    P2LAYER LayerDescription[1];
} P2PktProgressiveLayers;

typedef struct
{
    BYTE  Type;
    DWORD Length;
    BYTE  CompressionMethod;
    BYTE  CompressionOrder;
    BYTE  HashMethod;
    BYTE  AllowedBitErr;
    WORD  Threshold;  /* M=P/J => CountThreshold/RescaleThreshold */
    WORD  NumHeaders; /* M=P */
    WORD  NumLists;   /* M=P */
} P2PktLosslessInfo;

typedef struct
{
    BYTE  Type;
    DWORD Length;
    WORD  Quality;
    WORD  MinThreshold;
    WORD  MaxThreshold;
} P2PktWaveletInfo;

/* P2PktWaveletInfo2 has the same Type as P2PktWaveletInfo
    they are distinguished from each other by Length */
typedef struct
{
    BYTE  Type;
    DWORD Length;
    BYTE  TolerancePercentOutputSize;
    BYTE  AutoQuantizationThreshold;
    BYTE  NumSlices;
    BYTE  ChunkProgSlices;
    DWORD MaxOutputSize;
    WAVECOMPONENT Y;
    /*WAVECOMPONENT I; -- may be present depending on Length */
    /*WAVECOMPONENT Q; -- may be present depending on Length */
} P2PktWaveletInfo2;

typedef struct
{
    BYTE  Type;
    DWORD Length;
    BYTE  Location;
    WORD  TiffTag;
    WORD  TiffType;
    DWORD TiffCount;
    BYTE  TiffData[1];
} P2PktTiffTag;
#define LOC_PRIMARYIMAGEIFD     ( 0 )
#define LOC_THUMBNAILIFD        ( 1 )
#define LOC_EXIFIFD             ( 2 )
#define LOC_APP3IFD             ( 3 )
#define LOC_GPSINFOIFD          ( 4 )
#define LOC_INTEROPERABILITYIFD ( 5 )
/* ^ values for P2PktTiffTag.Location */


typedef struct
{
    BYTE  Type;
    DWORD Length;
    LONG  Op;
    DWORD Code;
    char  Name[1];
} P2PktRegistration;


typedef struct
{
    BYTE  Type;
    DWORD Length;
    DWORD SFID;
    DWORD SFLen;
    BYTE  SFData[1];
} P2PktModca;



typedef struct
{
    BYTE  Type;
    DWORD Length;
    WORD  SDFID;
    WORD  SDFLen;
    BYTE  SDFData[1];
} P2PktIoca;



typedef struct
{
    BYTE  Type;
    DWORD Length;
    char  CalsID[9];
    char  CalsData[1];
} P2PktCals;



typedef struct
{
    BYTE  Type;
    DWORD Length;
    WORD  k;
    DWORD Offset[1];
} P2PktTableOfRecursive;


typedef struct
{
    BYTE  Type;
    DWORD Length;
    BYTE  Data[1];
} P2PktRecursiveSlice;


typedef struct
{
    BYTE  Type;
    DWORD Length;
    BYTE  T;            /* offset tag 0 = generic, 1 = levels, 2 = restarts,... */
    WORD  k;            /* # of offsets */
    DWORD Offset[1];
} P2PktOffset;


#define P2SEVERITY_FATAL    (255)
#define P2SEVERITY_WARNING  (128)
#define P2SEVERITY_INFO     (0)
typedef struct
{
    BYTE  Type;
    DWORD Length;
    BYTE  ErrorSeverity;
    char  ErrorText[1];
} P2PktErrorText;


typedef struct
{
    BYTE  Type;
    DWORD Length;
    char  Key[80];  // including null terminator
    char  Text[1];
} P2PktPngText;


#define P2JBIG2Com_GlobalPage 0
#define P2JBIG2Com_LocalPage  1
#define P2JBIG2Com_8bitchars  0  // ANSI (ISO/IEC 8859)
#define P2JBIG2Com_16bitchars 1  // UNICODE (ISO/IEC 10646 UCS-2)
typedef struct
{
    BYTE  Type;
    DWORD Length;
    DWORD PageNum;   // page association number
    DWORD SegNum;    // number of this segment, ties different packets together
    DWORD ValueOfs;  // offset to Value in NameThenValue string
    BYTE  CharSet;
    char  NameThenValue[1];  // Name:optional_null:Value:optional_null
} P2PktJBIG2Comment;


typedef struct
{
    WORD  manufacturerId;
    DWORD equipmentType;
    BYTE  date[8];
    WORD  num;
} IPTC_3_10_Struct;

typedef struct
{
    WORD owner;
    BYTE compression;
    BYTE revision;
} IPTC_3_110_Struct;


#define IPTC_UNDEFINED      (0)
#define IPTC_BYTE           (1)
#define IPTC_ASCII          (2)
#define IPTC_INTEGER1       (3)
#define IPTC_INTEGER2       (4)
#define IPTC_INTEGER3       (5)
#define IPTC_INTEGER4       (6)
#define IPTC_3_10_STRUCT    (7)
#define IPTC_3_110_STRUCT   (8)


typedef struct
{
    BYTE  Type;
    DWORD Length;
    BYTE  Record;
    BYTE  DataSet;
    DWORD DataSetType;
    DWORD DataSetLength;
    BYTE  DataField[1];
} P2PktIPTCDataSet;

typedef struct
{
    BYTE  Type;
    DWORD Length;
    BYTE  DataToSkip[1];
} P2PktSkip;

typedef struct
{
    BYTE  Type;
    DWORD Length;
    DWORD ExtendedType;
    BYTE  Data[1];
} P2PktExtended;



typedef struct
{
    BYTE Type;
    DWORD Length;
} P2PktNull;

typedef struct
{
    BYTE Type;
    DWORD Length;
    BYTE Data[1];
} P2PktGeneric;



typedef union
{
    P2PktNull                NullPkt;
    P2PktGeneric             Generic;
    P2PktFileVersion         FileVersion;
    P2PktScript              Script;
    P2PktFileDescription     FileDescription;
    P2PktTableOfClusters     TableOfClusters;
    P2PktClusterHeader       ClusterHeader;
    P2PktManifestFile        ManifestFile;
    P2PktVirtualFile         VirtualFile;
    P2PktRelativeFile        RelativeFile;
    P2PktCopyright           Copyright;
    P2PktAccessInformation   AccessInformation;
    P2PktPromotion           Promotion;
    P2PktRecursiveFile       RecursiveFile;
    P2PktTileDescription     TileDescription;
    P2PktPhysicalDimensions  PhysicalDimensions;
    P2PktRegion              Region;
    P2PktBMPHeader           BMPHeader;
    P2PktOrientation         Orientation;
    P2PktComment             Comment;
    P2PktRawData             RawData;
    P2PktWatermark           Watermark;
    P2PktColorPalette        ColorPalette;
    P2PktGammaLuminanceTable GammaLuminanceTable;
    P2PktGammaRGBTable       GammaRGBTable;
    P2PktFilter              Filter;
    P2PktUserDecryption      UserDecryption;
    P2PktPageInfo            PageInfo;
    P2PktDisplayInfo         DisplayInfo;
    P2PktProgressiveLayers   ProgressiveLayers;
    P2PktLosslessInfo        LosslessInfo;
    P2PktWaveletInfo         WaveletInfo;
    P2PktTiffTag             TiffTag;
    P2PktRegistration        Registration;
    P2PktModca               Modca;
    P2PktIoca                Ioca;
    P2PktCals                Cals;
    P2PktOffset              Offset;
    P2PktErrorText           Error;
    P2PktPngText             PngText;
    P2PktJBIG2Comment        JBIG2Comment;
    P2PktIPTCDataSet         IPTCDataSet;
    P2PktSkip                Skip;
    P2PktExtended            Extended;
} P2Pkt;



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

#endif  /* #if !defined(_PIC2FILE) */
