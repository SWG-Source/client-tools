// ======================================================================
//
// SlotRule.h
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_SlotRule_H
#define INCLUDED_SlotRule_H

// ======================================================================

class Iff;
class SlottedContainer;
class Object;

// ======================================================================
/**
 * Represents a rule that can be tested against a slotted container to see
 * if its condition is met.
 *
 * The rule may actually be the root of a hierarchy of arbitrarily complex tests.
 *
 * SWG uses these rules to drive when belt and bandolier customization variables
 * get turned on and off based on what appears in certain slots.
 */

class SlotRule
{
public:

	static void      install();

	static SlotRule *create(Iff &iff);

public:

	virtual ~SlotRule();

	virtual  bool test(SlottedContainer const &container, SlottedContainer const * alternateContainer = NULL, Object * owner = NULL) const = 0;

protected:

	SlotRule();

};

// ======================================================================

#endif
