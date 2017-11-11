// ======================================================================
//
// SetupClientObject.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SetupClientObject_H
#define INCLUDED_SetupClientObject_H

// ======================================================================

class SetupClientObject
{
public:

	class Data
	{
	public:

		Data ();

	private:

		bool m_viewer;

	private:

		friend class SetupClientObject;
	};

public:

	static void install (const Data& data);

	static void setupGameData (Data& data);
	static void setupToolData (Data& data);

private:

	SetupClientObject();
	SetupClientObject(const SetupClientObject &);
	SetupClientObject &operator =(const SetupClientObject &);
};

// ======================================================================

#endif
