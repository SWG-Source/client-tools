// ============================================================================
//
// ColorRamp.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_ColorRamp_H
#define INCLUDED_ColorRamp_H

#include <list>
#include <string>

class ColorRampEdit;
class Iff;

//-----------------------------------------------------------------------------
class ColorRampControlPoint
{
friend class ColorRamp;

public:

	ColorRampControlPoint();
	explicit ColorRampControlPoint(float const percent, float const red, float const green, float const blue);

	float getPercent() const;
	float getRed() const;
	float getGreen() const;
	float getBlue() const;

private:

	float m_percent;  // [0..1]
	float m_red;      // [0..1]
	float m_green;    // [0..1]
	float m_blue;     // [0..1]
};

//-----------------------------------------------------------------------------
class ColorRamp
{
public:

	typedef std::list<ColorRampControlPoint> ControlPointList;

	enum InterpolationType
	{
		IT_linear,
		IT_spline,
	};

	enum SampleType
	{
		ST_all,
		ST_single,
	};

public:

	ColorRamp();
	ColorRamp(ColorRamp const &rhs);
	~ColorRamp();

	ColorRamp &  operator =(ColorRamp const &rhs);

#ifdef _DEBUG
	// Set/get the name

	std::string const &getName() const;
	void               setName(std::string const &name);
#endif // _DEBUG

	// Adds the control point into the correct place in the list

	ControlPointList::iterator insert(ColorRampControlPoint const &controlPoint);

	// Removes the control point at the iterator

	void        remove(ControlPointList::iterator &iter);

	// Changes the values of the control point at the iterator
	
	void        setControlPoint(ControlPointList::const_iterator const &iter, ColorRampControlPoint const &controlPoint);

	// Remove all the values

	void        clear();

	// Returns the color represented at the percent int the color ramp

	void        getColorAtPercent(float const percent, float &red, float &green, float &blue) const;

	// Set some random values

	void        randomize();

	// Load the waveform from an iff

	void        load(Iff &iff);

	// Write the waveform to an iff

	void        write(Iff &iff) const;

	// Get the begin and end iterator from the control point list

	ControlPointList::iterator getIteratorBegin();
	ControlPointList::iterator getIteratorEnd();
	ControlPointList::const_iterator getIteratorBegin() const;
	ControlPointList::const_iterator getIteratorEnd() const;

	// Get the number of control points in the list

	int         getControlPointCount() const;

	SampleType        getSampleType() const;
	InterpolationType getInterpolationType() const;

	void              setInterpolationType(InterpolationType const interpolationType);
	void              setSampleType(SampleType const sampleType);

private:

	void load_0000(Iff &iff);
	void load_0001(Iff &iff);

	ControlPointList  m_controlPointList;
	InterpolationType m_interpolationType;
	SampleType        m_sampleType;
	std::string       m_name;

#ifdef _DEBUG
	int  m_controlPointCount;

	// Verifies the iterator belongs to this color ramp

	bool verifyIteratorValid(ControlPointList::const_iterator const &iter) const;
#endif // _DEBUG
};

//-----------------------------------------------------------------------------
class ColorRampControlPointIter
{
public:

	ColorRampControlPointIter();

	void reset(ColorRamp::ControlPointList::const_iterator const &iter);

	ColorRamp::ControlPointList::const_iterator m_iter;
	float                                       m_randomSamplePercent; // [0..1]
};

// ============================================================================

#endif // INCLUDED_ColorRamp_H
