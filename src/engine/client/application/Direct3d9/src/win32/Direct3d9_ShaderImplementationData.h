// ======================================================================
//
// Direct3d9_ShaderImplementationData.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_Direct3d9_ShaderImplementationData_H
#define INCLUDED_Direct3d9_ShaderImplementationData_H

// ======================================================================

#include "clientGraphics/ShaderImplementation.h"

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Production.h"

#include <d3d9.h>
#include <vector>
#include <map>

// ======================================================================

class Direct3d9_ShaderImplementationData : public ShaderImplementationGraphicsData
{
public:

	static void install();
	static void lostDevice();

public:

	Direct3d9_ShaderImplementationData(const ShaderImplementation &implementation);
	virtual ~Direct3d9_ShaderImplementationData();

	void     apply(int passNumber) const;

public:

	template <class T> struct State
	{
		T       state;
		DWORD   value;
		State(T s, DWORD v) : state(s), value(v) {}
	};

	typedef State<D3DRENDERSTATETYPE>        RenderState;
	typedef std::vector<RenderState>         RenderStates;

	typedef State<D3DTEXTURESTAGESTATETYPE > TextureStageState;
	typedef std::vector<TextureStageState>   TextureStageStates;

	class Stage
	{
	public:
#ifdef FFP
		void construct(const ShaderImplementation::Pass::Stage &stage);
#endif
#ifdef VSPS
		void construct(const ShaderImplementation::Pass::PixelShader::TextureSampler &textureSampler);
#endif
		void apply(int stageNumber) const;
	public:
		TextureStageStates m_textureStageState;
	};
	typedef std::vector<Stage> Stages;

	class Pass
	{
	public:

		Pass();
		~Pass();

		void construct(const ShaderImplementation::Pass &pass);
		void apply() const;

	public:

		bool                   m_alphaBlendEnable;
		uint8                  m_colorWriteEnable;
		RenderStates           m_renderState;
#ifdef VSPS
#if PRODUCTION == 0
		const ShaderImplementationPassPixelShaderProgram *m_pixelShaderProgram;
#else
		IDirect3DPixelShader9 *m_pixelShader;
#endif
#endif
#ifdef FFP
		Stages                 m_stage;
#endif
	};

	typedef std::vector<Pass> Passes;

private:

	typedef std::vector<const bool *> OptionFlags;

private:

	Passes       m_pass;
};

// ======================================================================

#endif
