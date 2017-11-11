// ======================================================================
//
// CuiChatterSpeechManager.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CuiChatterSpeechManager_H
#define INCLUDED_CuiChatterSpeechManager_H

// ====================================================================== 

//----------------------------------------------------------------------

class CuiChatterSpeechManager
{
public:
	static void install ();
	static void remove ();

	static stdvector<std::string>::fwd const & getData();

private:
	static void loadData();

private:
	CuiChatterSpeechManager ();
	CuiChatterSpeechManager (const CuiChatterSpeechManager & rhs);
	CuiChatterSpeechManager & operator= (const CuiChatterSpeechManager & rhs);
};

// ======================================================================

#endif
