// ======================================================================
//
// Direct3d11_LightManager.h
// copyright (c) 2026 Galaxies Reborn
//
// Light selection, and the twenty-eight vertex constants plus five pixel constants the
// corpus reads them from.
//
// ----------------------------------------------------------------------
// What this is
//
// The engine hands the backend an unbounded list of lights per object. The shaders have room
// for a fixed eight: one parallel with specular, two parallel without, one point with
// specular, four point without, plus a collapsed ambient. Choosing which eight, and packing
// them into c16..c43, is all this class does.
//
// The layout is not ours to choose. LightData below is a byte-for-byte transcription of the
// C++ struct DX9 uploaded, which is itself the layout the shipped
// vertex_shader_constants.inc declares -- so the struct, the register base and the field order
// are all asset ABI. The static_asserts at the top of the .cpp are what keeps that honest.
//
// ----------------------------------------------------------------------
// Two upload paths, not one
//
// selectLights picks the eight, then takes one of two paths:
//
//   applyLights_vertexShader      -- the full twenty-eight registers plus the four extended
//                                    ones. Taken when the SELECTION changed.
//
//   applyLights_vertexShader_dot3 -- two to four registers. Taken when the selection is
//                                    unchanged but the object moved, because the dot3 block
//                                    holds the light direction and camera position in OBJECT
//                                    space and so is the only part a move invalidates.
//
// That split is the whole reason this is worth a class. Lighting constants are the largest
// per-object upload in the frame and the selection rarely changes between adjacent objects.
//
// ----------------------------------------------------------------------
// Four packed alpha components, which look like bugs and are not
//
// The dot3 registers carry unrelated scalars in their alpha channels, because by the time
// these features were added there were no free registers left in the 2.0 profile:
//
//   dot3.diffuseColor.a               alphaFadeOpacityEnabled
//   dot3.specularColor.a              alphaFadeOpacity
//   tangentMinusDiffuseColor.a        bloomEnabled
//   tangentMinusBackColor.a           current time, for scrolling effects
//   dot3.localDirection.w             material specular power
//
// Every one of those is read by shipped shader code, so none can be cleaned up without
// changing assets. This is also why setAlphaFadeOpacity and setBloomEnabled land here rather
// than in the constant buffers: the components they set are inside registers this class owns
// and rewrites wholesale, so two writers would fight.
//
// ----------------------------------------------------------------------
// Inherited fixes, carried deliberately
//
// The x64 DX9 build this ports from carries five local lighting fixes over the SOE original,
// each with a diagnosis in its comment. The image-changing ones are no longer reproduced
// unconditionally -- the rule became: the data renders as authored, and each compensation is
// a config opt-in (see ConfigDirect3d11):
//
//   1. Light colours are clamped non-negative at upload. (Kept: correctness, not tuning.)
//   2. Scene ambient has a 0.3 floor. (REMOVED -- see applyLights_vertexShader; it flattened
//      every low-ambient interior to grey.)
//   3. A missing parallel-specular light falls back to ambient for the pixel dot3 diffuse.
//      (Kept: a fallback for an absent light, not a rewrite of a present one.)
//   4. A light with no hemispheric colours gets a synthesised hemisphere at 0.65/0.30.
//      (Opt-in via [Direct3d11] synthesizeHemisphericLight, default off -- it erased the
//      shade side of every character; measurement at setExtendedLightData.)
//   5. obeysLightScale is ignored; the unscaled accessors are always used.
//
// Number 5 is why this class has no function-pointer indirection where DX9 has six. DX9 swaps
// between scaled and unscaled Light accessors through member pointers; the x64 build assigns
// the unscaled ones on every call regardless of the flag, which makes the indirection dead
// weight. The calls here are direct, and setObeysLightScale keeps the flag only because it
// participates in the selection-changed comparison.
//
// ----------------------------------------------------------------------
// Not ported
//
// The fixed-function path -- setupLightData, applyLights_fixedFunctionPipeline and the eight
// D3DLIGHT9 slots -- has no meaning in a shader-only backend.
//
// setUsingVertexShaderProgram is gone for the same reason: DX9 only ever calls it from inside
// an #if defined(FFP) && defined(VSPS) block, to decide which of the two paths above to take.
// There is one path here.
//
// SortLightsDistance and ms_noDot3Light are not carried. Both are defined in DX9 and neither
// is referenced anywhere -- the sort functor has no call site, which is also what makes
// ms_objectPosition dead, so that member is gone too.
//
// ======================================================================

#ifndef INCLUDED_Direct3d11_LightManager_H
#define INCLUDED_Direct3d11_LightManager_H

// ======================================================================

#include "sharedMath/Transform.h"
#include "sharedMath/Vector.h"
#include "sharedMath/VectorRgba.h"

#include <vector>

class Light;
class VectorArgb;

// ======================================================================

class Direct3d11_LightManager
{
public:
	typedef stdvector<const Light *>::fwd LightList;

	static void install();
	static void beginFrame();

	static void setLights(LightList const &lightList);
	static void setObeysLightScale(bool obeysLightScale);
	static void setObjectToWorldTransform(Transform const &objectToWorldTransform);
	static void setCameraPosition(Vector const &cameraPosition);
	static void setFullAmbientOn(bool on);

	// Select and upload, if anything changed. Called from prepareToDraw.
	static void selectLights();

	// The three scalars that ride in the alpha channels of the dot3 registers. Set through
	// here rather than through the constant buffers because this class rewrites those whole
	// registers and would otherwise clobber them.
	static void setAlphaFadeOpacity(bool enabled, float opacity);
	static bool getAlphaFadeOpacityEnabled();
	static void setBloomEnabled(bool enabled);

public:
	static constexpr int ParallelSpecularCount = 1;
	static constexpr int ParallelCount = 2;
	static constexpr int PointSpecularCount = 1;
	static constexpr int PointCount = 4;

	// A plain four-float row. Everything below is built from it so that every member is
	// exactly one constant register and the struct offsets are the register offsets.
	struct VectorXyzw
	{
		float x;
		float y;
		float z;
		float w;
	};

	struct VectorAttenuation
	{
		float k0;
		float k1;
		float k2;
		float k3;
	};

	struct ParallelSpecularData
	{
		VectorXyzw direction;
		VectorRgba diffuseColor;
		VectorRgba specularColor;
	};

	struct ParallelData
	{
		VectorXyzw direction;
		VectorRgba diffuseColor;
	};

	struct PointSpecularData
	{
		VectorXyzw position;
		VectorRgba diffuseColor;
		VectorAttenuation attenuation;
		VectorRgba specularColor;
	};

	struct PointData
	{
		VectorXyzw position;
		VectorRgba diffuseColor;
		VectorAttenuation attenuation;
	};

	struct Dot3Data
	{
		VectorXyzw localCameraPosition;
		VectorXyzw localDirection; // w carries the material specular power
		VectorRgba diffuseColor;   // a carries alphaFadeOpacityEnabled
		VectorRgba specularColor;  // a carries alphaFadeOpacity
	};

	// Twenty-eight registers at c16, in this order. Asset ABI; see the file comment.
	struct LightData
	{
		VectorRgba ambient;
		ParallelSpecularData parallelSpecular[ParallelSpecularCount];
		ParallelData parallel[ParallelCount];
		PointSpecularData pointSpecular[PointSpecularCount];
		PointData point[PointCount];
		Dot3Data dot3;
	};

	struct HemisphericLightData
	{
		VectorRgba backColor;
		VectorRgba tangentColor;
		VectorRgba tangentMinusBackColor;
		VectorRgba tangentMinusDiffuseColor; // a carries bloomEnabled
	};

	// Four registers at c60.
	struct ExtendedLightData
	{
		HemisphericLightData parallelSpecular[1];
	};

	// Five registers at PSCR_dot3LightDirection.
	struct PixelDot3Data
	{
		VectorXyzw localDirection;
		VectorRgba diffuseColor;
		VectorRgba specularColor;
		VectorRgba tangentMinusDiffuseColor;
		VectorRgba tangentMinusBackColor; // a carries the current time
	};

	struct SelectedLights
	{
		bool dirty;
		bool obeysLightScale;
		VectorRgba ambient;
		Light const *parallelSpecular[ParallelSpecularCount];
		Light const *parallel[ParallelCount];
		Light const *pointSpecular[PointSpecularCount];
		Light const *point[PointCount];

		bool operator!=(SelectedLights const &rhs) const;
		SelectedLights &operator=(SelectedLights const &rhs);
	};

private:
	Direct3d11_LightManager();
	Direct3d11_LightManager(Direct3d11_LightManager const &);
	Direct3d11_LightManager &operator=(Direct3d11_LightManager const &);

	static void applyLights_vertexShader();
	static void applyLights_vertexShader_dot3();

	static void setExtendedLightData(
		HemisphericLightData &extendedParallelSpecular,
		Light const *light,
		VectorArgb const &diffuseColor,
		float bloomEnabled);
};

// ======================================================================

#endif
