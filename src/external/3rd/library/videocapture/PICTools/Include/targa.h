// targa.h
// PICTools ProductVersion 2.00.567
/*.T    Header to support TARGA routines   */
/* +-----------------------------------------------------------------------+ */
/* |                                                                       | */
/* |   Support for TARGA files                                             | */
/* |                                                                       | */
/* +-----------------------------------------------------------------------+ */

// Copyright (C) 2004-2008 Pegasus Imaging Corporation

/*modified 1/23/96 -- jweber -- added TARGA_FOOTER*/

#if !defined(_TARGA)
#define _TARGA

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

typedef struct  {
            BYTE    id_size, cm_type, image_type;
            WORD    cm_org, cm_length;
            BYTE    cm_elt_size;
            WORD    x_org, y_org;
            WORD    width, height;
            BYTE    ips, idb;
        } TARGA_HEAD;

typedef struct {
    DWORD extension_offset;
        /*offset of extension area, or 0*/
    DWORD developer_offset;
        /*offset of developer directory, or 0*/
    BYTE signature[18];
        /*must be "TRUEVISION-XFILE.0"; else not a valid footer*/
} TARGA_FOOTER;

typedef struct {
    short tag;
    long offset;
    long size;
} TARGA_DEVELOPER_TAG;

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

#endif
