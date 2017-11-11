
#include "_precompile.h"
#include "UITextbox.h"

#include "UICanvas.h"
#include "UIClipboard.h"
#include "UIClock.h"
#include "UIData.h"
#include "UIDataSource.h"
#include "UIEventCallback.h"
#include "UIFontCharacter.h"
#include "UIGridStyle.h"
#include "UIIMEManager.h"
#include "UIListbox.h"
#include "UIManager.h"
#include "UIMessage.h"
#include "UISystemDependancies.h"
#include "UIPropertyDescriptor.h"
#include "UITextStyle.h"
#include "UITextboxStyle.h"
#include "UIUtils.h"

#include <cassert>
#include <limits>
#include <vector>

const char * const UITextbox::TypeName                            = "Textbox";
const UIString::value_type	UITextbox::PasswordCharacter   = '*';

const UILowerString UITextbox::PropertyName::MaxLength             = UILowerString ("MaxLength");
const UILowerString UITextbox::PropertyName::MaxLines              = UILowerString ("MaxLines");
const UILowerString UITextbox::PropertyName::OnChange              = UILowerString ("OnChange");
const UILowerString UITextbox::PropertyName::OnOverflow            = UILowerString ("OnOverflow");
const UILowerString UITextbox::PropertyName::Password              = UILowerString ("Password");
const UILowerString UITextbox::PropertyName::Style                 = UILowerString ("Style");
const UILowerString UITextbox::PropertyName::Text                  = UILowerString ("Text");
const UILowerString UITextbox::PropertyName::LocalText             = UILowerString ("LocalText");
const UILowerString UITextbox::PropertyName::WrapWidth             = UILowerString ("WrapWidth");
const UILowerString UITextbox::PropertyName::Editable              = UILowerString ("Editable");
const UILowerString UITextbox::PropertyName::TextColor             = UILowerString ("TextColor");
const UILowerString UITextbox::PropertyName::NumericInteger        = UILowerString ("NumericInteger");
const UILowerString UITextbox::PropertyName::NumericFloat          = UILowerString ("NumericFloat");
const UILowerString UITextbox::PropertyName::NumericNegative       = UILowerString ("NumericNegative");
const UILowerString UITextbox::PropertyName::EditableUnicode       = UILowerString ("EditableUnicode");
const UILowerString UITextbox::PropertyName::IME                   = UILowerString ("IME");

const UILowerString UITextbox::MethodName::IsEmpty                 = UILowerString ("IsEmpty");

const unsigned char max_integer_length = 8;

//-----------------------------------------------------------------
#define _TYPENAME UITextbox

namespace UITextboxNamespace
{
	//================================================================
	// Appearance category
	_GROUPBEGIN(Appearance)
		_DESCRIPTOR(Style,"",T_object),
		_DESCRIPTOR(Text,"",T_string),
		_DESCRIPTOR(WrapWidth,"",T_string),
		_DESCRIPTOR(TextColor,"",T_color),
		_DESCRIPTOR(NumericInteger,"",T_bool),
		_DESCRIPTOR(NumericFloat,"",T_bool),
		_DESCRIPTOR(NumericNegative,"",T_bool)
	_GROUPEND(Appearance, 2, 0);
	//================================================================

	//================================================================
	// Behavior category
	_GROUPBEGIN(Behavior)
		_DESCRIPTOR(MaxLength,"",T_int),
		_DESCRIPTOR(MaxLines,"",T_int),
		_DESCRIPTOR(Password,"",T_bool),
		_DESCRIPTOR(Editable,"",T_bool),
		_DESCRIPTOR(EditableUnicode,"",T_bool)
	_GROUPEND(Behavior, 2, 1);
	//================================================================

	//================================================================
	// AdvancedBehavior category
	_GROUPBEGIN(AdvancedBehavior)
		_DESCRIPTOR(OnChange,"",T_string),
		_DESCRIPTOR(OnOverflow,"",T_string),
		_DESCRIPTOR(IME,"",T_string)
	_GROUPEND(AdvancedBehavior, 2, 2);
	//================================================================
}
using namespace UITextboxNamespace;

//----------------------------------------------------------------------

UITextbox::UITextbox () :
mStyle                (0),
mText                 (),
mLocalText            (),
mRenderData           (),
mCaratIndex           (0),
mSelectionStart       (0),
mSelectionEnd         (0),
mSelectionFixedEnd    (0),
mWrapWidth            (UITextbox::NoWrapping),
mNextCaratBlinkTime   (0),
mTextboxAttributeBits (TABF_Editable | TABF_KeyClickOn),
mMaxLines             (1),
mMaxLength            (-1),
mMaxIntegerLength     (max_integer_length),
mTextDirty            (false),
mLinePointers         (),
mLineWidths           (),
mRenderLinePointers   (),
mRenderLineWidths     (),
mTextColor            (0xff, 0xff, 0xff),
mMaximumCharacters    (1024 * 1024 * 16),
mConsumeNextCharacter(false)
{
	ClearSelection();
}

//----------------------------------------------------------------------

UITextbox::~UITextbox ()
{
	SetStyle(0);
}

//----------------------------------------------------------------------

bool UITextbox::IsA( const UITypeID type ) const
{
	return type == TUITextbox || UIWidget::IsA( type );
}

//----------------------------------------------------------------------

const char *UITextbox::GetTypeName () const
{
	return TypeName;
}

//----------------------------------------------------------------------

UIBaseObject *UITextbox::Clone () const
{
	return new UITextbox;
}

//-----------------------------------------------------------------

void UITextbox::SetText( const UIString &NewString )
{
	mText = NewString;

	UIManager::gUIManager ().CreateLocalizedString (mText, mLocalText);

	// call this to get various internal data setup properly
	SetLocalText (mLocalText);
};

//-----------------------------------------------------------------

void UITextbox::GetText( UIString &OutString ) const
{
	OutString = mText;
}
//-----------------------------------------------------------------

void UITextbox::SetLocalText( const UIString &NewString )
{
	mLocalText = NewString;
	Unicode::truncateToUTF8Size(mLocalText, static_cast<unsigned int>(GetMaximumCharacters()));

	if( mCaratIndex > static_cast<long>(mLocalText.length()) )
		mCaratIndex = mLocalText.length();

	if( mSelectionStart > static_cast<long>(mLocalText.length()) )
		mSelectionStart = mLocalText.length();

	if( mSelectionEnd > static_cast<long>(mLocalText.length()) )
		mSelectionEnd = mLocalText.length();

	if( mSelectionFixedEnd > static_cast<long>(mLocalText.length()) )
		mSelectionFixedEnd = mLocalText.length();

	mTextDirty = true;
};
//-----------------------------------------------------------------

void UITextbox::GetLocalText( UIString &OutString ) const
{
	OutString = mLocalText;
}
//-----------------------------------------------------------------

void UITextbox::SetStyle( UITextboxStyle *NewStyle )
{
	if( NewStyle )
		NewStyle->Attach( this );

	if( mStyle )
		mStyle->Detach( this );

	mStyle     = NewStyle;
	mTextDirty = true;
}

//-----------------------------------------------------------------

UIStyle *UITextbox::GetStyle () const
{
	return mStyle;
};

//-----------------------------------------------------------------

const UICursor *UITextbox::GetMouseCursor () const
{
	const UICursor *theCursor = UIWidget::GetMouseCursor();

	if( !theCursor && mStyle )
		theCursor = mStyle->GetMouseCursor();

	return theCursor;
}

//-----------------------------------------------------------------

UICursor *UITextbox::GetMouseCursor ()
{

	UICursor *theCursor = UIWidget::GetMouseCursor();

	if( !theCursor && mStyle )
		theCursor = mStyle->GetMouseCursor();

	return theCursor;
}

//-----------------------------------------------------------------

void UITextbox::GetScrollExtent( UISize &Out ) const
{
	if( mTextDirty )
		const_cast<UITextbox *>( this )->CacheTextMeasurements();

	UIWidget::GetScrollExtent( Out );
}

//-----------------------------------------------------------------

void UITextbox::GetScrollSizes( UISize &PageSize, UISize &LineSize ) const
{
	PageSize.x = GetWidth();
	PageSize.y = GetHeight();

	if( mStyle )
	{
		UIRect TextPadding;
		mStyle->GetTextPadding( TextPadding );

		PageSize.x -= TextPadding.left + TextPadding.right;
		PageSize.y -= TextPadding.top + TextPadding.bottom;
	}

	LineSize = UISize(1,1);
}

//-----------------------------------------------------------------

void UITextbox::SetSelected( const bool NewSelected )
{
	UIWidget::SetSelected( NewSelected );

	if( NewSelected )
	{
		if (GetIMEEnabled())
		{
			UIManager::gUIManager().getUIIMEManager()->SetContextWidget(this);
			UIManager::gUIManager().getUIIMEManager()->SetEnabled(true);			
			//Make sure the box is synced up with the current state of the IME composition
			UIMessage compositionSyncMessage;
			compositionSyncMessage.Type = UIMessage::IMEComposition;
			ProcessMessage(compositionSyncMessage);
		}
		else
		{
			UIManager::gUIManager().getUIIMEManager()->SetEnabled(false);			
			UIManager::gUIManager().getUIIMEManager()->SetContextWidget(NULL);
		}
		ResetCarat();
	}
	else
	{
		mComposition.clear();
		mCompositionAttrs = NULL;
		mTextDirty = true;
	}
}

//-----------------------------------------------------------------

void UITextbox::SelectAll()
{
	mSelectionStart = 0;
	mSelectionEnd = static_cast<long>(mLocalText.length());
}

//-----------------------------------------------------------------

void UITextbox::SetWrapWidth( const long NewWrapWidth )
{
	mWrapWidth = NewWrapWidth;
}

//-----------------------------------------------------------------

long UITextbox::GetWrapWidth () const
{
	return mWrapWidth;
}

//-----------------------------------------------------------------

bool UITextbox::ProcessMessage( const UIMessage &theMessage )
{
	if( UIWidget::ProcessMessage( theMessage ) )
		return true;

	switch( theMessage.Type )
	{

	case UIMessage::IMEComposition:
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

		mTextDirty = true;

		return true;
	}
	case UIMessage::IMEChangeCandidate:
	{
		return true;
	}
	case UIMessage::IMECloseCandidate:
	{		
		return true;
	}
	case UIMessage::IMEEndComposition:
	{
		mComposition.clear();
		mCompositionAttrs = NULL;

		return true;
	}

	case UIMessage::KeyDown:
	case UIMessage::KeyRepeat:
		{
			bool TextChanged   = false;
			long OldCaratIndex = mCaratIndex;

			if( theMessage.IsCopyCommand() )
			{
				CopySelectionToClipboard();
				return true;
			}
			else if( theMessage.IsCutCommand() )
			{
				CopySelectionToClipboard();
				DeleteSelection(false);
				return true;
			}
			else if( theMessage.IsPasteCommand() )
			{
				DeleteSelection(false);
				PasteFromClipboard();
				return true;
			}

			if( theMessage.Keystroke == UIMessage::BackSpace )
			{				
				if(mComposition.empty())
					TextChanged = performBackspaceKey (false, theMessage.Modifiers.isShiftDown ());
				else
					TextChanged = false;
			}
			else if( theMessage.Keystroke == UIMessage::Delete )
			{
				if(mComposition.empty())
					TextChanged = performDeleteKey (false, theMessage.Modifiers.isShiftDown ());
				else
					TextChanged=false;
			}
			else if ( theMessage.Keystroke == UIMessage::Escape )
			{
				return ClearSelection();
			}
			else if( theMessage.Keystroke == UIMessage::LeftArrow )
			{
				if( theMessage.Modifiers.LeftControl || theMessage.Modifiers.RightControl )
					MoveCaratLeftOneWord();
				else
					MoveCaratLeft();
			}
			else if( theMessage.Keystroke == UIMessage::RightArrow )
			{
				if( theMessage.Modifiers.LeftControl || theMessage.Modifiers.RightControl )
					MoveCaratRightOneWord();
				else
					MoveCaratRight();
			}
			else if( theMessage.Keystroke == UIMessage::UpArrow )
				MoveCaratUp();
			else if( theMessage.Keystroke == UIMessage::DownArrow )
				MoveCaratDown();
			else if( theMessage.Keystroke == UIMessage::Home )
			{
				if( theMessage.Modifiers.LeftControl || theMessage.Modifiers.RightControl )
					MoveCaratToStartOfDocument();
				else
					MoveCaratToStartOfLine();
			}
			else if( theMessage.Keystroke == UIMessage::End )
			{
				if( theMessage.Modifiers.LeftControl || theMessage.Modifiers.RightControl )
					MoveCaratToEndOfDocument();
				else
					MoveCaratToEndOfLine();
			}
			else if( (theMessage.Keystroke == UIMessage::Enter))
			{
				if (theMessage.Modifiers.isControlDown ())
					return false;

				//-- don't eat enter on one-line textboxes
				if (mMaxLines == 1)
					return false;
			}
			else
			{
				return false;
			}

			handleTextChanged (OldCaratIndex, TextChanged, theMessage.Modifiers.isShiftDown ());

			return true;//TextChanged;
		}

	case UIMessage::Character:

		if (mConsumeNextCharacter)
		{
			mConsumeNextCharacter = false;
			UIManager::gUIManager().getUIIMEManager()->ResendCharacter(theMessage.Keystroke);

			break;
		}

		if (!GetEditable ())
		{
			Ding ();
			break;
		}

		if(theMessage.Keystroke >= ' ')
		{
			if (!IsSelected ())
				SetSelected (true);

			if (theMessage.Keystroke == '-' && IsNumericNegative () && (IsNumericFloat () || IsNumericInteger ()))
			{
				if (mCaratIndex != 0 || mLocalText.find ('-') != mLocalText.npos)
				{
					Ding ();
					break;
				}
			}

			else if (!iswdigit (theMessage.Keystroke))
			{
				if (IsNumericInteger ())
				{
					Ding ();
					break;
				}
				else if (IsNumericFloat () && (theMessage.Keystroke != '.' || mLocalText.find ('.') != mLocalText.npos))
				{
					Ding ();
					break;
				}
			}

			if (!IsEditableUnicode () && theMessage.Keystroke > 128)
			{
				Ding ();
				break;
			}

			UIString::value_type ch = theMessage.Keystroke;

			if( ch == '\r' )
				ch = '\n';

			DeleteSelection(false);

			unsigned int localTextUTF8Size = Unicode::utf8Size(mLocalText);

			if( (mMaxLength > 0) && localTextUTF8Size > (unsigned)mMaxLength )
			{
				SendCallback( &UIEventCallback::OnTextboxOverflow, PropertyName::OnOverflow );
			}
			else
			{
				//-- truncate integers to 9 digits
				// Leaving as wide for integers, since the conversion would be wasted
				if (IsNumericInteger () && mLocalText.size () >= static_cast<size_t>(mMaxIntegerLength))
				{
					Ding ();
					break;
				}

				// How many bytes will this character take up in UTF8?
				unsigned short chBytes = Unicode::utf8CharSize(ch);

				// insert keystroke
				if (localTextUTF8Size + chBytes <= (unsigned)mMaximumCharacters)
				{
					mLocalText.insert( mCaratIndex, &ch, 1 );
					++mCaratIndex;
				}

				CacheTextMeasurements();

				if( (mMaxLines > 0) && (mLinePointers.size() - 1) > (unsigned)mMaxLines )
				{
					// Too many lines, remove keystroke
					--mCaratIndex;
					mLocalText.erase( mCaratIndex );
					mTextDirty = true;
					SendCallback( &UIEventCallback::OnTextboxOverflow, PropertyName::OnOverflow );
				}
				else
				{
					mTextDirty = true;
					SendCallback( &UIEventCallback::OnTextboxChanged, PropertyName::OnChange );
					SendDataChangedCallback (PropertyName::LocalText, mLocalText);

					if (mStyle && (mTextboxAttributeBits & TABF_KeyClickOn) != 0)
						mStyle->PlaySoundKeyclick ();
				}
			}
		}

		break;

	case UIMessage::LeftMouseDown:
		MoveCaratToPoint( theMessage.MouseCoords + GetScrollLocation() );
		if (!IsSelected ())
			SetSelected (true);
		else
			ResetCarat();

		if( theMessage.Modifiers.LeftShift || theMessage.Modifiers.RightShift )
			ModifySelection( mCaratIndex );
		else
			ClearSelection();

		break;

	case UIMessage::LeftMouseDoubleClick:

		SelectCurrentWord ();
		break;

	case UIMessage::MouseMove:
		if( theMessage.Modifiers.LeftMouseDown )
		{
			MoveCaratToPoint( theMessage.MouseCoords + GetScrollLocation() );
			ResetCarat();

			ModifySelection( mCaratIndex );
		}
		break;
	default:
		return false;
	}

	ScrollCaratOnScreen ();
	return true;
}

//----------------------------------------------------------------------

void UITextbox::Render( UICanvas &DestinationCanvas ) const
{
//	UIWidget::RenderDefault (DestinationCanvas);, mStyle ? mStyle->GetRectangleStyles () : 0);

	UIWidget::Render (DestinationCanvas);

	if( !mStyle )
		return;

	UITime CurrentTime = UIClock::gUIClock().GetTime();

	if( IsSelected() && GetEditable () && (CurrentTime > mNextCaratBlinkTime))
	{
		UITime BlinkInterval = UISystemDependancies::Get().GetCaratBlinkTickCount();

		const_cast<UITextbox *>( this )->mTextboxAttributeBits ^= TABF_DrawCarat;
		const_cast<UITextbox *>( this )->mNextCaratBlinkTime    = CurrentTime - (CurrentTime % BlinkInterval) + BlinkInterval;
	}

	UITextStyle	*CurrentTextStyle = mStyle->GetTextStyle( GetVisualState() );
	UIGridStyle *CurrentGridStyle = mStyle->GetGridStyle( GetVisualState() );
	UIRect			 TextPadding;

	mStyle->GetTextPadding( TextPadding );

	const UIPoint & CurrentScrollLocation = GetScrollLocation();

	// Render background unscrolled
	DestinationCanvas.PushState();
	DestinationCanvas.Translate( CurrentScrollLocation );

	if( CurrentGridStyle )
	{
		bool Selected = false;
		CurrentGridStyle->Render( GetAnimationState(), DestinationCanvas, GetSize() - UISize(1,1), UISize( 1, 1 ), &Selected );

		DestinationCanvas.Flush();
	}

	// Do this after we've compensated for the scroll out parent assigned us
	// because CacheTextMeasurements may move the scroll location
	if( mTextDirty )
		const_cast<UITextbox *>( this )->CacheTextMeasurements();

	DestinationCanvas.Clip( UIRect( TextPadding.left, TextPadding.top, GetWidth() - TextPadding.right, GetHeight() - TextPadding.bottom ) );
	DestinationCanvas.Translate( -CurrentScrollLocation.x + TextPadding.left, -CurrentScrollLocation.y + TextPadding.top );

	// Render selection background
	if( IsEnabled () && IsSelected() && (mSelectionStart != mSelectionEnd) && mSelectionStart >= 0 && mSelectionEnd >= 0)
	{
		DestinationCanvas.ModifyOpacity( mStyle->GetSelectionOpacity() );

		long		FirstSelectedRow = GetRowFromOffset( mSelectionStart );
		long		LastSelectedRow  = GetRowFromOffset( mSelectionEnd );
		UIColor FillColor				 = mStyle->GetSelectionColor();
		UIRect  FillRect;

		if( FirstSelectedRow < LastSelectedRow )
		{
			if( mLocalText[mSelectionStart - 1] == '\n' )
				FirstSelectedRow++;
		}

		if( FirstSelectedRow == LastSelectedRow )
		{
			FillRect.top    = FirstSelectedRow * CurrentTextStyle->GetLeading();
			FillRect.bottom = FillRect.top + CurrentTextStyle->GetLeading();

			FillRect.left   = GetXCoordFromRowAndOffset( FirstSelectedRow, mSelectionStart );
			FillRect.right  = GetXCoordFromRowAndOffset( FirstSelectedRow, mSelectionEnd );

			DestinationCanvas.ClearTo( FillColor, FillRect );
		}
		else
		{
			// Fill first section
			FillRect.top    = FirstSelectedRow * CurrentTextStyle->GetLeading();
			FillRect.bottom = FillRect.top + CurrentTextStyle->GetLeading();

			FillRect.left   = GetXCoordFromRowAndOffset( FirstSelectedRow, mSelectionStart );
			FillRect.right  = GetWidth();

			DestinationCanvas.ClearTo( FillColor, FillRect );

			// Fill middle section
			if( LastSelectedRow - FirstSelectedRow > 1 )
			{
				FillRect.left   = 0;
				FillRect.right  = GetWidth();

				FillRect.top    = (FirstSelectedRow + 1) * CurrentTextStyle->GetLeading();
				FillRect.bottom = LastSelectedRow * CurrentTextStyle->GetLeading();

				DestinationCanvas.ClearTo( FillColor, FillRect );
			}

			// Fill trailing section
			FillRect.top    = LastSelectedRow * CurrentTextStyle->GetLeading();
			FillRect.bottom = FillRect.top + CurrentTextStyle->GetLeading();

			FillRect.left   = 0;
			FillRect.right  = GetXCoordFromRowAndOffset( LastSelectedRow, mSelectionEnd );

			DestinationCanvas.ClearTo( FillColor, FillRect );
		}

		DestinationCanvas.Flush();
		DestinationCanvas.RestoreOpacity();
	}

	// Render text
	if( CurrentTextStyle )
	{
		UIRect TextRect;

		const UIColor oldColor (DestinationCanvas.GetColor ());

		if ((mTextboxAttributeBits & TABF_TextColorOverride) != 0)
			DestinationCanvas.ModifyColor (mTextColor);
		else
			DestinationCanvas.ModifyColor (mStyle->GetTextColor ());

		if (mTextboxAttributeBits & TABF_Password )
		{
			UIString PasswordString( mLocalText.size(), PasswordCharacter );
			//CurrentTextStyle->RenderText( PasswordString, &DestinationCanvas, UIPoint(-mTextScroll,0) );
			CurrentTextStyle->RenderText( UITextStyle::Left, PasswordString, DestinationCanvas, UIPoint(0,0) );
		}
		else
		{
			/*
			CurrentTextStyle->RenderPrewrappedText( UITextStyle::Alignment::Left, mLinePointers, mLineWidths,
				&DestinationCanvas, UIPoint(-mTextScroll,0), TextRect.Size() );
			*/
			CurrentTextStyle->RenderPrewrappedText( UITextStyle::Left, mRenderLinePointers, mRenderLineWidths,
				DestinationCanvas, UIPoint(0,0), TextRect.Size() );
		}

		DestinationCanvas.SetColor (oldColor);

		DestinationCanvas.Flush();
	}

	// Render carat
	if( IsEnabled () && IsSelected() && GetEditable () && (mTextboxAttributeBits & TABF_DrawCarat) )
		RenderCarat( DestinationCanvas );

	DestinationCanvas.PopState();
}

//----------------------------------------------------------------------

void UITextbox::MoveCaratLeft ()
{
	if( mCaratIndex > 0 )
	{
		--mCaratIndex;

		// Pass over things like \r, etc
		while( (mCaratIndex > 0) &&
			// TODO: make isprint () Unicode compliant - jww
			     !iswprint( mLocalText[mCaratIndex] ) &&
					 (mLocalText[mCaratIndex] != '\n') )
		{
			--mCaratIndex;
		}

		ScrollCaratOnScreen();
		CalculateCaratRect();
	}
}

//----------------------------------------------------------------------

void UITextbox::MoveCaratLeftOneWord ()
{
	int newpos = 0;
	if (!UIUtils::MovePointLeftOneWord (mLocalText, mCaratIndex, newpos))
		return;

	mCaratIndex = newpos;

	ScrollCaratOnScreen();
	CalculateCaratRect();
}

//----------------------------------------------------------------------

void UITextbox::MoveCaratRight ()
{
	long DataSize = mLocalText.size();

	if( mCaratIndex < DataSize )
	{
		++mCaratIndex;

		// Pass over things like \r, etc
		while( (mCaratIndex != DataSize ) &&
			// TODO: make isprint () Unicode compliant - jww
			     !iswprint( mLocalText[mCaratIndex] ) &&
					 (mLocalText[mCaratIndex] != '\n') )
		{
			++mCaratIndex;
		}

		ScrollCaratOnScreen();
		CalculateCaratRect();
	}
}

//----------------------------------------------------------------------

void UITextbox::MoveCaratRightOneWord ()
{
	int newpos = 0;
	if (!UIUtils::MovePointRightOneWord (mLocalText, mCaratIndex, newpos))
		return;

	mCaratIndex = newpos;

	ScrollCaratOnScreen();
	CalculateCaratRect();
}

//----------------------------------------------------------------------

void UITextbox::MoveCaratUp ()
{
	MoveCaratVertically( -1 );
}

//----------------------------------------------------------------------

void UITextbox::MoveCaratDown ()
{
	MoveCaratVertically( 1 );
}

//----------------------------------------------------------------------

void UITextbox::MoveCaratVertically( long MovementDirection )
{
	if( !mStyle )
		return;

	UITextStyle *CurrentTextStyle = mStyle->GetTextStyle( GetVisualState() );

	if( !CurrentTextStyle )
		return;

	if( mTextDirty )
		CacheTextMeasurements();

	if( !mLocalText.empty() )
	{
		long CaratLineNumber = mCaratRect.top / CurrentTextStyle->GetLeading();

		if( (CaratLineNumber + MovementDirection) < 0 )
			return;

		if( (CaratLineNumber + MovementDirection) >= static_cast<long>(mLinePointers.size() - 1) )
			return;

		CaratLineNumber += MovementDirection;

		long PixelOffset = 0;
		long CaratOffset = mCaratRect.left;

		const UIString::value_type *s = mLinePointers[CaratLineNumber];

		for( ; s < mLinePointers[CaratLineNumber + 1]; ++s )
		{
			if( *s != '\n' )
			{
				UIFontCharacter *Glyph = CurrentTextStyle->GetCharacter( *s );

				if( Glyph )
				{
					if( PixelOffset + (Glyph->GetAdvance() / 2) > CaratOffset )
						break;

					PixelOffset += Glyph->GetAdvance();
				}
			}
			else
				break;
		}

		mCaratIndex = s - mLinePointers[0];
		ScrollCaratOnScreen();
		CalculateCaratRect();
	}
}

//----------------------------------------------------------------------

const UIPoint UITextbox::GetCaratPos() const
{
 	return UIPoint(GetXCoordFromRowAndOffset(0, mCaratIndex), mCaratRect.bottom);
}

//----------------------------------------------------------------------


void UITextbox::SelectCurrentWord ()
{
	size_t wordStart = 0;
	size_t wordEnd   = static_cast<size_t>(mLocalText.npos); // broken msvc compiler

	if (mCaratIndex < 0)
		return;

	wordStart = mLocalText.find_last_of  (Unicode::whitespace, mCaratIndex);
	wordEnd   = mLocalText.find_first_of (Unicode::whitespace, mCaratIndex);

	if (wordStart == mLocalText.npos)
		wordStart = 0;
	if (wordEnd == mLocalText.npos)
		wordEnd = mLocalText.size ();

	//-- try to highlight contiguous breaking characters
	if (wordStart == wordEnd)
	{
		wordStart = mLocalText.find_last_not_of  (Unicode::whitespace, mCaratIndex);
		wordEnd   = mLocalText.find_first_not_of (Unicode::whitespace, mCaratIndex);
	}

	if (wordStart == mLocalText.npos)
		wordStart = 0;
	else if (mLocalText [wordStart] == ' ')
		++wordStart;

	if (wordEnd == mLocalText.npos)
	{
		mCaratIndex = mLocalText.size ();
		wordEnd = mCaratIndex;
	}
	else
		mCaratIndex = wordEnd;

	if (wordStart != wordEnd)
	{
		mSelectionStart    = wordStart;
		mSelectionEnd      = wordEnd;
		mSelectionFixedEnd = wordEnd;

		mCaratIndex = wordStart;
		CalculateCaratRect();
		ScrollCaratOnScreen();
		mCaratIndex = wordEnd;
	}
	else
	{
		mSelectionStart    = -1;
		mSelectionEnd      = -1;
		mSelectionFixedEnd = -1;
	}

	CalculateCaratRect();
	ScrollCaratOnScreen();
}

//----------------------------------------------------------------------

long UITextbox::GetCaratOffsetFromPoint( const UIPoint & widgetPt )
{
	if( mLocalText.empty() )
		return 0;

	if (mRenderLinePointers.empty ())
		return 0;

	if( !mStyle )
		return 0;

	UITextStyle	* const CurrentStyle = mStyle->GetTextStyle( GetVisualState() );

	if( !CurrentStyle )
		return 0;

	const UIRect & textPadding = mStyle->GetTextPadding ();
	const UIPoint pt = widgetPt - textPadding.Location ();

	if( mTextDirty )
		CacheTextMeasurements();

	long CaratRow = pt.y / CurrentStyle->GetLeading();

	if( CaratRow < 0 )
		CaratRow = 0;
	else if( CaratRow >= static_cast<long> (mRenderLinePointers.size() - 1))
	{
		if (mRenderLinePointers.size () > 1)
			CaratRow = mRenderLinePointers.size() - 2;
		else
			CaratRow = mRenderLinePointers.size() - 1;
	}

	long  PixelOffset              = textPadding.left;
	const UIString::value_type * s = mRenderLinePointers[CaratRow];

#if 0
	//-- Debugging to look at the string we're advancing over
	UIString const wstring(s);
	UI_REPORT_LOG(true, ("ws=[%s]\n", Unicode::wideToNarrow(wstring).c_str()));
#endif

	int skippedSlashes = 0;

	while (PixelOffset <= pt.x && *s)
	{
		//-- We need to skip the first slash of a "\\".  The UI will embed an extra \ for ever \ added.
		if (*s == '\\' && *(s + 1) && *(s + 1) == '\\')
		{
			++s;
			++skippedSlashes;
		}

		UIFontCharacter * const Glyph = CurrentStyle->GetCharacter (*s);			

		if (Glyph)
		{
			if (abs(PixelOffset - pt.x) < (Glyph->GetWidth() / 2))
				break;

			PixelOffset += Glyph->GetAdvance();
		}

		++s;

		if( s == mRenderLinePointers[CaratRow + 1] )
		{
			if( CaratRow != static_cast<long>(mRenderLinePointers.size() - 2 ))
				--s;

			break;
		}
	}

	return s - mRenderLinePointers[0] - skippedSlashes;

}

//----------------------------------------------------------------------

void UITextbox::MoveCaratToPoint( const UIPoint &pt )
{
	mCaratIndex = GetCaratOffsetFromPoint( pt );

	ScrollCaratOnScreen();
	CalculateCaratRect();
}

//----------------------------------------------------------------------

void UITextbox::MoveCaratToStartOfLine ()
{
	if( mTextDirty )
		CacheTextMeasurements();

	if( !mLocalText.empty() )
	{
		long CaratLineNumber = GetCaratLine();

		mCaratIndex = mRenderLinePointers[CaratLineNumber] - mRenderLinePointers[0];

		ScrollCaratOnScreen();
		CalculateCaratRect();
	}
}

//----------------------------------------------------------------------

void UITextbox::MoveCaratToEndOfLine ()
{
	if( mTextDirty )
		CacheTextMeasurements();

	if( !mLocalText.empty() )
	{
		MoveCaratToStartOfLine();

		int const dataSize = static_cast<int>(mLocalText.size());
		while (mCaratIndex < dataSize)
			MoveCaratRight();
	}
}

//----------------------------------------------------------------------

void UITextbox::MoveCaratToStartOfDocument ()
{
	mCaratIndex = 0;

	ScrollCaratOnScreen();
	CalculateCaratRect();
}

//----------------------------------------------------------------------

void UITextbox::MoveCaratToEndOfDocument ()
{
	mCaratIndex = mLocalText.size();

	ScrollCaratOnScreen();
	CalculateCaratRect();
}

//----------------------------------------------------------------------

void UITextbox::RenderCarat( UICanvas &theCanvas ) const
{
	theCanvas.ClearTo( mStyle->GetCaratColor(), mCaratRect );
}

//----------------------------------------------------------------------

void UITextbox::GetLinkPropertyNames( UIPropertyNameVector &In ) const
{
	In.push_back( PropertyName::Style );
	In.push_back( PropertyName::Text );
	UIWidget::GetLinkPropertyNames( In );
}

//----------------------------------------------------------------------

void UITextbox::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	UIWidget::GetPropertyGroups(o_groups, category);
	GROUP_TEST_AND_PUSH(Appearance, category, o_groups);
	GROUP_TEST_AND_PUSH(Behavior, category, o_groups);
	GROUP_TEST_AND_PUSH(AdvancedBehavior, category, o_groups);
}

//----------------------------------------------------------------------

void UITextbox::GetPropertyNames( UIPropertyNameVector &In, bool forCopy ) const
{
	In.push_back( PropertyName::MaxLength );
	In.push_back( PropertyName::MaxLines );
	In.push_back( PropertyName::OnChange );
	In.push_back( PropertyName::OnOverflow );
	In.push_back( PropertyName::Password );
	In.push_back( PropertyName::Style );
	In.push_back( PropertyName::Text );
	In.push_back( PropertyName::WrapWidth );
	In.push_back( PropertyName::Editable );
	In.push_back( PropertyName::EditableUnicode );
	In.push_back( PropertyName::TextColor );

	In.push_back( PropertyName::NumericInteger  );
	In.push_back( PropertyName::NumericFloat    );
	In.push_back( PropertyName::NumericNegative );
	In.push_back( PropertyName::IME );

	UIWidget::GetPropertyNames( In, forCopy );
}

//----------------------------------------------------------------------

bool UITextbox::SetProperty( const UILowerString & Name, const UIString &Value )
{
	if( Name == PropertyName::MaxLength )
	{
		if( Value.empty() || UIUnicode::icmp (Value,  UI_UNICODE_T ("Unlimited")) == 0)
		{
			mMaxLength = -1;
			return true;
		}
		else
			return UIUtils::ParseLong( Value, mMaxLength );
	}
	else if( Name == PropertyName::MaxLines )
	{
		if( Value.empty() || UIUnicode::icmp (Value,  UI_UNICODE_T ("Unlimited")) == 0)
		{
			mMaxLines = -1;
			return true;
		}
		else
			return UIUtils::ParseLong( Value, mMaxLines );
	}
	else if( Name == PropertyName::Password )
	{
		bool b;
		if( !UIUtils::ParseBoolean( Value, b ) )
			return false;

		if( b )
			mTextboxAttributeBits |=  TABF_Password;
		else
			mTextboxAttributeBits &= ~TABF_Password;

		return true;
	}
	else if( Name == PropertyName::Editable )
	{
		bool b;
		if( !UIUtils::ParseBoolean( Value, b ) )
			return false;

		SetEditable (b);
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
	else if( Name == PropertyName::NumericInteger )
	{
		bool b;
		if( !UIUtils::ParseBoolean( Value, b ) )
			return false;

		SetNumericInteger (b);
		return true;
	}
	else if( Name == PropertyName::NumericFloat)
	{
		bool b;
		if( !UIUtils::ParseBoolean( Value, b ) )
			return false;

		SetNumericFloat (b);
		return true;
	}
	else if( Name == PropertyName::NumericNegative)
	{
		bool b;
		if( !UIUtils::ParseBoolean( Value, b ) )
			return false;

		SetNumericNegative (b);
		return true;
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
	else if( Name == PropertyName::Style )
	{
		UIBaseObject *NewStyle = GetObjectFromPath( Value, TUITextboxStyle );

		if( NewStyle || Value.empty() )
		{
			SetStyle( reinterpret_cast<UITextboxStyle *>( NewStyle ) );
			return true;
		}
	}
	else if( Name == PropertyName::WrapWidth )
	{
		if( UIUnicode::icmp (Value,  UI_UNICODE_T ("Automatic")) == 0)
			mWrapWidth = AutomaticWrapping;
		else if( UIUnicode::icmp (Value,  UI_UNICODE_T ("None")) == 0)
			mWrapWidth = NoWrapping;
		else
			return UIUtils::ParseLong( Value, mWrapWidth );

		return true;
	}
	else if( Name == PropertyName::TextColor )
	{
		return UIUtils::ParseColor (Value, mTextColor);
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

	return UIWidget::SetProperty( Name, Value );
}

//----------------------------------------------------------------------

bool UITextbox::GetProperty( const UILowerString & Name, UIString &Value ) const
{
	if( Name == MethodName::IsEmpty )
	{
		if( mLocalText.size() == 0 )
			Value = UI_UNICODE_T("true");
		else
			Value = UI_UNICODE_T("false");

		return true;
	}
	else if( Name == PropertyName::MaxLength )
	{
		if( mMaxLength < 0 )
			Value = UI_UNICODE_T("Unlimited");
		else
			UIUtils::FormatLong( Value, mMaxLength );

		return true;
	}
	else if( Name == PropertyName::MaxLines )
	{
		if( mMaxLines < 0 )
			Value = UI_UNICODE_T("Unlimited");
		else
			UIUtils::FormatLong( Value, mMaxLines );

		return true;
	}
	else if( Name == PropertyName::Password )
		return UIUtils::FormatBoolean( Value, (mTextboxAttributeBits & TABF_Password) != 0 );
	else if( Name == PropertyName::Editable )
		return UIUtils::FormatBoolean( Value, GetEditable ());
	else if( Name == PropertyName::EditableUnicode )
		return UIUtils::FormatBoolean( Value, IsEditableUnicode ());
	else if( Name == PropertyName::NumericInteger )
		return UIUtils::FormatBoolean( Value, IsNumericInteger ());
	else if( Name == PropertyName::NumericFloat)
		return UIUtils::FormatBoolean( Value, IsNumericFloat ());
	else if( Name == PropertyName::NumericNegative)
		return UIUtils::FormatBoolean( Value, IsNumericNegative ());
	else if( Name == PropertyName::Text )
	{
		GetText( Value );
		return true;
	}
	else if( Name == PropertyName::LocalText )
	{
		GetLocalText( Value );
		return true;
	}
	else if( Name == PropertyName::Style )
	{
		if( mStyle )
		{
			GetPathTo( Value, mStyle );
			return true;
		}
	}
	else if( Name == PropertyName::WrapWidth )
	{
		if( mWrapWidth == AutomaticWrapping )
			Value = UI_UNICODE_T("Automatic");
		else if( mWrapWidth == NoWrapping )
			Value = UI_UNICODE_T("None");
		else
			UIUtils::FormatLong( Value, mWrapWidth );

		return true;
	}
	else if( Name == PropertyName::TextColor )
	{
		return UIUtils::FormatColor (Value, mTextColor);
	}
	else if ( Name == PropertyName::IME )
	{
		return UIUtils::FormatBoolean( Value, (mTextboxAttributeBits & TABF_IME) != 0 );
	}

	return UIWidget::GetProperty( Name, Value );
}

//----------------------------------------------------------------------

/*
void UITextbox::SetRect( const UIRect &In )
{
	mTextDirty = true;
	UIWidget::SetRect( In );
}

  */
//----------------------------------------------------------------------

void UITextbox::SetSize( const UISize &In )
{
	mTextDirty = true;
	UIWidget::SetSize( In );
}

//----------------------------------------------------------------------
/*
void UITextbox::SetWidth( const long In )
{
	mTextDirty = true;
	UIWidget::SetWidth( In );
}
*/

//----------------------------------------------------------------------

long UITextbox::GetCaratLine ()
{
	if( !mStyle )
		return 0;

	UITextStyle *CurrentTextStyle = mStyle->GetTextStyle( GetVisualState() );

	if( !CurrentTextStyle )
		return 0;

	return mCaratRect.top / CurrentTextStyle->GetLeading();
}

//----------------------------------------------------------------------

void UITextbox::ResetCarat ()
{
	mTextboxAttributeBits |= TABF_DrawCarat;
	mNextCaratBlinkTime    = UIClock::gUIClock().GetTime() + UISystemDependancies::Get().GetCaratBlinkTickCount();
}

//----------------------------------------------------------------------

void UITextbox::CalculateCaratRect ()
{
	if( !mStyle )
		return;

	UITextStyle *CurrentTextStyle = mStyle->GetTextStyle( GetVisualState() );
	UIString		 s;
	UIPoint			 TextSize;
	long         CaratLineNumber = 0;

	if( !CurrentTextStyle )
		return;

	// Fix many unchecked settings of mCaratIndex to illegal values
	if ((unsigned long)mCaratIndex > mLocalText.length())
		mCaratIndex = mLocalText.length();

	if( mTextDirty )
		CacheTextMeasurements();

	unsigned int caretIndex = mCaratIndex + UIManager::gUIManager().getUIIMEManager()->GetCompCaratIndex();

	if( !mRenderData.empty())
	{
		CaratLineNumber = GetRowFromOffset( caretIndex );

		if( mTextboxAttributeBits & TABF_Password )
			s.append( caretIndex, PasswordCharacter );
		else
		{
			if( caretIndex > 0 )
			{
				if( mRenderData[caretIndex - 1] == '\n' )
					CaratLineNumber++;
			}

			long AmountToCopy = caretIndex - (mRenderLinePointers[CaratLineNumber] - mRenderLinePointers[0]);

			if( AmountToCopy > 0 )
			{
				long realAmountToCopy = 0;
				long index = 0;
				UIString::const_iterator i = mRenderLinePointers[CaratLineNumber];
				UIString::const_iterator end;
				if(CaratLineNumber >= static_cast<long>((mRenderLinePointers.size() - 1)))
					end = mRenderData.end();
				else
					end = mRenderLinePointers[CaratLineNumber+1];
				bool ignoreNextEscape = false;
				for( index = 0; index != AmountToCopy; ++i, ++index )
				{
					UIString::value_type const theChar  = *i;
					
					if( !ignoreNextEscape && theChar == '\\' )
					{
						long width;
						long indentation;
						UIString::const_iterator next = CurrentTextStyle->EatEscapeSequence( i, end, ignoreNextEscape, width, indentation);

						if (next == end)
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
				realAmountToCopy = static_cast<long>(i - mRenderLinePointers[CaratLineNumber]);
				s.assign( mRenderLinePointers[CaratLineNumber], realAmountToCopy );
			}
		}
	}

	CurrentTextStyle->MeasureText( s, TextSize, UITextStyle::UseLastCharAdvance, true );

	mCaratRect.left   = TextSize.x;
	mCaratRect.top    = CaratLineNumber * CurrentTextStyle->GetLeading();

	mCaratRect.right  = mCaratRect.left + mStyle->GetCaratWidth();
	mCaratRect.bottom = mCaratRect.top + CurrentTextStyle->GetLeading();

	//-----------------------------------------------------------------
	//-- scroll the widget horizontally

	UIPoint ScrollLocation( GetScrollLocation() );
	UISize  ScrollExtent;
	GetScrollExtent (ScrollExtent);

	const UIRect TextPadding (mStyle ? mStyle->GetTextPadding () : UIRect ());
	const long horizontalPadding = TextPadding.left + TextPadding.right;

	if (mCaratRect.left < ScrollLocation.x)
		ScrollLocation.x = mCaratRect.left;
	else if (mCaratRect.right > (ScrollLocation.x + GetWidth () - horizontalPadding - mCaratRect.Width ()))
		ScrollLocation.x += mCaratRect.right - (ScrollLocation.x + GetWidth () - horizontalPadding);

	if ((ScrollLocation.x + GetWidth ()) > ScrollExtent.x)
		ScrollLocation.x = ScrollExtent.x - GetWidth ();

	SetScrollLocation (ScrollLocation);
}

//----------------------------------------------------------------------

void UITextbox::ScrollCaratOnScreen ()
{
	if( !mStyle )
		return;

	UITextStyle *CurrentTextStyle = mStyle->GetTextStyle( GetVisualState() );
	UIString		 s;
	UIPoint			 TextSize;
	long         CaratLineNumber = 0;

	if( !CurrentTextStyle )
		return;

	if( mTextDirty )
		CacheTextMeasurements();

	if( !mLocalText.empty() )
	{
		UISize  PageSize, LineSize;
		long    CaratPixelOffset;
		long    Leading;

		Leading          = CurrentTextStyle->GetLeading();
		CaratLineNumber  = GetRowFromOffset( mCaratIndex );
		CaratPixelOffset = CaratLineNumber * Leading;

		GetScrollSizes( PageSize, LineSize );
		UIPoint CurrentScrollLocation (GetScrollLocation());

		if( CaratPixelOffset < CurrentScrollLocation.y )
		{
			CurrentScrollLocation.y = CaratPixelOffset;
		}
		else if( CaratPixelOffset + Leading > CurrentScrollLocation.y + PageSize.y )
		{
			CurrentScrollLocation.y = CaratPixelOffset + Leading - PageSize.y;
		}

		SetScrollLocation( CurrentScrollLocation );
	}

	ResetCarat ();
}

//----------------------------------------------------------------------

void UITextbox::CacheTextMeasurements ()
{
	if( mStyle )
	{
		UITextStyle *CurrentTextStyle = mStyle->GetTextStyle( GetVisualState() );

		if( CurrentTextStyle )
		{
			UIPoint ScrollExtent;
			UIRect  TextPadding;
			long    WrapWidth;

			if( mWrapWidth == AutomaticWrapping )
				WrapWidth = GetWidth() - mStyle->GetTextPadding().left - mStyle->GetTextPadding().right;
			else if( mWrapWidth == NoWrapping )
				WrapWidth = INT_MAX;
			else
				WrapWidth = mWrapWidth;


			mLinePointers.clear();
			mLineWidths.clear();

			// Cook data, generate escape characters for renderer as needed.
			mRenderData.clear ();//erase( mRenderData.begin(), mRenderData.end() );
			mRenderLinePointers.clear();
			mRenderLineWidths.clear();

			int offset = 0;
			unsigned char curAttr = 0;
			int c = 0;
			UIString::const_iterator j;
				
			for( UIString::const_iterator i = mLocalText.begin(); i != mLocalText.end(); ++i )
			{
				UIString::value_type ch = *i;
								
				if (offset == mCaratIndex)
				{
					for (j = mComposition.begin(), c = 0; j != mComposition.end(); ++j, ++c)
					{
						unsigned char attr = mCompositionAttrs[c];
						if((c == 0) || (attr != curAttr))
						{
							curAttr = attr;
							mRenderData.append(UIIMEManagerNamespace::colorModifiers[attr]);
						}
						UIString::value_type ch2 = *j;

						mRenderData.append( 1, ch2);
					}
					if(!mComposition.empty())
						mRenderData.append(L"\\#.");
				}

				if( ch == '\\' )
					mRenderData.append( 2, ch );
				else
					mRenderData.append( 1, ch );

				offset++;
			}

			if (offset == mCaratIndex)
			{
				for (j = mComposition.begin(), c = 0; j != mComposition.end(); ++j, ++c)
				{
					UIString::value_type ch2 = *j;
					unsigned char attr = mCompositionAttrs[c];
					if((c == 0) || (attr != curAttr))
					{
						curAttr = attr;
						mRenderData.append(UIIMEManagerNamespace::colorModifiers[attr]);
					}
					mRenderData.append( 1, ch2);
				}
				if(!mComposition.empty())
					mRenderData.append(L"\\#.");
			}

			CurrentTextStyle->GetWrappedTextInfo( mRenderData, mMaxLines, WrapWidth, ScrollExtent.x, ScrollExtent.y,
								&mLinePointers, &mLineWidths, UITextStyle::UseLastCharAdvance, false );

			// Get cooked data info
			CurrentTextStyle->GetWrappedTextInfo( mRenderData, mMaxLines, WrapWidth, ScrollExtent.x, ScrollExtent.y,
				                                    &mRenderLinePointers, &mRenderLineWidths );

			mStyle->GetTextPadding( TextPadding );

			ScrollExtent.x += TextPadding.left + TextPadding.right + mStyle->GetCaratWidth();
			ScrollExtent.y += TextPadding.top + TextPadding.bottom;

			SetScrollExtent( ScrollExtent );
			mTextDirty = false;

			ScrollCaratOnScreen();
			CalculateCaratRect();
		}
	}
}

//----------------------------------------------------------------------

long UITextbox::GetRowFromOffset( long OffsetFromBase ) const
{
	if( mTextDirty )
		const_cast<UITextbox *>(this)->CacheTextMeasurements();

	UIStringConstIteratorVector::const_iterator i = mRenderLinePointers.begin();

	long RowNumber = 0;

	for( ++i; i != mRenderLinePointers.end(); ++i )
	{
		if( *i - *mRenderLinePointers.begin() >= OffsetFromBase )
			break;
		else
			RowNumber++;
	}

	return RowNumber;
}

//----------------------------------------------------------------------

long UITextbox::GetXCoordFromRowAndOffset( long row, long OffsetFromBase ) const
{
	if( !mStyle )
		return 0;

	const UITextStyle * const CurrentTextStyle = mStyle->GetTextStyle( GetVisualState() );

	if( !CurrentTextStyle )
		return 0;

	if( mTextDirty )
		const_cast<UITextbox *>(this)->CacheTextMeasurements();

	assert (row >= 0 && static_cast<size_t>(row) < mRenderLinePointers.size ());
	assert (OffsetFromBase >= 0);

	UIString::const_iterator s = mRenderLinePointers[row];
	long        w = 0;

	OffsetFromBase -= std::distance (mRenderLinePointers[0], s);

	while( OffsetFromBase > 0)
	{
		const UIFontCharacter * const Glyph = CurrentTextStyle->GetCharacter( *s );

		if( Glyph )
			w += Glyph->GetAdvance();

		++s;
		--OffsetFromBase;
	}

	return w;
}

//----------------------------------------------------------------------

void UITextbox::ModifySelection( long NewSelectionFloatingEnd )
{
	if( mSelectionStart == -1 )
	{
		// New selection started
		mSelectionStart		 = NewSelectionFloatingEnd;
		mSelectionEnd			 = NewSelectionFloatingEnd;
		mSelectionFixedEnd = NewSelectionFloatingEnd;
	}
	else
	{
		if( NewSelectionFloatingEnd < mSelectionFixedEnd )
		{
			mSelectionStart = NewSelectionFloatingEnd;
			mSelectionEnd   = mSelectionFixedEnd;
		}
		else
		{
			mSelectionStart = mSelectionFixedEnd;
			mSelectionEnd   = NewSelectionFloatingEnd;
		}
	}
}

//----------------------------------------------------------------------

bool UITextbox::ClearSelection ()
{
	if (mSelectionStart != -1 ||
		mSelectionEnd != -1 ||
		mSelectionFixedEnd != -1)
	{
		mSelectionStart    = -1;
		mSelectionEnd      = -1;
		mSelectionFixedEnd = -1;
		return true;
	}
	return false;
}

//----------------------------------------------------------------------

void UITextbox::DeleteSelection( bool dingOnFailure )
{
	if( GetEditable () && mSelectionStart != -1 )
	{
		mLocalText.erase( mSelectionStart, mSelectionEnd - mSelectionStart );
		mCaratIndex = mSelectionStart;
		ClearSelection();
		mTextDirty = true;
	}
	else
	{
		if (dingOnFailure)
			Ding ();
	}
}

//----------------------------------------------------------------------

void UITextbox::CopySelectionToClipboard ()
{
	if( mTextboxAttributeBits & TABF_Password )
	{
		UIClipboard::gUIClipboard().EraseText ();
	}
	else if( mSelectionStart != mSelectionEnd )
	{
		UIString DataInSelection( mLocalText, mSelectionStart, mSelectionEnd - mSelectionStart );

		UIClipboard::gUIClipboard().SetText(DataInSelection);
	}
}

//----------------------------------------------------------------------

void UITextbox::PasteFromClipboard ()
{
	if (!GetEditable ())
	{
		Ding ();
		return;
	}

	UIString TextOnClipboard;
	UIClipboard::gUIClipboard().GetText( TextOnClipboard );

	if(!IsEditableUnicode())
	{
		if(Unicode::isUnicode(TextOnClipboard))
		{
			Ding();
			return;
		}
	}

	unsigned int space = mMaximumCharacters - Unicode::utf8Size(mLocalText);
	if (space == 0)
	{
		Ding();
		return;
	}

	Unicode::truncateToUTF8Size(TextOnClipboard, space);

	if (TextOnClipboard.size() == 0)
	{
		Ding();
		return;
	}

	mLocalText.insert( mCaratIndex, TextOnClipboard );
	mTextDirty = true;

	mCaratIndex += TextOnClipboard.size();

	//-- truncate integers to 9 digits
	if (IsNumericInteger () && mLocalText.size () > static_cast<size_t>(mMaxIntegerLength))
	{
		mLocalText.erase (static_cast<size_t>(mMaxIntegerLength), static_cast<size_t>(Unicode::String::npos));
		Ding ();
		mCaratIndex = std::min (mCaratIndex, static_cast<long>(mLocalText.size ()));
	}

	ScrollCaratOnScreen();
	CalculateCaratRect();
}

//-----------------------------------------------------------------

void UITextbox::SetEditable (bool b)
{
	if (b)
		mTextboxAttributeBits |= TABF_Editable;
	else
		mTextboxAttributeBits &= ~TABF_Editable;
}

void UITextbox::SetIMEEnabled (bool b)
{
	if (b)
		mTextboxAttributeBits |= TABF_IME;		
	else
		mTextboxAttributeBits &= ~TABF_IME;
}

//----------------------------------------------------------------------

void UITextbox::SetTextColor (const UIColor & color)
{
	mTextColor = color;
}

//-----------------------------------------------------------------

void  UITextbox::SetNumericInteger (bool b)
{
	if (b)
	{
		mTextboxAttributeBits |= TABF_NumericInteger;
		SetNumericFloat (false);
		ValidateText ();
	}
	else
		mTextboxAttributeBits &= ~TABF_NumericInteger;
}

//-----------------------------------------------------------------

void  UITextbox::SetNumericFloat   (bool b)
{
	if (b)
	{
		mTextboxAttributeBits |= TABF_NumericFloat;
		SetNumericInteger (false);
		ValidateText ();
	}
	else
		mTextboxAttributeBits &= ~TABF_NumericFloat;
}

//-----------------------------------------------------------------

void  UITextbox::SetNumericNegative   (bool b)
{
	if (b)
	{
		mTextboxAttributeBits |= TABF_NumericNegative;
		ValidateText ();
	}
	else
		mTextboxAttributeBits &= ~TABF_NumericNegative;
}

//-----------------------------------------------------------------

void UITextbox::SetEditableUnicode (bool b)
{
	if (b)
	{
		mTextboxAttributeBits |= TABF_EditableUnicode;
		ValidateText ();
	}
	else
		mTextboxAttributeBits &= ~TABF_EditableUnicode;
}

//-----------------------------------------------------------------

void UITextbox::SetMaxIntegerLength (unsigned char maxIntegerLength)
{
	mMaxIntegerLength = std::max(maxIntegerLength, max_integer_length);
}

//-----------------------------------------------------------------

int   UITextbox::GetNumericIntegerValue () const
{
	return atoi (Unicode::wideToNarrow (mLocalText).c_str ());
}

//-----------------------------------------------------------------

float UITextbox::GetNumericFloatValue () const
{
	return static_cast<float>(atof (Unicode::wideToNarrow (mLocalText).c_str ()));
}

//-----------------------------------------------------------------

void UITextbox::Ding () const
{
	UIManager::gUIManager ().PlaySoundGenericNegative ();
}

//-----------------------------------------------------------------

void UITextbox::ValidateText ()
{
	// @todo: transform text to match flags
}

//----------------------------------------------------------------------

bool UITextbox::performDeleteKey (bool doHandleTextChanged, bool isShiftDown)
{
	long OldCaratIndex = mCaratIndex;
	bool changed = false;

	if( mSelectionStart != -1 )
	{
		DeleteSelection(true);
		changed = true;
	}
	else if( mCaratIndex != static_cast<long>(mLocalText.size()) && GetEditable ())
	{
		mLocalText.erase( mCaratIndex, 1 );
		changed = true;
	}
	else
		Ding ();

	if (doHandleTextChanged)
		handleTextChanged (OldCaratIndex, changed, isShiftDown);

	return changed;
}

//----------------------------------------------------------------------

bool UITextbox::performBackspaceKey (bool doHandleTextChanged, bool isShiftDown)
{
	long OldCaratIndex = mCaratIndex;
	bool changed = false;

	if( mSelectionStart != -1 )
	{
		DeleteSelection(true);
		changed = true;
	}
	else if( mCaratIndex != 0 && GetEditable ())
	{
		mLocalText.erase( --mCaratIndex, 1 );
		changed = true;
	}
	else
		Ding ();

	if (doHandleTextChanged)
		handleTextChanged (OldCaratIndex, changed, isShiftDown);

	return changed;
}

//----------------------------------------------------------------------

void UITextbox::handleTextChanged (size_t OldCaratIndex, bool TextChanged, bool isShiftDown)
{
	if (isShiftDown)
	{
		if (!TextChanged )
		{
			if( mSelectionStart == -1 )
				ModifySelection( OldCaratIndex );

			ModifySelection( mCaratIndex );
		}
	}
	else
		ClearSelection ();

	if( TextChanged )
	{
		mTextDirty = true;
		SendCallback( &UIEventCallback::OnTextboxChanged, PropertyName::OnChange );
		SendDataChangedCallback (PropertyName::LocalText, mLocalText);

		if (mStyle && (mTextboxAttributeBits & TABF_KeyClickOn) != 0)
			mStyle->PlaySoundKeyclick ();
	}

	ResetCarat();
	ScrollCaratOnScreen ();
}

void UITextbox::ConsumeNextCharacter()
{
	mConsumeNextCharacter = true;
}

//----------------------------------------------------------------------

void UITextbox::SetKeyClickOn   (bool b)
{
	if (b)
		mTextboxAttributeBits |= TABF_KeyClickOn;
	else
		mTextboxAttributeBits &= ~TABF_KeyClickOn;
}

//----------------------------------------------------------------------

void UITextbox::SetTextColorOverride   (bool b)
{
	if (b)
		mTextboxAttributeBits |= TABF_TextColorOverride;
	else
		mTextboxAttributeBits &= ~TABF_TextColorOverride;
	
}

//----------------------------------------------------------------------

bool UITextbox::CanSelect () const
{
	return UIWidget::CanSelect () && GetEditable ();
}

//----------------------------------------------------------------------

void UITextbox::SetMaximumCharacters(int const maximumCharacters)
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

int UITextbox::GetMaximumCharacters() const
{
	return mMaximumCharacters;
}

//----------------------------------------------------------------------

bool UITextbox::IsAtMaximumCharacters() const
{
	return (GetCharacterCount() >= mMaximumCharacters);
}

//----------------------------------------------------------------------

int UITextbox::GetCharacterCount() const
{
	return static_cast<int>(Unicode::wideToUTF8(mLocalText).size());
}

//----------------------------------------------------------------------
