// ======================================================================
//
// AIDebuggingMessagesArchiveTypes.h
// copyright(c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_AIDebuggingMessagesArchiveTypes_H
#define INCLUDED_AIDebuggingMessagesArchiveTypes_H

//-----------------------------------------------------------------------


// ======================================================================

struct AIPathInfo_NodeInfo
{
	int node;
	enum eState
	{
		kReported=1,
		kCanMove=2,
		kTarget=4,
		kPassed=8,
		kFacingTarget=16,
		kInCone=32,
		kFinalTarget=64
	};
	eState state;
	bool operator==(AIPathInfo_NodeInfo const &other) const
	{
		return node == other.node && state == other.state;
	}
};

// ======================================================================

#endif
