/*
 * $Header: cdemoses.h 14-jul-99.12:47:29 mjaeger Exp $
 */

/* Copyright (c) 1998, 1999,, 2000 Oracle Corporation.  All rights reserved.
*/

/* NOTE:  See 'header_template.doc' in the 'doc' dve under the 'forms'
      directory for the header file template that includes instructions.
*/

/*
   NAME
     cdemoses.h - C Demo for Session Management

   DESCRIPTION
   This program Illustrates the following functionality:
     (1) Session Switching
      Session Switching allows applications to multiplex several users
      over the same connection. This allows apps to multiplex several
      sessions over one connection without losing the database privilege
      and security features.

     (2) Session Migration
     Session Migration lets applications move sessions across connections.
     With this feature, the application can move sessions around
     dynamically based on system load and the application could implement
     its own application level user priority scheme.

   NOTES

   MODIFIED   (MM/DD/YY)
   lchidamb    06/12/00 - remove s.h
   mjaeger     07/14/99 - bug 808870: OCCS: convert tabs, no long lines
   lchidamb    10/14/98 - prototype changes
   lchidamb    10/13/98 - session management demo header
   lchidamb    10/13/98 - Creation

*/



#ifndef cdemoses_ORACLE
# define cdemoses_ORACLE



/*---------------------------------------------------------------------------
                     PUBLIC TYPES AND CONSTANTS
  ---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------
                     PRIVATE TYPES AND CONSTANTS
  ---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------
                           PUBLIC FUNCTIONS
  ---------------------------------------------------------------------------*/

void initialize_main(/*_ OCIEnv **envhpp, OCIError **errhpp _*/);

void terminate_main(/*_ OCIEnv *envhp, OCIError *errhp _*/);

void initialize_server(/*_ OCIEnv *envhp, OCIError *errhp,
                         OCIServer **srvhpp _*/);

void terminate_server(/*_ OCIEnv *envhp, OCIError *errhp,
                        OCIServer *srvhp _*/);

void initialize_user(/*_ OCIEnv *envhp, OCIError *errhp,
                       OCIServer *srvhp, OCISession **userhpp,
                              char *name _*/);

void initialize_migratable_user(/*_ OCIEnv *envhp, OCIError *errhp,
                       OCIServer *srvhp, OCISession *primary,
                       OCISession **userhpp,
                       char *name _*/);

void terminate_user(/*_ OCIEnv *envhp, OCIError *errhp,
                      OCIServer *srvhp, OCISession *userhp _*/);

void initialize_statement(/*_ OCIEnv *envhp, OCIError *errhp,
                            OCIStmt **stmhpp _*/);

void terminate_statement(/*_ OCIEnv *envhp, OCIError *errhp,
                           OCIStmt *stmhp _*/);

void error_report(/*_ OCIError *errhp, CONST text *op _*/);

void execute_statement(/*_ OCIEnv *envhp, OCIError *errhp,
                         OCISession *userhp, OCIServer *srvhp,
                         OCIStmt *stmhp _*/);


/*---------------------------------------------------------------------------
                          PRIVATE FUNCTIONS
  ---------------------------------------------------------------------------*/


#endif                                              /* cdemoses_ORACLE */
