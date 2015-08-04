rem
rem $Header: oci24.sql 14-jul-99.14:18:57 mjaeger Exp $
rem
rem Copyright (c) 1995, 1999, Oracle Corporation.  All rights reserved.
rem
rem    NAME
rem      oci24.sql
rem    DESCRIPTION
rem      Script for A22400 OCI Techniques White Paper
rem      Demo script for oci24.c
rem    MODIFIED   (MM/DD/YY)
rem     mjaeger    07/14/99 -  bug 808870: OCCS: convert tabs, no long lines
rem     svedala    09/11/98 -  a "/" required after create package-bug 717842
rem     cchau      08/18/97 -  enable dictionary protection
rem     echen      01/10/97 -  change internal to sys/change_on_install
rem     vraghuna   03/01/95 -  Creation

set echo on;
connect sys/knl_test7 as sysdba;

rem
rem Create a new user - call it ocitest
rem
drop user ocitest cascade;
create user ocitest identified by ocitest;
grant connect, resource to ocitest;

rem
rem Created needed tables
rem
connect ocitest/ocitest;

create table oci24tab (ename char(10), empno number, sal number);

create or replace package oci24pkg as

procedure oci24proc(
        name in oci24tab.ename%type,
        salary out oci24tab.sal%type);

procedure oci24proc(
        id_num in oci24tab.empno%type,
        salary out oci24tab.sal%type);

function oci24proc (
        name oci24tab.ename%type) return number;

end oci24pkg;
/


