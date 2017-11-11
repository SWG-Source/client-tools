// PRIVATE ==============================================================
//
// RenderWorld_Commander.h
// Copyright 2002, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_RenderWorld_Commander_H
#define INCLUDED_RenderWorld_Commander_H

// ======================================================================

class CellProperty;
class NetworkId;
class Transform;
class RenderWorldCamera;

#include "dpvsCommander.hpp"

// ======================================================================

class RenderWorldCommander : public DPVS::Commander
{
public:

#ifdef _DEBUG
	static int getNumberOfVisibleObjects();
	static int getNumberOfPortalsCrossed();
#endif

	static bool wasObjectRenderedThisFrame(NetworkId const & id);
	static void setCamera(const RenderWorldCamera * camera, CellProperty const * cameraCell, Transform const & cameraToWorld);

public:

	RenderWorldCommander();

	virtual void command(DPVS::Commander::Command command);

private:

	RenderWorldCommander(const RenderWorldCommander &);
	RenderWorldCommander &operator =(const RenderWorldCommander &);
};

// ======================================================================

#endif
