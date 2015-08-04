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
 * Description: 	Debugging code
 *
 * $Archive: /dpvs/implementation/sources/dpvsDebug.cpp $
 * $Author: wili $ 
 * $Revision: #2 $
 * $Modtime: 4/24/02 17:25 $
 * $Date: 2003/08/24 $
 * 
 ******************************************************************************/

#include "dpvsDebug.hpp"
#include "dpvsSurface.hpp"
#include "dpvsDataPasser.hpp"
#include "dpvsImpCommander.hpp"
#include "dpvsRemoteDebugger.hpp"

#include <cstdarg>
#include <cstdio>

using namespace DPVS;

//------------------------------------------------------------------------
// Static variables
//------------------------------------------------------------------------

UINT32					Debug::s_lineDrawFlags		= 0;
UINT32					Debug::s_bufferFlags		= 0;
Surface*				Debug::s_defaultSurface		= null;
Library::Services		Debug::s_defaultServices;	// default instance
Library::Services*		Debug::s_services			= &s_defaultServices;

/*****************************************************************************
 *
 * Function:		DPVS::Debug::init()
 *
 * Description:		Initializes the debug system and assigns the
 *					Services object
 *
 * Parameters:		s = pointer to Services object or null
 *
 * Notes:			If 'null' is passed, the default Services object is
 *					used.
 *
 *****************************************************************************/

void Debug::init (Library::Services* s)
{
	setServices(s);
	DPVS_ASSERT(!s_defaultSurface);
}

/*****************************************************************************
 *
 * Function:		DPVS::Debug::getDefaultSurface()
 *
 * Description:		Returns reference to the "default surface"
 *
 * Returns:			reference to the "default surface"
 *
 *****************************************************************************/

Surface& Debug::getDefaultSurface	(void)						
{ 
	if (!s_defaultSurface)
		s_defaultSurface = NEW<Surface>();
	return *s_defaultSurface; 
}

/*****************************************************************************
 *
 * Function:		DPVS::Debug::exit()
 *
 * Description:		Shuts down the debug system
 *
 * Notes:			Called only by Library::exit()
 *
 *****************************************************************************/

void Debug::exit (void)
{
	DELETE(s_defaultSurface);
	s_defaultSurface	= null;
	// note that services is not changed to null yet, because the Library::exit()
	// function needs to run heap shutdown still
}

/******************************************************************************
 *
 * Function:		Debug::print ( SRCSTRING format, ... )
 *
 * Description:		Operates exactly like standard C library printf() except
 *					that the output is redirected to the application.
 *					The maximum number of characters a single print() call can
 *					output is 1024;
 *
 * Parameters:		format	= format string
 *					...		= any number of parameters
 *
 * Notes:			There is no debug checking for the length of the string
 *					because the vsprintf() variants with maximum lengths
 *					are not available in a portable fashion...
 *
 ******************************************************************************/

void Debug::print(const char* format, ... )
{
	Commander* cmd = DataPasser::getCommander();

	if (!cmd || !format)								// no commander or format specified
		return;
		
	char string[1024+1];								// temporary array

	va_list		argptr;
	va_start	(argptr, format);
	vsprintf	(string, format, argptr);
	va_end		(argptr);

#if defined (DPVS_DEBUG)
	UINT32 oldFPUMode = getFPUMode();
#endif

	cmd->getImplementation()->setTextMessage(string);	// set string
	cmd->command(Commander::TEXT_MESSAGE);				// report to user
	cmd->getImplementation()->setTextMessage(null);		// set null

	DPVS_ASSERT(getFPUMode() == oldFPUMode && "FPU mode changed in user callback!");
}

/******************************************************************************
 *
 * Function:		Debug::drawline(Library::LineType type,const Vector2& a, const Vector2& b, const Vector4& color)
 *
 * Description:		Directs 2D line parameters to Commander::command()
 *
 * Parameters:		type	= type of the line
 *					a		= start point of the line
 *					b		= end point of the line
 *					color	= color of the line
 *
 ******************************************************************************/

void Debug::drawLine (Library::LineType type,const Vector2& a, const Vector2& b, const Vector4& color)
{
	Commander* c = DataPasser::getCommander();
	if (!c)													// no commander?
		return;

#if defined (DPVS_USE_REMOTE_DEBUGGER)
	RemoteDebuggerWrapper::drawLine(a,b,color);
#endif
	c->getImplementation()->setLine2D(type,a,b,color);		// set 2D line

#if defined (DPVS_DEBUG)
	UINT32 oldFPUMode = getFPUMode();
#endif

	c->command(Commander::DRAW_LINE_2D);					// report to user
	DPVS_ASSERT(getFPUMode() == oldFPUMode && "FPU mode changed in user callback!");
}

/******************************************************************************
 *
 * Function:		Debug::drawline(Library::LineType type,const Vector3& a, const Vector3& b, const Vector4& color)
 *
 * Description:		Directs 3D line parameters to Commander::command()
 *
 * Parameters:		type	= type of the line
 *					a		= start point of the line
 *					b		= end point of the line
 *					color	= color of the line
 *
 ******************************************************************************/

void Debug::drawLine(Library::LineType type,const Vector3& a, const Vector3& b, const Vector4& color)
{
	Commander* c = DataPasser::getCommander();				// no commander?
	if (!c)
		return;
	c->getImplementation()->setLine3D(type,a,b,color);

#if defined (DPVS_DEBUG)
	UINT32 oldFPUMode = getFPUMode();
#endif

	c->command(Commander::DRAW_LINE_3D);					// report to user
	DPVS_ASSERT(getFPUMode() == oldFPUMode && "FPU mode changed in user callback!");
}

//------------------------------------------------------------------------
