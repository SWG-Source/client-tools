// ======================================================================
//
// ClientEffectTemplateList.h
// Copyright 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================


#ifndef INCLUDED_ClientEffectTemplateList_H
#define INCLUDED_ClientEffectTemplateList_H

// ======================================================================

class CellProperty;
class ClientEffect;
class ClientEffectTemplate;
class CrcLowerString;
class Iff;
class LessPointerComparator;
class Object;
class Transform;
class Vector;

// ======================================================================

/** This class maintains a list of all active client effect templates and provides the 
 *  interface for all other game systems to create an manipulate ClientEffectTemplates.
 */
class ClientEffectTemplateList
{
	//allow ClientEffectTemplate private access (specifically to stopTracking())
	friend ClientEffectTemplate;

public:

	static void install();
	static void remove();

	static const ClientEffectTemplate* fetch(const CrcLowerString& filename);

	static ClientEffect*         createClientEffect(const CrcLowerString& filename, const CellProperty* cell, const Vector& position, const Vector& up);
	static ClientEffect*         createClientEffect(const CrcLowerString& filename, Object* object, const CrcLowerString& hardPointName);
	static ClientEffect*         createClientEffect(const CrcLowerString& filename, Object* object, Transform const & transform);

private:
	static void stopTracking(const ClientEffectTemplate* clientEffectTemplate);

private:
	//disabled
	ClientEffectTemplateList();
	ClientEffectTemplateList(const ClientEffectTemplateList&);
	ClientEffectTemplateList& operator=(const ClientEffectTemplateList&);

private:
	typedef stdmap<const CrcLowerString*, ClientEffectTemplate*, LessPointerComparator>::fwd  TemplateMap;

	static bool         ms_installed;
	///a container holding all currently loaded ClientEffectTemplates
	static TemplateMap* ms_templates;
};

// ======================================================================

#endif
