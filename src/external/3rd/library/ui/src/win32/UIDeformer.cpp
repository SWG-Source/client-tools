#include "_precompile.h"

#include "UIDeformer.h"

#include "UICanvas.h"
#include "UIClock.h"
#include "UIPropertyDescriptor.h"
#include "UIUtils.h"
#include "UIWidget.h"

#include <cfloat>
#include <cmath>
#include <vector>

const char *UIDeformer::TypeName = "Deformer";

//======================================================================================
#define _TYPENAME UIDeformer

namespace UIDeformerNamespace
{
}
using namespace UIDeformerNamespace;
//======================================================================================

UIDeformer::UIDeformer() :
UIEffector(),
UINotification(),
UIWatchable(),
mTarget(NULL)
{
}

//=============================================================================

UIDeformer::~UIDeformer()
{
	mTarget = NULL;
}

//=============================================================================

const char *UIDeformer::GetTypeName() const
{
	return TypeName;
}

//=============================================================================

void UIDeformer::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	UIEffector::GetPropertyGroups(o_groups, category);
}

//----------------------------------------------------------------------

void UIDeformer::GetPropertyNames( UIPropertyNameVector &In, bool forCopy ) const
{
	UIEffector::GetPropertyNames( In, forCopy );
}

//=============================================================================

bool UIDeformer::SetProperty( const UILowerString & Name, const UIString &Value )
{
	return UIEffector::SetProperty( Name, Value );
}

//=============================================================================

bool UIDeformer::GetProperty( const UILowerString & Name, UIString &Value ) const
{
	return UIEffector::GetProperty( Name, Value );
}

//=============================================================================

UIEffector::EffectResult UIDeformer::OnCreate( UIBaseObject * const theObject )
{
	if (theObject->IsA(TUIWidget) == true)
	{
		SetTarget(static_cast<UIWidget *>(theObject));
	}

	return Continue;
}

//=============================================================================

UIEffector::EffectResult UIDeformer::Effect( UIBaseObject * const)
{
 	return Continue;
}

//=============================================================================

void UIDeformer::SetTarget(UIWidget * const newTarget)
{
	if (GetTarget() != newTarget)
	{
		UIWidget * const oldTarget = mTarget;
		mTarget = newTarget;
		
		OnTargetChanged(newTarget, oldTarget);
	
		if (newTarget)
		{
			PlaySoundActivate();
		}
		else
		{
			PlaySoundDeactivate();
		}
	}
}

//=============================================================================

void UIDeformer::OnTargetChanged(UIWidget * newTarget, UIWidget * oldTarget)
{
	if (oldTarget && oldTarget->IsA(TUIWidget))
	{
		if (oldTarget == mTarget)
		{
			oldTarget->SetSize(mOriginalSize);
		}

		oldTarget->Detach(0);
		static_cast<UIWidget*>(oldTarget)->SetDeformer(0);
	}

	if (newTarget && newTarget->IsA(TUIWidget))
	{
		newTarget->Attach(0);
		mOriginalSize = newTarget->GetSize();
		static_cast<UIWidget*>(newTarget)->SetDeformer(this);
	}
}

//=============================================================================

UIWidget * UIDeformer::GetTarget() const
{
	return mTarget;
}

//=============================================================================

void UIDeformer::Notify( UINotificationServer * /*NotifyingObject*/, UIBaseObject * /*ContextObject*/, Code /*NotificationCode*/ )
{

}

//=============================================================================

bool UIDeformer::Detach(UIBaseObject const *)
{
	SetTarget(0);

	return UIEffector::Detach(0);
}

//=============================================================================

UIFloatPoint UIDeformer::GetDeformedScale() const
{
	return UIFloatPoint::one;
}


//=============================================================================
