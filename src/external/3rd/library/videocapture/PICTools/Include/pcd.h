// pcd.h
// PICTools ProductVersion 2.00.567

#ifndef _CD_H_
#define _CD_H_


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


/* Simple Informational structure` for Kodak's PhotoCD */

#ifndef CDSize
typedef enum {
    CD_SIZE_64 = 0,     /*   64 x    96 */
    CD_SIZE_128,        /*  128 x   192 */
    CD_SIZE_256,        /*  256 x   384 */
    CD_SIZE_512,        /*  512 x   768 */
    CD_SIZE_1024,       /* 1024 x  1536 */
    CD_SIZE_2048,       /* 2048 x  3072 */
    /* following resolution is not supported */
    CD_SIZE_2096        /* 4096 x  6144 */
} CDSize;

#endif /* CDSize */


/* MediaID value */
#ifndef NEGATIVE_COLOR

#define NEGATIVE_COLOR          0
#define REVERSAL_COLOR          1
#define HARDCOPY_COLOR          2
#define HARDCOPY_THERMAL        3
#define NEGATIVE_BLACKWHITE     4
#define REVERSAL_BLACKWHITE     5
#define HARDCOPY_BLACKWHITE     6
#define INTERNEGATIVE           7
#define SYNTHETIC_IMAGE         8

#endif /* NEGATIVE_COLOR */

typedef struct {
    unsigned short    Vers;                 /* Specification version */
    unsigned long     CreateTime;           /* Image create time in seconds from 1/1/70 */
    unsigned long     ModiedTime;           /* Modification time in seconds from 1/1/70 (0xffffffff = na)*/
    char              MediaId;              /* Media ID: */
    char              Copyright;            /* 0x1 = copyright restrictions:
                                               see copyrightFile for details,
                                               0xFF = undefined copyright restrictions */
    unsigned char     ScanSize[2];          /* pixel size:
                                              byte 0 = microns
                                              byte 1 = fraction (right of decimal point) */
    char              MediaType[32];        /* Media product type */
    char              ScanVendor[32];       /* Scanner vendor */
    char              ScanProdID[32];       /* Scanner type */
    char              CopyrightFile[32];   /* name of file containing Copyright */
} CDInfo, *CDInfoPtr;


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



#endif  /* _CD_H_ */

