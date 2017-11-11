// ======================================================================
//
// ClientHeadTracking.h
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_ClientHeadTracking_H
#define INCLUDED_ClientHeadTracking_H

// ======================================================================

class ClientHeadTracking
{
public:

	static void install();

	static bool isSupported();
	static bool getEnabled();
	static void setEnabled(bool enabled);
	static void getYawAndPitch(float & yaw, float & pitch);
};

// ======================================================================

#endif
