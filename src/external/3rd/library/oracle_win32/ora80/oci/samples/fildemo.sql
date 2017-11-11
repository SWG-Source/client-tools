rem
rem $Header: fildemo.sql 18-apr-00.13:04:53 hyeh Exp $
rem
rem  fildemo.sql
rem
rem Copyright (c) 1995, 1999,, 2000 Oracle Corporation.  All rights reserved.
rem
rem    NAME
rem      fildemo.sql - PL/SQL procedures demonstrating BFILE access
rem
rem    DESCRIPTION
rem      Demonstration/Test program for use of Oracle8 Large Objects
rem
rem    NOTES
rem      . Outline for this test/demo borrowed from lobdemo.sql.
rem      . This test/demo uses dbms_output and utl_raw packages.
rem      . This test script also makes use of two input files gpfdf1.dat,
rem        and gpfdf2.dat which contain binary test data. These test files
rem        will be located in the /vobs/rdbms/test/tkoo/data directory.
rem      . DIRECTORY values are set by substitution variables ^1 and ^2
rem      . Run this script inside a emacs shell.
rem
rem    MODIFIED   (MM/DD/YY)
rem    hyeh        04/18/00 - cleanup test
rem    hyeh        08/10/99 - use sqlplus syntax
rem    mjaeger     07/14/99 - bug 808870: OCCS: convert tabs, no long lines
rem    cchau       08/18/97 - enable dictionary protection
rem    ramkrish    12/06/96 - Add more failure tests
rem    ramkrish    11/22/96 - GETNAME -> FILEGETNAME
rem    ramkrish    11/05/96 - addnal tests + misc cleanup
rem    ramkrish    11/04/96 - interface chg
rem    ramkrish    09/25/96 - Add more tests
rem    ramkrish    07/26/96 - Creation
rem

REMARK >>>> Set System Variables For Current SQLPlus Session <<<<
SET FEEDBACK 1
SET NUMWIDTH 10
SET PAGESIZE 24
SET LINESIZE 80
SET TRIMSPOOL ON
SET TAB OFF
SET DEFINE '^'
SET VERIFY OFF

REM ################
REM  SET ENVIRONMENT
REM ################

REM Cleanup test environment from previous runs
set echo on
connect sys/knl_test7 as sysdba;
set serveroutput on;
drop directory gpfdd1;
drop directory gpfdd2;
drop user gpfd cascade;
REM uncomment to spool output to log
REM spool fildemo.log

REM Create a new user and grant the package to the new user
grant connect, resource to gpfd identified by gpfd;

REM Create test directories (change to point to your favorite directories)
create directory gpfdd1 as '^1';
create directory gpfdd2 as '^2';

REM ERR: Test without directories

REM ERR: Test without physical directories
REM create directory gpfdd1 as '/bfile_non_existent_directory';
REM create directory gpfdd2 as '/bfile_non_existent_directory';

REM ERR: Test without privileges on the directory

REM Grant access to new user
grant read on directory gpfdd1 to gpfd with grant option;
grant read on directory gpfdd2 to gpfd with grant option;

REM Check status of created directories
select ob.name "DIRECTORY", us.name "GRANTEE",
       tpmap.name "PRIV NAME", oa.option$ "GRANT OPTION"
from   obj$ ob, user$ us, objauth$ oa, table_privilege_map tpmap
where  (ob.type# = 23)          and
       (ob.obj# = oa.obj#)      and
       (oa.grantee# = us.user#) and
       (oa.privilege# = tpmap.privilege) and
       (us.name = 'GPFD')
order by ob.name, us.name, tpmap.name;

REM start session as new user
connect gpfd/gpfd

REM verify success of directory ddl
select * from all_directories order by directory_name;

REM create test table
create table gpfdt1(c1 integer, c2 bfile);

REM insert values into the table
insert into gpfdt1 values (1, bfilename('GPFDD1', 'gpfdf1.dat'));
insert into gpfdt1 values (2, bfilename('GPFDD2', 'gpfdf2.dat'));
insert into gpfdt1 values (3, bfilename('GPFDD1', 'gpfdf3.dat'));
commit;

REM ERR: Test with non-existent files
REM insert into gpfdt1 values (1, bfilename('GPFDD1', 'noexist1.dat'));
REM insert into gpfdt1 values (2, bfilename('GPFDD2', 'noexist2.dat'));
REM insert into gpfdt1 values (3, bfilename('GPFDD1', 'noexist3.dat'));
REM commit;

REM ERR: Test without privileges on the file
REM insert into gpfdt1 values (1, bfilename('GPFDD1', 'gpfdf1.bak.dat'));
REM insert into gpfdt1 values (2, bfilename('GPFDD2', 'gpfdf2.bak.dat'));
REM insert into gpfdt1 values (3, bfilename('GPFDD1', 'gpfdf3.bak.dat'));
REM commit;

REM log output onscreen
set echo on
set serveroutput on size 5000

REM verify inserts
declare
  fil1 bfile;
  dname varchar2(100);
  fname varchar2(20);
begin
  for i in 1..3 loop
    fil1 := null;
    dname := null;
    fname := null;
    select  c2 into fil1 from gpfdt1 where c1 = i;
    dbms_lob.filegetname(fil1, dname, fname);
    dbms_output.put_line('c1: '|| to_char(i) || ', directory: ' ||
                          dname || ', file: ' || fname);
  end loop;
end;
/

REM ##############
REM  EXECUTE TESTS
REM ##############

REM ---------------------------------------.
REM  successful runs from dbms_lob routines
REM ---------------------------------------.
declare
  fil1  bfile;
  fil2  bfile;
  fil3  bfile;
  pos1  integer;
  pos2  integer;
  ret   integer;
  amt   integer;
  occ   integer;
  len   integer;
  rdamt binary_integer;
  buf   raw(40);
  fname varchar2(40);
  dirnm varchar2(40);
begin
  dbms_output.put_line('--------------- EXISTS Begin --------------');
  ret := -1;
  select c2 into fil1 from gpfdt1 where c1 = 1;
  ret := dbms_lob.fileexists(fil1);
  if (ret = 1) then
    dbms_output.put_line('File 1 exists');
  else
    dbms_output.put_line('File 1 does not exist');
  end if;
  ret := -1;
  select c2 into fil2 from gpfdt1 where c1 = 2;
  ret := dbms_lob.fileexists(fil2);
  if (ret = 1) then
    dbms_output.put_line('File 2 exists');
  else
    dbms_output.put_line('File 2 does not exist');
  end if;
  dbms_output.put_line('--------------- EXISTS End   --------------');
  dbms_output.put_line(' ');
  --.
  --.
  dbms_output.put_line('--------------- LENGTH Begin --------------');
  len := -1;
  select c2 into fil1 from gpfdt1 where c1 = 1;
  len := dbms_lob.getlength(fil1);
  dbms_output.put_line('Length of File 1 is: ' || len);
  len := -1;
  select c2 into fil2 from gpfdt1 where c1 = 2;
  len := dbms_lob.getlength(fil2);
  dbms_output.put_line('Length of File 2 is: ' || len);
  dbms_output.put_line('--------------- LENGTH End   --------------');
  dbms_output.put_line(' ');
  --.
  --.
  dbms_output.put_line('--------------- GETNAME Begin ---------------');
  fname := '';
  dirnm := '';
  select c2 into fil1 from gpfdt1 where c1 = 1;
  dbms_lob.filegetname(fil1, dirnm, fname);
  dbms_output.put_line('Getname File 1 Dir: ' || dirnm || ' File: ' || fname);
  fname := '';
  dirnm := '';
  select c2 into fil2 from gpfdt1 where c1 = 2;
  dbms_lob.filegetname(fil2, dirnm, fname);
  dbms_output.put_line('Getname File 2 Dir: ' || dirnm || ' File: ' || fname);
  dbms_output.put_line('--------------- GETNAME End   ---------------');
  dbms_output.put_line(' ');
  --.
  --.
  dbms_output.put_line('--------------- FOPEN Begin ---------------');
  select c2 into fil1 from gpfdt1 where c1 = 1;
  dbms_lob.fileopen(fil1, dbms_lob.file_readonly);
  select c2 into fil2 from gpfdt1 where c1 = 2;
  dbms_lob.fileopen(fil2, dbms_lob.file_readonly);
  dbms_output.put_line('--------------- FOPEN End   ---------------');
  dbms_output.put_line(' ');
  --.
  --.
  dbms_output.put_line('--------------- ISOPEN Begin --------------');
  ret := -1;
  ret := dbms_lob.fileisopen(fil1);
  if (ret = 1) then
    dbms_output.put_line('File 1 is open');
  else
    dbms_output.put_line('File 1 is not open');
  end if;
  ret := -1;
  ret := dbms_lob.fileisopen(fil2);
  if (ret = 1) then
    dbms_output.put_line('File 2 is open');
  else
    dbms_output.put_line('File 2 is not open');
  end if;
  dbms_output.put_line('--------------- ISOPEN End   --------------');
  dbms_output.put_line(' ');
  --.
  --.
  dbms_output.put_line('--------------- READ Begin ----------------');
  -- Case 1: Read 40 bytes starting from offset 1 from File 1
  rdamt := 40;
  pos1 := 1;
  buf := '';
  dbms_lob.read(fil1, rdamt, pos1, buf);
  dbms_output.put_line('Read F1 40/1 buf: '|| utl_raw.cast_to_varchar2(buf));
  -- Case 2: Read 40 bytes starting from offset 1 from File 2
  rdamt := 40;
  pos2 := 1;
  buf := '';
  dbms_lob.read(fil2, rdamt, pos2, buf);
  dbms_output.put_line('Read F2 40/1 buf: '|| utl_raw.cast_to_varchar2(buf));
  -- Case 3: Read 40 bytes starting from offset 5 from File 1
  rdamt := 40;
  pos1 := 5;
  buf := '';
  dbms_lob.read(fil1, rdamt, pos1, buf);
  dbms_output.put_line('Read F1 40/5 buf: '|| utl_raw.cast_to_varchar2(buf));
  -- Case 4: Read 40 bytes starting from offset 5 from File 2
  rdamt := 40;
  pos2 := 5;
  buf := '';
  dbms_lob.read(fil2, rdamt, pos2, buf);
  dbms_output.put_line('Read F2 40/5 buf: '|| utl_raw.cast_to_varchar2(buf));
  -- Case 5: Read 40 bytes starting from offset length(F1) from File 1
  rdamt := 40;
  pos1 := dbms_lob.getlength(fil1);
  buf := '';
  dbms_lob.read(fil1, rdamt, pos1, buf);
  dbms_output.put_line('Read F1 40/lenF1: ' || utl_raw.cast_to_varchar2(buf));
  -- Case 6: Read 40 bytes starting from offset length(F2) from File 2
  rdamt := 40;
  pos2 := dbms_lob.getlength(fil2);
  buf := '';
  dbms_lob.read(fil2, rdamt, pos2, buf);
  dbms_output.put_line('Read F2 40/lenF2: ' || utl_raw.cast_to_varchar2(buf));
  dbms_output.put_line('--------------- READ End   ----------------');
  dbms_output.put_line(' ');
  --.
  --.
  dbms_output.put_line('--------------- SUBSTR Begin --------------');
  -- Case 1: Substr 40 bytes starting from offset 1 from File 1
  rdamt := 40;
  pos1 := 1;
  buf := '';
  buf := dbms_lob.substr(fil1, rdamt, pos1);
  dbms_output.put_line('Substr F1 40/1: ' || utl_raw.cast_to_varchar2(buf));
  -- Case 2: Substr 40 bytes starting from offset 1 from File 2
  rdamt := 40;
  pos2 := 1;
  buf := '';
  buf := dbms_lob.substr(fil2, rdamt, pos2);
  dbms_output.put_line('Substr F2 40/1: ' || utl_raw.cast_to_varchar2(buf));
  -- Case 3: Substr 40 bytes starting from offset 5 from File 1
  rdamt := 40;
  pos1 := 5;
  buf := '';
  buf := dbms_lob.substr(fil1, rdamt, pos1);
  dbms_output.put_line('Substr F1 40/5: ' || utl_raw.cast_to_varchar2(buf));
  -- Case 4: Substr 40 bytes starting from offset 5 from File 2
  rdamt := 40;
  pos2 := 5;
  buf := '';
  buf := dbms_lob.substr(fil2, rdamt, pos2);
  dbms_output.put_line('Substr F2 40/5: ' || utl_raw.cast_to_varchar2(buf));
  -- Case 5: Substr 40 bytes starting from offset length(F1) from File 1
  rdamt := 40;
  pos1 := dbms_lob.getlength(fil1);
  buf := '';
  buf := dbms_lob.substr(fil1, rdamt, pos1);
  dbms_output.put_line('Substr F1 40/lenF1: '|| utl_raw.cast_to_varchar2(buf));
  -- Case 6: Substr 40 bytes starting from offset length(F2) from File 2
  rdamt := 40;
  pos2 := dbms_lob.getlength(fil2);
  buf := '';
  buf := dbms_lob.substr(fil2, rdamt, pos2);
  dbms_output.put_line('Substr F2 1/lenF2: '|| utl_raw.cast_to_varchar2(buf));
  dbms_output.put_line('--------------- SUBSTR End   --------------');
  dbms_output.put_line(' ');
  --.
  --.
  dbms_output.put_line('--------------- COMPARE Begin -------------');
  select c2 into fil3 from gpfdt1 where c1 = 3;
  dbms_lob.fileopen(fil3, dbms_lob.file_readonly);
  -- Case 1: Compare File 1 & 3 40 bytes, offset 1|1
  ret := -1;
  amt := 40;
  pos1 := 1;
  pos2 := 1;
  ret := dbms_lob.compare(fil1, fil3, amt, pos1, pos2);
  dbms_output.put_line('Compare F1/F3 40/1|1 returns: ' || ret);
  -- Case 2: Compare File 1 & 3 40 bytes, offset 5|5
  ret := -1;
  amt := 40;
  pos1 := 5;
  pos2 := 5;
  ret := dbms_lob.compare(fil1, fil3, amt, pos1, pos2);
  dbms_output.put_line('Compare F1/F3 40/5|5 returns: ' || ret);
  -- Case 3: Compare File 1 & 3 40 bytes, offset 1|5
  ret := -1;
  amt := 40;
  pos1 := 1;
  pos2 := 5;
  ret := dbms_lob.compare(fil1, fil3, amt, pos1, pos2);
  dbms_output.put_line('Compare F1/F3 40/1|5 returns: ' || ret);
  -- Case 4: Compare File 1 & 3 40 bytes, offset 5|1
  ret := -1;
  amt := 40;
  pos1 := 5;
  pos2 := 1;
  ret := dbms_lob.compare(fil1, fil3, amt, pos1, pos2);
  dbms_output.put_line('Compare F1/F3 40/5|1 returns: ' || ret);
  -- Case 5: Compare File 1 & 2 4 bytes, offset 1|31
  ret := -1;
  amt := 4;
  pos1 := 1;
  pos2 := 31;
  ret := dbms_lob.compare(fil1, fil2, amt, pos1, pos2);
  dbms_output.put_line('Compare F1/F2 4/1|31 returns: ' || ret);
  -- Case 6: Compare File 1 & 2 4 bytes, offset 1|1
  ret := -1;
  amt := 4;
  pos1 := 1;
  pos2 := 1;
  ret := dbms_lob.compare(fil1, fil2, amt, pos1, pos2);
  dbms_output.put_line('Compare F1/F2 40/1|1 returns: ' || ret);
  dbms_lob.fileclose(fil3);
  dbms_output.put_line('--------------- COMPARE End   -------------');
  dbms_output.put_line(' ');
  --.
  --.
  dbms_output.put_line('--------------- INSTR Begin ---------------');
  -- Case 1: Instr File 1, offset 1, 1st occurrence
  buf := '';
  buf := utl_raw.cast_to_raw('this is a test file');
  ret := -1;
  occ := 1;
  pos1 := 1;
  ret := dbms_lob.instr(fil1, buf, pos1, occ);
  dbms_output.put_line('Instr F1 offset 1/occ 1 returns: ' || ret);
  -- Case 2: Instr File 2, offset 1, 1st occurrence
  buf := '';
  buf := utl_raw.cast_to_raw('file test a is this');
  ret := -1;
  occ := 1;
  pos2 := 1;
  ret := dbms_lob.instr(fil2, buf, pos2, occ);
  dbms_output.put_line('Instr F2 offset 1/occ 1 returns: ' || ret);
  -- Case 3: Instr File 1, offset 1, 2nd occurrence
  buf := '';
  buf := utl_raw.cast_to_raw('this is a test file');
  ret := -1;
  occ := 2;
  pos1 := 1;
  ret := dbms_lob.instr(fil1, buf, pos1, occ);
  dbms_output.put_line('Instr F1 offset 1/occ 2 returns: ' || ret);
  -- Case 4: Instr File 2, offset 1, 2nd occurrence
  buf := '';
  buf := utl_raw.cast_to_raw('file test a is this');
  ret := -1;
  occ := 2;
  pos2 := 1;
  ret := dbms_lob.instr(fil2, buf, pos2, occ);
  dbms_output.put_line('Instr F2 offset 1/occ 2 returns: ' || ret);
  -- Case 5: Instr File 1, offset 20, 1st occurrence
  buf := '';
  buf := utl_raw.cast_to_raw('this is a test file');
  ret := -1;
  occ := 1;
  pos1 := 20;
  ret := dbms_lob.instr(fil1, buf, pos1, occ);
  dbms_output.put_line('Instr F1 offset 20/occ 1 returns: ' || ret);
  -- Case 6: Instr File 2, offset 1, 1st occurrence
  buf := '';
  buf := utl_raw.cast_to_raw('file test a is this');
  ret := -1;
  occ := 1;
  pos2 := 20;
  ret := dbms_lob.instr(fil2, buf, pos2, occ);
  dbms_output.put_line('Instr F2 offset 20/occ 1 returns: ' || ret);
  -- Case 7: Instr File 1, offset 1, 3rd occurrence
  buf := '';
  buf := utl_raw.cast_to_raw('this is a test file');
  ret := -1;
  occ := 3;
  pos1 := 1;
  ret := dbms_lob.instr(fil1, buf, pos1, occ);
  dbms_output.put_line('Instr F1 offset 1/occ 3 returns: ' || ret);
  -- Case 8: Instr File 2, offset 1, 3rd occurrence
  buf := '';
  buf := utl_raw.cast_to_raw('file test a is this');
  ret := -1;
  occ := 3;
  pos2 := 1;
  ret := dbms_lob.instr(fil2, buf, pos2, occ);
  dbms_output.put_line('Instr F2 offset 1/occ 3 returns: ' || ret);
  dbms_output.put_line('--------------- INSTR End   ---------------');
  dbms_output.put_line(' ');
  --.
  --.
  dbms_output.put_line('--------------- FCLOSE Begin --------------');
  dbms_lob.fileclose(fil1);
  dbms_lob.fileclose(fil2);
  dbms_output.put_line('--------------- FCLOSE End   --------------');
  dbms_output.put_line(' ');
end;
/
REM ---------------------------------.
REM  successful dbms_lob.filecloseall
REM ---------------------------------.
declare
  fil1  bfile;
  fil2  bfile;
  ret   integer;
begin
  dbms_output.put_line('---------- TEST FCLOSE_ALL Begin ----------');
  dbms_output.put_line(' ');
  dbms_output.put_line('--------------- FOPEN Begin ---------------');
  select c2 into fil1 from gpfdt1 where c1 = 1;
  dbms_lob.fileopen(fil1, dbms_lob.file_readonly);
  select c2 into fil2 from gpfdt1 where c1 = 2;
  dbms_lob.fileopen(fil2, dbms_lob.file_readonly);
  dbms_output.put_line('--------------- FOPEN End   ---------------');
  dbms_output.put_line(' ');
  --.
  --.
  dbms_output.put_line('--------------- FCLOSE_ALL Begin ----------');
  dbms_lob.filecloseall;
  dbms_output.put_line('--------------- FCLOSE_ALL End   ----------');
  dbms_output.put_line(' ');
  --.
  --.
  dbms_output.put_line('--------------- ISOPEN Begin --------------');
  ret := -1;
  ret := dbms_lob.fileisopen(fil1);
  if (ret = 1) then
    dbms_output.put_line('File 1 is open');
  else
    dbms_output.put_line('File 1 is not open');
  end if;
  ret := -1;
  ret := dbms_lob.fileisopen(fil2);
  if (ret = 1) then
    dbms_output.put_line('File 2 is open');
  else
    dbms_output.put_line('File 2 is not open');
  end if;
  dbms_output.put_line('--------------- ISOPEN End   --------------');
  dbms_output.put_line(' ');
  dbms_output.put_line('---------- TEST FCLOSE_ALL End   ----------');
end;
/
REM ------------------------------------------------.
REM  fileopen with locator initlzed with bfilename()
REM ------------------------------------------------.
declare
  fil bfile;
  ret integer := -1;
begin
  fil := bfilename('GPFDD1', 'gpfdf1.dat');
  dbms_lob.fileopen(fil, dbms_lob.file_readonly);
  ret := dbms_lob.fileisopen(fil);
  if (ret = 1) then
    dbms_output.put_line('File is open');
  end if;
  dbms_lob.fileclose(fil);
end;
/
REM --------------------------------------------------.
REM  fileexists with locator initlzed with bfilename()
REM --------------------------------------------------.
declare
  fil bfile;
  ret integer := -1;
begin
  fil := bfilename('GPFDD1', 'gpfdf1.dat');
  ret := dbms_lob.fileexists(fil);
  if (ret = 1) then
    dbms_output.put_line('File exists');
  else
    dbms_output.put_line('File does not exist');
  end if;
  fil := bfilename('GPFDD1', 'noexist1.dat');
  ret := -1;
  ret := dbms_lob.fileexists(fil);
  if (ret = 1) then
    dbms_output.put_line('File exists');
  else
    dbms_output.put_line('File does not exist');
  end if;
end;
/
REM ---------------------------------------------------.
REM  bfile locator scope testing with PL/SQL procedures
REM ---------------------------------------------------.
declare
  fil bfile;
  procedure fread(fil in bfile) is
    amt integer;
    pos integer;
    buf raw(40);
  begin
    -- Read 40 bytes starting from offset 1 from file
    amt := 40;
    pos := 1;
    buf := '';
    dbms_lob.read(fil, amt, pos, buf);
    dbms_output.put_line('FREAD 40/1 buf: '|| utl_raw.cast_to_varchar2(buf));
  end fread;
begin
  select c2 into fil from gpfdt1 where c1 = 1;
  dbms_lob.fileopen(fil, dbms_lob.file_readonly);
  fread(fil);
  dbms_lob.fileclose(fil);
end;
/
REM --------------------------------------------------.
REM  bfile locator scope testing with bfileADT methods
REM --------------------------------------------------.

REM ------------------------------.
REM  consecutive dbms_lob.fileopen
REM ------------------------------.
declare
  fil bfile;
begin
  select c2 into fil from gpfdt1 where c1 = 1;
  dbms_lob.fileopen(fil, dbms_lob.file_readonly);
  dbms_lob.fileopen(fil, dbms_lob.file_readonly);
  dbms_lob.fileclose(fil);
end;
/
REM -------------------------------.
REM  consecutive dbms_lob.fileclose
REM -------------------------------.
declare
  fil bfile;
begin
  select c2 into fil from gpfdt1 where c1 = 1;
  dbms_lob.fileopen(fil, dbms_lob.file_readonly);
  dbms_lob.fileclose(fil);
  dbms_lob.fileclose(fil);
end;
/
REM ---------------------------------.
REM  dbms_lob.read with unopened file
REM ---------------------------------.
declare
  fil bfile;
  pos integer;
  amt binary_integer;
  buf raw(40);
begin
  amt := 40;
  pos := 1;
  buf := '';
  select c2 into fil from gpfdt1 where c1 = 1;
  dbms_lob.read(fil, amt, pos, buf);
  exception
    when dbms_lob.unopened_file
    then dbms_output.put_line('Unopened file exception raised');
end;
/
REM -----------------------------------.
REM  dbms_lob.substr with unopened file
REM -----------------------------------.
declare
  fil bfile;
  pos integer;
  amt integer;
  buf raw(40);
begin
  amt := 40;
  pos := 1;
  buf := '';
  select c2 into fil from gpfdt1 where c1 = 1;
  buf := dbms_lob.substr(fil, amt, pos);
end;
/
REM ------------------------------------.
REM  dbms_lob.compare with unopened file
REM ------------------------------------.
declare
  fil1 bfile;
  fil2 bfile;
  pos1 integer;
  pos2 integer;
  amt  integer;
  ret  binary_integer;
begin
  amt := 40;
  pos1 := 1;
  pos2 := 1;
  select c2 into fil1 from gpfdt1 where c1 = 1;
  select c2 into fil2 from gpfdt1 where c1 = 2;
  ret := dbms_lob.compare(fil1, fil2, amt, pos1, pos2);
end;
/
REM ----------------------------------.
REM  dbms_lob.instr with unopened file
REM ----------------------------------.
declare
  fil bfile;
  pos integer;
  occ integer;
  pat raw(5);
  ret binary_integer;
begin
  pos := 40;
  occ := 1;
  pat := '01234';
  select c2 into fil from gpfdt1 where c1 = 1;
  ret := dbms_lob.instr(fil, pat, pos, occ);
end;
/
REM ---------------------------------------.
REM  dbms_lob.fileisopen with unopened file
REM ---------------------------------------.
declare
  fil bfile;
  ret integer := -1;
begin
  select c2 into fil from gpfdt1 where c1 = 1;
  ret := dbms_lob.fileisopen(fil);
  if (ret = 1) then
    dbms_output.put_line('File is open');
  else
    dbms_output.put_line('File is not open');
  end if;
end;
/
REM -------------------------------.
REM  dbms_lob.read past End of file
REM -------------------------------.
declare
  fil bfile;
  pos integer;
  amt binary_integer;
  buf raw(40);
begin
  select c2 into fil from gpfdt1 where c1 = 1;
  dbms_lob.fileopen(fil, dbms_lob.file_readonly);
  amt := 40;
  pos := 1 + dbms_lob.getlength(fil);
  buf := '';
  dbms_lob.read(fil, amt, pos, buf);
  dbms_output.put_line('Read F1 past EOF: '|| utl_raw.cast_to_varchar2(buf));
  dbms_lob.fileclose(fil);
  exception
    when no_data_found
    then
      begin
        dbms_output.put_line('End of File reached. Closing file');
        dbms_lob.fileclose(fil);
      end;
end;
/
REM ---------------------------------.
REM  dbms_lob.substr past End of file
REM ---------------------------------.
declare
  fil bfile;
  pos integer;
  amt binary_integer;
  buf raw(40);
begin
  select c2 into fil from gpfdt1 where c1 = 1;
  dbms_lob.fileopen(fil, dbms_lob.file_readonly);
  amt := 40;
  pos := 1 + dbms_lob.getlength(fil);
  buf := '';
  buf := dbms_lob.substr(fil, amt, pos);
  dbms_output.put_line('Substr F1 past EOF: '|| utl_raw.cast_to_varchar2(buf));
  dbms_lob.fileclose(fil);
end;
/
REM --------------------------------.
REM  dbms_lob.instr past End of file
REM --------------------------------.
declare
  fil bfile;
  pos integer;
  occ integer;
  pat raw(5);
  ret integer;
begin
  select c2 into fil from gpfdt1 where c1 = 1;
  dbms_lob.fileopen(fil, dbms_lob.file_readonly);
  occ := 1;
  pos := 1 + dbms_lob.getlength(fil);
  pat := utl_raw.cast_to_raw('test ');
  ret := dbms_lob.instr(fil, pat, pos, occ);
  dbms_output.put_line('Instr F1 past EOF: '|| ret);
  dbms_lob.fileclose(fil);
end;
/
REM ----------------------------------.
REM  dbms_lob.compare past End of file
REM ----------------------------------.
declare
  fil1 bfile;
  fil2 bfile;
  pos1 integer;
  pos2 integer;
  amt  integer;
  ret  binary_integer;
begin
  select c2 into fil1 from gpfdt1 where c1 = 1;
  dbms_lob.fileopen(fil1, dbms_lob.file_readonly);
  select c2 into fil2 from gpfdt1 where c1 = 2;
  dbms_lob.fileopen(fil2, dbms_lob.file_readonly);
  amt := 40;
  pos1 := 1 + dbms_lob.getlength(fil1);
  pos2 := 1;
  ret := dbms_lob.compare(fil1, fil2, amt, pos1, pos2);
  dbms_output.put_line('Compare with F1 past EOF: '|| ret);
  amt := 40;
  pos1 := 1;
  pos2 := 1 + dbms_lob.getlength(fil2);
  ret := dbms_lob.compare(fil1, fil2, amt, pos1, pos2);
  dbms_output.put_line('Compare with F2 past EOF: '|| ret);
  dbms_lob.fileclose(fil1);
  dbms_lob.fileclose(fil2);
end;
/
REM ---------------------------.
REM  Too many dbms_lob.fileopen
REM ---------------------------.
declare
  fil1 bfile;
  fil2 bfile;
  fil3 bfile;
  fil4 bfile;
  fil5 bfile;
  fil6 bfile;
  fil7 bfile;
  fil8 bfile;
  fil9 bfile;
  fila bfile;
  filb bfile;
begin
  select c2 into fil1 from gpfdt1 where c1 = 1;
  dbms_lob.fileopen(fil1, dbms_lob.file_readonly);
  select c2 into fil2 from gpfdt1 where c1 = 1;
  dbms_lob.fileopen(fil2, dbms_lob.file_readonly);
  select c2 into fil3 from gpfdt1 where c1 = 1;
  dbms_lob.fileopen(fil3, dbms_lob.file_readonly);
  select c2 into fil4 from gpfdt1 where c1 = 1;
  dbms_lob.fileopen(fil4, dbms_lob.file_readonly);
  select c2 into fil5 from gpfdt1 where c1 = 1;
  dbms_lob.fileopen(fil5, dbms_lob.file_readonly);
  select c2 into fil6 from gpfdt1 where c1 = 1;
  dbms_lob.fileopen(fil6, dbms_lob.file_readonly);
  select c2 into fil7 from gpfdt1 where c1 = 1;
  dbms_lob.fileopen(fil7, dbms_lob.file_readonly);
  select c2 into fil8 from gpfdt1 where c1 = 1;
  dbms_lob.fileopen(fil8, dbms_lob.file_readonly);
  select c2 into fil9 from gpfdt1 where c1 = 1;
  dbms_lob.fileopen(fil9, dbms_lob.file_readonly);
  select c2 into fila from gpfdt1 where c1 = 1;
  dbms_lob.fileopen(fila, dbms_lob.file_readonly);
  -- should error below with default SESSION_MAX_OPEN_FILES=10
  select c2 into filb from gpfdt1 where c1 = 1;
  dbms_lob.fileopen(filb, dbms_lob.file_readonly);
  exception
  when dbms_lob.open_toomany
  then
  begin
    dbms_output.put_line('Too many open files. Closing files');
    dbms_lob.fileclose(fil1);
    dbms_lob.fileclose(fil2);
    dbms_lob.fileclose(fil3);
    dbms_lob.fileclose(fil4);
    dbms_lob.fileclose(fil5);
    dbms_lob.fileclose(fil6);
    dbms_lob.fileclose(fil7);
    dbms_lob.fileclose(fil8);
    dbms_lob.fileclose(fil9);
    dbms_lob.fileclose(fila);
  end;
end;
/
REM ------------------------------------------.
REM  boolean operations on NULL bfile locators
REM ------------------------------------------.
declare
  fil bfile := NULL;
  ret integer := -1;
begin
  ret := dbms_lob.fileexists(fil);
  if (ret = 1) then
    dbms_output.put_line('File exists');
  else
    dbms_output.put_line('File does not exist');
  end if;
end;
/
declare
  fil bfile := NULL;
  ret integer := -1;
begin
  ret := dbms_lob.fileisopen(fil);
  if (ret = 1) then
    dbms_output.put_line('File is open');
  else
    dbms_output.put_line('File is open');
  end if;
end;
/
REM ----------------------------------------.
REM  Revoke privs and attempt file operation
REM ----------------------------------------.
connect sys/knl_test7 as sysdba;
revoke read on  directory gpfdd1 from gpfd;
connect gpfd/gpfd;
declare
  fil bfile;
begin
  select c2 into fil from gpfdt1 where c1 = 1;
  dbms_lob.fileopen(fil, dbms_lob.file_readonly);
  dbms_lob.fileclose(fil);
end;
/
declare
  fil bfile;
  pos integer;
  amt binary_integer;
  buf raw(40);
begin
  amt := 40;
  pos := 1;
  buf := '';
  fil := bfilename('GPFDD1', 'gpfdf1.dat');
  dbms_lob.read(fil, amt, pos, buf);
  exception
    when dbms_lob.noexist_directory
    then dbms_output.put_line('Directory does not exist');
end;
/
REM #############
REM  END SESSION
REM #############
connect sys/knl_test7 as sysdba;
drop user gpfd cascade;
drop directory gpfdd1;
drop directory gpfdd2;
set serveroutput off
set echo off
REM spool off
exit
