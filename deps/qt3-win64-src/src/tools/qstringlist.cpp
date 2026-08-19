/****************************************************************************
** $Id: qstringlist.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of QStringList
**
** Created : 990406
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech ASA of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qstringlist.h"

#ifndef QT_NO_STRINGLIST
#include "qregexp.h"
#include "qstrlist.h"
#include "qdatastream.h"
#include "qtl.h"

/*!
    \class QStringList qstringlist.h
    \reentrant
    \brief The QStringList class provides a list of strings.

    \ingroup tools
    \ingroup shared
    \ingroup text
    \mainclass

    It is used to store and manipulate strings that logically belong
    together. Essentially QStringList is a QValueList of QString
    objects. Unlike QStrList, which stores pointers to characters,
    QStringList holds real QString objects. It is the class of choice
    whenever you work with Unicode strings. QStringList is part of the
    \link qtl.html Qt Template Library\endlink.

    Like QString itself, QStringList objects are implicitly shared, so
    passing them around as value-parameters is both fast and safe.

    Strings can be added to a list using append(), operator+=() or
    operator<<(), e.g.
    \code
    QStringList fonts;
    fonts.append( "Times" );
    fonts += "Courier";
    fonts += "Courier New";
    fonts << "Helvetica [Cronyx]" << "Helvetica [Adobe]";
    \endcode

    String lists have an iterator, QStringList::Iterator(), e.g.
    \code
    for ( QStringList::Iterator it = fonts.begin(); it != fonts.end(); ++it ) {
	cout << *it << ":";
    }
    cout << endl;
    // Output:
    //	Times:Courier:Courier New:Helvetica [Cronyx]:Helvetica [Adobe]:
    \endcode

    Many Qt functions return string lists by value; to iterate over
    these you should make a copy and iterate over the copy.

    You can concatenate all the strings in a string list into a single
    string (with an optional separator) using join(), e.g.
    \code
    QString allFonts = fonts.join( ", " );
    cout << allFonts << endl;
    // Output:
    //	Times, Courier, Courier New, Helvetica [Cronyx], Helvetica [Adobe]
    \endcode

    You can sort the list with sort(), and extract a new list which
    contains only those strings which contain a particular substring
    (or match a particular regular expression) using the grep()
    functions, e.g.
    \code
    fonts.sort();
    cout << fonts.join( ", " ) << endl;
    // Output:
    //	Courier, Courier New, Helvetica [Adobe], Helvetica [Cronyx], Times

    QStringList helveticas = fonts.grep( "Helvetica" );
    cout << helveticas.join( ", " ) << endl;
    // Output:
    //	Helvetica [Adobe], Helvetica [Cronyx]
    \endcode

    Existing strings can be split into string lists with character,
    string or regular expression separators, e.g.
    \code
    QString s = "Red\tGreen\tBlue";
    QStringList colors = QStringList::split( "\t", s );
    cout << colors.join( ", " ) << endl;
    // Output:
    //	Red, Green, Blue
    \endcode
*/

/*!
    \fn QStringList::QStringList()

    Creates an empty string list.
*/

/*!
    \fn QStringList::QStringList( const QStringList& l )

    Creates a copy of the list \a l. This function is very fast
    because QStringList is implicitly shared. In most situations this
    acts like a deep copy, for example, if this list or the original
    one or some other list referencing the same shared data is
    modified, the modifying list first makes a copy, i.e.
    copy-on-write.
    In a threaded environment you may require a real deep copy
    \omit see \l QDeepCopy\endomit.
*/

/*!
    \fn QStringList::QStringList (const QString & i)

    Constructs a string list consisting of the single string \a i.
    Longer lists are easily created as follows:

    \code
    QStringList items;
    items << "Buy" << "Sell" << "Update" << "Value";
    \endcode
*/

/*!
    \fn QStringList::QStringList (const char* i)

    Constructs a string list consisting of the single Latin-1 string \a i.
*/

/*!
    \fn QStringList::QStringList( const QValueList<QString>& l )

    Constructs a new string list that is a copy of \a l.
*/

/*!
    Sorts the list of strings in ascending case-sensitive order.

    Sorting is very fast. It uses the \link qtl.html Qt Template
    Library's\endlink efficient HeapSort implementation that has a
    time complexity of O(n*log n).

    If you want to sort your strings in an arbitrary order consider
    using a QMap. For example you could use a QMap\<QString,QString\>
    to create a case-insensitive ordering (e.g. mapping the lowercase
    text to the text), or a QMap\<int,QString\> to sort the strings by
    some integer index, etc.
*/
void QStringList::sort()
{
    qHeapSort( *this );
}

/*!
    \overload

    This version of the function uses a QChar as separator, rather
    than a regular expression.

    \sa join() QString::section()
*/

QStringList QStringList::split( const QChar &sep, const QString &str,
				bool allowEmptyEntries )
{
    return split( QString(sep), str, allowEmptyEntries );
}

/*!
    \overload

    This version of the function uses a QString as separator, rather
    than a regular expression.

    If \a sep is an empty string, the return value is a list of
    one-character strings: split( QString( "" ), "four" ) returns the
    four-item list, "f", "o", "u", "r".

    If \a allowEmptyEntries is TRUE, a null string is inserted in
    the list wherever the separator matches twice without intervening
    text.

    \sa join() QString::section()
*/

QStringList QStringList::split( const QString &sep, const QString &str,
				bool allowEmptyEntries )
{
    QStringList lst;

    int j = 0;
    int i = str.find( sep, j );

    while ( i != -1 ) {
	if ( i > j && i <= (int)str.length() )
	    lst << str.mid( j, i - j );
	else if ( allowEmptyEntries )
	    lst << QString::null;
	j = i + sep.length();
	i = str.find( sep, sep.length() > 0 ? j : j+1 );
    }

    int l = str.length() - 1;
    if ( str.mid( j, l - j + 1 ).length() > 0 )
	lst << str.mid( j, l - j + 1 );
    else if ( allowEmptyEntries )
	lst << QString::null;

    return lst;
}

#ifndef QT_NO_REGEXP
/*!
    Splits the string \a str into strings wherever the regular
    expression \a sep occurs, and returns the list of those strings.

    If \a allowEmptyEntries is TRUE, a null string is inserted in
    the list wherever the separator matches twice without intervening
    text.

    For example, if you split the string "a,,b,c" on commas, split()
    returns the three-item list "a", "b", "c" if \a allowEmptyEntries
    is FALSE (the default), and the four-item list "a", "", "b", "c"
    if \a allowEmptyEntries is TRUE.

    If \a sep does not match anywhere in \a str, split() returns a
    single element list with the element containing the single string
    \a str.

    \sa join() QString::section()
*/

QStringList QStringList::split( const QRegExp &sep, const QString &str,
				bool allowEmptyEntries )
{
    QStringList lst;

    QRegExp tep = sep;

    int j = 0;
    int i = tep.search( str, j );

    while ( i != -1 ) {
	if ( str.mid( j, i - j ).length() > 0 )
	    lst << str.mid( j, i - j );
	else if ( allowEmptyEntries )
	    lst << QString::null;
	if ( tep.matchedLength() == 0 )
	    j = i + 1;
	else
	    j = i + tep.matchedLength();
	i = tep.search( str, j );
    }

    int l = str.length() - 1;
    if ( str.mid( j, l - j + 1 ).length() > 0 )
	lst << str.mid( j, l - j + 1 );
    else if ( allowEmptyEntries )
	lst << QString::null;

    return lst;
}
#endif

/*!
    Returns a list of all the strings containing the substring \a str.

    If \a cs is TRUE, the grep is done case-sensitively; otherwise
    case is ignored.

    \code
    QStringList list;
    list << "Bill Gates" << "John Doe" << "Bill Clinton";
    list = list.grep( "Bill" );
    // list == ["Bill Gates", "Bill Clinton"]
    \endcode

    \sa QString::find()
*/

QStringList QStringList::grep( const QString &str, bool cs ) const
{
    QStringList res;
    for ( QStringList::ConstIterator it = begin(); it != end(); ++it )
	if ( (*it).contains(str, cs) )
	    res << *it;

    return res;
}

#ifndef QT_NO_REGEXP
/*!
    \overload

    Returns a list of all the strings that match the regular
    expression \a rx.

    \sa QString::find()
*/

QStringList QStringList::grep( const QRegExp &rx ) const
{
    QStringList res;
    for ( QStringList::ConstIterator it = begin(); it != end(); ++it )
	if ( (*it).find(rx) != -1 )
	    res << *it;

    return res;
}
#endif

/*!
    Replaces every occurrence of the string \a before in the strings
    that constitute the string list with the string \a after. Returns
    a reference to the string list.

    If \a cs is TRUE, the search is case sensitive; otherwise the
    search is case insensitive.

    Example:
    \code
    QStringList list;
    list << "alpha" << "beta" << "gamma" << "epsilon";
    list.gres( "a", "o" );
    // list == ["olpho", "beto", "gommo", "epsilon"]
    \endcode

    \sa QString::replace()
*/
QStringList& QStringList::gres( const QString &before, const QString &after,
				bool cs )
{
    QStringList::Iterator it = begin();
    while ( it != end() ) {
	(*it).replace( before, after, cs );
	++it;
    }
    return *this;
}

#ifndef QT_NO_REGEXP_CAPTURE
/*!
    \overload

    Replaces every occurrence of the regexp \a rx in the string
    with \a after. Returns a reference to the string list.

    Example:
    \code
    QStringList list;
    list << "alpha" << "beta" << "gamma" << "epsilon";
    list.gres( QRegExp("^a"), "o" );
    // list == ["olpha", "beta", "gamma", "epsilon"]
    \endcode

    For regexps containing \link qregexp.html#capturing-text
    capturing parentheses \endlink, occurrences of <b>\\1</b>,
    <b>\\2</b>, ..., in \a after are replaced with \a{rx}.cap(1),
    cap(2), ...

    Example:
    \code
    QStringList list;
    list << "Bill Clinton" << "Gates, Bill";
    list.gres( QRegExp("^(.*), (.*)$"), "\\2 \\1" );
    // list == ["Bill Clinton", "Bill Gates"]
    \endcode

    \sa QString::replace()
*/
QStringList& QStringList::gres( const QRegExp &rx, const QString &after )
{
    QStringList::Iterator it = begin();
    while ( it != end() ) {
	(*it).replace( rx, after );
	++it;
    }
    return *this;
}

#endif

/*!
    Joins the string list into a single string with each element
    separated by the string \a sep (which can be empty).

    \sa split()
*/
QString QStringList::join( const QString &sep ) const
{
    QString res;
    bool alredy = FALSE;
    for ( QStringList::ConstIterator it = begin(); it != end(); ++it ) {
	if ( alredy )
	    res += sep;
	alredy = TRUE;
	res += *it;
    }

    return res;
}

#ifndef QT_NO_DATASTREAM
Q_EXPORT QDataStream &operator>>( QDataStream & s, QStringList& l )
{
    return s >> (QValueList<QString>&)l;
}

Q_EXPORT QDataStream &operator<<( QDataStream & s, const QStringList& l )
{
    return s << (const QValueList<QString>&)l;
}
#endif

/*!
    Converts from an ASCII-QStrList \a ascii to a QStringList (Unicode).
*/
QStringList QStringList::fromStrList(const QStrList& ascii)
{
    QStringList res;
    const char * s;
    for ( QStrListIterator it(ascii); (s=it.current()); ++it )
	res << s;
    return res;
}

/*! \fn void QStringList::detach()
    \reimp
*/


#endif //QT_NO_STRINGLIST
