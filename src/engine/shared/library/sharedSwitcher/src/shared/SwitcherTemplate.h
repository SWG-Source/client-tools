// ======================================================================
//
// SwitcherTemplate.h
// jeff grills
//
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwitcherTemplate_H
#define INCLUDED_SwitcherTemplate_H

// ======================================================================

class Iff;
class Switcher;

// ======================================================================

class SwitcherTemplate
{
public:

	static SwitcherTemplate *create(Iff &iff);

public:

	SwitcherTemplate();
	virtual ~SwitcherTemplate();

	virtual Switcher *create() const = 0;

private:

	/// Disabled.
	SwitcherTemplate(const SwitcherTemplate &);

	/// Disabled.
	SwitcherTemplate &operator =(const SwitcherTemplate &);
};

// ======================================================================

#endif
