#ifndef __UIGBUFFER_H__
#define __UIGBUFFER_H__

#ifndef __UIWIDGET_H__
#include "UIWidget.h"
#endif // __UIWIDGET_H__

#ifndef __UIGBUFFER_COMMAND_H__
#include "UIGBuffer_Command.h"
#endif // __UIGBUFFER_COMMAND_H__

//======================================================================================
class UIGBuffer : public UIWidget
{
public:

	static const char * TypeName;

                            UIGBuffer();
	virtual                ~UIGBuffer();

	virtual bool            IsA( const UITypeID ) const;
	virtual const char     *GetTypeName() const;
	virtual UIBaseObject   *Clone() const;

	virtual bool            CanSelect() const;
	virtual bool            WantsMessage( const UIMessage & ) const;

	virtual UIStyle        *GetStyle() const { return 0; };

	virtual bool            ProcessMessage( const UIMessage & );
	virtual void            Render( UICanvas & ) const;

			void            ClearBuffer();

			void            AddQuad(UIPoint const & p1, UIPoint const & p2, UIPoint const & p3, UIPoint const & p4, UIColor const & color, float opacity, float buffer);
			void            AddLine(UIPoint const & p1, UIPoint const & p2, UIColor const & color,  float opacity, float buffer);
			void            AddWidget(UIWidget * widget, float buffer);
private:
							UIGBuffer( UIGBuffer const & );
	UIGBuffer              &operator = ( UIGBuffer const & );

private:

	UIGBuffer_Command::List mCommandList;

};

#endif // __UIGBUFFER_H__