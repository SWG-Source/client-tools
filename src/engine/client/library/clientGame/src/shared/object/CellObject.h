// ======================================================================
//
// CellObject.h
// Copyright 2001 Sony Online Entertainment
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_CellObject_H
#define INCLUDED_CellObject_H

// ======================================================================

#include "clientGame/ClientObject.h"
#include "sharedMathArchive/VectorArchive.h"

class IndexedTriangleList;

// ======================================================================

class CellObject : public ClientObject
{
public:
	struct Messages
	{
		struct PublicChanged
		{
			typedef CellObject Payload;
		};
	};

	static CellObject * getCellObject(NetworkId const & networkId);
	static CellObject * asCellObject(Object * object);
	static CellObject const * asCellObject(Object const * object);

	virtual CellObject *asCellObject();
	virtual CellObject const *asCellObject() const;

	explicit CellObject(const SharedObjectTemplate *newTemplate);
	virtual ~CellObject();

	virtual void endBaselines();

	void setCell(int cellNumber);
	int  getCell() const;
	Unicode::String const & getCellLabel() const;
	Vector const & getLabelOffset() const;
	void setAccessAllowed(bool allowed);
	bool getAccessAllowed() const;

	const IndexedTriangleList* getRadarShape () const;
	const stdvector<Vector>::fwd* getRadarEdges () const;
	const stdvector<Vector>::fwd* getRadarPortalEdges () const;

	virtual void getObjectInfo(stdmap<std::string, stdmap<std::string, Unicode::String>::fwd >::fwd & propertyMap) const;

private:
	CellObject();
	CellObject(const CellObject &);
	CellObject &operator =(const CellObject &);

private:
	struct Callbacks
	{
		struct PublicChanged
		{
			void modified(CellObject & target, const bool &oldValue, const bool & newValue, bool local);
		};
	};

	Archive::AutoDeltaVariableCallback<bool, Callbacks::PublicChanged, CellObject> m_isPublic;
	Archive::AutoDeltaVariable<int>  m_cellNumber;
	Archive::AutoDeltaVariable<Unicode::String> m_cellLabel;
	Archive::AutoDeltaVariable<Vector> m_labelLocationOffset;

	bool m_accessAllowed;

	IndexedTriangleList*             m_radarShape;
	stdvector<Vector>::fwd*          m_radarEdges;
	stdvector<Vector>::fwd*          m_radarPortalEdges;
};

// ======================================================================

inline void CellObject::setCell(int cellNumber)
{
	m_cellNumber = cellNumber;
}

// ----------------------------------------------------------------------

inline int CellObject::getCell() const
{
	return m_cellNumber.get();
}

// ----------------------------------------------------------------------

inline Unicode::String const & CellObject::getCellLabel () const
{
	return m_cellLabel.get();
}

// ----------------------------------------------------------------------

inline Vector const & CellObject::getLabelOffset() const
{
	return m_labelLocationOffset.get();
}

// ======================================================================

#endif
