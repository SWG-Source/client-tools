// ======================================================================
//
// MayaPerPixelLighting.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_MayaPerPixelLighting_H
#define INCLUDED_MayaPerPixelLighting_H

// ======================================================================

class Messenger;
class MFnMesh;

// ======================================================================

/**
 * Support class containing functions to collect and manage
 * data associated with per-pixel lighting.
 *
 * This class is used by the character system export path.  The static
 * mesh exporter embeds similar (though not identical) functionality into
 * its data collection and writing path.
 */

class MayaPerPixelLighting
{
public:

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	/**
	 * Represents a shader's request for dot 3 lighting.
	 */

	class ShaderRequestInfo
	{
	public:

		ShaderRequestInfo(bool requestDot3, int calculationTextureCoordinateSet);

		bool  isDot3Requested() const;
		int   getCalculationTextureCoordinateSet() const;

	private:

		// Disabled.
		ShaderRequestInfo();

	private:
	
		bool  m_requestDot3;
		int   m_calculationTextureCoordinateSet;

	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	/**
	 * Represents a key used for associating a specific vertex with a piece
	 * of per-pixel lighting information.
	 */

	class Dot3Key
	{
	public:

		Dot3Key(int positionIndex, int normalIndex, int shaderIndex);

		bool operator <(const Dot3Key &rhs) const;

	private:

		// Disabled.
		Dot3Key();
		Dot3Key &operator =(const Dot3Key&);

	private:

		int  m_positionIndex;
		int  m_normalIndex;
		int  m_shaderIndex;

	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class Dot3Difference
	{
	public:

		Dot3Difference(int index, float dx, float dy, float dz, float dw);

		int    getIndex()  const;
		float  getDeltaX() const;
		float  getDeltaY() const;
		float  getDeltaZ() const;
		float  getDeltaW() const;

	private:

		// Disabled.
		Dot3Difference();

	private:

		int    m_index;
		float  m_dx;
		float  m_dy;
		float  m_dz;
		float  m_dw;

	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	typedef stdvector<Dot3Difference>::fwd     Dot3DifferenceVector;
	typedef stdmap<const Dot3Key, int>::fwd    Dot3KeyMap;
	typedef stdvector<float>::fwd              FloatVector;
	typedef stdvector<ShaderRequestInfo>::fwd  ShaderRequestInfoVector;

public:

	static void install(Messenger *newMessenger);
	static void remove();

	static bool getShaderRequestInfo(const MFnMesh &fnMesh, ShaderRequestInfoVector &shaderRequestInfoVector);
	static bool generateDot3KeyMap(const MFnMesh &fnMesh, const ShaderRequestInfoVector &shaderRequestInfoVector, Dot3KeyMap &dot3KeyMap, int &dot3ValueCount);
	static bool computeDot3Values(const MFnMesh &fnMesh, const ShaderRequestInfoVector &shaderRequestInfoVector, const Dot3KeyMap &dot3KeyMap, FloatVector &dot3ComponentVector, int dot3ValueCount);
	static bool computeDot3DifferenceVector(const FloatVector &baseDot3Vector, const FloatVector &targetDot3Vector, Dot3DifferenceVector &differenceVector);

private:

	class Dot3Builder;
	class Mapper;

};

// ======================================================================
// class MayaPerPixelLighting::ShaderRequestInfo
// ======================================================================

inline MayaPerPixelLighting::ShaderRequestInfo::ShaderRequestInfo(bool requestDot3, int calculationTextureCoordinateSet) :
	m_requestDot3(requestDot3),
	m_calculationTextureCoordinateSet(calculationTextureCoordinateSet)
{
}

// ----------------------------------------------------------------------

inline bool MayaPerPixelLighting::ShaderRequestInfo::isDot3Requested() const
{
	return m_requestDot3;
}

// ----------------------------------------------------------------------

inline int MayaPerPixelLighting::ShaderRequestInfo::getCalculationTextureCoordinateSet() const
{
	return m_calculationTextureCoordinateSet;
}
	
// ======================================================================
// class MayaPerPixelLighting::Dot3Key
// ======================================================================

inline MayaPerPixelLighting::Dot3Key::Dot3Key(int positionIndex, int normalIndex, int shaderIndex) :
	m_positionIndex(positionIndex),
	m_normalIndex(normalIndex),
	m_shaderIndex(shaderIndex)
{
}

// ======================================================================
// class MayaPerPixelLighting::Dot3Difference
// ======================================================================

inline MayaPerPixelLighting::Dot3Difference::Dot3Difference(int index, float dx, float dy, float dz, float dw) :
	m_index(index),
	m_dx(dx),
	m_dy(dy),
	m_dz(dz),
	m_dw(dw)
{
}

// ----------------------------------------------------------------------

inline int MayaPerPixelLighting::Dot3Difference::getIndex() const
{
	return m_index;
}

// ----------------------------------------------------------------------

inline float MayaPerPixelLighting::Dot3Difference::getDeltaX() const
{
	return m_dx;
}

// ----------------------------------------------------------------------

inline float MayaPerPixelLighting::Dot3Difference::getDeltaY() const
{
	return m_dy;
}

// ----------------------------------------------------------------------

inline float MayaPerPixelLighting::Dot3Difference::getDeltaZ() const
{
	return m_dz;
}

// ----------------------------------------------------------------------

inline float MayaPerPixelLighting::Dot3Difference::getDeltaW() const
{
	return m_dw;
}

// ======================================================================

#endif
