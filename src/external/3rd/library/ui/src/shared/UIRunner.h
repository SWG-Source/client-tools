//======================================================================
//
// UIRunner.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_UIRunner_H
#define INCLUDED_UIRunner_H

#include "UIPage.h"
#include "UINotification.h"
class UIEffector;

//======================================================================

class UIRunner :
public UIPage,
public UINotification
{
public:
	static const char * const TypeName;

	class PropertyName
	{
	public:
		static const UILowerString RunAdvance;
		static const UILowerString RunDelayAdvance;
		static const UILowerString RunDelayRestart;
		static const UILowerString RunLooping;
		static const UILowerString RunLoops;
		static const UILowerString RunPaused;
		static const UILowerString RunReversing;
		static const UILowerString RunSimultaneous;
		static const UILowerString RunSpacing;
		static const UILowerString RunWrapAround;
		static const UILowerString RunOffDuringRestart;
		static const UILowerString RunEffectorsOn;
		static const UILowerString RunEffectorsOff;
		static const UILowerString RunRandomizedPosition;
	};

	class MethodName
	{
	public:
		static const UILowerString RunRestart;
	};

	                        UIRunner();
	virtual                ~UIRunner();

	virtual bool            IsA                (const UITypeID) const;
	virtual const char     *GetTypeName        () const;
	virtual UIBaseObject   *Clone              () const;

	virtual void            GetPropertyGroups(UIPropertyGroupVector &, UIPropertyCategories::Category = UIPropertyCategories::C_ALL_CATEGORIES) const;
	virtual void            GetPropertyNames   (UIPropertyNameVector &, bool forCopy ) const;
	virtual void            GetLinkPropertyNames (UIPropertyNameVector &) const;

	virtual bool            SetProperty        (const UILowerString & Name, const UIString &Value);
	virtual bool            GetProperty        (const UILowerString & Name, UIString &Value) const;

	virtual void            Notify             (UINotificationServer *notifyingObject, UIBaseObject *contextObject, UINotification::Code notificationCode);

	void                    SetPaused          (bool b);

	static int              GetRandom          (int low, int high);

	void                    SetRunDelayAdvance (int milliseconds);

	void                    ToggleAll          (bool b);
	void                    SetProgress        (int progress);

	void PickRandomWidgetPosition(UIWidget & widget);

private:

	typedef ui_stdvector<char>::fwd StateVector;
	typedef ui_stdvector<UIEffector *>::fwd EffectorVector;

	                        UIRunner      (const UIRunner & rhs);
	UIRunner &              operator=     (const UIRunner & rhs);
	void                    setChildState (UIWidget & wid, bool active);
	void                    updateSimultaneousAndSpacing ();

	bool                    PopulateEffectors (EffectorVector & ev, const Unicode::String & Value);
	void                    SetEffectors      (const EffectorVector & ev, bool on);
	void                    SetEffectors      (UIWidget & wid, const EffectorVector & ev, bool on);


	int                    mRunSimultaneous;         //positive is normal, negative is random, zero is invalid
	int                    mRunSpacing;              //zero is no spacing, positive is positive spacing, negative is random spacing
	int                    mRunAdvance;              //zero is random
	int                    mRunLoops;                //zero is infinite, negative is random from 1 to abs (value);
	bool                   mRunReversing;
	bool                   mRunPaused;
	bool                   mRunWrapAround;
	bool                   mRunOffDuringRestart;
	int                    mRunDelayRestart;         //negative is random
	int                    mRunDelayAdvance;         //negative is random
	unsigned int           mRunTimeNextRestart;
	unsigned int           mRunTimeNextAdvance;
	int                    mRunLoopsRemaining;       //negative is infinite
	int                    mRunProgress;
	bool mRunRandomizedPostion;

	StateVector *          mStates;
	unsigned int           mLastUpdate;
	bool                   mLoopFinished;
	bool                   mProgressForward;

	int                    mCurrentNumValidProgressSteps;
	int                    mCurrentNumSimultaneous;

	typedef ui_stdvector<int>::fwd IntVector;
	IntVector *            mSpacings;

	EffectorVector *       mEffectorsOn;
	EffectorVector *       mEffectorsOff;

};

//----------------------------------------------------------------------

inline const char  * UIRunner::GetTypeName () const
{
	return TypeName;
}

//======================================================================

#endif
