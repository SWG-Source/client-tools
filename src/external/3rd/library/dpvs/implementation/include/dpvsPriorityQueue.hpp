#ifndef __DPVSPRIORITYQUEUE_HPP
#define __DPVSPRIORITYQUEUE_HPP
/*****************************************************************************
 *
 * dPVS
 * -----------------------------------------
 *
 * (C) 1999-2001 Hybrid Holding, Ltd.
 * All Rights Reserved.
 *
 * This file consists of unpublished, proprietary source code of
 * Hybrid Holding, and is considered Confidential Information for
 * purposes of non-disclosure agreement. Disclosure outside the terms
 * outlined in signed agreement may result in irrepairable harm to
 * Hybrid Holding and legal action against the party in breach.
 *
 *
 * Desc:	Priority queue template class
 *
 * $Archive: /dpvs/implementation/include/dpvsPriorityQueue.hpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 8.05.01 11:42 $
 * $Date: 2003/01/22 $
 * 
 *****************************************************************************/

#if !defined (__DPVSARRAY_HPP)
#	include "dpvsArray.hpp"
#endif

namespace DPVS
{

/******************************************************************************
 *
 * Class:			DPVS::PQueue
 *
 * Description:		Priority queue template class
 *
 ******************************************************************************/

template <class Item>
class PQueue
{
public:
	DPVS_FORCE_INLINE 				PQueue				(void) : m_elements(0),m_delayed(false) {}
	DPVS_FORCE_INLINE				~PQueue				(void)				{}
	DPVS_FORCE_INLINE void			checkConsistency	(void);
	DPVS_FORCE_INLINE void			getMin				(Item& item);
	DPVS_FORCE_INLINE int			getSize				(void) const		{ return m_elements - (int)(m_delayed); }
	DPVS_FORCE_INLINE void			insert				(const Item& item);
	DPVS_FORCE_INLINE bool			isEmpty				(void) const		{ return getSize()==0;	}
	DPVS_FORCE_INLINE void			popMin				(Item& item);
	DPVS_FORCE_INLINE void			removeMin			(void);
	DPVS_FORCE_INLINE void			removeAll			(void)				{ m_items.resize(0); m_elements = 0; m_delayed = false; }
private:

	DPVS_FORCE_INLINE void			removeReal			(void);
	DPVS_FORCE_INLINE void			replaceMin			(const Item& item);

	static DPVS_FORCE_INLINE int	getParent			(int n)		{ return ((n+1)>>1)-1;	}
	static DPVS_FORCE_INLINE int	getLeftChild		(int n)		{ return ((n+1)<<1)-1;	}
	static DPVS_FORCE_INLINE int	getRightChild		(int n)		{ return ((n+1)<<1);	}
	DPVS_FORCE_INLINE void			siftDown			(void);
	DPVS_FORCE_INLINE void			checkNode			(int n) const;

	Array<Item>		m_items;			// array of items
	int				m_elements;			// number of elements
	bool			m_delayed;			// is the last remove delayed?
};

template <class Item> DPVS_FORCE_INLINE void PQueue<Item>::siftDown(void)
{
	int n = 0;

	for (;;)
	{
		DPVS_ASSERT (n >= 0 && n < m_elements);

		int leftChild	= getLeftChild(n);

		if((leftChild+1) < m_elements)						// if has right child, compare with left
		{
			if(m_items[(leftChild+1)] <= m_items[leftChild])		// has right child
				leftChild++;
		} else
		{
			if(leftChild >= m_elements)							// 
				break;
		}

		DPVS_ASSERT (leftChild >= 0 && leftChild < m_elements);

		if (m_items[n] <= m_items[leftChild])
			break;

		swap(m_items[n], m_items[leftChild]);				// swap smaller node

		n = leftChild;
	}
}

template <class Item> DPVS_FORCE_INLINE void PQueue<Item>::removeReal(void)
{
	DPVS_ASSERT (m_elements > 0);
	DPVS_ASSERT (m_delayed);
	m_delayed = false;

	m_items[0] = m_items[--m_elements];		// replace root node with last node in array
	if(m_elements > 1) 
		siftDown();							// if more than one item, sift it down
}

template <class Item> DPVS_FORCE_INLINE void PQueue<Item>::getMin(Item& item)
{ 
	if (m_delayed)
		removeReal();
	DPVS_ASSERT (m_elements > 0); 
	item = m_items[0]; 
}


template <class Item> DPVS_FORCE_INLINE void PQueue<Item>::replaceMin(const Item& item)
{
	DPVS_ASSERT (m_elements > 0);

	m_items[0] = item;
	siftDown();
}

template <class Item> DPVS_FORCE_INLINE void PQueue<Item>::insert(const Item& item)
{
	if (m_delayed)
	{
		replaceMin(item);
		m_delayed = false;
	} else
	{
		if (m_elements == m_items.getSize())
		{
			int newElements = m_elements*2;			// 50% increase...
			if (newElements < 32)					// always allocate at least 32 elements
				newElements = 32;					
			m_items.resize(newElements);			
		}

		int n = m_elements;						
		
		m_items[m_elements++] = item;

		while (n > 0)
		{
			int parent = getParent(n);
			if (m_items[parent]<=m_items[n])
				break;
			swap(m_items[parent], m_items[n]);			// swap items
			n = parent;
		}
	}
}


template <class Item> DPVS_FORCE_INLINE void PQueue<Item>::removeMin(void)
{
	if (m_delayed)
		removeReal();
	m_delayed = true;
}

template <class Item> DPVS_FORCE_INLINE void PQueue<Item>::popMin(Item& item)
{
	if (m_delayed)
		removeReal();
	item = m_items[0];
	m_delayed = true;
}

template <class Item> DPVS_FORCE_INLINE void PQueue<Item>::checkNode(int n) const
{
	if(n >= m_elements) 
		return;

	if(n != 0 && getParent(n) < m_elements)
	{
		DPVS_ASSERT(m_items[getParent(n)] <= m_items[n]);
	}

	checkNode(getLeftChild(n));
	checkNode(getRightChild(n));
}

template <class Item> DPVS_FORCE_INLINE void PQueue<Item>::checkConsistency(void) 
{
	if (m_delayed)
		removeReal();
	checkNode(0);
}


} // DPVS

//------------------------------------------------------------------------
#endif //__DPVSPRIORITYQUEUE_HPP


