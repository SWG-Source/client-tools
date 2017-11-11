// ============================================================================
//
// NpcCuiViewer.h
// Copyright(c) 2004, Sony Online Entertainment
//
// ============================================================================

#ifndef NpcCuiViewer_H
#define NpcCuiViewer_H

#include "UIEventCallback.h"
#include "clientUserInterface/CuiMediator.h"

#include <vector>

class Object;
class CreatureObject;
class CuiMessageBox;
class CuiWidget3dObjectListViewer;
class GameCamera;
class UIButton;
class UICheckbox;
class UIList;
class UIPage;
class UIText;
class UIVolumePage;

class SkeletalAppearanceTemplate;
class TangibleObject;
class CustomizationData;

// ============================================================================

class NpcCuiViewer : public CuiMediator, public UIEventCallback
{
public:
	explicit NpcCuiViewer(UIPage & page);

	virtual void OnButtonPressed( UIWidget *context );
	virtual void OnVolumePageSelectionChanged( UIWidget *context );
	virtual bool OnMessage( UIWidget *context, const UIMessage & msg );
	virtual void update(float);

	void loadModel(const std::string & templateName);
	void loadSAT(const std::string & satFilename);
	
	CreatureObject * getCreature();
	Object * getSelectedWearable();
	void setSelectedWearable(Object * o);
	void getSelectedWearableName(std::string & wearableName) const;
	void getSelectedWearableNum(int & wearableNum) const;

	void setupWearableControl ();
	void wear(const char * mgName);
	void unwear();
	void unwear(const char * wearableName);
	void addWearable (const char * wearableName);
	bool isWearing(const char * wearableName);
	void clearWearables();
	void doneAddingWearables();
	void updateCurrentWearables();
	void clearCurrentWearables();

	void createCustomizationLink(Object & wearable, Object & original, bool privateParts) const;

	void zoomBone(const char * bone);
	void setFitDistanceFactor(float val);
	float getFitDistanceFactor() const;

	bool isDirty() const;
	void setDirty(bool b);

	void setEnvironmentTexture(char const *baseFilename);

protected:
	virtual void performActivate();
	virtual void performDeactivate();

private:
	void updateAvatarSelection();
	CreatureObject * createPlayerModel(const std::string & templateName) const;

	virtual ~NpcCuiViewer();
	NpcCuiViewer();
	NpcCuiViewer(const NpcCuiViewer &);
	NpcCuiViewer & operator=(const NpcCuiViewer &);

	Object * m_avatar;
	Object * m_selectedWearable;

	CuiWidget3dObjectListViewer * m_objectViewer;

	SkeletalAppearanceTemplate * m_skeletalAppearanceTemplate;
	std::string m_skeletalAppearanceTemplateFilename;

	UIPage * m_pageWearables;

	UIVolumePage * m_currentVolumePage;
	UIVolumePage * m_wearableVolumePage;

	std::vector<Object *> m_wearables;
	std::vector<Object *> m_currentWearables;
	UIWidget * m_sampleWearableBox;

	bool m_dirty;
};

// ----------------------------------------------------------------------------

inline Object * NpcCuiViewer::getSelectedWearable()
{
	return m_selectedWearable;
}

// ----------------------------------------------------------------------------

inline void NpcCuiViewer::setSelectedWearable(Object * o)
{
	if(m_selectedWearable != o)
	{
		m_selectedWearable = o;
		setDirty(true);
	}
}

// ----------------------------------------------------------------------------

inline bool NpcCuiViewer::isDirty() const
{
	return m_dirty;
}

// ----------------------------------------------------------------------------

inline void NpcCuiViewer::setDirty(bool b)
{
	m_dirty = b;
}

// ============================================================================

#endif
