#ifndef __DPVSXFIRSTTRAVERSAL_HPP
#define __DPVSXFIRSTTRAVERSAL_HPP
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
 * Description: 	Template classes for depth/breadth first traversal
 *					and containers.
 *
 * Notes:
 *
 * $Archive: /dpvs/implementation/include/dpvsXFirstTraversal.hpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 28.06.01 12:37 $
 * $Date: 2003/01/22 $
 * 
 ******************************************************************************/

#if !defined (__DPVSARRAY_HPP)
#	include "dpvsArray.hpp"
#endif


#define DPVS_DEPTH_FIRST		//TEMP


namespace DPVS
{

	template <class T> class MultiContainer
	{
		public:
						MultiContainer	()				{ reset(); }

			void		reset			(void)
			{
				m_containerIndex=-1;
				m_listIndex=0;
				m_totalCount=0;
			}

			void		append			(const T& e)
			{
				DPVS_ASSERT(m_containerIndex>=0);
				m_totalCount++;
				m_container.getElement(m_containerIndex).m_count++;
				m_list.getElement(m_listIndex++)=e;
			}

			void		newContainer	(void)
			{
				m_containerIndex++;
				Entry& e = m_container.getElement(m_containerIndex);
				e.m_first = m_listIndex;
				e.m_count = 0;
			}

			int			getContainer	(int num, const T*& array) const
			{
				DPVS_ASSERT(m_containerIndex>=0);
				DPVS_ASSERT(num>=0 && num<=m_containerIndex);

				if (m_container[num].m_count > 0)			
				{
					int index = m_container[num].m_first;
					array = &m_list[index];
					return m_container[num].m_count;
				} else
				{
					array = null;							
					return 0;
				}
			}

			T			getPreviouslyAppended	(void) const	{ DPVS_ASSERT(m_listIndex>0); return m_list[m_listIndex-1]; }

			bool		hasContainer	(int num) const		{ return num>=0 && num<=m_containerIndex; }
			int			getCount		(void) const		{ return m_totalCount; }

		private:

			class Entry
			{
			public:
				int	m_first;
				int	m_count;
			};

			DynamicArray<Entry>	m_container;		// # of entries per container
			DynamicArray<T>		m_list;				// list of data
			int					m_containerIndex;	// current index inside count
			int					m_listIndex;		// current index inside list
			int					m_totalCount;
	};


	template <class T> class BreadthFirstTraversal
	{
		public:
			BreadthFirstTraversal()						{ reset(); }

			void		reset			(void)			{ m_currentIndex = 0; m_appendIndex = 0; }
			void		append			(T e)			{ m_list[m_appendIndex++] = e; }
			int			traverse		(void)
			{
				bool stop = false;
				int	levelsTraversed = 0;

				while(!stop)
				{
					m_currentIndex++;
					if(m_list[m_currentIndex] || m_currentIndex<m_appendIndex)		// skip nulls
						stop = true;
					else
						levelsTraversed++;
				}

				return levelsTraversed;
			}

			const T&	getCurrent		(void) const	{ DPVS_ASSERT(m_currentIndex<=m_appendIndex); return m_list[m_currentIndex]; }
			int			getCurrentIndex	(void) const	{ return m_currentIndex; }
			int			getParentIndex	(void) const
			{
				if(m_currentIndex==0)
					return -1;

				int curr = m_currentIndex;
				do{
					curr--;
				} while(!m_list[curr] && curr>0);	// skip nulls
				return curr;
			}

			void		setCurrentIndex	(int index)		{ DPVS_ASSERT(index<m_appendIndex); m_currentIndex = index; }
			int			getCount		(void) const	{ return m_appendIndex; }
			bool		isLast			(void) const	{ return m_currentIndex>=m_appendIndex; }

		private:
			DynamicArray<T>		m_list;				// list of data
			int					m_currentIndex;		// current index inside array
			int					m_appendIndex;		// where do appends go
	};

	

	template <class T> class DepthFirstTraversal
	{
		public:
			DepthFirstTraversal()						{ reset(); }

			void		reset			(void)
			{
				m_currentIndex = 0;
				m_appendIndex = 0;
				m_list.getElement(0).reset();
			}

			void		append			(T e)
			{
				Entry& curr = m_list.getElement(m_currentIndex);

				if(curr.m_firstChildIndex == INVALID && e)
					 curr.m_firstChildIndex = m_appendIndex;

				Entry& app = m_list.getElement(m_appendIndex);
				app.m_firstChildIndex	= INVALID;
				app.m_parentIndex		= m_appendIndex!=m_currentIndex ? m_currentIndex : INVALID;
				app.m_data				= e;

				m_list.getElement(++m_appendIndex).reset();
			}

			int			traverse		(void)
			{
				const DynamicArray<Entry>& list = m_list;	// DEBUG DEBUG ADDED BY WILI

				int levelsTraversed = 0;

				// traverse to first child if exists and its data is != null
				if(list[m_currentIndex].m_firstChildIndex != INVALID && list[list[m_currentIndex].m_firstChildIndex].m_data)
				{
					m_currentIndex = list[m_currentIndex].m_firstChildIndex;
					levelsTraversed = 1;
				}
				else
				{
					bool seek = true;
					while(seek)
					{
						int index = m_currentIndex+1;				// move to next sister
						if(list[index].m_data)
						{
							m_currentIndex = index;					// traverse to next entry on the same level if exists
							seek = false;
						}
						else
						{
							m_currentIndex = list[m_currentIndex].m_parentIndex;
							levelsTraversed--;						// one level backwards

							if(m_currentIndex<=0)					// back in the beginning -> all entries have been traversed
							{
								m_currentIndex = m_appendIndex;
								seek = false;
							}
						}
					}
				}

				return levelsTraversed;
			}

			const T&	getCurrent		(void) const	{ return m_list[m_currentIndex].m_data; }
			int			getCurrentIndex	(void) const	{ return m_currentIndex; }
			int			getAppendIndex	(void) const	{ return m_appendIndex; }
			int			getParentIndex	(void) const	{ return m_list[m_currentIndex].m_parentIndex; }
			void		setCurrentIndex	(int index)		{ DPVS_ASSERT(index<=m_appendIndex); m_currentIndex = index; }
			int			getCount		(void) const	{ return m_appendIndex; }
			bool		isLast			(void) const	{ return m_currentIndex>=m_appendIndex; }

		private:

			enum
			{
				INVALID = -1
			};

			class Entry
			{
			public:
				void reset ()				{ m_firstChildIndex = -1; m_parentIndex=-1; m_data=0; }
				int		m_firstChildIndex;
				int		m_parentIndex;
				T		m_data;
			};

			DynamicArray<Entry>	m_list;				// list of data
			int					m_currentIndex;		// current index inside array
			int					m_appendIndex;		// where do appends go
	};





/*
	// DEBUG DEBUG DEBUG DEBUG
	DepthFirstTraversal<char>	df;
//	BreadthFirstTraversal<char>	df;
	char result[7];
	result[6] = 0;
	char* v=result;

	df.append('A');		// root node
	df.append(0);

	*v++ = df.getCurrent();

	df.append('B');
	df.append('C');
	df.append(0);

	df.traverse();		// A -> B
	*v++ = df.getCurrent();

	df.append('D');
	df.append('E');
	df.append(0);

	df.traverse();		// B -> D
	*v++ = df.getCurrent();

	df.append(0);

	df.traverse();		// D -> E
	*v++ = df.getCurrent();

	df.append(0);

	df.traverse();		// E -> C
	*v++ = df.getCurrent();

	df.append('F');
	df.append(0);

	df.traverse();		// C -> F
	*v++ = df.getCurrent();

//	df.traverse();		// F -> A
//	*v++ = df.getCurrent();

	int tmp = 0;
	*/

} // DPVS

//------------------------------------------------------------------------
#endif //__DPVSXFIRSTTRAVERSAL_HPP
