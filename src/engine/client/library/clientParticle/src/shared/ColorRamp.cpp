// ============================================================================
//
// ColorRamp.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "clientParticle/FirstClientParticle.h"
#include "clientParticle/ColorRamp.h"

#include "sharedRandom/Random.h"
#include "sharedFile/Iff.h"
#include "sharedMath/CatmullRomSpline.h"

// ============================================================================
//
// ColorRampControlPoint
//
// ============================================================================

//-----------------------------------------------------------------------------
ColorRampControlPointIter::ColorRampControlPointIter()
 : m_iter()
 , m_randomSamplePercent(-1.0f)
{
}

//-----------------------------------------------------------------------------
void ColorRampControlPointIter::reset(ColorRamp::ControlPointList::const_iterator const &iter)
{
	m_iter = iter;
	m_randomSamplePercent = Random::randomReal(0.0f, 1.0f);
}

// ============================================================================
//
// ColorRampControlPoint
//
// ============================================================================

//-----------------------------------------------------------------------------
ColorRampControlPoint::ColorRampControlPoint()
 : m_percent(0.0f)
 , m_red(1.0f)
 , m_green(1.0f)
 , m_blue(1.0f)
{
}

//-----------------------------------------------------------------------------
ColorRampControlPoint::ColorRampControlPoint(float const percent, float const red, float const green, float const blue)
 : m_percent(percent)
 , m_red(red)
 , m_green(green)
 , m_blue(blue)
{
}

//-----------------------------------------------------------------------------
float ColorRampControlPoint::getPercent() const
{
	return m_percent;
}

//-----------------------------------------------------------------------------
float ColorRampControlPoint::getRed() const
{
	return m_red;
}

//-----------------------------------------------------------------------------
float ColorRampControlPoint::getGreen() const
{
	return m_green;
}

//-----------------------------------------------------------------------------
float ColorRampControlPoint::getBlue() const
{
	return m_blue;
}

// ============================================================================
//
// ColorRamp
//
// ============================================================================

Tag const TAG_CLRR = TAG(C,L,R,R);

//-----------------------------------------------------------------------------
ColorRamp::ColorRamp()
 : m_controlPointList()
 , m_interpolationType(IT_linear)
 , m_sampleType(ST_all)
#ifdef _DEBUG
 , m_name("Color")
 , m_controlPointCount(0)
#endif // _DEBUG
{
}

//-----------------------------------------------------------------------------
ColorRamp::ColorRamp(ColorRamp const &rhs)
 : m_controlPointList(rhs.m_controlPointList)
 , m_interpolationType(rhs.m_interpolationType)
 , m_sampleType(rhs.m_sampleType)
#ifdef _DEBUG
 , m_name(rhs.m_name)
 , m_controlPointCount(0)
#endif // _DEBUG
{
#ifdef _DEBUG
	m_controlPointCount = static_cast<int>(m_controlPointList.size());
#endif // _DEBUG
}

//-----------------------------------------------------------------------------
ColorRamp::~ColorRamp()
{
}

//-----------------------------------------------------------------------------
ColorRamp &ColorRamp::operator =(ColorRamp const &rhs)
{
	if (this == &rhs)
	{
		return *this;
	}

	m_controlPointList = rhs.m_controlPointList;

#ifdef _DEBUG
	m_controlPointCount = static_cast<int>(m_controlPointList.size());
	m_name = rhs.m_name;
#endif // _DEBUG

	m_interpolationType = rhs.m_interpolationType;
	m_sampleType = rhs.m_sampleType;

	return *this;
}

//-----------------------------------------------------------------------------
ColorRamp::ControlPointList::iterator ColorRamp::insert(ColorRampControlPoint const &controlPoint)
{
	// Insert the control point in the correct place in the list based on the
	// percent value

	bool inserted = false;
	ControlPointList::iterator result = m_controlPointList.begin();
	ControlPointList::iterator current = m_controlPointList.begin();

	for (; current != m_controlPointList.end(); ++current)
	{
		if (controlPoint.getPercent() < current->getPercent())
		{
			result = m_controlPointList.insert(current, controlPoint);
			inserted = true;

			break;
		}
	}

	// This handles inserting in to the list if there is nothing in the list
	// or if the item needs to be at the end of the list

	if (!inserted)
	{
		result = m_controlPointList.insert(m_controlPointList.end(), controlPoint);
	}

#ifdef _DEBUG
	m_controlPointCount = static_cast<int>(m_controlPointList.size());
#endif // _DEBUG

	return result;
}

//-----------------------------------------------------------------------------
void ColorRamp::remove(ControlPointList::iterator &iter)
{
	iter = m_controlPointList.erase(iter);

#ifdef _DEBUG
	m_controlPointCount = static_cast<int>(m_controlPointList.size());
#endif // _DEBUG
}

//-----------------------------------------------------------------------------
void ColorRamp::setControlPoint(ControlPointList::const_iterator const &iter, ColorRampControlPoint const &controlPoint)
{
	for (ControlPointList::iterator current = m_controlPointList.begin(); current != m_controlPointList.end(); ++current)
	{
		if (current == iter)
		{
			float percent = controlPoint.getPercent();

			// If this is the start or end control point, then we know what the percent must be

			ControlPointList::iterator temp = current;

			if (iter == m_controlPointList.begin())
			{
				// Start control point

				percent = 0.0f;
			}
			else if ((iter != m_controlPointList.end()) &&
			         (++temp == m_controlPointList.end()))
			{
				// End control point

				percent = 1.0f;
			}
			else
			{
				// Clamp the percent of the neighboring control points

				ControlPointList::iterator temp = current;

				if ((temp != m_controlPointList.begin()) &&
					(temp != m_controlPointList.end()) &&
					 (++temp != m_controlPointList.end()))
				{
					ControlPointList::iterator previous = current;
					ControlPointList::iterator next = current;

					--previous;
					++next;

					if (percent < 0.0f)
					{
						percent = 0.0f;
					}
					else if (percent > 1.0f)
					{
						percent = 1.0f;
					}

					if (percent < previous->getPercent())
					{
						// See if we need to push any of the control points to
						// the left

						while ((previous != m_controlPointList.begin()) &&
							   (previous->getPercent() > percent))
						{
							float const red = previous->getRed();
							float const green = previous->getGreen();
							float const blue = previous->getBlue();

							*previous = ColorRampControlPoint(percent, red, green, blue);
							--previous;
						}
					}
					else if (percent > next->getPercent())
					{
						// See if we need to push any of the control points to
						// the right

						while ((next != m_controlPointList.end()) &&
							   (next->getPercent() < percent))
						{
							float const red = next->getRed();
							float const green = next->getGreen();
							float const blue = next->getBlue();

							*next = ColorRampControlPoint(percent, red, green, blue);
							++next;
						}
					}
				}
			}

			// Set control point

			float const red = controlPoint.getRed();
			float const green = controlPoint.getGreen();
			float const blue = controlPoint.getBlue();

			*current = ColorRampControlPoint(percent, red, green, blue);

			break;
		}
	}
}

//-----------------------------------------------------------------------------
void ColorRamp::clear()
{
	m_controlPointList.clear();

#ifdef _DEBUG
	m_controlPointCount = static_cast<int>(m_controlPointList.size());
#endif // _DEBUG
}

//-----------------------------------------------------------------------------
void ColorRamp::getColorAtPercent(float const percent, float &red, float &green, float &blue) const
{
	DEBUG_FATAL(percent < 0.0f, ("ColorRamp::getColorAtPercent() - The percent must be >= 0: %f", percent));
	DEBUG_FATAL(percent > 1.0f, ("ColorRamp::getColorAtPercent() - The percent must be <= 1: %f", percent));
	DEBUG_FATAL(m_controlPointList.size() < 2, ("ColorRamp::getColorAtPercent() - There must be at least 2 control points, currently there is %d control points.", m_controlPointList.size()));

	// Set default values

	red = 0.0f;
	green = 0.0f;
	blue = 0.0f;

	switch (m_interpolationType)
	{
		case IT_linear:
			{
				ControlPointList::const_iterator current = m_controlPointList.begin();
				ControlPointList::const_iterator previous = current;
				++current;

				for (; current != m_controlPointList.end(); ++current)
				{
					float const previousPercent = previous->getPercent();
					float const currentPercent = current->getPercent();

					DEBUG_FATAL(previousPercent > currentPercent, ("ColorRamp::getColorAtPercent() - previousPercent (%.3f) > currentPercent (%.3f)", previousPercent, currentPercent));

					if ((percent >= previousPercent) &&
						(percent <= currentPercent))
					{
						float const differencePercent = currentPercent - previousPercent;
						float const relativePercent = (differencePercent <= 0.0f) ? 0.0f : ((percent - previousPercent) / differencePercent);
						float const differenceRed = static_cast<float>(current->getRed() - previous->getRed());
						float const differenceGreen = static_cast<float>(current->getGreen() - previous->getGreen());
						float const differenceBlue = static_cast<float>(current->getBlue() - previous->getBlue());

						red = previous->getRed() + (differenceRed * relativePercent);
						green = previous->getGreen() + (differenceGreen * relativePercent);
						blue = previous->getBlue() + (differenceBlue * relativePercent);

						break;
					}

					previous = current;
				}
			}
			break;
		case IT_spline:
			{
				float c1x;
				float c1y;
				float c2x;
				float c2y;
				float c3x;
				float c3y;
				float c4x;
				float c4y;
				float resultX;
				float resultY;

				// We have to find the four neighboring control points

				ControlPointList::const_iterator c1 = m_controlPointList.begin();
				ControlPointList::const_iterator c2 = m_controlPointList.begin();
				ControlPointList::const_iterator c3 = m_controlPointList.begin();
				ControlPointList::const_iterator c4 = m_controlPointList.begin();

				if (m_controlPointList.size() == 2)
				{
					// Handling 2 controls points is easy

					++c3;
					++c4;
				}
				else
				{
					// Find the correct positions of the control points

					int index = 0;

					for (; c3 != m_controlPointList.end();)
					{
						++c3;
						c4 = c3;
						++c4;

						if (c4 == m_controlPointList.end())
						{
							c4 = c3;
						}

						float const previousPercent = c2->getPercent();
						float const currentPercent = c3->getPercent();

						DEBUG_FATAL((previousPercent > currentPercent), ("ColorRamp::getColorAtPercent() - previousPercent(%.3f) > currentPercent(%.3f)", previousPercent, currentPercent));

						if ((percent >= previousPercent) &&
							(percent <= currentPercent))
						{
							break;
						}
						else
						{
							++c2;

							if (index > 0)
							{
								++c1;
							}
						}

						++index;
					}
				}

				// Find the position between the second and third control points

				float const difference = c3->getPercent() - c2->getPercent();
				float const t = (difference > 0.0f) ? ((percent - c2->getPercent()) / difference) : 0.0f;

				DEBUG_FATAL((t < 0.0f), ("ColorRamp::getColorAtPercent() - The spline position t(%f) is < 0", t));
				DEBUG_FATAL((t > 1.0f), ("ColorRamp::getColorAtPercent() - The spline position t(%f) is > 1", t));

				// Now that we have found the correct position of the control points,
				// we can find the individual color components

				c1x = c1->getPercent();
				c2x = c2->getPercent();
				c3x = c3->getPercent();
				c4x = c4->getPercent();

				// Red

				c1y = c1->getRed();
				c2y = c2->getRed();
				c3y = c3->getRed();
				c4y = c4->getRed();

				CatmullRomSpline::getCatmullRomSplinePoint(c1x, c1y, c2x, c2y, c3x, c3y, c4x, c4y, t, resultX, resultY);

				red = clamp(0.0f, resultY, 1.0f);

				// Green

				c1y = c1->getGreen();
				c2y = c2->getGreen();
				c3y = c3->getGreen();
				c4y = c4->getGreen();

				CatmullRomSpline::getCatmullRomSplinePoint(c1x, c1y, c2x, c2y, c3x, c3y, c4x, c4y, t, resultX, resultY);

				green = clamp(0.0f, resultY, 1.0f);

				// Blue

				c1y = c1->getBlue();
				c2y = c2->getBlue();
				c3y = c3->getBlue();
				c4y = c4->getBlue();

				CatmullRomSpline::getCatmullRomSplinePoint(c1x, c1y, c2x, c2y, c3x, c3y, c4x, c4y, t, resultX, resultY);

				blue = clamp(0.0f, resultY, 1.0f);
			}
			break;
		default:
			{
				DEBUG_FATAL(true, ("ColorRamp::getColorAtPercent() - Invalid interpolation type specified."));
			}
			break;
	}

	DEBUG_FATAL((red < 0.0f) || (red > 1.0f), ("red is out of range: %f", red));
	DEBUG_FATAL((green < 0.0f) || (green > 1.0f), ("green is out of range: %f", green));
	DEBUG_FATAL((blue < 0.0f) || (blue > 1.0f), ("blue is out of range: %f", blue));
}

//-----------------------------------------------------------------------------
void ColorRamp::randomize()
{
	// Remove all the control points

	clear();

	// Add some random control points

	insert(ColorRampControlPoint(0.0f, Random::randomReal(0.0f, 1.0f), Random::randomReal(0.0f, 1.0f), Random::randomReal(0.0f, 1.0f)));

	int const controlPoints = (rand() % 7) + 1;
	float const delta = 1.0f / static_cast<float>(controlPoints + 1);

	for (int i = 0; i < controlPoints; ++i)
	{
		float const percent = delta + delta * static_cast<float>(i);

		insert(ColorRampControlPoint(percent, Random::randomReal(0.0f, 1.0f), Random::randomReal(0.0f, 1.0f), Random::randomReal(0.0f, 1.0f)));
	}

	insert(ColorRampControlPoint(1.0f, Random::randomReal(0.0f, 1.0f), Random::randomReal(0.0f, 1.0f), Random::randomReal(0.0f, 1.0f)));
}

//-----------------------------------------------------------------------------
void ColorRamp::load(Iff &iff)
{
	iff.enterForm(TAG_CLRR);
	{
		switch (iff.getCurrentName())
		{
			case TAG_0000:
				{
					load_0000(iff);
					break;
				}
			case TAG_0001:
				{
					load_0001(iff);
					break;
				}
			default:
				{
					char currentTagName[5];
					ConvertTagToString(iff.getCurrentName(), currentTagName);
		
					FATAL(true, ("ColorRamp::load() - Unsupported data version: %s", currentTagName));
				}
		}
	}
	iff.exitForm(TAG_CLRR);
}

//-----------------------------------------------------------------------------
void ColorRamp::load_0000(Iff &iff)
{
	m_controlPointList.clear();
	
	iff.enterChunk(TAG_0000);
	{
		// Interpolation type
	
		m_interpolationType = static_cast<InterpolationType>(iff.read_uint32());
	
		// Number of control points
		
		int const controlPointCount = static_cast<int>(iff.read_uint32());
	
		// Data for each control point

		for (int i = 0; i < controlPointCount; ++i)
		{
			float const percent = iff.read_float();
			float const red = iff.read_float();
			float const green = iff.read_float();
			float const blue = iff.read_float();
	
			DEBUG_FATAL((percent < 0.0f) || (percent > 1.0f), ("ColorRamp::load_0000() - Percent out of range: (%.3f)", percent));
			DEBUG_FATAL((red < 0.0f) || (red > 1.0f), ("ColorRamp::load_0000() - Red out of range: (%.3f)", red));
			DEBUG_FATAL((green < 0.0f) || (green > 1.0f), ("ColorRamp::load_0000() - Green out of range: (%.3f)", green));
			DEBUG_FATAL((blue < 0.0f) || (blue > 1.0f), ("ColorRamp::load_0000() - Blue out of range: (%.3f)", blue));

			ColorRampControlPoint controlPoint(percent, red, green, blue);
	
			insert(controlPoint);
		}
	}
	iff.exitChunk(TAG_0000);
}

//-----------------------------------------------------------------------------
void ColorRamp::load_0001(Iff &iff)
{
	m_controlPointList.clear();
	
	iff.enterChunk(TAG_0001);
	{
		// Interpolation type
	
		m_interpolationType = static_cast<InterpolationType>(iff.read_uint32());
	
		// Sample type
	
		m_sampleType = static_cast<SampleType>(iff.read_uint32());

		// Number of control points
		
		int const controlPointCount = static_cast<int>(iff.read_uint32());
	
		// Data for each control point

		for (int i = 0; i < controlPointCount; ++i)
		{
			float const percent = iff.read_float();
			float const red = iff.read_float();
			float const green = iff.read_float();
			float const blue = iff.read_float();

			DEBUG_FATAL((percent < 0.0f) || (percent > 1.0f), ("ColorRamp::load_0001() - Percent out of range: (%.3f)", percent));
			DEBUG_FATAL((red < 0.0f) || (red > 1.0f), ("ColorRamp::load_0001() - Red out of range: (%.3f)", red));
			DEBUG_FATAL((green < 0.0f) || (green > 1.0f), ("ColorRamp::load_0001() - Green out of range: (%.3f)", green));
			DEBUG_FATAL((blue < 0.0f) || (blue > 1.0f), ("ColorRamp::load_0001() - Blue out of range: (%.3f)", blue));
	
			ColorRampControlPoint controlPoint(percent, red, green, blue);
	
			insert(controlPoint);
		}
	}
	iff.exitChunk(TAG_0001);
}

//-----------------------------------------------------------------------------
void ColorRamp::write(Iff &iff) const
{
	iff.insertForm(TAG_CLRR);
	{
		iff.insertChunk(TAG_0001);
		{
			// Interpolation type

			iff.insertChunkData(static_cast<uint32>(m_interpolationType));

			// Sample type

			iff.insertChunkData(static_cast<uint32>(m_sampleType));

			// Number of control points
			
			int const controlPointCount = static_cast<int>(m_controlPointList.size());

			iff.insertChunkData(static_cast<uint32>(controlPointCount));

			// Data for each control point

			for (ControlPointList::const_iterator current = m_controlPointList.begin(); current != m_controlPointList.end(); ++current)
			{
				float const percent = current->getPercent();
				float const red = current->getRed();
				float const green = current->getGreen();
				float const blue = current->getBlue();

				iff.insertChunkData(percent);
				iff.insertChunkData(red);
				iff.insertChunkData(green);
				iff.insertChunkData(blue);
			}
		}
		iff.exitChunk(TAG_0001);
	}
	iff.exitForm(TAG_CLRR);
}

#ifdef _DEBUG
//-----------------------------------------------------------------------------
std::string const &ColorRamp::getName() const
{
	return m_name;
}

//-----------------------------------------------------------------------------
void ColorRamp::setName(std::string const &name)
{
	m_name = name;
}
#endif // _DEBUG

//-----------------------------------------------------------------------------
ColorRamp::SampleType ColorRamp::getSampleType() const
{
	return m_sampleType;
}

//-----------------------------------------------------------------------------
ColorRamp::InterpolationType ColorRamp::getInterpolationType() const
{
	return m_interpolationType;
}

//-----------------------------------------------------------------------------
ColorRamp::ControlPointList::iterator ColorRamp::getIteratorBegin()
{
	return m_controlPointList.begin();
}

//-----------------------------------------------------------------------------
ColorRamp::ControlPointList::iterator ColorRamp::getIteratorEnd()
{
	return m_controlPointList.end();
}

//-----------------------------------------------------------------------------
ColorRamp::ControlPointList::const_iterator ColorRamp::getIteratorBegin() const
{
	return m_controlPointList.begin();
}

//-----------------------------------------------------------------------------
ColorRamp::ControlPointList::const_iterator ColorRamp::getIteratorEnd() const
{
	return m_controlPointList.end();
}

//-----------------------------------------------------------------------------
int ColorRamp::getControlPointCount() const
{
	return static_cast<int>(m_controlPointList.size());
}

//-----------------------------------------------------------------------------
void ColorRamp::setInterpolationType(InterpolationType const interpolationType)
{
	m_interpolationType = interpolationType;
}

//-----------------------------------------------------------------------------
void ColorRamp::setSampleType(SampleType const sampleType)
{
	m_sampleType = sampleType;
}

#ifdef _DEBUG
//-----------------------------------------------------------------------------
bool ColorRamp::verifyIteratorValid(ControlPointList::const_iterator const &iter) const
{
	bool result = false;
	ControlPointList::const_iterator current = m_controlPointList.begin();

	for (; current != m_controlPointList.end(); ++current)
	{
		if (current == iter)
		{
			result = true;
			break;
		}
	}

	return result;
}
#endif // _DEBUG

// ============================================================================
