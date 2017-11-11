// ======================================================================
// 
// AiDebugStringManager.h
// Copyright 2004 Sony Online Entertainment, Inc.
//
// ======================================================================

#ifndef INCLUDED_AiDebugStringManager_H
#define INCLUDED_AiDebugStringManager_H

#ifdef _DEBUG

class NetworkId;

// ----------------------------------------------------------------------
class AiDebugStringManager
{
public:

	static void install();
	static void add(NetworkId const & object, std::string const & text);
	static void alter();
	static int getNumberProcessedLastFrame();

private:

	// Disable

	AiDebugStringManager();
	~AiDebugStringManager();
	AiDebugStringManager(AiDebugStringManager const &);
	AiDebugStringManager & operator =(AiDebugStringManager const &);
};

// ======================================================================

#endif // _DEBUG
#endif // INCLUDED_AiDebugStringManager_H
