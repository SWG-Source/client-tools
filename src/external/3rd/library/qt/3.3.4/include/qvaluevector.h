/****************************************************************************
** $Id: qt/qvaluevector.h   3.3.4   edited May 27 2003 $
**
** Definition of QValueVector class
**
** Copyright (C) 1992-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
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

#ifndef QVALUEVECTOR_H
#define QVALUEVECTOR_H

#ifndef QT_H
#include "qtl.h"
#include "qshared.h"
#include "qdatastream.h"
#endif // QT_H

#ifndef QT_NO_STL
#include <vector>
#endif

template <class T>
class QValueVectorPrivate : public QShared
{
public:
    typedef T value_type;
    typedef T* pointer;

    QValueVectorPrivate()
	: start( 0 ), finish( 0 ), end( 0 )
    {
    }

    QValueVectorPrivate( const QValueVectorPrivate<T>& x );
    QValueVectorPrivate( size_t size );

    void derefAndDelete() // work-around for hp-cc
    {
	if ( deref() )
	    delete this;
    }

#if defined(Q_TEMPLATEDLL)
    // Workaround MS bug in memory de/allocation in DLL vs. EXE
    virtual
#endif
    ~QValueVectorPrivate()
    {
	delete[] start;
    }

    size_t size() const
    {
	return finish - start;
    }

    bool empty() const
    {
	return start == finish;
    }

    size_t capacity() const
    {
	return end - start;
    }

    void insert( pointer pos, const T& x );
    void insert( pointer pos, size_t n, const T& x );
    void reserve( size_t n );

    void clear()
    {
	delete[] start;
	start = 0;
	finish = 0;
	end = 0;
    }


    pointer start;
    pointer finish;
    pointer end;

private:
    pointer growAndCopy( size_t n, pointer s, pointer f );

    QValueVectorPrivate<T>& operator=( const QValueVectorPrivate<T>& x );

};

template <class T>
Q_INLINE_TEMPLATES QValueVectorPrivate<T>::QValueVectorPrivate( const QValueVectorPrivate<T>& x )
    : QShared()
{
    int i = x.size();
    if ( i > 0 ) {
	start = new T[ i ];
	finish = start + i;
	end = start + i;
#if defined(__xlC__) && __xlC__ < 0x400 // xlC 3.6 confused by const
	qCopy( (pointer)x.start, (pointer)x.finish, start );
#else
	qCopy( x.start, x.finish, start );
#endif
    } else {
	start = 0;
	finish = 0;
	end = 0;
    }
}

template <class T>
Q_INLINE_TEMPLATES QValueVectorPrivate<T>::QValueVectorPrivate( size_t size )
{
    if ( size > 0 ) {
	start = new T[size];
	finish = start + size;
	end = start + size;
    } else {
	start = 0;
	finish = 0;
	end = 0;
    }
}

template <class T>
Q_INLINE_TEMPLATES void QValueVectorPrivate<T>::insert( pointer pos, const T& x )
{
    const size_t lastSize = size();
    const size_t n = lastSize !=0 ? 2*lastSize : 1;
    const size_t offset = pos - start;
    pointer newStart = new T[n];
    pointer newFinish = newStart + offset;
    qCopy( start, pos, newStart );
    *newFinish = x;
    qCopy( pos, finish, ++newFinish );
    delete[] start;
    start = newStart;
    finish = newStart + lastSize + 1;
    end = newStart + n;
}

template <class T>
Q_INLINE_TEMPLATES void QValueVectorPrivate<T>::insert( pointer pos, size_t n, const T& x )
{
    if ( size_t( end - finish ) >= n ) {
	// enough room
	const size_t elems_after = finish - pos;
	pointer old_finish = finish;
	if ( elems_after > n ) {
	    qCopy( finish - n, finish, finish );
	    finish += n;
	    qCopyBackward( pos, old_finish - n, old_finish );
	    qFill( pos, pos + n, x );
	} else {
	    pointer filler = finish;
	    size_t i = n - elems_after;
	    for ( ; i > 0; --i, ++filler )
		*filler = x;
	    finish += n - elems_after;
	    qCopy( pos, old_finish, finish );
	    finish += elems_after;
	    qFill( pos, old_finish, x );
	}
    } else {
	// not enough room
	const size_t lastSize = size();
	const size_t len = lastSize + QMAX( lastSize, n );
	pointer newStart = new T[len];
	pointer newFinish = qCopy( start, pos, newStart );
	// fill up inserted space
	size_t i = n;
	for ( ; i > 0; --i, ++newFinish )
	    *newFinish = x;
	newFinish = qCopy( pos, finish, newFinish );
	delete[] start;
	start = newStart;
	finish = newFinish;
	end = newStart + len;
    }
}

template <class T>
Q_INLINE_TEMPLATES void QValueVectorPrivate<T>::reserve( size_t n )
{
    const size_t lastSize = size();
    pointer tmp = growAndCopy( n, start, finish );
    start = tmp;
    finish = tmp + lastSize;
    end = start + n;
}

template <class T>
Q_INLINE_TEMPLATES Q_TYPENAME QValueVectorPrivate<T>::pointer QValueVectorPrivate<T>::growAndCopy( size_t n, pointer s, pointer f )
{
    pointer newStart = new T[n];
    qCopy( s, f, newStart );
    delete[] start;
    return newStart;
}

template <class T> class QDeepCopy;

template <class T>
class QValueVector
{
public:
    typedef T value_type;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;
    typedef value_type* iterator;
    typedef const value_type* const_iterator;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef size_t size_type;
#ifndef QT_NO_STL
    typedef ptrdiff_t difference_type;
#else
    typedef int difference_type;
#endif

    QValueVector()
    {
	sh = new QValueVectorPrivate<T>;
    }

    QValueVector( const QValueVector<T>& v )
    {
	sh = v.sh;
	sh->ref();
    }

    QValueVector( size_type n, const T& val = T() );

#ifndef QT_NO_STL
    QValueVector( std::vector<T>& v ) // ### remove in 4.0
    {
	sh = new QValueVectorPrivate<T>( v.size() );
	qCopy( v.begin(), v.end(), begin() );
    }

    QValueVector( const std::vector<T>& v )
    {
	sh = new QValueVectorPrivate<T>( v.size() );
	qCopy( v.begin(), v.end(), begin() );
    }
#endif

    ~QValueVector()
    {
	sh->derefAndDelete();
    }

    QValueVector<T>& operator= ( const QValueVector<T>& v )
    {
	v.sh->ref();
	sh->derefAndDelete();
	sh = v.sh;
	return *this;
    }

#ifndef QT_NO_STL
    QValueVector<T>& operator= ( const std::vector<T>& v )
    {
	clear();
	resize( v.size() );
	qCopy( v.begin(), v.end(), begin() );
	return *this;
    }
#endif

    size_type size() const { return sh->size(); }

    bool empty() const { return sh->empty(); }

    size_type capacity() const
    {
	return size_type( sh->capacity() );
    }

    iterator begin()
    {
	detach();
	return sh->start;
    }

    const_iterator begin() const
    {
	return sh->start;
    }

    const_iterator constBegin() const
    {
	return sh->start;
    }

    iterator end()
    {
	detach();
	return sh->finish;
    }

    const_iterator end() const
    {
	return sh->finish;
    }

    const_iterator constEnd() const
    {
	return sh->finish;
    }

    reference at( size_type i, bool* ok = 0 )
    {
	detach();
	if ( ok )
	    *ok = ( i < size() );
	return *( begin() + i );
    }

    const_reference at( size_type i, bool* ok = 0 ) const
    {
	if ( ok )
	    *ok = ( i < size() );
	return *( begin() + i );
    }

    reference operator[]( size_type i )
    {
	detach();
	return *( begin() + i );
    }

    const_reference operator[]( size_type i ) const
    {
	return *( begin() + i );
    }

    reference front()
    {
	Q_ASSERT( !empty() );
	detach();
	return *begin();
    }

    const_reference front() const
    {
	Q_ASSERT( !empty() );
	return *begin();
    }

    reference back()
    {
	Q_ASSERT( !empty() );
	detach();
	return *( end() - 1 );
    }

    const_reference back() const
    {
	Q_ASSERT( !empty() );
	return *( end() - 1 );
    }

    void push_back( const T& x )
    {
	detach();
	if ( sh->finish == sh->end ) {
	    sh->reserve( size()+size()/2+1 );
	}
	*sh->finish = x;
	++sh->finish;
    }

    void pop_back()
    {
	detach();
	if ( empty() )
	    return;
	--sh->finish;
    }

    iterator insert( iterator pos, const T& x );
    iterator insert( iterator pos, size_type n, const T& x );

    void reserve( size_type n )
    {
	if ( capacity() < n ) {
	    detach();
	    sh->reserve( n );
	}
    }

    void resize( size_type n, const T& val = T() )
    {
	if ( n < size() )
	    erase( begin() + n, end() );
	else
	    insert( end(), n - size(), val );
    }

    void clear()
    {
	detach();
	sh->clear();
    }

    iterator erase( iterator pos )
    {
	detach();
	if ( pos + 1 != end() )
	    qCopy( pos + 1, sh->finish, pos );
	--sh->finish;
	return pos;
    }

    iterator erase( iterator first, iterator last )
    {
	detach();
	qCopy( last, sh->finish, first );
	sh->finish = sh->finish - ( last - first );
	return first;
    }

    // ### remove in Qt 4.0
    bool operator==( const QValueVector<T>& x )
    {
	return size()==x.size() ? qEqual( constBegin(), constEnd(), x.begin()) : FALSE;
    }

    bool operator==( const QValueVector<T>& x ) const
    {
	return size()==x.size() ? qEqual( begin(), end(), x.begin() ) : FALSE;
    }

    typedef T ValueType;
    typedef ValueType *Iterator;
    typedef const ValueType *ConstIterator;

    size_type count() const { return size(); }
    bool isEmpty() const { return empty(); }

    reference first() { return front(); }
    const_reference first() const { return front(); }
    reference last() { return back(); }
    const_reference last() const { return back(); }
    void append( const T& x ) { push_back( x ); }

protected:
    void detach()
    {
	if ( sh->count > 1 ) { detachInternal(); }
    }
    void detachInternal();
    QValueVectorPrivate<T>* sh;

private:
    friend class QDeepCopy< QValueVector<T> >;
};

template <class T>
Q_INLINE_TEMPLATES QValueVector<T>::QValueVector( size_type n, const T& val )
{
    sh = new QValueVectorPrivate<T>( n );
    qFill( begin(), end(), val );
}

template <class T>
Q_INLINE_TEMPLATES void QValueVector<T>::detachInternal()
{
    sh->deref();
    sh = new QValueVectorPrivate<T>( *sh );
}

template <class T>
Q_INLINE_TEMPLATES Q_TYPENAME QValueVector<T>::iterator QValueVector<T>::insert( iterator pos, const T& x )
{
    size_type offset = pos - sh->start;
    detach();
    if ( pos == end() ) {
	if ( sh->finish == sh->end )
	    push_back( x );
	else {
	    *sh->finish = x;
	    ++sh->finish;
	}
    } else {
	if ( sh->finish == sh->end ) {
	    sh->insert( pos, x );
	} else {
	    *sh->finish = *(sh->finish - 1);
	    ++sh->finish;
	    qCopyBackward( pos, sh->finish - 2, sh->finish - 1 );
	    *pos = x;
	}
    }
    return begin() + offset;
}

template <class T>
Q_INLINE_TEMPLATES Q_TYPENAME QValueVector<T>::iterator QValueVector<T>::insert( iterator pos, size_type n, const T& x )
{
    if ( n != 0 ) {
	size_type offset = pos - sh->start;
	detach();
	pos = begin() + offset;
	sh->insert( pos, n, x );
    }
    return pos;
}


#ifndef QT_NO_DATASTREAM
template<class T>
Q_INLINE_TEMPLATES QDataStream& operator>>( QDataStream& s, QValueVector<T>& v )
{
    v.clear();
    Q_UINT32 c;
    s >> c;
    v.resize( c );
    for( Q_UINT32 i = 0; i < c; ++i )
    {
	T t;
	s >> t;
	v[i] = t;
    }
    return s;
}

template<class T>
Q_INLINE_TEMPLATES QDataStream& operator<<( QDataStream& s, const QValueVector<T>& v )
{
    s << (Q_UINT32)v.size();
    // ### use typename QValueVector<T>::const_iterator once all supported
    // ### compilers know about the 'typename' keyword.
    const T* it = v.begin();
    for( ; it != v.end(); ++it )
	s << *it;
    return s;
}
#endif // QT_NO_DATASTREAM

#define Q_DEFINED_QVALUEVECTOR
#include "qwinexport.h"
#endif // QVALUEVECTOR_H
