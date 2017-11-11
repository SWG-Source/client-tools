#ifndef __UIMESSAGE_H__
#define __UIMESSAGE_H__

#include "UIMessageModifierData.h"
#include "UITypes.h"

class UIWidget;

struct UIMessage
{
public:

	static unsigned short BackSpace;
	static unsigned short Copy;
	static unsigned short Cut;
	static unsigned short Delete;
	static unsigned short DownArrow;
	static unsigned short End;
	static unsigned short Enter;
	static unsigned short Escape;
	static unsigned short Home;
	static unsigned short Insert;
	static unsigned short LeftArrow;
	static unsigned short PageDown;
	static unsigned short PageUp;
	static unsigned short Paste;
	static unsigned short RightArrow;
	static unsigned short SelectAll;
	static unsigned short Space;
	static unsigned short Tab;
	static unsigned short Undo;
	static unsigned short UpArrow;

	UIMessage();

	// Message type functions
	bool IsKeyMessage         () const;
	bool IsMouseMessage       () const;
	bool IsMouseButtonMessage () const;
	bool MovesFocus           () const;
	bool IsDragMessage        () const;

	bool IsCharacterMessage   () const;
	bool IsMouseDownCommand   () const;
	bool IsMouseUpCommand     () const;

	// Message interpretation functions
	bool IsCutCommand         () const;
	bool IsCopyCommand        () const;
	bool IsPasteCommand       () const;
	bool IsUndoCommand        () const;
	bool IsSelectAllCommand   () const;

	bool IsIMEMessage					() const;

#if _WIN32

	bool CreateFromWindowsMessage( unsigned int uMsg, long wParam, long lParam );

#endif // WIN32

	enum MessageType
	{
		KeyFirst,
			KeyDown,
			KeyUp,
			KeyRepeat,
			Character,
		KeyLast,	

		MouseFirst,
			LeftMouseDown,
			MiddleMouseDown,
			RightMouseDown,

		MouseLastFocusChanger,

			LeftMouseDoubleClick,
			MiddleMouseDoubleClick,
			RightMouseDoubleClick,

			LeftMouseUp,
			MiddleMouseUp,
			RightMouseUp,
		
		MouseLastButton,

			MouseMove,
			MouseEnter,
			MouseExit,
			MouseWheel,

			//-- these 2 messages aren't necessily fired by the mouse, but
			//-- they are always handled like mouse messages

			ContextRequest,                  //-- the user asked for the context menu

		MouseLast,

		DragFirst,
			DragStart,
			DragEnd,
			DragCancel,
			DragOver,
		DragLast,

		IMEFirst,
			IMEComposition,
			IMEChangeCandidate,
			IMECloseCandidate,
			IMEEndComposition,
		IMELast,

		NumMessages
	};
	
	MessageType Type;

	typedef UIMessageModifierData ModifierData;

	ModifierData Modifiers;

	// @todo: this must become unsigned
	unsigned short Keystroke;
	short          Data;
	UIPoint        MouseCoords;

	UIWidget *DragSource;
	UIWidget *DragObject;
	UIWidget *DragTarget;
};

//----------------------------------------------------------------------

inline bool UIMessage::IsKeyMessage       () const
{
	return (Type >= KeyFirst) && (Type <= KeyLast); 
}

//----------------------------------------------------------------------

inline bool UIMessage::IsMouseMessage     () const
{
	return (Type >= MouseFirst) && (Type <= MouseLast); 
}

inline bool UIMessage::IsMouseButtonMessage () const
{
	return (Type >= MouseFirst) && (Type <= MouseLastButton); 
}

//----------------------------------------------------------------------

inline bool UIMessage::IsDragMessage      () const
{
	return (Type >=
		DragFirst) && (Type <= DragLast); 
}

//----------------------------------------------------------------------

inline bool UIMessage::IsCharacterMessage () const
{
	return Type == Character; 
}

//----------------------------------------------------------------------

inline bool UIMessage::IsMouseDownCommand () const
{
	return (Type == LeftMouseDown || Type == MiddleMouseDown || Type == RightMouseDown); 
}

//----------------------------------------------------------------------

inline bool UIMessage::IsMouseUpCommand   () const
{
	return (Type == LeftMouseUp || Type == MiddleMouseUp || Type == RightMouseUp); 
}

//----------------------------------------------------------------------

#endif // __UIMESSAGE_H__
