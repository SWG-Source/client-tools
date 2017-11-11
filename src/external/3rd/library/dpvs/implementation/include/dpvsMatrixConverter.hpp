#ifndef __DPVSMATRIXCONVERTER_HPP
#define __DPVSMATRIXCONVERTER_HPP

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
 * Description: 	Matrix class
 *
 * $Archive: /dpvs/implementation/include/dpvsMatrixConverter.hpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 7.05.01 12:06 $
 * $Date: 2003/01/22 $
 * 
 ******************************************************************************/

#if !defined (__DPVSPRIVATEDEFS_HPP)
#include "dpvsPrivateDefs.hpp"
#endif

namespace DPVS
{

/******************************************************************************
 *
 * Class:			DPVS::MatrixConverter
 *
 * Description:		Class for converting 4x4 and 4x3 matrices between
 *					column major and row major formats.
 *
 *****************************************************************************/

class MatrixConverter
{
public:
	static void	convertMatrix		(Matrix4x3&  dst, const Matrix4x4& src);
	static void	convertMatrix		(Matrix4x3&  dst, const Matrix4x4d& src);
	static void	convertMatrix		(Matrix4x4&  dst);
	static void	convertMatrix		(Matrix4x4&  dst, const Matrix4x3& src);
	static void	convertMatrix		(Matrix4x4&  dst, const Matrix4x4& src);
	static void	convertMatrix		(Matrix4x4&  dst, const Matrix4x4d& src);
	static void	convertMatrix		(Matrix4x4d& dst, const Matrix4x3& src);
	static void	convertMatrix		(Matrix4x4d& dst, const Matrix4x4& src);

	static void	setColumnMajor		(bool v)				{ s_columnMajor = v; }
	static bool	isColumnMajor		(void)					{ return s_columnMajor; }
private:
	static bool	s_columnMajor;
};

} //DPVS

//------------------------------------------------------------------------
#endif //__DPVSMATRIXCONVERTER_HPP
