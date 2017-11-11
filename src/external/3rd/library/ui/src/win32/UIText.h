#ifndef __UITEXT_H__
#define __UITEXT_H__

#include "UITextStyle.h"
#include "UIWidget.h"

class UiMemoryBlockManager;
class UITextStyleWrappedText;
class UIIMEComposition;
class UIListbox;


//-----------------------------------------------------------------

class UIText : public UIWidget
{
public:

	typedef ui_stdvector<Unicode::String::const_iterator>::fwd      UIStringConstIteratorVector;

	static const char * const TypeName;

	class PropertyName
	{
	public:
		static const UILowerString ColorCarat;
		static const UILowerString ColorSelection;
		static const UILowerString CursorMark;
		static const UILowerString CursorPoint;
		static const UILowerString Editable;
		static const UILowerString EditableUnicode;
		static const UILowerString LocalText;
		static const UILowerString Margin;
		static const UILowerString MaxLines;
		static const UILowerString OpacityCarat;
		static const UILowerString OpacitySelection;
		static const UILowerString PreLocalized;
		static const UILowerString Style;
		static const UILowerString Text;
		static const UILowerString TextAlignment;
		static const UILowerString TextAlignmentVertical;
		static const UILowerString TextColor;
		static const UILowerString TextCapital;
		static const UILowerString TextUnroll;
		static const UILowerString TextUnrollOnce;
		static const UILowerString TextUnrollSpeed;
		static const UILowerString TextUnrollSound;
		static const UILowerString TextSelectable;
		static const UILowerString TruncateElipsis;
		static const UILowerString IME;
		static const UILowerString DropShadow;
		static const UILowerString DropShadowAutoDetect;
	};

	class TextAlignmentVerticalNames
	{
	public:
		static const std::string    Top;
		static const std::string    Center;
		static const std::string    Bottom;
	};

	enum TextAlignmentVertical
	{
		TAV_top,
		TAV_center,
		TAV_bottom
	};

	enum TextFlag
	{
		TF_dirty            = 0x0001,
		TF_drawCarat        = 0x0002,
		TF_editable         = 0x0004,
		TF_preLocalized     = 0x0008,
		TF_truncateElipsis  = 0x0010,
		//TF_scrollLockBottom = 0x0020,
		TF_textSelectable   = 0x0040,
		TF_textCapital      = 0x0080,
		TF_textUnroll       = 0x0100,
		TF_textUnrollOnce   = 0x0200,
		TF_IME              = 0x0400,		
		TF_EditableUnicode  = 0x0800,
		TF_drawLast         = 0x1000,
	};
	                                 UIText                      ();
	virtual                         ~UIText                      ();

	virtual bool                     IsA                         (const UITypeID) const;
	virtual const char              *GetTypeName                 () const;
	virtual UIBaseObject            *Clone                       () const;

	virtual void                     GetPropertyGroups(UIPropertyGroupVector &, UIPropertyCategories::Category = UIPropertyCategories::C_ALL_CATEGORIES) const;
	virtual void                     GetPropertyNames            (UIPropertyNameVector &, bool forCopy) const;
	virtual void                     GetLinkPropertyNames (UIPropertyNameVector &) const;
	virtual void                     CopyPropertiesFrom(const UIBaseObject & rhs);

	virtual bool                     SetProperty                 (const UILowerString & Name, const UIString &Value);
	virtual bool                     GetProperty                 (const UILowerString & Name, UIString &Value) const;

//	virtual void                     SetRect                     (const UIRect &);
	virtual void                     SetSize                     (const UISize &);
//	virtual void                     SetWidth                    (const long);

	virtual bool                     ProcessMessage              (const UIMessage &);
	virtual void                     Render                      (UICanvas &) const;
	virtual bool                     CanSelect                   () const;

	virtual void                     GetScrollExtent             (UISize & size) const;
	virtual void                     GetScrollSizes              (UISize &PageSize, UISize &LineSize) const;

	        void                     SetStyle                    (UITextStyle *);
	        UITextStyle             *GetTextStyle                () const;
	virtual UIStyle                 *GetStyle                    () const;

	        void                     SetText                     (const UIString &);
	        void                     GetText                     (UIString &) const;
	        const UIString &         GetText                     () const;

	        void                     SetLocalText                (const UIString &);
	        void                     GetLocalText                (UIString &) const;
	        const UIString &         GetLocalText                () const;

	        int                      GetLineCount                () const;
	        void                     RemoveLeadingLines          (int LinesToRemove);
	        void                     AppendLocalText             (const UIString &);

	        void                     SizeToContent               ();

	        bool                     IsEmpty                     () const;
	        int                      GetDataLength               () const;

	virtual void                     ResetLocalizedStrings       ();

	const UIRect &                   GetMargin                   () const;
	void                             SetMargin                   (const UIRect & margin);

	const UIColor &                  GetTextColor                () const;
	void                             SetTextColor                (const UIColor & color);

	        bool                     IsPreLocalized              () const;
	        void                     SetPreLocalized             (bool b);

	const UISize &                   GetTextExtent               () const;

	        void                     SetMaxLines                 (long lines);
	        long                     GetMaxLines                 () const;

	        void                     SetTruncateElipsis          (bool t);
	        bool                     IsTruncateElipsis           () const;

	void                             SetTextAlignment            (UITextStyle::Alignment align);

	void                             SetTextAlignmentVertical    (TextAlignmentVertical t);

	static bool                      ParseTextAlignmentVertical  (const Unicode::String & str, TextAlignmentVertical & tav);
	static void                      FormatTextAlignmentVertical (Unicode::String & Value, const TextAlignmentVertical tav);

	void                             SetCursorMark  (int mark);
	void                             SetCursorPoint (int point);
	int                              GetCursorMark  () const;
	int                              GetCursorPoint () const;

	int                              GetCharacterFromLocation (const UIPoint & pt) const;
	int                              GetLineFromCharacter     (int c, int & startC) const;

	void                             ReplaceSelection (const Unicode::String & str);
	bool                             DeleteSelection  ();
	bool                             ClearSelection   ();

	bool                             IsEditable       () const;
	void                             SetEditable      (bool b);
	void                             Ding             ();

	bool                             HasTextFlag (TextFlag flag) const;
	bool                             SetTextFlag (TextFlag flag, bool value);

	void                             EnsureCaratVisible ();

	const UIPoint                    GetCaratPos () const;
 	const long                       GetMaximumCharHeight () const;
	const UIPoint                    FindCaratPos (int point) const;
	int                              FindCharacterFromCaratOffset (const UIPoint & offset) const;

	void                             CopySelectionToClipboard () const;
	void                             PasteFromClipboard ();

	bool                             MoveCaratLeftOneWord ();
	bool                             MoveCaratRightOneWord ();

	void                             SetTextSelectable        (bool b);
	bool                             IsTextActuallySelectable () const;

	void                             Clear                 ();

	bool                             GetTextCapital        () const;
	void                             SetTextCapital        (bool b);

	bool                             GetTextUnroll         () const;
	void                             SetTextUnroll         (bool b);

	float                            GetTextUnrollSpeed    () const;
	void                             SetTextUnrollSpeed    (float charactersPerSec);

	void                             ResetTextUnroll       ();

	int                              GetTextUnrollProgress () const;
	void                             SetTextUnrollProgress (int p);

	bool                             GetTextUnrollOnce     () const;
	void                             SetTextUnrollOnce     (bool b);

	void                             SetMaximumCharacters  (int const maximumCharacters);
	int                              GetMaximumCharacters  () const;
	bool                             IsAtMaximumCharacters () const;
	int                              GetCharacterCount     () const;

	virtual void                     SetSelected           (const bool NewSelected );

	void                             SetIMEEnabled          (bool b);
	bool                             GetIMEEnabled          () const;
	
	void                             SetEditableUnicode     (bool b);
	bool                             IsEditableUnicode      () const;

	void							 ConsumeNextCharacter();

	void SetUnrollSound(std::string const & sound);
	std::string const & GetUnrollSound() const;

	void SetDropShadow(bool dropShadow);
	bool GetDropShadow() const;

	void SetDropShadowAutoDetect(bool dropShadow);
	bool GetDropShadowAutoDetect() const;

private:

	                                 UIText( UIText const & );
	        UIText                  &operator = ( UIText const & );

	        void                    CacheTextMeasurements ();
			const UIPoint           GetTextPos            () const;
			void                    SyncRenderDataToLocalText ();

	UITextStyle                    *mStyle;
	UIString                        mData;
	UIString                        mLocalText;
	UIString                        mRenderData;

	UITextStyleWrappedText *        mLines;

	UITextStyle::Alignment          mTextAlignment;

	UIRect                          mMargin;

	UISize                          mTextExtent;

	UIColor                         mTextColor;
	UIColor                         mColorCarat;
	UIColor                         mColorSelection;

	long                            mMaxLines;

	TextAlignmentVertical           mTextAlignmentVertical;

	int                             mCursorMark;
	int                             mCursorPoint;

	unsigned int                    mTextFlags;

	mutable unsigned int            mNextCaratBlinkTime;

	UIPoint                         mOldScrollLocationToBottom;

	float                           mTextUnrollSpeed;

	mutable int                     mTextUnrollProgress;
	mutable unsigned long           mLastRenderTicks;
	unsigned long                   mTextUnrollStartTick;

	std::string * mTextUnrollSound;
	int                             mMaximumCharacters;

	UIString mComposition;	// todo: use a pointer.
	unsigned char * mCompositionAttrs;

	bool mMouseDown : 1;
	bool mConsumeNextCharacter : 1;
	bool mDropShadow : 1;
	bool mDropShadowAutoDetect : 1;
};

//-----------------------------------------------------------------

inline const UIColor & UIText::GetTextColor () const
{
	return mTextColor;
}

//-----------------------------------------------------------------

inline void UIText::ResetLocalizedStrings ()
{
	SetText (mData);
}

//-----------------------------------------------------------------

inline const UIRect & UIText::GetMargin () const
{
	return mMargin;
}

//----------------------------------------------------------------------
/**
* Prelocalized UIText traffics only in the LocalText fields, any calls to SetText () have no effect,
* and calls to GetText () return an empty string.
*/
inline bool UIText::IsPreLocalized () const
{
	return HasTextFlag (TF_preLocalized);
}

//----------------------------------------------------------------------

inline long UIText::GetMaxLines () const
{
	return mMaxLines;
}

//----------------------------------------------------------------------

inline bool UIText::IsTruncateElipsis () const
{
	return HasTextFlag (TF_truncateElipsis);
}

//-----------------------------------------------------------------

inline UITextStyle             *UIText::GetTextStyle                () const
{
	return mStyle;
};

//-----------------------------------------------------------------

inline const UIString &         UIText::GetText                     () const
{
	return mData;
}

//-----------------------------------------------------------------

inline const UIString &         UIText::GetLocalText                () const
{
	return mLocalText;
}

//-----------------------------------------------------------------

inline bool                     UIText::IsEmpty                     () const
{
	return mData.empty();
};

//-----------------------------------------------------------------

inline int                      UIText::GetDataLength               () const
{
	return static_cast<int>(mData.size());
};

//----------------------------------------------------------------------

inline int UIText::GetCursorMark  () const
{
	return mCursorMark;
}

//----------------------------------------------------------------------

inline int UIText::GetCursorPoint () const
{
	return mCursorPoint;
}

//----------------------------------------------------------------------

inline bool UIText::IsEditable       () const
{
	return HasTextFlag (TF_editable);
}

//----------------------------------------------------------------------

inline bool UIText::HasTextFlag (TextFlag flag) const
{
	return (mTextFlags & static_cast<int>(flag)) != 0;
}

//----------------------------------------------------------------------

inline bool UIText::IsTextActuallySelectable () const
{
	return HasTextFlag (TF_textSelectable) || IsEditable ();
}

//----------------------------------------------------------------------

inline bool UIText::GetTextCapital () const
{
	return HasTextFlag (TF_textCapital);
}

//----------------------------------------------------------------------

inline bool UIText::GetTextUnroll  () const
{
	return HasTextFlag (TF_textUnroll);
}

//----------------------------------------------------------------------

inline float UIText::GetTextUnrollSpeed () const
{
	return mTextUnrollSpeed;
}

//----------------------------------------------------------------------

inline int UIText::GetTextUnrollProgress () const
{
	return mTextUnrollProgress;
}

//----------------------------------------------------------------------

inline bool UIText::GetTextUnrollOnce     () const
{
	return HasTextFlag (TF_textUnrollOnce);
}

//-----------------------------------------------------------------

inline bool UIText::GetIMEEnabled () const
{
	return HasTextFlag (TF_IME);
}

//-----------------------------------------------------------------

inline bool  UIText::IsEditableUnicode   () const
{
	return (static_cast<int>(mTextFlags) & TF_EditableUnicode) != 0;
}

//-----------------------------------------------------------------

inline void UIText::SetDropShadow(bool dropShadow)
{
	mDropShadow = dropShadow;
}

//-----------------------------------------------------------------

inline bool UIText::GetDropShadow() const
{
	return mDropShadow;
}

//-----------------------------------------------------------------

inline void UIText::SetDropShadowAutoDetect(bool dropShadow)
{
	mDropShadowAutoDetect = dropShadow;
}

//-----------------------------------------------------------------

inline bool UIText::GetDropShadowAutoDetect() const
{
	return mDropShadowAutoDetect;
}

#endif // __UITEXT_H__