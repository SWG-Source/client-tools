// ======================================================================
//
// DetailAppearanceBuilder.h
// Portions Copyright 1999, Bootprint Entertainment, Inc.
// Portions Copyright 2000 - 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_DetailAppearanceBuilder_H
#define INCLUDED_DetailAppearanceBuilder_H

#include "MeshBuilder.h"
#include <vector>
#include <string>

class Iff;
class IndexedTriangleList;

//-------------------------------------------------------------------

class DetailAppearanceBuilder : public MeshBuilder
{
public:

	DetailAppearanceBuilder(
		MObject const & rootMayaObject, 
		bool usePivotPoint,
		bool disableLodCrossFade,
		Vector const & minVector, 
		Vector const & maxVector
	);

	virtual ~DetailAppearanceBuilder();

	void        addAppearance (int id, const char* name, real nearDistance, real farDistance);
	void addRadarShape (const IndexedTriangleList* radarShape);
	void        addTestShape (const IndexedTriangleList* testShape);
	void        addWriteShape (const IndexedTriangleList* writeShape);

	bool        sort (void);
	bool        write (const char* filename) const;

	bool hasFrames ( void ) const;

protected:

	void        clear (void);

	class Entry
	{
	public:

		Entry (int id, const char* name, real nearDistance, real farDistance);
		~Entry (void);

		int   m_id;
		std::string m_name;
		real  m_nearDistance;
		real  m_farDistance;
	};

	bool    write            ( Iff & iff ) const;
	void    writeEntries     ( Iff & iff ) const;
	void writeRadarShape (Iff& iff) const;
	void    writeTestShape   ( Iff & iff ) const;
	void    writeWriteShape  ( Iff & iff ) const;

	static bool entrySorter( Entry const * A, Entry const * B );

	typedef std::vector<Entry*> EntryList;

protected:

	EntryList    m_entryList;

	IndexedTriangleList* m_radarShape;
	IndexedTriangleList* m_testShape;
	IndexedTriangleList* m_writeShape;

	const bool m_usePivotPoint;
	const bool m_disableLodCrossFade;

private:

	DetailAppearanceBuilder();
	DetailAppearanceBuilder(DetailAppearanceBuilder const &);
	DetailAppearanceBuilder & operator=(DetailAppearanceBuilder const &);
};

// ----------------------------------------------------------------------

#endif // #ifndef INCLUDED_DetailAppearanceBuilder_H
