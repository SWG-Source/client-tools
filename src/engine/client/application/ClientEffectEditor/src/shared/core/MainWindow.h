// ======================================================================
//
// MainWindow.h
// Copyright 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_MainWindow_H
#define INCLUDED_MainWindow_H

// ======================================================================

#include "BaseMainWindow.h"

#include "clientGame/ClientEffectTemplateRW.h"

// ======================================================================

class ClientEffectEditorListViewItem;
class QListViewItem;
class QPopupMenu;

// ======================================================================

class MainWindow: public BaseMainWindow
{
friend ClientEffectEditorListViewItem;

public:
	MainWindow(QWidget* newParent = 0, const char* newName = 0, WFlags fl = WType_TopLevel);
	~MainWindow();

//inherited from BaseMainWindow
public slots:
	virtual void newClientEffect();
	virtual void loadClientEffect();
	virtual bool saveClientEffect();
	virtual bool closeClientEffect();
	virtual void playClientEffect();
	virtual void listViewContextMenuRequested( QListViewItem *, const QPoint &, int );

protected:
	virtual bool close(bool alsoDelete);

private:
	void resetClientEffect();
	bool loadPathedClientEffect(const std::string& relativeFileName);
	bool savePathedClientEffect(const std::string& fullyPathedFileName);
	void copyIffToGUI();
	void copyGUIToIff();
	QListViewItem * getParticleAppearancesBaseItem() const;
	QListViewItem * getSoundsBaseItem() const;
	QListViewItem * getLightsBaseItem() const;
	QListViewItem * getCameraShakesBaseItem() const;
	QListViewItem * getForceFeedbackEffectsBaseItem() const;
	void setupListView() const;
	void updateCaption();

	void deleteEntryNode(ClientEffectEditorListViewItem const * item);

	void addParticleAppearanceTreeNode(ClientEffectTemplate::CreateAppearanceFunc const &);
	void addSoundTreeNode(ClientEffectTemplate::PlaySoundFunc const &);
	void addLightTreeNode(ClientEffectTemplate::CreateLightFunc const &) const;
	void addCameraShakeTreeNode(ClientEffectTemplate::CameraShakeFunc const &) const;
	void addForceFeedbackEffectTreeNode(ClientEffectTemplate::ForceFeedbackFunc const &) const;

	void addNewParticleEffect();
	void addNewSound();
	void addNewCameraShake() const;
	void addNewLight() const;
	void addNewForceFeedbackEffect();

private:
	// disabled
	MainWindow(const MainWindow&);
	MainWindow &operator =(const MainWindow&);

private:
	ClientEffectTemplateRW m_effectTemplate;
	bool                   m_effectTemplateModified;
	Object *               m_avatar;
};

// ======================================================================

#endif
