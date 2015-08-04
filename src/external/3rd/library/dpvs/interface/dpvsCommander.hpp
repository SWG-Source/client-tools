#ifndef __DPVSCOMMANDER_HPP
#define __DPVSCOMMANDER_HPP
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
 * Description:     Commander interface
 *
 * $Id: //depot/products/dpvs/interface/dpvsCommander.hpp#5 $
 * $Date: 2004/11/12 $
 * $Author: wili $
 * 
 ******************************************************************************/

#if !defined (__DPVSLIBRARY_HPP)
#   include "dpvsLibrary.hpp"
#endif

/******************************************************************************
 *
 * Class:           DPVS::Commander
 *
 * Description:     Commander object
 *
 * Notes:           The commander interface handles user<->DPVS communication.
 *                  This approach was taken both to avoid extending API in
 *                  future releases and to keep future DLLs binary compatible.
 *
 *                  The application user should create a new class derived
 *                  from DPVS::Commander and overload the 
 *                  DPVS::Commander::command() function to provide a valid 
 *                  implementation. See documentation for the function for
 *                  further information.
 *
 *****************************************************************************/
/******************************************************************************
 *
 * Class:           DPVS::Commander::Instance
 *
 * Description:     Passes object-related information to the user
 *
 * Notes:           This class is used solely by DPVS::Commander for providing 
 *                  additional information during DPVS::Commander::command()
 *                  callbacks.
 *
 *****************************************************************************/
/******************************************************************************
 *
 * Class:           DPVS::Commander::Viewer
 *
 * Description:     Passes view-related information to user
 *
 * Notes:           This class is used by DPVS::Commander to provide
 *                  additional view-specific information during certain
 *                  DPVS::Commander::command() callbacks.
 *
 *****************************************************************************/

namespace DPVS
{
class Object;
class ImpCommander;
class Cell;

class Commander
{
public:
    enum Command
    {
        QUERY_BEGIN                     = 0x00, // beginning of resolveVisibility() [NONE]
        QUERY_END                       = 0x01, // end of resolveVisibility()       [NONE]
        QUERY_ABORT						= 0x02,	// NOT USED (RESERVED FOR FUTURE)
        PORTAL_ENTER                    = 0x10, // traversed through a portal       [getInstance()]
        PORTAL_EXIT                     = 0x11, // came back through a portal       [getInstance()]
		PORTAL_PRE_EXIT					= 0x13,	// about to come back through a portal [NONE]
        CELL_IMMEDIATE_REPORT			= 0x12,	// cell to react to (immediately)	[getCell()]
		VIEW_PARAMETERS_CHANGED         = 0x20, // new viewing parameters           [getViewer()]
        INSTANCE_VISIBLE                = 0x30, // object to draw                   [getInstance()]
        REMOVAL_SUGGESTED               = 0x31, // garbage collect suggestion       [getInstance()]
		INSTANCE_IMMEDIATE_REPORT		= 0x32,	// object to react to				[getInstance()]
		REGION_OF_INFLUENCE_ACTIVE      = 0x40, // region of influence turned on    [getInstance()]
        REGION_OF_INFLUENCE_INACTIVE    = 0x41, // region of influence turned off   [getInstance()]
        STENCIL_VALUES_CHANGED          = 0x50, // stencil buffer test and write values have changed    [getStencilValues()]
        STENCIL_MASK                    = 0x51, // new object should be written to stencil buffer       [getInstance()]
        RESERVED0						= 0x52,	// reserved for future use
        TEXT_MESSAGE                    = 0x60, // DEBUG [getTextMessage()]
        DRAW_LINE_2D                    = 0x61, // DEBUG [getLine2D()]
        DRAW_LINE_3D                    = 0x62, // DEBUG [getLine3D()]
        DRAW_BUFFER                     = 0x63, // DEBUG [getBuffer()]
        RESERVED1			            = 0x70, // reserved for future use
        RESERVED2						= 0x71, // reserved for future use
        COMMAND_MAX                     = 0x7FFFFFFF    // force enumerations to be int
    };

    class Instance
    {
    public:
        struct Projection
        {
            int     left;                   // left screen coordinate                       
            int     right;                  // right screen coordinate (exclusive)
            int     top;                    // top screen coordinate 
            int     bottom;                 // bottom screen coordinate (exclusive)
            float   zNear;                  // near depth value
            float   zFar;                   // far depth value
        };

		enum Clip
		{
			FRONT	= (1<<0),				// front clip plane	
			BACK	= (1<<1),				// back clip plane
			LEFT	= (1<<2),				// left clip plane
			RIGHT	= (1<<3),				// right clip plane
			TOP		= (1<<4),				// top clip plane
			BOTTOM	= (1<<5)				// bottom clip plane
		};

		DPVSDEC UINT32	getClipMask				(void) const;
        DPVSDEC float   getImportance           (void) const;
        DPVSDEC Object*	getObject               (void) const;               // pointer to object
        DPVSDEC void    getObjectToCameraMatrix	(Matrix4x4&) const;         // view without projection
        DPVSDEC void	getObjectToCameraMatrix (Matrix4x4d&) const;        // view without projection
        DPVSDEC bool    getProjectionSize       (Projection&) const;
    private:
        Instance&       operator=               (const Instance&);          // not allowed
                        Instance                (const Instance&);          // not allowed
						Instance                (void);
						~Instance               (void);
        friend class ImpCommander;
        const class ImpObject*  m_imp;                                              // opaque pointer
    };

    class Viewer
    {
    public:
        enum Handedness
        {
            LEFT_HANDED     = 0,	// matrix is left-handed
            RIGHT_HANDED    = 1     // matrix is right-handed
        };

        DPVSDEC void	getFrustum				(Frustum&) const;
        DPVSDEC int		getFrustumPlaneCount    (void) const;
        DPVSDEC void	getFrustumPlane         (int index, Vector4& plane) const;
        DPVSDEC void	getScissor              (int& left,int& top,int& right,int& bottom) const;
        DPVSDEC void	getProjectionMatrix     (Matrix4x4&,  Handedness) const;    
        DPVSDEC void	getProjectionMatrix     (Matrix4x4d&, Handedness) const;
        DPVSDEC void	getCameraToWorldMatrix  (Matrix4x4&) const; 
        DPVSDEC void	getCameraToWorldMatrix  (Matrix4x4d&) const;
		DPVSDEC bool	isMirrored				(void) const;

    private:                                
						Viewer                  (const Viewer&);    // not allowed
        Viewer&			operator=               (const Viewer&);    // not allowed
						Viewer                  (void);
						~Viewer                 (void);
        friend class ImpCamera;                             
        class ImpCamera* m_imp;                                         // implementation pointer
    };


    DPVSDEC virtual				~Commander				(void);
    DPVSDEC virtual void		command					(Command c) = 0;      
    class ImpCommander*         getImplementation		(void) const;

protected:

    DPVSDEC						Commander				(void);
    DPVSDEC Library::BufferType getBuffer				(const unsigned char*&s, int& w,int& h) const;
	DPVSDEC Cell*				getCell					(void) const;
    DPVSDEC const Instance*		getInstance				(void) const;
    DPVSDEC Library::LineType	getLine2D				(Vector2& a, Vector2& b, Vector4& color) const;
    DPVSDEC Library::LineType	getLine3D				(Vector3& a, Vector3& b, Vector4& color) const;
    DPVSDEC void				getStencilValues		(int& test,int& write) const;
    DPVSDEC const char*			getTextMessage			(void) const;
    DPVSDEC const Viewer*		getViewer				(void) const;
private:
	friend class ImpCommander;
                                Commander				(const Commander&);     // not allowed
    Commander&                  operator=				(const Commander&);     // not allowed
    ImpCommander*				m_imp;											// implementation pointer
};

} // DPVS
                                            
//------------------------------------------------------------------------
#endif // __DPVSCAMERA_HPP
