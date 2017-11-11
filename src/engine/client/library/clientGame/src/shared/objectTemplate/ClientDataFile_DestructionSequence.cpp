//======================================================================
//
// ClientDataFile_DestructionSequence.c
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientDataFile_DestructionSequence.h"

#include "clientAudio/SoundTemplate.h"
#include "clientAudio/SoundTemplateList.h"
#include "clientGame/ClientDataFile_Breakpoint.h"
#include "clientGame/ClientEffectTemplate.h"
#include "clientGame/ClientEffectTemplateList.h"
#include "clientGraphics/RenderWorld.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedMath/Transform.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/Object.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedObject/ObjectTemplateList.h"
#include <vector>

//----------------------------------------------------------------------

namespace DestructionSequenceNamespace
{
	Tag const TAG_SPTL = TAG(S,P,T,L);
	Tag const TAG_SXPL = TAG(S,X,P,L);
	Tag const TAG_SEDF = TAG(S,E,D,F);
	Tag const TAG_ASNL = TAG(A,S,N,L);
	Tag const TAG_SDAS = TAG(S,D,A,S);
	Tag const TAG_SDOL = TAG(S,D,O,L);
	Tag const TAG_SDOE = TAG(S,D,O,E);

	Tag const TAG_BRKP = TAG(B,R,K,P);
}

using namespace DestructionSequenceNamespace;

//----------------------------------------------------------------------
//----------------------------------------------------------------------
//----------------------------------------------------------------------

DestructionSequence::DestructionChildObject::DestructionChildObject() :
m_objectTemplateName(),
m_appearanceTemplateName(),
m_hardpointName(),
m_objectTemplate(NULL),
m_appearanceTemplate(NULL)
{
}

//----------------------------------------------------------------------

void DestructionSequence::DestructionChildObject::load(Iff & iff)
{
	m_objectTemplateName = iff.read_stdstring();
	m_appearanceTemplateName = iff.read_stdstring();
	m_hardpointName = iff.read_stdstring();
}

//----------------------------------------------------------------------

void DestructionSequence::DestructionChildObject::preload()
{
	if (!m_objectTemplateName.empty())
	{
		m_objectTemplate = ObjectTemplateList::fetch(m_objectTemplateName.c_str());
		if (NULL == m_objectTemplate)
		{
			WARNING(true, ("DestructionSequence::DestructionChildObject invalid child object [%s]\n", m_objectTemplateName.c_str()));
		}
	}

	else if (!m_appearanceTemplateName.empty())
	{
		m_appearanceTemplate = AppearanceTemplateList::fetch(m_appearanceTemplateName.c_str());
		if (NULL == m_appearanceTemplate)
		{
			WARNING(true, ("DestructionSequence::DestructionChildObject invalid child appearance [%s]\n", m_appearanceTemplateName.c_str()));
		}
	}
}

//----------------------------------------------------------------------

void DestructionSequence::DestructionChildObject::unload()
{
	if (NULL != m_objectTemplate)
	{
		m_objectTemplate->releaseReference();
		m_objectTemplate = NULL;
	}

	if (NULL != m_appearanceTemplate)
	{
		AppearanceTemplateList::release(m_appearanceTemplate);
		m_appearanceTemplate = NULL;
	}
}

//----------------------------------------------------------------------

void DestructionSequence::DestructionChildObject::apply(Object & parent) const
{
	Object * child = NULL;

	if (NULL != m_objectTemplate)
		child = m_objectTemplate->createObject();
	else if (NULL != m_appearanceTemplate)
	{
		child = new Object;
		child->setAppearance(m_appearanceTemplate->createAppearance());
	}

	if (NULL == child)
		return;

	RenderWorld::addObjectNotifications (*child);
	parent.addChildObject_o(child);

	if (!m_hardpointName.empty())
	{
		Appearance const * const parentAppearance = parent.getAppearance();
		if (NULL != parentAppearance)
		{
			Transform t;
			if (parentAppearance->findHardpoint(ConstCharCrcString(m_hardpointName.c_str()), t))
			{
				child->setTransform_o2p(t);
			}
			else
				WARNING(true, ("ClientDataFile::DestructionSequence invalid hardpoint [%s] for parent object [%s]", m_hardpointName.c_str(), parent.getDebugInformation()));
		}
	}
}

//----------------------------------------------------------------------
//----------------------------------------------------------------------
//----------------------------------------------------------------------

void DestructionSequence::ClientEffectData::load(Iff & iff)
{
	iff.read_string(m_clientEffectName);
	m_nominalSize = iff.read_float();
}

//----------------------------------------------------------------------

void DestructionSequence::ClientEffectData::preload()
{
	m_clientEffectTemplate = ClientEffectTemplateList::fetch(ConstCharCrcLowerString(m_clientEffectName.c_str()));
}

//----------------------------------------------------------------------

void DestructionSequence::ClientEffectData::unload()
{
	if (NULL != m_clientEffectTemplate)
	{
		m_clientEffectTemplate->release();
		m_clientEffectTemplate = NULL;
	}
}

//----------------------------------------------------------------------
//----------------------------------------------------------------------
//----------------------------------------------------------------------
//----------------------------------------------------------------------

void DestructionSequence::AmbientSoundData::load(Iff & iff)
{
	iff.read_string(m_soundTemplateName);
	m_distanceFactor = iff.read_float();
}

//----------------------------------------------------------------------

void DestructionSequence::AmbientSoundData::preload()
{
	m_soundTemplate = SoundTemplateList::fetch(m_soundTemplateName.c_str());
}

//----------------------------------------------------------------------

void DestructionSequence::AmbientSoundData::unload()
{
	if (NULL != m_soundTemplate)
	{
		SoundTemplateList::release(m_soundTemplate);
		m_soundTemplate = NULL;
	}
}
//----------------------------------------------------------------------
//----------------------------------------------------------------------
//----------------------------------------------------------------------

DestructionSequence::DestructionSequence() :
m_sequenceTimeLength(0.0f),
m_clientEffectSplit(),
m_clientEffectFinale(),
m_breakpoints(new BreakpointVector),
m_sequenceExplosions(new ClientEffectDataVector),
m_ambientSounds(new AmbientSoundDataVector),
m_destructionChildObjects(new DestructionChildObjectVector)
{
}

//----------------------------------------------------------------------

DestructionSequence::~DestructionSequence()
{
	unload();
	delete m_breakpoints;
	delete m_sequenceExplosions;
	delete m_ambientSounds;
	delete m_destructionChildObjects;
}

//----------------------------------------------------------------------

void DestructionSequence::preload()
{
	m_clientEffectSplit.preload();
	m_clientEffectFinale.preload();

	{
		for (ClientEffectDataVector::iterator it = m_sequenceExplosions->begin(); it != m_sequenceExplosions->end(); ++it)
			(*it).preload();
	}

	{
		for (AmbientSoundDataVector::iterator it = m_ambientSounds->begin(); it != m_ambientSounds->end(); ++it)
			(*it).preload();
	}

	{
		for (DestructionChildObjectVector::iterator it = m_destructionChildObjects->begin(); it != m_destructionChildObjects->end(); ++it)
			(*it).preload();
	}
}

//----------------------------------------------------------------------

void DestructionSequence::unload()
{
	m_clientEffectSplit.unload();
	m_clientEffectFinale.unload();

	{
		for (ClientEffectDataVector::iterator it = m_sequenceExplosions->begin(); it != m_sequenceExplosions->end(); ++it)
			(*it).unload();
	}

	{
		for (AmbientSoundDataVector::iterator it = m_ambientSounds->begin(); it != m_ambientSounds->end(); ++it)
			(*it).unload();
	}

	{
		for (DestructionChildObjectVector::iterator it = m_destructionChildObjects->begin(); it != m_destructionChildObjects->end(); ++it)
			(*it).unload();
	}
}

//----------------------------------------------------------------------

void DestructionSequence::load(Iff & iff)
{
	iff.enterChunk (TAG_INFO);
	{
		m_sequenceTimeLength = iff.read_float();

		m_clientEffectSplit.load(iff);
		m_clientEffectFinale.load(iff);
	}
	iff.exitChunk(TAG_INFO);

	//----------------------------------------------------------------------

	iff.enterForm(TAG_SPTL);
	{
		while (iff.enterForm(TAG_BRKP, true))
		{
			Breakpoint bp;
			bp.load(iff);
			m_breakpoints->push_back(bp);
			iff.exitForm(TAG_BRKP);
		}
	}
	iff.exitForm(TAG_SPTL);

	//----------------------------------------------------------------------

	iff.enterForm(TAG_SXPL);
	{
		while (iff.enterChunk(TAG_SEDF, true))
		{
			ClientEffectData ced;
			ced.load(iff);
			m_sequenceExplosions->push_back(ced);
			iff.exitChunk(TAG_SEDF);
		}
	}
	iff.exitForm(TAG_SXPL);

	//----------------------------------------------------------------------

	iff.enterForm(TAG_ASNL);
	{
		while (iff.enterChunk(TAG_SDAS, true))
		{
			AmbientSoundData asd;
			asd.load(iff);
			m_ambientSounds->push_back(asd);
			iff.exitChunk(TAG_SDAS);
		}
	}
	iff.exitForm(TAG_ASNL);

	//----------------------------------------------------------------------

	if (iff.enterForm(TAG_SDOL, true))
	{
		while (iff.enterChunk(TAG_SDOE, true))
		{
			DestructionChildObject dco;
			dco.load(iff);
			m_destructionChildObjects->push_back(dco);
			iff.exitChunk(TAG_SDOE);
		}
		iff.exitForm(TAG_SDOL);
	}

}

//----------------------------------------------------------------------

DestructionSequence::BreakpointVector const & DestructionSequence::getBreakpoints() const
{
	return *NON_NULL(m_breakpoints);
}

//----------------------------------------------------------------------

DestructionSequence::ClientEffectDataVector const & DestructionSequence::getSequenceExplosions() const
{
	return *NON_NULL(m_sequenceExplosions);
}

//----------------------------------------------------------------------

DestructionSequence::AmbientSoundDataVector const & DestructionSequence::getAmbientSounds() const
{
	return *NON_NULL(m_ambientSounds);
}

//----------------------------------------------------------------------

float DestructionSequence::getSequenceTimeLength() const
{
	return m_sequenceTimeLength;
}

//----------------------------------------------------------------------

DestructionSequence::ClientEffectData const & DestructionSequence::getClientEffectDataSplit() const
{
	return m_clientEffectSplit;
}

//----------------------------------------------------------------------

DestructionSequence::ClientEffectData const & DestructionSequence::getClientEffectDataFinale() const
{
	return m_clientEffectFinale;
}

//----------------------------------------------------------------------

void DestructionSequence::applyChildObjects(Object & parent) const
{
	{
		for (DestructionChildObjectVector::iterator it = m_destructionChildObjects->begin(); it != m_destructionChildObjects->end(); ++it)
			(*it).apply(parent);
	}
}

//----------------------------------------------------------------------

