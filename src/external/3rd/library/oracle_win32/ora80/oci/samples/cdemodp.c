#ifdef RCSID
static char *RCSid =
   "$Header: cdemodp.c 14-jul-99.13:06:13 mjaeger Exp $ ";
#endif /* RCSID */

/* Copyright (c) 1998, 1999, Oracle Corporation.  All rights reserved.
*/

/*
 *      -- cdemodp.c --
 * This is one of two C files needed to create a demo that loads
 * data through direct path api.
 *
 * To build and run the demo, please read directions located near
 * the top of cdemodp_lip.c.
 *
 */

/*
   NAME
     cdemodp.c - C Demo program for Direct Path api

   DESCRIPTION
     - Direct Path Api driver program to demonstrate loading.

   NOTES
     Demonstrates usage of the direct path API.

   MODIFIED   (MM/DD/YY)
   mjaeger     07/14/99 - bug 808870: OCCS: convert tabs, no long lines
   cmlim       06/07/99 - same as tkpidrv.c@@/main/15 minus operation mode
                        - (mode_tbl) & query_stats()
   cmlim       03/25/99 - make same as tkpidrv.c minus op mode (mode_tbl)
   abrumm      03/01/99 - OCI_ATTR_ROW_COUNT change
   abrumm      02/10/99 - field_set: trim trailing whitespace bug
   cmlim       02/02/99 - set OCI_DIRPATH_COL_NULL correctly
                        - use O_BINARY open mode for WIN32COMMON, WIN32, _WIN32
                        - do not use non-public functions (ie. ktsgnospc)
                        - removed "include <unistd.h>" - not needed
                        - write out number of records LOADED
   abrumm      12/22/98 - fix LOAD_NO_DATA, CONVERT_CONTINUE case
   abrumm      10/13/98 - fix field_set
   cmlim       11/17/98 - add attributes & session options; increase clen to ub
   cmlim       10/07/98 - change stderr to stdout where appropriate
   cmlim       10/02/98 - remove sn.h & deliberate error cases; fix field_set
   cmlim       09/16/98 - data-loading driver to demo direct path api
   cmlim       09/16/98 - Creation (abrumm 04/07/98)
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <fcntl.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <oratypes.h>
#include <oci.h>
#include <cdemodp0.h>
#include <cdemodp.h>

#ifndef bit
# define bit(x, y) ((x) & (y))
#endif

#ifndef OER
# define OER(x) (x)
#endif

struct loadctl
{
  ub4                 nrow_ctl;            /* number of rows in column array */
  ub2                 ncol_ctl;         /* number of columns in column array */
  OCIEnv             *envhp_ctl;                       /* environment handle */
  OCIServer          *srvhp_ctl;                            /* server handle */
  OCIError           *errhp_ctl;                             /* error handle */
  OCIError           *errhp2_ctl;                /* yet another error handle */
  OCISvcCtx          *svchp_ctl;                          /* service context */
  OCISession         *authp_ctl;                   /* authentication context */
  OCIParam           *colLstDesc_ctl;        /* column list parameter handle */
  OCIDirPathCtx      *dpctx_ctl;                      /* direct path context */
  OCIDirPathColArray *dpca_ctl;           /* direct path column array handle */
  OCIDirPathStream   *dpstr_ctl;                /* direct path stream handle */
  ub1                *buf_ctl;    /* pre-alloc'd buffer for out-of-line data */
  ub4                 bufsz_ctl;                 /* size of buf_ctl in bytes */
  ub4                 bufoff_ctl; /* offset into buf_ctl which is not in use */
  ub4                *otor_ctl;                  /* Offset to Recnum mapping */
  ub1                *inbuf_ctl;                 /* buffer for input records */
  struct pctx         pctx_ctl;                     /* partial field context */
};

/* Forward references: */
STATICF void  field_flush(/*_ struct loadctl *ctlp, ub4 rowoff _*/);
STATICF sword field_set(/*_ struct loadctl *ctlp, struct tbl *tblp,
                            text *recp, ub4 rowoff _*/);

STATICF void  init_load(/*_ struct loadctl *ctl, struct tbl *table,
                            struct sess *session _*/);

STATICF void  simple_load(/*_ struct loadctl *ctlp, struct tbl *tblp,
                              struct sess *session, FILE *inputfp _*/);

STATICF void  finish_load(/*_ struct loadctl *ctl _*/);

STATICF void  errprint(/*_ dvoid *errhp, ub4 htype, sb4 *errcodep _*/);

STATICF void  checkerr(/*_ dvoid *errhp, ub4 htype, sword status,
                           text *note, sb4 state, text *file, sb4 line _*/);
STATICF void  cleanup(/*_ struct loadctl *ctlp, sb4 ex_status _*/);
STATICF sword do_convert(/*_ struct loadctl *ctlp, ub4 startoff, ub4 rowcnt,
                             ub4 *cvtCntp, ub2 *badcoffp _*/);
STATICF sword do_load(/*_ struct loadctl *ctlp, ub4 *loadCntp _*/);
STATICF int           main(/*_ int argc, char *argv[] _*/);


/* OCI_CHECK(errhp, ub4 errhptype, sb4 status, struct loadctl *ctlp,
 *          OCIfunction());
 * errhp is typically a (OCIError *), and errhptype is OCI_HTYPE_ERROR.
 * errhp in some cases may be an (OCIEnv *), and errhptype is OCI_HTYPE_ENV.
 */
#define OCI_CHECK(errhp, htype, status, ctlp, OCIfunc) \
if (OCI_SUCCESS != ((status) = (OCIfunc))) \
{ \
  checkerr((dvoid *)(errhp), (ub4)(htype), (sword)(status), (text *)0, \
           (sb4)0, (text *)__FILE__, (sb4)__LINE__); \
  if ((status) != OCI_SUCCESS_WITH_INFO) \
    cleanup((struct loadctl *)ctlp, (sb4)1); \
} else

#define CHECKERR(errhp, htype, status) \
  checkerr((dvoid *)errhp, (ub4)(htype), (sword)(status), (text *)0, \
           (sb4)0, (text *)__FILE__, (sb4)__LINE__);

#define FATAL(note, state) \
do \
{ \
  checkerr((dvoid *)0, (ub4)OCI_HTYPE_ERROR, (sword)OCI_SUCCESS,           \
           (text *)(note), (sb4)(state), (text *)__FILE__, (sb4)__LINE__); \
  cleanup((ctlp), (sb4)2); \
} while (0)

/* External references: */
externref struct tbl    table;
externref struct sess    session;

/* External definitions: */
externdef FILE         *output_fp;                         /* for error msgs */


int main(argc, argv)
int argc;
char *argv[];
{
  sword   ociret;
  struct  loadctl  ctl;
  struct  loadctl *ctlp = &ctl;

  output_fp = (session.outfn_sess) ? fopen((char *)session.outfn_sess, "w")
                                   : stderr;

  memset((dvoid *)ctlp, 0, sizeof(struct loadctl));

  /* set up OCI environment and connect to the ORACLE server */

  OCI_CHECK((dvoid *)0, (ub4)0, ociret, ctlp,
            OCIInitialize((ub4) OCI_DEFAULT, (dvoid *)0,
                          (dvoid * (*)(dvoid *, size_t)) 0,
                          (dvoid * (*)(dvoid *, dvoid *, size_t))0,
                          (void (*)(dvoid *, dvoid *)) 0 ));

  OCI_CHECK(ctlp->envhp_ctl, OCI_HTYPE_ENV, ociret, ctlp,
            OCIEnvInit((OCIEnv **)&ctlp->envhp_ctl, OCI_DEFAULT, (size_t)0,
                       (dvoid **)0));

  /* allocate error handles */
  OCI_CHECK(ctlp->envhp_ctl, OCI_HTYPE_ENV, ociret, ctlp,
            OCIHandleAlloc((dvoid *)ctlp->envhp_ctl,
                           (dvoid **)&ctlp->errhp_ctl, OCI_HTYPE_ERROR,
                           (size_t)0, (dvoid **)0));
  OCI_CHECK(ctlp->envhp_ctl, OCI_HTYPE_ENV, ociret, ctlp,
            OCIHandleAlloc((dvoid *)ctlp->envhp_ctl,
                           (dvoid **)&ctlp->errhp2_ctl, OCI_HTYPE_ERROR,
                           (size_t)0, (dvoid **)0));

  /* server contexts */
  OCI_CHECK(ctlp->envhp_ctl, OCI_HTYPE_ENV, ociret, ctlp,
            OCIHandleAlloc((dvoid *)ctlp->envhp_ctl,
                           (dvoid **)&ctlp->srvhp_ctl, OCI_HTYPE_SERVER,
                           (size_t)0, (dvoid **)0));

  OCI_CHECK(ctlp->envhp_ctl, OCI_HTYPE_ENV, ociret, ctlp,
            OCIHandleAlloc((dvoid *)ctlp->envhp_ctl,
                           (dvoid **)&ctlp->svchp_ctl, OCI_HTYPE_SVCCTX,
                           (size_t)0, (dvoid **)0));

  OCI_CHECK(ctlp->errhp_ctl, OCI_HTYPE_ERROR, ociret, ctlp,
            OCIServerAttach(ctlp->srvhp_ctl, ctlp->errhp_ctl,
                            session.inst_sess,
                            (sb4)strlen((const char *)session.inst_sess),
                            OCI_DEFAULT));

  /* set attribute server context in the service context */
  OCI_CHECK(ctlp->errhp_ctl, OCI_HTYPE_ERROR, ociret, ctlp,
            OCIAttrSet((dvoid *)ctlp->svchp_ctl, OCI_HTYPE_SVCCTX,
                       (dvoid *)ctlp->srvhp_ctl, (ub4)0, OCI_ATTR_SERVER,
                       ctlp->errhp_ctl));

  OCI_CHECK(ctlp->envhp_ctl, OCI_HTYPE_ENV, ociret, ctlp,
            OCIHandleAlloc((dvoid *)ctlp->envhp_ctl,
                           (dvoid **)&ctlp->authp_ctl, (ub4)OCI_HTYPE_SESSION,
                           (size_t)0, (dvoid **)0));

  OCI_CHECK(ctlp->errhp_ctl, OCI_HTYPE_ERROR, ociret, ctlp,
            OCIAttrSet((dvoid *)ctlp->authp_ctl, (ub4)OCI_HTYPE_SESSION,
                       (dvoid *)session.username_sess,
                       (ub4)strlen((char *)session.username_sess),
                       (ub4)OCI_ATTR_USERNAME, ctlp->errhp_ctl));

  OCI_CHECK(ctlp->errhp_ctl, OCI_HTYPE_ERROR, ociret, ctlp,
            OCIAttrSet((dvoid *)ctlp->authp_ctl, (ub4)OCI_HTYPE_SESSION,
                       (dvoid *)session.password_sess,
                       (ub4)strlen((char *)session.password_sess),
                       (ub4)OCI_ATTR_PASSWORD, ctlp->errhp_ctl));

  /* begin a session */
  OCI_CHECK(ctlp->errhp_ctl, OCI_HTYPE_ERROR, ociret, ctlp,
            OCISessionBegin(ctlp->svchp_ctl, ctlp->errhp_ctl, ctlp->authp_ctl,
                            OCI_CRED_RDBMS, (ub4)OCI_DEFAULT));

  /* set authentication context into service context */
  OCI_CHECK(ctlp->errhp_ctl, OCI_HTYPE_ERROR, ociret, ctlp,
            OCIAttrSet((dvoid *)ctlp->svchp_ctl, (ub4)OCI_HTYPE_SVCCTX,
                       (dvoid *)ctlp->authp_ctl, (ub4)0, (ub4)OCI_ATTR_SESSION,
                       ctlp->errhp_ctl));

  init_load(ctlp, &table, &session);                  /* initialize the load */
  simple_load(ctlp, &table, &session, stdin);                   /* load data */
  finish_load(ctlp);                                      /* finish the load */

  cleanup(ctlp, (sb4)0);
  /* NOTREACHED */
}

/* init_load:
 *   Function which prepares for a direct path load using the direct
 *   path API on the table described by 'tblp'.  The loadctl
 *   structure given by 'ctlp' has appropriately initialized
 *   environment, and service context handles (alredy connected to
 *   the server) prior to calling this function.
 */
STATICF void
init_load(ctlp, tblp, sessp)
struct loadctl *ctlp;
struct tbl     *tblp;
struct sess    *sessp;
{
  struct  col   *colp;
  struct  fld   *fldp;
  sword         ociret;                        /* return code from OCI calls */
  OCIDirPathCtx *dpctx;                               /* direct path context */
  OCIParam      *colDesc;                     /* column parameter descriptor */
  ub1            parmtyp;
  ub1           *timestamp = (ub1 *)0;
  ub4            size;
  ub2            i;
  ub4            pos;

  /* allocate and initialize a direct path context */
  OCI_CHECK(ctlp->envhp_ctl, OCI_HTYPE_ENV, ociret, ctlp,
            OCIHandleAlloc((dvoid *)ctlp->envhp_ctl,
                           (dvoid **)&ctlp->dpctx_ctl,
                           (ub4)OCI_HTYPE_DIRPATH_CTX,
                           (size_t)0, (dvoid **)0));

  dpctx = ctlp->dpctx_ctl;                                      /* shorthand */

  OCI_CHECK(ctlp->errhp_ctl, OCI_HTYPE_ERROR, ociret, ctlp,
            OCIAttrSet((dvoid *)dpctx, (ub4)OCI_HTYPE_DIRPATH_CTX,
                       (dvoid *)tblp->name_tbl,
                       (ub4)strlen((const char *)tblp->name_tbl),
                       (ub4)OCI_ATTR_NAME, ctlp->errhp_ctl));


  if (tblp->subname_tbl && *tblp->subname_tbl)    /* set (sub)partition name */
  {
    OCI_CHECK(ctlp->errhp_ctl, OCI_HTYPE_ERROR, ociret, ctlp,
              OCIAttrSet((dvoid *)dpctx, (ub4)OCI_HTYPE_DIRPATH_CTX,
                         (dvoid *)tblp->subname_tbl,
                         (ub4)strlen((const char *)tblp->subname_tbl),
                         (ub4)OCI_ATTR_SUB_NAME, ctlp->errhp_ctl));
  }

  if (tblp->owner_tbl)                            /* set schema (owner) name */
  {
    OCI_CHECK(ctlp->errhp_ctl, OCI_HTYPE_ERROR, ociret, ctlp,
              OCIAttrSet((dvoid *)dpctx, (ub4)OCI_HTYPE_DIRPATH_CTX,
                         (dvoid *)tblp->owner_tbl,
                         (ub4)strlen((const char *)tblp->owner_tbl),
                         (ub4)OCI_ATTR_SCHEMA_NAME, ctlp->errhp_ctl));
  }

  /* Note: setting tbl default datemask will not trigger client library
   * to check strings for dates - only setting column datemask will.
   */
  if (tblp->dfltdatemask_tbl)
  {
    OCI_CHECK(ctlp->errhp_ctl, OCI_HTYPE_ERROR, ociret, ctlp,
              OCIAttrSet((dvoid *)dpctx, (ub4)OCI_HTYPE_DIRPATH_CTX,
                         (dvoid *)tblp->dfltdatemask_tbl,
                         (ub4)strlen((const char *)tblp->dfltdatemask_tbl),
                         (ub4)OCI_ATTR_DATEFORMAT, ctlp->errhp_ctl));
  }

  if (tblp->parallel_tbl)                 /* set table level parallel option */
  {
    OCI_CHECK(ctlp->errhp_ctl, OCI_HTYPE_ERROR, ociret, ctlp,
              OCIAttrSet((dvoid *)dpctx, (ub4)OCI_HTYPE_DIRPATH_CTX,
                         (dvoid *)&tblp->parallel_tbl,
                         (ub4)0, (ub4)OCI_ATTR_DIRPATH_PARALLEL,
                         ctlp->errhp_ctl));
  }

  if (tblp->nolog_tbl)                       /* set table level nolog option */
  {
    OCI_CHECK(ctlp->errhp_ctl, OCI_HTYPE_ERROR, ociret, ctlp,
              OCIAttrSet((dvoid *)dpctx, (ub4)OCI_HTYPE_DIRPATH_CTX,
                         (dvoid *)&tblp->nolog_tbl, (ub4)0,
                         (ub4)OCI_ATTR_DIRPATH_NOLOG, ctlp->errhp_ctl));
  }

  /* set number of columns to be loaded */
  OCI_CHECK(ctlp->errhp_ctl, OCI_HTYPE_ERROR, ociret, ctlp,
            OCIAttrSet((dvoid *)dpctx, (ub4)OCI_HTYPE_DIRPATH_CTX,
                       (dvoid *)&tblp->ncol_tbl,
                       (ub4)0, (ub4)OCI_ATTR_NUM_COLS, ctlp->errhp_ctl));


  /* get the column parameter list */
  OCI_CHECK(ctlp->errhp_ctl, OCI_HTYPE_ERROR, ociret, ctlp,
            OCIAttrGet((dvoid *)dpctx,
                       OCI_HTYPE_DIRPATH_CTX,
                       (dvoid *)&ctlp->colLstDesc_ctl, (ub4 *)0,
                       OCI_ATTR_LIST_COLUMNS, ctlp->errhp_ctl));

  /* get attributes of the column parameter list */
  OCI_CHECK(ctlp->errhp_ctl, OCI_HTYPE_ERROR, ociret, ctlp,
            OCIAttrGet((CONST dvoid *)ctlp->colLstDesc_ctl,
                       OCI_DTYPE_PARAM,
                       (dvoid *)&parmtyp, (ub4 *)0,
                       OCI_ATTR_PTYPE, ctlp->errhp_ctl));

  if (parmtyp != OCI_PTYPE_LIST)
  {
    fprintf(output_fp, "ERROR: expected parmtyp of OCI_PTYPE_LIST, got %d\n",
            (int)parmtyp);
  }

  /* Now set the attributes of each column by getting a parameter
   * handle on each column, then setting attributes on the parameter
   * handle for the column.
   * Note that positions within a column list descriptor are 1-based.
   */
  for (i = 0, pos = 1, colp = tblp->col_tbl, fldp = tblp->fld_tbl;
       i < tblp->ncol_tbl;
       i++, pos++, colp++, fldp++)
  {
    /* get parameter handle on the column */
    OCI_CHECK(ctlp->errhp_ctl, OCI_HTYPE_ERROR, ociret, ctlp,
              OCIParamGet((CONST dvoid *)ctlp->colLstDesc_ctl,
                          (ub4)OCI_DTYPE_PARAM, ctlp->errhp_ctl,
                          (dvoid **)&colDesc, pos));


    colp->id_col = i;                            /* position in column array */

    /* set external attributes on the column */

    /* column name */
    OCI_CHECK(ctlp->errhp_ctl, OCI_HTYPE_ERROR, ociret, ctlp,
              OCIAttrSet((dvoid *)colDesc, (ub4)OCI_DTYPE_PARAM,
                         (dvoid *)colp->name_col,
                         (ub4)strlen((const char *)colp->name_col),
                         (ub4)OCI_ATTR_NAME, ctlp->errhp_ctl));

    /* column type */
    OCI_CHECK(ctlp->errhp_ctl, OCI_HTYPE_ERROR, ociret, ctlp,
              OCIAttrSet((dvoid *)colDesc, (ub4)OCI_DTYPE_PARAM,
                         (dvoid *)&colp->exttyp_col, (ub4)0,
                         (ub4)OCI_ATTR_DATA_TYPE, ctlp->errhp_ctl));

    /* max data size */
    OCI_CHECK(ctlp->errhp_ctl, OCI_HTYPE_ERROR, ociret, ctlp,
              OCIAttrSet((dvoid *)colDesc, (ub4)OCI_DTYPE_PARAM,
                         (dvoid *)&fldp->maxlen_fld, (ub4)0,
                         (ub4)OCI_ATTR_DATA_SIZE, ctlp->errhp_ctl));

    /* If column is chrdate or date, set column (input field) date mask
     * to trigger client library to check string for a valid date.
     * Note: OCIAttrSet() may be called here w/ a null ptr or null string.
     */
    if (colp->date_col)
    {
      OCI_CHECK(ctlp->errhp_ctl, OCI_HTYPE_ERROR, ociret, ctlp,
                OCIAttrSet((dvoid *)colDesc, (ub4)OCI_DTYPE_PARAM,
                         (dvoid *)colp->datemask_col,
                         (colp->datemask_col) ?
                           (ub4)strlen((const char *)colp->datemask_col) : 0,
                         (ub4)OCI_ATTR_DATEFORMAT, ctlp->errhp_ctl));
    }

    if (colp->prec_col)
    {
      OCI_CHECK(ctlp->errhp_ctl, OCI_HTYPE_ERROR, ociret, ctlp,
                OCIAttrSet((dvoid *)colDesc, (ub4)OCI_DTYPE_PARAM,
                         (dvoid *)&colp->prec_col, (ub4)0,
                         (ub4)OCI_ATTR_PRECISION, ctlp->errhp_ctl));
    }

    if (colp->scale_col)
    {
      OCI_CHECK(ctlp->errhp_ctl, OCI_HTYPE_ERROR, ociret, ctlp,
                OCIAttrSet((dvoid *)colDesc, (ub4)OCI_DTYPE_PARAM,
                         (dvoid *)&colp->scale_col, (ub4)0,
                         (ub4)OCI_ATTR_SCALE, ctlp->errhp_ctl));
    }

    if (colp->csid_col)
    {
      OCI_CHECK(ctlp->errhp_ctl, OCI_HTYPE_ERROR, ociret, ctlp,
                OCIAttrSet((dvoid *)colDesc, (ub4)OCI_DTYPE_PARAM,
                         (dvoid *)&colp->csid_col, (ub4)0,
                         (ub4)OCI_ATTR_CHARSET_ID, ctlp->errhp_ctl));
    }

    /* free the parameter handle to the column descriptor */
    OCI_CHECK((dvoid *)0, 0, ociret, ctlp,
              OCIDescriptorFree((dvoid *)colDesc, OCI_DTYPE_PARAM));
  }

  /* read back some of the attributes for purpose of illustration */
  for (i = 0, pos = 1, colp = tblp->col_tbl, fldp = tblp->fld_tbl;
       i < tblp->ncol_tbl;
       i++, pos++, colp++, fldp++)
  {
    text *s;
    ub4   slen;
    ub4   maxdsz;
    ub2   dty;

    /* get parameter handle on the column */
    OCI_CHECK(ctlp->errhp_ctl, OCI_HTYPE_ERROR, ociret, ctlp,
              OCIParamGet((CONST dvoid *)ctlp->colLstDesc_ctl,
                          (ub4)OCI_DTYPE_PARAM, ctlp->errhp_ctl,
                          (dvoid **)&colDesc, pos));


    /* get column name */
    OCI_CHECK(ctlp->errhp_ctl, OCI_HTYPE_ERROR, ociret, ctlp,
              OCIAttrGet((dvoid *)colDesc,
                         OCI_DTYPE_PARAM,
                         (dvoid *)&s, (ub4 *)&slen,
                         OCI_ATTR_NAME, ctlp->errhp_ctl));

    /* check string length */
    if (slen != (ub4)strlen((const char *)colp->name_col))
    {
      fprintf(output_fp,
        "*** ERROR *** bad col name len in column parameter\n");
      fprintf(output_fp, "\texpected %d, got %d\n",
             (int)strlen((const char *)colp->name_col), (int)slen);
    }

    if (strncmp((const char *)s, (const char *)colp->name_col, (size_t)slen))
    {
      fprintf(output_fp,"*** ERROR *** bad column name in column parameter\n");
      fprintf(output_fp, "\texpected %s, got %s\n",
              (char *)colp->name_col, (char *)s);
    }

    /* get column type */
    OCI_CHECK(ctlp->errhp_ctl, OCI_HTYPE_ERROR, ociret, ctlp,
              OCIAttrGet((dvoid *)colDesc,
                         OCI_DTYPE_PARAM,
                         (dvoid *)&dty, (ub4 *)0,
                         OCI_ATTR_DATA_TYPE, ctlp->errhp_ctl));
    if (dty != colp->exttyp_col)
    {
      fprintf(output_fp,
        "*** ERROR *** bad OCI_ATTR_DATA_TYPE in col param\n");
      fprintf(output_fp, "\tColumn name %s\n", colp->name_col);
      fprintf(output_fp, "\t\texpected %d, got %d\n",
              (int)colp->exttyp_col, (int)dty);
    }

    /* get the max data size */
    OCI_CHECK(ctlp->errhp_ctl, OCI_HTYPE_ERROR, ociret, ctlp,
              OCIAttrGet((dvoid *)colDesc,
                         OCI_DTYPE_PARAM,
                         (dvoid *)&maxdsz, (ub4 *)0,
                         OCI_ATTR_DATA_SIZE, ctlp->errhp_ctl));
    if (maxdsz != fldp->maxlen_fld)
    {
      fprintf(output_fp,
        "*** ERROR *** bad OCI_ATTR_DATA_SIZE in col param\n");
      fprintf(output_fp, "\tColumn name %s\n", colp->name_col);
      fprintf(output_fp, "\t\texpected %d, got %d\n",
              (int)fldp->maxlen_fld, (int)maxdsz);
    }

    /* free the parameter handle to the column descriptor */
    OCI_CHECK((dvoid *)0, 0, ociret, ctlp,
              OCIDescriptorFree((dvoid *)colDesc, OCI_DTYPE_PARAM));
  }

  {
    char *vbuf;

    OCI_CHECK(ctlp->errhp_ctl, OCI_HTYPE_ERROR, ociret, ctlp,
              OCIAttrSet((dvoid *)dpctx, (ub4)OCI_HTYPE_DIRPATH_CTX,
                         (dvoid *)&tblp->xfrsz_tbl,
                         (ub4)0, (ub4)OCI_ATTR_BUF_SIZE, ctlp->errhp_ctl));

    /* minimize read system calls */
    vbuf = (char *)malloc((size_t)tblp->xfrsz_tbl);
    if (vbuf != (char *)0)
      (void)setvbuf(stdin, vbuf, _IOFBF, (size_t)tblp->xfrsz_tbl);
  }

  /* prepare the load */
  OCI_CHECK(ctlp->errhp_ctl, OCI_HTYPE_ERROR, ociret, ctlp,
            OCIDirPathPrepare(dpctx, ctlp->svchp_ctl, ctlp->errhp_ctl));

  /* Allocate column array and stream handles.
   * Note that for the column array and stream handles
   * the parent handle is the direct path context.
   * Also note that Oracle errors are returned via the
   * environment handle associated with the direct path context.
   */
  OCI_CHECK(ctlp->envhp_ctl, OCI_HTYPE_ENV, ociret, ctlp,
            OCIHandleAlloc((dvoid *)ctlp->dpctx_ctl, (dvoid **)&ctlp->dpca_ctl,
                           (ub4)OCI_HTYPE_DIRPATH_COLUMN_ARRAY,
                           (size_t)0, (dvoid **)0));

  OCI_CHECK(ctlp->envhp_ctl, OCI_HTYPE_ENV, ociret, ctlp,
            OCIHandleAlloc((dvoid *)ctlp->dpctx_ctl,(dvoid **)&ctlp->dpstr_ctl,
                           (ub4)OCI_HTYPE_DIRPATH_STREAM,
                           (size_t)0, (dvoid **)0));

  /* get number of rows in the column array just allocated */
  OCI_CHECK(ctlp->errhp_ctl, OCI_HTYPE_ERROR, ociret, ctlp,
            OCIAttrGet((CONST dvoid *)(ctlp->dpca_ctl),
                       OCI_HTYPE_DIRPATH_COLUMN_ARRAY,
                       (dvoid *)(&ctlp->nrow_ctl), (ub4 *)0,
                       OCI_ATTR_NUM_ROWS, ctlp->errhp_ctl));

  /* get number of columns in the column array just allocated */
  OCI_CHECK(ctlp->errhp_ctl, OCI_HTYPE_ERROR, ociret, ctlp,
            OCIAttrGet((CONST dvoid *)(ctlp->dpca_ctl),
                       OCI_HTYPE_DIRPATH_COLUMN_ARRAY,
                       (dvoid *)(&ctlp->ncol_ctl), (ub4 *)0,
                       OCI_ATTR_NUM_COLS, ctlp->errhp_ctl));


  /* allocate buffer for input records */
  ctlp->inbuf_ctl = (ub1 *)malloc(ctlp->nrow_ctl * sessp->maxreclen_sess);
  if (ctlp->inbuf_ctl == (ub1 *)0)
  {
    perror("malloc");
    FATAL("init_load:malloc:inbuf_ctl alloc failure",
          ctlp->nrow_ctl * sessp->maxreclen_sess);
  }

  /* allocate Offset-TO-Record number mapping array */
  ctlp->otor_ctl = (ub4 *)malloc(ctlp->nrow_ctl * sizeof(ub4));
  if (ctlp->otor_ctl == (ub4 *)0)
  {
    perror("malloc");
    FATAL("init_load:malloc:otor_ctl alloc failure",
          ctlp->nrow_ctl * sizeof(ub4));
  }

  CLEAR_PCTX(ctlp->pctx_ctl);                  /* initialize partial context */

/*
  fprintf(output_fp, "init_load: %ld column array rows\n",
          (long)ctlp->nrow_ctl);
*/

  return;
}

/*------------------------------ simple_load ------------------------------*/
/*
  This function reads input records from 'inputfp', parses the input
  records into fields according to the field description given by
  tblp->fld_tbl, and loads the data into the database.

  LOBs can be loaded with this function in a piecewise manner.  This
  function is written as a state machine, which cycles through the
  following states:
    RESET, GET_RECORD, FIELD_SET, DO_CONVERT, DO_LOAD, END_OF_INPUT

  The normal case of all scalar data, where multiple records fit
  entirely in memory, cycles through the following states:
    RESET, [[GET_RECORD, FIELD_SET]+, DO_CONVERT, DO_LOAD]+, RESET

  The case of loading one or more LOB columns, which do not fit entirely
  in memory, has the following state transitions:
    RESET, GET_RECORD, [FIELD_SET, DO_CONVERT, DO_LOAD]+, RESET
  Note that the second and subsequent transitions to the FIELD_SET
  state have a partial record context.

  A mapping of column array offset to input record number (otor_ctl[])
  is maintained by this function for error reporting and recovery.
 */
STATICF void
simple_load(ctlp, tblp, sessp, inputfp)
struct loadctl *ctlp;
struct tbl     *tblp;
struct sess    *sessp;
FILE           *inputfp;
{
  sword  fsetrv;                              /* return value from field_set */
  sword  cvtrv;                              /* return value from do_convert */
  sword  ldrv;                                  /* return value from do_load */
  ub4    startoff;                     /* starting row offset for conversion */
  ub4    nxtLoadOff;                /* column array offset to be loaded next */
  ub4    rowCnt;                  /* count of rows populated in column array */
  ub4    cvtCnt;                                  /* count of rows converted */
  ub4    lastoff;                    /* last row offset used in column array */
  sword  state;                               /* current state machine state */
  sword  done;                          /* set to TRUE when load is complete */
  ub4    input_recnum;                        /* current input record number */
  ub4    load_recnum;   /* record number corresponding to last record loaded */
  ub4    err_recnum;                 /* record number corresponding to error */
  text   *recp;
  ub4    cvtcontcnt;                 /* # of times CONVERT_CONTINUE returned */

  /* set initial state */
  input_recnum = 0;
  load_recnum  = UB4MAXVAL;
  err_recnum   = 0;
  state        = RESET;
  fsetrv       = FIELD_SET_COMPLETE;
  cvtrv        = CONVERT_SUCCESS;
  ldrv         = LOAD_SUCCESS;
  done         = FALSE;
  cvtcontcnt   = 0;

  while (!done)
  {
    switch (state)
    {
    case RESET:    /* Reset column array and direct stream state to be empty */
    {
      startoff   = 0;             /* reset starting offset into column array */
      lastoff    = 0;                      /* last entry set of column array */
      rowCnt     = 0;                  /* count of rows partial and complete */
      cvtCnt     = 0;                             /* count of converted rows */
      nxtLoadOff = 0;

      /* Reset column array state in case a previous conversion needed
       * to be continued, or a row is expecting more data.
       */
      (void) OCIDirPathColArrayReset(ctlp->dpca_ctl, ctlp->errhp_ctl);

      /* Reset the stream state since we are starting a new stream
       * (i.e. don't want to append to existing data in the stream.)
       */
      (void) OCIDirPathStreamReset(ctlp->dpstr_ctl,  ctlp->errhp_ctl);

      state = GET_RECORD;                     /* get some more input records */
      /* FALLTHROUGH */
    }

    case GET_RECORD:
    {
      assert(lastoff < ctlp->nrow_ctl);                /* array bounds check */

      recp = (text *)(ctlp->inbuf_ctl + (lastoff * sessp->maxreclen_sess));

      if (fgets((char *)recp, (int)sessp->maxreclen_sess, inputfp)
                != (char *)NULL)
      {
        /* set column array offset to input record number map */
        ctlp->otor_ctl[lastoff] = ++input_recnum;
        if ((input_recnum % 10000) == 0)
          fprintf(output_fp, "record number: %d\n", (int)input_recnum);
        state = FIELD_SET;
        /* FALLTHROUGH */
      }
      else
      {
        if (lastoff)
          lastoff--;
        state = END_OF_INPUT;
        break;
      }
    }

    case FIELD_SET:
    {
      /* map input data fields to DB columns, set column array entries */
      fsetrv = field_set(ctlp, tblp, recp, lastoff);
      rowCnt = lastoff + 1;

      if (rowCnt == ctlp->nrow_ctl || fsetrv != FIELD_SET_COMPLETE)
      {
        /* array is full, or have a large partial column, or the
         * secondary buffer is in use by an OUTOFLINE field.
         */
        state = DO_CONVERT;
        /* FALLTHROUGH */
      }
      else
      {
        lastoff++;                             /* use next column array slot */
        state = GET_RECORD;                               /* get next record */
        break;
      }
    }

    case DO_CONVERT:
    {
      /* Either one of the following is true:
       * - the column array is full
       * - there is a large partial column
       * - the secondary buffer used by field_set() is in use
       * - previous conversion returned CONVERT_CONTINUE and
       *   now the conversion is being resumed.
       *
       * In any case, convert and load the data.
       */
      ub4    cvtBadRoff;                   /* bad row offset from conversion */
      ub2    cvtBadCoff;                /* bad column offset from conversion */

      while (startoff <= lastoff)
      {
        ub4 cvtCntPerCall = 0;   /* rows converted in one call to do_convert */

        /* note that each call to do_convert() will convert all contiguous rows
         * in the colarray until it hit a row in error while converting.
         */
        cvtrv = do_convert(ctlp, startoff, rowCnt, &cvtCntPerCall,
                           &cvtBadCoff);
        cvtCnt += cvtCntPerCall; /* sum of rows converted so far in colarray */
        if (cvtrv == CONVERT_SUCCESS)
        {
          /* One or more rows converted successfully, break
           * out of the conversion loop and load the rows.
           */
          assert(cvtCntPerCall > 0);
          state = DO_LOAD;
          break;
        }
        else if (cvtrv == CONVERT_ERROR)
        {
          /* Conversion error.  Reject the bad record and
           * continue on with the next record (if any).
           * cvtBadRoff is the 0-based index of the bad row in
           * the column array.  cvtBadCoff is the 0-based index
           * of the bad column (of the bad row) in the column
           * array.
           */
          assert(cvtCntPerCall >= 0);
          cvtBadRoff = startoff + cvtCntPerCall;
          err_recnum = ctlp->otor_ctl[cvtBadRoff];    /* map to input_recnum */
          fprintf(output_fp, "Conversion Error on record %d, column %d\n",
                           (int)err_recnum, (int)cvtBadCoff + 1);

          /* print err msg txt */
          errprint((dvoid *)(ctlp->errhp_ctl), OCI_HTYPE_ERROR,
                   (sb4 *)0);

         /* Check to see if the conversion error occurred on a
          * continuation of a partially loaded row.
          * If so, either (a) flush the partial row from the server, or
          * (b) mark the column as being 0 length and complete.
          * In the latter case (b), any data already loaded into the column
          * from a previous LoadStream call remains, and we can continue
          * field setting, conversion and loading with the next column.
          * Here, we implement (a), and flush the row from the server.
          */
          if (err_recnum == load_recnum)
          {
            /* Conversion error occurred on record which has been
             * partially loaded (by a previous stream).
             * XXX May be better to have an attribute of the direct path
             * XXX context which indicates that the last row loaded was
             * XXX partial.
             *
             * Flush the output pipe.  Note that on conversion error,
             * no part of the row data for the row in error makes it
             * into the stream buffer.
             * Here we flush the partial row from the server.  The
             * stream state is reset if no rows are successfully
             * converted.
             */

            /* flush partial row from server */
            (void) OCIDirPathFlushRow(ctlp->dpctx_ctl, ctlp->errhp_ctl);
          }

          if (cvtBadRoff == lastoff)
          {
            /* Conversion error occurred on the last populated slot
             * of the column array.
             * Flush the input stream of any data for this row,
             * and re-use this slot for another input record.
             */
            field_flush(ctlp, lastoff);
            state    = GET_RECORD;
            startoff = cvtBadRoff;              /* only convert the last row */
            rowCnt = 0;    /* already tried converting all rows in col array */
            assert(startoff <= lastoff);
            break;
          }
          else
          {
            /* Skip over bad row and continue conversion with next row.
             * We don't attempt to fill in this slot with another record.
             */
            startoff = cvtBadRoff + 1;
            assert(startoff <= lastoff);
            continue;
          }
        }
        else if (cvtrv == CONVERT_NEED_DATA)      /* partial col encountered */
        {
          /* Partial (large) column encountered, load the piece
           * and loop back up to field_set to get the rest of
           * the partial column.
           * startoff is set to the offset into the column array where
           * we need to resume conversion from, which should be the
           * last entry that we set (lastoff).
           */
          state    = DO_LOAD;

          /* Set our row position in column array to resume
           * conversion at when DO_LOAD transitions to DO_CONVERT.
           */
          assert(cvtCntPerCall >= 0);
          startoff = startoff + cvtCntPerCall;
          assert(startoff == lastoff);
          break;
        }
        else if (cvtrv == CONVERT_CONTINUE)
        {
          /* The stream buffer is full and there is more data in
           * the column array which needs to be converted.
           * Load the stream (DO_LOAD) and transition back to
           * DO_CONVERT to convert the remainder of the column array,
           * without calling the field setting function in between.
           * The sequence {DO_CONVERT, DO_LOAD} may occur many times
           * for a long row or column.
           * Note that startoff becomes the offset into the column array
           * where we need to resume conversion from.
           */
          cvtcontcnt++;
          state    = DO_LOAD;

          /* Set our row position in column array (startoff) to
           * resume conversion at when we transition from the
           * DO_LOAD state back to DO_CONVERT.
           */
          assert(cvtCntPerCall >= 0);
          startoff = startoff + cvtCntPerCall;
          assert(startoff <= lastoff);

          break;
        }
      }                                                         /* end while */
      break;
    }

    case DO_LOAD:
    {
      ub4    loadCnt;                     /* count of rows loaded by do_load */

      ldrv       = do_load(ctlp, &loadCnt);
      nxtLoadOff = nxtLoadOff + loadCnt;

      switch (ldrv)
      {
      case LOAD_SUCCESS:
      {
        /* The stream has been loaded successfully.  What we do next
         * depends on the result of the previous conversion step.
         */
        load_recnum = ctlp->otor_ctl[nxtLoadOff - 1];
        if (cvtrv == CONVERT_SUCCESS || cvtrv == CONVERT_ERROR)
        {
          /* The column array was successfully converted (or the
           * last row was in error).
           * Fill up another array with more input records.
           */
          state = RESET;
        }
        else if (cvtrv == CONVERT_CONTINUE)
        {
          /* There is more data in column array to convert and load. */
          state    = DO_CONVERT;

          /* Note that when do_convert returns CONVERT_CONTINUE that
           * startoff was set to the row offset into the column array
           * of where to resume conversion.  The loadCnt returned by
           * OCIDirPathLoadStream is the number of rows successfully
           * loaded.
           * Do a sanity check on the attributes here.
           */
          if (startoff != nxtLoadOff)                              /* sanity */
            fprintf(output_fp, "LOAD_SUCCESS/CONVERT_CONTINUE: %ld:%ld\n",
                    (long)nxtLoadOff, startoff);

          /* Reset the direct stream state so conversion starts at
           * the beginning of the stream.
           */
          (void) OCIDirPathStreamReset(ctlp->dpstr_ctl, ctlp->errhp_ctl);
        }
        else
        {
          /* Note that if the previous conversion step returned
           * CONVERT_NEED_DATA then the load step would have returned
           * LOAD_NEED_DATA too (not LOAD_SUCCESS).
           */
          FATAL("DO_LOAD:LOAD_SUCCESS: unexpected cvtrv", cvtrv);
        }
        break;
      }

      case LOAD_ERROR:
      {
        sb4  oraerr;
        ub4  badRowOff;

        badRowOff   = nxtLoadOff;
        nxtLoadOff += 1;                              /* account for bad row */
        err_recnum      = ctlp->otor_ctl[badRowOff];  /* map to input_recnum */
        fprintf(output_fp, "Error on record %ld\n", (long)err_recnum);

        /* print err msg txt */
        errprint((dvoid *)(ctlp->errhp_ctl), OCI_HTYPE_ERROR, &oraerr);

        /* On a load error, all rows up to the row in error are loaded.
         * account for that here by setting load_recnum only when some
         * rows have been loaded.
         */
        if (loadCnt != 0)
          load_recnum = err_recnum - 1;

        if (oraerr == OER(600))
          FATAL("DO_LOAD:LOAD_ERROR: server internal error", oraerr);

        if (err_recnum == input_recnum)
        {
          /* Error occurred on last input row, which may or may not
           * be in a partial state. Flush any remaining input for
           * the bad row.
           */
          field_flush(ctlp, badRowOff);
        }

        if (err_recnum == load_recnum)
        {
          /* Server has part of this row already, flush it */
          (void) OCIDirPathFlushRow(ctlp->dpctx_ctl, ctlp->errhp_ctl);
        }

        if (badRowOff == lastoff)
        {
          /* Error occurred on the last entry in the column array,
           * go process more input records and set up another array.
           */
          state = RESET;
        }
        else
        {
          /* Otherwise, continue loading this stream.  Note that the
           * stream positions itself to the next row on error.
           */
          state    = DO_LOAD;
        }
        break;
      }

      case LOAD_NEED_DATA:
      {
        load_recnum = ctlp->otor_ctl[nxtLoadOff];
        if (cvtrv == CONVERT_NEED_DATA)
          state = FIELD_SET;                         /* need more input data */
        else if (cvtrv == CONVERT_CONTINUE)
          state = DO_CONVERT;   /* have input data, continue with conversion */
        else
          FATAL("DO_LOAD:LOAD_NEED_DATA: unexpected cvtrv", cvtrv);

        /* Reset the direct stream state so conversion starts at
         * the beginning of the stream.
         */
        (void) OCIDirPathStreamReset(ctlp->dpstr_ctl, ctlp->errhp_ctl);
        break;
      }

      case LOAD_NO_DATA:
      {
        /* Attempt to either load an empty stream, or a stream
         * which has been completely processed.
         */
        if (cvtrv == CONVERT_CONTINUE)
        {
          /* Reset stream state so we convert into an empty stream buffer. */
          (void) OCIDirPathStreamReset(ctlp->dpstr_ctl,  ctlp->errhp_ctl);
          state = DO_CONVERT;           /* convert remainder of column array */
        }
        else
          state = RESET;                      /* get some more input records */
        break;
      }

      default:
        FATAL("DO_LOAD: unexpected return value", ldrv);
        break;
      }
      break;
    }

    case END_OF_INPUT:
    {
      if (cvtCnt)
        state = DO_LOAD; /* deal with data already converted, but not loaded */
      else if (rowCnt)
        state = DO_CONVERT; /* deal with a partially populated column array */
      else
        done = TRUE;
      break;
    }

    default:
      FATAL("SIMPLE_LOAD: unexpected state", state);
      break;
    }                                                  /* end switch (state) */
  }

/*
  fprintf(output_fp, "do_convert returned CONVERT_CONTINUE %ld times\n",
          (long)cvtcontcnt);
*/

  fprintf(output_fp, "Number of input records processed = %ld\n",
          (long)input_recnum);
}

STATICF void
finish_load(ctlp)
struct loadctl *ctlp;
{
  sword ociret;                                 /* return code from OCI call */

  /* Execute load finishing logic without freeing server data structures
   * related to the load.
   */
  OCI_CHECK(ctlp->errhp_ctl, OCI_HTYPE_ERROR, ociret, ctlp,
            OCIDirPathDataSave(ctlp->dpctx_ctl, ctlp->errhp_ctl,
                               (ub4)OCI_DIRPATH_DATASAVE_FINISH));

  /* free up server data structures for the load. */
  OCI_CHECK(ctlp->errhp_ctl, OCI_HTYPE_ERROR, ociret, ctlp,
            OCIDirPathFinish(ctlp->dpctx_ctl, ctlp->errhp_ctl));
}

/*------------------------------ do_convert ------------------------------*/
/*
 NAME:     do_convert

 FUNCTION: Convert the data in the column array to stream format.

 RETURNS:
   CONVERT_SUCCESS:
     All data in the column array has been successfully converted.
     *cvtCntp is the number of rows successfully converted.
   CONVERT_ERROR:
     Conversion error occurred on the row after the last successfully
     converted row.
     Client Action:
       Continue converting the column array by calling this function
       again with startoff adjusted to skip over the row in error.

   CONVERT_NEED_DATA:
     All data in the column array has been converted, but the last
     column processed was marked as a partial.
   CONVERT_CONTINUE:
     Not all of the data in the column array has been converted due to
     lack of space in the stream buffer.
     Client Action:
       Load the converted stream data, reset the stream, and call this
       function again without modifying the column array and setting
       startoff to the appropriate position in the array.
 */
sword
do_convert(ctlp, startoff, rowcnt, cvtCntp, badcoffp)
struct loadctl *ctlp;             /* pointer to control structure   (IN/OUT) */
ub4             rowcnt;           /* number of rows in column array     (IN) */
ub4             startoff;     /* starting row offset into column array  (IN) */
ub4            *cvtCntp;       /* count of rows successfully converted (OUT) */
ub2            *badcoffp;   /* column offset into col array of bad col (OUT) */
{
  sword retval = CONVERT_SUCCESS;
  sword ocierr, ocierr2;
  ub2   badcol = 0;

  *cvtCntp = 0;

  if (startoff >= rowcnt)
    FATAL("DO_CONVERT: bad startoff", startoff);

  if (rowcnt)
  {
    /* convert array to stream, filter out bad records */
    ocierr = OCIDirPathColArrayToStream(ctlp->dpca_ctl, ctlp->dpctx_ctl,
                                        ctlp->dpstr_ctl, ctlp->errhp_ctl,
                                        rowcnt, startoff);
    switch (ocierr)
    {
    case OCI_SUCCESS:        /* everything succesfully converted to stream */
      retval = CONVERT_SUCCESS;
      break;

    case OCI_ERROR:            /* some error, most likely a conversion error */
      /* Tell the caller that a conversion error occurred along
       * with the number of rows successfully converted (*cvtCntp).
       * Note that the caller is responsible for adjusting startoff
       * accordingly and calling us again to resume conversion of
       * the remaining rows.
       */
      retval  = CONVERT_ERROR;                         /* conversion error */
      break;

    case OCI_CONTINUE:                              /* stream buffer is full */
      /* The stream buffer could not contain all of the data in
       * the column array.
       * The client should load the converted data, and loop
       * back to convert the remaining data in the column array.
       */
      retval  = CONVERT_CONTINUE;
      break;

    case OCI_NEED_DATA:                        /* partial column encountered */
      /* Everything converted, but have a partial column.
       * Load this stream, and return to caller for next piece.
       */
      retval = CONVERT_NEED_DATA;
      break;

    default:                                 /* unexpected OCI return value! */
      FATAL("do_convert:OCIDirPathColArrayToStream:Unexpected OCI return code",
             ocierr);
      /* NOTREACHED */
      break;
    }

    OCI_CHECK(ctlp->errhp2_ctl, OCI_HTYPE_ERROR, ocierr2, ctlp,
              OCIAttrGet((CONST dvoid *)ctlp->dpca_ctl,
                         OCI_HTYPE_DIRPATH_COLUMN_ARRAY,
                         (dvoid *)(cvtCntp), (ub4 *)0,
                         OCI_ATTR_ROW_COUNT, ctlp->errhp2_ctl));

    OCI_CHECK(ctlp->errhp2_ctl, OCI_HTYPE_ERROR, ocierr2, ctlp,
              OCIAttrGet((CONST dvoid *)ctlp->dpca_ctl,
                         OCI_HTYPE_DIRPATH_COLUMN_ARRAY,
                         (dvoid *)(&badcol), (ub4 *)0,
                         OCI_ATTR_COL_COUNT, ctlp->errhp2_ctl));
  }

  *badcoffp = badcol;

  return retval;
}

/*------------------------------ do_load ------------------------------*/
/*
 NAME:     do_load

 FUNCTION: Load a direct path stream.

 PARAMETERS:
   ctlp:     Pointer to control structure.  Note that errhp_ctl
             contains error information when an error is returned
             from this function.

   loadCntp: Count of rows loaded on this call.

 RETURNS:
   LOAD_SUCCESS:
     All data loaded succesfully.
     Client Action:
       Supply another stream and call again, or finish the load.

   LOAD_ERROR:
     Error while loading occured.  *loadCntp is the number of
     rows successfully loaded this call.
     Client Action:
       Use *loadCntp to compute current column array position and
       map the column array position to the input record and reject
       the record.

       if (this is a continuation of a row)
       {
         /o server has data for this row buffered o/
         flush the row data
       }

       if (end-of-stream has not been reached)
       {
         call this function again,
         stream loading will resume with the next row in the stream.
       }
       else if (end-of-stream has been reached)
       {
         build another stream and call this function again,
         or finish the load.
       }

   LOAD_NEED_DATA:
     Last row was not complete.
     Client Action:
       Caller needs to supply more data for the row (a column is
       being pieced.)  Note that the row offset can be determined
       by either the cvtCnt returned from do_convert, or from the
       loadCntp returned by do_load.  The column offset for the
       column being pieced is available as an attribute of
       the column array.
 */
sword
do_load(ctlp, loadCntp)
struct loadctl *ctlp;             /* pointer to control structure   (IN/OUT) */
ub4            *loadCntp;                     /* number of rows loaded (OUT) */
{
  sword ocierr;                                          /* OCI return value */
  sword retval;                           /* return value from this function */
  sword getRowCnt = FALSE;                       /* return row count if TRUE */

  if (loadCntp != (ub4 *)0)
  {
    *loadCntp = 0;
    getRowCnt = TRUE;
  }

  /* Load the stream.
   * Note that the position in the stream is maintained internally to
   * the stream handle, along with offset information for the column
   * array which produced the stream.  When the conversion to stream
   * format is done, the data is appended to the stream.  It is the
   * responsibility of the caller to reset the stream when appropriate.
   * On errors, the position is moved to the next row, or the end of
   * the stream if the error occurs on the last row.  The next LoadStream
   * call will start on the next row, if any.
   * If a LoadStream call is made, and end of stream has been reached,
   * OCI_NO_DATA is returned.
   */

#if 1
  ocierr = OCIDirPathLoadStream(ctlp->dpctx_ctl, ctlp->dpstr_ctl,
                                ctlp->errhp_ctl);
#else
  {
    ub1 *bufaddr;
    ub4  buflen;

    OCI_CHECK(ctlp->errhp_ctl, OCI_HTYPE_ERROR, ocierr, ctlp,
              OCIAttrGet((CONST dvoid *)(ctlp->dpstr_ctl),
                         OCI_HTYPE_DIRPATH_STREAM,
                         (dvoid *)&bufaddr, (ub4 *)0,
                         OCI_ATTR_BUF_ADDR, ctlp->errhp_ctl));
    OCI_CHECK(ctlp->errhp_ctl, OCI_HTYPE_ERROR, ocierr, ctlp,
              OCIAttrGet((CONST dvoid *)(ctlp->dpstr_ctl),
                         OCI_HTYPE_DIRPATH_STREAM,
                         (dvoid *)&buflen, (ub4 *)0,
                         OCI_ATTR_BUF_SIZE, ctlp->errhp_ctl));
    write(1, (char *)bufaddr, (int)buflen);
    fprintf(output_fp, "Wrote %d bytes from stream\n", (int)buflen);
    getRowCnt = FALSE;
  }
#endif

  switch (ocierr)
  {
    case OCI_SUCCESS:
      /* all data succcesfully loaded */
      retval    = LOAD_SUCCESS;
      break;

    case OCI_ERROR:
      /* Error occurred while loading: could be a partition mapping
       * error, null constraint violation, or an out of space
       * condition.  In any case, we return the number of rows
       * processed (successfully loaded).
       */
      retval    = LOAD_ERROR;
      break;

    case OCI_NEED_DATA:
      /* Last row was not complete.
       * The caller needs to supply another piece.
       */
      retval    = LOAD_NEED_DATA;
      break;

    case OCI_NO_DATA:
      /* the stream was empty */
      retval    = LOAD_NO_DATA;
      break;

    default:
      FATAL("do_load:OCIDirPathLoadStream:Unexpected OCI return code", ocierr);
      /* NOTREACHED */
      break;
  }

  if (getRowCnt)
  {
    sword  ocierr2;

    OCI_CHECK(ctlp->errhp2_ctl, OCI_HTYPE_ERROR, ocierr2, ctlp,
              OCIAttrGet((CONST dvoid *)(ctlp->dpstr_ctl),
                         OCI_HTYPE_DIRPATH_STREAM,
                         (dvoid *)loadCntp, (ub4 *)0,
                         OCI_ATTR_ROW_COUNT, ctlp->errhp2_ctl));
  }

  return retval;
}


/*------------------------------ field_flush ------------------------------*/
/*
 NAME      field_flush

 FUNCTION: Helper function which cleans up the partial context
           state, and clears it.
 */
STATICF void
field_flush(ctlp, rowoff)
struct loadctl *ctlp;
ub4             rowoff;
{
  if (ctlp->pctx_ctl.valid_pctx)
  {
    /* Partial context is valid; make sure the request is
     * for the context corresponding to the current row.
     */
    assert(rowoff == ctlp->pctx_ctl.row_pctx);
    (void) close(ctlp->pctx_ctl.fd_pctx);
    free((void *)ctlp->pctx_ctl.fnm_pctx);
  }
  CLEAR_PCTX(ctlp->pctx_ctl);
}


/*------------------------------ field_set ------------------------------*/
/*
 NAME:     field_set

 FUNCTION: Set the input data fields to their corresponding
           data columns.

   Simple field setting.
   Computes address and length of fields in the input record,
   and sets the corresponding column array entry for each
   input field.

   This function only deals with positional fields.

   Leading white space is trimmed from the field if FLD_STRIP_LEAD_BLANK
   is set.

   Trailing white space is trimmed from the field if FLD_STRIP_TRAIL_BLANK
   is set.

   Fields which consist of all spaces are loaded as null columns.

   Fields which are marked as FLD_OUTOFLINE are interpreted
   as being file names which can be passed directly to an
   open system call.

 RETURNS:
   FIELD_SET_COMPLETE:
     All fields are complete, the partial context is not valid.

   FIELD_SET_BUF
     All fields are complete, the partial context is not valid, but
     data is buffered in a secondary buffer and the column array has
     one or more pointers into the secondary buffer.  The caller
     must convert the column array to stream format before calling
     this function again.

   FIELD_SET_PARTIAL:
     A field is in the partial state, the partial context is valid
     and is required to continue processing the field.  Note that
     when a field is partial, the row which contains the column
     corresponding to the field is partial also.

   FIELD_SET_ERROR:
     A read error occured on a secondary (out-of-line) data file.

 NOTES:
    Discuss how partials are handled.
 */

/* Deal with WIN32 CR-LF weirdness */
#if defined(WIN32COMMON) || defined(WIN32) || defined(_WIN32)
#define TKPIDRV_OPEN_MODE (O_RDONLY | O_BINARY)
#else
#define TKPIDRV_OPEN_MODE (O_RDONLY)
#endif

STATICF sword
field_set(ctlp, tblp, recp, rowoff)
struct loadctl    *ctlp;                           /* load control structure */
struct tbl        *tblp;                                 /* table descriptor */
text              *recp;                                     /* input record */
ub4                rowoff;                        /* column array row offset */
{
  ub1  *cval;
  ub4   thiscol;
  ub4   clen, j;                                            /* column length */
  ub1   cflg;
  sword ociret;
  int   fd;                          /* file descriptor for out-of-line data */
  char *filename;                           /* filename for out-of-line data */
  sword  partial;

  ctlp->bufoff_ctl = 0;

  if ((partial = (sword)ctlp->pctx_ctl.valid_pctx) == TRUE)
  {
    /* partial context is valid; resume where we left off */
    assert(rowoff == ctlp->pctx_ctl.row_pctx);
    thiscol = ctlp->pctx_ctl.col_pctx;
  }
  else
    thiscol = 0;

  for (/* empty */; thiscol < tblp->ncol_tbl; thiscol++)
  {
    struct col *colp =  &tblp->col_tbl[thiscol];
    struct fld *fldp =  &tblp->fld_tbl[thiscol];

    if (partial)
    {
      /* partials are always from a secondary file */
      fd       = ctlp->pctx_ctl.fd_pctx;
      filename = ctlp->pctx_ctl.fnm_pctx;
    }
    else                                                         /* !partial */
    {
      fd       = -1;
      filename = (char *)0;
      cval     = (ub1 *)recp + fldp->begpos_fld - 1;
      clen     = fldp->endpos_fld - fldp->begpos_fld + 1;

      j = 0;
      if (bit(fldp->flag_fld, FLD_STRIP_LEAD_BLANK))
      {
        /* trim leading white space */
        for (/*empty*/; j < clen; j++)
          if (!isspace((int)cval[j]))
            break;
      }

      if (j >= clen)
        clen = 0;                              /* null column, handled below */
      else
      {
        if (bit(fldp->flag_fld, FLD_STRIP_TRAIL_BLANK))
        {
          /* trim trailing white space */
          while (clen && isspace((int)cval[clen - 1]))
            clen--;
        }
        cval = cval + j;
        clen = clen - j;
      }


      if (clen)
      {
        if (bit(fldp->flag_fld, FLD_INLINE))
        {
          cflg = OCI_DIRPATH_COL_COMPLETE;
        }
        else if (bit(fldp->flag_fld, FLD_OUTOFLINE))
        {
          filename = (char *)malloc((size_t)clen+1);
          if (!filename)
          {
            perror("malloc");
            FATAL("field_set: cannot malloc buf for filename", (clen + 1));
          }
          (void) memcpy((dvoid *)filename, (dvoid *)cval, (size_t)clen);
          filename[clen] = 0;
          fd = open(filename, TKPIDRV_OPEN_MODE);
          SET_PCTX(ctlp->pctx_ctl, rowoff, thiscol, (ub4)0, fd, filename);
          LEN_PCTX(ctlp->pctx_ctl) = 0;
        }
        else
        {
          FATAL("field_set: unknown field type", fldp->flag_fld);
        }
      }
      else
      {
        cflg = OCI_DIRPATH_COL_NULL;               /* all spaces become null */
        cval = (ub1 *)0;
      }
    }

    if (bit(fldp->flag_fld, FLD_OUTOFLINE))
    {
      char *buf;
      ub4   bufsz;
      int   cnt;

      if (!ctlp->buf_ctl)
      {
        ctlp->buf_ctl   = (ub1 *)malloc((size_t)SECONDARY_BUF_SIZE);
        ctlp->bufsz_ctl = SECONDARY_BUF_SIZE;
      }

      if ((ctlp->bufsz_ctl - ctlp->bufoff_ctl) > SECONDARY_BUF_SLOP)
      {
        buf   = (char *)ctlp->buf_ctl + ctlp->bufoff_ctl;  /* buffer pointer */
        bufsz = (int)ctlp->bufsz_ctl  - ctlp->bufoff_ctl;     /* buffer size */

        if (fd == -1)
          cnt = 0;
        else
          cnt = read(fd, buf, bufsz);

        if (cnt != -1)
        {
          cval = (ub1 *)buf;
          clen = (ub4)cnt;

          if (cnt < bufsz)                    /* all file data has been read */
          {
            /* mark column as null or complete */
            if (cnt == 0 && LEN_PCTX(ctlp->pctx_ctl) == 0)
              cflg = OCI_DIRPATH_COL_NULL;
            else
              cflg = OCI_DIRPATH_COL_COMPLETE;

            field_flush(ctlp, rowoff);          /* close file, free filename */

            /* adjust offset into buffer for use by next field */
            ctlp->bufoff_ctl += cnt;
          }
          else
            cflg  = OCI_DIRPATH_COL_PARTIAL;
        }
        else
        {
          /* XXX: do something on read failure, like return an error context */
          field_flush(ctlp, rowoff);          /* close file, free filename */
          return FIELD_SET_ERROR;
        }
      }
      else
      {
        /* no room in secondary buffer, return a 0 length partial
         * and pick it up next time.
         */
        cflg = OCI_DIRPATH_COL_PARTIAL;
        clen = 0;
        cval = (ub1 *)NULL;
      }
    }

    OCI_CHECK(ctlp->errhp_ctl, OCI_HTYPE_ERROR, ociret, ctlp,
              OCIDirPathColArrayEntrySet(ctlp->dpca_ctl, ctlp->errhp_ctl,
                                         rowoff, colp->id_col,
                                         cval, clen, cflg));

    if (cflg == OCI_DIRPATH_COL_PARTIAL)
    {
      /* Partials only occur for OutOfLine data
       * remember the row offset, column offset,
       * total length of the column so far,
       * and file descriptor to get data from on
       * subsequent calls to this function.
       */
      LEN_PCTX(ctlp->pctx_ctl) += clen;
      return FIELD_SET_PARTIAL;
    }
  }

  CLEAR_PCTX(ctlp->pctx_ctl);
  if (ctlp->bufoff_ctl)             /* data in secondary buffer for this row */
    return FIELD_SET_BUF;
  else
    return FIELD_SET_COMPLETE;
}


STATICF void
errprint(errhp, htype, errcodep)
dvoid  *errhp;
ub4     htype;
sb4    *errcodep;
{
  text errbuf[512];

  if (errhp)
  {
    sb4  errcode;

    if (errcodep == (sb4 *)0)
      errcodep = &errcode;

    (void) OCIErrorGet((dvoid *)errhp, (ub4) 1, (text *) NULL, errcodep,
                       errbuf, (ub4) sizeof(errbuf), htype);
    (void) fprintf(output_fp, "Error - %.*s\n", 512, errbuf);
  }
}

STATICF void
checkerr(errhp, htype, status, note, code, file, line)
dvoid *errhp;
ub4    htype;
sword  status;
text  *note;
sb4    code;
text  *file;
sb4    line;
{
  sb4 errcode = 0;

  if ((status != OCI_SUCCESS))
    (void) fprintf(output_fp, "OCI Error %ld occurred at File %s:%ld\n",
                   (long)status, (char *)file, (long)line);

  if (note)
    (void) fprintf(output_fp, "File %s:%ld (code=%ld)  %s\n",
                   (char *)file, (long)line, (long)code, (char *)note);

  switch (status)
  {
  case OCI_SUCCESS:
    break;
  case OCI_SUCCESS_WITH_INFO:
    (void) fprintf(output_fp, "Error - OCI_SUCCESS_WITH_INFO\n");
    errprint(errhp, htype, &errcode);
    break;
  case OCI_NEED_DATA:
    (void) fprintf(output_fp, "Error - OCI_NEED_DATA\n");
    break;
  case OCI_NO_DATA:
    (void) fprintf(output_fp, "Error - OCI_NODATA\n");
    break;
  case OCI_ERROR:
    errprint(errhp, htype, &errcode);
    break;
  case OCI_INVALID_HANDLE:
    (void) fprintf(output_fp, "Error - OCI_INVALID_HANDLE\n");
    break;
  case OCI_STILL_EXECUTING:
    (void) fprintf(output_fp, "Error - OCI_STILL_EXECUTE\n");
    break;
  case OCI_CONTINUE:
    (void) fprintf(output_fp, "Error - OCI_CONTINUE\n");
    break;
  default:
    break;
  }
}


/* cleanup
 *   Free up handles and exit with the supplied exit status code.
 */
STATICF void
cleanup(ctlp, ex_status)
struct loadctl *ctlp;
sb4    ex_status;
{
  sword ociret;

  /* Free the column array and stream handles if they have been
   * allocated.  We don't need to do this since freeing the direct
   * path context will free the heap which these child handles have
   * been allocated from.  I'm doing this just to exercise the code
   * path to free these handles.
   */
  if (ctlp->dpca_ctl)
  {
    ociret = OCIHandleFree((dvoid *)ctlp->dpca_ctl,
                           OCI_HTYPE_DIRPATH_COLUMN_ARRAY);
    if (ociret != OCI_SUCCESS)
      CHECKERR(ctlp->errhp_ctl, OCI_HTYPE_ERROR, ociret);
  }

  if (ctlp->dpstr_ctl)
  {
    ociret = OCIHandleFree((dvoid *)ctlp->dpstr_ctl,
                           OCI_HTYPE_DIRPATH_STREAM);
    if (ociret != OCI_SUCCESS)
      CHECKERR(ctlp->errhp_ctl, OCI_HTYPE_ERROR, ociret);
  }

  if (ctlp->dpctx_ctl)
  {
    ociret = OCIHandleFree((dvoid *)ctlp->dpctx_ctl, OCI_HTYPE_DIRPATH_CTX);
    if (ociret != OCI_SUCCESS)
      CHECKERR(ctlp->errhp_ctl, OCI_HTYPE_ERROR, ociret);
  }

  if (ctlp->errhp_ctl && ctlp->srvhp_ctl)
  {
    (void) OCIServerDetach(ctlp->srvhp_ctl, ctlp->errhp_ctl, OCI_DEFAULT );
    ociret = OCIHandleFree((dvoid *)ctlp->srvhp_ctl, OCI_HTYPE_SERVER);
    if (ociret != OCI_SUCCESS)
      CHECKERR(ctlp->errhp_ctl, OCI_HTYPE_ERROR, ociret);
  }

  if (ctlp->svchp_ctl)
    (void) OCIHandleFree((dvoid *) ctlp->svchp_ctl, OCI_HTYPE_SVCCTX);
  if (ctlp->errhp_ctl)
    (void) OCIHandleFree((dvoid *) ctlp->errhp_ctl, OCI_HTYPE_ERROR);

  if ((output_fp != stdout) && (output_fp != stderr))
    fclose(output_fp);

  exit((int)ex_status);
}


/* end of file cdemodp.c */
