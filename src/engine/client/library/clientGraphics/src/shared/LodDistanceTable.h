// ======================================================================
//
// LodDistanceTable.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_LodDistanceTable_H
#define INCLUDED_LodDistanceTable_H

// ======================================================================

class Iff;

// ======================================================================

class LodDistanceTable
{
public:

	explicit LodDistanceTable(Iff &iff);
	~LodDistanceTable();

	int   getDetailLevel(float distanceFromCameraSquared, int currentDetailLevel, float lodBias) const;

	void  write(Iff &iff);

private:

	struct Level;

	typedef stdvector<Level>::fwd  LevelVector;

private:

	void load_0000(Iff &iff);

	// Disabled.
	LodDistanceTable();
	LodDistanceTable(const LodDistanceTable&);
	LodDistanceTable &operator =(const LodDistanceTable&);

private:

	LevelVector *m_levels;
	int          m_levelCount;

};

// ======================================================================

#endif
