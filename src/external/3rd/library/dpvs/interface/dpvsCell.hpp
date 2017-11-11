#ifndef __DPVSCELL_HPP
#define __DPVSCELL_HPP
/*****************************************************************************
 *
 * dPVS
 * -----------------------------------------
 *
 * (C) 1999-2004 Hybrid Graphics, Ltd.
 * All Rights Reserved.
 *
 * Dynamic PVS and dPVS are trademarks of Criterion Software Ltd.
 *
 * This file consists of unpublished, proprietary source code of
 * Hybrid Graphics, and is considered Confidential Information for
 * purposes of non-disclosure agreement. Disclosure outside the terms
 * outlined in signed agreement may result in irrepairable harm to
 * Hybrid Graphics and legal action against the party in breach.
 *
 * Description:     Cell interface
 *
 * $Id: //depot/products/dpvs/interface/dpvsCell.hpp#4 $
 * $Date: 2004/11/12 $
 * $Author: wili $
 * 
 ******************************************************************************/

#if !defined (__DPVSREFERENCECOUNT_HPP)
#   include "dpvsReferenceCount.hpp"
#endif
    
namespace DPVS
{
/******************************************************************************
 *
 * Class:           DPVS::Cell
 *
 * Description:     A cell defines a region in space with arbitrary topology.
 *
 * Notes:           Every object/camera in the world must belong to some
 *                  cell. Regions of Influence are limited to the cell they
 *                  belong.
 *  
 *                  Multiple cells can be connected by using portals.
 *
 * See Also:        DPVS::PhysicalPortal
 *
 *****************************************************************************/

class Cell : public ReferenceCount
{
public:
    enum Property
    {
        ENABLED					= 0,    // is the cell enabled?
		REPORT_IMMEDIATELY		= 1		// report entrance to cell immediately (i.e. when entering cell inside "real" traversal, not the buffered messages)
    };

	static DPVSDEC Cell*	create					(void);
    DPVSDEC void			getCellToWorldMatrix    (Matrix4x4&) const;
    DPVSDEC void			getCellToWorldMatrix    (Matrix4x4d&) const;
    DPVSDEC void			getWorldToCellMatrix    (Matrix4x4&) const;
    DPVSDEC void			getWorldToCellMatrix    (Matrix4x4d&) const;
    DPVSDEC void			set                     (Property,bool);
    DPVSDEC void			setCellToWorldMatrix    (const Matrix4x4&);
    DPVSDEC void			setCellToWorldMatrix    (const Matrix4x4d&);
    DPVSDEC bool			test                    (Property) const;
    class ImpCell*          getImplementation       (void) const;
protected:
							Cell					(class ImpReferenceCount*);
	virtual void			destruct				(void) const;
private:                                        
                            Cell                    (const Cell&);              // not allowed
    Cell&                   operator=               (const Cell&);              // not allowed
};

} // DPVS

//------------------------------------------------------------------------
#endif // __DPVSCELL_HPP
