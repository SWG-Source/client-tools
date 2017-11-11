//
// SetupClientGame.h
// asommers 7-1-99
//
// copyright 1999, bootprint entertainment
// copyright 2001 Sony Online Entertainment

#ifndef INCLUDED_SetupClientGame_H
#define INCLUDED_SetupClientGame_H

//-------------------------------------------------------------------

class SetupClientGame
{
public:

	struct Data
	{
	public:

		Data ();

	public:

		bool m_viewer;
		bool m_disableLazyInteriorLayoutCreation;
	};

private:

	SetupClientGame (void);
	~SetupClientGame (void);
	SetupClientGame (const SetupClientGame&);
	SetupClientGame& operator= (const SetupClientGame&);

public:

	static void setupGameData (Data& data);
	static void setupToolData (Data& data);

	static void install (const Data& data);
	static void remove (void);
};

//-------------------------------------------------------------------

#endif
