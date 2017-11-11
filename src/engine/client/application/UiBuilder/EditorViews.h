#ifndef INCLUDED_EditorViews_H
#define INCLUDED_EditorViews_H

class SerializedObjectBuffer;

#include <list>

class EditorViews
{
public:

	// ---------------------------------------
	// Cursor types
	enum CursorType {
		 CT_Normal
		,CT_Crosshair
		,CT_Hand
		,CT_SizeAll  // Four-pointed arrow pointing north, south, east, and west
		,CT_SizeNESW // Double-pointed arrow pointing northeast and southwest
		,CT_SizeNS   // Double-pointed arrow pointing north and south
		,CT_SizeNWSE // Double-pointed arrow pointing northwest and southeast
		,CT_SizeWE   // Double-pointed arrow pointing west and east
		,CT_Wait     // Hour-glass

		,CT_COUNT
	};

	virtual void setCursor(CursorType showType)=0;
	virtual void setCapture()=0;
	virtual void releaseCapture()=0;
	virtual void redrawViews(bool synchronous)=0;
	virtual void copyObjectBuffersToClipboard(const std::list<SerializedObjectBuffer> &i_buffers)=0;
	virtual bool pasteObjectBuffersFromClipboard(std::list<SerializedObjectBuffer> &o_buffers)=0;

	virtual void onRootSize(int width, int height)=0;
};

#endif
