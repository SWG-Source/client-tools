rem
rem $Header: oci21.sql 14-jul-99.14:17:19 mjaeger Exp $
rem
rem Copyright (c) 1995, 1999, Oracle Corporation.  All rights reserved.
rem
rem    NAME
rem      oci21.sql
rem    DESCRIPTION
rem      Script for A22400 OCI Techniques White Paper
rem      Demo script for oci21.c
rem    MODIFIED   (MM/DD/YY)
rem     mjaeger    07/14/99 -  bug 808870: OCCS: convert tabs, no long lines
rem     cchau      08/18/97 -  enable dictionary protection
rem     echen      01/10/97 -  change internal to sys/change_on_install
rem     vraghuna   03/01/95 -  Creation

set echo on;
connect sys/knl_test7 as sysdba;
drop user ocitest cascade;
create user ocitest identified by ocitest;
grant connect,resource to ocitest;
connect ocitest/ocitest;

create table oci21tab (col1 varchar2(30));
insert into oci21tab values ('A');
insert into oci21tab values ('AB');
insert into oci21tab values ('ABC');
insert into oci21tab values ('ABCD');
insert into oci21tab values ('ABCDE');
insert into oci21tab values ('ABCDEF');
insert into oci21tab values ('ABCDEFG');
insert into oci21tab values ('ABCDEFGH');
insert into oci21tab values ('ABCDEFGHI');
insert into oci21tab values ('ABCDEFGHIJ');
insert into oci21tab values ('ABCDEFGHIJK');
insert into oci21tab values ('ABCDEFGHIJKL');

commit;

