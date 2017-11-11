#ifndef __DPVSIMPCOMMANDER_HPP
#define __DPVSIMPCOMMANDER_HPP
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
 * Description: 	Commander implementation class
 *
 * $Archive: /dpvs/implementation/include/dpvsImpCommander.hpp $
 * $Author: wili $ 
 * $Revision: #2 $
 * $Modtime: 8.01.02 13:25 $
 * $Date: 2003/08/24 $
 * 
 ******************************************************************************/

#if !defined (__DPVSPRIVATEDEFS_HPP)
#	include "dpvsPrivateDefs.hpp"
#endif
#if !defined (__DPVSCOMMANDER_HPP)
#	include "dpvsCommander.hpp"
#endif
#if !defined (__DPVSVECTOR_HPP)
#	include "dpvsVector.hpp"
#endif
#if !defined (__DPVSINSTANCECOUNT_HPP)
#	include "dpvsInstanceCount.hpp"
#endif

namespace DPVS
{
class OcclusionBuffer;
class Surface;
	
DPVS_FORCE_INLINE class ImpCommander* Commander::getImplementation   (void) const	{ return m_imp; }

//------------------------------------------------------------------------
// Some Commander::Instance functions...
//------------------------------------------------------------------------

DPVS_FORCE_INLINE Commander::Instance::Instance(void)
{
	m_imp = 0;
}

DPVS_FORCE_INLINE Commander::Instance::~Instance(void)
{
	DPVS_ASSERT (!m_imp);		// huh -- someone leaked!
	m_imp = 0;
}
	
/******************************************************************************
 *
 * Class:			DPVS::ImpCommander
 *
 * Description:		Internal implementation of the Commander class
 *
 *****************************************************************************/

class ImpCommander
{
private:
								ImpCommander		(const ImpCommander&);
	ImpCommander&				operator=			(const ImpCommander&);
	const Commander::Viewer*	m_viewer;				// pointer to viewer
	const char*					m_textMessage;			// text debug
	Vector4						m_lineColor;			// current line color
	Library::LineType			m_lineType;				// current line type
	OcclusionBuffer*			m_occlusionBuffer;		// current debug buffer
	Library::BufferType			m_bufferType;			// current buffer type
	int							m_stencilTestValue;		// stencil test value
	int							m_stencilWriteValue;	// stencil write value
	Commander*					m_commander;			// pointer back to Commander
	Commander::Instance			m_instance;				// instance (not a pointer)
	Vector2						m_line2D[2];			// current 2D debug line
	Vector3						m_line3D[2];			// current 3D debug line
	ImpCommander*				m_prev;					// previous ImpCommander
	ImpCommander*				m_next;					// next ImpCommander
	Cell*						m_cell;					// pointer to cell
	static ImpCommander*		s_first;				// first ImpCommander

public:

	static			  void							releaseAll			(void);

	DPVS_FORCE_INLINE 								ImpCommander		(Commander*);
	DPVS_FORCE_INLINE 								~ImpCommander		(void);
	DPVS_FORCE_INLINE void							command             (Commander::Command c)						
	{ 
#if defined (DPVS_DEBUG)
		UINT32 oldFPUMode = getFPUMode();
#endif // DPVS_DEBUG

		m_commander->command(c); 
		DPVS_ASSERT(getFPUMode() == oldFPUMode && "FPU mode changed in user callback!");
	}

	Library::BufferType								getBuffer			(Surface &s);
	DPVS_FORCE_INLINE Cell*							getCell				(void) const								{ return m_cell; }
	DPVS_FORCE_INLINE const Commander::Instance&	getInstance			(void) const								{ return m_instance; }
	DPVS_FORCE_INLINE Library::LineType				getLine2D			(Vector2& a, Vector2& b, Vector4& color)	{ a=m_line2D[0]; b=m_line2D[1]; color=m_lineColor; return m_lineType; }
	DPVS_FORCE_INLINE Library::LineType				getLine3D			(Vector3& a, Vector3& b, Vector4& color)	{ a=m_line3D[0]; b=m_line3D[1]; color=m_lineColor; return m_lineType; }
	DPVS_FORCE_INLINE void							getStencilValues	(int &test,int &write) const				{ test=m_stencilTestValue; write=m_stencilWriteValue; }
	DPVS_FORCE_INLINE const char*					getTextMessage		(void) const								{ return m_textMessage; }
	DPVS_FORCE_INLINE const Commander::Viewer*		getViewer			(void) const								{ return m_viewer; }
	DPVS_FORCE_INLINE bool							isInstanceSpecified	(void) const								{ return m_instance.m_imp != null; }
	DPVS_FORCE_INLINE void							setBufferType		(Library::BufferType b)						{ m_bufferType = b; }
	DPVS_FORCE_INLINE void							setCell				(Cell* c)									{ m_cell = c; }
	DPVS_FORCE_INLINE void							setInstanceObject	(const ImpObject* o)						{ m_instance.m_imp = o; }
	DPVS_FORCE_INLINE void							setLine2D			(Library::LineType t, const Vector2& a, const Vector2& b, const Vector4& color) { m_line2D[0] = a; m_line2D[1] = b; m_lineColor = color; m_lineType = t; }
	DPVS_FORCE_INLINE void							setLine3D			(Library::LineType t, const Vector3& a, const Vector3& b, const Vector4& color) { m_line3D[0] = a; m_line3D[1] = b; m_lineColor = color; m_lineType = t; }
	DPVS_FORCE_INLINE void							setOcclusionBuffer	(OcclusionBuffer* b)						{ m_occlusionBuffer = b; }
	DPVS_FORCE_INLINE void							setStencilValues	(int test,int write)						{ m_stencilTestValue=test; m_stencilWriteValue=write; }
	DPVS_FORCE_INLINE void							setTextMessage		(const char* s)								{ m_textMessage = s; }
	DPVS_FORCE_INLINE void							setViewer			(const Commander::Viewer* v)				{ m_viewer = v; }
};


DPVS_FORCE_INLINE ImpCommander::ImpCommander (Commander* c) :
	m_viewer(null),
	m_textMessage(null),
	m_lineColor(),
	m_lineType(Library::LINE_MISC),
	m_occlusionBuffer(null),
	m_bufferType(Library::BUFFER_COVERAGE),
	m_stencilTestValue(0),
	m_stencilWriteValue(0),
	m_commander(c),
	m_cell(null)
{
	m_prev = null;
	m_next = s_first;
	if (m_next)
		m_next->m_prev = this;
	s_first = this;
}

DPVS_FORCE_INLINE ImpCommander::~ImpCommander (void)										
{ 
	DPVS_ASSERT(!m_viewer); 
	DPVS_ASSERT(!m_cell);

	if (m_prev)
		m_prev->m_next = m_next;
	else
	{
		DPVS_ASSERT(this == s_first);
		s_first = m_next;
	}
	if (m_next)
		m_next->m_prev = m_prev;

	m_prev = null;
	m_next = null;

	DPVS_ASSERT(m_commander);
	m_commander->m_imp = null;					// make sure we break the relation between the Commander and the ImpCommander...
}

//------------------------------------------------------------------------
// Some Commander::Viewer inline code
//------------------------------------------------------------------------

DPVS_FORCE_INLINE Commander::Viewer::Viewer() : m_imp(null)
{
	InstanceCount::incInstanceCount(InstanceCount::VIEWER);
}

DPVS_FORCE_INLINE Commander::Viewer::~Viewer	(void)
{
	InstanceCount::decInstanceCount(InstanceCount::VIEWER);
}

DPVS_FORCE_INLINE Commander::Viewer::Viewer (const Viewer&) : m_imp(null)
{
	InstanceCount::incInstanceCount(InstanceCount::VIEWER);
}

DPVS_FORCE_INLINE Commander::Viewer&	Commander::Viewer::operator=	(const Viewer&)
{
	m_imp = 0;
	return *this;
}

} //namespace DPVS

//------------------------------------------------------------------------
#endif //__DPVSIMPCOMMANDER_HPP
