/**********************************************************************
** Copyright (C) 2000-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of Qt Linguist.
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
** See http://www.trolltech.com/gpl/ for GPL licensing information.
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "simtexth.h"

#include <metatranslator.h>

#include <qcstring.h>
#include <qdict.h>
#include <qmap.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qvaluelist.h>

#include <string.h>

typedef QValueList<MetaTranslatorMessage> TML;

/*
  How similar are two texts?  The approach used here relies on co-occurrence
  matrices and is very efficient.

  Let's see with an example: how similar are "here" and "hither"?  The
  co-occurrence matrix M for "here" is M[h,e] = 1, M[e,r] = 1, M[r,e] = 1, and 0
  elsewhere; the matrix N for "hither" is N[h,i] = 1, N[i,t] = 1, ...,
  N[h,e] = 1, N[e,r] = 1, and 0 elsewhere.  The union U of both matrices is the
  matrix U[i,j] = max { M[i,j], N[i,j] }, and the intersection V is
  V[i,j] = min { M[i,j], N[i,j] }.  The score for a pair of texts is

      score = (sum of V[i,j] over all i, j) / (sum of U[i,j] over all i, j),

  a formula suggested by Arnt Gulbrandsen.  Here we have

      score = 2 / 6,

  or one third.

  The implementation differs from this in a few details.  Most importantly,
  repetitions are ignored; for input "xxx", M[x,x] equals 1, not 2.
*/

/*
  Every character is assigned to one of 20 buckets so that the co-occurrence
  matrix requires only 20 * 20 = 400 bits, not 256 * 256 = 65536 bits or even
  more if we want the whole Unicode.  Which character falls in which bucket is
  arbitrary.

  The second half of the table is a replica of the first half, because of
  laziness.
*/
static const int indexOf[256] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
//      !   "   #   $   %   &   '   (   )   *   +   ,   -   .   /
    0,  2,  6,  7,  10, 12, 15, 19, 2,  6,  7,  10, 12, 15, 19, 0,
//  0   1   2   3   4   5   6   7   8   9   :   ;   <   =   >   ?
    1,  3,  4,  5,  8,  9,  11, 13, 14, 16, 2,  6,  7,  10, 12, 15,
//  @   A   B   C   D   E   F   G   H   I   J   K   L   M   N   O
    0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  6,  10, 11, 12, 13, 14,
//  P   Q   R   S   T   U   V   W   X   Y   Z   [   \   ]   ^   _
    15, 12, 16, 17, 18, 19, 2,  10, 15, 7,  19, 2,  6,  7,  10, 0,
//  `   a   b   c   d   e   f   g   h   i   j   k   l   m   n   o
    0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  6,  10, 11, 12, 13, 14,
//  p   q   r   s   t   u   v   w   x   y   z   {   |   }   ~
    15, 12, 16, 17, 18, 19, 2,  10, 15, 7,  19, 2,  6,  7,  10, 0,

    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  2,  6,  7,  10, 12, 15, 19, 2,  6,  7,  10, 12, 15, 19, 0,
    1,  3,  4,  5,  8,  9,  11, 13, 14, 16, 2,  6,  7,  10, 12, 15,
    0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  6,  10, 11, 12, 13, 14,
    15, 12, 16, 17, 18, 19, 2,  10, 15, 7,  19, 2,  6,  7,  10, 0,
    0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  6,  10, 11, 12, 13, 14,
    15, 12, 16, 17, 18, 19, 2,  10, 15, 7,  19, 2,  6,  7,  10, 0
};

/*
  The entry bitCount[i] (for i between 0 and 255) is the number of bits used to
  represent i in binary.
*/
static const int bitCount[256] = {
    0,  1,  1,  2,  1,  2,  2,  3,  1,  2,  2,  3,  2,  3,  3,  4,
    1,  2,  2,  3,  2,  3,  3,  4,  2,  3,  3,  4,  3,  4,  4,  5,
    1,  2,  2,  3,  2,  3,  3,  4,  2,  3,  3,  4,  3,  4,  4,  5,
    2,  3,  3,  4,  3,  4,  4,  5,  3,  4,  4,  5,  4,  5,  5,  6,
    1,  2,  2,  3,  2,  3,  3,  4,  2,  3,  3,  4,  3,  4,  4,  5,
    2,  3,  3,  4,  3,  4,  4,  5,  3,  4,  4,  5,  4,  5,  5,  6,
    2,  3,  3,  4,  3,  4,  4,  5,  3,  4,  4,  5,  4,  5,  5,  6,
    3,  4,  4,  5,  4,  5,  5,  6,  4,  5,  5,  6,  5,  6,  6,  7,
    1,  2,  2,  3,  2,  3,  3,  4,  2,  3,  3,  4,  3,  4,  4,  5,
    2,  3,  3,  4,  3,  4,  4,  5,  3,  4,  4,  5,  4,  5,  5,  6,
    2,  3,  3,  4,  3,  4,  4,  5,  3,  4,  4,  5,  4,  5,  5,  6,
    3,  4,  4,  5,  4,  5,  5,  6,  4,  5,  5,  6,  5,  6,  6,  7,
    2,  3,  3,  4,  3,  4,  4,  5,  3,  4,  4,  5,  4,  5,  5,  6,
    3,  4,  4,  5,  4,  5,  5,  6,  4,  5,  5,  6,  5,  6,  6,  7,
    3,  4,  4,  5,  4,  5,  5,  6,  4,  5,  5,  6,  5,  6,  6,  7,
    4,  5,  5,  6,  5,  6,  6,  7,  5,  6,  6,  7,  6,  7,  7,  8
};

struct CoMatrix
{
    /*
      The matrix has 20 * 20 = 400 entries.  This requires 50 bytes, or 13
      words.  Some operations are performed on words for more efficiency.
    */
    union {
	Q_UINT8 b[52];
	Q_UINT32 w[13];
    };

    CoMatrix() { memset( b, 0, 52 ); }
    CoMatrix( const char *text ) {
	char c = '\0', d;
	memset( b, 0, 52 );
	/*
	  The Knuth books are not in the office only for show; they help make
	  loops 30% faster and 20% as readable.
	*/
	while ( (d = *text) != '\0' ) {
	    setCoocc( c, d );
	    if ( (c = *++text) != '\0' ) {
		setCoocc( d, c );
		text++;
	    }
	}
    }

    void setCoocc( char c, char d ) {
	int k = indexOf[(uchar) c] + 20 * indexOf[(uchar) d];
	b[k >> 3] |= k & 0x7;
    }

    int worth() const {
	int w = 0;
	for ( int i = 0; i < 50; i++ )
	    w += bitCount[b[i]];
	return w;
    }
};

static inline CoMatrix reunion( const CoMatrix& m, const CoMatrix& n )
{
    CoMatrix p;
    for ( int i = 0; i < 13; i++ )
	p.w[i] = m.w[i] | n.w[i];
    return p;
}

static inline CoMatrix intersection( const CoMatrix& m, const CoMatrix& n )
{
    CoMatrix p;
    for ( int i = 0; i < 13; i++ )
	p.w[i] = m.w[i] & n.w[i];
    return p;
}

CandidateList similarTextHeuristicCandidates( const MetaTranslator *tor,
					    const char *text,
					    int maxCandidates )
{
    QValueList<int> scores;
    CandidateList candidates;
    CoMatrix cmTarget( text );
    int targetLen = qstrlen( text );

    TML all = tor->translatedMessages();
    TML::Iterator it;

    for ( it = all.begin(); it != all.end(); ++it ) {
	if ( (*it).type() == MetaTranslatorMessage::Unfinished ||
	     (*it).translation().isEmpty() )
	    continue;

	QString s = tor->toUnicode( (*it).sourceText(), (*it).utf8() );
	CoMatrix cm( s.latin1() );
	int delta = QABS( (int) s.length() - targetLen );

	/*
	  Here is the score formula. A comment above contains a
	  discussion on a similar (but simpler) formula.
	*/
	int score = ( (intersection(cm, cmTarget).worth() + 1) << 10 ) /
		    ( reunion(cm, cmTarget).worth() + (delta << 1) + 1 );

	if ( (int) candidates.count() == maxCandidates &&
	     score > scores[maxCandidates - 1] )
	    candidates.remove( candidates.last() );
	if ( (int) candidates.count() < maxCandidates && score >= 190 ) {
	    Candidate cand( s, (*it).translation() );

	    int i;
	    for ( i = 0; i < (int) candidates.count(); i++ ) {
		if ( score >= scores[i] ) {
		    if ( score == scores[i] ) {
			if ( candidates[i] == cand )
			    goto continue_outer_loop;
		    } else {
			break;
		    }
		}
	    }
	    scores.insert( scores.at(i), score );
	    candidates.insert( candidates.at(i), cand );
	}
    continue_outer_loop:
	;
    }
    return candidates;
}
