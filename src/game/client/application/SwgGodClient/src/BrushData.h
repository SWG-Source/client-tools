// ======================================================================
//
// BrushData.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_BrushData_H
#define INCLUDED_BrushData_H

#include "Singleton/Singleton.h"
#include "GodClientData.h"

// ======================================================================

/**
* BrushData is a singleton for storing information about the user's currently
* assigned brushes
*/

class BrushData : public Singleton<BrushData>, public MessageDispatch::Emitter
{

public:

	struct Messages
	{
		static const char* const BRUSHES_CHANGED;
		static const char* const BRUSH_DRAGGED;
	};

	struct BrushStruct
	{
		GodClientData::ClipboardList_t objects;
		std::string                    name;
	};

	//declare nested class here, define in cpp file
	struct BrushContainer;

	BrushData();
	~BrushData();
	void                      addBrush(const std::string& name, const GodClientData::ClipboardList_t& objects);
	void                      removeBrush(size_t index);
	size_t                    getNumBrushes() const;
	const BrushStruct&        getBrush(size_t index) const;
	void                      setSelectedBrush(const std::string& name);
	void                      serialize() const;
	void                      unserialize();
	const GodClientData::ClipboardList_t* getSelectedBrush() const;

private:
	//disabled
	BrushData(const BrushData& rhs);
	BrushData& operator= (const BrushData& rhs);

private:
	BrushContainer *  m_brushes;
	std::string       m_selectedBrush;
};

// ======================================================================

#endif
