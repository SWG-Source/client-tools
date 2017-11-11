rem
rem $Header: cdemoucb.sql 14-jul-99.13:55:09 mjaeger Exp $
rem
rem cdemoucb.sql
rem
rem Copyright (c) 1998, 1999, Oracle Corporation.  All rights reserved.
rem
rem    NAME
rem      cdemoucb.sql - <one-line expansion of the name>
rem
rem    DESCRIPTION
rem      <short description of component this file declares/defines>
rem
rem    NOTES
rem      <other useful comments, qualifications, etc.>
rem
rem    MODIFIED   (MM/DD/YY)
rem    mjaeger     07/14/99 - bug 808870: OCCS: convert tabs, no long lines
rem    bgoyal      10/16/98 - Sql for cdemoucb.c
rem    bgoyal      10/16/98 - Created
rem
connect internal;
drop user cdemoucb cascade;
grant connect, resource to cdemoucb identified by cdemoucb;

