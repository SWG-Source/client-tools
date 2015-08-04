// ======================================================================
//
// Direct3d9_StaticShaderData.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_Direct3d9_StaticShaderData_H
#define INCLUDED_Direct3d9_StaticShaderData_H

// ======================================================================

class Material;
class Direct3d9_TextureData;

#include "Direct3d9_VertexShaderUtilities.h"
#include "clientGraphics/ShaderImplementation.h"
#include "clientGraphics/StaticShader.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Production.h"
#include "sharedMath/VectorRgba.h"

#include <vector>
#include <d3d9.h>

// ======================================================================

class Direct3d9_StaticShaderData : public StaticShaderGraphicsData
{
public:

	static void install();
	static void beginFrame();

	explicit Direct3d9_StaticShaderData(const StaticShader &shader);
	virtual ~Direct3d9_StaticShaderData();

	virtual void                        update(const StaticShader &shader);
	virtual int                         getTextureSortKey() const;

	bool                                isValid() const;
	bool                                apply(int pass) const;

private:

	class Stage
	{
	private:

		struct SamplerState
		{
			D3DSAMPLERSTATETYPE            state;
			DWORD                          value;
			SamplerState(D3DSAMPLERSTATETYPE s, DWORD v) : state(s), value(v) {}
		};

		typedef std::vector<SamplerState>   SamplerStates;

	private:

		bool                                 m_placeholder;
#ifdef FFP
		bool                                 m_textureCoordinateSetValid;
		bool                                 m_textureScrollValid;
#endif
		Direct3d9_TextureData const * const *m_texture;
#ifdef FFP
		DWORD                                m_textureCoordinateSet;
		float                                m_textureScroll[2];
#endif
		SamplerStates                        m_samplerStates;

	public:
		Stage();
#ifdef FFP
		void construct(const StaticShader &shader, const ShaderImplementation::Pass &pass, const ShaderImplementation::Pass::Stage &stage);
#endif
#ifdef VSPS
		void construct(const StaticShader &shader, const ShaderImplementation::Pass::PixelShader::TextureSampler &textureSampler);
#endif
		bool getTextureSortKey(int &value) const;
		void apply(int stage) const;
	};

	typedef std::vector<Stage> Stages;

	class Pass
	{
	public:

		// this struct exists so that we can use a pointer to it to set vertex shader constants
		struct PaddedMaterial
		{
			D3DMATERIAL9                      material;
			float                             pad1;
			float                             pad2;
			float                             pad3;
		};

	private:
		bool                                            m_materialValid;
		bool                                            m_textureFactorValid;
		bool                                            m_alphaTestReferenceValueValid;
		bool                                            m_stencilReferenceValueValid;
		bool                                            m_fullAmbient;
		bool                                            m_textureScrollValid;
#ifdef VSPS
#if PRODUCTION == 0
		const ShaderImplementationPassVertexShader *    m_vertexShader;
		uint32                                          m_textureCoordinateSetKey;
#else
		IDirect3DVertexShader9 *                        m_vertexShader;
#endif
#endif
		PaddedMaterial                                  m_material;
#ifdef FFP
		uint32                                          m_textureFactor;
#endif
#ifdef VSPS		
		VectorRgba                                      m_textureFactorData[2];
#endif
		float                                           m_textureScroll[4];
		uint8                                           m_alphaTestReferenceValue;
		uint32                                          m_stencilReferenceValue;
		ShaderImplementation::Pass::FogMode             m_fogMode;
		Stages                                          m_stage;

	public:

		static void install();

		void construct(const StaticShader &shader, const ShaderImplementation::Pass &pass);
		bool getTextureSortKey(int &value) const;
		bool apply() const;

	private:

#ifdef _DEBUG
		static bool                 ms_useDefaultMaterial;
		static PaddedMaterial       ms_defaultMaterial;
		static PaddedMaterial       ms_debugMaterial;
#ifdef VSPS
		static VectorRgba           ms_debugTextureFactorData[2];
#endif
#endif
	};

	typedef std::vector<Pass> Passes;

private:

	/// Disabled.
	Direct3d9_StaticShaderData();

	/// Disabled.
	Direct3d9_StaticShaderData(const Direct3d9_StaticShaderData &);

	/// Disabled.
	Direct3d9_StaticShaderData &operator =(const Direct3d9_StaticShaderData &);

private:

	void construct(const StaticShader &shader);
	bool validate();

private:

	const ShaderImplementation *m_implementation;
	Passes                      m_pass;
};

// ======================================================================

inline bool Direct3d9_StaticShaderData::isValid() const
{
	return m_implementation != NULL;
}

// ======================================================================


#endif
