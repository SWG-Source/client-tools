// ======================================================================
//
// Bloom.h
// Copyright 2004, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_Bloom_H
#define INCLUDED_Bloom_H

// ======================================================================

class Bloom
{
public:

	static void install();

	static bool isSupported();

	static bool isEnabled();
	static void setEnabled(bool enabled);
		
	static void preSceneRender();
	static void postSceneRender();

private:

	static void remove();

	static void enable();
	static void disable();

	Bloom();
	Bloom(Bloom const &);
	~Bloom();
};

// ======================================================================

#endif
