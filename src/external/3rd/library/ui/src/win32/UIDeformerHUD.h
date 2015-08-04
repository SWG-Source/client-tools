#ifndef __UIDeformerHUD_H__
#define __UIDeformerHUD_H__

//=============================================================================

#include "UIDeformer.h"

//=============================================================================

class UIDeformerHUD : public UIDeformer
{
public:
	//----------------------------------------------------------------------

	static const char			*TypeName;

	class PropertyName
	{
	public:
		static const UILowerString WarpScale;
		static const UILowerString WarpShift;
	};

	//----------------------------------------------------------------------

	UIDeformerHUD();
	virtual ~UIDeformerHUD();

	virtual bool IsA( const UITypeID Type ) const;
	virtual const char * GetTypeName( void ) const;
	virtual UIBaseObject * Clone() const;

	virtual void GetPropertyGroups(UIPropertyGroupVector &, UIPropertyCategories::Category = UIPropertyCategories::C_ALL_CATEGORIES) const;
	virtual void GetPropertyNames( UIPropertyNameVector &, bool forCopy ) const;
	virtual bool SetProperty( const UILowerString & Name, const UIString &Value );
	virtual bool GetProperty( const UILowerString & Name, UIString &Value ) const;		

	virtual bool Deform(UICanvas & canvas, UIFloatPoint const * points, UIFloatPoint * out, size_t count);

private:
	UIDeformerHUD(UIDeformerHUD const & rhs);
	UIDeformerHUD const & operator=(UIDeformerHUD const & rhs);

	float mWarpScale;
	float mWarpShift;
};

//=============================================================================

inline bool UIDeformerHUD::IsA( const UITypeID Type ) const
{
	return (Type == TUIDeformerHUD) || UIDeformer::IsA( Type );
};

#endif // __UIDeformerHUD_H__