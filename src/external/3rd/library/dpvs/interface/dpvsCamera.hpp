#ifndef __DPVSCAMERA_HPP
#define __DPVSCAMERA_HPP
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
 * Description:     Camera interface
 *
 * $Id: //depot/products/dpvs/interface/dpvsCamera.hpp#4 $
 * $Date: 2004/11/12 $
 * $Author: wili $
 * 
 ******************************************************************************/

#if !defined (__DPVSREFERENCECOUNT_HPP)
#   include "dpvsReferenceCount.hpp"
#endif
    
/******************************************************************************
 *
 * Class:           DPVS::Camera
 *
 * Description:     Camera object
 *
 * Notes:           The camera interface is used for determining visible 
 *                  objects in a scene. The camera defines the viewer's
 *                  position and orientation and the shape of the
 *                  view frustum. The camera interface is also used to
 *                  select culling methods used inside the visibility
 *                  query.
 *
 *****************************************************************************/

namespace DPVS
{
class Camera : public ReferenceCount
{
public:
    enum Property
    {
        VIEWFRUSTUM_CULLING     = (1<<0),       // enable view frustum culling
        OCCLUSION_CULLING       = (1<<1),       // enable occlusion culling (note that VIEWFRUSTUM_CULLING gets automatically enabled if this is set)
        DISABLE_VIRTUALPORTALS  = (1<<2),       // disables traversal through any virtual portals
        SCOUT                   = (1<<3),       // scout camera, consult manual for details
		PREPARE_RESEND			= (1<<4),		// prepare to resend the results of this query. (DEBUG feature)
        RESEND                  = (1<<5),       // resend the results of the previous query (without re-resolving the visibility). (DEBUG feature)
        OPTIMIZE                = (1<<6)        // allow visibility query to spend additional time for optimizing future queries
    };

	static DPVSDEC Camera*	create						(void);
	DPVSDEC void			getCameraToCellMatrix       (Matrix4x4&) const;
    DPVSDEC void			getCameraToCellMatrix       (Matrix4x4d&) const;
    DPVSDEC void			getCameraToWorldMatrix      (Matrix4x4&) const;
    DPVSDEC void			getCameraToWorldMatrix      (Matrix4x4d&) const;
    DPVSDEC class Cell*		getCell                     (void) const;
    DPVSDEC void			getFrustum                  (Frustum&) const;
    DPVSDEC int				getHeight                   (void) const;
    DPVSDEC void			getObjectMinimumCoverage    (float& width, float& height, float& opacity) const;
	DPVSDEC void			getPixelCenter				(float& xoffset, float& yoffset) const;
    DPVSDEC unsigned int	getProperties               (void) const;
    DPVSDEC void			getScissor                  (int& left, int& top, int& right, int& bottom) const;
    DPVSDEC int				getWidth                    (void) const;
    DPVSDEC bool			isPointVisible              (const Vector3&) const;
    DPVSDEC bool			isRectangleVisible          (float xmin, float ymin, float xmax, float ymax, float zmax) const;
    DPVSDEC void			resolveVisibility           (class Commander*, int recursionDepth, float importanceThreshold=0.0) const;
    DPVSDEC void			setCameraToCellMatrix       (const Matrix4x4&);
    DPVSDEC void			setCameraToCellMatrix       (const Matrix4x4d&);
    DPVSDEC void			setCell                     (class Cell*);
    DPVSDEC void			setFrustum                  (const Frustum&);
    DPVSDEC void			setObjectMinimumCoverage    (float pixelWidth, float pixelHeight, float opacity);
    DPVSDEC void			setParameters               (int screenWidth, int screenHeight, unsigned int propertyMask=VIEWFRUSTUM_CULLING|OCCLUSION_CULLING, float imageSpaceScalingX=1.f, float imageSpaceScalingY=1.f);
	DPVSDEC void			setPixelCenter				(float xOffset, float yOffset);
    DPVSDEC void			setScissor                  (int left, int top, int right, int bottom);
    DPVSDEC void			setStaticCoverageMask       (const unsigned char* buffer, int width, int height, int pitch);
    DPVSDEC void			setStaticZBuffer            (const float* buffer, int width, int height, int pitch, float farValue);

    class ImpCamera*        getImplementation           (void) const;
protected:                                          
							Camera						(class ImpCamera*);
    DPVSDEC virtual			~Camera                     (void);
private:                                                    
                            Camera                      (const Camera&);    // not allowed
    Camera&                 operator=                   (const Camera&);    // not allowed
    class ImpCamera*        m_imp;                                          // opaque pointer
};
} // DPVS
                                            
//------------------------------------------------------------------------
#endif // __DPVSCAMERA_HPP
