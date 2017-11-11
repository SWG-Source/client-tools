#ifndef __UIVolumePage_H__
#define __UIVolumePage_H__

#include "UIEventCallback.h"
#include "UIPage.h"

//======================================================================================

class UIVolumePage : public UIPage, public UIEventCallback
{
public:

	//----------------------------------------------------------------------

	struct PropertyName
	{
		static const UILowerString CellSize;
		static const UILowerString CellPadding;
		static const UILowerString CellMax;
		static const UILowerString CellCount;
		static const UILowerString CellCountFixed;
		static const UILowerString CellForceDragable;
		static const UILowerString CellDragAccepts;
		static const UILowerString CellSelectable;
		static const UILowerString MultiSelection;
		static const UILowerString CellAlignmentV;
		static const UILowerString CellAlignmentH;
		static const UILowerString CellSelectionOverlay;
		static const UILowerString CellSelectionOverlayOpacity;
		static const UILowerString DoNotOverrideMaximumSize;
	};

	//----------------------------------------------------------------------

	enum VerticalAlignment
	{
		VA_Top,
		VA_Center,
		VA_Bottom
	};

	//----------------------------------------------------------------------

	enum HorizontalAlignment
	{
		HA_Left,
		HA_Center,
		HA_Right
	};

	//----------------------------------------------------------------------

	static const char            *TypeName;

	                              UIVolumePage();
	virtual                      ~UIVolumePage();

	virtual bool                  IsA( const UITypeID ) const;
	virtual const char           *GetTypeName( void ) const;
	virtual UIBaseObject         *Clone( void ) const;


	virtual bool                  OnMessage  (UIWidget *Context, const UIMessage & msg );
	virtual void                  OnShow     (UIWidget *Context );
	virtual void                  OnHide     (UIWidget *Context );
	virtual void                  OnHoverIn  (UIWidget *context);
	virtual void                  OnHoverOut (UIWidget *context);


	virtual void                  GetPropertyGroups(UIPropertyGroupVector &, UIPropertyCategories::Category = UIPropertyCategories::C_ALL_CATEGORIES) const;
	virtual void                  GetPropertyNames( UIPropertyNameVector &In, bool forCopy ) const;
	virtual void                  GetLinkPropertyNames (UIPropertyNameVector &) const;
	virtual void CopyPropertiesFrom(const UIBaseObject & rhs);

	virtual bool                  SetProperty( const UILowerString & Name, const UIString &Value );
	virtual bool                  GetProperty( const UILowerString & Name, UIString &Value ) const;

	virtual bool                  AddChild( UIBaseObject * );
	        bool                  InsertChildBefore (UIBaseObject * childToAdd, const UIBaseObject * childToPrecede);
	        bool                  InsertChildAfter  (UIBaseObject * childToAdd, const UIBaseObject * childTosucceed);
	virtual bool                  RemoveChild( UIBaseObject * );
	virtual bool                  MoveChild( UIBaseObject *ObjectToMove, ChildMovementDirection MoveDirection );

	        void                  Clear ();

	virtual UIStyle              *GetStyle( void ) const { return 0; };

	virtual void                  Render( UICanvas & ) const;


	virtual bool                  ProcessMessage( const UIMessage & );
	void                          GetScrollSizes( UISize &PageSize, UISize &LineSize ) const;

//	virtual void                  Link( void );

	virtual void                  Pack();

	virtual void                  SetSize( const UISize & );

	        long                  GetCellMax           () const;
	        long                  GetWidgetCount       () const;
	        void                  SetCellMax           (long val);
	const   UISize &              GetCellCount         () const;
	const   UISize &              GetCellCountFixed    () const;
	        void                  SetCellCountFixed    (const UISize & count);

	const   UISize &              GetCellSize          () const;
	        void                  SetCellSize          (const UISize & count);

	const   UISize &              GetCellPadding       () const;
	        void                  SetCellPadding       (const UISize & count);

	UIWidget *                    GetLastSelectedChild ();
	        void                  GetSelection         (UIWidget::UIWidgetVector & selection) const;

	        int                   GetLastSelectedIndex () const;
	        void                  SetSelectionIndex    (int index);
	        void                  SetSelection         (UIWidget * child, bool const clearPreviousSelections = true);

	        void                  SetMultiSelection    (bool b);
	        bool                  GetMultiSelection    () const;
	        bool                  GetDoMultiSelection  () const;
	        bool                  GetCellForceDragable () const;

	        VerticalAlignment     GetCellAlignmentVertical   () const;
	        HorizontalAlignment   GetCellAlignmentHorizontal () const;

	        void                  CenterChild        (const UIWidget & child);
	        void                  EnsureChildVisible (const UIWidget & child);

	static  bool                  FindVerticalAlignment     (const char * name, VerticalAlignment & va);
	static  bool                  FindHorizontalAlignment   (const char * name, HorizontalAlignment & ha);
	static  bool                  FormatVerticalAlignment   (VerticalAlignment va, UIString & Value);
	static  bool                  FormatHorizontalAlignment (HorizontalAlignment ha, UIString & Value);

	const UIRectangleStyle *      GetCellSelectionOverlay () const;
	void                          SetCellSelectionOverlay (UIRectangleStyle * rs);

	UIPoint                       FindChildCell           (int index) const;

	UIWidget *                    FindCell                (const UIPoint & thePoint, bool clip = false);
	UIWidget *                    FindCell                (int index);
	int                           FindCellIndex           (const UIWidget & child) const;

	bool OptimizeChildSpacing(int count);

private:

	                        UIVolumePage( UIVolumePage & );
	                        UIVolumePage                 &operator = ( UIVolumePage & );

private:

	UISize                  mCellSize;
	UISize                  mCellPadding;
	UISize                  mCellCount;
	UISize                  mCellCountFixed;

	long                    mCellMax;
	long                    mWidgetCount;

	bool                    mPackingRecursionGuard;
	bool                    mCallbackForwardingRecursionGuard;

	UIWidget *              mLastSelectedChild;

	bool                    mMultiSelection;
	bool					mDoMultiSelection;
	bool                    mCellForceDragable;
	bool                    mCellSelectable;

	VerticalAlignment       mCellAlignmentVertical;
	HorizontalAlignment     mCellAlignmentHorizontal;

	UIRectangleStyle *      mCellSelectionOverlay;
	float                   mCellSelctionOverlayOpacity;

	bool                    mDoNotOverrideMaximumSize;
	
	bool                    mMouseDown;

};


//-----------------------------------------------------------------

inline long UIVolumePage::GetCellMax () const
{
	return mCellMax;
}

//-----------------------------------------------------------------

inline long UIVolumePage::GetWidgetCount () const
{
	return mWidgetCount;
}
//-----------------------------------------------------------------

inline void UIVolumePage::SetCellMax (long val)
{
	mCellMax = val;
}

//-----------------------------------------------------------------

inline const   UISize & UIVolumePage::GetCellCount () const
{
	return mCellCount;
}

//----------------------------------------------------------------------

inline UIWidget * UIVolumePage::GetLastSelectedChild ()
{
	return mLastSelectedChild;
}

//----------------------------------------------------------------------

/**
* A zero in the x or y values of the fixed cell count indicates that
* That dimension is not fixed.
*/
inline const   UISize & UIVolumePage::GetCellCountFixed () const
{
	return mCellCountFixed;
}

//----------------------------------------------------------------------

inline bool UIVolumePage::GetMultiSelection () const
{
	return mMultiSelection;
}

//----------------------------------------------------------------------

inline bool UIVolumePage::GetDoMultiSelection () const
{
	return mDoMultiSelection;
}

//----------------------------------------------------------------------

inline bool UIVolumePage::GetCellForceDragable () const
{
	return mCellForceDragable;
}

//----------------------------------------------------------------------

inline UIVolumePage::VerticalAlignment   UIVolumePage::GetCellAlignmentVertical () const
{
	return mCellAlignmentVertical;
}

//----------------------------------------------------------------------

inline UIVolumePage::HorizontalAlignment UIVolumePage::GetCellAlignmentHorizontal () const
{
	return mCellAlignmentHorizontal;
}

//----------------------------------------------------------------------

inline const UIRectangleStyle *      UIVolumePage::GetCellSelectionOverlay () const
{
	return mCellSelectionOverlay;
}

//----------------------------------------------------------------------

inline const   UISize & UIVolumePage::GetCellSize          () const
{
	return mCellSize;
}

//----------------------------------------------------------------------

inline const   UISize & UIVolumePage::GetCellPadding       () const
{
	return mCellPadding;
}

//-----------------------------------------------------------------

#endif // __UIVolumePage_H__
