#ifndef __UITEXTBOX_H__
#define __UITEXTBOX_H__

#include "UIWidget.h"

#include <vector>

class UITextboxStyle;
class UIIMEComposition;
class UIListbox;

class UITextbox : public UIWidget
{
public:
	
	typedef std::vector<UIString::const_iterator>       UIStringConstIteratorVector;
	
	enum
	{
		NoWrapping        = -1,
		AutomaticWrapping = -2,
	};
	
	class PropertyName
	{
	public:
		static const UILowerString MaxLength;
		static const UILowerString MaxLines;
		static const UILowerString OnChange;
		static const UILowerString OnOverflow;
		static const UILowerString Password;
		static const UILowerString Style;
		static const UILowerString Text;
		static const UILowerString LocalText;
		static const UILowerString WrapWidth;
		static const UILowerString Editable;
		static const UILowerString TextColor;
		static const UILowerString NumericInteger;
		static const UILowerString NumericFloat;
		static const UILowerString NumericNegative;
		static const UILowerString NumericValue;
		static const UILowerString EditableUnicode;
		static const UILowerString IME;
	};
	
	class MethodName
	{
	public:
		static const UILowerString IsEmpty;
	};
	
	static const char * const TypeName;
	static const UIString::value_type  PasswordCharacter;
	
	UITextbox();
	virtual                 ~UITextbox();
	
	virtual bool             IsA                    ( const UITypeID ) const;
	virtual const char     * GetTypeName            () const;
	virtual UIBaseObject   * Clone                  () const;
	
	virtual void             GetPropertyGroups      (UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const;
	virtual void             GetPropertyNames       ( UIPropertyNameVector &, bool forCopy ) const;
	virtual void             GetLinkPropertyNames   ( UIPropertyNameVector &In ) const;

	virtual bool             SetProperty            ( const UILowerString & Name, const UIString &Value );
	virtual bool             GetProperty            ( const UILowerString & Name, UIString &Value ) const;
	
	virtual  void            SetSize                ( const UISize & );
	
	virtual const UICursor * GetMouseCursor         () const;
	virtual UICursor       * GetMouseCursor         ();
	
	virtual void             GetScrollExtent        ( UISize & ) const;
	virtual void             GetScrollSizes         ( UISize &PageSize, UISize &LineSize ) const;
	
	virtual void             SetSelected            ( const bool );        
	virtual void             SelectAll              ();

	virtual bool             ProcessMessage         ( const UIMessage & );
	virtual void             Render                 ( UICanvas & ) const;
	
	void                     SetStyle               ( UITextboxStyle * );  
	UITextboxStyle *         GetTextboxStyle        () const { return mStyle; };
	virtual  UIStyle        *GetStyle               () const;
	
	void                     SetText                ( const UIString & );
	void                     GetText                ( UIString & ) const;
	const UIString &         GetText                () const { return mText; }
	
	void                     SetLocalText           ( const UIString & );
	void                     GetLocalText           ( UIString & ) const;
	const UIString &         GetLocalText           () const { return mLocalText; }
	
	void                     ResetCarat             ();
	
	void                     SetWrapWidth           ( const long );
	long                     GetWrapWidth           () const;
	
	void                     MoveCaratToStartOfLine ();
	void                     MoveCaratToEndOfLine   ();
	long                     GetCaratIndex          () const;
	
	void                     MoveCaratRightOneWord  ();
	void                     MoveCaratRight         ();
	void                     MoveCaratLeft          ();
	void                     MoveCaratLeftOneWord   ();

	const UIPoint            GetCaratPos            () const;

	virtual void             ResetLocalizedStrings  ();

	void                     SetEditable            (bool b);
	bool                     GetEditable            () const;

	void                     SetIMEEnabled          (bool b);
	bool                     GetIMEEnabled          () const;

	bool                     ClearSelection         ();

	const UIColor &          GetTextColor           () const;
	void                     SetTextColor           (const UIColor & color);
	void                     SetTextColorOverride   (bool b);

	void                     SetNumericInteger      (bool b);
	void                     SetNumericFloat        (bool b);
	void                     SetNumericNegative     (bool b);
	void                     SetEditableUnicode     (bool b);

	void                     SetMaxIntegerLength    (unsigned char maxIntegerLength);

	bool                     IsNumericInteger       () const;
	bool                     IsNumericFloat         () const;
	bool                     IsNumericNegative      () const;
	bool                     IsEditableUnicode      () const;

	int                      GetNumericIntegerValue () const;
	float                    GetNumericFloatValue   () const;

	void                     Ding                   () const;

	bool                     performDeleteKey       (bool doHandleTextChanged, bool isShiftDown);
	bool                     performBackspaceKey    (bool doHandleTextChanged, bool isShiftDown);

	void                     handleTextChanged      (size_t OldCaratIndex, bool TextChanged, bool isShiftDown);

	void                     SetKeyClickOn          (bool b);

	bool                     CanSelect              () const;

	void                     SetMaximumCharacters  (int const maximumCharacters);
	int                      GetMaximumCharacters  () const;
	bool                     IsAtMaximumCharacters () const;
	int                      GetCharacterCount     () const;

	// Tell the UI to consume the next character sent to the TextBox
	void										 ConsumeNextCharacter();

private:
	
	UITextbox( UITextbox & );
	UITextbox               &operator = ( UITextbox & );
	
	enum TextboxAttributeBitFlags
	{
		TABF_DrawCarat         = 0x0001,
		TABF_Password          = 0x0002,
		TABF_Editable          = 0x0004,
		TABF_TextColorOverride = 0x0008,
		TABF_NumericInteger    = 0x0010,
		TABF_NumericFloat      = 0x0020,
		TABF_EditableUnicode   = 0x0040,
		TABF_NumericNegative   = 0x0080,
		TABF_KeyClickOn        = 0x0100,
		TABF_IME							 = 0x0200
	};
	
	long                GetRowFromOffset( long OffsetFromBase ) const;
	long                GetXCoordFromRowAndOffset( long row, long OffsetFromBase ) const;
	
	void                MoveCaratUp();
	void                MoveCaratDown();          
	void                MoveCaratVertically( long MovementDirection );
	void                MoveCaratToStartOfDocument();
	void                MoveCaratToEndOfDocument();
	long                GetCaratOffsetFromPoint( const UIPoint & );
	
	void                MoveCaratToPoint( const UIPoint & );
	
	long                GetCaratLine();
	void                RenderCarat( UICanvas & ) const;
	void                CalculateCaratRect();
	void                ScrollCaratOnScreen();
	
	void                CacheTextMeasurements();
	
	void                ModifySelection( long NewSelectionFloatingEnd );
	void                SelectCurrentWord ();
	void                DeleteSelection          (bool dingOnFailure);
	void                CopySelectionToClipboard();
	void                PasteFromClipboard();

	void                ValidateText ();
	
	UITextboxStyle             *mStyle;
	UIString                    mText;
	UIString                    mLocalText;
	UIString                    mRenderData;
	long                        mCaratIndex;        // Index of the character that the carat preceeds
	UIRect                      mCaratRect;         // Location of the carat in local coordinates  
	UIPoint                     mCaratPos;          // Position of the logical carat, the visible carat can be moved to where the composition string ends (mCaratRect)
	long                        mSelectionStart;    // Index of the first character in the selection
	long                        mSelectionEnd;      // Index of the first character that follows the selection
	long                        mSelectionFixedEnd;  // One of mSelectedStart, mSelectionEnd, whichever is fixed
	//   this is used when changing the selection with the mouse
	//   or cursor, only the free end moves after the selection
	//   has begun.
	long                        mWrapWidth;  
	UITime                      mNextCaratBlinkTime;  
	unsigned short              mTextboxAttributeBits;  
	long                        mMaxLines;
	long                        mMaxLength;

	unsigned char               mMaxIntegerLength;

	mutable bool                        mTextDirty;
	mutable UIStringConstIteratorVector mLinePointers;
	mutable std::vector<long>           mLineWidths;
	mutable UIStringConstIteratorVector mRenderLinePointers;
	mutable std::vector<long>           mRenderLineWidths;

	UIColor                         mTextColor;

	unsigned int                    mFlags;
	int                             mMaximumCharacters;

	bool														mConsumeNextCharacter;

	UIString												mComposition;
	unsigned char *                                         mCompositionAttrs;
};

//-----------------------------------------------------------------

inline long UITextbox::GetCaratIndex() const
{
	return mCaratIndex;
}

//-----------------------------------------------------------------

inline void UITextbox::ResetLocalizedStrings ()
{
	SetText (mText);
}

//-----------------------------------------------------------------

inline bool UITextbox::GetEditable () const
{
	return (mTextboxAttributeBits & TABF_Editable) != 0;
}

//----------------------------------------------------------------------

inline const UIColor & UITextbox::GetTextColor () const
{
	return mTextColor;
}

//-----------------------------------------------------------------

inline bool  UITextbox::IsNumericInteger () const
{
	return (mTextboxAttributeBits & TABF_NumericInteger) != 0;
}

//-----------------------------------------------------------------

inline bool  UITextbox::IsNumericFloat   () const
{
	return (mTextboxAttributeBits & TABF_NumericFloat) != 0;
}

//-----------------------------------------------------------------

inline bool  UITextbox::IsNumericNegative   () const
{
	return (mTextboxAttributeBits & TABF_NumericNegative) != 0;
}

//-----------------------------------------------------------------

inline bool  UITextbox::IsEditableUnicode   () const
{
	return (mTextboxAttributeBits & TABF_EditableUnicode) != 0;
}

//-----------------------------------------------------------------

inline bool UITextbox::GetIMEEnabled () const
{
	return (mTextboxAttributeBits & TABF_IME) != 0;
}

//----------------------------------------------------------------------

#endif // __UITEXTBOX_H__