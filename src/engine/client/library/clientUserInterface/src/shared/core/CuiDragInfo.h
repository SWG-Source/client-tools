//======================================================================
//
// CuiDragInfo.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiDragInfo_H
#define INCLUDED_CuiDragInfo_H

//======================================================================

#include "clientUserInterface/CuiDragInfoTypes.h"
#include "sharedObject/CachedNetworkId.h"
#include "UILowerString.h"
#include "UIString.h"
#include <string>

class UIBaseObject;
class ClientObject;
class AttachmentData;

//----------------------------------------------------------------------

class CuiDragInfo
{
public:
	struct Properties
	{
		static const UILowerString CommandValue;
		static const UILowerString CommandName;
		static const UILowerString CommandString;
		static const UILowerString Oob;
		static const UILowerString DragInfoName;
	};

	struct DragTypes
	{
		static const char * const GameObject;
		static const char * const Command;
		static const char * const AttachmentData;
		static const char * const Macro;
	};

	                        CuiDragInfo          ();
	explicit                CuiDragInfo          (const UIBaseObject & dragWidget);
	                       ~CuiDragInfo          ();

	void                    set                  (const UIBaseObject & dragWidget);
	void                    set                  (const AttachmentData & ad);

	void                    setWidget            (UIBaseObject & dragWidget) const;
	void                    clear                ();
	ClientObject *          getClientObject      () const;
	bool                    ok                   () const;
	bool                    operator<            (const CuiDragInfo & rhs) const;
	const AttachmentData *  getAttachmentData    () const;

	static const char * const getDragTypeString  (int type);
	static CuiDragInfoTypes::Type getDragType    (const std::string & str);

	UIString               name;
	std::string            str;
	std::string            cmd;
	CachedNetworkId        objectId;
	CuiDragInfoTypes::Type type;
	float                  commandValue;
	bool                   commandValueValid;

private:
	AttachmentData *       m_attachmentData;
};

//----------------------------------------------------------------------

inline bool CuiDragInfo::ok () const
{
	return (type != CuiDragInfoTypes::CDIT_none);
}

//----------------------------------------------------------------------

inline bool CuiDragInfo::operator<(const CuiDragInfo & rhs) const
{
	if (type < rhs.type)
		return true;
	
	if (type == rhs.type)
	{
		if (objectId < rhs.objectId)
			return true;
		
		if (objectId == rhs.objectId)
		{
			if (cmd < rhs.cmd)
				return true;
			
			if (cmd == rhs.cmd)
			{
				if (str < rhs.str)
					return true;
				
				if (str == rhs.str)
				{
					if (name < rhs.name)
						return true;

					if(name == rhs.name)
					{
						if (commandValueValid)
						{
							if (commandValue < rhs.commandValue)
								return true;
						}
					}
				}
			}
		}
	}
	
	return false;
}

//----------------------------------------------------------------------

inline const AttachmentData * CuiDragInfo::getAttachmentData    () const
{
	return m_attachmentData;
}

//======================================================================

#endif
