rem
rem $Header: cdemodp_lip.sql 14-jul-99.13:49:14 mjaeger Exp $
rem
rem cdemodp_lip.sql
rem
rem Copyright (c) 1998, 1999, Oracle Corporation.  All rights reserved.
rem
rem    NAME
rem      cdemodp_lip.sql - C Demo Direct Path api for LineItem Partition table
rem
rem    DESCRIPTION
rem      - creates a lineitem partition tbl for loading data w/ direct path api
rem        via direct path API.
rem      - execute this script before running cdemodp driver
rem        w/cdemodp_lip client.
rem    NOTES
rem
rem    MODIFIED   (MM/DD/YY)
rem    mjaeger     07/14/99 - bug 808870: OCCS: convert tabs, no long lines
rem    abrumm      10/14/98 - fix typo
rem    cmlim       10/05/98 - create a diff tbl for data loaded thru conv path
rem    cmlim       09/16/98 - create tbl for cdemodp_lip client
rem    cmlim       09/16/98 - sql to create lineitem partition tbl
rem    cmlim       09/16/98 - Created (abrumm 04/07/98)
rem

set echo off;
connect scott/tiger;

rem     for direct path
drop table LINEITEM_DP;
create table LINEITEM_DP
 (L_ORDERKEY                               NUMBER,
  L_PARTKEY                                NUMBER,
  L_SUPPKEY                                NUMBER,
  L_LINENUMBER                             NUMBER,
  L_QUANTITY                               NUMBER,
  L_EXTENDEDPRICE                          NUMBER,
  L_DISCOUNT                               NUMBER,
  L_TAX                                    NUMBER,
  L_RETURNFLAG                             CHAR(1),
  L_LINESTATUS                             CHAR(1),
  L_SHIPDATE                               DATE,
  L_COMMITDATE                             DATE,
  L_RECEIPTDATE                            DATE,
  L_SHIPINSTRUCT                           VARCHAR2(25),
  L_SHIPMODE                               VARCHAR2(10),
  L_COMMENT                                VARCHAR2(44))
  partition by range (L_ORDERKEY)
  (partition LINEITEM1 values less than (12250)    ,
   partition LINEITEM2 values less than (24500)    ,
   partition LINEITEM3 values less than (36750)    ,
   partition LINEITEM4 values less than (49000)    ,
   partition LINEITEM5 values less than (61250)    ,
   partition LINEITEM6 values less than (73500)    ,
   partition LINEITEM7 values less than (maxvalue) );

rem     for conventional path
drop table LINEITEM_CV;
create table LINEITEM_CV
 (L_ORDERKEY                               NUMBER,
  L_PARTKEY                                NUMBER,
  L_SUPPKEY                                NUMBER,
  L_LINENUMBER                             NUMBER,
  L_QUANTITY                               NUMBER,
  L_EXTENDEDPRICE                          NUMBER,
  L_DISCOUNT                               NUMBER,
  L_TAX                                    NUMBER,
  L_RETURNFLAG                             CHAR(1),
  L_LINESTATUS                             CHAR(1),
  L_SHIPDATE                               DATE,
  L_COMMITDATE                             DATE,
  L_RECEIPTDATE                            DATE,
  L_SHIPINSTRUCT                           VARCHAR2(25),
  L_SHIPMODE                               VARCHAR2(10),
  L_COMMENT                                VARCHAR2(44))
  partition by range (L_ORDERKEY)
  (partition LINEITEM1 values less than (12250)    ,
   partition LINEITEM2 values less than (24500)    ,
   partition LINEITEM3 values less than (36750)    ,
   partition LINEITEM4 values less than (49000)    ,
   partition LINEITEM5 values less than (61250)    ,
   partition LINEITEM6 values less than (73500)    ,
   partition LINEITEM7 values less than (maxvalue) );

exit;
