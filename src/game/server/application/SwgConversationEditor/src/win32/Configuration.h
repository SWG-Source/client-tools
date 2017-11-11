// ======================================================================
//
// Configuration.h
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_Configuration_H
#define INCLUDED_Configuration_H

// ======================================================================

class Configuration
{
public:

	static bool install ();

	static void populateAnimationActions (CComboBox & comboBox);

	static CString const & getEmptyAnimationAction ();

private:

	static bool loadIni ();
};

// ======================================================================

#endif
