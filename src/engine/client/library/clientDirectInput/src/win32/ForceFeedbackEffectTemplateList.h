// ======================================================================
//
// ForceFeedbackEffectTemplateList.h
// Copyright Sony Online Entertainment
//
// ======================================================================

#ifndef ForceFeedbackEffectTemplateLIST_H
#define ForceFeedbackEffectTemplateLIST_H

// ======================================================================

class ForceFeedbackEffectTemplate;

// ======================================================================

// Maintains a list of ForceFeedbackEffectTemplates.

class ForceFeedbackEffectTemplateList
{
friend class ForceFeedbackEffectTemplate;

public:
	static void install();
	static void remove();

	static ForceFeedbackEffectTemplate const * fetch(std::string const & fileName, bool fireAndForget = true);
	static ForceFeedbackEffectTemplate const * fetch(ForceFeedbackEffectTemplate const * ForceFeedbackEffectTemplate);
	static void release(ForceFeedbackEffectTemplate const * ForceFeedbackEffectTemplate);

private:
	static void addNamedForceFeedbackEffectTemplate(ForceFeedbackEffectTemplate * ForceFeedbackEffectTemplate);
	static bool removeForceFeedbackEffectTemplate(ForceFeedbackEffectTemplate * ForceFeedbackEffectTemplate);

private:
	// disabled
	ForceFeedbackEffectTemplateList();
	ForceFeedbackEffectTemplateList(const ForceFeedbackEffectTemplateList &);
	ForceFeedbackEffectTemplateList &operator =(const ForceFeedbackEffectTemplateList &);
};


// ======================================================================

#endif // ForceFeedbackEffectTemplateLIST_H
