//======================================================================
//
// CuiContainerSelectionChanged.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiContainerSelectionChanged_H
#define INCLUDED_CuiContainerSelectionChanged_H

//======================================================================

namespace MessageDispatch
{
	template <typename MessageType, typename IdentifierType> class Transceiver;
}

//----------------------------------------------------------------------

class ClientObject;

//----------------------------------------------------------------------

class CuiContainerSelectionChanged
{
public:
	struct Type;
	typedef std::pair<int, ClientObject *> PositionPair;
	typedef PositionPair Payload;
	typedef MessageDispatch::Transceiver <const Payload &,     Type>     TransceiverType;
};

//======================================================================

#endif
