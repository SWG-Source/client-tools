// ATI_Compress
// A library to compress/decompress textures

#ifndef ATI_COMPRESS
#define ATI_COMPRESS

typedef unsigned long ATI_TC_DWORD;
typedef unsigned char ATI_TC_BYTE;

typedef enum
{
    ATI_TC_FORMAT_ARGB_8888,
    ATI_TC_FORMAT_DXT1,
    ATI_TC_FORMAT_DXT3,
    ATI_TC_FORMAT_DXT5,
	ATI_TC_FORMAT_DXT5_xGBR,
	ATI_TC_FORMAT_DXT5_RxBG,
	ATI_TC_FORMAT_DXT5_RBxG,
	ATI_TC_FORMAT_DXT5_xRBG,
	ATI_TC_FORMAT_DXT5_RGxB,
	ATI_TC_FORMAT_DXT5_xGxR,
    ATI_TC_FORMAT_ATI2N,
    ATI_TC_FORMAT_ATI2N_XY,
    ATI_TC_FORMAT_ATI2N_DXT5,
    ATI_TC_FORMAT_MAX = ATI_TC_FORMAT_ATI2N_DXT5
} ATI_TC_FORMAT;

typedef struct _ATI_TC_Texture
{
	ATI_TC_DWORD	dwSize;				/* Size of this structure */
	ATI_TC_DWORD	dwWidth;			/* Width of the texture */
	ATI_TC_DWORD	dwHeight;			/* Height of the texture */
	ATI_TC_DWORD	dwPitch;			/* Distance to start of next line - necessary only for uncompressed textures */
	ATI_TC_FORMAT	format;				/* Format of the texture */
	ATI_TC_DWORD	dwDataSize;			/* Size of the allocated texture data */
	ATI_TC_BYTE*	pData;				/* Pointer to the texture data */
} ATI_TC_Texture;

typedef enum
{
    ATI_TC_OK = 0,
    ATI_TC_ABORTED,
    ATI_TC_ERR_INVALID_SOURCE_TEXTURE,
    ATI_TC_ERR_INVALID_DEST_TEXTURE,
    ATI_TC_ERR_UNSUPPORTED_SOURCE_FORMAT,
    ATI_TC_ERR_UNSUPPORTED_DEST_FORMAT,
    ATI_TC_ERR_SIZE_MISMATCH,
    ATI_TC_ERR_UNABLE_TO_INIT_CODEC,
    ATI_TC_ERR_GENERIC
} ATI_TC_ERROR;

typedef struct _ATI_TC_CompressOptions
{
	ATI_TC_DWORD	dwSize;					/* Size of this structure */

	/* Channel Weightings */
	/* With swizzled formats the weighting applies to the data within the specified channel */ 
	/* not the channel itself. */
	double			fWeightingRed;			/* Weighting of the Red or X Channel */
	double			fWeightingGreen;		/* Weighting of the Green or Y Channel */
	double			fWeightingBlue;			/* Weighting of the Blue or Z Channel */
	BOOL			bUseAdaptiveWeighting;	/* Adapt weighting on a per-block basis */
} ATI_TC_CompressOptions;


#ifdef __cplusplus
extern "C" {
#endif

/*
**	ATI_TC_Feedback_Proc
**	Feedback proc for conversion
**	Return non-NULL value to abort conversion
*/

typedef ATI_TC_DWORD (__cdecl * ATI_TC_Feedback_Proc)(float fProgress, void* pUser1, void* pUser2);

/*
**	ATI_TC_CalculateBufferSize
**	Calculates the required buffer size for the specified texture
*/

ATI_TC_DWORD __cdecl ATI_TC_CalculateBufferSize(const ATI_TC_Texture* pTexture);


/*
**	ATI_TC_ConvertTexture
**	Converts the source texture to the destination texture
*/

ATI_TC_ERROR __cdecl ATI_TC_ConvertTexture(const ATI_TC_Texture* pSourceTexture,	/* [in]  - Pointer to the source texture */
											ATI_TC_Texture* pDestTexture,			/* [out] - Pointer to the destination texture */
											const ATI_TC_CompressOptions* pOptions, /* [in]  - Pointer to the compression options - can be NULL */
											ATI_TC_Feedback_Proc pFeedbackProc,		/* [in]  - Pointer to the feedback proc - can be NULL */
											void* pUser1,							/* [in]  - User data to pass to the feedback proc */
											void* pUser2);							/* [in]  - User data to pass to the feedback proc */


#ifdef __cplusplus
};
#endif

#endif // !ATI_COMPRESS