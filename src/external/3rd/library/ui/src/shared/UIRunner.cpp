//======================================================================
//
// UIRunner.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "_precompile.h"
#include "UIRunner.h"

#include "UIClock.h"
#include "UIEffector.h"
#include "UIManager.h"
#include "UIPropertyDescriptor.h"
#include "UIUtils.h"
#include <algorithm>
#include <cstdlib>
#include <vector>

//======================================================================

const char * const UIRunner::TypeName = "Runner";

const UILowerString UIRunner::PropertyName::RunAdvance           = UILowerString ("RunAdvance");
const UILowerString UIRunner::PropertyName::RunDelayAdvance      = UILowerString ("RunDelayAdvance");
const UILowerString UIRunner::PropertyName::RunDelayRestart      = UILowerString ("RunDelayRestart");
const UILowerString UIRunner::PropertyName::RunLoops             = UILowerString ("RunLoops");
const UILowerString UIRunner::PropertyName::RunPaused            = UILowerString ("RunPaused");
const UILowerString UIRunner::PropertyName::RunReversing         = UILowerString ("RunReversing");
const UILowerString UIRunner::PropertyName::RunSimultaneous      = UILowerString ("RunSimultaneous");
const UILowerString UIRunner::PropertyName::RunSpacing           = UILowerString ("RunSpacing");
const UILowerString UIRunner::PropertyName::RunWrapAround        = UILowerString ("RunWrapAround");
const UILowerString UIRunner::PropertyName::RunOffDuringRestart  = UILowerString ("RunOffDuringRestart");
const UILowerString UIRunner::PropertyName::RunEffectorsOn       = UILowerString ("RunEffectorsOn");
const UILowerString UIRunner::PropertyName::RunEffectorsOff      = UILowerString ("RunEffectorsOff");
const UILowerString UIRunner::PropertyName::RunRandomizedPosition= UILowerString ("RunRandomizedPosition");


//----------------------------------------------------------------------

const UILowerString UIRunner::MethodName::RunRestart;

//======================================================================================
#define _TYPENAME UIRunner

namespace UIRunnerNamespace
{
	//================================================================
	// Behavior category
	_GROUPBEGIN(Behavior)
		_DESCRIPTOR(RunAdvance, "", T_int),
		_DESCRIPTOR(RunDelayAdvance, "", T_int),
		_DESCRIPTOR(RunDelayRestart, "", T_int),
		_DESCRIPTOR(RunLoops, "", T_int),
		_DESCRIPTOR(RunPaused, "", T_bool),
		_DESCRIPTOR(RunReversing, "", T_bool),
		_DESCRIPTOR(RunSimultaneous, "", T_int),
		_DESCRIPTOR(RunSpacing, "", T_int),
		_DESCRIPTOR(RunWrapAround, "", T_bool),
		_DESCRIPTOR(RunOffDuringRestart, "", T_bool),
		_DESCRIPTOR(RunEffectorsOn, "", T_string),
		_DESCRIPTOR(RunEffectorsOff, "", T_string),
		_DESCRIPTOR(RunRandomizedPosition, "", T_bool),
	_GROUPEND(Behavior, 1, 0);
	//================================================================
}
using namespace UIRunnerNamespace;
//======================================================================================

UIRunner::UIRunner   () :
UIPage               (),
UINotification       (),
mRunSimultaneous     (1),
mRunSpacing          (0),
mRunAdvance          (1),
mRunLoops            (0),
mRunReversing        (false),
mRunPaused           (true),
mRunWrapAround(false),
mRunOffDuringRestart(false),
mRunDelayRestart     (1),
mRunDelayAdvance     (1),
mRunTimeNextRestart  (0),
mRunTimeNextAdvance  (0),
mRunLoopsRemaining   (0),
mRunProgress         (0),
mRunRandomizedPostion(false),
mStates              (new StateVector),
mLastUpdate          (0),
mLoopFinished        (false),
mProgressForward     (true),
mCurrentNumValidProgressSteps (0),
mCurrentNumSimultaneous       (0),
mSpacings                     (new IntVector),
mEffectorsOn                  (new EffectorVector),
mEffectorsOff                 (new EffectorVector)
{
}

//----------------------------------------------------------------------

UIRunner::~UIRunner()
{
	UIClock::gUIClock ().StopListening  (this);

	delete mStates;
	mStates = 0;

	delete mSpacings;
	mSpacings = 0;

	std::for_each (mEffectorsOn->begin (),  mEffectorsOn->end (), DetachFunctor (0));
	std::for_each (mEffectorsOff->begin (), mEffectorsOff->end (), DetachFunctor (0));

	delete mEffectorsOff;
	mEffectorsOff = 0;

	delete mEffectorsOn;
	mEffectorsOn = 0;
}

//----------------------------------------------------------------------

bool UIRunner::IsA         (const UITypeID type) const
{
	return type == TUIRunner || UIPage::IsA (type);
}

//----------------------------------------------------------------------

UIBaseObject * UIRunner::Clone       () const
{
	return new UIRunner;
}

//----------------------------------------------------------------------
void UIRunner::GetLinkPropertyNames  (UIPropertyNameVector & in) const
{
	in.push_back (PropertyName::RunEffectorsOn);
	in.push_back (PropertyName::RunEffectorsOff);

	UIPage::GetLinkPropertyNames (in);
}

//----------------------------------------------------------------------

void UIRunner::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	UIPage::GetPropertyGroups(o_groups, category);
	GROUP_TEST_AND_PUSH(Behavior, category, o_groups);
}

//----------------------------------------------------------------------

void UIRunner::GetPropertyNames  (UIPropertyNameVector & in, bool forCopy) const
{
	in.push_back (PropertyName::RunAdvance         );
	in.push_back (PropertyName::RunDelayAdvance    );
	in.push_back (PropertyName::RunDelayRestart    );
	in.push_back (PropertyName::RunLoops           );
	in.push_back (PropertyName::RunPaused          );
	in.push_back (PropertyName::RunReversing       );
	in.push_back (PropertyName::RunSimultaneous    );
	in.push_back (PropertyName::RunSpacing  );
	in.push_back (PropertyName::RunWrapAround      );
	in.push_back (PropertyName::RunOffDuringRestart);
	in.push_back (PropertyName::RunEffectorsOn);
	in.push_back (PropertyName::RunEffectorsOff);
	in.push_back (PropertyName::RunRandomizedPosition);

	UIPage::GetPropertyNames (in, forCopy);
}

//----------------------------------------------------------------------

bool UIRunner::SetProperty (const UILowerString & Name, const UIString &Value)
{
	if (Name == PropertyName::RunAdvance         )
	{
		return UIUtils::ParseInteger (Value, mRunAdvance);
	}
	else if (Name == PropertyName::RunDelayAdvance    )
	{
		int ms = 0;
		if (UIUtils::ParseInteger (Value, ms))
		{
			SetRunDelayAdvance (ms);
			return true;
		}

		return false;
	}
	else if (Name == PropertyName::RunDelayRestart    )
	{
		if (UIUtils::ParseInteger (Value, mRunDelayRestart))
		{
			mRunDelayRestart = mRunDelayRestart * 60 / 1000;
			return true;
		}

		return false;
	}
	else if (Name == PropertyName::RunLoops           )
	{
		if (UIUtils::ParseInteger (Value, mRunLoops))
		{
			if (!mRunPaused)
			{
				if (mRunLoops == 0)
					mRunLoopsRemaining = -1;
				else if (mRunLoops > 0)
					mRunLoopsRemaining = mRunLoops;
				else
					mRunLoopsRemaining = GetRandom (1, -mRunLoops);
			}
			
			return true;
		}
		return false;
	}
	else if (Name == PropertyName::RunPaused          )
	{
		bool b = false;
		if (UIUtils::ParseBoolean (Value, b))
		{
			SetPaused (b);
			return true;
		}
		return false;
	}
	else if (Name == PropertyName::RunReversing       )
	{
		return UIUtils::ParseBoolean (Value, mRunReversing);
	}
	else if (Name == PropertyName::RunSimultaneous    )
	{
		return UIUtils::ParseInteger (Value, mRunSimultaneous);
	}
	else if (Name == PropertyName::RunSpacing)
	{
		return UIUtils::ParseInteger (Value, mRunSpacing);
	}
	else if (Name == PropertyName::RunWrapAround      )
	{
		return UIUtils::ParseBoolean (Value, mRunWrapAround);
	}
	else if (Name == PropertyName::RunOffDuringRestart)
	{
		return UIUtils::ParseBoolean (Value, mRunOffDuringRestart);
	}
	else if (Name == PropertyName::RunEffectorsOn)
	{
		PopulateEffectors (*mEffectorsOn, Value);
		// fall through
	}
	else if (Name == PropertyName::RunEffectorsOff)
	{
		PopulateEffectors (*mEffectorsOff, Value);
		// fall through
	}
	else if (Name == PropertyName::RunRandomizedPosition)
	{
		return UIUtils::ParseBoolean (Value, mRunRandomizedPostion);
	}

	return UIPage::SetProperty (Name, Value);
}

//----------------------------------------------------------------------

bool UIRunner::GetProperty (const UILowerString & Name, UIString &Value) const
{
	if (Name == PropertyName::RunAdvance         )
	{
		return UIUtils::FormatInteger (Value, mRunAdvance);
	}
	else if (Name == PropertyName::RunDelayAdvance)
	{
		return UIUtils::FormatInteger (Value, mRunDelayAdvance * 1000 / 60 );
	}
	else if (Name == PropertyName::RunDelayRestart)
	{
		return UIUtils::FormatInteger (Value, mRunDelayRestart * 1000 / 60);
	}
	else if (Name == PropertyName::RunLoops           )
	{
		return UIUtils::FormatInteger (Value, mRunLoops);
	}
	else if (Name == PropertyName::RunPaused          )
	{
		return UIUtils::FormatBoolean (Value, mRunPaused);
	}
	else if (Name == PropertyName::RunReversing       )
	{
		return UIUtils::FormatBoolean (Value, mRunReversing);
	}
	else if (Name == PropertyName::RunSimultaneous    )
	{
		return UIUtils::FormatInteger (Value, mRunSimultaneous);
	}
	else if (Name == PropertyName::RunSpacing)
	{
		return UIUtils::FormatInteger (Value, mRunSpacing);
	}
	else if (Name == PropertyName::RunWrapAround      )
	{
		return UIUtils::FormatBoolean (Value, mRunWrapAround);
	}
	else if (Name == PropertyName::RunOffDuringRestart)
	{
		return UIUtils::FormatBoolean (Value, mRunOffDuringRestart);
	}
	else if (Name == PropertyName::RunEffectorsOn)
	{
		//-- fall through
	}
	else if (Name == PropertyName::RunEffectorsOff)
	{
		//-- fall through
	}
	else if (Name == PropertyName::RunRandomizedPosition)
	{
		return UIUtils::FormatBoolean(Value, mRunRandomizedPostion);
	}


	return UIPage::GetProperty (Name, Value);
}

//----------------------------------------------------------------------

void UIRunner::Notify (UINotificationServer *, UIBaseObject *, UINotification::Code )
{
	{
		const UIWidget * wid = this;

		for (; wid; )
		{
			const UIBaseObject * const p = wid->GetParent ();
			if (p)
			{
				if (!wid->WillDraw ())
					return;

				if (p->IsA (TUIWidget))
					wid = static_cast<const UIWidget *>(p);
				else
					return;
			}
			else
				break;
		}
	}

	const unsigned int curTime = UIClock::gUIClock ().GetTime ();

	UIPage::UIWidgetVector & wv = GetWidgetVector ();
	const int numChildren       = static_cast<int>(wv.size ());

	bool spacingNeedsUpdate = true;

	{
		const int numStates         = static_cast<int>(mStates->size ());

		if (numChildren != numStates)
		{
			mRunTimeNextRestart = 0;
			mRunTimeNextAdvance = 0;
			mStates->resize (numChildren, 0);
			updateSimultaneousAndSpacing ();
			spacingNeedsUpdate = false;
		}
	}

	//-- attempt a restart
	if (mLoopFinished)
	{
		// zero run loops remaining means we are done
		// negative means keep going infinitely
		if (mRunLoopsRemaining == 0)
		{
			SetPaused (true);
			ToggleAll (false);
			return;
		}

		else if (curTime < mRunTimeNextRestart)
		{
			if (mRunTimeNextAdvance && curTime > mRunTimeNextAdvance)
			{
				if (mRunOffDuringRestart)
				{
					for (int i = 0; i < numChildren; ++i)
					{
						char & cur  = (*mStates) [i];

						if (cur)
						{
							setChildState (*wv [i], false);
							cur = 0;
						}
					}
				}
				mRunTimeNextAdvance = 0;
			}
			return;
		}

		// negative means keep going infinitely
		if (mRunLoopsRemaining > 0)
			--mRunLoopsRemaining;

		if (mRunReversing)
			mProgressForward = !mProgressForward;
		else
			mProgressForward = true;

		if (spacingNeedsUpdate)
		{
			updateSimultaneousAndSpacing ();
			spacingNeedsUpdate = false;
		}

		if (mProgressForward)
			mRunProgress     = 0;
		else
			mRunProgress     = mCurrentNumValidProgressSteps - 1;

		mLoopFinished = false;
	}

	//--
	if (curTime > mRunTimeNextAdvance)
	{
		if (spacingNeedsUpdate)
			updateSimultaneousAndSpacing ();

		static StateVector statesCopy;
		statesCopy = *mStates;

		int index = mRunProgress;

		const bool forward = ((mProgressForward && mRunAdvance >= 0) || (!mProgressForward && mRunAdvance <= 0));

		std::fill (mStates->begin (), mStates->end (), 0);

		typedef ui_stdvector<int>::fwd IntVector;
		static IntVector iv;
		iv.clear ();

		{
			for (int i = 0; i < mCurrentNumSimultaneous; ++i)
			{
				if (index < 0)
					index = numChildren - (-index % numChildren);
				else if (index >= numChildren)
				{
					if (mRunReversing)
						break;

					if (!mRunWrapAround)
						break;

					index %= numChildren;
				}

				(*mStates) [index] = 1;

				index += 1 + (*mSpacings) [i];
			}
		}

		for (int i = 0; i < numChildren; ++i)
		{
			const char prev = statesCopy [i];
			const char cur  = (*mStates) [i];

			if (!prev && cur)
			{
				setChildState (*wv [i], true);
			}
			else if (prev && !cur)
			{
				setChildState (*wv [i], false);
			}
		}

		if (forward)
		{
			if (mRunAdvance > 0)
				mRunProgress += mRunAdvance;
			else if (mRunAdvance < 0)
				mRunProgress -= mRunAdvance;
			else
				mRunProgress += GetRandom (0, numChildren - 1);
		}
		else
		{
			if (mRunAdvance > 0)
				mRunProgress -= mRunAdvance;
			else if (mRunAdvance < 0)
				mRunProgress += mRunAdvance;
			else
				mRunProgress -= GetRandom (0, numChildren - 1);
		}

		//-- handle update

		//- negative delay advance means random
		if (mRunDelayAdvance >= 0)
			mRunTimeNextAdvance	= curTime + mRunDelayAdvance;
		else
			mRunTimeNextAdvance	= curTime + GetRandom (1, -mRunDelayAdvance);

		if ((forward && mRunProgress >= mCurrentNumValidProgressSteps) || (!forward && mRunProgress < 0))
		{
			mLoopFinished = true;

			int restartDelay = 0;

			if (mRunDelayRestart >= 0)
				restartDelay = mRunDelayRestart;
			else
				restartDelay = GetRandom (1, -mRunDelayRestart);

			mRunTimeNextRestart = mRunTimeNextAdvance + restartDelay;

			if (mRunReversing)
			{
				if (restartDelay == 0)
				{
					mRunTimeNextAdvance = curTime;
					mRunTimeNextRestart = curTime;
				}
			}
		}
	}
}

//----------------------------------------------------------------------

int UIRunner::GetRandom   (int low, int high)
{
	const int r = rand ();

	const int range = high - low + 1;

	return low + (r * range / RAND_MAX);
}

//----------------------------------------------------------------------

void UIRunner::SetPaused   (bool b)
{
	if (b != mRunPaused)
	{
		mRunPaused = b;

		if (mRunPaused)
			UIClock::gUIClock ().StopListening  (this);
		else
		{
			UIClock::gUIClock ().ListenPerFrame (this);

			//-- zero is infinite
			if (mRunLoops == 0)
				mRunLoopsRemaining = -1;
			else if (mRunLoops > 0)
				mRunLoopsRemaining = mRunLoops;
			else
				mRunLoopsRemaining = GetRandom (1, -mRunLoops);

			mRunProgress        = 0;
			mLoopFinished       = true;
			mRunTimeNextRestart = 0;
			mRunTimeNextAdvance = 0;
		}
	}
}

//----------------------------------------------------------------------

void UIRunner::setChildState (UIWidget & wid, bool active)
{
	if (active)
	{
		SetEffectors (wid, *mEffectorsOff, false);
		SetEffectors (wid, *mEffectorsOn,  true);

		if (mRunRandomizedPostion)
		{
			PickRandomWidgetPosition(wid);
		}
	}
	else
	{
		SetEffectors (wid, *mEffectorsOn, false);
		SetEffectors (wid, *mEffectorsOff, true);
	}
}

//----------------------------------------------------------------------

void UIRunner::updateSimultaneousAndSpacing ()
{
	//-- compute the number of blips that will be simultaneously active this step
	if (mRunSimultaneous >= 0)
		mCurrentNumSimultaneous = mRunSimultaneous;
	else
		mCurrentNumSimultaneous = GetRandom (1, -mRunSimultaneous);

	mSpacings->clear ();

	int totalSpacing = 0;

	{
		for (int i = 0; i < mCurrentNumSimultaneous; ++i)
		{
			int spacing = 0;

			if (mRunSpacing >= 0)
				spacing = mRunSpacing;
			else
				spacing = GetRandom (0, -mRunSpacing);

			mSpacings->push_back (spacing);

			if (i < (mCurrentNumSimultaneous - 1))
				totalSpacing += spacing;
		}
	}

	//-- if we are reversing, the number of valid progress states

	UIPage::UIWidgetVector & wv = GetWidgetVector ();
	const int numChildren       = static_cast<int>(wv.size ());

	mCurrentNumValidProgressSteps = numChildren;
	if (mRunReversing && mCurrentNumSimultaneous >= 0 && mRunSpacing >= 0)
		mCurrentNumValidProgressSteps -= mCurrentNumSimultaneous + totalSpacing - 1;
}

//----------------------------------------------------------------------

bool UIRunner::PopulateEffectors (EffectorVector & ev, const Unicode::String & Value)
{
	bool ok = true;
	SetEffectors (ev, false);

	std::for_each (ev.begin (),  ev.end (), DetachFunctor (0));
	ev.clear ();

	size_t endpos = 0;

	static Unicode::String token;
	while (Unicode::getFirstToken (Value, endpos, endpos, token))
	{
		UIEffector * const effector = static_cast<UIEffector *>(GetObjectFromPath (token, TUIEffector));

		if (effector)
		{
			effector->Attach (0);
			ev.push_back (effector);
		}
		else
			ok = false;
	}

	return ok;
}

//----------------------------------------------------------------------

void UIRunner::SetEffectors (const EffectorVector & ev, bool on)
{
	const UIPage::UIWidgetVector & wv = GetWidgetVector ();

	for (UIPage::UIWidgetVector::const_iterator it = wv.begin (); it != wv.end (); ++it)
	{
		UIWidget * const wid = *it;

		SetEffectors (*wid, ev, on);
	}
}

//----------------------------------------------------------------------

void UIRunner::SetEffectors (UIWidget & wid, const EffectorVector & ev, bool on)
{
	if (UIManager::isUIReady()) 
	{
		UIManager & uiManager = UIManager::gUIManager();
		for (EffectorVector::const_iterator eit = ev.begin (); eit != ev.end (); ++eit)
		{
			UIEffector * const effector = *eit;
			
			if (on)
				uiManager.ExecuteEffector(effector, &wid, false);
			else
				wid.CancelEffector (*effector);
		}
	}
}

//----------------------------------------------------------------------

void UIRunner::SetRunDelayAdvance (int milliseconds)
{
	mRunDelayAdvance = milliseconds * 60 / 1000;
}

//----------------------------------------------------------------------

void UIRunner::ToggleAll          (bool b)
{
	if (b)
	{
		SetEffectors (*mEffectorsOff, false);
		SetEffectors (*mEffectorsOn,  true);
	}
	else
	{
		SetEffectors (*mEffectorsOn,  false);
		SetEffectors (*mEffectorsOff, true);
	}
}

//----------------------------------------------------------------------

void UIRunner::SetProgress        (int progress)
{
	mRunProgress = progress;
	mProgressForward = true;
}

//----------------------------------------------------------------------

void UIRunner::PickRandomWidgetPosition(UIWidget & widget)
{
	// Get the current location.
	UIPoint WidgetLocation = widget.GetLocation();
	bool IsChanged = false;

	//-- Try the width first.
	UIScalar WidgetWidth = widget.GetWidth();
	UIScalar PageWidth = GetWidth();

	if(WidgetWidth < PageWidth)
	{
		UIScalar const EndOffset = PageWidth - WidgetWidth;
		WidgetLocation.x = rand() % EndOffset;
		IsChanged = true;
	}


	//-- Try the height.
	UIScalar WidgetHeight = widget.GetHeight();
	UIScalar PageHeight = GetHeight();
	
	if(WidgetHeight < PageHeight)
	{
		UIScalar const EndOffset = PageHeight - WidgetHeight;
		WidgetLocation.y = rand() % EndOffset;
		IsChanged = true;
	}

	if (IsChanged)
	{
		widget.SetLocation(WidgetLocation);
	}

}

//======================================================================
