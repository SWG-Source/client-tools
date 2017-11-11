#ifndef __UIPAGE_H__
#define __UIPAGE_H__

#include "UIWidget.h"

class UIButton;
class UITextStyle;

//======================================================================================

class UIPage : public UIWidget
{
public:
	static void install(){}
	static void remove(){}

public:
	typedef ui_stdlist<UIWidget *>::fwd	UIWidgetList;

	static const char       *TypeName;
	
	struct PropertyName
	{
		static const UILowerString DoNotPackChildren;
		static const UILowerString ParentSize;
	};
	
	struct MethodName
	{
		static const UILowerString Pack;
		static const UILowerString Wrap;
	};
	                        UIPage                       ();
	virtual                ~UIPage                       ();
	
	virtual bool            IsA                          ( const UITypeID ) const;
	virtual const char     *GetTypeName                  () const;
	virtual UIBaseObject   *Clone                        () const;
	
	virtual bool            SetProperty                  ( const UILowerString & Name, const UIString &Value );
	virtual bool            GetProperty                  ( const UILowerString & Name, UIString &Value ) const;
	virtual void            GetPropertyGroups(UIPropertyGroupVector &, UIPropertyCategories::Category = UIPropertyCategories::C_ALL_CATEGORIES) const;
	virtual void            GetPropertyNames             (UIPropertyNameVector & , bool forCopy ) const;

	
	virtual bool            AddChild                     ( UIBaseObject * );
	virtual bool            InsertChildBefore            (UIBaseObject * childToAdd, const UIBaseObject * childToPrecede);
	virtual bool            InsertChildAfter             (UIBaseObject * childToAdd, const UIBaseObject * childTosucceed);
	virtual UIBaseObject   *GetChild                     ( const char *ChildName ) const;
	virtual bool            RemoveChild                  ( UIBaseObject * );
	virtual void            SelectChild                  ( UIBaseObject * );
	virtual void            GetChildren                  ( UIObjectList & ) const;
	virtual unsigned long   GetChildCount                () const;
	virtual bool            CanChildMove                 ( UIBaseObject *, ChildMovementDirection );
	virtual bool            MoveChild                    ( UIBaseObject *, ChildMovementDirection );
	        void            ChangeFocusOrder             (UIWidget *insertBefore, UIWidget *child);
	virtual UIWidget       *GetWidgetFromPoint           ( const UIPoint &, bool mustGetInput ) const;
	
	virtual UIStyle        *GetStyle                     () const { return 0; };
	
	virtual void            Render                       (UICanvas & ) const;
	virtual void            MinimizeResources            ();
	
	virtual void            SetSelected                  (const bool );
	virtual bool            CanSelect                    () const;
	bool                    CanSelectForTab              () const;
	
	virtual bool            ProcessMessage               (const UIMessage & );
	
	virtual void            Link                         ();
	
	virtual void            Pack                         ();
	        void            PackIfDirty                  ();
			
	        void            AddOverlay                   (UIWidget * );
	        void            RemoveOverlay                (UIWidget * );
	
	        void            GetWidgetList                (UIWidgetList & ) const;
	
	virtual void            ResetLocalizedStrings        ();
	virtual void            SetScrollExtent              (const UISize & );

	        bool            ProcessMouseMessageUsingControlSet          ( const UIMessage &, UIObjectList &, bool & childWasHit );
	virtual UIWidget *      GetFocusedLeafWidget         ();

	        void            ReleaseMouseLock             (const UIPoint & point);
			void            GiveWidgetMouseLock          (UIWidget *);
	const UIObjectList &    GetChildrenRef               () const;

	virtual void            SetUnderMouse                ( bool b);
	        void            UpdateUnderMouse             (const UIPoint & pt);

	virtual void            Clear                        ();

	        UIButton       *FindDefaultButton            (bool useFocusedChild);
	        UIButton       *FindCancelButton             (bool useFocusedChild);


	static UIPage *         DuplicateInto                (UIPage & parent, const char * const path);
	        void            MoveKeyboardFocus            (bool Forward );

			void            ForcePackChildren            ();
	void                    SetPackDirty                 (bool b) const;

	const UIWidget *        GetWidgetWithMouseLock         () const;

	void SetDoNotPackChildren(bool bIgnore) const;
	bool GetDoNotPackChildren() const;

	enum PageAttributeFlags
	{
		PA_Initialize					= (0x00000000),
		PA_WidgetVectorDirty			= (0x00000001),
		PA_PackDirty					= (0x00000002),
		PA_DoNotPackChildren			= (0x00000004),
		PA_IsPacking					= (0x00000008),
		PA_PackIgnoresHiddenObjects		= (0x00000010),
		PA_ParentSize                   = (0x00000020)
	};

	bool SetPageAttribute(PageAttributeFlags flag, bool value) const;
	bool HasPageAttribute(PageAttributeFlags flag) const { return (mPageAttributeFlags & static_cast<int>(flag)) != 0; }

	template <typename T> T *    GetCodeDataObject(const UIData * theData, UITypeID id, T *& var, const char * name, bool optional) const
	{
		UIBaseObject * const result = GetCodeDataObject(theData, id, name, optional);
		
		if (result && result->IsA(id))
		{
			var = static_cast<T*>(result);
			return var;
		}
		
		UI_REPORT_LOG_PRINT_ALWAYS(("UIPage [%s] Missing CodeData.\n", GetFullPath().c_str()));;
		
		return 0;
	}
	
	template <typename T> T *    GetCodeDataObject(UITypeID id, T *& var, const char * name, bool optional) const
	{
		if (mCodeData) // Did you link?
		{
			return GetCodeDataObject(mCodeData, id, var, name, optional);
		}
		
		UI_REPORT_LOG_PRINT_ALWAYS(("UIPage [%s] Missing CodeData pointer.\n", GetFullPath().c_str()));;
		
		return NULL;
	}
	
	template <typename T> void   GetCodeDataObject(UITypeID id, T *& var, const char * name) const
	{
		GetCodeDataObject(id, var, name, false);
	}
	
	UIBaseObject * GetCodeDataObject(const UIData * theData, UITypeID id, const char * name, bool optional) const;

	virtual void OnSizeChanged(UISize const & newSize, UISize const & oldSize);
	virtual void WrapChildren();

	virtual UIBaseObject * GetPreviousChild(UIBaseObject const * const child) const;
	virtual UIBaseObject * GetNextChild(UIBaseObject const * const child) const;

	void SetTextStyle(UITextStyle const * const style, bool const overrideSize);
	void ApplyPackingToChildWidgets();

	UIData const * GetCodeData() const;
	void SetSizeToParentSize();

	void GetVisibleChildren(UISmartObjectVector & children);
protected:

	void                    ApplyPacking                 (UIWidget *theWidget) const;
	UIWidgetVector &        GetWidgetVector              () const;

	void                    SetWidgetWithMouseLock       (UIWidget * w);

private:
	                        UIPage( UIPage const & );
	                        UIPage                 &operator = ( UIPage const & );
	
	        void            ProcessTab                  (const UIMessage &, bool allowParentCycling = true);
	        UIWidget       *GetNextWidgetInTabOrder     (const UIBaseObject * ) const;
	        UIWidget       *GetPreviousWidgetInTabOrder (const UIBaseObject * ) const;
	        void            BuildWidgetVector           () const;
	        void            CleanUpFocusHandles         ();
private:
	
	UIObjectList *          mObjects;
	UIObjectList *          mTabOrder;
	UIWidgetVector *        mScratchVector;
	
	UIWidget *              mWidgetWithKeyboardFocus;
	UIWidget *              mWidgetWithMouseLock;
	UIWidget *              mWidgetUnderMouse;


	mutable UIWidgetVector * mWidgets;
	mutable int mPageAttributeFlags;

	UIData * mCodeData;
};


//----------------------------------------------------------------------

#endif // __UIPAGE_H__
