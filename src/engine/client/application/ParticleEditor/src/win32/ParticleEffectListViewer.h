// ============================================================================
//
// ParticleEffectListViewer.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_ParticleEffectListViewer_H
#define INCLUDED_ParticleEffectListViewer_H

#include "clientParticle/Particle.h"
#include "clientParticle/ParticleAttachmentDescription.h"
#include "clientParticle/ParticleEffectAppearance.h"
#include "clientParticle/ParticleEmitter.h"
#include "clientParticle/ParticleEmitterDescription.h"
#include "clientParticle/ParticleEmitterGroup.h"

class Iff;
class ParticleDescription;
class QListViewItem;
class QPopupMenu;
class QTimer;

//-----------------------------------------------------------------------------
class ParticleEditorListViewItem : public QListViewItem
{
public:

	enum Tag
	{
		T_particleAttachment,
		T_particle,
		T_particleEmitter,
		T_particleEmitterGroup,
		T_particleEffect,
		T_particleEffectGroup
	};

	ParticleEditorListViewItem(QListViewItem *parent, char const *name);
	ParticleEditorListViewItem(QListView *parent, char const *name);

	virtual void write(Iff &iff) = 0;
	virtual Tag  getTag() const = 0;

protected:

	virtual void paintCell(QPainter *painter, QColorGroup const &colorGroup, int column, int width, int align);
	virtual void paintFocus(QPainter *painter, QColorGroup const &colorGroup, QRect const &rect);
};

//-----------------------------------------------------------------------------
class ParticleAttachmentListViewItem : public ParticleEditorListViewItem
{
public:

	ParticleAttachmentListViewItem(QListViewItem *parent, char const *name);
	virtual ~ParticleAttachmentListViewItem();

	virtual void                         write(Iff &iff);

	void                                 setDescription(ParticleAttachmentDescription const &particleAttachmentDescription);
	void                                 setName(std::string const &name);
	virtual Tag                          getTag() const { return T_particleAttachment; }

	ParticleAttachmentDescription const &getParticleAttachmentDescription() const;

	//void                           loadFromDescription(ParticleDescription const &particleDescription);
	//void                           initializeDefault();

private:

	ParticleAttachmentDescription m_particleAttachmentDescription;
};

//-----------------------------------------------------------------------------
class ParticleListViewItem : public ParticleEditorListViewItem
{
public:

	ParticleListViewItem(QListViewItem *parent, char const *name);
	virtual ~ParticleListViewItem();

	virtual void                    write(Iff &iff);
	void                            loadFromDescription(ParticleDescription const &particleDescription);
	void                            setDescription(ParticleDescription const &particleDescription);
	ParticleDescription const &     getParticleDescription() const;
	void                            setName(std::string const &name);
	void                            initializeDefault();
	ParticleAttachmentListViewItem *addChild(std::string const &name);
	void                            removeChild(ParticleEditorListViewItem *particleEditorListViewItem);
	void                            addToDescription(ParticleDescription **particleDescription);
	virtual Tag                     getTag() const { return T_particle; }

private:

	ParticleDescription *m_particleDescription;

	typedef std::list<ParticleAttachmentListViewItem *> ParticleAttachmentListViewItemList;
	ParticleAttachmentListViewItemList m_particleAttachmentListViewItemList;
};

//-----------------------------------------------------------------------------
class ParticleEmitterListViewItem : public ParticleEditorListViewItem
{
public:

	ParticleEmitterListViewItem(QListViewItem *parent, char const *name);
	virtual ~ParticleEmitterListViewItem();

	virtual void                      write(Iff &iff);
	void                              addToDescription(ParticleEmitterDescription &particleEmitterDescription);
	void                              loadFromDescription(ParticleEmitterDescription const &particleEmitterDescription);
	void                              setDescription(ParticleEmitterDescription const particleEmitterDescription);
	ParticleListViewItem *            addChild(std::string const &name);
	ParticleEmitterDescription const &getParticleEmitterDescription() const;
	void                              setName(std::string const &name);
	void                              updateParticleCount();
	virtual Tag                       getTag() const { return T_particleEmitter; }

private:

	ParticleEmitterDescription m_particleEmitterDescription;

	typedef std::list<ParticleListViewItem *> ParticleListViewItemList;
	ParticleListViewItemList m_particleListViewItemList;

private:

	// Disabled

	ParticleEmitterListViewItem(ParticleEmitterListViewItem const &);
	ParticleEmitterListViewItem &operator =(ParticleEmitterListViewItem const &);
};

//-----------------------------------------------------------------------------
class ParticleEmitterGroupListViewItem : public ParticleEditorListViewItem
{
public:

	ParticleEmitterGroupListViewItem(QListViewItem *parent, char const *name);
	virtual ~ParticleEmitterGroupListViewItem();

	virtual void                           write(Iff &iff);
	void                                   addToDescription(ParticleEmitterGroupDescription &particleEmitterGroupDescription);
	void                                   loadFromDescription(ParticleEmitterGroupDescription const &particleEmitterGroupDescription);
	void                                   setDescription(ParticleEmitterGroupDescription const particleEmitterGroupDescription);
	ParticleEmitterListViewItem *          addChild(std::string const &name);
	void                                   removeChild(ParticleEditorListViewItem *particleEditorListViewItem);
	ParticleEmitterGroupDescription const &getParticleEmitterGroupDescription() const;
	void                                   updateParticleCount();
	void                                   addEmitter(ParticleEmitterDescription const &particleEmitterDescription);
	virtual Tag                            getTag() const { return T_particleEmitterGroup; }

private:

	ParticleEmitterGroupDescription m_particleEmitterGroupDescription;

	typedef std::list<ParticleEmitterListViewItem *> ParticleEmitterListViewItemList;
	ParticleEmitterListViewItemList m_particleEmitterListViewItemList;

private:

	// Disabled

	ParticleEmitterGroupListViewItem(ParticleEmitterGroupListViewItem const &);
	ParticleEmitterGroupListViewItem &operator =(ParticleEmitterGroupListViewItem const &);
};

//-----------------------------------------------------------------------------
class ParticleEffectListViewItem : public ParticleEditorListViewItem
{
public:

	ParticleEffectListViewItem(QListViewItem *parent, char const *name);
	virtual ~ParticleEffectListViewItem();

	virtual void                      write(Iff &iff);
	void                              addToDescription(ParticleEffectDescription &particleEffectDescription);
	void                              loadFromDescription(ParticleEffectDescription const &particleEffectDescription);
	void                              setDescription(ParticleEffectDescription const particleEffectDescription);
	ParticleEmitterGroupListViewItem *addChild(std::string const &name);
	ParticleEffectDescription const & getParticleEffectDescription() const;
	void                              updateParticleCount();
	virtual Tag                       getTag() const { return T_particleEffect; }

private:

	ParticleEffectDescription m_particleEffectDescription;

	typedef std::list<ParticleEmitterGroupListViewItem *> ParticleEmitterGroupListViewItemList;
	ParticleEmitterGroupListViewItemList m_particleEmitterGroupListViewItemList;

private:

	// Disabled

	ParticleEffectListViewItem(ParticleEffectListViewItem const &);
	ParticleEffectListViewItem &operator =(ParticleEffectListViewItem const &);
};

//-----------------------------------------------------------------------------
class ParticleEffectGroupListViewItem : public ParticleEditorListViewItem
{
public:

	ParticleEffectGroupListViewItem(QListView *parent, char const *name);
	virtual ~ParticleEffectGroupListViewItem();

	virtual void                write(Iff &iff);
	virtual void                load(Iff &iff);
	ParticleEffectListViewItem *addChild(std::string const &name);
	void                        updateParticleCount();
	void setParticleEffectFileName(std::string const & fileName, int const particleCount);
	virtual Tag                 getTag() const { return T_particleEffectGroup; }

private:

	ParticleEffectListViewItem * m_particleEffectListViewItem;
};

//-----------------------------------------------------------------------------
class ParticleEffectListViewer : public QListView
{
	Q_OBJECT

public:

	ParticleEffectListViewer(QWidget *parent, char const *name, WFlags const flags = 0);

	// Reset the particle effect to the defaults

	void reset();

	// Create an iff describing the particle effect

	void write(Iff &iff);

	// Load a particle effect from an iff

	void load(Iff &iff);

	// Set the filename of the particle effect

	void setParticleEffectFileName(std::string const &fileName);

	static std::string getParticleEffectName(char const *text);
	static std::string getParticleEmitterGroupName(char const *text);
	static std::string getParticleEmitterName(char const *text);
	static std::string getParticleName(char const *text, ParticleDescription const *particleDescription);
	static std::string getParticleAttachmentName(char const *text);

	void setSelectedListViewItem(QListViewItem *listViewItem);
	void setObject(Object const *object);

signals:

	void showParticleAttachmentAttributes(const ParticleAttachmentDescription &);
	void showParticleAttributes(const ParticleDescription &);
	void showParticleEmitterAttributes(const ParticleEmitterDescription &);
	void showParticleEmitterGroupAttributes(const ParticleEmitterGroupDescription &);
	void showParticleEffectAttributes(const ParticleEffectDescription &);
	void showParticleEffectGroupAttributes();
	void forceRebuild();
	void signalUnDefine();

private slots:

	void onAttributeNameChanged(const std::string &name);
	void onParticleAttachmentDescriptionChanged(const ParticleAttachmentDescription &);
	void onParticleDescriptionChanged(const ParticleDescription &);
	void onParticleEmitterDescriptionChanged(const ParticleEmitterDescription &);
	void onParticleEmitterGroupDescriptionChanged(const ParticleEmitterGroupDescription &);
	void onParticleEffectDescriptionChanged(const ParticleEffectDescription &);
	void onTimerTimeout();

private:

	void showParticleEffectPopUpMenu(QPoint const &point);
	void showParticleEmitterGroupPopUpMenu(QPoint const &point);
	void showParticleEmitterPopUpMenu(QPoint const &point);
	void showParticlePopUpMenu(QPoint const &point);
	void showParticleAttachmentPopUpMenu(QPoint const &point);

	virtual void contentsMouseReleaseEvent(QMouseEvent *mouseEvent);

	enum
	{
		resetItem = 0,
		addItem,
		importItem,
		exportItem,
		deleteItem
	};

	ParticleEditorListViewItem *     m_selectedListViewItem;
	ParticleEffectGroupListViewItem *m_rootListViewItem;
	QTimer *                         m_timer;
	ConstWatcher<Object>             m_object;
};

// ============================================================================

#endif // INCLUDED_ParticleEffectListViewer_H
