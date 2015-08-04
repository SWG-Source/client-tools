// ======================================================================
//
// SpaceTargetBracketOverlay.h
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_SpaceTargetBracketOverlay_H
#define INCLUDED_SpaceTargetBracketOverlay_H

// ======================================================================

class NetworkId;
class Transform;

// ======================================================================

class SpaceTargetBracketOverlay
{
public:

	static void install();

	static void update(float elapsedTime);

	static bool getShowTargetStatusForAllShips();
	static void setShowTargetStatusForAllShips(bool showTargetStatusForAllShips);

	static void setCommTarget(NetworkId const & commTarget);

	static void setDisabled(bool b);

public:

	SpaceTargetBracketOverlay();
	~SpaceTargetBracketOverlay();

	void render() const;
};

// ======================================================================

#endif
