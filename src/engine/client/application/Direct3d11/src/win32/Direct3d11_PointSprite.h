// ======================================================================
//
// Direct3d11_PointSprite.h
// copyright (c) 2026 Galaxies Reborn
//
// Point sprites, expanded to quads by a geometry shader.
//
// ----------------------------------------------------------------------
// What D3D11 took away
//
// D3D9's D3DRS_POINTSPRITEENABLE turned a point primitive into a screen-aligned quad
// D3DRS_POINTSIZE pixels across, with generated texture coordinates. D3D11 removed the whole
// mechanism: a point primitive there is exactly one pixel and there is no size state at all.
// So without this, every point sprite in the scene is a single pixel.
//
// A geometry shader is the replacement, and this is a case where it is the right tool rather
// than the only one. The alternative -- expanding on the CPU into a dynamic buffer -- means
// reading back vertex data the engine wrote and knowing its layout, for geometry that is already
// tiny.
//
// ----------------------------------------------------------------------
// One shader, because there is one caller
//
// A general point sprite implementation is a hard problem: a geometry shader sits between the
// vertex and pixel stages and its input signature must be a subset of the vertex shader's output
// while its output must satisfy the pixel shader's input. Those vary per program, so a general
// version means generating and compiling a geometry shader per signature at runtime.
//
// None of that is needed here, and the measurement is what says so. Exactly one thing in the
// client draws point sprites: StarAppearance, the star field. It uses one shader,
// shader/stars.sht, at a fixed two pixels. That shader's vertex program outputs
// POSITION0, COLOR0 and FOG and nothing else, and its pixel program is a single assembly
// instruction -- "mul r0, v0, c[textureFactor]" -- which samples no texture at all. So the
// sprite needs no texture coordinates, and one hand-written geometry shader over
// {SV_Position, COLOR0, FOG} covers the entire feature.
//
// setPointScaleEnable, setPointScaleFactor, setPointSizeMin and setPointSizeMax have no caller
// anywhere in the engine. They are recorded so the state is not silently lost, and the
// attenuation they describe is not implemented -- with no caller there is nothing to reproduce
// and no way to test it. If one ever appears it is reported by name.
//
// ----------------------------------------------------------------------
// The size, and why it is a constant rather than baked in
//
// The half-extent is (pixels / viewportDimension) * w. Multiplying by w cancels the perspective
// divide, so the quad is a constant pixel size at any depth -- which is what D3D9 did with
// POINTSCALEENABLE off, the only mode this engine ever uses.
//
// It goes in a constant buffer at b2 rather than being compiled in, so a size change is a
// 16-byte write instead of a shader compile. b0 is $Globals, b1 is the pixel epilogue and b3 is
// the per-object ring, so b2 is the one free slot in the geometry stage's namespace.
//
// ======================================================================

#ifndef INCLUDED_Direct3d11_PointSprite_H
#define INCLUDED_Direct3d11_PointSprite_H

// ======================================================================

class Direct3d11_PointSprite
{
public:
	static void install();
	static void remove();

	// The D3D9 render states, recorded here.
	static void setEnabled(bool enabled);
	static void setSize(float size);
	static float getSize();

	// The states with no engine caller. Recorded, reported once if ever used.
	static void setSizeMinimum(float size);
	static void setSizeMaximum(float size);
	static void setScaleEnabled(bool enabled);
	static void setScaleFactor(float a, float b, float c);

	static bool isEnabled();

	// Bind the geometry shader and push the size, or unbind it. Called from the draw path with
	// whether this draw is a point list -- a triangle list must never keep the expander bound.
	static void apply(bool isPointList);

private:
	Direct3d11_PointSprite();
	Direct3d11_PointSprite(Direct3d11_PointSprite const &);
	Direct3d11_PointSprite &operator=(Direct3d11_PointSprite const &);
};

// ======================================================================

#endif
