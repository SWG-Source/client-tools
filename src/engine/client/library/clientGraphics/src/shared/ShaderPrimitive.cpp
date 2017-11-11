// ======================================================================
//
// ShaderPrimitive.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "clientGraphics/FirstClientGraphics.h"
#include "clientGraphics/ShaderPrimitive.h"

#include "clientGraphics/StaticShader.h"
#include "sharedObject/AlterResult.h"

// ======================================================================

ShaderPrimitive::ShaderPrimitive() :
m_radius(0.0f)
{
}

// ----------------------------------------------------------------------

ShaderPrimitive::~ShaderPrimitive()
{
}

// ----------------------------------------------------------------------

void ShaderPrimitive::getCostEstimate(int &, int &, float &) const
{
}

// ----------------------------------------------------------------------
/**
 * This routine should be called once per frame.
 */

float ShaderPrimitive::alter(const float /*elapsedTime*/)
{
	// This class functionality does not require a per-frame alter.
	return AlterResult::cms_keepNoAlter;
}

// ----------------------------------------------------------------------
/**
 * This routine should be called once per render.
 */

const StaticShader &ShaderPrimitive::prepareToView() const
{
	DEBUG_FATAL(true, ("Do not call this routine"));
	return reinterpret_cast<StaticShader *>(NULL)[0]; //lint !e413 !e527 // likely use of null pointer, unreachable
}

// ----------------------------------------------------------------------
/**
 * Get the world Y position of the primitive
 * @return The world Y position of the primitive
 */

const Vector ShaderPrimitive::getPosition_w() const
{
	DEBUG_FATAL(true, ("Do not call this routine"));
	return Vector::zero; //lint !e527 // Unreachable // Reachable in release.
}

// ----------------------------------------------------------------------
/**
 * Get the depth sort key for this primitive.
 * @return A value to allow sorting by depth
 */

float ShaderPrimitive::getDepthSquaredSortKey() const
{
	DEBUG_FATAL(true, ("Do not call this routine"));
	return 0; //lint !e527 // Unreachable // Reachable in release.
}

// ----------------------------------------------------------------------
/**
 * Get the vertex buffer sort key for this primitive.
 * @return A value to allow sorting by vertex buffer.
 */

int ShaderPrimitive::getVertexBufferSortKey() const
{
	DEBUG_FATAL(true, ("Do not call this routine"));
	return 0; //lint !e527 // Unreachable // Reachable in release.
}

// ----------------------------------------------------------------------
/**
 * This routine may be called one or more times (up to the number of passes required).
 */

void ShaderPrimitive::prepareToDraw() const
{
	DEBUG_FATAL(true, ("Do not call this routine"));
}

// ----------------------------------------------------------------------
/**
 * This routine will be called once for each rendering pass required.
 */

void ShaderPrimitive::draw() const
{
	DEBUG_FATAL(true, ("Do not call this routine"));
}

// ----------------------------------------------------------------------
/**
 * Set the CustomizationData instance to be associated with this
 * ShaderPrimitive instance.
 *
 * Notes to Derived-class implementers: You do not need to chain down
 * to this function.  You do not need to override this function if the
 * ShaderPrimtive does not have any knowledge of any entities that need
 * to be customized.  Note Shader instances will need to know about
 * CustomizationData soon.  This default implementation does nothing.
 *
 * @param customizationData  the new CustomizationData instance to be
 *                           associated with this ShaderPrimitive instance.
 *                           This value may be NULL and may be the same
 *                           customizationData value already associated with
 *                           this ShaderPrimitive instance.
 */

void ShaderPrimitive::setCustomizationData(CustomizationData * /* customizationData */)
{
}

// ----------------------------------------------------------------------
/**
 * Add all CustomizationData variables that influence this ShaderPrimitive instance
 * to the given CustomizationData instance.
 *
 * This is primarily useful as a mechanism for tools.  The game should already
 * know which customization variables it has enabled for objects via the
 * ObjectTemplate system.
 *
 * Derived classes do not need to chain down to this function.  This implementation
 * does nothing and is provided to prevent unnecessary declaration of an empty function
 * in the many classes derived from this class.
 *
 * Implementers: there are no guarantees as to the order of calls between 
 * setCustomizationData() and this function.  In other words, the implementation 
 * should use the given CustomizationData arg to add variables.
 *
 * @param customizationData  the CustomizationData instance to which new
 *                           variables will be added.
 */
void ShaderPrimitive::addCustomizationVariables(CustomizationData & /* customizationData */) const
{
}

// ----------------------------------------------------------------------
/**
 * Post-E3, this should move to a derived class
 */

void ShaderPrimitive::setSkinningMode(SkinningMode /* skinningMode */)
{
}

// ----------------------------------------------------------------------

void ShaderPrimitive::calculateSkinnedGeometryNow()
{
}

// ----------------------------------------------------------------------
/**
 * Find out if this shader primitive is ready to be used.
 *
 * Derived class implementers: override this function and do not chain
 * down if there is the potential for your shader primitive to not always
 * be ready for rendering.  For example, a texture renderer shader primitive
 * will not be ready for rendering if the baked texture has not yet been
 * baked.  Appearances can make use of this mechanism to handle waiting for
 * textures to bake before allowing themselves to be displayed.
 *
 * @return  true if this ShaderPrimitive instance is ready to be used; false otherwise.
 */

bool ShaderPrimitive::isReady() const
{
	return true;
}

// ----------------------------------------------------------------------

bool ShaderPrimitive::collide(const Vector & /*start_o*/, const Vector & /*end_o*/, CollisionInfo & /*result*/) const
{
	return false;
}

// ----------------------------------------------------------------------

SoftwareBlendSkeletalShaderPrimitive *ShaderPrimitive::asSoftwareBlendSkeletalShaderPrimitive()
{
	return NULL;
}

// ----------------------------------------------------------------------

SoftwareBlendSkeletalShaderPrimitive const *ShaderPrimitive::asSoftwareBlendSkeletalShaderPrimitive() const
{
	return NULL;
}

// ----------------------------------------------------------------------

TextureRendererShaderPrimitive *ShaderPrimitive::asTextureRendererShaderPrimitive()
{
	return NULL;
}

// ----------------------------------------------------------------------

TextureRendererShaderPrimitive const *ShaderPrimitive::asTextureRendererShaderPrimitive() const
{
	return NULL;
}

// ======================================================================
