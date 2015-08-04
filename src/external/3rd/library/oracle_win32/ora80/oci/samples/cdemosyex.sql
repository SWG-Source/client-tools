rem
rem $Header: cdemosyex.sql 14-jul-99.13:54:39 mjaeger Exp $
rem
rem cdemosyex.sql
rem
rem Copyright (c) 1998, 1999, Oracle Corporation.  All rights reserved.
rem
rem    NAME
rem      cdemosyex.sql - C DEMO for executing SYstem EVents
rem
rem    DESCRIPTION
rem      This SQL script causes the triggers, defined in cdemosyev.sql,
rem      to fire. This results in enqueueing of message(s) into queue(s).
rem      If there are subscription defined on the queue that are satisfied,
rem      a list of recipients are computed. All registered recipients are
rem      notified by invoking callback functions.
rem
rem    NOTES
rem      <other useful comments, qualifications, etc.>
rem
rem    MODIFIED   (MM/DD/YY)
rem    mjaeger     07/14/99 - bug 808870: OCCS: convert tabs, no long lines
rem    sasuri      11/23/98 - add shutdown notification
rem    sasuri      06/16/98 - system event demo files
rem    sasuri      06/16/98 - Created
rem

connect event/event
drop table foo;
create table foo(n number);
drop table foo;
connect foo/bar
connect pubsub/pubsub
drop table foo;
create table foo(n number);
drop table foo;
disconnect
connect internal
shutdown
connect internal
startup
shutdown
