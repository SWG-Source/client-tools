#ifndef __INCLUDE_ILIB_H__
#define __INCLUDE_ILIB_H__

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************
**									     
**  Image library 
**
**  Name    : ilib.h		header file for use with the image library
**  Author  : A.Hervas
**  Version : Beta 1.01, Wed May 18th 1994
**
**  Copyright (C) 1997 Nothing Real LLC
**
**  These coded instructions, statements and computer programs contain
**  unpublished information proprietary to Nothing Real LLC and
**  Alias|Wavefront Inc. and are protected by Canadian and US federal
**  copyright laws. They may not be disclosed to third parties or
**  copied or duplicated, in whole or in part, without prior written
**  consent of Alias|Wavefront Inc.
**
**  Unpublished-rights reserved under the Copyright Laws of the United States.
**
*****************************************************************************/

#include "flib.h"

#ifndef IMAGE_EXPORT /* for fcheck and irix */
#define IMAGE_EXPORT
#endif
/*
**	NAME
**	    ILintro - introduction to the image library
**
**	DESCRIPTION
**	    La librairie d'IO image est une partie de la librairie libfl.a
**	    Un ensemble de routines permet la lecture et l'ecriture d'images
**	    dans un fichier structure (cf: FLintro).
**
**	FILE FORMAT
**	    Le format d'un fichier image est tres souple, les contraintes
**	    sur le type, le nombre et la position relative des differents
**	    blocs de donnees etant tres reduites et souvent uniquement
**	    d'ordre logique (par exemple, "le header doit se trouver avant
**	    les blocs pixels", par opposition a des contraintes plus
**	    statiques telles que "le header commence a l'offset 124").
**	    Un fichier image etant avant tout un fichier (au sens libfl du
**	    terme) toute liberte est laissee a l'utilisateur d'inserer des
**	    blocs supplementaires.
**	    Une image minimale est constituee d'un groupe FOR4 (alignes sur 
**	    un mot)	de type CIMG contenant, dans l'ordre:
**	
**	    - un header BMHD (bitmap header) 
**	    - un groupe FOR4 de type TBMP (tiled bitmap)
**
**	    Les informations pixels sont regroupees dans le groupe TBMP
**	    ce qui autorise, le cas echeant, un saut rapide du groupe.
**	    Toujours dans sa version minimale, le groupe TBMP contient
**	    des blocs de donnees relatif aux pixels, dans un ordre
**	    quelconque. Par exemple, pour une image decoupees en 4 tiles:
**
**	    FOR4 <taille> CIMG
**	      BMHD 24 ... definition de la taille, des plans, etc...
**	      FOR4 <taille2> TBMP
**	        RGBA <ttile1> ... pixels du tile 1 ...
**	        RGBA <ttile2> ... pixels du tile 2 ...
**	        RGBA <ttile3> ... pixels du tile 3 ...
**	        RGBA <ttile4> ... pixels du tile 4 ...
**
**	    Le header est defini par la structure ILheader (cf: ilibh).
**	    Les blocs RGBA ont la structure suivante:
**	
**	    [x1, y1, x2, y2] : coordonnees du tile (2 octets chacune)
**	    [pixels] : encodes suivant le mode de compression.
**
**	    Si l'image a un z-buffer, celui est decrit par des blocs ZBUF
**	    ayant la meme structure que les blocs RGBA, encodes RLE.
**
**	FUNCTIONS
**	    Des fonctions permettent de lire et d'ecrire des image en mode
**	    ligne a ligne sans se soucier de la gestion du tiling.
**	    Les images peuvent egalement etre automatiquement zoomees et/ou
**	    corrigees (par lookup) a la lecture (la correction sur des
**	    donnees compressees etant nettement plus efficace).
**	    Enfin, un systeme de convertion automatique permet de lire
**	    des images stockees sous d'autres formats.
**
**	    Pour plus de details sur les routines de la librairie image
**	    se referer aux man pages ainsi qu'aux includes et exemples
**	    fournis avec la librairie.
*/

typedef	int (*ILgfunc)(void *, const void **, const float **);

typedef struct _ILheader
{
    unsigned int	width;
    unsigned int	height;
    unsigned short	prnum;
    unsigned short	prden;
    unsigned int	flags;
    unsigned short	bytes;
    unsigned short	tiles;
    unsigned int	compress;
    int		orgx;
    int		orgy;

} ILheader;

typedef struct _ILpage
{
    FLnode	node;
    unsigned int	fpos;
    unsigned int	size;
    unsigned short	coords[4];

} ILpage;

typedef struct _ILfkern
{
    uint	n;
    int		off;
    int *	w;
    
} ILfkern;

typedef struct _ILfint
{
    FLnode	node;
    int		use;
    float	rad, min, max;
    float *	tab;
    
} ILfint;

typedef struct _ILzoom
{
    FLnode	node;	
    FLmkey	memory;
    int		updated;
    uint	w,h;
    uint	zw,zh;
    int		extend;
    float	xblur;
    float	yblur;
    float	xoff;
    float	yoff;
    uint	lw,clw;
    int		degen;
    ILfint *	xshape;
    ILfint *	yshape;
    ILfint *	s_xshape;
    ILfint *	s_yshape;
    unsigned int *	cbuf;
    float *	zbuf;
    int *	xmap;
    int		curay;
    uint	y,ay,ny;
    ILfkern*	xfilt;
    ILfkern*	yfilt;
    uint	nrows;
    int **	fcrows;
    float **	fzrows;
    int *	acrow;
    float *	azrow;
    void *	id;
    ILgfunc	gfunc;
    uint	iavail;
    int *	iptr;
    int		gback;
    float	xlm,ybm;
    float	xrm,ytm;
    
} ILzoom;

typedef struct _ILcdef
{
    uint	    width;
    uint	    height;
    uint	    depth;
    uint	    mask;
    uint	    ignore;
    uint	    channels;
    uint	    stride;
    uint	    btrunc;
    uint	    size;
    const unsigned char **  lookups;
    
} ILcdef;

typedef	int (*ILcfunc)(void *, const void *, const ILcdef *);

typedef struct _ILimage
{
    FLfile		fp;

    int			version;
    ILheader		bmhd;	    /*	bitmap header		    */
    unsigned int		ictl;	    /*	image control field	    */
    
    int			twidth;	    /*	tiling			    */
    int			theight;
    
    FLfunc		auxfunc;    /*	callback for aux blocks	    */
    FLfunc		usrfunc;    /*	callback for user blocks    */
    
    unsigned int		bitmask;    /*	mask of active channels	    */
    int			channels;   /*	nb of color channels	    */
    int			pwidth;	    /*	pixel width in bytes	    */
    int			lwidth;	    /*	line width in bytes	    */
    
    char *		line;	    /*	line buffer in line mode    */
    float *		zline;	    /*	z-line in line mode	    */
    int			waterline;  /*	nb of lines in write buffer */
    int			startline;  /*	# of the first line in wbuf */
   
    FLlist		pages;	    /*	page buffer		    */
    int			pbound;	    /*	page boundary		    */
    int			pheight;    /*	page height		    */
    
    void *		color;	    /*	color buffer for line mode  */
    float *		zbuffer;    /*	z-buffer for line mode	    */
    
    int			samples;    /*	# of samples in page buffer */
    int			zsamples;   /*	same for Z page buffer	    */
    
    const unsigned char **	lookups;
    ILcfunc		cfunc;
    ILzoom *		zoom;
    
    FLlist		fields;
    int			headerTellPos;
    
} ILimage;

#define	IL_DefaultVersion   FLmakeversion(1,0)
#define	IL_Version	    FLmakeversion(1,1)

/*	builtin compressors	    */

#define	ILC_None	0
#define	ILC_RLE		1
#define	ILC_QRL		2
#define	ILC_QR4		3

#define	ILC_Builtin	31
#define	ILC_User	32
#define	ILC_Max		63

#define	ILC_Default	ILC_RLE

/*	Bitmap header's flags	    */

#define	ILH_RGB		0x00000001
#define	ILH_Alpha	0x00000002
#define	ILH_Zbuffer	0x00000004
#define	ILH_Abuffer	0x00000008
#define	ILH_Black	0x00000010
#define	ILH_Type	0x0000001f

#define	ILH_RGBA	(ILH_RGB | ILH_Alpha)
#define	ILH_RGBAZ	(ILH_RGB | ILH_Alpha | ILH_Zbuffer)

/*	Reserved for mapping	    */

#define	ILH_Reserved	0xffff0000
#define	ILH_LBuffer	0x00020000
#define	ILH_RBump	0x00100000
#define	ILH_GBump	0x00200000
#define	ILH_BBump	0x00400000
#define	ILH_ABump	0x00800000
#define	ILH_TxFlags	0x00f20000

/*	Control flags		    */

#define	ILF_HdrOK	0x00000001	/*  Header read or written	*/
#define	ILF_PageIO	0x00000002	/*  set when writing TBMP	*/
#define	ILF_LineIO	0x00000004	/*  set in line mode		*/
#define	ILF_LBLoaded	0x00000008	/*  long buffer loaded (maps)	*/
#define	ILF_Finish	0x00000010	/*  TBMP written and closed	*/
#define	ILF_Forced	0x00000020	/*  switches auto-tiling off	*/
#define	ILF_Extended	0x00000080	/*  16 extension of 8 bits img	*/
#define	ILF_Reading	0x00001000	/*  r or r+ mode		*/
#define	ILF_SysInit	0x000007ff

/*  User modifiable control flags   */

#define	ILF_User	0xffff0000
#define	ILF_Updown	0x00010000	/*  line are r/w from top	*/
#define	ILF_Vertical	0x00020000	/*  not yet implemented		*/
#define	ILF_Pack	0x00040000	/*  do not expand to RGBA	*/
#define	ILF_Full	0x00100000	/*  r/w in 16 bits format	*/
#define	ILF_NoMask	0x00200000	/*  Ignore mask when reading	*/ 

/*	Status of pages in cache    */

#define ILP_Loaded	0x00000001
#define ILP_Expanded	0x00000002
#define ILP_Zpage	0x00000004

/*	Indexes of corners	    */

#define	IL_TX1		0
#define	IL_TY1		1
#define	IL_TX2		2
#define	IL_TY2		3

/*	flags for 16 bit	    */

#define ILE_None	0
#define ILE_In		1
#define ILE_Out		2
#define ILE_InOut	3

/*	flags for ILconfig	    */

#define ICFG_NoCvt	0x00000001
#define ICFG_PipeCvt	0x00000002
#define ICFG_FifoCpr	0x00000004
#define ICFG_NoFCache	0x00000008

/*	Field types and qualifiers  */

#define	IL_FByte	0x01
#define	IL_FHalf	0x02
#define	IL_FWord	0x04
#define	IL_FFloat	0x04
#define	IL_FDouble	0x08

#define	IL_FType	FLmakeid(0, 0, 0, 0x0f)

#define	IL_FVolatile	0x00
#define	IL_FPermanent	0x10

#define	IL_FQual	FLMakeid(0, 0, 0, 0xf0)

/*	some actually used fields   */

#define	IL_FFormat	FLmakeid('F', 'M', 'T', IL_FByte)

/*
**	function prototypes
**
*****************************************************************************/

extern	IMAGE_EXPORT	const unsigned char	ildeflut[256];

extern	IMAGE_EXPORT ILimage *	ILopen(const char *, const char *);
extern	IMAGE_EXPORT	ILimage *	ILreopen(const char *, const char *, ILimage *);
extern	IMAGE_EXPORT int		ILclose(ILimage *);
extern	IMAGE_EXPORT	int		ILnext(ILimage *);

extern	IMAGE_EXPORT void		ILinfilter(const char *);
extern	IMAGE_EXPORT	void		ILoutfilter(const char *);

extern	IMAGE_EXPORT	int		ILrange(const char *, float *, char *, float *);

extern	IMAGE_EXPORT	ILheader *	ILgetheader(ILimage *);

extern	IMAGE_EXPORT int		ILgetsize(ILimage *, int *, int *);
extern	IMAGE_EXPORT int		ILgetorigin(ILimage *, int *, int *);
extern	IMAGE_EXPORT	int		ILgetratio(ILimage *, int *, int *);
extern	IMAGE_EXPORT int		ILgettype(ILimage *);
extern	IMAGE_EXPORT int		ILgetbpp(ILimage *);
extern	IMAGE_EXPORT	int		ILgetcompress(ILimage *);
extern	IMAGE_EXPORT	int		ILgetpages(ILimage *);

extern	IMAGE_EXPORT int		ILsetsize(ILimage *, int, int);
extern	IMAGE_EXPORT int		ILsetorigin(ILimage *, int, int);
extern	IMAGE_EXPORT int		ILsettype(ILimage *, int);
extern	IMAGE_EXPORT	int		ILsetratio(ILimage *, int, int);
extern	IMAGE_EXPORT int		ILsetbpp(ILimage *, int);
extern	IMAGE_EXPORT	int		ILsetcompress(ILimage *, int);
extern	IMAGE_EXPORT	int		ILsetpages(ILimage *, int);
extern	IMAGE_EXPORT	int		ILsetpagesize(ILimage *, int, int);
extern	IMAGE_EXPORT	int		ILsetlookups(ILimage *, const unsigned char **);

extern	IMAGE_EXPORT int		ILgetline(ILimage *, const void **, const float **);
extern	IMAGE_EXPORT int		ILreadline(ILimage *, void *, float *);

extern	IMAGE_EXPORT	int		ILsetzoom(ILimage *, const char *, int, int,
				float, float, float, float);
extern	IMAGE_EXPORT	void		ILdefaultzoom(const char *);

extern	IMAGE_EXPORT	ILzoom *	ILnewzoom(const char *);
extern	IMAGE_EXPORT	void		ILsetzoomblur(ILzoom *, float, float);
extern	IMAGE_EXPORT	void		ILsetzoomoffset(ILzoom *, float, float);
extern	IMAGE_EXPORT	void		ILsetzoommargin(ILzoom *, float, float, float, float);
extern	IMAGE_EXPORT	void		ILdelzoom(ILzoom *);
extern	IMAGE_EXPORT	int		ILbindzoom(ILzoom *, void *, ILgfunc, int, int, int,
				int, int);
extern	IMAGE_EXPORT	int		ILreadzoom(ILzoom *, void *, float *);
extern	IMAGE_EXPORT	int		ILgetzoom(ILzoom *, const void **, const float **);
extern  IMAGE_EXPORT int     ILputheader(ILimage *img);

extern	IMAGE_EXPORT int		ILwriteline(ILimage *, const void *, const float *);
extern	IMAGE_EXPORT	void *		ILbgnline(ILimage *, float **);
extern	IMAGE_EXPORT	int		ILendline(ILimage *);

extern	IMAGE_EXPORT	int		ILwritetile(ILimage *, const void *, const float *,
			    const int *, int);
extern	IMAGE_EXPORT	int		ILendtile(ILimage *);

extern	IMAGE_EXPORT void		ILconfig(int, int);
extern	IMAGE_EXPORT int		ILctrl(ILimage *, int, int);

extern	IMAGE_EXPORT	void		ILaddcompressor(int, const char *, ILcfunc, ILcfunc);
extern	IMAGE_EXPORT	void		ILfindcompressor(int, char *, ILcfunc *, ILcfunc *);

extern	IMAGE_EXPORT	int		ILpack(void *, const void *, const ILcdef *);
extern	IMAGE_EXPORT	int		ILpackrle(void *, const void *, const ILcdef *);
extern	IMAGE_EXPORT	int		ILpackqrl(void *, const void *, const ILcdef *);
extern	IMAGE_EXPORT	int		ILpackqr4(void *, const void *, const ILcdef *);
extern	IMAGE_EXPORT int		ILunpack(void *, const void *, const ILcdef *);
extern	IMAGE_EXPORT int		ILunpackrle(void *, const void *, const ILcdef *);
extern	IMAGE_EXPORT int		ILunpackqrl(void *, const void *, const ILcdef *);
extern	IMAGE_EXPORT	int		ILunpackqr4(void *, const void *, const ILcdef *);

extern	IMAGE_EXPORT void		ILfill(unsigned int *, int, int, int, int, int);

extern	IMAGE_EXPORT	int		ILloadbm(ILimage *, void *, float *);
extern	IMAGE_EXPORT	int		ILload(ILimage *, void *, float *);
extern	IMAGE_EXPORT	int		ILsave(ILimage *, const void *, const float *, int);

extern	IMAGE_EXPORT	int		ILgetfield(ILimage *, FLid, const void **);
extern	IMAGE_EXPORT	int		ILsetfield(ILimage *, FLid, uint size, const void *);
extern	IMAGE_EXPORT int		ILcopyfields(ILimage *dest, ILimage *src);

/*
**  Private functions
*/

extern	IMAGE_EXPORT	void		ILinitio(ILimage *);
extern	IMAGE_EXPORT	void		ILinitfields(ILimage *);
extern	IMAGE_EXPORT	int		ILreadfields(ILimage *);
extern	IMAGE_EXPORT	int		ILwritefields(ILimage *);
extern	IMAGE_EXPORT	void *		ILgetpath(void);
extern	IMAGE_EXPORT	int		ILrebindzoom(ILzoom *);

/*
**  temporary
*/

extern	IMAGE_EXPORT	int		ILqblur(unsigned int *, const unsigned int *, int, int, float);

/*
**  under development
*/

extern	IMAGE_EXPORT	int		ILdfft1x(float *, const float *, int);
extern	IMAGE_EXPORT	int		ILifft1x(float *, const float *, int);
extern	IMAGE_EXPORT	int		ILdfft1f(float *, const float *, int);
extern	IMAGE_EXPORT	int		ILifft1f(float *, const float *, int);
extern	IMAGE_EXPORT	void		ILqft(float *, float *, int);

extern	IMAGE_EXPORT	int		ILcload(ILimage *, void *, int, int, int, int, 
			    float, float, int);

#if (BYTE_ORDER == LITTLE_ENDIAN)
extern	IMAGE_EXPORT  void	ConvertABGRtoBGRA(void *dest, const ILcdef *cd);
extern	IMAGE_EXPORT  void	ConvertBGRAtoABGR(void *dest, const ILcdef *cd);
#endif 
#ifdef __cplusplus
}
#endif
#endif

