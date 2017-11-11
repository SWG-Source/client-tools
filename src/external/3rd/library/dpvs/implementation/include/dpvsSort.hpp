#ifndef __DPVSSORT_HPP
#define __DPVSSORT_HPP
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
 * Description: 	Sorting routines
 *
 * $Archive: /dpvs/implementation/include/dpvsSort.hpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 7.06.01 14:40 $
 * $Date: 2003/01/22 $
 * 
 ******************************************************************************/

#if !defined (__DPVSPRIVATEDEFS_HPP)
#	include "dpvsPrivateDefs.hpp"
#endif


namespace DPVS
{

//------------------------------------------------------------------------
// Prototypes for sort functions. Note that class T must have
// operators =, > and < in order for the functions to compile.
//------------------------------------------------------------------------

template <class T> inline void insertionSort(T* a, int N);
template <class T> inline void quickSort	(T* a, int N);

//------------------------------------------------------------------------
// Implementation
//------------------------------------------------------------------------

template <class T> DPVS_FORCE_INLINE bool isSorted(const T* a, int num)
{
	DPVS_ASSERT(a && num >= 0);
	for (int i = 1; i < num; i++)
		if (a[i-1] > a[i])
			return false;
	return true;
}

template <class T> DPVS_FORCE_INLINE void insertionSort(T* a, int N)
{
	DPVS_ASSERT(a && N >= 0);
	for (int i = 1; i < N; i++)
	if (a[i-1] > a[i])
	{
		T		v(a[i]);
		int		j	= i;
		while (a[j-1]> v)
		{
			a[j] = a[j-1];						// copy data
			j--;
			if (!j)
				break;
		};
		a[j]	= v;
	}

	DPVS_ASSERT(isSorted(a,N));				// make sure that the data is sorted
}

template <class T> DPVS_FORCE_INLINE int median3 (T* elements, int low, int high)
{
	DPVS_ASSERT(low >= 0 && high >= 2);
	int l = low;
	int c = (unsigned int)(high + low)>>1;
	int h = high-2;											// DEBUG DEBUG requires switchpoint bigger than 2 !!

	if(elements[l] > elements[h]) swap(l, h);
	if(elements[l] > elements[c]) swap(l, c);
	if(elements[c] > elements[h]) swap(c, h);

	DPVS_ASSERT(!(elements[l] > elements[c]));
	DPVS_ASSERT(!(elements[c] > elements[h]));

	return c;
}

// DEBUG DEBUG we made it non-inline because some idiot compilers
// barf when using recursive inline functions!!

// The quickSort() here used Median3 partitioning.

template <class T> /*inline*/ void quickSort(T* elements, int low, int high)
{
	//--------------------------------------------------------------------
	// Reached cut-off point --> switch to insertionSort..
	//--------------------------------------------------------------------

	const int SWITCHPOINT = 15;								// optimal value, see paper :)

	if((high - low) <= SWITCHPOINT)
	{
		insertionSort(&elements[low], high - low);
		return;
	}

	//--------------------------------------------------------------------
	// Select pivot using median-3
	//--------------------------------------------------------------------

	int	pivotIndex	= median3(elements, low, high);
	swap(elements[high-1], elements[pivotIndex]);			// hide pivot to highest entry

	T pivot(elements[high-1]);

	//--------------------------------------------------------------------
	// Partition data
	//--------------------------------------------------------------------

	int i = low  - 1;
	int j = high - 1;

	while (i < j)
	{
		do { i++; } while(elements[i] < pivot);
		do { j--; } while(elements[j] > pivot);

		DPVS_ASSERT(i>=low && j>=low && i < high && j < high);

		swap(elements[i], elements[j]);
	}

	//--------------------------------------------------------------------
	// Restore pivot
	//--------------------------------------------------------------------

	T tmp(elements[j]);
	elements[j]	= elements[i];
	elements[i]	= elements[high-1];
	elements[high-1] = tmp;

	//--------------------------------------------------------------------
	// sort sub-partitions
	//--------------------------------------------------------------------

	if((i - low) > 1)		quickSort(elements, low, i);
	if((high - (i+1)) > 1)	quickSort(elements, i+1, high);
}

template <class T> inline void quickSort(T* a, int N)
{
	quickSort(a, 0, N);
	DPVS_ASSERT(isSorted(a, N));		// make sure that the data is sorted
}

} // DPVS

//------------------------------------------------------------------------
#endif // __DPVSSORT_HPP

