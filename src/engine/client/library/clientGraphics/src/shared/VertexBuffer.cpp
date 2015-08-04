// ====================================================================
//
// VertexBuffer.cpp
//
// copyright 1998 Bootprint Entertainment
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#include "clientGraphics/FirstClientGraphics.h"
#include "clientGraphics/VertexBuffer.h"

#include "clientGraphics/Graphics.h"
#include "clientGraphics/GraphicsOptionTags.h"
#include "clientGraphics/VertexBufferIterator.h"
#include "sharedFile/Iff.h"

#include <algorithm>
#include <map>

// ======================================================================

namespace VertexBufferNamespace
{
	const Tag TAG_DOT3 = TAG (D,O,T,3);
	const Tag TAG_VTXA = TAG (V,T,X,A);
}
using namespace VertexBufferNamespace;

// ======================================================================

VertexBuffer::VertexBuffer(const VertexBufferFormat &format)
:
	m_format(format),
	m_descriptor(NULL)
#ifdef _DEBUG
	,m_debugIteratorLockCount(0)
#endif
{
#ifdef _DEBUG
	// @todo make this a compile-time assert
	DEBUG_FATAL(VertexBufferDescriptor::MAX_TEXTURE_COORDINATE_SETS != VertexBufferFormat::MAX_TEXTURE_COORDINATE_SETS, ("Different number of max tex coord sets"));
#endif
}

// ----------------------------------------------------------------------

VertexBuffer::VertexBuffer()
:
	m_format(),
	m_descriptor(NULL)
#ifdef _DEBUG
	,m_debugIteratorLockCount(0)
#endif
{
#ifdef _DEBUG
	// @todo make this a compile-time assert
	DEBUG_FATAL(VertexBufferDescriptor::MAX_TEXTURE_COORDINATE_SETS != VertexBufferFormat::MAX_TEXTURE_COORDINATE_SETS, ("Different number of max tex coord sets"));
#endif
}

// ----------------------------------------------------------------------

VertexBuffer::~VertexBuffer()
{
	m_descriptor = NULL;
}

// ----------------------------------------------------------------------

void VertexBuffer::load(Iff &iff)
{
	iff.enterForm(TAG_VTXA);

		switch (iff.getCurrentName())
		{
			case TAG_0001:
				load_0001(iff);
				break;

			case TAG_0002:
				load_0002(iff);
				break;

			case TAG_0003:
				load_0003(iff);
				break;

			default:
				{
					char buffer[256];
					iff.formatLocation(buffer, sizeof(buffer));
					DEBUG_FATAL(true, ("Bad version number in %s", buffer));
				}
		}

	iff.exitForm(TAG_VTXA);
}

// ----------------------------------------------------------------------

void VertexBuffer::load_0001(Iff &iff)
{
	iff.enterForm(TAG_0001);

		iff.enterChunk(TAG_INFO);
			const int numberOfVertices = iff.read_int32();
			DEBUG_FATAL(numberOfVertices == 0, ("cannot create a 0 length vertexbuffer"));
			const int numberOfUVSets  = iff.read_int32();
			const uint flags = iff.read_uint32();
		iff.exitChunk (TAG_INFO);

		// convert to the new flags
		const int VBFF_transformed = BINARY1(0001);
		if (flags & VBFF_transformed)
			m_format.setTransformed();

		const int VBFF_normal      = BINARY1(0010);
		if (flags & VBFF_normal)
			m_format.setNormal();

		const int VBFF_diffuse     = BINARY1(0100);
		if (flags & VBFF_diffuse)
			m_format.setColor0();

		// all texture coordinates were 2d
		m_format.setNumberOfTextureCoordinateSets(numberOfUVSets);
		for (int i = 0; i < numberOfUVSets; ++i)
			m_format.setTextureCoordinateSetDimension(i, 2);

		VertexBufferWriteIterator v = preLoad(numberOfVertices);

		iff.enterChunk (TAG_DATA);

			for ( ; !iff.atEndOfForm(); ++v)
			{
				v.setPosition(iff.read_floatVector());

				if (isTransformed())
					v.setOoz(iff.read_float());

				if (hasNormal())
					v.setNormal(iff.read_floatVector());

				if (hasColor0())
				{
					VectorArgb argb;
					argb.a = iff.read_float();
					argb.r = iff.read_float();
					argb.g = iff.read_float();
					argb.b = iff.read_float();
					v.setColor0(argb);
				}

				const int numberOfTextureCoordinateSets = getNumberOfTextureCoordinateSets();
				for (int j = 0; j < numberOfTextureCoordinateSets ; ++j)
				{
					const real tc0 = iff.read_float();
					const real tc1 = iff.read_float();
					v.setTextureCoordinates(j, tc0, tc1);
				}
			}

		iff.exitChunk(TAG_DATA);
	iff.exitForm(TAG_0001);

	postLoad();
}

// ----------------------------------------------------------------------

void VertexBuffer::load_0002(Iff &iff)
{
	// the exit for this form is at the end of loadVertexData_0002
	iff.enterForm(TAG_0002);

		// get basic vertex array info
		iff.enterChunk(TAG_INFO);

			// get the vertex buffer format and see if we need to ditch the DOT3 texture coordinate set
			m_format.setFlags(static_cast<VertexBufferFormat::Flags>(iff.read_uint32()));
			int numberOfTextureCoordinateSets = m_format.getNumberOfTextureCoordinateSets();
			bool skipDot3 = false;
			if (numberOfTextureCoordinateSets > 0 && m_format.getTextureCoordinateSetDimension(numberOfTextureCoordinateSets-1) == 4 && GraphicsOptionTags::get(TAG_DOT3) == false)
			{
				--numberOfTextureCoordinateSets;
				m_format.setTextureCoordinateSetDimension(numberOfTextureCoordinateSets, 1);
				m_format.setNumberOfTextureCoordinateSets(numberOfTextureCoordinateSets);
				skipDot3 = true;
			}

			const int numberOfVertices = iff.read_int32();
			DEBUG_FATAL(numberOfVertices == 0, ("cannot create a 0 length vertexbuffer"));
		iff.exitChunk (TAG_INFO);

		VertexBufferWriteIterator v = preLoad(numberOfVertices);

		iff.enterChunk (TAG_DATA);

			for ( ; !iff.atEndOfForm(); ++v)
			{
				if (hasPosition())
					v.setPosition(iff.read_floatVector());

				if (isTransformed())
					v.setOoz(iff.read_float());

				if (hasNormal())
					v.setNormal(iff.read_floatVector());

				if (hasColor0())
					v.setColor0(iff.read_uint32());

				if (hasColor1())
					v.setColor1(iff.read_uint32());

				for (int j = 0; j < numberOfTextureCoordinateSets ; ++j)
				{
					const int dimension = getTextureCoordinateSetDimension(j);
					for (int k = 0; k < dimension; ++k)
						v.setTextureCoordinate(j, k, iff.read_float());
				}

				if (skipDot3)
				{
					IGNORE_RETURN(iff.read_float());
					IGNORE_RETURN(iff.read_float());
					IGNORE_RETURN(iff.read_float());
					IGNORE_RETURN(iff.read_float());
				}
			}

		postLoad();

		iff.exitChunk (TAG_DATA);
	iff.exitForm(TAG_0002);
}

// ----------------------------------------------------------------------

void VertexBuffer::load_0003(Iff &iff)
{
	// the exit for this form is at the end of loadVertexData_0003
	iff.enterForm(TAG_0003);

		// get basic vertex array info
		iff.enterChunk(TAG_INFO);

			// get the vertex buffer format and see if we need to ditch the DOT3 texture coordinate set
			m_format.setFlags(static_cast<VertexBufferFormat::Flags>(iff.read_uint32()));
			int numberOfTextureCoordinateSets = m_format.getNumberOfTextureCoordinateSets();
			bool skipDot3 = false;
			if (numberOfTextureCoordinateSets > 0 && m_format.getTextureCoordinateSetDimension(numberOfTextureCoordinateSets-1) == 4 && GraphicsOptionTags::get(TAG_DOT3) == false)
			{
				--numberOfTextureCoordinateSets;
				m_format.setTextureCoordinateSetDimension(numberOfTextureCoordinateSets, 1);
				m_format.setNumberOfTextureCoordinateSets(numberOfTextureCoordinateSets);
				skipDot3 = true;
			}

			int numberOfVertices = iff.read_int32();
			DEBUG_FATAL(numberOfVertices == 0, ("cannot create a 0 length vertexbuffer"));
		iff.exitChunk (TAG_INFO);

		VertexBufferWriteIterator v = preLoad(numberOfVertices);

		iff.enterChunk (TAG_DATA);

			for ( ; !iff.atEndOfForm(); ++v)
			{
				if (hasPosition())
					v.setPosition(iff.read_floatVector());

				if (isTransformed())
					v.setOoz(iff.read_float());

				if (hasNormal())
					v.setNormal(iff.read_floatVector());

				if (hasPointSize())
					v.setPointSize(iff.read_float());

				if (hasColor0())
					v.setColor0(iff.read_uint32());

				if (hasColor1())
					v.setColor1(iff.read_uint32());

				for (int j = 0; j < numberOfTextureCoordinateSets ; ++j)
				{
					const int dimension = getTextureCoordinateSetDimension(j);
					for (int k = 0; k < dimension; ++k)
						v.setTextureCoordinate(j, k, iff.read_float());
				}

				if (skipDot3)
				{
					IGNORE_RETURN(iff.read_float());
					IGNORE_RETURN(iff.read_float());
					IGNORE_RETURN(iff.read_float());
					IGNORE_RETURN(iff.read_float());
				}
			}

			postLoad();

		iff.exitChunk (TAG_DATA);
	iff.exitForm(TAG_0003);
}

// ----------------------------------------------------------------------
/**
 * Write out all the vertices in this VertexBuffer.
 * 
 * This will write all data to the form named VRTA, inserted
 * at the current iff position.
 * 
 * @param iff  Iff to write the vertices into
 */

void VertexBuffer::write(Iff &iff, VertexBufferReadIterator v, int numberOfVertices) const
{
	iff.insertForm (TAG_VTXA);
		iff.insertForm (TAG_0003);

			iff.insertChunk (TAG_INFO);
				iff.insertChunkData(static_cast<uint32>(m_format.getFlags()));
				iff.insertChunkData(static_cast<int32>(numberOfVertices));
			iff.exitChunk (TAG_INFO);

			iff.insertChunk (TAG_DATA);

				for ( ; numberOfVertices; --numberOfVertices, ++v)
				{
					if (hasPosition())
						iff.insertChunkFloatVector(v.getPosition());

					if (isTransformed())
						iff.insertChunkData(static_cast<float>(v.getOoz()));

					if (hasNormal())
						iff.insertChunkFloatVector(v.getNormal());

					if (hasPointSize())
						iff.insertChunkData(v.getPointSize());

					if (hasColor0())
						iff.insertChunkData(v.getColor0());

					if (hasColor1())
						iff.insertChunkData(v.getColor1());

					const int numberOfTextureCoordinateSets = getNumberOfTextureCoordinateSets();
					for (int j = 0; j < numberOfTextureCoordinateSets; ++j)
					{
						const int dimension = getTextureCoordinateSetDimension(j);
						for (int k = 0; k < dimension; ++k)
							iff.insertChunkData(static_cast<float>(v.getTextureCoordinate(j, k)));
					}
				}

			iff.exitChunk (TAG_DATA);

		iff.exitForm (TAG_0003);
	iff.exitForm (TAG_VTXA);
}

// ======================================================================
