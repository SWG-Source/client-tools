#ifndef __UIBASEOBJECT_H__
#define __UIBASEOBJECT_H__

#include "UILowerString.h"
#include "UISmartPointer.h"
#include "UIString.h"
#include "UITypeID.h"
#include "UITypes.h"
#include "UiMemoryBlockManagerMacros.h"
#include "UIPropertyCategories.h"


//======================================================================================
class UIData;
class UIPropertyGroup;

class UIBaseObject
{
public:
	UI_MEMORY_BLOCK_MANAGER_INTERFACE;

	typedef ui_stdmap<UILowerString, UIString>::fwd   UIPropertyNameMap;
	typedef ui_stdvector<UILowerString>::fwd          UIPropertyNameVector;
	typedef ui_stdlist<UIBaseObject *>::fwd           UIObjectList;
	typedef ui_stdvector<UIBaseObject *>::fwd         UIObjectVector;

	typedef UISmartPointer<UIBaseObject>              UIBaseObjectPointer;
	typedef ui_stdlist<UIBaseObjectPointer>::fwd      UISmartObjectList;
	typedef ui_stdvector<UIBaseObjectPointer>::fwd    UISmartObjectVector;

	typedef ui_stdvector<UIPropertyCategories::Category>::fwd UICategoryVector;
	typedef ui_stdvector<UIPropertyGroup *>::fwd              UIPropertyGroupVector;

	//----------------------------------------------------------------------

	struct DetachFunctor
	{
		explicit DetachFunctor (UIBaseObject * detacher) : m_detacher (detacher) {}

		void operator () (UIBaseObject * obj) const
		{
			obj->Detach (m_detacher);
		}

	private:
		DetachFunctor ();

		UIBaseObject * m_detacher;
	};

	class RemoveFromParentFunctor
	{
	public:
		template <typename KeyType, typename ObjectType>
			void operator ()(std::pair<KeyType, ObjectType> &pairArgument) const
		{
			if (pairArgument.second)
			{
				pairArgument.second->RemoveFromParent();
			}
		}
	};

	//----------------------------------------------------------------------

	class PropertyName
	{
	public:
		static const UILowerString  Name;
		static const UILowerString  SourceFile;
	};
	class CategoryName
	{
	public:
		static const UILowerString  Base;
	};

	static const char * const   TypeName;

	                            UIBaseObject          ();

	virtual bool                IsA                   (const UITypeID Type) const;
	virtual const char         *GetTypeName           () const;
	virtual UIBaseObject       *Clone                 () const = 0;
	virtual void                Destroy               ();

	virtual void                Attach                (const UIBaseObject *);
	virtual bool                Detach                (const UIBaseObject *);
	        unsigned short      GetRefCount           () const;

	        void                SetName               (const UINarrowString & );
	const UINarrowString &      GetName               () const;

	        bool                IsName                (const char * const str ) const;
	        bool                IsName                (const char * const str, unsigned long ) const;

	virtual void                GetPropertyGroups     (UIPropertyGroupVector &, UIPropertyCategories::Category = UIPropertyCategories::C_ALL_CATEGORIES) const;

	virtual void                GetPropertyNames      (UIPropertyNameVector & , bool forCopy) const;
	virtual void                GetLinkPropertyNames  (UIPropertyNameVector &) const;

	        bool                HasProperty           (const UILowerString &  ) const;
	        bool                IsPropertyRemovable   (const UILowerString & Name ) const;
	virtual bool                RemoveProperty        (const UILowerString & Name );
	        void                PurgeProperty         (const UILowerString & Name )  ;

	virtual bool                SetProperty           (const UILowerString & Name, const UIString &Value );
	        bool                SetPropertyBoolean    (const UILowerString & Name, bool Value );
	        bool                SetPropertyColor      (const UILowerString & Name, const UIColor &Value );
	        bool                SetPropertyFloat      (const UILowerString & Name, float Value );
	        bool                SetPropertyInteger    (const UILowerString & Name, int   Value );
	        bool                SetPropertyLong       (const UILowerString & Name, long Value );
	        bool                SetPropertyNarrow     (const UILowerString & Name, const UINarrowString &Value);
	        bool                SetPropertyPoint      (const UILowerString & Name, const UIPoint & Value );
	        bool                SetPropertyRect       (const UILowerString & Name, const UIRect &Value );

	virtual bool                GetProperty           (const UILowerString & Name, UIString &Value ) const;
	        bool                GetPropertyBoolean    (const UILowerString & Name, bool &Value ) const;
	        bool                GetPropertyColor      (const UILowerString & Name, UIColor &Value ) const;
	        bool                GetPropertyColorOrPalette  (const UILowerString & Name, UIColor &Value ) const;
	        bool                GetPropertyFloat      (const UILowerString & Name, float &Value ) const;
	        bool                GetPropertyInteger    (const UILowerString & Name, int   &Value ) const;
	        bool                GetPropertyLong       (const UILowerString & Name, long &Value ) const;
	        bool                GetPropertyNarrow     (const UILowerString & Name, UINarrowString &Value) const;
	        bool                GetPropertyPoint      (const UILowerString & Name, UIPoint &Value ) const;
	        bool                GetPropertyRect       (const UILowerString & Name, UIRect &Value ) const;

	virtual void                ResetLocalizedStrings ();

	virtual void                CopyPropertiesFrom    (const UIBaseObject & rhs);
	        UIPropertyNameMap  *GetPropertyMap        ();
	  const UIPropertyNameMap  *GetPropertyMap        () const;

	virtual void                GetCategories           (UIPropertyCategories::CategoryMask &o_categories) const;
	virtual void                GetPropertiesInCategory (UIPropertyCategories::Category category, UIPropertyNameVector &) const;

	        void                SetParent             (UIBaseObject * );

	        UIBaseObject       *GetParent             ();
	  const UIBaseObject       *GetParent             () const;

	        UIBaseObject       *GetParent             (UITypeID);
	  const UIBaseObject       *GetParent             (UITypeID) const;

	        bool               isAncestor             (const UIBaseObject *) const;

	  
	        UIBaseObject       *GetRoot               () const;

	// Base object does not support children, these are just hooks
	virtual bool                AddChild              (UIBaseObject * );
	virtual bool                RemoveChild           (UIBaseObject * );
	virtual void                SelectChild           (UIBaseObject * );
	virtual UIBaseObject       *GetChild              (const char * ObjectName ) const;
	virtual void                GetChildren           (UIObjectList & ) const;
	virtual unsigned long       GetChildCount         () const;
	virtual void                RemoveFromParent();



	// Function that causes the object to unload as much data as possible
	virtual void                MinimizeResources     () {};

	enum ChildMovementDirection
	{
		Up,
		Down,
		Top,
		Bottom
	};

	virtual bool                 CanChildMove        (UIBaseObject *, ChildMovementDirection );
	virtual bool                 MoveChild           (UIBaseObject *, ChildMovementDirection );

	        void                 GetPathTo           (UINarrowString &, const UIBaseObject * ) const;
	        void                 GetPathTo           (UIString &,       const UIBaseObject * ) const;

	const UINarrowString         GetFullPath         () const;

	        UIBaseObject        *GetObjectFromPath   (const char * str ) const;
	        UIBaseObject        *GetObjectFromPath   (const char * str, UITypeID ) const;
	        UIBaseObject        *GetObjectFromPath   (const UIString & str ) const;
	        UIBaseObject        *GetObjectFromPath   (const UIString & str, UITypeID ) const;

	virtual void                 Link ();

	static void                  GetOutstandingObjects (UIObjectVector & ov);

	virtual UIBaseObject *       DuplicateObject       () const;

	bool                         NotifyActionListener  () const;

	// UISmartPointer interface.
	static void Lock(UIBaseObject * const obj)
	{
		if (obj) 
		{
			obj->Attach(0);
		}
	}

	static void Unlock(UIBaseObject * const obj)
	{
		if (obj) 
		{
			obj->Detach(0);
		}
	}

	//
	void DebugDestroyed() const;

	static void garbageCollect();

protected:
	virtual                    ~UIBaseObject          () = 0;

	/**
	* Returns true if the newObject is != member
	*/
	template <typename T> bool   AttachMember (T * & member, T * newObject)
	{
		if (newObject == member)
			return false;

		if (newObject)
			newObject->Attach (this);

		if (member)
			member->Detach (this);

		member = newObject;

		return true;
	}

	                             UIBaseObject (const UIBaseObject &);
	UIBaseObject &               operator=    (const UIBaseObject &);

#if UI_USE_PROFILER
	char * mFullPath;
#endif

private:
	virtual bool                RemoveProperty        (const char * );
	virtual bool                SetProperty           (const char * , const UIString &);
	virtual bool                GetProperty           (const char * , UIString & );

	// Do not change the order or scope of these member variables.
	//--
	unsigned short               mReferences;
protected:
	unsigned short               mDestroyed;
	UINarrowString               mName;
	//--

private:

	UIBaseObject                *mParent;
	UIPropertyNameMap           *mProperties;

#ifdef _DEBUG
public:
	int mDebugFlag;
#endif
};

#define DEBUG_NOT_DESTROYED_VALUE  0xf33d
#define DEBUG_DESTROYED_VALUE  0xd1ed

#ifdef _DEBUG
	#include <assert.h>
	#define DEBUG_DESTROYED() {UI_DEBUG_REPORT_LOG_PRINT(mDestroyed != DEBUG_NOT_DESTROYED_VALUE, ("Referencing a destroyed or corrupt UI object %s, addr = %x, destroyed flag = %x\n", mName.c_str(), this, mDestroyed)); assert(mDestroyed == DEBUG_NOT_DESTROYED_VALUE);}
#else
	#define DEBUG_DESTROYED()
#endif

//-----------------------------------------------------------------

inline void UIBaseObject::DebugDestroyed() const
{
	DEBUG_DESTROYED();
}

//-----------------------------------------------------------------

inline void UIBaseObject::GetPathTo (UIString & out, const UIBaseObject * obj) const
{
	DEBUG_DESTROYED();

	UINarrowString nstr;
	GetPathTo (nstr, obj);
	out = UIUnicode::narrowToWide (nstr);
}

//-----------------------------------------------------------------

inline UIBaseObject* UIBaseObject::GetObjectFromPath (const UIString & str ) const
{
	DEBUG_DESTROYED();

	return GetObjectFromPath (UIUnicode::wideToNarrow (str).c_str ());
}

//-----------------------------------------------------------------

inline UIBaseObject* UIBaseObject::GetObjectFromPath (const UIString & str, UITypeID id) const
{
	DEBUG_DESTROYED();

	return GetObjectFromPath (UIUnicode::wideToNarrow (str).c_str (), id);
}

//-----------------------------------------------------------------

inline void UIBaseObject::ResetLocalizedStrings ()
{
	DEBUG_DESTROYED();
}

//----------------------------------------------------------------------

inline bool UIBaseObject::IsA (const UITypeID Type ) const
{
	DEBUG_DESTROYED();

	return (Type == TUIObject);
};

//----------------------------------------------------------------------

inline unsigned short UIBaseObject::GetRefCount         () const
{
	DEBUG_DESTROYED();

	return mReferences;
};

//----------------------------------------------------------------------

inline const UINarrowString & UIBaseObject::GetName             () const
{
	DEBUG_DESTROYED();

	return mName;
}

//----------------------------------------------------------------------

inline UIBaseObject::UIPropertyNameMap  * UIBaseObject::GetPropertyMap        ()
{
	DEBUG_DESTROYED();

	return mProperties;
};

//----------------------------------------------------------------------

inline UIBaseObject::UIPropertyNameMap const * UIBaseObject::GetPropertyMap        () const
{
	DEBUG_DESTROYED();

	return mProperties;
};

//----------------------------------------------------------------------

inline UIBaseObject       *UIBaseObject::GetParent ()
{
	DEBUG_DESTROYED();

	return mParent;
};

//----------------------------------------------------------------------

inline const UIBaseObject       *UIBaseObject::GetParent  () const
{
	DEBUG_DESTROYED();

	return mParent;
};


//-----------------------------------------------------------------

#endif // __UIBASEOBJECT_H__
