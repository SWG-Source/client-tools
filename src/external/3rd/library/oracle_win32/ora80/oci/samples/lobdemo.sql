rem
rem $Header: lobdemo.sql 10-aug-99.14:34:11 hyeh Exp $
rem
rem lobdemo.sql
rem
rem Copyright (c) 1995, 1999, Oracle Corporation.  All rights reserved.
rem
rem    NAME
rem      lobdemo.sql - PL/SQL procedures demonstrating LOB access
rem
rem    DESCRIPTION
rem      Demonstration/Test program for use of Oracle8 Large Objects
rem
rem    NOTES
rem      . This test/demo uses dbms_output and utl_raw packages
rem      . The INSERT statements currently outside the tst_routines need to
rem        be moved into the tst_routine once the bug is solved.
rem
rem    MODIFIED   (MM/DD/YY)
rem    hyeh        08/10/99 - use sqlplus syntax
rem    mjaeger     07/14/99 - bug 808870: OCCS: convert tabs, no long lines
rem    cchau       08/18/97 - enable dictionary protection
rem    ramkrish    11/05/96 - addnal tests + misc cleanup
rem    ramkrish    11/04/96 - interface chg
rem    mchien      07/17/96 - created
rem

REM initialize environment
REMARK >>>> Set System Variables For Current SQLPlus Session <<<<
SET FEEDBACK 1
SET NUMWIDTH 10
SET PAGESIZE 24
SET LINESIZE 80
SET TRIMSPOOL ON
SET TAB OFF
SET DEFINE '^'

set echo on
connect sys/knl_test7 as sysdba;
drop user gpld cascade;

REM create a new user
grant connect, resource to gpld identified by gpld;

REM start session as new user
connect gpld/gpld
set serveroutput on

REM create test table
create table gpldt1(c1 number, c2 blob, c3 clob);

REM -----------------------.
REM  TEST DBMS_LOB ROUTINES
REM -----------------------.

REM ------------.
REM  TEST APPEND
REM ------------.
create or replace procedure tst_append is
  dblob blob;
  sblob blob;
  dclob clob;
  sclob clob;
  spos  integer;
  amt   binary_integer;
  bufb  raw(20);
  bufc  varchar2(20);
begin
  dbms_output.put_line('--------------- APPEND Begin ---------------');

  /* test append for BLOBs */
  select c2 into dblob from gpldt1 where c1 = 1 for update;
  select c2 into sblob from gpldt1 where c1 = 2;
  dbms_lob.append(dblob, sblob);

  /* test append for CLOBs */
  select c3 into dclob from gpldt1 where c1 = 1 for update;
  select c3 into sclob from gpldt1 where c1 = 2;
  dbms_lob.append(dclob, sclob);

  /* end Xn */
  commit;

  /* test sanity of APPEND operation */
  amt := 20;
  spos := 1;
  select c2 into sblob from gpldt1 where c1 = 1;
  dbms_lob.read(sblob, amt, spos, bufb);
  dbms_output.put_line('Result BLOB: ' || utl_raw.cast_to_varchar2(bufb));

  select c3 into sclob from gpldt1 where c1 = 1;
  dbms_lob.read(sclob, amt, spos, bufc);
  dbms_output.put_line('Result CLOB: ' || bufc);

  dbms_output.put_line('--------------- APPEND End   ---------------');
  dbms_output.put_line(' ');
  exception
    when no_data_found
    then dbms_output.put_line('APPEND operation error');
end tst_append;
/
show errors procedure tst_append;
/
insert into gpldt1 values (1,utl_raw.cast_to_raw('0123456789'),'abcdefghij');
insert into gpldt1 values (2,utl_raw.cast_to_raw('1234567890'),'bcdefghija');
commit;
begin
 tst_append;
end;
/

REM -------------.
REM  TEST COMPARE
REM -------------.
create or replace procedure tst_compare (amt in integer) is
  dblob blob;
  sblob blob;
  dclob clob;
  sclob clob;
  dpos  integer;
  spos  integer;
  ret   integer;
begin
  dbms_output.put_line('--------------- COMPARE Begin ---------------');

  /* test COMPARE for BLOBs */
  dpos := 1;
  spos := 1;
  select c2 into dblob from gpldt1 where c1 = 3;
  select c2 into sblob from gpldt1 where c1 = 4;
  ret := -1;
  ret := dbms_lob.compare(dblob, sblob, amt, dpos, spos);
  dbms_output.put_line('Return value for BLOB: ' || ret);

  /* test COMPARE for CLOBs */
  dpos := 1;
  spos := 1;
  select c3 into dclob from gpldt1 where c1 = 3;
  select c3 into sclob from gpldt1 where c1 = 4;
  ret := -1;
  ret := dbms_lob.compare(dclob, sclob, amt, dpos, spos);
  dbms_output.put_line('Return value for CLOB: ' || ret);

  dbms_output.put_line('--------------- COMPARE End   ---------------');
  dbms_output.put_line(' ');
end tst_compare;
/
show errors procedure tst_compare;
/
REM  Case 1: Identical Data
insert into gpldt1 values (3,utl_raw.cast_to_raw('2345678901'),'abcdefghij');
insert into gpldt1 values (4,utl_raw.cast_to_raw('2345678901'),'abcdefghij');
begin
 tst_compare(10);
end;
/
rollback;
REM  Case 2: lob 1 smaller than lob 2 - differing in the first byte
insert into gpldt1 values (3,utl_raw.cast_to_raw('2345678901'),'cdefghijab');
insert into gpldt1 values (4,utl_raw.cast_to_raw('4567890123'),'efghijabcd');
begin
 tst_compare(10);
end;
/
rollback;
REM  Case 3: lob 1 greater than lob 2 - differing in the first byte
insert into gpldt1 values (3,utl_raw.cast_to_raw('4567890123'),'efghijabcd');
insert into gpldt1 values (4,utl_raw.cast_to_raw('2345678901'),'cdefghijab');
begin
 tst_compare(10);
end;
/
rollback;
REM  Case 4: lob 1 greater than lob 2 - but with lob 2 being empty
insert into gpldt1 values (3,utl_raw.cast_to_raw('4567890123'),'efghijabcd');
insert into gpldt1 values (4, EMPTY_BLOB(), EMPTY_CLOB());
begin
 tst_compare(10);
end;
/
rollback;
REM  Case 5: enter the real world - identical lobs greater than 2K in size
insert into gpldt1 values (3, utl_raw.cast_to_raw('4567890123'),
                           lpad('efghijabcd', 3000, 'efghijabcd'));
insert into gpldt1 values (4, utl_raw.cast_to_raw('4567890123'),
                           lpad('efghijabcd', 3000, 'efghijabcd'));
begin
 tst_compare(3000);
end;
/
rollback;
REM  Case 6: enter the real world - lob1 < lob2 - lobs greater than 2K in size
insert into gpldt1 values (3, utl_raw.cast_to_raw('4567890123'),
                           lpad('efghijabcd', 3000, 'cdefghijab'));
insert into gpldt1 values (4, utl_raw.cast_to_raw('4567890123'),
                           lpad('efghijabcd', 3000, 'efghijabcd'));
begin
 tst_compare(3000);
end;
/
rollback;
REM  Case 7: enter the real world - lob1 > lob2 - lobs greater than 2K in size
insert into gpldt1 values (4, utl_raw.cast_to_raw('4567890123'),
                           lpad('efghijabcd', 3000, 'cdefghijab'));
insert into gpldt1 values (3, utl_raw.cast_to_raw('4567890123'),
                           lpad('efghijabcd', 3000, 'efghijabcd'));
begin
 tst_compare(3000);
end;
/
commit;

REM ----------.
REM  TEST COPY
REM ----------.
create or replace procedure tst_copy
 (amt in out integer, dpos in integer, spos in integer, ramt in out integer) is
  dblob blob;
  sblob blob;
  dclob clob;
  sclob clob;
  bufb  raw(30);
  bufc  varchar2(30);
begin
  dbms_output.put_line('--------------- COPY Begin ---------------');

  /* test COPY for BLOBs */
  select c2 into dblob from gpldt1 where c1 = 5 for update;
  select c2 into sblob from gpldt1 where c1 = 6;
  dbms_lob.copy(dblob, sblob, amt, dpos, spos);
  commit;

  /* test COPY for CLOBs */
  select c3 into dclob from gpldt1 where c1 = 5 for update;
  select c3 into sclob from gpldt1 where c1 = 6;
  dbms_lob.copy(dclob, sclob, amt, dpos, spos);
  commit;

  /* check sanity of COPY operation */
  select c2 into sblob from gpldt1 where c1 = 5;
  dbms_lob.read(sblob, ramt, 1, bufb);
  dbms_output.put_line('Result BLOB: ' || utl_raw.cast_to_varchar2(bufb));

  select c3 into sclob from gpldt1 where c1 = 5;
  dbms_lob.read(sclob, ramt, 1, bufc);
  dbms_output.put_line('Result CLOB: ' || bufc);

  dbms_output.put_line('--------------- COPY End   ---------------');
  dbms_output.put_line(' ');
  exception
    when no_data_found
    then dbms_output.put_line('no_data_found error in COPY');
end tst_copy;
/
show errors procedure tst_copy;
/
insert into gpldt1 values (5,utl_raw.cast_to_raw('4567890123'),'efghijabcd');
insert into gpldt1 values (6,utl_raw.cast_to_raw('5678901234'),'fghijabcde');
commit;
REM Case 1: Append lob2 to lob1
declare
 amt integer;
 ramt integer;
begin
 amt := 10;
 ramt := 20;
 tst_copy(amt, 11, 1, ramt);
end;
/
REM Case 2: Copy 10 bytes of lob2 over 6-EoL bytes of lob1
declare
 amt integer;
 ramt integer;
begin
 amt := 10;
 ramt := 15;
 tst_copy(amt, 6, 1, ramt);
end;
/
REM Case 3: Copy 10 bytes of lob2 starting from 16th position in lob1,
REM         creating a hole of 5 bytes.
declare
 amt integer;
 ramt integer;
begin
 amt := 10;
 ramt := 25;
 tst_copy(amt, 16, 1, ramt);
end;
/

REM -----------.
REM  TEST ERASE
REM -----------.
create or replace procedure tst_erase is
  dblob blob;
  dclob clob;
  amt   integer;
  pos   integer;
  bufb  raw(10);
  bufc  varchar2(10);
  amtread integer;
  posread integer;
  begin
  dbms_output.put_line('--------------- ERASE Begin ---------------');

  /* test ERASE for BLOBs */
  amt := 5;
  pos := 3;
  select c2 into dblob from gpldt1 where c1 = 7 for update;
  dbms_lob.erase(dblob, amt, pos);

  /* test ERASE for CLOBs */
  select c3 into dclob from gpldt1 where c1 = 7 for update;
  dbms_lob.erase(dclob, amt, pos);

  /* end Xn */
  commit;

  /* check the sanity of ERASE operation */

  /* fill in the hole so it can be printable */
  bufb := utl_raw.cast_to_raw('XXXXX');
  bufc := 'XXXXX';

  select c2 into dblob from gpldt1 where c1 = 7 for update;
  dbms_lob.write(dblob, amt, pos, bufb);
  select c2 into dblob from gpldt1 where c1 = 7;
  amtread := 10;
  posread := 1;
  dbms_lob.read(dblob, amtread, posread, bufb);
  dbms_output.put_line('Result BLOB: ' || utl_raw.cast_to_varchar2(bufb));

  select c3 into dclob from gpldt1 where c1 = 7 for update;
  dbms_lob.write(dclob, amt, pos, bufc);
  select c3 into dclob from gpldt1 where c1 = 7;
  dbms_lob.read(dclob, amtread, posread, bufc);

  dbms_output.put_line('Result CLOB: ' || bufc);

  dbms_output.put_line('--------------- ERASE End   ---------------');
  dbms_output.put_line(' ');
  exception
    when no_data_found
    then dbms_output.put_line('COPY operation has some problems');
end tst_erase;
/
show errors procedure tst_erase;
/
insert into gpldt1 values (7,utl_raw.cast_to_raw('6789012345'),'ghijabcdef');
commit;
begin
 tst_erase;
end;
/

REM -----------.
REM  TEST INSTR
REM -----------.
create or replace procedure tst_instr
 (occ in integer, pos in integer, pat in integer) is
  dblob blob;
  dclob clob;
  bufb  raw(5);
  bufc  varchar2(5);
  retb  integer;
  retc  integer;
begin
  dbms_output.put_line('--------------- INSTR Begin ---------------');

  if (pat = 0) then
    begin
     bufb := utl_raw.cast_to_raw('01234');
     bufc := 'abcde';
    end;
  else
    begin
     bufb := utl_raw.cast_to_raw('43210');
     bufc := 'edcba';
    end;
  end if;
 /* test INSTR for BLOBs */
  select c2 into dblob from gpldt1 where c1 = 8;

  retb := dbms_lob.instr(dblob, bufb, pos, occ);
  dbms_output.put_line('Pos: ' || retb);

  /* test INSTR for CLOBs */
  select c3 into dclob from gpldt1 where c1 = 8;

  retc := dbms_lob.instr(dclob, bufc, pos, occ);
  dbms_output.put_line('Pos: ' || retc);
  dbms_output.put_line('--------------- INSTR End   ---------------');
  dbms_output.put_line(' ');
end tst_instr;
/
show errors procedure tst_instr;
/
insert into gpldt1 values (8,utl_raw.cast_to_raw('7890123456'),'hijabcdefg');
REM Case 1: first occurrence starting from first byte
declare
 occ integer;
 pos integer;
begin
 occ := 1;
 pos := 1;
 tst_instr(occ, pos, 0);
end;
/
 REM Case 2: starting offset at last byte of LOB
declare
 occ integer;
 pos integer;
begin
 occ := 1;
 pos := 10;
 tst_instr(occ, pos, 0);
end;
/
  REM Case 3: occurrence not present
declare
 occ integer;
 pos integer;
begin
 occ := 2;
 pos := 1;
 tst_instr(occ, pos, 0);
end;
/
REM Case 4: no matching string
declare
 occ integer;
 pos integer;
begin
 occ := 1;
 pos := 1;
 tst_instr(occ, pos, 1);
end;
/
rollback;
insert into gpldt1 values (8,utl_raw.cast_to_raw('0123401234'),'abcdeabcde');
commit;
REM Case 5: occurrence > 1
declare
 occ integer;
 pos integer;
begin
 occ := 2;
 pos := 1;
 tst_instr(occ, pos, 0);
end;
/

REM ------------.
REM  TEST LENGTH
REM ------------.
create or replace procedure tst_length is
  dblob blob;
  dclob clob;
  lenb  integer;
  lenc  integer;
  c4val varchar2(32);
begin
  dbms_output.put_line('--------------- LENGTH Begin ---------------');

  /* test LENGTH for BLOBs */
  select c2 into dblob from gpldt1 where c1 = 9;
  lenb := dbms_lob.getlength(dblob);
  dbms_output.put_line('Length of BLOB is: ' || lenb);

  /* test LENGTH for CLOBs */
  select c3 into dclob from gpldt1 where c1 = 9;
  lenc := dbms_lob.getlength(dclob);
  dbms_output.put_line('Length of CLOB is: ' || lenc);

  dbms_output.put_line('--------------- LENGTH End   ---------------');
  dbms_output.put_line(' ');
end tst_length;
/
show errors procedure tst_length;
/
insert into gpldt1 values (9,utl_raw.cast_to_raw('8901234567'),'ijabcdefgh');
commit;
begin
 tst_length;
end;
/

REM ----------.
REM  TEST READ
REM ----------.
create or replace procedure tst_read(amt in out integer, pos in integer) is
  dblob blob;
  dclob clob;
  bufb  raw(10);
  bufc  varchar2(10);
begin
  dbms_output.put_line('--------------- READ Begin ---------------');

  /* test READ for BLOBs */
  select c2 into dblob from gpldt1 where c1 = 10;
  dbms_lob.read(dblob, amt, pos, bufb);
  dbms_output.put_line('BLOB Read buffer: '|| utl_raw.cast_to_varchar2(bufb));

  /* test READ for CLOBs */
  select c3 into dclob from gpldt1 where c1 = 10;
  dbms_lob.read(dclob, amt, pos, bufc);
  dbms_output.put_line('CLOB Read buffer: '|| bufc);

  dbms_output.put_line('--------------- READ End   ---------------');
  dbms_output.put_line(' ');
  exception
    when dbms_lob.invalid_argval
    then dbms_output.put_line('Error in argument value');
    when no_data_found
    then dbms_output.put_line('End of LOB reached');

end tst_read;
/
show errors procedure tst_read;
/
insert into gpldt1 values (10,utl_raw.cast_to_raw('9012345678'),'jabcdefghi');
commit;
REM Case 1: Read 10 bytes starting from offset 1
declare
 amt integer;
 pos integer;
begin
 amt := 10;
 pos := 1;
 tst_read(amt, pos);
end;
/
REM Case 2: Read 10 bytes starting from offset 5
declare
 amt integer;
 pos integer;
begin
 amt := 10;
 pos := 5;
 tst_read(amt, pos);
end;
/
REM Case 3: Read 10 bytes starting from offset 10 - last byte
declare
 amt integer;
 pos integer;
begin
 amt := 10;
 pos := 10;
 tst_read(amt, pos);
end;
/
REM Case 4: Read 10 bytes starting from offset 11 - end of LOB
declare
 amt integer;
 pos integer;
begin
 amt := 10;
 pos := 11;
 tst_read(amt, pos);
end;
/
REM Case 5: Read 15 bytes into a 10 byte buffer - error
declare
 amt integer;
 pos integer;
begin
 amt := 15;
 pos := 1;
 tst_read(amt, pos);
end;
/

REM ------------.
REM  TEST SUBSTR
REM ------------.
create or replace procedure tst_substr (amt in integer, pos in integer) is
  dblob blob;
  dclob clob;
  bufb  raw(10);
  bufc  varchar2(10);
begin
  dbms_output.put_line('--------------- SUBSTR Begin ---------------');

  /* test SUBSTR for BLOBs */
  select c2 into dblob from gpldt1 where c1 = 11;
  bufb := '';
  bufb := dbms_lob.substr(dblob, amt, pos);
  dbms_output.put_line('SUBSTR BLOB: ' || utl_raw.cast_to_varchar2 (bufb));

  /* test SUBSTR for CLOBs */
  select c3 into dclob from gpldt1 where c1 = 11;
  bufc := '';
  bufc := dbms_lob.substr(dclob, amt, pos);
  dbms_output.put_line('SUBSTR CLOB: ' || bufc);

  dbms_output.put_line('--------------- SUBSTR End   ---------------');
  dbms_output.put_line(' ');
end tst_substr;
/
show errors procedure tst_substr;
/
insert into gpldt1 values (11,utl_raw.cast_to_raw('0123456789'),'abcdefghij');
REM  Case 1: Substr 10 bytes starting from offset 1
declare
 amt integer;
 pos integer;
begin
 amt := 10;
 pos := 1;
 tst_substr(amt, pos);
end;
/
REM  Case 2: Substr 10 bytes starting from offset 5
declare
 amt integer;
 pos integer;
begin
 amt := 10;
 pos := 5;
 tst_substr(amt, pos);
end;
/
REM  Case 3: Substr 10 bytes starting from offset 10 - last byte
declare
 amt integer;
 pos integer;
begin
 amt := 10;
 pos := 10;
 tst_substr(amt, pos);
end;
/
REM  Case 4: Substr 10 bytes starting from offset 11 - end of LOB
declare
 amt integer;
 pos integer;
begin
 amt := 10;
 pos := 11;
 tst_substr(amt, pos);
end;
/
rollback;
insert into gpldt1 values (11,utl_raw.cast_to_raw('012345678901234'),
                           'abcdefghijabcde');
commit;
declare
 amt integer;
 pos integer;
begin
 -- Case 5: Substr 15 bytes into a 10 byte buffer - PL/SQL error
 amt := 15;
 pos := 1;
 tst_substr(amt, pos);
end;
/

REM ----------.
REM  TEST TRIM
REM ----------.
create or replace procedure tst_trim (nlen in out integer) is
  dblob blob;
  dclob clob;
  bufb  raw(5);
  bufc  varchar2(5);
begin
  dbms_output.put_line('--------------- TRIM Begin ---------------');

  /* test TRIM for BLOBs */
  select c2 into dblob from gpldt1 where c1 = 12 for update;
  dbms_lob.trim(dblob, nlen);

  /* test TRIM for CLOBs */
  select c3 into dclob from gpldt1 where c1 = 12 for update;
  dbms_lob.trim(dclob, nlen);

  /* end Xn */
  commit;

  /* check sanity of TRIM operation */
  select c2 into dblob from gpldt1 where c1 = 12;
  dbms_lob.read(dblob, nlen, 1, bufb);
  dbms_output.put_line('Result BLOB: ' || utl_raw.cast_to_varchar2(bufb));

  select c3 into dclob from gpldt1 where c1 = 12;
  dbms_lob.read(dclob, nlen, 1, bufc);
  dbms_output.put_line('Result CLOB: ' || bufc);

  dbms_output.put_line('--------------- TRIM End   ---------------');
  dbms_output.put_line(' ');
end tst_trim;
/
show errors procedure tst_trim;
/
insert into gpldt1 values (12,utl_raw.cast_to_raw('1234567890'),'bcdefghija');
commit;
REM  Case 1: trim to a length of 5 bytes
declare
 nlen integer;
begin
 nlen := 5;
 tst_trim(nlen);
end;
/
delete from gpldt1 where c1 = 12;
insert into gpldt1 values (12,utl_raw.cast_to_raw('1234567890'),'bcdefghija');
REM  Case 2: trim to a length of 15 bytes, 5 greater than what is present
declare
 nlen integer;
begin
 nlen := 15;
 tst_trim(nlen);
end;
/

REM -----------.
REM  TEST WRITE
REM -----------.
create or replace procedure tst_write
 (amt in out integer, pos in integer, pat in integer) is
  dblob blob;
  dclob clob;
  bufb  raw(20);
  bufc  varchar2(20);
  ramt integer;
begin
  dbms_output.put_line('--------------- WRITE Begin ---------------');

  if (pat = 1) then
  /*  Cases 1 - 3, and 5 */
    begin
      bufb := utl_raw.cast_to_raw('65432');
      bufc := 'gfedc';
    end;
  else begin
  /* Case 4: */
      bufb := utl_raw.cast_to_raw('0123456789012345678901234');
      bufc := 'abcdefghijabcdefghijabcde';
    end;
  end if;
  /* test WRITE for BLOBs */

  select c2 into dblob from gpldt1 where c1 = 13 for update;
  dbms_lob.write(dblob, amt, pos, bufb);

  /* test WRITE for CLOBs */
  select c3 into dclob from gpldt1 where c1 = 13 for update;
  dbms_lob.write(dclob, amt, pos, bufc);

  /* end Xn */
  commit;

  /* check sanity of WRITE operation */

  select c2 into dblob from gpldt1 where c1 = 13;
  ramt := dbms_lob.getlength(dblob);
  dbms_lob.read(dblob, ramt, 1, bufb);
  if (pos < 12) then
    dbms_output.put_line('Result BLOB: ' || utl_raw.cast_to_varchar2(bufb));
  end if;
  dbms_output.put_line('Length is: ' || ramt);

  select c3 into dclob from gpldt1 where c1 = 13;
  ramt := dbms_lob.getlength(dclob);
  dbms_lob.read(dclob, ramt, 1, bufc);
  if (pos < 12) then
     dbms_output.put_line('Result CLOB: ' || bufc);
  end if;
  dbms_output.put_line('Length is: ' || ramt);

  dbms_output.put_line('--------------- WRITE End   ---------------');
  dbms_output.put_line(' ');
end tst_write;
/
show errors procedure tst_write;
/
insert into gpldt1 values (13,utl_raw.cast_to_raw('2345678901'),'cdefghijab');
commit;
REM  Case 1: write 5 bytes starting from 6th byte
declare
 amt integer;
 pos integer;
begin
 amt := 5;
 pos := 6;
 tst_write(amt, pos, 1);
end;
/
delete from gpldt1 where c1 = 13;
insert into gpldt1 values (13,utl_raw.cast_to_raw('2345678901'),'cdefghijab');
commit;
REM  Case 2: append 5 bytes starting from the 11th byte
declare
 amt integer;
 pos integer;
begin
 amt := 5;
 pos := 11;
 tst_write(amt, pos, 1);
end;
/
delete from gpldt1 where c1 = 13;
insert into gpldt1 values (13,utl_raw.cast_to_raw('2345678901'),'cdefghijab');
commit;
REM  Case 3: append 5 bytes starting from the 16th byte - 5 byte hole
declare
 amt integer;
 pos integer;
begin
 amt := 5;
 pos := 16;
 tst_write(amt, pos, 1);
end;
/
declare
 amt integer;
 pos integer;
begin
 -- Now fill up the 5 byte hole that starts from the 11th byte
 amt := 5;
 pos := 11;
 tst_write(amt, pos, 1);
end;
/
delete from gpldt1 where c1 = 13;
insert into gpldt1 values (13,utl_raw.cast_to_raw('2345678901'),'cdefghijab');
commit;
REM  Case 4: write 25 bytes from a 20 byte buffer - PL/SQL error
declare
 amt integer;
 pos integer;
begin
 amt := 25;
 pos := 1;
 tst_write(amt, pos, 0);
end;
/
REM  Case 5: specify 25 bytes, but provide only 5 bytes from a 20 byte buffer
declare
 amt integer;
 pos integer;
begin
 amt := 25;
 pos := 1;
 tst_write(amt, pos, 1);
end;
/

REM --------------------.
REM  TEST DATA_INTEGRITY
REM --------------------.
create or replace procedure tst_data_integrity is
  dblob blob;
  dclob clob;
  amt   integer;
  pos   integer;
  bufb  raw(10);
  bufc  varchar2(10);
begin
  dbms_output.put_line('--------------- DATA_INTEGRITY Begin ---------------');

  /* WRITE data begin */
  amt := 5;
  pos := 1;
  select c2 into dblob from gpldt1 where c1 = 14 for update;
  bufb := utl_raw.cast_to_raw('98765');
  dbms_lob.write(dblob, amt, pos, bufb);

  select c3 into dclob from gpldt1 where c1 = 14 for update;
  bufc := 'jihgf';
  dbms_lob.write(dclob, amt, pos, bufc);

  /* READ data begin */
  amt := 10;
  pos := 1;
  select c2, c3 into dblob, dclob from gpldt1 where c1 = 14;
  dbms_lob.read(dblob, amt, pos, bufb);
  dbms_output.put_line('Output BLOB 14: ' || utl_raw.cast_to_varchar2(bufb));

  dbms_lob.read(dclob, amt, pos, bufc);
  dbms_output.put_line('Output CLOB 14: ' || bufc);

  dbms_output.put_line('BEFORE UPDATE');
  select c2, c3 into dblob, dclob from gpldt1 where c1 = 11 for update;
  dbms_lob.read(dblob, amt, pos, bufb);
  dbms_output.put_line('Output BLOB 11: ' || utl_raw.cast_to_varchar2(bufb));

  dbms_lob.read(dclob, amt, pos, bufc);
  dbms_output.put_line('Output CLOB 11: ' || bufc);

  select c2, c3 into dblob, dclob from gpldt1 where c1 = 14;
  update gpldt1 set c2 = dblob, c3 = dclob where c1 = 11;
  commit;

  dbms_output.put_line('AFTER UPDATE1');
  select c2, c3 into dblob, dclob from gpldt1 where c1 = 11;
  dbms_lob.read(dblob, amt, pos, bufb);
  dbms_output.put_line('Output BLOB 11: ' || utl_raw.cast_to_varchar2(bufb));

  dbms_lob.read(dclob, amt, pos, bufc);
  dbms_output.put_line('Output CLOB 11: ' || bufc);

  update gpldt1 set c2 = (select c2 from gpldt1 where c1 = 14),
                    c3 = (select c3 from gpldt1 where c1 = 14)
          where c1 = 11;
  commit;

  dbms_output.put_line('AFTER UPDATE2');
  select c2 into dblob from gpldt1 where c1 = 11;
  dbms_lob.read(dblob, amt, pos, bufb);
  dbms_output.put_line('Output BLOB 11: ' || utl_raw.cast_to_varchar2(bufb));

  select c3 into dclob from gpldt1 where c1 = 11;
  dbms_lob.read(dclob, amt, pos, bufc);
  dbms_output.put_line('Output CLOB 11: ' || bufc);

  dbms_output.put_line('--------------- DATA_INTEGRITY End   ---------------');
  dbms_output.put_line(' ');
end tst_data_integrity;
/
show errors procedure tst_data_integrity;
/
insert into gpldt1 values (14,utl_raw.cast_to_raw('3456789012'),'defghijabc');
commit;
begin
 tst_data_integrity;
end;
/

REM ----------------.
REM  TEST EXCEPTIONS
REM ----------------.
declare
  dblob blob;
  sblob blob;
begin
  dbms_output.put_line('--------------- exception append --------------');
  dblob := NULL;
  sblob := NULL;
  dbms_lob.append(dblob, sblob);
  exception
    when value_error
    then dbms_output.put_line('APPEND: Value Error');
end;
/
declare
  dblob blob;
  sblob blob;
  amt integer;
  posd integer;
  poss integer;
  ret  integer;
begin
  dbms_output.put_line('--------------- exception compare -------------');
  dblob := NULL;
  sblob := NULL;
  amt := 1;
  posd := 1;
  poss := 1;
  ret := 1;
  ret := dbms_lob.compare(dblob, sblob, amt, posd, poss);
  if (ret != NULL)
  then
    dbms_output.put_line('COMPARE: Incorrect error - ret: ' || ret);
  else
    dbms_output.put_line('COMPARE: Invalid Inputs' || ret);
  end if;
end;
/
declare
  dblob blob;
  sblob blob;
  amt integer;
  posd integer;
  poss integer;
  ret  integer;
begin
  dbms_output.put_line('--------------- exception compare -------------');
  select c2 into dblob from gpldt1 where c1 = 12;
  select c2 into sblob from gpldt1 where c1 = 13;
  amt := 0;
  posd := 0;
  poss := 0;
  ret := dbms_lob.compare(dblob, sblob, amt, posd, poss);
  if (ret != NULL)
  then
    dbms_output.put_line('COMPARE: Incorrect error - ret: ' || ret);
  else
    dbms_output.put_line('COMPARE: Invalid Inputs' || ret);
  end if;
end;
/
declare
  dblob blob;
  sblob blob;
  amt integer;
  posd integer;
  poss integer;
  ret  integer;
begin
  dbms_output.put_line('--------------- exception compare -------------');
  select c2 into dblob from gpldt1 where c1 = 12;
  select c2 into sblob from gpldt1 where c1 = 13;
  amt := -1;
  posd := -1;
  poss := -1;
  ret := dbms_lob.compare(dblob, sblob, amt, posd, poss);
  if (ret != NULL)
  then
    dbms_output.put_line('COMPARE: Incorrect error - ret: ' || ret);
  else
    dbms_output.put_line('COMPARE: Invalid Inputs' || ret);
  end if;
end;
/
declare
  dblob blob;
  sblob blob;
  amt integer;
  posd integer;
  poss integer;
  ret  integer;
begin
  dbms_output.put_line('--------------- exception compare -------------');
  select c2 into dblob from gpldt1 where c1 = 12;
  select c2 into sblob from gpldt1 where c1 = 13;
  amt := 4294967295+1;
  posd := 4294967295+1;
  poss := 4294967295+1;
  ret := dbms_lob.compare(dblob, sblob, amt, posd, poss);
  if (ret != NULL)
  then
    dbms_output.put_line('COMPARE: Incorrect error - ret: ' || ret);
  else
    dbms_output.put_line('COMPARE: Invalid Inputs' || ret);
  end if;
end;
/
declare
  dblob blob;
  sblob blob;
  amt integer;
  posd integer;
  poss integer;
  ret  integer;
begin
  dbms_output.put_line('--------------- exception copy -------------');
  dblob := NULL;
  sblob := NULL;
  amt := 1;
  posd := 1;
  poss := 1;
  dbms_lob.copy(dblob, sblob, amt, posd, poss);
  exception
  when value_error
  then dbms_output.put_line('COPY: Value Error');
  when dbms_lob.invalid_argval
  then dbms_output.put_line('COPY: Invalid argument value');
  when dbms_lob.access_error
  then dbms_output.put_line('COPY: Data overrun error');
end;
/
declare
  dblob blob;
  sblob blob;
  amt integer;
  posd integer;
  poss integer;
  ret  integer;
begin
  dbms_output.put_line('--------------- exception copy -------------');
  select c2 into dblob from gpldt1 where c1 = 12 for update;
  select c2 into sblob from gpldt1 where c1 = 13;
  amt := NULL;
  posd := NULL;
  poss := NULL;
  dbms_lob.copy(dblob, sblob, amt, posd, poss);
  exception
  when value_error
  then dbms_output.put_line('COPY: Value Error');
  when dbms_lob.invalid_argval
  then dbms_output.put_line('COPY: Invalid argument value');
  when dbms_lob.access_error
  then dbms_output.put_line('COPY: Data overrun error');
end;
/
declare
  dblob blob;
  sblob blob;
  amt integer;
  posd integer;
  poss integer;
  ret  integer;
begin
  dbms_output.put_line('--------------- exception copy -------------');
  select c2 into dblob from gpldt1 where c1 = 12 for update;
  select c2 into sblob from gpldt1 where c1 = 13;
  amt := 0;
  posd := 0;
  poss := 0;
  dbms_lob.copy(dblob, sblob, amt, posd, poss);
  exception
  when value_error
  then dbms_output.put_line('COPY: Value Error');
  when dbms_lob.invalid_argval
  then dbms_output.put_line('COPY: Invalid argument value');
  when dbms_lob.access_error
  then dbms_output.put_line('COPY: Data overrun error');
end;
/
declare
  dblob blob;
  sblob blob;
  amt integer;
  posd integer;
  poss integer;
  ret  integer;
begin
  dbms_output.put_line('--------------- exception copy -------------');
  select c2 into dblob from gpldt1 where c1 = 12 for update;
  select c2 into sblob from gpldt1 where c1 = 13;
  amt := -1;
  posd := -1;
  poss := -1;
  dbms_lob.copy(dblob, sblob, amt, posd, poss);
  exception
  when value_error
  then dbms_output.put_line('COPY: Value Error');
  when dbms_lob.invalid_argval
  then dbms_output.put_line('COPY: Invalid argument value');
  when dbms_lob.access_error
  then dbms_output.put_line('COPY: Data overrun error');
end;
/
declare
  dblob blob;
  sblob blob;
  amt integer;
  posd integer;
  poss integer;
  ret  integer;
begin
  dbms_output.put_line('--------------- exception copy -------------');
  select c2 into dblob from gpldt1 where c1 = 12 for update;
  select c2 into sblob from gpldt1 where c1 = 13;
  amt := 4294967295+1;
  posd := 4294967295+1;
  poss := 4294967295+1;
  dbms_lob.copy(dblob, sblob, amt, posd, poss);
  exception
  when value_error
  then dbms_output.put_line('COPY: Value Error');
  when dbms_lob.invalid_argval
  then dbms_output.put_line('COPY: Invalid argument value');
  when dbms_lob.access_error
  then dbms_output.put_line('COPY: Data overrun error');
end;
/
declare
  dblob blob;
  amt   integer;
  pos   integer;
begin
  dbms_output.put_line('--------------- exception erase ----------------');
  dblob := NULL;
  amt := 1;
  pos := 1;
  dbms_lob.erase(dblob, amt, pos);
  exception
    when value_error
    then dbms_output.put_line('ERASE: Value Error');
    when dbms_lob.invalid_argval
    then dbms_output.put_line('ERASE: Invalid argument value');
    when dbms_lob.access_error
    then dbms_output.put_line('ERASE: Data overrun error');
end;
/
declare
  dblob blob;
  amt   integer;
  pos   integer;
begin
  dbms_output.put_line('--------------- exception erase ----------------');
  select c2 into dblob from gpldt1 where c1 = 12 for update;
  amt := NULL;
  pos := NULL;
  dbms_lob.erase(dblob, amt, pos);
  exception
    when value_error
    then dbms_output.put_line('ERASE: Value Error');
    when dbms_lob.invalid_argval
    then dbms_output.put_line('ERASE: Invalid argument value');
    when dbms_lob.access_error
    then dbms_output.put_line('ERASE: Data overrun error');
end;
/
declare
  dblob blob;
  amt   integer;
  pos   integer;
begin
  dbms_output.put_line('--------------- exception erase ----------------');
  select c2 into dblob from gpldt1 where c1 = 12 for update;
  amt := 0;
  pos := 0;
  dbms_lob.erase(dblob, amt, pos);
  exception
    when value_error
    then dbms_output.put_line('ERASE: Value Error');
    when dbms_lob.invalid_argval
    then dbms_output.put_line('ERASE: Invalid argument value');
    when dbms_lob.access_error
    then dbms_output.put_line('ERASE: Data overrun error');
end;
/
declare
  dblob blob;
  amt   integer;
  pos   integer;
begin
  dbms_output.put_line('--------------- exception erase ----------------');
  select c2 into dblob from gpldt1 where c1 = 12 for update;
  amt := -1;
  pos := -1;
  dbms_lob.erase(dblob, amt, pos);
  exception
    when value_error
    then dbms_output.put_line('ERASE: Value Error');
    when dbms_lob.invalid_argval
    then dbms_output.put_line('ERASE: Invalid argument value');
    when dbms_lob.access_error
    then dbms_output.put_line('ERASE: Data overrun error');
end;
/
declare
  dblob blob;
  amt   integer;
  pos   integer;
begin
  dbms_output.put_line('--------------- exception erase ----------------');
  select c2 into dblob from gpldt1 where c1 = 12 for update;
  amt := 4294967295+1;
  pos := 4294967295+1;
  dbms_lob.erase(dblob, amt, pos);
  exception
    when value_error
    then dbms_output.put_line('ERASE: Value Error');
    when dbms_lob.invalid_argval
    then dbms_output.put_line('ERASE: Invalid argument value');
    when dbms_lob.access_error
    then dbms_output.put_line('ERASE: Data overrun error');
end;
/
declare
  dblob blob;
  dclob clob;
  bufb  raw(10);
  bufc  varchar2(10);
  pos   integer;
  occ   integer;
  ret   integer;
begin
  dbms_output.put_line('--------------- exception instr ------------');
  dblob := NULL;
  dclob := NULL;
  bufb := '12345';
  bufc := 'abcde';
  pos := 1;
  occ := 1;
  ret := dbms_lob.instr(dblob, bufb, pos, occ);
  if (ret != NULL)
  then
    dbms_output.put_line('INSTR: Incorrect error handling');
  else
    dbms_output.put_line('INSTR: Invalid Inputs');
  end if;
  ret := dbms_lob.instr(dclob, bufc, pos, occ);
  if (ret != NULL)
  then
    dbms_output.put_line('INSTR: Incorrect error handling');
  else
    dbms_output.put_line('INSTR: Invalid Inputs');
  end if;
end;
/
declare
  dblob blob;
  dclob clob;
  bufb  raw(10);
  bufc  varchar2(10);
  pos   integer;
  occ   integer;
  ret   integer;
begin
  dbms_output.put_line('--------------- exception instr ------------');
  select c2 into dblob from gpldt1 where c1 = 12;
  select c3 into dclob from gpldt1 where c1 = 12;
  bufb := '12345';
  bufc := 'abcde';
  occ := NULL;
  pos := NULL;
  ret := dbms_lob.instr(dblob, bufb, pos, occ);
  if (ret != NULL)
  then
    dbms_output.put_line('INSTR: Incorrect error handling');
  else
    dbms_output.put_line('INSTR: Invalid Inputs');
  end if;
  ret := dbms_lob.instr(dclob, bufc, pos, occ);
  if (ret != NULL)
  then
    dbms_output.put_line('INSTR: Incorrect error handling');
  else
    dbms_output.put_line('INSTR: Invalid Inputs');
  end if;
end;
/
declare
  dblob blob;
  dclob clob;
  bufb  raw(10);
  bufc  varchar2(10);
  pos   integer;
  occ   integer;
  ret   integer;
begin
  dbms_output.put_line('--------------- exception instr ------------');
  select c2 into dblob from gpldt1 where c1 = 12;
  select c3 into dclob from gpldt1 where c1 = 12;
  bufb := '12345';
  bufc := 'abcde';
  occ := 0;
  pos := 0;
  ret := dbms_lob.instr(dblob, bufb, pos, occ);
  if (ret != NULL)
  then
    dbms_output.put_line('INSTR: Incorrect error handling');
  else
    dbms_output.put_line('INSTR: Invalid Inputs');
  end if;
  ret := dbms_lob.instr(dclob, bufc, pos, occ);
  if (ret != NULL)
  then
    dbms_output.put_line('INSTR: Incorrect error handling');
  else
    dbms_output.put_line('INSTR: Invalid Inputs');
  end if;
end;
/
declare
  dblob blob;
  dclob clob;
  bufb  raw(10);
  bufc  varchar2(10);
  pos   integer;
  occ   integer;
  ret   integer;
begin
  dbms_output.put_line('--------------- exception instr ------------');
  select c2 into dblob from gpldt1 where c1 = 12;
  select c3 into dclob from gpldt1 where c1 = 12;
  bufb := '12345';
  bufc := 'abcde';
  occ := -1;
  pos := -1;
  ret := dbms_lob.instr(dblob, bufb, pos, occ);
  if (ret != NULL)
  then
    dbms_output.put_line('INSTR: Incorrect error handling');
  else
    dbms_output.put_line('INSTR: Invalid Inputs');
  end if;
  ret := dbms_lob.instr(dclob, bufc, pos, occ);
  if (ret != NULL)
  then
    dbms_output.put_line('INSTR: Incorrect error handling');
  else
    dbms_output.put_line('INSTR: Invalid Inputs');
  end if;
end;
/
declare
  dblob blob;
  dclob clob;
  bufb  raw(10);
  bufc  varchar2(10);
  pos   integer;
  occ   integer;
  ret   integer;
begin
  dbms_output.put_line('--------------- exception instr ------------');
  select c2 into dblob from gpldt1 where c1 = 12;
  select c3 into dclob from gpldt1 where c1 = 12;
  bufb := '12345';
  bufc := 'abcde';
  occ := 4294967295+1;
  pos := 4294967295+1;
  ret := dbms_lob.instr(dblob, bufb, pos, occ);
  if (ret != NULL)
  then
    dbms_output.put_line('INSTR: Incorrect error handling');
  else
    dbms_output.put_line('INSTR: Invalid Inputs');
  end if;
  ret := dbms_lob.instr(dclob, bufc, pos, occ);
  if (ret != NULL)
  then
    dbms_output.put_line('INSTR: Incorrect error handling');
  else
    dbms_output.put_line('INSTR: Invalid Inputs');
  end if;
end;
/
declare
  dblob blob;
  dclob clob;
  ret   integer;
begin
  dbms_output.put_line('--------------- exception length ------------');
  dblob := NULL;
  dclob := NULL;
  ret := dbms_lob.getlength(dblob);
  if (ret != NULL)
  then
    dbms_output.put_line('LENGTH: Incorrect error handling');
  else
    dbms_output.put_line('LENGTH: NULL Input');
  end if;
  ret := dbms_lob.getlength(dclob);
  if (ret != NULL)
  then
    dbms_output.put_line('LENGTH: Incorrect error handling');
  else
    dbms_output.put_line('LENGTH: NULL Input');
  end if;
end;
/
declare
  dblob blob;
  bufb  raw(10);
  amt   integer;
  pos   integer;
begin
  dbms_output.put_line('--------------- exception read --------------');
  dblob := NULL;
  dbms_lob.read(dblob, amt, pos, bufb);
  exception
    when value_error
    then dbms_output.put_line('READ: Value Error');
    when dbms_lob.invalid_argval
    then dbms_output.put_line('READ: Invalid argument value');
    when dbms_lob.access_error
    then dbms_output.put_line('READ: Data overrun error');
end;
/
declare
  dblob blob;
  bufb  raw(10);
  amt   integer;
  pos   integer;
begin
  dbms_output.put_line('--------------- exception read --------------');
  select c2 into dblob from gpldt1 where c1 = 12;
  amt := NULL;
  pos := NULL;
  dbms_lob.read(dblob, amt, pos, bufb);
  exception
    when value_error
    then dbms_output.put_line('READ: Value Error');
    when dbms_lob.invalid_argval
    then dbms_output.put_line('READ: Invalid argument value');
    when dbms_lob.access_error
    then dbms_output.put_line('READ: Data overrun error');
end;
/
declare
  dblob blob;
  bufb  raw(10);
  amt   integer;
  pos   integer;
begin
  dbms_output.put_line('--------------- exception read --------------');
  select c2 into dblob from gpldt1 where c1 = 12;
  amt := 0;
  pos := 0;
  dbms_lob.read(dblob, amt, pos, bufb);
  exception
    when value_error
    then dbms_output.put_line('READ: Value Error');
    when dbms_lob.invalid_argval
    then dbms_output.put_line('READ: Invalid argument value');
    when dbms_lob.access_error
    then dbms_output.put_line('READ: Data overrun error');
end;
/
declare
  dblob blob;
  bufb  raw(10);
  amt   integer;
  pos   integer;
begin
  dbms_output.put_line('--------------- exception read --------------');
  select c2 into dblob from gpldt1 where c1 = 12;
  amt := -1;
  pos := -1;
  dbms_lob.read(dblob, amt, pos, bufb);
  exception
    when value_error
    then dbms_output.put_line('READ: Value Error');
    when dbms_lob.invalid_argval
    then dbms_output.put_line('READ: Invalid argument value');
    when dbms_lob.access_error
    then dbms_output.put_line('READ: Data overrun error');
end;
/
declare
  dblob blob;
  bufb  raw(10);
  amt   integer;
  pos   integer;
begin
  dbms_output.put_line('--------------- exception read --------------');
  select c2 into dblob from gpldt1 where c1 = 12;
  amt := 32767+1;
  pos := 1;
  dbms_lob.read(dblob, amt, pos, bufb);
  exception
    when value_error
    then dbms_output.put_line('READ: Value Error');
    when dbms_lob.invalid_argval
    then dbms_output.put_line('READ: Invalid argument value');
    when dbms_lob.access_error
    then dbms_output.put_line('READ: Data overrun error');
end;
/
declare
  dblob blob;
  bufb  raw(10);
  amt   integer;
  pos   integer;
begin
  dbms_output.put_line('--------------- exception read --------------');
  select c2 into dblob from gpldt1 where c1 = 12;
  amt := 1;
  pos := 4294967295+1;
  dbms_lob.read(dblob, amt, pos, bufb);
  exception
    when value_error
    then dbms_output.put_line('READ: Value Error');
    when dbms_lob.invalid_argval
    then dbms_output.put_line('READ: Invalid argument value');
    when dbms_lob.access_error
    then dbms_output.put_line('READ: Data overrun error');
end;
/
declare
  dblob blob;
  bufb  raw(10);
  amt   integer;
  pos   integer;
begin
  dbms_output.put_line('--------------- exception read --------------');
  select c2 into dblob from gpldt1 where c1 = 12;
  amt := 1;
  pos := 4294967295;
  bufb := NULL;
  dbms_lob.read(dblob, amt, pos, bufb);
  exception
    when value_error
    then dbms_output.put_line('READ: Value Error');
    when dbms_lob.invalid_argval
    then dbms_output.put_line('READ: Invalid argument value');
    when dbms_lob.access_error
    then dbms_output.put_line('READ: Data overrun error');
end;
/
REM  Empty LOB in SUBSTR
select c1, dbms_lob.substr(empty_blob()), dbms_lob.substr(empty_clob())
  from gpldt1;

declare
  dblob blob;
  bufb  raw(10);
  amt   integer;
  pos   integer;
begin
  dbms_output.put_line('--------------- exception substr ------------');
  dblob := NULL;
  amt := 1;
  pos := 1;
  bufb := dbms_lob.substr(dblob, amt, pos);
  if (bufb != NULL)
  then
    dbms_output.put_line('SUBSTR: Incorrect error handling');
  else
    dbms_output.put_line('SUBSTR: Invalid Inputs');
  end if;
end;
/
declare
  dblob blob;
  bufb  raw(10);
  amt   integer;
  pos   integer;
begin
  dbms_output.put_line('--------------- exception substr ------------');
  select c2 into dblob from gpldt1 where c1 = 12;
  amt := NULL;
  pos := NULL;
  bufb := dbms_lob.substr(dblob, amt, pos);
  if (bufb != NULL)
  then
    dbms_output.put_line('SUBSTR: Incorrect error handling');
  else
    dbms_output.put_line('SUBSTR: Invalid Inputs');
  end if;
end;
/
declare
  dblob blob;
  bufb  raw(10);
  amt   integer;
  pos   integer;
begin
  dbms_output.put_line('--------------- exception substr ------------');
  select c2 into dblob from gpldt1 where c1 = 12;
  amt := 0;
  pos := 0;
  bufb := dbms_lob.substr(dblob, amt, pos);
  if (bufb != NULL)
  then
    dbms_output.put_line('SUBSTR: Incorrect error handling');
  else
    dbms_output.put_line('SUBSTR: Invalid Inputs');
  end if;
end;
/
declare
  dblob blob;
  bufb  raw(10);
  amt   integer;
  pos   integer;
begin
  dbms_output.put_line('--------------- exception substr ------------');
  select c2 into dblob from gpldt1 where c1 = 12;
  amt := -1;
  pos := -1;
  bufb := dbms_lob.substr(dblob, amt, pos);
  if (bufb != NULL)
  then
    dbms_output.put_line('SUBSTR: Incorrect error handling');
  else
    dbms_output.put_line('SUBSTR: Invalid Inputs');
  end if;
end;
/
declare
  dblob blob;
  bufb  raw(10);
  amt   integer;
  pos   integer;
begin
  dbms_output.put_line('--------------- exception substr ------------');
  select c2 into dblob from gpldt1 where c1 = 12;
  amt := 4294967295+1;
  pos := 4294967295+1;
  bufb := dbms_lob.substr(dblob, amt, pos);
  if (bufb != NULL)
  then
    dbms_output.put_line('SUBSTR: Incorrect error handling');
  else
    dbms_output.put_line('SUBSTR: Invalid Inputs');
  end if;
end;
/
declare
  dblob blob;
  bufb  raw(10);
  amt   integer;
  pos   integer;
begin
  dbms_output.put_line('--------------- exception substr ------------');
  select c2 into dblob from gpldt1 where c1 = 12;
  amt := 4294967295;
  pos := 4294967295;
  bufb := NULL;
  bufb := dbms_lob.substr(dblob, amt, pos);
  if (bufb != NULL)
  then
    dbms_output.put_line('SUBSTR: Incorrect error handling');
  else
    dbms_output.put_line('SUBSTR: Invalid Inputs');
  end if;
end;
/
declare
  dblob blob;
  bufb  raw(10);
  amt   integer;
  pos   integer;
begin
  dbms_output.put_line('--------------- exception substr ------------');
  select c2 into dblob from gpldt1 where c1 = 12;
  amt := 4294967295;
  pos := 4294967295;
  bufb := NULL;
  bufb := dbms_lob.substr(dblob, amt, pos);
  if (bufb != NULL)
  then
    dbms_output.put_line('SUBSTR: Incorrect error handling');
  else
    dbms_output.put_line('SUBSTR: Invalid Inputs');
  end if;
end;
/
declare
  dblob blob;
  len   integer;
begin
  dbms_output.put_line('--------------- exception trim ------------');
  dblob := NULL;
  len := 1;
  dbms_lob.trim(dblob, len);
  exception
    when value_error
    then dbms_output.put_line('TRIM: Value Error');
    when dbms_lob.invalid_argval
    then dbms_output.put_line('TRIM: Invalid argument value');
    when dbms_lob.access_error
    then dbms_output.put_line('TRIM: Data overrun error');
end;
/
declare
  dblob blob;
  len   integer;
begin
  dbms_output.put_line('--------------- exception trim ------------');
  select c2 into dblob from gpldt1 where c1 = 12 for update;
  len := NULL;
  dbms_lob.trim(dblob, len);
  exception
    when value_error
    then dbms_output.put_line('TRIM: Value Error');
    when dbms_lob.invalid_argval
    then dbms_output.put_line('TRIM: Invalid argument value');
    when dbms_lob.access_error
    then dbms_output.put_line('TRIM: Data overrun error');
end;
/
declare
  dblob blob;
  len   integer;
begin
  dbms_output.put_line('--------------- exception trim ------------');
  select c2 into dblob from gpldt1 where c1 = 12 for update;
  len := 0;
  dbms_lob.trim(dblob, len);
  exception
    when value_error
    then dbms_output.put_line('TRIM: Value Error');
    when dbms_lob.invalid_argval
    then dbms_output.put_line('TRIM: Invalid argument value');
    when dbms_lob.access_error
    then dbms_output.put_line('TRIM: Data overrun error');
end;
/
declare
  dblob blob;
  len   integer;
begin
  dbms_output.put_line('--------------- exception trim ------------');
  select c2 into dblob from gpldt1 where c1 = 12 for update;
  len := -1;
  dbms_lob.trim(dblob, len);
  exception
    when value_error
    then dbms_output.put_line('TRIM: Value Error');
    when dbms_lob.invalid_argval
    then dbms_output.put_line('TRIM: Invalid argument value');
    when dbms_lob.access_error
    then dbms_output.put_line('TRIM: Data overrun error');
end;
/
declare
  dblob blob;
  len   integer;
begin
  dbms_output.put_line('--------------- exception trim ------------');
  select c2 into dblob from gpldt1 where c1 = 12 for update;
  len := 4294967295+1;
  dbms_lob.trim(dblob, len);
  exception
    when value_error
    then dbms_output.put_line('TRIM: Value Error');
    when dbms_lob.invalid_argval
    then dbms_output.put_line('TRIM: Invalid argument value');
    when dbms_lob.access_error
    then dbms_output.put_line('TRIM: Data overrun error');
end;
/
declare
  dblob blob;
  bufb  raw(10);
  amt   integer;
  pos   integer;
begin
  dbms_output.put_line('--------------- exception write -------------');
  dblob := NULL;
  bufb := '12345';
  pos := 1;
  amt := 1;
  dbms_lob.write(dblob, amt, pos, bufb);
  exception
    when value_error
    then dbms_output.put_line('WRITE: Value Error');
    when dbms_lob.invalid_argval
    then dbms_output.put_line('WRITE: Invalid argument value');
    when dbms_lob.access_error
    then dbms_output.put_line('WRITE: Data overrun error');
end;
/
declare
  dblob blob;
  bufb  raw(10);
  amt   integer;
  pos   integer;
begin
  dbms_output.put_line('--------------- exception write -------------');
  select c2 into dblob from gpldt1 where c1 = 12 for update;
  bufb := '12345';
  pos := NULL;
  amt := NULL;
  dbms_lob.write(dblob, amt, pos, bufb);
  exception
    when value_error
    then dbms_output.put_line('WRITE: Value Error');
    when dbms_lob.invalid_argval
    then dbms_output.put_line('WRITE: Invalid argument value');
    when dbms_lob.access_error
    then dbms_output.put_line('WRITE: Data overrun error');
end;
/
declare
  dblob blob;
  bufb  raw(10);
  amt   integer;
  pos   integer;
begin
  dbms_output.put_line('--------------- exception write -------------');
  select c2 into dblob from gpldt1 where c1 = 12 for update;
  bufb := '12345';
  pos := 0;
  amt := 0;
  dbms_lob.write(dblob, amt, pos, bufb);
  exception
    when value_error
    then dbms_output.put_line('WRITE: Value Error');
    when dbms_lob.invalid_argval
    then dbms_output.put_line('WRITE: Invalid argument value');
    when dbms_lob.access_error
    then dbms_output.put_line('WRITE: Data overrun error');
end;
/
declare
  dblob blob;
  bufb  raw(10);
  amt   integer;
  pos   integer;
begin
  dbms_output.put_line('--------------- exception write -------------');
  select c2 into dblob from gpldt1 where c1 = 12 for update;
  bufb := '12345';
  pos := -1;
  amt := -1;
  dbms_lob.write(dblob, amt, pos, bufb);
  exception
    when value_error
    then dbms_output.put_line('WRITE: Value Error');
    when dbms_lob.invalid_argval
    then dbms_output.put_line('WRITE: Invalid argument value');
    when dbms_lob.access_error
    then dbms_output.put_line('WRITE: Data overrun error');
end;
/
declare
  dblob blob;
  bufb  raw(10);
  amt   integer;
  pos   integer;
begin
  dbms_output.put_line('--------------- exception write -------------');
  select c2 into dblob from gpldt1 where c1 = 12 for update;
  bufb := '12345';
  amt := 1;
  pos := 4294967295+1;
  dbms_lob.write(dblob, amt, pos, bufb);
  exception
    when value_error
    then dbms_output.put_line('WRITE: Value Error');
    when dbms_lob.invalid_argval
    then dbms_output.put_line('WRITE: Invalid argument value');
    when dbms_lob.access_error
    then dbms_output.put_line('WRITE: Data overrun error');
end;
/
declare
  dblob blob;
  bufb  raw(10);
  amt   integer;
  pos   integer;
begin
  dbms_output.put_line('--------------- exception write -------------');
  select c2 into dblob from gpldt1 where c1 = 12 for update;
  bufb := '12345';
  amt := 32767+1;
  pos := 1;
  dbms_lob.write(dblob, amt, pos, bufb);
  exception
    when value_error
    then dbms_output.put_line('WRITE: Value Error');
    when dbms_lob.invalid_argval
    then dbms_output.put_line('WRITE: Invalid argument value');
    when dbms_lob.access_error
    then dbms_output.put_line('WRITE: Data overrun error');
end;
/
declare
  dblob blob;
  bufb  raw(10);
  amt   integer;
  pos   integer;
begin
  dbms_output.put_line('--------------- exception write -------------');
  select c2 into dblob from gpldt1 where c1 = 12 for update;
  bufb := '';
  amt := 1;
  pos := 1;
  dbms_lob.write(dblob, amt, pos, bufb);
  exception
    when value_error
    then dbms_output.put_line('WRITE: Value Error');
    when dbms_lob.invalid_argval
    then dbms_output.put_line('WRITE: Invalid argument value');
    when dbms_lob.access_error
    then dbms_output.put_line('WRITE: Data overrun error');
end;
/
declare
  dblob blob;
  bufb  raw(10);
  amt   integer;
  pos   integer;
begin
  dbms_output.put_line('--------------- exception write -------------');
  select c2 into dblob from gpldt1 where c1 = 12 for update;
  bufb := NULL;
  amt := 1;
  pos := 1;
  dbms_lob.write(dblob, amt, pos, bufb);
  exception
    when value_error
    then dbms_output.put_line('WRITE: Value Error');
    when dbms_lob.invalid_argval
    then dbms_output.put_line('WRITE: Invalid argument value');
    when dbms_lob.access_error
    then dbms_output.put_line('WRITE: Data overrun error');
end;
/

REM ###################
REM  TERMINATE and EXIT
REM ###################
connect sys/knl_test7 as sysdba;
drop user gpld cascade;
set serveroutput off;
set echo off;
exit;
