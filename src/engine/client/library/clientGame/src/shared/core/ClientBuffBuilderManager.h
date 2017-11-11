// ======================================================================
//
// ClientBuffBuilderManager.h
// copyright (c) 2006 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ClientBuffBuilderManager_H
#define INCLUDED_ClientBuffBuilderManager_H

// ======================================================================

namespace MessageDispatch
{
	class Callback;
};

//-----------------------------------------------------------------------

class ClientBuffBuilderManager
{
public:
	static void install();

private:
	//disabled
	ClientBuffBuilderManager (ClientBuffBuilderManager const & rhs);
	ClientBuffBuilderManager & operator= (ClientBuffBuilderManager const & rhs);

private:
	static void remove();
};

// ======================================================================

#endif
