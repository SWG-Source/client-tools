// ============================================================================
//
// SampleId.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_SampleId_H
#define INCLUDED_SampleId_H

// ============================================================================
//
// SampleId
//
// ============================================================================

//-----------------------------------------------------------------------------
class SampleId
{
public:

	SampleId();
	explicit SampleId(int const id);

	int  getId() const;
	void invalidate();
	bool isValid() const;

	// The less than (<) operator must be overloaded because it is used in a map

	bool operator <(SampleId const &rhs) const;

private:

	int m_id;
};

//-----------------------------------------------------------------------------
inline bool SampleId::operator <(SampleId const &rhs) const
{
	return (m_id < rhs.m_id);
}

// ============================================================================

#endif // INCLUDED_SampleId_H
