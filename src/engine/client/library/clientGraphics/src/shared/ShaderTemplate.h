// ======================================================================
//
// ShaderTemplate.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ShaderTemplate_H
#define INCLUDED_ShaderTemplate_H

// ======================================================================

class CustomizationVariable;
class Shader;

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/PersistentCrcString.h"

// ======================================================================

class ShaderTemplate
{
	friend class ShaderTemplateList;

public:

	explicit ShaderTemplate(const CrcString &name);
	virtual ~ShaderTemplate();

	void fetch() const;
	void release() const;

	const CrcString               &getName() const;

	virtual int                    getCustomizationVariableCount() const;
	virtual const std::string     &getCustomizationVariableName(int index) const;
	virtual CustomizationVariable *createCustomizationVariable(int index) const;

	virtual bool                   isOpaqueSolid() const = 0;
	virtual bool                   isCollidable() const = 0;
	virtual bool                   castsShadows() const = 0;
	virtual bool                   containsPrecalculatedVertexLighting() const;

	virtual const Shader          *fetchShader() const = 0;
	virtual Shader                *fetchModifiableShader() const = 0;

#ifdef _DEBUG
	void                           setDebugName(CrcString const *debugName) const;
#endif

private:

	/// Disabled.
	ShaderTemplate();

	/// Disabled.
	ShaderTemplate(const ShaderTemplate &);

	/// Disabled.
	ShaderTemplate &operator =(const ShaderTemplate &);

private:

	mutable int          m_users;
	PersistentCrcString  m_name;

#ifdef _DEBUG
	mutable CrcString const *m_debugName;
#endif
};

// ======================================================================

inline const CrcString &ShaderTemplate::getName() const
{
#ifdef _DEBUG
	if (m_debugName)
		return *m_debugName;
	else
		return m_name;
#else
	return m_name;
#endif
}

// ======================================================================

#endif
