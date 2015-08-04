// ======================================================================
//
// Switcher.h
// jeff grills
//
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_Switcher_H
#define INCLUDED_Switcher_H

// ======================================================================

class Switcher
{
public:

	Switcher();
	virtual ~Switcher();

	virtual bool needsAlter() const = 0;
	virtual void alter(real time) = 0;
	virtual int  getState() const = 0;
	virtual int  getNumberOfStates() const = 0;

private:

	/// Disabled.
	Switcher(const Switcher &);

	/// Disabled.
	Switcher &operator =(const Switcher &);
};

// ======================================================================

#endif
