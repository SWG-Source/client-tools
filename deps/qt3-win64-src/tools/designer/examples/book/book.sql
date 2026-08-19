-- The following SQL generates the database 
-- used by the 'book' example programs
-- Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
--
-- This file is part of an example program for Qt.  This example
-- program may be used, distributed and modified without limitation.

DROP TABLE author;
DROP TABLE book;
DROP TABLE sequence;

CREATE TABLE author 
( id integer primary key, 
forename varchar(40), 
surname varchar(40) );

CREATE TABLE book 
( id integer primary key, 
title varchar(40), 
price numeric(10,2), 
authorid integer, 
notes varchar(255) );

create index book_authorid_idx on book( authorid );

CREATE TABLE sequence
( tablename varchar(10),
sequence numeric);

INSERT INTO author VALUES ( 0, 'Philip K', 'Dick' );
INSERT INTO author VALUES ( 1, 'Robert', 'Heinlein' );
INSERT INTO author VALUES ( 2, 'Sarah', 'Paretsky' );

INSERT INTO book VALUES ( 0, 'The Man Who Japed', 6.99, 0, 'A good book' );
INSERT INTO book VALUES ( 1, 'The Man in the High Castle', 9.99, 0, 'Worth reading' );
INSERT INTO book VALUES ( 2, 'The Number of the Beast', 8.99, 1, 'Get this!' );
INSERT INTO book VALUES ( 3, 'Indemnity Only', 9.99, 2, 'Cool' );
INSERT INTO book VALUES ( 4, 'Burn Marks', 9.99, 2, 'Need to make notes' );
INSERT INTO book VALUES ( 5, 'Deadlock', 9.99, 2, 'Hmmm..' );

INSERT INTO sequence VALUES ( 'author', 2 );
INSERT INTO sequence VALUES ( 'book', 5 );
