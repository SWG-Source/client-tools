// ======================================================================
//
// VertexBufferFormat.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_VertexBufferFormat_H
#define INCLUDED_VertexBufferFormat_H

// ======================================================================

class VertexBufferFormat
{
	// Compiler generated copy constructor, assignment operator, and destructor are correct

public:

	enum
	{
		MAX_TEXTURE_COORDINATE_SETS = 8
	};

public:

	VertexBufferFormat();

	uint32 getFlags() const;
	void   setFlags(uint32 flags);

	bool hasPosition() const;
	bool isTransformed() const;
	bool hasNormal() const;
	bool hasPointSize() const;
	bool hasColor0() const;
	bool hasColor1() const;
	int  getNumberOfTextureCoordinateSets() const;
	int  getTextureCoordinateSetDimension(int textureCoordinateSet) const;

	void setPosition(bool enabled=true);
	void setTransformed(bool enabled=true);
	void setNormal(bool enabled=true);
	void setPointSize(bool enabled=true);
	void setColor0(bool enabled=true);
	void setColor1(bool enabled=true);
	void setNumberOfTextureCoordinateSets(int numberOfTextureCoordinateSets);
	void setTextureCoordinateSetDimension(int textureCoordinateSet, int dimension);

	bool operator ==(const VertexBufferFormat &rhs) const;
	bool operator !=(const VertexBufferFormat &rhs) const;

	void formatFormat(std::string &result) const;

private:

	enum
	{
		TextureCoordinateSetCountShift           = 8,
		TextureCoordinateSetCountMask            = BINARY1(1111),

		TextureCoordinateSetDimensionBaseShift   = 12,
		TextureCoordinateSetDimensionPerSetShift = 2,
		TextureCoordinateSetDimensionAdjustment  = 1,
		TextureCoordinateSetDimensionMask        = BINARY1(0011),

		BlendCountShift                          = 24,
		BlendMask                                = BINARY1(0111)
	};
public:
	enum Flags
	{
		F_none                     = BINARY8(0000,0000,0000,0000,0000,0000,0000,0000),
		F_position                 = BINARY8(0000,0000,0000,0000,0000,0000,0000,0001),
		F_transformed              = BINARY8(0000,0000,0000,0000,0000,0000,0000,0010),
		F_normal                   = BINARY8(0000,0000,0000,0000,0000,0000,0000,0100),
		F_color0                   = BINARY8(0000,0000,0000,0000,0000,0000,0000,1000),
		F_color1                   = BINARY8(0000,0000,0000,0000,0000,0000,0001,0000),

		F_pointSize                = BINARY8(0000,0000,0000,0000,0000,0000,0010,0000),

		F_textureCoordinateCount0  = BINARY8(0000,0000,0000,0000,0000,0000,0000,0000),
		F_textureCoordinateCount1  = BINARY8(0000,0000,0000,0000,0000,0001,0000,0000),
		F_textureCoordinateCount2  = BINARY8(0000,0000,0000,0000,0000,0010,0000,0000),
		F_textureCoordinateCount3  = BINARY8(0000,0000,0000,0000,0000,0011,0000,0000),
		F_textureCoordinateCount4  = BINARY8(0000,0000,0000,0000,0000,0100,0000,0000),
		F_textureCoordinateCount5  = BINARY8(0000,0000,0000,0000,0000,0101,0000,0000),
		F_textureCoordinateCount6  = BINARY8(0000,0000,0000,0000,0000,0110,0000,0000),
		F_textureCoordinateCount7  = BINARY8(0000,0000,0000,0000,0000,0111,0000,0000),
		F_textureCoordinateCount8  = BINARY8(0000,0000,0000,0000,0000,1000,0000,0000),

		F_textureCoordinateSet0_1d = BINARY8(0000,0000,0000,0000,0000,0000,0000,0000),
		F_textureCoordinateSet0_2d = BINARY8(0000,0000,0000,0000,0001,0000,0000,0000),
		F_textureCoordinateSet0_3d = BINARY8(0000,0000,0000,0000,0010,0000,0000,0000),
		F_textureCoordinateSet0_4d = BINARY8(0000,0000,0000,0000,0011,0000,0000,0000),
		F_textureCoordinateSet1_1d = BINARY8(0000,0000,0000,0000,0000,0000,0000,0000),
		F_textureCoordinateSet1_2d = BINARY8(0000,0000,0000,0000,0100,0000,0000,0000),
		F_textureCoordinateSet1_3d = BINARY8(0000,0000,0000,0000,1000,0000,0000,0000),
		F_textureCoordinateSet1_4d = BINARY8(0000,0000,0000,0000,1100,0000,0000,0000),

		F_textureCoordinateSet2_1d = BINARY8(0000,0000,0000,0000,0000,0000,0000,0000),
		F_textureCoordinateSet2_2d = BINARY8(0000,0000,0000,0001,0000,0000,0000,0000),
		F_textureCoordinateSet2_3d = BINARY8(0000,0000,0000,0010,0000,0000,0000,0000),
		F_textureCoordinateSet2_4d = BINARY8(0000,0000,0000,0011,0000,0000,0000,0000),
		F_textureCoordinateSet3_1d = BINARY8(0000,0000,0000,0000,0000,0000,0000,0000),
		F_textureCoordinateSet3_2d = BINARY8(0000,0000,0000,0100,0000,0000,0000,0000),
		F_textureCoordinateSet3_3d = BINARY8(0000,0000,0000,1000,0000,0000,0000,0000),
		F_textureCoordinateSet3_4d = BINARY8(0000,0000,0000,1100,0000,0000,0000,0000),

		F_textureCoordinateSet4_1d = BINARY8(0000,0000,0000,0000,0000,0000,0000,0000),
		F_textureCoordinateSet4_2d = BINARY8(0000,0000,0001,0000,0000,0000,0000,0000),
		F_textureCoordinateSet4_3d = BINARY8(0000,0000,0010,0000,0000,0000,0000,0000),
		F_textureCoordinateSet4_4d = BINARY8(0000,0000,0011,0000,0000,0000,0000,0000),
		F_textureCoordinateSet5_1d = BINARY8(0000,0000,0000,0000,0000,0000,0000,0000),
		F_textureCoordinateSet5_2d = BINARY8(0000,0000,0100,0000,0000,0000,0000,0000),
		F_textureCoordinateSet5_3d = BINARY8(0000,0000,1000,0000,0000,0000,0000,0000),
		F_textureCoordinateSet5_4d = BINARY8(0000,0000,1100,0000,0000,0000,0000,0000),

		F_textureCoordinateSet6_1d = BINARY8(0000,0000,0000,0000,0000,0000,0000,0000),
		F_textureCoordinateSet6_2d = BINARY8(0000,0001,0000,0000,0000,0000,0000,0000),
		F_textureCoordinateSet6_3d = BINARY8(0000,0010,0000,0000,0000,0000,0000,0000),
		F_textureCoordinateSet6_4d = BINARY8(0000,0011,0000,0000,0000,0000,0000,0000),
		F_textureCoordinateSet7_1d = BINARY8(0000,0000,0000,0000,0000,0000,0000,0000),
		F_textureCoordinateSet7_2d = BINARY8(0000,0100,0000,0000,0000,0000,0000,0000),
		F_textureCoordinateSet7_3d = BINARY8(0000,1000,0000,0000,0000,0000,0000,0000),
		F_textureCoordinateSet7_4d = BINARY8(0000,1100,0000,0000,0000,0000,0000,0000)

#if 0
		,
		F_blend0                   = BINARY8(0000,0000,0000,0000,0000,0000,0000,0000),
		F_blend1                   = BINARY8(0001,0000,0000,0000,0000,0000,0000,0000),
		F_blend2                   = BINARY8(0010,0000,0000,0000,0000,0000,0000,0000),
		F_blend3                   = BINARY8(0011,0000,0000,0000,0000,0000,0000,0000),
		F_blend4                   = BINARY8(0100,0000,0000,0000,0000,0000,0000,0000),
		F_blend5                   = BINARY8(0101,0000,0000,0000,0000,0000,0000,0000)
#endif
	};

private:

	uint32  m_flags;
};

// ======================================================================

inline VertexBufferFormat::VertexBufferFormat()
: m_flags(0)
{
}

// ----------------------------------------------------------------------
/**
 * @internal
 */

inline void VertexBufferFormat::setFlags(uint32 flags)
{
	m_flags = flags;
}

// ----------------------------------------------------------------------
/**
 * @internal
 */
inline uint32 VertexBufferFormat::getFlags() const
{
	return m_flags;
}

// ----------------------------------------------------------------------

inline bool VertexBufferFormat::hasPosition() const
{
	return (m_flags & F_position) != 0;
}

// ----------------------------------------------------------------------

inline bool VertexBufferFormat::isTransformed() const
{
	return (m_flags & F_transformed) != 0;
}

// ----------------------------------------------------------------------

inline bool VertexBufferFormat::hasNormal() const
{
	return (m_flags & F_normal) != 0;
}

// ----------------------------------------------------------------------

inline bool VertexBufferFormat::hasPointSize() const
{
	return (m_flags & F_pointSize) != 0;
}

// ----------------------------------------------------------------------

inline bool VertexBufferFormat::hasColor0() const
{
	return (m_flags & F_color0) != 0;
}

// ----------------------------------------------------------------------

inline bool VertexBufferFormat::hasColor1() const
{
	return (m_flags & F_color1) != 0;
}

// ----------------------------------------------------------------------

inline int VertexBufferFormat::getNumberOfTextureCoordinateSets() const
{
	return (m_flags >> TextureCoordinateSetCountShift) & (TextureCoordinateSetCountMask);
}

// ----------------------------------------------------------------------

inline int VertexBufferFormat::getTextureCoordinateSetDimension(int textureCoordinateSet) const
{
	const int shift = TextureCoordinateSetDimensionBaseShift + (textureCoordinateSet * TextureCoordinateSetDimensionPerSetShift);
	return ((m_flags >> shift) & TextureCoordinateSetDimensionMask) + TextureCoordinateSetDimensionAdjustment;
}

// ----------------------------------------------------------------------

inline void VertexBufferFormat::setPosition(bool enabled)
{
	if (enabled)
		m_flags |= F_position;
	else
		m_flags &= ~F_position;
}

// ----------------------------------------------------------------------

inline void VertexBufferFormat::setTransformed(bool enabled)
{
	if (enabled)
		m_flags |= F_transformed;
	else
		m_flags &= ~F_transformed;
}

// ----------------------------------------------------------------------

inline void VertexBufferFormat::setNormal(bool enabled)
{
	if (enabled)
		m_flags |= F_normal;
	else
		m_flags &= ~F_normal;
}

// ----------------------------------------------------------------------

inline void VertexBufferFormat::setPointSize(bool enabled)
{
	if(enabled)
		m_flags |= F_pointSize;
	else
		m_flags &= ~F_pointSize;
}

// ----------------------------------------------------------------------

inline void VertexBufferFormat::setColor0(bool enabled)
{
	if (enabled)
		m_flags |= F_color0;
	else
		m_flags &= ~F_color0;
}

// ----------------------------------------------------------------------

inline void VertexBufferFormat::setColor1(bool enabled)
{
	if (enabled)
		m_flags |= F_color1;
	else
		m_flags &= ~F_color1;
}

// ----------------------------------------------------------------------

inline void VertexBufferFormat::setNumberOfTextureCoordinateSets(int numberOfTextureCoordinateSets)
{
	DEBUG_FATAL(numberOfTextureCoordinateSets < 0 || numberOfTextureCoordinateSets > MAX_TEXTURE_COORDINATE_SETS, ("Invalid texture coordinate set"));
	m_flags = (m_flags & ~(TextureCoordinateSetCountMask << TextureCoordinateSetCountShift)) | (static_cast<uint32>(numberOfTextureCoordinateSets) << TextureCoordinateSetCountShift);
}

// ----------------------------------------------------------------------

inline void VertexBufferFormat::setTextureCoordinateSetDimension(int textureCoordinateSet, int dimension)
{
	DEBUG_FATAL(textureCoordinateSet < 0 || textureCoordinateSet >= MAX_TEXTURE_COORDINATE_SETS, ("Invalid texture coordinate set"));
	DEBUG_FATAL(dimension < 1 || dimension > 4, ("Invalid texture coordinate set dimension"));
	const uint shift = static_cast<uint>(TextureCoordinateSetDimensionBaseShift + (textureCoordinateSet * TextureCoordinateSetDimensionPerSetShift));
	m_flags = (m_flags & ~(static_cast<uint>(TextureCoordinateSetDimensionMask) << shift)) | (static_cast<uint32>(dimension - TextureCoordinateSetDimensionAdjustment) << shift);
}

// ----------------------------------------------------------------------

inline bool VertexBufferFormat::operator ==(const VertexBufferFormat &rhs) const
{
	return m_flags == rhs.m_flags;
}

// ----------------------------------------------------------------------

inline bool VertexBufferFormat::operator !=(const VertexBufferFormat &rhs) const
{
	return !(*this == rhs);
}

// ======================================================================

#endif
