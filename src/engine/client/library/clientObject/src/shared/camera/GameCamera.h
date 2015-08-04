//===================================================================
//
// GameCamera.h
// asommers 2-26-99
//
// Portions copyright 1999, bootprint entertainment
// Portions copyright 2000-2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
//===================================================================

#ifndef INCLUDED_GameCamera_H
#define INCLUDED_GameCamera_H

//===================================================================

#include "clientGraphics/RenderWorldCamera.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Timer.h"
#include "sharedMath/VectorArgb.h"

//===================================================================

class GameCamera : public RenderWorldCamera
{
public:

	static void install ();

	static void setColor (const VectorArgb& color);
	static void setDisableGlare (bool disableGlare);
	static bool getDisableGlare ();
	static void setUserEnableGlare (bool enableGlare);
	static bool getUserEnableGlare ();

public:

	GameCamera ();
	virtual ~GameCamera ();

	virtual float alter (float time);

	void jitter (const Vector& jitterDirection_w, float jitterTime, float jitterAngle, float jitterFrequency);
	void flash (const VectorArgb& flashColor, float flashTime);

	void renderFlash () const;

	bool isJittering() const;

protected:

	virtual void drawScene () const;

private:

	void setVertexBuffer (const VectorArgb& color) const;

private:

	GameCamera (const GameCamera&);
	GameCamera& operator= (const GameCamera&);

private:

	//-- jittering
	bool                 m_jittering;
	Vector               m_jitterDirection_w;
	Timer                m_jitterTimer;
	float                m_jitterAngle;
	float                m_jitterFrequency;

	//-- flashing
	bool                 m_flashing;
	Timer                m_flashTimer;
	VectorArgb           m_flashColor;
};

//----------------------------------------------------------------------

inline bool GameCamera::isJittering() const
{
	return m_jittering;
}

//===================================================================

#endif
