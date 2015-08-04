//======================================================================
//
// SwgCuiSpaceRadarPlanar_Blip.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiSpaceRadarPlanar_Blip_H
#define INCLUDED_SwgCuiSpaceRadarPlanar_Blip_H

//======================================================================
#include "swgClientUserInterface/SwgCuiSpaceRadarPlanar.h"

#include "sharedMath/Vector.h"

//----------------------------------------------------------------------

class UIWidget;
class SwgCuiSpaceRadarPlanar;

//----------------------------------------------------------------------

class SwgCuiSpaceRadarPlanar::Blip
{
public:
	explicit Blip(SwgCuiSpaceRadarPlanar & planarRadar);
	~Blip();

private:
	Blip();
	Blip(Blip const &);
	Blip const & operator=(Blip const &);

	void setVisible(bool value);

private:
	SwgCuiSpaceRadarPlanar & m_radar;

	float m_decayTimer;
	Vector m_blipvert;
	bool m_isDamaging;

	UIWidget * m_imageHead;
	UIWidget * m_imageTail;

	friend class SwgCuiSpaceRadarPlanar;
};

//======================================================================

#endif
