//======================================================================
//
// InputScheme.h
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_InputScheme_H
#define INCLUDED_InputScheme_H

//======================================================================

class InputMap;
class Callback;

//----------------------------------------------------------------------

class InputScheme
{
public:
	
	enum Flags
	{
		F_modalChat    = 0x0001,
		F_mouseMode    = 0x0002,
		F_chaseCam     = 0x0004,
		F_turnStrafes  = 0x0008,
		F_modeless     = 0x0010,
		F_swgMouseMap  = 0x0020, // <-- this defines the alternate mouse mapping for the new SWG modeless interface.
		F_canFireSecondariesFromToolbar = 0x0040,
		F_mouseLeftAndRightMoves = 0x0080
	};

	typedef stdvector<std::string>::fwd StringVector;

	static Unicode::String       localizeTypeName       (const std::string & type);

	static void                  getTypes               (StringVector & sv);
	static bool                  resetFromType          (const std::string & type, bool confirmed);

	static InputMap *            fetchGroundInputMap    ();
	static void                  releaseGroundInputMap  (InputMap & imap);

	static Callback *            getResetCallback       ();

	static std::string const &   getLastInputSchemeType ();
};

//======================================================================

#endif
