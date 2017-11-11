#include "_precompile.h"
#include "UIText.h"

#include "UICanvas.h"
#include "UIClipboard.h"
#include "UIClock.h"
#include "UIData.h"
#include "UIDataSource.h"
#include "UIIMEManager.h"
#include "UIListbox.h"
#include "UIManager.h"
#include "UIMessage.h"
#include "UIPalette.h"
#include "UIPropertyDescriptor.h"
#include "UISystemDependancies.h"
#include "UITextStyle.h"
#include "UITextStyleManager.h"
#include "UITextStyleWrappedText.h"
#include "UIUtils.h"
#include "UIWidgetRectangleStyles.h"
#include "UnicodeUtils.h"

#include <cassert>
#include <vector>

//----------------------------------------------------------------------


//----------------------------------------------------------------------
const char * const UIText::TypeName                       = "Text";

const UILowerString UIText::PropertyName::ColorCarat              = UILowerString ("ColorCarat");
const UILowerString UIText::PropertyName::ColorSelection          = UILowerString ("ColorSelection");
const UILowerString UIText::PropertyName::CursorMark              = UILowerString ("CursorMark");
const UILowerString UIText::PropertyName::CursorPoint             = UILowerString ("CursorPoint");
const UILowerString UIText::PropertyName::Editable                = UILowerString ("Editable");
const UILowerString UIText::PropertyName::EditableUnicode         = UILowerString ("EditableUnicode");
const UILowerString UIText::PropertyName::LocalText               = UILowerString ("LocalText");
const UILowerString UIText::PropertyName::Margin                  = UILowerString ("Margin");
const UILowerString UIText::PropertyName::MaxLines                = UILowerString ("MaxLines");
const UILowerString UIText::PropertyName::OpacityCarat            = UILowerString ("OpacityCarat");
const UILowerString UIText::PropertyName::OpacitySelection        = UILowerString ("OpacitySelection");
const UILowerString UIText::PropertyName::PreLocalized            = UILowerString ("PreLocalized");
const UILowerString UIText::PropertyName::Style                   = UILowerString ("Font");
const UILowerString UIText::PropertyName::Text                    = UILowerString ("Text");
const UILowerString UIText::PropertyName::TextAlignment           = UILowerString ("TextAlignment");
const UILowerString UIText::PropertyName::TextAlignmentVertical   = UILowerString ("TextAlignmentVertical");
const UILowerString UIText::PropertyName::TextColor               = UILowerString ("TextColor");
const UILowerString UIText::PropertyName::TextCapital             = UILowerString ("TextCapital");
const UILowerString UIText::PropertyName::TextSelectable          = UILowerString ("TextSelectable");
const UILowerString UIText::PropertyName::TextUnroll              = UILowerString ("TextUnroll");
const UILowerString UIText::PropertyName::TextUnrollOnce          = UILowerString ("TextUnrollOnce");
const UILowerString UIText::PropertyName::TextUnrollSpeed         = UILowerString ("TextUnrollSpeed");
const UILowerString UIText::PropertyName::TextUnrollSound         = UILowerString ("TextUnrollSound");

const UILowerString UIText::PropertyName::TruncateElipsis         = UILowerString ("TruncateElipsis");
const UILowerString UIText::PropertyName::IME					  = UILowerString ("IME");
const UILowerString UIText::PropertyName::DropShadow = UILowerString ("DropShadow");
const UILowerString UIText::PropertyName::DropShadowAutoDetect = UILowerString ("DropShadowAutoDetect");

const std::string UIText::TextAlignmentVerticalNames::Top    = "Top";
const std::string UIText::TextAlignmentVerticalNames::Center = "Center";
const std::string UIText::TextAlignmentVerticalNames::Bottom = "Bottom";

//-----------------------------------------------------------------
#define _TYPENAME UIText

namespace UITextNamespace
{
	void unfubarMicrosoftInvalidTextCharacters (Unicode::String & str)
	{
		typedef std::pair <Unicode::unicode_char_t, Unicode::unicode_char_t> FubarCharacterInfo;
		typedef std::pair <Unicode::unicode_char_t, Unicode::String>         FubarStringInfo;

		static const FubarCharacterInfo FubarCharactersCourtesyMicrosoft [] =
		{
			FubarCharacterInfo (139,   '<'),
			FubarCharacterInfo (155,   '>'),

			FubarCharacterInfo (8216,   '\''),
			FubarCharacterInfo (8217,   '\''),
			FubarCharacterInfo (8221,   '"'),
			FubarCharacterInfo (8220,   '"'),
			FubarCharacterInfo (149,   '-'),
			FubarCharacterInfo (150,   '-'),
			FubarCharacterInfo (151,   '-'),
			FubarCharacterInfo (152,   '~')
		};
		
		static const int FubarCharactersCourtesyMicrosoftLength = sizeof (FubarCharactersCourtesyMicrosoft) / sizeof (FubarCharactersCourtesyMicrosoft[0]);
		
		static const FubarStringInfo FubarStringsCourtesyMicrosoft [] =
		{
			FubarStringInfo (133,   Unicode::narrowToWide ("...")),
				//-- carriage return
			FubarStringInfo (13,    Unicode::narrowToWide (""))
		};
		
		static const int FubarStringsCourtesyMicrosoftLength = sizeof (FubarStringsCourtesyMicrosoft) / sizeof (FubarStringsCourtesyMicrosoft[0]);
		
		{
			for (int i = 0; i < FubarCharactersCourtesyMicrosoftLength; ++i)
			{
				const Unicode::unicode_char_t correctCharacter = FubarCharactersCourtesyMicrosoft [i].second;
				const Unicode::unicode_char_t fubarCharacter   = FubarCharactersCourtesyMicrosoft [i].first;
				
				size_t pos = 0;
				while ((pos = str.find (fubarCharacter, pos)) != Unicode::String::npos)
				{
					str [pos] = correctCharacter;
					++pos;
				}
			}
		}
		
		{
			for (int i = 0; i < FubarStringsCourtesyMicrosoftLength; ++i)
			{
				const Unicode::unicode_char_t fubarCharacter   = FubarStringsCourtesyMicrosoft [i].first;
				const Unicode::String & correctString          = FubarStringsCourtesyMicrosoft [i].second;
				
				size_t pos = 0;
				
				while ((pos = str.find (fubarCharacter, pos)) != Unicode::String::npos)
				{
					str.replace (pos, 1, correctString);
					pos += correctString.size ();
				}
			}
		}
	}

	std::string const s_emptyString;

	//================================================================
	// Basic category.
	_GROUPBEGIN(Basic)
		_DESCRIPTOR(CursorMark, "", T_int),
		_DESCRIPTOR(CursorPoint, "", T_int),
		_DESCRIPTOR(Editable, "", T_bool),
		_DESCRIPTOR(EditableUnicode, "", T_bool),
		_DESCRIPTOR(IME, "", T_bool),
		_DESCRIPTOR(LocalText, "", T_string),
		_DESCRIPTOR(Margin, "", T_rect),
		_DESCRIPTOR(MaxLines, "", T_int),
		_DESCRIPTOR(PreLocalized, "", T_bool),
		_DESCRIPTOR(Style, "", T_object),
		_DESCRIPTOR(Text, "", T_string),
		_DESCRIPTOR(TextSelectable, "", T_bool),
	_GROUPEND(Basic, 2, int(UIPropertyCategories::C_Basic));
	//================================================================

	//================================================================
	// Appearance category.
	_GROUPBEGIN(Appearance)
		_DESCRIPTOR(ColorCarat, "", T_color),
		_DESCRIPTOR(ColorSelection, "", T_color),
		_DESCRIPTOR(OpacityCarat, "", T_float),
		_DESCRIPTOR(OpacitySelection, "", T_float),
		_DESCRIPTOR(TextColor, "", T_color),
		_DESCRIPTOR(DropShadow, "", T_bool),
		_DESCRIPTOR(DropShadowAutoDetect, "", T_bool),
	_GROUPEND(Appearance, 2, int(UIPropertyCategories::C_Appearance));
	//================================================================

	//================================================================
	// Behavior category.
	_GROUPBEGIN(Behavior)
		_DESCRIPTOR(TextAlignment, "", T_string), // ENUM
		_DESCRIPTOR(TextAlignmentVertical, "", T_string), // ENUM
		_DESCRIPTOR(TextCapital, "", T_bool),
		_DESCRIPTOR(TextUnroll, "", T_bool),
		_DESCRIPTOR(TextUnrollOnce, "", T_bool),
		_DESCRIPTOR(TextUnrollSpeed, "", T_float),
		_DESCRIPTOR(TextUnrollSound, "", T_string),
		_DESCRIPTOR(TruncateElipsis, "", T_bool),
	_GROUPEND(Behavior, 2, int(UIPropertyCategories::C_Behavior));
	//================================================================
}

using namespace UITextNamespace;

//-----------------------------------------------------------------

UIText::UIText             () :
UIWidget(),
mStyle                     (0),
mData                      (),
mLocalText                 (),
mRenderData                (),
mLines                     (new UITextStyleWrappedText),
mTextAlignment             (UITextStyle::Left),
mMargin                    (0,0,0,0),
mTextExtent                (0,0),
mTextColor                 (UIColor::white),
mColorCarat                (0xFF,0xFF,0x00,0xaa),
mColorSelection            (0xAA,0x88,0x55,0x88),
mMaxLines                  (-1),
mTextAlignmentVertical     (TAV_top),
mCursorMark                (0),
mCursorPoint               (0),
mTextFlags                 (TF_dirty | TF_truncateElipsis | TF_textSelectable),
mNextCaratBlinkTime        (0),
mOldScrollLocationToBottom (),
mTextUnrollSpeed           (180.0f),
mTextUnrollProgress        (0),
mLastRenderTicks           (0),
mTextUnrollStartTick       (0),
mTextUnrollSound(NULL),
mMaximumCharacters(1024 * 1024 * 16),
mMouseDown(false),
mConsumeNextCharacter(false),
mDropShadow(true),
mDropShadowAutoDetect(true),
mComposition(),
mCompositionAttrs(NULL)
{
	SetGetsInput (false);
}

//-----------------------------------------------------------------

UIText::~UIText ()
{
	SetStyle(0);

	delete mLines;
	mLines = 0;

	delete mTextUnrollSound;
	mTextUnrollSound = NULL;
}

//-----------------------------------------------------------------

bool UIText::IsA( const UITypeID QueriedType ) const
{
	return QueriedType == TUIText || UIWidget::IsA( QueriedType );
}

//-----------------------------------------------------------------

const char *UIText::GetTypeName () const
{
	return TypeName;
}

//-----------------------------------------------------------------

UIBaseObject *UIText::Clone () const
{
	return new UIText;
}

//-----------------------------------------------------------------
/*
void UIText::SetRect( const UIRect &NewRect )
{
	SetTextFlag (TF_dirty, true);
	UIWidget::SetRect( NewRect );
}
*/
//-----------------------------------------------------------------

void UIText::SetSize( const UISize &NewSize )
{
	if (!HasTextFlag (TF_dirty))
	{
		UISize scrollExtent;
		GetScrollExtent (scrollExtent);
		SetMinimumScrollExtent (scrollExtent);

		mOldScrollLocationToBottom = GetScrollLocation ();
		mOldScrollLocationToBottom.y += GetHeight ();
	}

	SetTextFlag (TF_dirty, true);
	UIWidget::SetSize( NewSize );

	SetMinimumScrollExtent (UISize::zero);
}

//-----------------------------------------------------------------
/*
void UIText::SetWidth( const long NewWidth )
{
	SetTextFlag (TF_dirty, true);
	UIWidget::SetWidth( NewWidth );
}
*/
//-----------------------------------------------------------------

void UIText::SetText( const UIString &NewString )
{
	//-- setting the unlocalized text of a prelocalized textbox has no effect
	if (IsPreLocalized ())
		return;

	if (&mData != &NewString)
		mData = NewString;

	UIManager::gUIManager ().CreateLocalizedString (mData, mLocalText);
	SetLocalText (mLocalText);
}

//-----------------------------------------------------------------

void UIText::GetText( UIString &OutString ) const
{
	OutString = mData;
}

//-----------------------------------------------------------------

void UIText::SetLocalText( const UIString &NewString )
{
	// rls - compare strings?
	if (IsEditable ())
	{
		mLocalText = NewString;
		Unicode::truncateToUTF8Size(mLocalText, static_cast<unsigned int>(GetMaximumCharacters()));
		// Cap the maximum size
		if (GetTextCapital())
		{
			mLocalText = Unicode::toUpper(mLocalText);
		}
	}
	else
	{
		if (GetTextCapital())
		{
			mLocalText = Unicode::toUpper(NewString);
		}
		else
		{
			mLocalText = NewString;
		}
	}

	unfubarMicrosoftInvalidTextCharacters (mLocalText);

	if (GetTextUnroll ())
		ResetTextUnroll ();

	SetTextFlag (TF_dirty, true);
}

//-----------------------------------------------------------------

void UIText::GetLocalText( UIString &OutString ) const
{
	OutString = mLocalText;
}

//-----------------------------------------------------------------

int UIText::GetLineCount () const
{
	if (HasTextFlag (TF_dirty))
		const_cast<UIText *>( this )->CacheTextMeasurements();

	if( mLines->linePointers.size() < 3 )
		return 0;

	return mLines->linePointers.size() - 2;
}

//-----------------------------------------------------------------

void UIText::RemoveLeadingLines( int LinesToRemove )
{
	if( LinesToRemove < 1 )
		return;

	if (HasTextFlag (TF_dirty))
		CacheTextMeasurements();

	const UISize marginSize (UISize (mMargin.left + mMargin.right, mMargin.top + mMargin.bottom ));

	if( mLines->linePointers.size() && ((int)mLines->linePointers.size() > (LinesToRemove - 2)) )
	{
		UIString LinesRemaining( mLines->linePointers[LinesToRemove] );

		mLocalText = LinesRemaining;	
		mLines->clear();
		SyncRenderDataToLocalText();
		mStyle->GetWrappedTextInfo( mRenderData, mMaxLines, GetWidth(), mTextExtent, &mLines->linePointers, &mLines->lineWidths);
		
		if( mStyle )
		{
			// DON_FIXME: ScrollExtent.x could be too large now...
			mTextExtent.y -= LinesToRemove * mStyle->GetLeading();

			SetScrollExtent( mTextExtent + marginSize);

			UISize ScrollExtent;
			GetScrollExtent (ScrollExtent);
			UIPoint ScrollLocation = GetScrollLocation();

			if( ScrollLocation.y + GetHeight() > ScrollExtent.y )
				ScrollLocation.y = ScrollExtent.y - GetHeight();

			SetScrollLocation( ScrollLocation );
		}
	}
	else
	{
		mLines->linePointers.clear();
		mLocalText.clear();
		mTextExtent = UISize (0,0);

		SetScrollExtent( mTextExtent + marginSize);
	}
}

//-----------------------------------------------------------------

void UIText::AppendLocalText( const UIString &StringToAppend )
{
	if (HasTextFlag (TF_dirty))
	{
		mLocalText.append (StringToAppend);
	}
	else
	{
		UIStringConstIteratorVector::iterator i;

		//@TODO this is crap.  this now assumes that all mLines->linePointers pointers are pointers into mRenderData,
		// which happens to be true right now.
		UIString::iterator OldBase = mRenderData.begin();
		mLocalText.append(StringToAppend);
		SyncRenderDataToLocalText();
		UIString::iterator NewBase = mRenderData.begin();

		// Rebase iterators
		if( OldBase != NewBase )
		{
			for( i = mLines->linePointers.begin(); i != mLines->linePointers.end(); ++i )
				*i = NewBase + (*i - OldBase);
		}

		const UISize marginSize (UISize (mMargin.left + mMargin.right, mMargin.top + mMargin.bottom ));

		if( mStyle )
		{
			if( mLines->linePointers.size() >= 2 )
			{
				// Append new line breaks
				UIStringConstIteratorVector	NewLineBreaks;
				std::vector<long>						NewLineWidths;
				UISize											ScrollExtentOfNewText;
				UIString										TextToLineWrap;
				UIString::const_iterator		TextToRewrap;

				TextToRewrap = mLines->linePointers[mLines->linePointers.size() - 2];
				TextToLineWrap.append( TextToRewrap );

				mStyle->GetWrappedTextInfo( TextToLineWrap, mMaxLines, GetWidth(), ScrollExtentOfNewText, &NewLineBreaks, &NewLineWidths );

				// Rebase iterators
				for( i = NewLineBreaks.begin(); i != NewLineBreaks.end(); ++i )
					*i = TextToRewrap + (*i - TextToLineWrap.begin());

				if( NewLineBreaks.size() )
				{
					mTextExtent.y -= mStyle->GetLeading();
					mTextExtent.y += ScrollExtentOfNewText.y;

					if( ScrollExtentOfNewText.x > mTextExtent.x )
						mTextExtent.x = ScrollExtentOfNewText.x;

					SetScrollExtent( mTextExtent + marginSize);

					mLines->linePointers.pop_back();
					mLines->linePointers.pop_back();

					mLines->lineWidths.pop_back();
					mLines->lineWidths.pop_back();

					mLines->linePointers.insert( mLines->linePointers.end(), NewLineBreaks.begin(), NewLineBreaks.end() );
					mLines->lineWidths.insert( mLines->lineWidths.end(), NewLineWidths.begin(), NewLineWidths.end() );
				}
			}
			else
			{
				mLines->clear();
				SyncRenderDataToLocalText();
				mStyle->GetWrappedTextInfo( mRenderData, mMaxLines, GetWidth(), mTextExtent, &mLines->linePointers, &mLines->lineWidths);

				SetScrollExtent( mTextExtent + marginSize);
			}
		}
	}
}

//-----------------------------------------------------------------

void UIText::SetStyle( UITextStyle *NewStyle )
{
	if( NewStyle )
		NewStyle->Attach( this );

	if( mStyle )
		mStyle->Detach( this );

	mStyle		 = NewStyle;
	SetTextFlag (TF_dirty, true);
}

//-----------------------------------------------------------------

UIStyle *UIText::GetStyle () const
{
	return mStyle;
};

//-----------------------------------------------------------------

bool UIText::CanSelect () const
{
	return IsSelectable () || IsEditable ();
}

//----------------------------------------------------------------------

void UIText::SetSelected( const bool NewSelected )
{
	UIWidget::SetSelected( NewSelected );

	if( NewSelected )
	{		
		if (GetIMEEnabled())
		{
			UIManager::gUIManager().getUIIMEManager()->SetContextWidget(this);
			UIManager::gUIManager().getUIIMEManager()->SetEnabled(true);		
		}
		else
		{			
			UIManager::gUIManager().getUIIMEManager()->SetEnabled(false);	
			UIManager::gUIManager().getUIIMEManager()->SetContextWidget(NULL);
		}
		mNextCaratBlinkTime = UIClock::gUIClock ().GetTime () + UISystemDependancies::Get().GetCaratBlinkTickCount();
		mComposition.clear();	
		mCompositionAttrs = NULL;
		SetTextFlag (TF_dirty, true);
		SetTextFlag (TF_drawCarat, true);
	}
	else
	{
		mComposition.clear();
		mCompositionAttrs = NULL;
		SetTextFlag (TF_dirty, true);
	}

}

//-----------------------------------------------------------------

bool UIText::ProcessMessage( const UIMessage &msg )
{
	if (UIWidget::ProcessMessage( msg ))
		return true;

	if (!mStyle)
		return false;

	//----------------------------------------------------------------------

	if (msg.Type == UIMessage::LeftMouseDown)
	{
		if (!IsSelected ())
			SetSelected (true);

		const int ch = GetCharacterFromLocation (msg.MouseCoords + GetScrollLocation ());
		SetCursorMark  (ch);
		SetCursorPoint (ch);

		mMouseDown   = true;
		return true;
	}

	else if (msg.Type == UIMessage::LeftMouseUp)
	{
		mMouseDown = false;
		return false;
	}

	//----------------------------------------------------------------------

	else if (msg.Type == UIMessage::MouseMove)
	{
		if (mMouseDown && msg.Modifiers.LeftMouseDown)
		{
			SetCursorPoint (GetCharacterFromLocation (msg.MouseCoords + GetScrollLocation ()));
			return true;
		}
	}

	//----------------------------------------------------------------------
	else if (msg.Type == UIMessage::IMEComposition)
	{
		if (!GetIMEEnabled())
		{
			return true;
		}

		mComposition = UIManager::gUIManager().getUIIMEManager()->GetCompString();
		mCompositionAttrs = UIManager::gUIManager().getUIIMEManager()->GetCompStringAttrs();

		// Remap half-width characters into ascii
		for (unsigned int i = 0; i < mComposition.length(); i++)
		{
			int ch = mComposition[i];

			// Remap half-width ascii characters to normal ascii characters so the composition window will
			// show composition strings correctly
			if (ch >= 0xFF41 && ch <= 0xFF5a)
			{
				mComposition[i] = (unsigned short) (mComposition[i] - 0xFEE0);
			}
		}

		SetTextFlag (TF_dirty, true);

		return true;
	}
	else if (msg.Type == UIMessage::IMEChangeCandidate)
	{
		return true;
	}
	else if (msg.Type == UIMessage::IMECloseCandidate)
	{		
		return true;
	}
	else if (msg.Type == UIMessage::IMEEndComposition)
	{
		mComposition.clear();
		mCompositionAttrs = NULL;

		return true;
	}
	//----------------------------------------------------------------------
	
	else if (msg.Type == UIMessage::KeyDown || msg.Type == UIMessage::KeyRepeat)
	{
		//----------------------------------------------------------------------

		if  (msg.IsCopyCommand ())
		{
			CopySelectionToClipboard ();
		}

		//----------------------------------------------------------------------

		else if (msg.IsCutCommand ())
		{
			CopySelectionToClipboard ();
			DeleteSelection ();
		}

		//----------------------------------------------------------------------

		else if (msg.IsPasteCommand ())
		{
			PasteFromClipboard ();
		}

		//----------------------------------------------------------------------

		else if (msg.IsSelectAllCommand ())
		{
			SetCursorMark (0);
			SetCursorPoint (static_cast<int>(mLocalText.size ()));
		}

		//----------------------------------------------------------------------

		else if (msg.Keystroke == UIMessage::Escape)
		{
			if (!ClearSelection ())
				return false;
		}

		//----------------------------------------------------------------------

		else if (msg.Keystroke == UIMessage::BackSpace)
		{
			if (!IsEditable ())
			{
				Ding ();
			}
			else
			{
				if(!mComposition.empty())
				{
					return true;
				}
				else
				{				
					if (GetCursorPoint () == GetCursorMark ())
					{
						if (GetCursorPoint () <= 0)
						{
							Ding ();
							return true;
						}
						else
						{
							SetCursorMark (GetCursorPoint () - 1);
						}
					}
					DeleteSelection ();
				}
			}
		}

		//----------------------------------------------------------------------

		else if (msg.Keystroke == UIMessage::Delete)
		{
			if (!IsEditable ())
			{
				Ding ();
			}
			else
			{
				if(!mComposition.empty())
				{
					return true;
				}
				else
				{	
					if (GetCursorPoint () == GetCursorMark ())
					{
						if (GetCursorPoint () > static_cast<int>(mLocalText.size ()))
						{
							Ding ();
							return true;
						}
						else
						{
							SetCursorMark (GetCursorPoint () + 1);
						}
					}
					DeleteSelection ();
				}
			}
		}

		//----------------------------------------------------------------------

		else if (msg.Keystroke == UIMessage::LeftArrow)
		{
			if (GetCursorPoint () <= 0 && (mCursorMark == mCursorPoint || msg.Modifiers.isShiftDown ()))
			{
				Ding ();
				return true;
			}
			else
			{
				if (msg.Modifiers.isControlDown ())
				{
					if (!MoveCaratLeftOneWord ())
					{
						Ding ();
						return true;
					}
				}
				else
					SetCursorPoint (GetCursorPoint () - 1);

				if (!msg.Modifiers.isShiftDown ())
					SetCursorMark (GetCursorPoint ());
			}
		}

		//----------------------------------------------------------------------

		else if (msg.Keystroke == UIMessage::RightArrow)
		{
			if (GetCursorPoint () >= static_cast<int>(mLocalText.size ()) && (mCursorMark == mCursorPoint || msg.Modifiers.isShiftDown ()))
			{
				Ding ();
				return true;
			}
			else
			{
				if (msg.Modifiers.isControlDown ())
				{
					if (!MoveCaratRightOneWord ())
					{
						Ding ();
						return true;
					}
				}
				else
					SetCursorPoint (GetCursorPoint () + 1);

				if (!msg.Modifiers.isShiftDown ())
					SetCursorMark (GetCursorPoint ());
			}
		}

		//----------------------------------------------------------------------


		else if (msg.Keystroke == UIMessage::UpArrow)
		{
			const int leading = mStyle->GetLeading ();
			const int index = FindCharacterFromCaratOffset (UIPoint (0L, -leading));

			if (index == mCursorPoint && (mCursorMark == mCursorPoint || msg.Modifiers.isShiftDown ()))
				Ding ();
			else
			{

				SetCursorPoint (index);

				if (!msg.Modifiers.isShiftDown ())
					SetCursorMark (GetCursorPoint ());
			}
		}

		//----------------------------------------------------------------------

		else if (msg.Keystroke == UIMessage::DownArrow)
		{
			const int leading = mStyle->GetLeading ();
			const int index = FindCharacterFromCaratOffset (UIPoint (0L, leading));

			if (index == mCursorPoint && (mCursorMark == mCursorPoint || msg.Modifiers.isShiftDown ()))
				Ding ();
			else
			{
			SetCursorPoint (index);

			if (!msg.Modifiers.isShiftDown ())
				SetCursorMark (GetCursorPoint ());
			}
		}

		//----------------------------------------------------------------------

		else if (msg.Keystroke == UIMessage::Home)
		{
			int index = 0;

			if (!msg.Modifiers.isControlDown ())
				index = FindCharacterFromCaratOffset (UIPoint (-16384, 0L));

			if (index == mCursorPoint && (mCursorMark == mCursorPoint || msg.Modifiers.isShiftDown ()))
				Ding ();
			{

			SetCursorPoint (index);

			if (!msg.Modifiers.isShiftDown ())
				SetCursorMark (GetCursorPoint ());
			}
		}

		//----------------------------------------------------------------------

		else if (msg.Keystroke == UIMessage::End)
		{
			int index = static_cast<int>(mLocalText.size ());

			if (!msg.Modifiers.isControlDown ())
				index = FindCharacterFromCaratOffset (UIPoint (16384, 0L));

			if (index == mCursorPoint && (mCursorMark == mCursorPoint || msg.Modifiers.isShiftDown ()))
				Ding ();
			else
			{
				SetCursorPoint (index);

				if (!msg.Modifiers.isShiftDown ())
					SetCursorMark (GetCursorPoint ());
			}
		}

		//----------------------------------------------------------------------

		else if (msg.Keystroke == UIMessage::Enter)
		{
			if (msg.Modifiers.isControlDown ())
				return false;

			const Unicode::String str (1, '\n');
			ReplaceSelection (str);
		}

		//----------------------------------------------------------------------

		else if (msg.Keystroke == UIMessage::Tab)
		{
			if (msg.Modifiers.isControlDown ())
				return false;

			const Unicode::String str (1, '\t');
			ReplaceSelection (str);
		}

		//----------------------------------------------------------------------

		else
		{
			return false;
		}

		//----------------------------------------------------------------------

		return true;
	}

	//----------------------------------------------------------------------

	else if (msg.Type == UIMessage::Character)
	{

		if (!IsEditable ())
		{
			Ding ();
		}
		else if (!IsEditableUnicode () && msg.Keystroke > 128)
		{
			Ding ();
		}
		else
		{
			if (mConsumeNextCharacter)
			{
				mConsumeNextCharacter = false;
				UIManager::gUIManager().getUIIMEManager()->ResendCharacter(msg.Keystroke);

				return true;
			}
			UIString::value_type ch = msg.Keystroke;

			if (ch >= ' ')
			{
				if (!IsSelected ())
					SetSelected (true);

				Unicode::String str (1, ch);

				ReplaceSelection (str);
			}
		}

		return true;
	}

	return false;

}

//-----------------------------------------------------------------

void UIText::Render( UICanvas &DestinationCanvas ) const
{
	//----------------------------------------------------------------------
	// Setup dropshadow state.
	bool const wasGlobalDropShadowEnabled = UITextStyle::GetGlobalDropShadowEnabled();

	UITextStyle::SetGlobalDropShadowEnabled(wasGlobalDropShadowEnabled && GetDropShadow());

	//----------------------------------------------------------------------
	//-- update carat blink state

	if (IsEditable () && IsSelected ())
	{
		const UITime CurrentTime = UIClock::gUIClock ().GetTime ();

		if (CurrentTime > mNextCaratBlinkTime)
		{
			const UITime BlinkInterval = UISystemDependancies::Get ().GetCaratBlinkTickCount ();

			const_cast<UIText *>( this )->SetTextFlag (TF_drawCarat, !HasTextFlag (TF_drawCarat));
			mNextCaratBlinkTime    = CurrentTime - (CurrentTime % BlinkInterval) + BlinkInterval;
		}
	}

	const unsigned long ticks = UIClock::gUIClock ().GetTime ();

	if (GetTextUnroll ())
	{
		const int deltaTicks  = (ticks - mTextUnrollStartTick);
		const float deltaSecs = deltaTicks / 60.0f;
		const int cur         = static_cast<int>(deltaSecs * mTextUnrollSpeed);

		if (cur != mTextUnrollProgress)
		{
			mTextUnrollProgress = cur;

			if (!GetUnrollSound().empty())
				UIManager::gUIManager().PlaySound(GetUnrollSound().c_str());

			if (GetTextUnrollOnce ())
			{
				//-- mLocalText.size () includes color codes, which is a problem we will ignore for now
				if (mTextUnrollProgress >= static_cast<int>(mLocalText.size ()))
					const_cast<UIText *>( this )->SetTextUnroll (false);
			}
		}
	}

	mLastRenderTicks =  ticks;

	UIWidget::Render (DestinationCanvas);

	if( mStyle )
	{
		bool const styleDropShadowEnabled = mStyle->GetDropShadowsEnabled();

		// if we don't want the autodetect code enabled, force drop shadows on the style.
		mStyle->SetDropShadowsEnabled(styleDropShadowEnabled || !GetDropShadowAutoDetect());

		UIRect clip (mMargin.left, mMargin.top, GetWidth () - mMargin.right, GetHeight () - mMargin.bottom);
		clip += GetScrollLocation ();
		
		if (DestinationCanvas.Clip (clip))
		{
			if (HasTextFlag (TF_dirty))
				const_cast<UIText *>( this )->CacheTextMeasurements();
			
			const UIPoint pos (GetTextPos ());
			
			int renderMark  = std::min (mCursorMark, mCursorPoint);
			int renderPoint = std::max (mCursorMark, mCursorPoint);
			
			const UISize theTextSize (GetSize() - UISize (mMargin.left + mMargin.right, mMargin.top + mMargin.bottom));
			
			const UIColor oldColor (DestinationCanvas.GetColor ());
			const float oldOpacity = DestinationCanvas.GetOpacity ();
			
			DestinationCanvas.ModifyColor   (mColorSelection);
			DestinationCanvas.ModifyOpacity (static_cast<float>(mColorSelection.a) / 255.0f);
			
			mStyle->RenderPrewrappedSelection(mTextAlignment, mLines->linePointers, mLines->lineWidths,
				DestinationCanvas,
				pos,
				theTextSize,
				mMaxLines,
				renderMark, renderPoint);
			
			DestinationCanvas.SetOpacity  (oldOpacity);
			DestinationCanvas.SetColor    (oldColor);
			DestinationCanvas.ModifyColor (mTextColor);
			
			
			//----------------------------------------------------------------------
			//-- Draw the background rs_text style.  This is used primarily as a mask.
			UIWidgetRectangleStyles const * const rectangleStyle = GetWidgetRectangleStyles();
			UIRectangleStyle const * const textStyle = rectangleStyle ? rectangleStyle->GetStyle(UIWidgetRectangleStyles::RS_Text) : NULL;
			
			mStyle->RenderPrewrappedText(mTextAlignment, mLines->linePointers, mLines->lineWidths,
				DestinationCanvas,
				pos,
				theTextSize,
				mMaxLines,
				IsTruncateElipsis(),
				mTextUnrollProgress,
				textStyle);
			
			DestinationCanvas.SetColor (oldColor);
			
			//-- render carat
			if (IsEditable () && IsSelected () && HasTextFlag (TF_drawCarat))
			{
				const UIPoint caratPos = GetCaratPos ();
				const long leading = mStyle->GetLeading ();
				UIRect caratRect (caratPos, UISize (std::max (1L, (leading / 5L)), leading));
				
				DestinationCanvas.ModifyOpacity (static_cast<float>(mColorCarat.a / 255.0f));
				DestinationCanvas.ClearTo  (mColorCarat, caratRect);
				DestinationCanvas.SetOpacity  (oldOpacity);
			}
		}

		// Reset drop shadows.
		mStyle->SetDropShadowsEnabled(styleDropShadowEnabled);
	}

	//----------------------------------------------------------------------
	// Reset dropshadow state.
	UITextStyle::SetGlobalDropShadowEnabled(wasGlobalDropShadowEnabled);
}

//-----------------------------------------------------------------

const UISize & UIText::GetTextExtent () const
{
	if (HasTextFlag (TF_dirty))
		const_cast<UIText *>( this )->CacheTextMeasurements();

	return mTextExtent;
}

//-----------------------------------------------------------------

void UIText::GetScrollExtent(UISize & size) const
{
	if (HasTextFlag (TF_dirty))
		const_cast<UIText *>( this )->CacheTextMeasurements();

	UIWidget::GetScrollExtent(size);
}

//-----------------------------------------------------------------

void UIText::GetScrollSizes( UISize &PageSize, UISize &LineSize ) const
{
	LineSize.x = 1;
	if (mStyle)
		LineSize.y = mStyle->GetLeading();
	else
		LineSize.y = 1;

	PageSize = GetSize();
	PageSize.y /= LineSize.y;
	PageSize.y *= LineSize.y;
}

//-----------------------------------------------------------------

void UIText::GetLinkPropertyNames( UIPropertyNameVector &In ) const
{
	In.push_back (PropertyName::PreLocalized);
	In.push_back (PropertyName::Style);
	In.push_back (PropertyName::Text);

	UIWidget::GetLinkPropertyNames( In );
}

//----------------------------------------------------------------------

void UIText::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	UIWidget::GetPropertyGroups(o_groups, category);
	GROUP_TEST_AND_PUSH(Basic, category, o_groups);
	GROUP_TEST_AND_PUSH(Behavior, category, o_groups);
	GROUP_TEST_AND_PUSH(Appearance, category, o_groups);
}

//----------------------------------------------------------------------

void UIText::GetPropertyNames( UIPropertyNameVector &In, bool forCopy ) const
{
	if (!forCopy)
	{
		UIPalette::GetPropertyNamesForType (TUIText, In);
		
		In.push_back (PropertyName::ColorCarat);
		In.push_back (PropertyName::ColorSelection);
		In.push_back (PropertyName::CursorMark);
		In.push_back (PropertyName::CursorPoint);
		In.push_back (PropertyName::Editable);
		In.push_back (PropertyName::EditableUnicode );
		In.push_back (PropertyName::LocalText);
		In.push_back (PropertyName::Margin);
		In.push_back (PropertyName::MaxLines);
		In.push_back (PropertyName::OpacityCarat);
		In.push_back (PropertyName::OpacitySelection);
		In.push_back (PropertyName::PreLocalized);
		In.push_back (PropertyName::Style);
		In.push_back (PropertyName::Text);
		In.push_back (PropertyName::TextAlignment);
		In.push_back (PropertyName::TextAlignmentVertical);
		In.push_back (PropertyName::TextColor);
		In.push_back (PropertyName::TextCapital);
		In.push_back (PropertyName::TextUnroll);
		In.push_back (PropertyName::TextUnrollOnce);
		In.push_back (PropertyName::TextUnrollSpeed);
		In.push_back (PropertyName::TextUnrollSound);
		In.push_back (PropertyName::TruncateElipsis);
		In.push_back (PropertyName::TextSelectable);	
		In.push_back(PropertyName::IME);
		In.push_back(PropertyName::DropShadow);
		In.push_back(PropertyName::DropShadowAutoDetect);
	}
	
	UIWidget::GetPropertyNames( In, forCopy );
}

//----------------------------------------------------------------------

void UIText::CopyPropertiesFrom(const UIBaseObject & rhs)
{
	if (rhs.IsA(TUIText))
	{
		UIText const & rhs_text = static_cast<UIText const &>(rhs);
		
		SetStyle(static_cast<UITextStyle *>(rhs_text.GetStyle()));
		
		SetPreLocalized(rhs_text.IsPreLocalized());
		
		SetMaxLines(rhs_text.GetMaxLines());

		SetText(rhs_text.GetText());
		if (rhs_text.IsPreLocalized())
			SetLocalText(rhs_text.GetLocalText());
		
		mMargin = rhs_text.mMargin;
		mTextColor = rhs_text.mTextColor;
		SetTextCapital(rhs_text.GetTextCapital());
		SetTextUnroll(rhs_text.GetTextUnroll());
		SetTextUnrollOnce(rhs_text.GetTextUnrollOnce());
		SetTextUnrollSpeed(rhs_text.GetTextUnrollSpeed());

		SetUnrollSound(rhs_text.GetUnrollSound());
		
		SetTextAlignment(rhs_text.mTextAlignment);
		SetTextAlignmentVertical(rhs_text.mTextAlignmentVertical);
		SetTruncateElipsis(rhs_text.IsTruncateElipsis());
		SetCursorMark(rhs_text.GetCursorMark());
		SetCursorPoint(rhs_text.GetCursorPoint());
		SetEditable(rhs_text.IsEditable());
		SetEditableUnicode(rhs_text.IsEditableUnicode());
		
		mColorCarat = rhs_text.mColorCarat;
		mColorSelection = rhs_text.mColorSelection;
		SetTextFlag (TF_IME, rhs_text.HasTextFlag(TF_IME));

		SetTextSelectable(rhs_text.HasTextFlag(TF_textSelectable));
		SetEditableUnicode(rhs_text.IsEditableUnicode());

		SetDropShadow(rhs_text.GetDropShadow());
		SetDropShadowAutoDetect(rhs_text.GetDropShadowAutoDetect());
		
		UIPalette::CopyPaletteEntriesForObject(TUIText, *this, rhs);
	}
	
	UIWidget::CopyPropertiesFrom(rhs);
}

//-----------------------------------------------------------------

bool UIText::SetProperty( const UILowerString & Name, const UIString &Value )
{
	if( Name == PropertyName::Style )
	{
		UIBaseObject * const NewStyle = UITextStyleManager::GetInstance()->GetFontForLogicalFont(Value);

		if( NewStyle || Value.empty() )
		{
			SetStyle( static_cast<UITextStyle *>( NewStyle ) );
			return true;
		}
	}
	else if( Name == PropertyName::Text )
	{
		SetText( Value );
		return true;
	}
	else if( Name == PropertyName::LocalText )
	{
		SetLocalText( Value );
		return true;
	}	
	else if (Name == PropertyName::IME )
	{
		bool b;
		if( !UIUtils::ParseBoolean( Value, b ) )
			return false;

		SetIMEEnabled (b);
		//It's assumed you can't have an IME-enabled field without it being editable
		if(b)
		{
			SetEditable(true);
			SetEditableUnicode(true);
		}
		return true;
	}
	else if( Name == PropertyName::EditableUnicode )
	{
		bool b;
		if( !UIUtils::ParseBoolean( Value, b ) )
			return false;

		SetEditableUnicode (b);
		return true;
	}
	else if( Name == PropertyName::Margin )
	{
		return UIUtils::ParseRect (Value, mMargin);
	}
	else if( Name == PropertyName::TextColor )
	{
		return UIUtils::ParseColor (Value, mTextColor);
	}
	else if( Name == PropertyName::TextCapital )
	{
		bool b = false;
		if (UIUtils::ParseBoolean (Value, b))
		{
			SetTextCapital (b);
			return true;
		}
		return false;
	}
	else if( Name == PropertyName::TextUnroll)
	{
		bool b = false;
		if (UIUtils::ParseBoolean (Value, b))
		{
			SetTextUnroll (b);
			return true;
		}
		return false;
	}
	else if( Name == PropertyName::TextUnrollOnce)
	{
		bool b = false;
		if (UIUtils::ParseBoolean (Value, b))
		{
			SetTextUnrollOnce (b);
			return true;
		}
		return false;
	}
	else if( Name == PropertyName::TextUnrollSpeed)
	{
		float f = 0.0f;
		if (UIUtils::ParseFloat (Value, f))
		{
			SetTextUnrollSpeed (f);
			return true;
		}
		return false;
	}
	else if( Name == PropertyName::TextUnrollSound)
	{
		SetUnrollSound(Unicode::wideToNarrow(Value));
		return true;
	}
	else if( Name == PropertyName::TextAlignment )
	{
		UITextStyle::Alignment a;
		if (UITextStyle::ParseAlignment (Value, a))
		{
			SetTextAlignment (a);
			return true;
		}
		return false;
	}
	else if( Name == PropertyName::TextAlignmentVertical )
	{
		TextAlignmentVertical tav = TAV_top;
		if (ParseTextAlignmentVertical (Value, tav))
		{
			SetTextAlignmentVertical (tav);
			return true;
		}
		return false;
	}
	else if ( Name == PropertyName::MaxLines )
	{
		return UIUtils::ParseLong (Value, mMaxLines);
	}
	else if ( Name == PropertyName::TruncateElipsis )
	{
		bool b = false;
		if (UIUtils::ParseBoolean (Value, b))
		{
			SetTruncateElipsis (b);
			return true;
		}
		return false;
	}
	else if (Name == PropertyName::CursorMark )
	{
		int mark = 0;
		if (UIUtils::ParseInteger(Value, mark))
		{
			SetCursorMark (mark);
			return true;
		}
		return false;
	}
	else if (Name == PropertyName::CursorPoint )
	{
		int point = 0;
		if (UIUtils::ParseInteger(Value, point))
		{
			SetCursorPoint (point);
			return true;
		}
		return false;
	}
	else if (Name == PropertyName::Editable )
	{
		bool b = false;
		if (UIUtils::ParseBoolean (Value, b))
		{
			SetEditable (b);
			return true;
		}
		return false;
	}
	else if ( Name == PropertyName::ColorCarat )
	{
		UIColor color;
		if (UIUtils::ParseColor (Value, color))
		{
			color.a = mColorCarat.a;
			mColorCarat = color;
			return true;
		}
		return false;
	}
	else if ( Name == PropertyName::ColorSelection )
	{
		UIColor color;
		if (UIUtils::ParseColor (Value, color))
		{
			color.a = mColorSelection.a;
			mColorSelection = color;
			return true;
		}
		return false;
	}
	else if ( Name == PropertyName::OpacityCarat )
	{
		float opacity = 0.0f;
		if (UIUtils::ParseFloat (Value, opacity))
		{
			mColorCarat.a = static_cast<unsigned char>(opacity * 255.0f);
			return true;
		}
		return false;
	}
	else if ( Name == PropertyName::OpacitySelection )
	{
		float opacity = 0.0f;
		if (UIUtils::ParseFloat (Value, opacity))
		{
			mColorSelection.a = static_cast<unsigned char>(opacity * 255.0f);
			return true;
		}
		return false;
	}

	else if (Name == PropertyName::PreLocalized)
	{
		bool b = false;
		if (UIUtils::ParseBoolean (Value, b))
		{
			SetPreLocalized (b);
			return true;
		}
		return false;
	}
	else if (Name == PropertyName::TextSelectable)
	{
		bool b = false;
		if (UIUtils::ParseBoolean (Value, b))
		{
			SetTextSelectable (b);
			return true;
		}
		return false;
	}
	else if (Name == PropertyName::DropShadow)
	{
		bool b = true;
		
		if(!UIUtils::ParseBoolean(Value, b))
			return false;
		
		SetDropShadow(b);
		
		return true;
	}
	else if (Name == PropertyName::DropShadowAutoDetect)
	{
		bool b = true;
		
		if(!UIUtils::ParseBoolean(Value, b))
			return false;
		
		SetDropShadowAutoDetect(b);
		
		return true;
	}
	else
	{
		UIPalette::SetPropertyForObject (*this, Name, Value);
	}


	return UIWidget::SetProperty( Name, Value );
}

//-----------------------------------------------------------------

bool UIText::GetProperty( const UILowerString & Name, UIString &Value ) const
{
	if( Name == PropertyName::Style )
	{
		if( mStyle )
		{
			Value = mStyle->GetLogicalName();
			return true;
		}
	}
	else if( Name == PropertyName::Text )
	{
		Value = mData;
		return true;
	}
	else if( Name == PropertyName::LocalText )
	{
		Value = mLocalText;
		return true;
	}
	else if ( Name == PropertyName::IME )
	{
		return UIUtils::FormatBoolean( Value, (mTextFlags & TF_IME) != 0 );
	}
	else if( Name == PropertyName::Margin )
	{
		return UIUtils::FormatRect (Value, mMargin);
	}
	else if( Name == PropertyName::EditableUnicode )
	{
		return UIUtils::FormatBoolean( Value, IsEditableUnicode ());
	}
	else if( Name == PropertyName::TextColor )
	{
		return UIUtils::FormatColor (Value, mTextColor);
	}
	else if( Name == PropertyName::TextCapital )
	{
		return UIUtils::FormatBoolean (Value, GetTextCapital ());
	}
	else if( Name == PropertyName::TextUnroll )
	{
		return UIUtils::FormatBoolean (Value, GetTextUnroll ());
	}
	else if( Name == PropertyName::TextUnrollOnce )
	{
		return UIUtils::FormatBoolean (Value, GetTextUnrollOnce ());
	}
	else if( Name == PropertyName::TextUnrollSpeed )
	{
		return UIUtils::FormatFloat (Value, GetTextUnrollSpeed ());
	}
	else if( Name == PropertyName::TextUnrollSound)
	{
		Value = Unicode::narrowToWide(GetUnrollSound());
		return true;
	}
	else if( Name == PropertyName::TextAlignment )
	{
		UITextStyle::FormatAlignment (Value, mTextAlignment);
		return true;
	}
	else if( Name == PropertyName::TextAlignmentVertical )
	{
		UIText::FormatTextAlignmentVertical (Value, mTextAlignmentVertical);
		return true;
	}
	else if ( Name == PropertyName::MaxLines )
	{
		return UIUtils::FormatLong (Value, mMaxLines);
	}
	else if ( Name == PropertyName::TruncateElipsis )
	{
		return UIUtils::FormatBoolean (Value, IsTruncateElipsis ());
	}
	else if ( Name == PropertyName::CursorMark )
	{
		return UIUtils::FormatInteger (Value, GetCursorMark ());
	}
	else if ( Name == PropertyName::CursorPoint )
	{
		return UIUtils::FormatInteger (Value, GetCursorPoint ());
	}
	else if ( Name == PropertyName::Editable )
	{
		return UIUtils::FormatBoolean (Value, IsEditable ());
	}
	else if ( Name == PropertyName::ColorCarat )
	{
		return UIUtils::FormatColor (Value, mColorCarat);
	}
	else if ( Name == PropertyName::ColorSelection )
	{
		return	UIUtils::FormatColor (Value, mColorSelection);
	}
	else if ( Name == PropertyName::OpacityCarat )
	{
		return UIUtils::FormatFloat (Value, static_cast<float>(mColorCarat.a) / 255.0f);
	}
	else if ( Name == PropertyName::OpacitySelection )
	{
		return	UIUtils::FormatFloat (Value, static_cast<float>(mColorSelection.a) / 255.0f );
	}
	else if (Name == PropertyName::PreLocalized)
	{
		return UIUtils::FormatBoolean  (Value, IsPreLocalized ());
	}
	else if (Name == PropertyName::TextSelectable)
	{
		return UIUtils::FormatBoolean  (Value, HasTextFlag (TF_textSelectable));
	}
	else if (Name == PropertyName::DropShadow)
	{
		return UIUtils::FormatBoolean(Value, GetDropShadow());
	}
	else if (Name == PropertyName::DropShadowAutoDetect)
	{
		return UIUtils::FormatBoolean(Value, GetDropShadowAutoDetect());
	}

	return UIWidget::GetProperty( Name, Value );
}

//-----------------------------------------------------------------

void UIText::CacheTextMeasurements ()
{
	if( mStyle )
	{
		const UISize marginSize (UISize (mMargin.left + mMargin.right, mMargin.top + mMargin.bottom ));
		const long   renderWidth = GetWidth () - marginSize.x;

		mLines->clear();
		SyncRenderDataToLocalText();
		mStyle->GetWrappedTextInfo( mRenderData, mMaxLines, renderWidth, mTextExtent, &mLines->linePointers, &mLines->lineWidths);
		mTextExtent.x = std::min (mTextExtent.x, renderWidth);

		SetTextFlag (TF_dirty, false);

		SetScrollExtent (mTextExtent + marginSize);


		const int size = static_cast<int>(mLocalText.size ());

		mCursorMark  = std::min (mCursorMark,  size);
		mCursorPoint = std::min (mCursorPoint, size);
	}
}

//-----------------------------------------------------------------

void UIText::SizeToContent ()
{
	if( mStyle )
	{
		SyncRenderDataToLocalText();
		mStyle->GetWrappedTextInfo( mRenderData, mMaxLines, GetMaximumSize ().x - mMargin.left - mMargin.right, mTextExtent, &mLines->linePointers, &mLines->lineWidths );

		// @todo: figure out why the MeasureText always seems a pixel too narrow
		SetSize( mTextExtent + UISize (mMargin.left + mMargin.right + 1L , mMargin.top + mMargin.bottom ) );
	}
}

//-----------------------------------------------------------------

void UIText::SetMargin (const UIRect & margin)
{
	mMargin = margin;
	SetTextFlag (TF_dirty, true);
}

//-----------------------------------------------------------------

void UIText::SetEditableUnicode (bool b)
{
	if (b)
	{
		mTextFlags |= TF_EditableUnicode;
	}
	else
		mTextFlags &= ~TF_EditableUnicode;
}

//-----------------------------------------------------------------

void UIText::SetTextColor (const UIColor & color)
{
	mTextColor = color;
}

//----------------------------------------------------------------------

void UIText::SetEditable      (bool b)
{
	SetTextFlag (TF_editable, b);
}

//----------------------------------------------------------------------

void UIText::SetPreLocalized (bool b)
{
	SetTextFlag (TF_preLocalized, b);

	if (b)
		mData.clear ();
}

//----------------------------------------------------------------------

void UIText::SetMaxLines (long lines)
{
	mMaxLines = lines;
}

//----------------------------------------------------------------------

void UIText::SetTruncateElipsis (bool t)
{
	SetTextFlag (TF_truncateElipsis, t);
}

//----------------------------------------------------------------------

void UIText::SetTextAlignment (UITextStyle::Alignment align)
{
	mTextAlignment = align;
}

//-----------------------------------------------------------------

void UIText::SetTextAlignmentVertical(TextAlignmentVertical t)
{
	mTextAlignmentVertical = t;
}

//----------------------------------------------------------------------

bool UIText::ParseTextAlignmentVertical (const Unicode::String & Value, TextAlignmentVertical & tav)
{
	if (Unicode::caseInsensitiveCompare (Value, TextAlignmentVerticalNames::Top, 0, Value.size ()))
		tav = TAV_top;
	else if (Unicode::caseInsensitiveCompare (Value, TextAlignmentVerticalNames::Center, 0, Value.size ()))
		tav = TAV_center;
	else if (Unicode::caseInsensitiveCompare (Value, TextAlignmentVerticalNames::Bottom, 0, Value.size ()))
		tav =  TAV_bottom;
	else
		return false;

	return true;
}

//----------------------------------------------------------------------

void UIText::FormatTextAlignmentVertical (Unicode::String & Value, const TextAlignmentVertical tav)
{
	if (tav == TAV_top)
		Value = Unicode::narrowToWide (TextAlignmentVerticalNames::Top);
	else if (tav == TAV_center)
		Value = Unicode::narrowToWide (TextAlignmentVerticalNames::Center);
	else
		Value = Unicode::narrowToWide (TextAlignmentVerticalNames::Bottom);
}

//----------------------------------------------------------------------

void UIText::SetCursorMark  (int mark)
{
	mCursorMark = mark;
}

//----------------------------------------------------------------------

void UIText::SetCursorPoint (int point)
{
	if (HasTextFlag (TF_dirty))
		const_cast<UIText *>( this )->CacheTextMeasurements();

	mCursorPoint = point;

	SetTextFlag (TF_drawCarat, true);
	if (!IsTextActuallySelectable ())
		ClearSelection ();

	EnsureCaratVisible ();
}

//----------------------------------------------------------------------
/*
int UIText::FindNextPrintableCharacter (int point, bool forward)
{
	point = max (0, point);
	const int count = static_cast<int>(mLocalText.size ());
	point = min (0, count;

	if (forward)
	{
		for (int i = point + 1; i < count; ++i)
		{
			const Unicode::unicode_char_t c = mLocalText [i];

			if (iswprint (c))
				return i;
		}
	}
	else
	{
		for (int i = point - 1; i >= 0; --i)
		{
			const Unicode::unicode_char_t c = mLocalText [i];

			if (iswprint (c))
				return i;
		}
	}

	return point;
}
*/

//----------------------------------------------------------------------

int UIText::GetLineFromCharacter  (const int c, int & startC) const
{
	int line = 0;
	size_t pos = 0;
	startC = 0;

	for ( ; pos != Unicode::String::npos; ++line)
	{
		pos = mLocalText.find ('\n', pos);

		if (static_cast<int>(pos) >= c)
			break;

		if (pos == Unicode::String::npos)
			break;

		++pos;

		startC = pos;
	}

	return line;
}

//----------------------------------------------------------------------

int UIText::GetCharacterFromLocation (const UIPoint & pt) const
{
	if (!mStyle)
		return 0;

	if (HasTextFlag (TF_dirty))
		const_cast<UIText *>( this )->CacheTextMeasurements();

	const UIPoint pos (GetTextPos ());
	const UISize theTextSize (GetSize() - UISize (mMargin.left + mMargin.right, mMargin.top + mMargin.bottom));

	return mStyle->FindCharacterFromLocation ( mTextAlignment, mLines->linePointers, mLines->lineWidths,
		pos,
		theTextSize,
		mMaxLines,
		pt);
}

//----------------------------------------------------------------------

void UIText::ReplaceSelection (const Unicode::String & str)
{
	Unicode::String copy (str);
	unfubarMicrosoftInvalidTextCharacters (copy);

	DeleteSelection ();

	//-- clamp the cursor point to sane values
	mCursorPoint = std::min (mCursorPoint, static_cast<int>(mLocalText.size ()));
	mCursorPoint = std::max (mCursorPoint, 0);

	const unsigned int copySize = Unicode::utf8Size(copy);
	const unsigned int spaceLeft = IsEditable () ? (GetMaximumCharacters() - GetCharacterCount()) : 0xffffffff;
	const unsigned int charactersToCopy = (copySize < spaceLeft) ? copySize : spaceLeft;
	const unsigned int startCopyPoint = 0;

	// insert keystroke
	SetPreLocalized (true);
	Unicode::truncateToUTF8Size(copy, charactersToCopy);
	mLocalText.insert (static_cast<size_t>(mCursorPoint), copy, startCopyPoint, copy.size());
	SetTextFlag (TF_dirty, true);

	const int point = mCursorPoint + copy.size ();

	SetCursorMark  (point);
	SetCursorPoint (point);

	EnsureCaratVisible ();
}

//----------------------------------------------------------------------

bool UIText::DeleteSelection ()
{
	SetPreLocalized (true);

	//-- clamp the cursor point to sane values
	mCursorPoint = std::min (mCursorPoint, static_cast<int>(mLocalText.size ()));
	mCursorPoint = std::max (mCursorPoint, 0);

	//-- clamp the cursor point/mark to sane values
	const int renderMark  = std::min (mCursorMark, mCursorPoint);
	const int renderPoint = std::max (mCursorMark, mCursorPoint);

	if (renderMark != renderPoint)
	{
		SetCursorMark  (renderMark);
		SetCursorPoint (renderMark);
		mLocalText.erase (renderMark, renderPoint - renderMark);
		SetTextFlag (TF_dirty, true);
		EnsureCaratVisible ();
		return true;
	}

	return false;
}

//----------------------------------------------------------------------

bool UIText::ClearSelection ()
{
	if (mCursorPoint == mCursorMark)
		return false;

	SetCursorMark (mCursorPoint);
	return true;
}

//----------------------------------------------------------------------

void UIText::Ding ()
{
	//@todo ding
}

//----------------------------------------------------------------------

bool UIText::SetTextFlag (TextFlag flag, bool value)
{
	const unsigned int bit = static_cast<unsigned int>(flag);

	if( value )
	{
		if ((mTextFlags & bit) == 0)
		{
			mTextFlags |= bit;
			return true;
		}
	}
	else
	{
		if ((mTextFlags & bit) != 0)
		{
			mTextFlags &= ~bit;
			return true;
		}
	}

	return false;
}

//----------------------------------------------------------------------

void UIText::EnsureCaratVisible ()
{
	if (!mStyle)
		return;

	const int leading = mStyle->GetLeading ();
	const UIPoint caratPos = GetCaratPos ();
	UIRect caratRect (caratPos, UISize (leading, leading));

	const UIPoint & scrollLocation = GetScrollLocation ();

	//-- perturb the bottom margin
	if (caratRect.top < scrollLocation.y + mMargin.top)
		caratRect -= UIPoint (0L, mMargin.top);
	else if (caratRect.bottom > scrollLocation.y - mMargin.bottom)
		caratRect += UIPoint (0L, mMargin.bottom);

	EnsureRectVisible (caratRect);
}

//----------------------------------------------------------------------

/**
* @todo cache this
*/

const UIPoint UIText::GetCaratPos () const
{
	int const compCaretIndex = UIManager::gUIManager().getUIIMEManager()->GetCompCaratIndex();
	
	int cursorPos = 0;
	if(!mComposition.empty())
	{	
		int index = 0;
		UIString::const_iterator i = mRenderData.begin() + mCursorPoint;
		bool ignoreNextEscape = false;
		for( index = 0; index != compCaretIndex; ++i, ++index )
		{
			UIString::value_type const theChar  = *i;
			
			if( !ignoreNextEscape && theChar == '\\' )
			{
				long width;
				long indentation;
				UIString::const_iterator next = (static_cast<UITextStyle *>(mStyle))->EatEscapeSequence( i, mRenderData.end(), ignoreNextEscape, width, indentation);

				if (next == mRenderData.end())
					break;
				
				// next is already pointing to the next character to
				// analyse, so compensate for the loop increment.
				if (next != i)
				{
					index--;
					i = --next;
					continue;
				}
			}		
			
			ignoreNextEscape = false;										
		}
		cursorPos = static_cast<int>(i - mRenderData.begin());
	}
	else
	{
		cursorPos = mCursorPoint;
	}

	return FindCaratPos (cursorPos);
}

//----------------------------------------------------------------------

const long UIText::GetMaximumCharHeight () const
{
 	return mStyle->GetLeading();
}

//----------------------------------------------------------------------
 
const UIPoint UIText::FindCaratPos (int point) const
{
	if (!mStyle)
		return UIPoint::zero;

	const UIPoint pos (GetTextPos ());
	const UISize theTextSize (GetSize() - UISize (mMargin.left + mMargin.right, mMargin.top + mMargin.bottom));
	return mStyle->FindLocationFromCharacter (mTextAlignment, mLines->linePointers, mLines->lineWidths, pos, theTextSize, point);
}

//----------------------------------------------------------------------

/**
* @todo cache this value, as well as carat pos
*/

const UIPoint UIText::GetTextPos () const
{
	if (mTextAlignmentVertical == TAV_bottom)
	{
		UISize extent;
		GetScrollExtent (extent);
		return UIPoint (mMargin.left, extent.y - mMargin.bottom - mTextExtent.y);
	}
	else if (mTextAlignmentVertical == TAV_center)
	{
		UIPoint pos (mMargin.left, 0L);
		UISize extent;
		GetScrollExtent (extent);

		long center = (extent.y + mMargin.top - mMargin.bottom) / 2L;

		return UIPoint (mMargin.left, center - (mTextExtent.y / 2L));
	}
	else
		return mMargin.Location ();
}

//----------------------------------------------------------------------

int UIText::FindCharacterFromCaratOffset (const UIPoint & offset) const
{
	const UIPoint caratPos = GetCaratPos ();
	const UIPoint targetPos (caratPos + offset);

	const int index = GetCharacterFromLocation (targetPos);
	const UIPoint actualPos (FindCaratPos (index));

	if (offset.y && caratPos.y == actualPos.y)
		return mCursorPoint;

	return index;
}

//----------------------------------------------------------------------

void UIText::CopySelectionToClipboard () const
{
	int renderMark  = std::min (mCursorMark, mCursorPoint);
	int renderPoint = std::max (mCursorMark, mCursorPoint);

	if (renderMark < renderPoint)
	{
		const UIString selectedText (mLocalText, renderMark, renderPoint - renderMark);
		UIClipboard::gUIClipboard ().SetText (selectedText);
	}
}

//----------------------------------------------------------------------

void UIText::PasteFromClipboard ()
{
	if (!IsEditable ())
	{
		Ding ();
		return;
	}

	UIString TextOnClipboard;
	UIClipboard::gUIClipboard ().GetText (TextOnClipboard);
	
	if(!IsEditableUnicode())
	{
		if(Unicode::isUnicode(TextOnClipboard))
		{
			Ding();
			return;
		}
	}

	ReplaceSelection (TextOnClipboard);
}

//----------------------------------------------------------------------

bool UIText::MoveCaratLeftOneWord ()
{
	int newpos = 0;
	if (!UIUtils::MovePointLeftOneWord (mLocalText, mCursorPoint, newpos))
		return false;

	SetCursorPoint (newpos);

	return true;
}

//----------------------------------------------------------------------

bool UIText::MoveCaratRightOneWord ()
{
	int newpos = 0;
	if (!UIUtils::MovePointRightOneWord (mLocalText, mCursorPoint, newpos))
		return false;

	SetCursorPoint (newpos);

	return true;
}

//----------------------------------------------------------------------

void UIText::SetTextSelectable (bool b)
{
	SetTextFlag (TF_textSelectable, b);

	if (!b)
	{
		SetCursorMark  (GetCursorPoint ());
	}
}

//----------------------------------------------------------------------

void UIText::Clear ()
{
	if (IsPreLocalized ())
		SetLocalText (Unicode::emptyString);
	else
		SetText (Unicode::emptyString);
}

//----------------------------------------------------------------------

void UIText::SetTextCapital (bool b)
{
	if (SetTextFlag (TF_textCapital, b))
		ResetLocalizedStrings ();
}

//----------------------------------------------------------------------

void UIText::SetTextUnroll  (bool b)
{
	if (SetTextFlag (TF_textUnroll, b))
	{
		ResetTextUnroll ();
		SetTextFlag     (TF_dirty, true);
	}
}

//----------------------------------------------------------------------

void UIText::SetTextUnrollOnce     (bool b)
{
	SetTextFlag (TF_textUnrollOnce, b);
}

//----------------------------------------------------------------------

void UIText::ResetTextUnroll ()
{
	mTextUnrollStartTick   = UIClock::gUIClock ().GetTime ();
	mTextUnrollProgress    = 0;
}

//----------------------------------------------------------------------

void UIText::SetTextUnrollSpeed (float charactersPerSec)
{
	mTextUnrollSpeed = charactersPerSec;
}

//----------------------------------------------------------------------

void UIText::SetTextUnrollProgress (int p)
{
	mTextUnrollProgress = p;
	const unsigned long ticks = UIClock::gUIClock ().GetTime ();

	if (mTextUnrollSpeed > 0.0f)
	{
		int deltaSecs       = static_cast<int> (static_cast<float>(p) / mTextUnrollSpeed);
		int deltaTicks      = deltaSecs * 60;
		mTextUnrollStartTick = ticks - deltaTicks;
	}
}

//----------------------------------------------------------------------

void UIText::SetMaximumCharacters(int const maximumCharacters)
{
	if (maximumCharacters < 0)
	{
		mMaximumCharacters = 0;
	}
	else
	{
		mMaximumCharacters = maximumCharacters;
	}
}

//----------------------------------------------------------------------

int UIText::GetMaximumCharacters() const
{
	return mMaximumCharacters;
}

//----------------------------------------------------------------------

bool UIText::IsAtMaximumCharacters() const
{
	return (GetCharacterCount() >= mMaximumCharacters);
}

//----------------------------------------------------------------------

int UIText::GetCharacterCount() const
{
	return static_cast<int>(Unicode::utf8Size(mLocalText));
}

//----------------------------------------------------------------------

void UIText::ConsumeNextCharacter()
{
	mConsumeNextCharacter = true;
}

//----------------------------------------------------------------------

void UIText::SetIMEEnabled (bool b)
{
	SetTextFlag (TF_IME, b);
}

//----------------------------------------------------------------------

void UIText::SyncRenderDataToLocalText()
{	
	unsigned char curAttr = 0;
	int c = 0;
	int offset = 0;
	UIString::const_iterator j;

	mRenderData.clear ();
	for( UIString::const_iterator i = mLocalText.begin(); i != mLocalText.end(); ++i )
	{	
		UIString::value_type const ch = *i;
		if (offset == mCursorMark)
		{
			for (j = mComposition.begin(), c = 0; j != mComposition.end(); ++j, ++c)
			{				
				unsigned char const attr = mCompositionAttrs[c];
				if((c == 0) || (attr != curAttr))
				{
					curAttr = attr;
					mRenderData.append(UIIMEManagerNamespace::colorModifiers[attr]);
				}
				
				UIString::value_type const ch2 = *j;
				mRenderData.append( 1, ch2);
			}					
			if(!mComposition.empty())
				mRenderData.append(L"\\#.");
		}
		mRenderData.append( 1, ch );
		offset++;
	}

	//If the cursor is at the end of the string, append to the end
	if (offset == mCursorMark)
	{
		for (j = mComposition.begin(), c = 0; j != mComposition.end(); ++j, ++c)
		{
			unsigned char const attr = mCompositionAttrs[c];
			if((c == 0) || (attr != curAttr))
			{
				curAttr = attr;
				mRenderData.append(UIIMEManagerNamespace::colorModifiers[attr]);
			}
			UIString::value_type const ch2 = *j;

			mRenderData.append( 1, ch2);
		}
		if(!mComposition.empty())
			mRenderData.append(L"\\#.");
	}
}

//----------------------------------------------------------------------

void UIText::SetUnrollSound(std::string const & sound)
{
	if (sound.empty())
	{
		delete mTextUnrollSound;
		mTextUnrollSound = NULL;
	}
	else
	{
		if (mTextUnrollSound) 
		{
			*mTextUnrollSound = sound;
		}
		else
		{
			mTextUnrollSound = new std::string(sound);
		}
	}
}

//----------------------------------------------------------------------

std::string const & UIText::GetUnrollSound() const
{
	return mTextUnrollSound ? *mTextUnrollSound : s_emptyString;
}

//-----------------------------------------------------------------
