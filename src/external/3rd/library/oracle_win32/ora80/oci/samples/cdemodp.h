/*
 * $Header: cdemodp.h 14-jul-99.12:42:52 mjaeger Exp $
 */

/* Copyright (c) 1998, 1999, Oracle Corporation.  All rights reserved.
*/

/*
   NAME
     cdemodp.h - C Demo prog for Direct Path api

   DESCRIPTION
     - Common header file for cdemodp driver and client progs.

   NOTES
     <other useful comments, qualifications, etc.>

   MODIFIED   (MM/DD/YY)
   mjaeger     07/14/99 - bug 808870: OCCS: convert tabs, no long lines
   cmlim       06/07/99 - add date_col fld to indicate column is chrdate or dat
   abrumm      12/22/98 - add outfn_sess to session structure
   cmlim       11/19/98 - increase begpos_fld & endpos_fld to ub4
   cmlim       11/17/98 - correct prec/scale types; add attributes & sess optio
   cmlim       10/02/98 - remove s.h
   cmlim       09/16/98 - common header file for cdemodp driver and client prog
   cmlim       09/16/98 - Creation (abrumm 04/07/98)

*/


#ifndef cdemodp_ORACLE
# define cdemodp_ORACLE

# include <oratypes.h>

# ifndef externdef
#  define externdef
# endif

/* External column attributes */
struct col
{
  text *name_col;                                             /* column name */
  ub2   id_col;                                            /* column load id */
  ub2   exttyp_col;                                         /* external type */
  text *datemask_col;                             /* datemask, if applicable */
  ub1   prec_col;                                /* precision, if applicable */
  sb1   scale_col;                                   /* scale, if applicable */
  ub2   csid_col;                                        /* character set id */
  ub1   date_col;            /* is column a chrdate or date? 1=TRUE. 0=FALSE */
};

/* Input field descriptor
 * For this example (and simplicity),
 * fields are strictly positional.
 */
struct fld
{
  ub4  begpos_fld;                             /* 1-based beginning position */
  ub4  endpos_fld;                             /* 1-based ending    position */
  ub4  maxlen_fld;                       /* max length for out of line field */
  ub4    flag_fld;
#define FLD_INLINE            0x1
#define FLD_OUTOFLINE         0x2
#define FLD_STRIP_LEAD_BLANK  0x4
#define FLD_STRIP_TRAIL_BLANK 0x8
};

struct tbl
{
  text        *owner_tbl;                                     /* table owner */
  text        *name_tbl;                                       /* table name */
  text        *subname_tbl;                        /* subname, if applicable */
  ub2          ncol_tbl;                     /* number of columns in col_tbl */
  text        *dfltdatemask_tbl;            /* table level default date mask */
  struct col  *col_tbl;                                 /* column attributes */
  struct fld  *fld_tbl;                                  /* field descriptor */
  ub1          parallel_tbl;                         /* parallel: 1 for true */
  ub1          nolog_tbl;                          /* no logging: 1 for true */
  ub4          xfrsz_tbl;                   /* transfer buffer size in bytes */
};

struct sess                        /* options for a direct path load session */
{
  text        *username_sess;                                        /* user */
  text        *password_sess;                                    /* password */
  text        *inst_sess;                            /* remote instance name */
  text        *outfn_sess;                                /* output filename */
  ub4          maxreclen_sess;          /* max size of input record in bytes */
};


#endif                                              /* cdemodp_ORACLE */
