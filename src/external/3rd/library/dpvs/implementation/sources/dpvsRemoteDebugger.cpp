/*****************************************************************************
 *
 * dPVS
 * -----------------------------------------
 *
 * (C) 1999-2002 Hybrid Holding, Ltd.
 * All Rights Reserved.
 *
 * This file consists of unpublished, proprietary source code of
 * Hybrid Holding, and is considered Confidential Information for
 * purposes of non-disclosure agreement. Disclosure outside the terms
 * outlined in signed agreement may result in irrepairable harm to
 * Hybrid Holding and legal action against the party in breach.
 *
 * Description: 	RemoteDebugger code
 *
 * $Archive: /dpvs/implementation/sources/dpvsRemoteDebugger.cpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 4/26/02 14:27 $
 * $Date: 2003/01/22 $
 * 
 ******************************************************************************/

#include "dpvsRemoteDebugger.hpp" // place all RemoteDebugger code here

#if defined (DPVS_USE_REMOTE_DEBUGGER)

// other includes here
#include <..\..\..\remotedebugger\library\interface\hlRemoteDebugger.hpp>

#include "dpvsDefs.hpp"
#include "dpvsVector.hpp"
#include "dpvsCamera.hpp"

// pragma lib here (don't change project file!!)
#ifdef DPVS_DEBUG
#	pragma comment(lib,"../../../remotedebugger/library/hlremotedebugger_d.lib")
#else
#	pragma comment(lib,"../../../remotedebugger/library/hlremotedebugger.lib")
#endif

using namespace DPVS;

namespace 
{
static struct 
{
	int						width;
	int						height;
	HL::RemoteDebugger *	rd;
	HL::TextChannel *		messageChannel;
	HL::VectorGraphChannel*	vec2DChannel;
}
data =
{
	100,
	100,
	null,
	null,
	null
};
}; // End of the empty namespace

// implementation here
// the actual method calls are in dpvsWrapper.cpp (see Library::init(), Library::exit() and Camera::resolveVisibility().
// Additional data can be supplied with Library::textCommand()

/*****************************************************************************
 *
 * Function:		RemoteDebuggerWrapper::init()
 *
 * Description:		Initializes the RemoteDebugger and trys to connect to the 
 *					local host.
 *
 * Returns:			
 *
 * Notes:			
 *
 *****************************************************************************/

void RemoteDebuggerWrapper::init(void)
{
	DPVS_ASSERT(!data.rd);
	DPVS_ASSERT(!data.messageChannel);

	Vector4 black(0,0,0,1);

	// Initialize RemoteDebugger
	data.rd = new HL::RemoteDebugger("Hybrid dPVS", null);
	data.rd->setReceiveUserMessages(true);
	data.rd->setChannelAcceptWait(1);
	data.rd->setSampleSendWait(10);

	// Initialize channels

	// Main message channel
	data.messageChannel = new HL::TextChannel("Message Channel", data.rd);

	// 2D vector channel
	data.vec2DChannel = new HL::VectorGraphChannel("2D vectors", data.rd);
	data.vec2DChannel->setBackgroundColor(reinterpret_cast<const HL::Vector4 &>(black));

}


/*****************************************************************************
 *
 * Function:		RemoteDebuggerWrapper::update()
 *
 * Description:		Sends samples of the data to the server.
 *
 * Notes:			
 *
 *****************************************************************************/

void RemoteDebuggerWrapper::update(void) 
{
	DPVS_ASSERT(data.rd && data.messageChannel);

	data.messageChannel->outputString("Frame drawn!\n");
	data.rd->flushChannels();
	
	//--------------------------------------------------------------------
	//This is supposedly called per frame. Let's clean vecs once per frame. 
	//--------------------------------------------------------------------
	
	data.vec2DChannel->clear();
	//data.vec2DChannel->setBackgroundColor(reinterpret_cast<const HL::Vector4 &>(Vector4(0,0,0,1)));
}	

/*****************************************************************************
 *
 * Function:		RemoteDebuggerWrapper::shutdown()
 *
 * Description:		Shuts down the remote debugger connection.
 *
 * Returns:			
 *
 * Notes:			
 *
 *****************************************************************************/

void RemoteDebuggerWrapper::shutdown(void)
{
	delete data.messageChannel;
	delete data.vec2DChannel;
	delete data.rd;

	data.messageChannel = null;
	data.vec2DChannel	= null;
	data.rd             = null;

}

/*****************************************************************************
 *
 * Function:		RemoteDebuggerWrapper::drawLine()
 *
 * Description:		Draws a 2D line to buffer.
 *
 * Parameters:		a		= Starting point of the line
 *					b		= End point of the line
 *					color	= color of the line
 *
 * Notes:			
 *
 *****************************************************************************/

void RemoteDebuggerWrapper::drawLine(const Vector2& a, const Vector2& b, const Vector4& color)
{
	static Vector3 * conv3D = new Vector3[2];
	conv3D[0][0] = a[0]/data.width;
	conv3D[0][1] = a[1]/data.height;
	conv3D[0][2] = 0;
	conv3D[1][0] = b[0]/data.width;
	conv3D[1][1] = b[1]/data.height;
	conv3D[1][2] = 0;
	data.vec2DChannel->setForegroundColor(reinterpret_cast<const HL::Vector4 &>(color));
	data.vec2DChannel->addLineList(reinterpret_cast<const HL::Vector3 *>(&conv3D[0]), 2);
}



void RemoteDebuggerWrapper::setScreenSize(int width, int height) 
{
	data.width = width;
	data.height = height;
}

#endif // DPVS_USE_REMOTE_DEBUGGER
