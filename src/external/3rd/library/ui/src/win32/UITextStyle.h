#ifndef __UITEXTSTYLE_H__
#define __UITEXTSTYLE_H__

#include "UIStyle.h"

class UIFontCharacter;
class UIRectangleStyle;
class UITextStyleWrappedText;

//----------------------------------------------------------------------

class UITextStyle : public UIStyle
{
public:

	typedef ui_stdunordered_map<long, UIFontCharacter *>::fwd      GlyphMap_t;
	typedef ui_stdvector<UIString::const_iterator>::fwd       UIStringConstIteratorVector;

	typedef UITextStyleWrappedText WrappedText;

	static const char         *TypeName;//lint !e1516 // data member hides inherited member

	class PropertyName
	{//lint !e578 // symbol hides symbol
	public:
		static const UILowerString Leading;
		static const UILowerString DropShadowDepth;
		static const UILowerString DropShadowsEnabled;
	};

	enum MeasureMethod
	{
		UseLastCharWidth,
		UseLastCharAdvance
	};

	enum Alignment
	{
		Left,
		Center,
		Right
	};

	// Kinsoku-Shori Japanese line break rule says that a line cannot end with any of the following characters
	static const unsigned short KinsokuDontBreakChars[];
	// Kinsoku-Shori Japanese line break rule says that a line cannot begin with any of the follow characters
	static const unsigned short KinsokuDontLetStart[];

	struct AlignmentNames
	{
		static const std::string Left;
		static const std::string Center;
		static const std::string Right;
	};

	                       UITextStyle();
	virtual               ~UITextStyle();

	virtual bool           IsA( const UITypeID ) const;
	virtual const char    *GetTypeName( void ) const;
	virtual UIBaseObject  *Clone( void ) const;

	virtual void           GetPropertyGroups(UIPropertyGroupVector &, UIPropertyCategories::Category = UIPropertyCategories::C_ALL_CATEGORIES) const;
	virtual void           GetPropertyNames( UIPropertyNameVector &, bool forCopy ) const;

	virtual bool           SetProperty( const UILowerString & Name, const UIString &Value );
	virtual bool           GetProperty( const UILowerString & Name, UIString &Value ) const;
	
	virtual bool           AddChild( UIBaseObject * );
	virtual bool           RemoveChild( UIBaseObject * );
	virtual UIBaseObject  *GetChild( const char * ) const;
	virtual void           GetChildren( UIObjectList & ) const;
	virtual unsigned long  GetChildCount( void ) const;

	//-- wide string methods

	typedef ui_stdvector<long>::fwd LongVector;

	        void           MeasureText( const UIString &, long &width, long &height, MeasureMethod = UseLastCharAdvance, 
	                                    bool ProcessEscapeCharacters = true ) const;
	        void           MeasureText( const UIString &, UISize &, MeasureMethod = UseLastCharAdvance, 
	                                    bool ProcessEscapeCharacters = true ) const;

	        void           MeasureWrappedText( const UIString &, const long WrapWidth, long &width, long &height, 
	                                           MeasureMethod = UseLastCharAdvance, bool ProcessEscapeCharacters = true ) const;
	        void           MeasureWrappedText( const UIString &, const long WrapWidth, UISize &theSize,
	                                           MeasureMethod = UseLastCharAdvance, bool ProcessEscapeCharacters = true ) const;

	        void           GetWrappedTextInfo( const UIString &, int maxLines, long WrapWidth, long &width, long &height, UIStringConstIteratorVector *, 
	                                           LongVector *, MeasureMethod = UseLastCharAdvance, bool ProcessEscapeCharacters = true,
											   bool ignoreLocaleForWrapping = false) const;
	        void           GetWrappedTextInfo( const UIString &, int maxLines, long WrapWidth, UISize &theSize, UIStringConstIteratorVector *,
	                                           LongVector *, MeasureMethod = UseLastCharAdvance, bool ProcessEscapeCharacters = true,
											   bool ignoreLocaleForWrapping = false ) const;

	        void           RenderText( Alignment, const UIString &, UICanvas &, const UIPoint &, const long * const WrapWidth = 0, long maxLines = -1, bool truncateElipsis = false,
				                               bool ignoreLocaleForWrapping = false ) const;
	        void           RenderPrewrappedText( Alignment, const UIStringConstIteratorVector &, 
												const LongVector &, UICanvas &, const UIPoint &, const UISize &, 
												long maxLines = -1, bool truncateElipsis = false, int numChars = -1,
												UIRectangleStyle const * const textMaskStyle = NULL) const;
	        void           RenderPrewrappedSelection( Alignment, const UIStringConstIteratorVector &,
	                                                    const LongVector &, UICanvas &, const UIPoint &, const UISize &, long maxLines,
														int mark, int point) const;

	class RenderLineData
	{
	public:
		UICanvas *               canvas;
		Alignment                align;
		UIString::const_iterator begin;
		UIString::const_iterator nextLine;
		UIPoint                  loc;
		long                     width;
		long                     lineWidth;
		bool                     truncateElipsis;
		int                      mark;
		int                      point;
		bool                     selection;
		int                      numCharsToRender;
		UIColor                  defaultColor;
		mutable long             currentIndentation;
		
		mutable bool             inEscapeSequence;
		int *                    findCol;
		mutable int              numCharsRendered;
		bool                     ignoreColors;

		RenderLineData () :
		canvas           (0),
		align            (Left),
		begin            (),
		nextLine         (),
		loc              (),
		width            (0),
		lineWidth        (0),
		truncateElipsis  (true),
		mark             (0),
		point            (0),
		selection        (false),
		numCharsToRender (0),
		defaultColor     (),
		currentIndentation (0),
		inEscapeSequence (false),
		findCol          (0),
		numCharsRendered (0),
		ignoreColors     (false)
		{
		}

		RenderLineData (UICanvas * _canvas, Alignment _align, const UIPoint & _loc, long _width, int _mark, int _point, const UIColor & _defaultColor) :
		canvas           (_canvas),
		align            (_align),
		begin            (),
		nextLine         (),
		loc              (_loc),
		width            (_width),
		lineWidth        (0),
		truncateElipsis  (true),
		mark             (_mark),
		point            (_point),
		selection        (false),
		numCharsToRender (0),
		currentIndentation (0),
		defaultColor     (_defaultColor),
		inEscapeSequence (false),
		findCol          (0),
		numCharsRendered (0),
		ignoreColors     (false)
		{
		}

	};

	long RenderPrewrappedLine(RenderLineData const & rld, UIRectangleStyle const * const textMaskStyle = NULL) const;

	int                  FindCharacterFromLocation ( Alignment align, const UIStringConstIteratorVector & LinePointers,
	                                                    const LongVector & LineWidths, const UIPoint & loc, const UISize & size, long maxLines, const UIPoint & pt) const;

	const UIPoint        FindLocationFromCharacter ( Alignment align, const UIStringConstIteratorVector & LinePointers,
	                                                    const LongVector & LineWidths, const UIPoint & loc, const UISize & size, int index) const;


	void SetAnimationState(long const index);
	long GetAnimationState() const;

	virtual void CopyPropertiesFrom(const UIBaseObject & rhs);

	void SetDropShadowDepth(const UIPoint & offset);
	UIPoint const & GetDropShadowDepth() const;

	void SetDropShadowsEnabled(bool bDrop);
	bool GetDropShadowsEnabled() const;

	//-- utility
	        void           MeasureCharacter( long code, long &width, long &height ) const;
	        void           MeasureCharacter( long code, UISize & ) const;

	        UIFontCharacter *GetCharacter( long code ) const;

	        void           SetLeading( const long );
	        long           GetLeading( void ) const;



	static bool            ParseAlignment (const Unicode::String & Value, Alignment & align);
	static void            FormatAlignment (Unicode::String & Value, const Alignment tav);

	static void            setEnabled (bool b);

	static void            setOpacityRelativeMin (float f);
	static float           getOpacityRelativeMin ();

	static bool            GetGlobalDropShadowEnabled ();
	static void            SetGlobalDropShadowEnabled (bool b);

	static UIPoint const & GetGlobalDropShadowDepth();
	static void SetGlobalDropShadowDepth(const UIPoint & drop);

	static bool GetGlobalAutoDropShadowEnabled();
	static void SetGlobalAutoDropShadowEnabled(bool b);

	const UIString &       GetLogicalName() const;
	void                   SetLogicalName(const UIString &logicalName);

	UIString::const_iterator EatEscapeSequence( const UIString::const_iterator & begin, const UIString::const_iterator & end, bool & ignoreNextEscape, long & pos, long & indentation) const;
	
private:

	UITextStyle & operator=     (const UITextStyle &);
	              UITextStyle   (const UITextStyle &);

	UIString::const_iterator ParseColorEscapeSequence( const UIString::const_iterator & begin, const UIString::const_iterator & end, const UIColor & defaultColor, UIColor &CurrentColor) const;

	enum { GLYPH_ARRAY_SIZE = 256 };

	UIFontCharacter        *mGlyphArray[GLYPH_ARRAY_SIZE];

	GlyphMap_t *            mGlyphMap;

	long               mLeading;
	float              mOpacity;
	bool mDropShadowEnabled;
	UIPoint *  mDropShadowDepth;

	static bool        ms_enabled;
	static float       ms_opacityRelativeMin;
	static bool        ms_dropShadowEnabled;
	static UIPoint     ms_dropShadowDepth;
	static bool ms_autoDropShadow;

	UIString           mLogicalName;
};

//----------------------------------------------------------------------

inline float  UITextStyle::getOpacityRelativeMin ()
{
	return ms_opacityRelativeMin;
}


//----------------------------------------------------------------------

inline bool UITextStyle::GetGlobalDropShadowEnabled()
{
	return ms_dropShadowEnabled;
}

//----------------------------------------------------------------------

inline const UIString & UITextStyle::GetLogicalName() const
{
	return mLogicalName;
}

//----------------------------------------------------------------------

inline void UITextStyle::SetLogicalName(const UIString& logicalName)
{
	mLogicalName = logicalName;
}

//----------------------------------------------------------------------

inline void UITextStyle::SetAnimationState(long const /*index*/)
{
}

//----------------------------------------------------------------------

inline long UITextStyle::GetAnimationState() const
{
	return 0L;
}

//----------------------------------------------------------------------

inline void UITextStyle::SetGlobalDropShadowEnabled(bool b)
{
	ms_dropShadowEnabled = b;
}

//----------------------------------------------------------------------

inline UIPoint const & UITextStyle::GetGlobalDropShadowDepth()
{
	return ms_dropShadowDepth;
}

//----------------------------------------------------------------------

inline void UITextStyle::SetGlobalDropShadowDepth(const UIPoint & drop)
{
	ms_dropShadowDepth = drop;
}

//----------------------------------------------------------------------

inline void UITextStyle::SetDropShadowsEnabled(bool bDrop)
{
	mDropShadowEnabled = bDrop;
}

//----------------------------------------------------------------------

inline bool UITextStyle::GetDropShadowsEnabled() const
{
	return mDropShadowEnabled;
}

//----------------------------------------------------------------------

inline bool UITextStyle::GetGlobalAutoDropShadowEnabled()
{
	return ms_autoDropShadow;
}

//----------------------------------------------------------------------

inline void UITextStyle::SetGlobalAutoDropShadowEnabled(bool b)
{
	ms_autoDropShadow = b;
}

//-----------------------------------------------------------------

#endif // __UITEXTSTYLE_H__
