// ============================================================================
//
// ParticleEffectListViewer.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstParticleEditor.h"
#include "ParticleEffectListViewer.h"
#include "ParticleEffectListViewer.moc"

#include "clientAudio/Audio.h" // Remove this when FilePath moves.
#include "clientParticle/ParticleAttachmentDescription.h"
#include "clientParticle/ParticleDescription.h"
#include "clientParticle/ParticleDescriptionQuad.h"
#include "clientParticle/ParticleDescriptionMesh.h"
#include "clientParticle/ParticleManager.h"
#include "clientParticle/ParticleEffectAppearance.h"
#include "clientParticle/ParticleEffectAppearanceTemplate.h"
#include "ParticleEditorUtility.h"
#include "sharedFile/FileNameUtils.h"
#include <vector>

// ============================================================================
//
// ParticleEffectListViewer
//
// ============================================================================

//-----------------------------------------------------------------------------
ParticleEditorListViewItem::ParticleEditorListViewItem(QListViewItem *parent, char const *name)
 : QListViewItem(parent, name)
{
}

//-----------------------------------------------------------------------------
ParticleEditorListViewItem::ParticleEditorListViewItem(QListView *parent, char const *name)
 : QListViewItem(parent, name)
{
}

//-----------------------------------------------------------------------------
void ParticleEditorListViewItem::paintCell(QPainter *painter, QColorGroup const &colorGroup, int column, int width, int align)
{
	NOT_NULL(painter);

	if (column == 0)
	{
		// Clear the background

		painter->fillRect(painter->window(), QBrush(QColor("white")));

		// Draw the current text

		QFontMetrics fm(painter->fontMetrics());
		char currentText[256];
		strncpy(currentText, static_cast<char const *>(text(0)), sizeof(currentText));

		int x = 4;
		int y = height() - fm.descent() - 2;

		if (isSelected())
		{
			painter->setPen(QColor("blue"));
			painter->drawRect(0, 0, width, height());
			painter->fillRect(width - 10, 0, width, height(), QColor("blue"));
		}

		ParticleEditorUtility::drawColorText(painter, x, y, currentText, QColor("black"));
	}
	else
	{
		QListViewItem::paintCell(painter, colorGroup, column, width, align);
	}
}

//-----------------------------------------------------------------------------
void ParticleEditorListViewItem::paintFocus(QPainter *painter, QColorGroup const &colorGroup, QRect const &rect)
{
	UNREF(painter);
	UNREF(colorGroup);
	UNREF(rect);
}

// ============================================================================
//
// ParticleAttachmentListViewItem
//
// ============================================================================

//-----------------------------------------------------------------------------
ParticleAttachmentListViewItem::ParticleAttachmentListViewItem(QListViewItem *parent, char const *name)
 : ParticleEditorListViewItem(parent, name)
 , m_particleAttachmentDescription()
{
}

//-----------------------------------------------------------------------------
ParticleAttachmentListViewItem::~ParticleAttachmentListViewItem()
{
}

//-----------------------------------------------------------------------------
void ParticleAttachmentListViewItem::write(Iff &iff)
{
	m_particleAttachmentDescription.write(iff);
}

//-----------------------------------------------------------------------------
ParticleAttachmentDescription const &ParticleAttachmentListViewItem::getParticleAttachmentDescription() const
{
	return m_particleAttachmentDescription;
}

//-----------------------------------------------------------------------------
void ParticleAttachmentListViewItem::setDescription(ParticleAttachmentDescription const &particleAttachmentDescription)
{
	// Copy the particle description

	m_particleAttachmentDescription = particleAttachmentDescription;

	// Rename the list itew item

	setName(FileNameUtils::get(m_particleAttachmentDescription.getAttachmentPath(), FileNameUtils::fileName | FileNameUtils::extension).c_str());

	// Send the event

	(dynamic_cast<ParticleEffectListViewer *>(listView()))->setSelectedListViewItem(this);
}

//-----------------------------------------------------------------------------
void ParticleAttachmentListViewItem::setName(std::string const &name)
{
	setText(0, ParticleEffectListViewer::getParticleAttachmentName(name.c_str()).c_str());
}

// ============================================================================
//
// ParticleListViewItem
//
// ============================================================================

//-----------------------------------------------------------------------------
ParticleListViewItem::ParticleListViewItem(QListViewItem *parent, char const *name)
 : ParticleEditorListViewItem(parent, name)
{
	m_particleDescription = new ParticleDescriptionQuad();
	m_particleDescription->initializeDefault();
}

//-----------------------------------------------------------------------------
ParticleListViewItem::~ParticleListViewItem()
{
	delete m_particleDescription;
}

//-----------------------------------------------------------------------------
void ParticleListViewItem::setName(std::string const &name)
{
	m_particleDescription->setName(name.c_str());
	setText(0, ParticleEffectListViewer::getParticleName(name.c_str(), m_particleDescription).c_str());
}

//-----------------------------------------------------------------------------
void ParticleListViewItem::write(Iff &iff)
{
	m_particleDescription->write(iff);
}

//-----------------------------------------------------------------------------
void ParticleListViewItem::loadFromDescription(ParticleDescription const &particleDescription)
{
	delete m_particleDescription;
	m_particleDescription = particleDescription.clone();

	setName(FileNameUtils::get(particleDescription.getName(), FileNameUtils::fileName | FileNameUtils::extension).c_str());

	// Clear the particle attachment list

	m_particleAttachmentListViewItemList.clear();

	// If there are any attachments, add them

	ParticleDescription::ParticleAttachmentDescriptions::const_iterator begin = m_particleDescription->getParticleAttachmentDescriptions().begin();
	ParticleDescription::ParticleAttachmentDescriptions::const_iterator end = m_particleDescription->getParticleAttachmentDescriptions().end();
	ParticleDescription::ParticleAttachmentDescriptions::const_iterator current = begin;

	for (; current != end; ++current)
	{
		// Add each attachment

		ParticleAttachmentListViewItem *particleAttachmentListViewItem = addChild(current->getAttachmentPath());

		// Set the child description

		particleAttachmentListViewItem->setDescription(*current);
	}
}

//-----------------------------------------------------------------------------
void ParticleListViewItem::setDescription(ParticleDescription const &particleDescription)
{
	// Save the previous particle type

	ParticleDescription::ParticleType particleTypePrevious = ParticleDescription::PT_quad;

	if (m_particleDescription)
	{
		particleTypePrevious = m_particleDescription->getParticleType();
	}

	// Copy the particle description

	delete m_particleDescription;
	m_particleDescription = particleDescription.clone();

	// Rename the list itew item

	setName(FileNameUtils::get(m_particleDescription->getName(), FileNameUtils::fileName | FileNameUtils::extension).c_str());

	// Only send this event if the type of particle changed

	if (m_particleDescription->getParticleType() != particleTypePrevious)
	{
		(dynamic_cast<ParticleEffectListViewer *>(listView()))->setSelectedListViewItem(this);
	}

//#ifdef _DEBUG
//	static int count = 0;
//	DEBUG_REPORT_LOG(1, ("ParticleListViewItem::setDescription(): %d\n", count++));
//#endif // _DEBUG
}

//-----------------------------------------------------------------------------
ParticleDescription const &ParticleListViewItem::getParticleDescription() const
{
	return *m_particleDescription;
}

//-----------------------------------------------------------------------------
void ParticleListViewItem::initializeDefault()
{
	m_particleDescription->initializeDefault();
}

//-----------------------------------------------------------------------------
ParticleAttachmentListViewItem *ParticleListViewItem::addChild(std::string const &name)
{
	ParticleAttachmentListViewItem *particleAttachmentListViewItem = new ParticleAttachmentListViewItem(this, name.c_str());
	insertItem(particleAttachmentListViewItem);
	m_particleAttachmentListViewItemList.push_back(particleAttachmentListViewItem);

	setOpen(true);

	return particleAttachmentListViewItem;
}

//-----------------------------------------------------------------------------
void ParticleListViewItem::removeChild(ParticleEditorListViewItem *particleEditorListViewItem)
{
	ParticleAttachmentListViewItem *particleAttachmentListViewItem = dynamic_cast<ParticleAttachmentListViewItem *>(particleEditorListViewItem);

	bool found = false;

	if (particleAttachmentListViewItem != NULL)
	{
		ParticleAttachmentListViewItemList::iterator iterParticleAttachmentListViewItemList = m_particleAttachmentListViewItemList.begin();

		for (; iterParticleAttachmentListViewItemList != m_particleAttachmentListViewItemList.end(); ++iterParticleAttachmentListViewItemList)
		{
			if ((*iterParticleAttachmentListViewItemList) == particleAttachmentListViewItem)
			{
				m_particleAttachmentListViewItemList.remove(*iterParticleAttachmentListViewItemList);

				found = true;

				break;
			}
		}
	}

	if (!found)
	{
		DEBUG_FATAL(!found, ("Unable to remove child list view item"));
	}
}

//-----------------------------------------------------------------------------
void ParticleListViewItem::addToDescription(ParticleDescription **particleDescription)
{
	delete (*particleDescription);
	(*particleDescription) = m_particleDescription->clone();

#ifdef _DEBUG
	int const size = static_cast<int>((*particleDescription)->getParticleAttachmentDescriptions().size());
	UNREF(size);
#endif // _DEBUG

	// Remove any previous attachments

	(*particleDescription)->clearParticleAttachmentDescriptions();

	// Add the attachments

	ParticleDescription::ParticleAttachmentDescriptions particleAttachmentDesciptions;
	ParticleAttachmentListViewItemList::const_iterator iterParticleAttachmentListViewItemList = m_particleAttachmentListViewItemList.begin();
	
	for (; iterParticleAttachmentListViewItemList != m_particleAttachmentListViewItemList.end(); ++iterParticleAttachmentListViewItemList)
	{
		ParticleAttachmentDescription const &particleAttachmentDescription = (*iterParticleAttachmentListViewItemList)->getParticleAttachmentDescription();

		// Only add attachments if a file is specified

		if (!particleAttachmentDescription.getAttachmentPath().empty())
		{
			particleAttachmentDesciptions.push_back(particleAttachmentDescription);
		}
	}

	(*particleDescription)->setParticleAttachmentDescriptions(particleAttachmentDesciptions);
}

// ============================================================================
//
// ParticleEmitterListViewItem
//
// ============================================================================

//-----------------------------------------------------------------------------
ParticleEmitterListViewItem::ParticleEmitterListViewItem(QListViewItem *parent, char const *name)
 : ParticleEditorListViewItem(parent, name)
{
	m_particleEmitterDescription.initializeDefault();
	//m_particleEmitterDescription.clearDescriptionList();
}

//-----------------------------------------------------------------------------
ParticleEmitterListViewItem::~ParticleEmitterListViewItem()
{
}

//-----------------------------------------------------------------------------
void ParticleEmitterListViewItem::setName(std::string const &name)
{
#ifdef _DEBUG
	m_particleEmitterDescription.m_emitterName = name;
#endif // _DEBUG
	setText(0, ParticleEffectListViewer::getParticleEmitterName(name.c_str()).c_str());
}

//-----------------------------------------------------------------------------
void ParticleEmitterListViewItem::write(Iff &iff)
{
	m_particleEmitterDescription.write(iff);
}

//-----------------------------------------------------------------------------
void ParticleEmitterListViewItem::addToDescription(ParticleEmitterDescription &particleEmitterDescription)
{
	particleEmitterDescription = m_particleEmitterDescription;

	// Add the children

	ParticleListViewItemList::const_iterator current = m_particleListViewItemList.begin();
	
	for (; current != m_particleListViewItemList.end(); ++current)
	{
		(*current)->addToDescription(&particleEmitterDescription.m_particleDescription);
	}
}

//-----------------------------------------------------------------------------
void ParticleEmitterListViewItem::loadFromDescription(ParticleEmitterDescription const &particleEmitterDescription)
{
#ifdef _DEBUG
	setName(particleEmitterDescription.m_emitterName);
#endif // _DEBUG

	m_particleEmitterDescription = particleEmitterDescription;

	// Clear the listViewItem list

	m_particleListViewItemList.clear();

	// Add the new listViewItems to the list

	ParticleListViewItem *particleListViewItem = addChild(ParticleEffectListViewer::getParticleName("Default", m_particleEmitterDescription.m_particleDescription));

	// Set the child description

	particleListViewItem->loadFromDescription(*particleEmitterDescription.m_particleDescription);

	//// Add the new listViewItems to the list
	//
	//ParticleDescription::const_iterator current = particleEmitterDescription.m_particleDescriptionList->begin();
	//
	//for (; current != particleEmitterDescription.m_particleDescriptionList->end(); ++current)
	//{
	//	// Add a new child
	//
	//	ParticleListViewItem *particleListViewItem = addChild(ParticleEffectListViewer::getNewParticleName());
	//
	//	// Set the child description
	//
	//	ParticleDescription *particleDescription = (*current);
	//	particleListViewItem->loadFromDescription(*particleDescription);
	//}

	//m_particleEmitterDescription.clearDescriptionList();
}

//-----------------------------------------------------------------------------
void ParticleEmitterListViewItem::setDescription(ParticleEmitterDescription const particleEmitterDescription)
{
	// Copy the particle emitter description

	m_particleEmitterDescription = particleEmitterDescription;

	// Remove the particle description lists because that data is stored by ParticleListViewItems

	//m_particleEmitterDescription.clearDescriptionList();

//#ifdef _DEBUG
//	static int count = 0;
//	DEBUG_REPORT_LOG(1, ("ParticleEmitterListViewItem::setDescription(): %d\n", count++));
//#endif // _DEBUG
}

//-----------------------------------------------------------------------------
ParticleListViewItem *ParticleEmitterListViewItem::addChild(std::string const &name)
{
	UNREF(name);

	ParticleListViewItem *particleListViewItem = new ParticleListViewItem(this, name.c_str());
	insertItem(particleListViewItem);
	m_particleListViewItemList.push_back(particleListViewItem);

	setOpen(true);

	return particleListViewItem;
}

//-----------------------------------------------------------------------------
ParticleEmitterDescription const &ParticleEmitterListViewItem::getParticleEmitterDescription() const
{
	return m_particleEmitterDescription;
}

//-----------------------------------------------------------------------------
void ParticleEmitterListViewItem::updateParticleCount()
{
	//char text[256];
	//sprintf(text, "%4d", ParticleManager::getParticleCount());
	//setText(1, text);
	//
	//ParticleEffectList::iterator current = m_particleEffectList.begin();
	//
	//for (; current != m_particleEffectList.end(); ++current)
	//{
	//	(*current)->updateParticleCount();
	//}
}

// ============================================================================
//
// ParticleEmitterGroupListViewItem
//
// ============================================================================

//-----------------------------------------------------------------------------
ParticleEmitterGroupListViewItem::ParticleEmitterGroupListViewItem(QListViewItem *parent, char const *name)
 : ParticleEditorListViewItem(parent, name)
{
	m_particleEmitterGroupDescription.initializeDefault();
	m_particleEmitterGroupDescription.clearParticleEmitterDescriptions();
}

//-----------------------------------------------------------------------------
ParticleEmitterGroupListViewItem::~ParticleEmitterGroupListViewItem()
{
}

//-----------------------------------------------------------------------------
void ParticleEmitterGroupListViewItem::write(Iff &iff)
{
	m_particleEmitterGroupDescription.write(iff);
}

//-----------------------------------------------------------------------------
void ParticleEmitterGroupListViewItem::addToDescription(ParticleEmitterGroupDescription &particleEmitterGroupDescription)
{
	particleEmitterGroupDescription = m_particleEmitterGroupDescription;
	DEBUG_FATAL(!particleEmitterGroupDescription.getParticleEmitterDescriptions().empty(), ("There should be no particle emitter descriptions defined at this point."));

	// Add the children

	ParticleEmitterGroupDescription::ParticleEmitterDescriptions particleEmitterDescriptions;
	ParticleEmitterListViewItemList::const_iterator current = m_particleEmitterListViewItemList.begin();

	for (; current != m_particleEmitterListViewItemList.end(); ++current)
	{
		ParticleEmitterDescription particleEmitterDescription;

		(*current)->addToDescription(particleEmitterDescription);

		particleEmitterDescriptions.push_back(particleEmitterDescription);
	}

	particleEmitterGroupDescription.setParticleEmitterDescriptions(particleEmitterDescriptions);
}

//-----------------------------------------------------------------------------
void ParticleEmitterGroupListViewItem::loadFromDescription(ParticleEmitterGroupDescription const &particleEmitterGroupDescription)
{
	m_particleEmitterGroupDescription = particleEmitterGroupDescription;
	m_particleEmitterGroupDescription.clearParticleEmitterDescriptions();

	// Clear the listViewItem list

	m_particleEmitterListViewItemList.clear();

	// Add the new listViewItems to the list

	ParticleEmitterGroupDescription::ParticleEmitterDescriptions::const_iterator current = particleEmitterGroupDescription.getParticleEmitterDescriptions().begin();

#ifdef _DEBUG
	int count = particleEmitterGroupDescription.getParticleEmitterDescriptions().size();
	UNREF(count);
#endif // _DEBUG

	for (; current != particleEmitterGroupDescription.getParticleEmitterDescriptions().end(); ++current)
	{
		addEmitter(*current);
	}
}

//-----------------------------------------------------------------------------
void ParticleEmitterGroupListViewItem::setDescription(ParticleEmitterGroupDescription const particleEmitterGroupDescription)
{
	m_particleEmitterGroupDescription = particleEmitterGroupDescription;
	m_particleEmitterGroupDescription.clearParticleEmitterDescriptions();

//#ifdef _DEBUG
//	static int count = 0;
//	DEBUG_REPORT_LOG(1, ("ParticleEmitterGroupListViewItem::setDescription(): %d\n", count++));
//#endif // _DEBUG
}

//-----------------------------------------------------------------------------
ParticleEmitterListViewItem *ParticleEmitterGroupListViewItem::addChild(std::string const &name)
{
	ParticleEmitterListViewItem *particleEmitterListViewItem = new ParticleEmitterListViewItem(this, name.c_str());
	insertItem(particleEmitterListViewItem);
	m_particleEmitterListViewItemList.push_back(particleEmitterListViewItem);

	setOpen(true);

	return particleEmitterListViewItem;
}

//-----------------------------------------------------------------------------
void ParticleEmitterGroupListViewItem::removeChild(ParticleEditorListViewItem *particleEditorListViewItem)
{
	ParticleEmitterListViewItem *particleEmitterListViewItem = dynamic_cast<ParticleEmitterListViewItem *>(particleEditorListViewItem);

	bool found = false;

	if (particleEmitterListViewItem != NULL)
	{
		ParticleEmitterListViewItemList::iterator current = m_particleEmitterListViewItemList.begin();

		for (; current != m_particleEmitterListViewItemList.end(); ++current)
		{
			if ((*current) == particleEmitterListViewItem)
			{
				m_particleEmitterListViewItemList.remove(*current);

				found = true;

				break;
			}
		}
	}

	if (!found)
	{
		DEBUG_FATAL(!found, ("Unable to remove child list view item"));
	}
}

//-----------------------------------------------------------------------------
ParticleEmitterGroupDescription const & ParticleEmitterGroupListViewItem::getParticleEmitterGroupDescription() const
{
	return m_particleEmitterGroupDescription;
}

//-----------------------------------------------------------------------------
void ParticleEmitterGroupListViewItem::updateParticleCount()
{
	ParticleEmitterGroupDescription m_particleEmitterGroupDescription;

	//char text[256];
	//sprintf(text, "%4d", ParticleManager::getParticleCount());
	//setText(1, text);
	//
	//ParticleEffectList::iterator current = m_particleEffectList.begin();
	//
	//for (; current != m_particleEffectList.end(); ++current)
	//{
	//	(*current)->updateParticleCount();
	//}
}

//-----------------------------------------------------------------------------
void ParticleEmitterGroupListViewItem::addEmitter(ParticleEmitterDescription const &particleEmitterDescription)
{
	// Add a new child

	ParticleEmitterListViewItem *particleEmitterListViewItem = addChild(ParticleEffectListViewer::getParticleEmitterName("Default"));

	// Set the child description

	particleEmitterListViewItem->loadFromDescription(particleEmitterDescription);
}

// ============================================================================
//
// ParticleEffectListViewItem
//
// ============================================================================

//-----------------------------------------------------------------------------
ParticleEffectListViewItem::ParticleEffectListViewItem(QListViewItem *parent, char const *name)
 : ParticleEditorListViewItem(parent, name)
{
	m_particleEffectDescription.initializeDefault();
	m_particleEffectDescription.clearParticleEmitterGroupDescriptions();
}

//-----------------------------------------------------------------------------
ParticleEffectListViewItem::~ParticleEffectListViewItem()
{
}

//-----------------------------------------------------------------------------
void ParticleEffectListViewItem::write(Iff &iff)
{
	m_particleEffectDescription.write(iff);
}

//-----------------------------------------------------------------------------
void ParticleEffectListViewItem::addToDescription(ParticleEffectDescription &particleEffectDescription)
{
	particleEffectDescription = m_particleEffectDescription;
	DEBUG_FATAL(!particleEffectDescription.getParticleEmitterGroupDescriptions().empty(), ("There should not be any particle emitter group descriptions defined at this time."));

	// Add the children

	ParticleEffectDescription::ParticleEmitterGroupDescriptions particleEmitterGroupDescriptions;
	ParticleEmitterGroupListViewItemList::const_iterator current = m_particleEmitterGroupListViewItemList.begin();

	for (; current != m_particleEmitterGroupListViewItemList.end(); ++current)
	{
		ParticleEmitterGroupDescription particleEmitterGroupDescription;

		(*current)->addToDescription(particleEmitterGroupDescription);

		particleEmitterGroupDescriptions.push_back(particleEmitterGroupDescription);
	}

	particleEffectDescription.setParticleEmitterGroupDescriptions(particleEmitterGroupDescriptions);
}

//-----------------------------------------------------------------------------
void ParticleEffectListViewItem::loadFromDescription(ParticleEffectDescription const &particleEffectDescription)
{
	m_particleEffectDescription = particleEffectDescription;

	// Clear the listViewItem list

	m_particleEmitterGroupListViewItemList.clear();

	// Add the new listViewItems to the list

	ParticleEffectDescription::ParticleEmitterGroupDescriptions::const_iterator current = particleEffectDescription.getParticleEmitterGroupDescriptions().begin();

	for (; current != particleEffectDescription.getParticleEmitterGroupDescriptions().end(); ++current)
	{
		// Add a new child

		ParticleEmitterGroupListViewItem *particleEmitterGroupListViewItem = addChild(ParticleEffectListViewer::getParticleEmitterGroupName("Default"));

		// Set the child description

		particleEmitterGroupListViewItem->loadFromDescription(*current);
	}

	m_particleEffectDescription.clearParticleEmitterGroupDescriptions();
}

//-----------------------------------------------------------------------------
void ParticleEffectListViewItem::setDescription(ParticleEffectDescription const particleEffectDescription)
{
	m_particleEffectDescription = particleEffectDescription;
	m_particleEffectDescription.clearParticleEmitterGroupDescriptions();
}

//-----------------------------------------------------------------------------
ParticleEmitterGroupListViewItem *ParticleEffectListViewItem::addChild(std::string const &name)
{
	UNREF(name);

	ParticleEmitterGroupListViewItem *particleEmitterGroupListViewItem = new ParticleEmitterGroupListViewItem(this, name.c_str());
	insertItem(particleEmitterGroupListViewItem);
	m_particleEmitterGroupListViewItemList.push_back(particleEmitterGroupListViewItem);

	setOpen(true);

	return particleEmitterGroupListViewItem;
}

//-----------------------------------------------------------------------------
ParticleEffectDescription const & ParticleEffectListViewItem::getParticleEffectDescription() const
{
	return m_particleEffectDescription;
}

//-----------------------------------------------------------------------------
void ParticleEffectListViewItem::updateParticleCount()
{
	//char text[256];
	//sprintf(text, "%4d", ParticleManager::getParticleCount());
	//setText(1, text);
	//
	//ParticleEffectList::iterator current = m_particleEffectList.begin();
	//
	//for (; current != m_particleEffectList.end(); ++current)
	//{
	//	(*current)->updateParticleCount();
	//}
}

// ============================================================================
//
// ParticleEffectGroupListViewItem
//
// ============================================================================

//-----------------------------------------------------------------------------
ParticleEffectGroupListViewItem::ParticleEffectGroupListViewItem(QListView *parent, char const *name)
 : ParticleEditorListViewItem(parent, name)
 , m_particleEffectListViewItem(NULL)
{
}

//-----------------------------------------------------------------------------
ParticleEffectGroupListViewItem::~ParticleEffectGroupListViewItem()
{
}

//-----------------------------------------------------------------------------
void ParticleEffectGroupListViewItem::write(Iff &iff)
{
	ParticleEffectDescription particleEffectDescription;
	m_particleEffectListViewItem->addToDescription(particleEffectDescription);
	particleEffectDescription.write(iff);
}

//-----------------------------------------------------------------------------
void ParticleEffectGroupListViewItem::load(Iff &iff)
{
	// Create a description and load it

	ParticleEffectDescription particleEffectDescription;
	particleEffectDescription.load(iff);

	// Set the particle effect name

	if (iff.getFileName() != NULL)
	{
		particleEffectDescription.setName(FileNameUtils::get(iff.getFileName(), FileNameUtils::fileName | FileNameUtils::extension).c_str());
	}
	else
	{
		particleEffectDescription.setName("Default");
	}

	// Add the list view item

	ParticleEffectListViewItem *particleEffectListViewItem = addChild(ParticleEffectListViewer::getParticleEffectName(particleEffectDescription.getName().c_str()));
	particleEffectListViewItem->loadFromDescription(particleEffectDescription);
}

//-----------------------------------------------------------------------------
ParticleEffectListViewItem *ParticleEffectGroupListViewItem::addChild(std::string const &name)
{
	delete m_particleEffectListViewItem;
	m_particleEffectListViewItem = new ParticleEffectListViewItem(this, name.c_str());

	insertItem(m_particleEffectListViewItem);

	setOpen(true);
	return m_particleEffectListViewItem;
}

//-----------------------------------------------------------------------------
void ParticleEffectGroupListViewItem::setParticleEffectFileName(std::string const & fileName, int const particleCount)
{
	std::string const trimmedFileName(FileNameUtils::get(fileName, FileNameUtils::fileName | FileNameUtils::extension));
	std::string const finalName(ParticleEffectListViewer::getParticleEffectName(trimmedFileName.empty() ? "Default" : trimmedFileName.c_str()));
	QString text;
	text.sprintf("%s  %d", finalName.c_str(), particleCount);

	m_particleEffectListViewItem->setText(0, text);
}

// ============================================================================
//
// ParticleEffectListViewer
//
// ============================================================================

//-----------------------------------------------------------------------------
ParticleEffectListViewer::ParticleEffectListViewer(QWidget *parent, char const *name, WFlags const flags)
 : QListView(parent, name, flags)
 , m_selectedListViewItem(NULL)
 , m_rootListViewItem(NULL)
 , m_timer(new QTimer(this, "ParticleCountTimer"))
{
	setFixedWidth(370);
	setRootIsDecorated(true);
	addColumn("Item", 192);

	// These must be set after the column is added

	setColumnWidthMode(0, QListView::Maximum);
	setColumnWidth(0, 250);

	// Set the min dimensions

	setMinimumHeight(256);

	// Create the default particle effect

	reset();

	// Setup a timer

	connect(m_timer, SIGNAL(timeout()), SLOT(onTimerTimeout()));
	m_timer->start(1000 / 24);
}

//-----------------------------------------------------------------------------
void ParticleEffectListViewer::setObject(Object const *object)
{
	m_object = object;
}

//-----------------------------------------------------------------------------
void ParticleEffectListViewer::reset()
{
	// Create the root item

	delete m_rootListViewItem;
	m_rootListViewItem = new ParticleEffectGroupListViewItem(this, "Particle Effects");
	m_rootListViewItem->addChild(getParticleEffectName("Default"))->addChild(getParticleEmitterGroupName("Default"))->addChild(getParticleEmitterName("Default"))->addChild(getParticleName("No Shader", NULL));

	// Set the initial selected section

	setSelectedListViewItem(m_rootListViewItem);
	setSorting(-1);
}

//-----------------------------------------------------------------------------
std::string ParticleEffectListViewer::getParticleEffectName(char const *text)
{
	char name[256];
	sprintf(name, "Particle Effect (~p %s ~n)", text);

	return name;
}

//-----------------------------------------------------------------------------
std::string ParticleEffectListViewer::getParticleEmitterGroupName(char const *text)
{
	char name[256];
	sprintf(name, "Emitter Group (~g %s ~n)", text);

	return name;
}

//-----------------------------------------------------------------------------
std::string ParticleEffectListViewer::getParticleEmitterName(char const *text)
{
	char name[256];
	sprintf(name, "Emitter (~r %s ~n)", text);

	return name;
}

//-----------------------------------------------------------------------------
std::string ParticleEffectListViewer::getParticleName(char const *text, ParticleDescription const *particleDescription)
{
	char name[256];

	if ((particleDescription == NULL) ||
	    (particleDescription->getParticleType() == ParticleDescription::PT_quad))
	{
		snprintf(name, sizeof(name), "Particle Quad (~b %s ~n)", text);
		name[sizeof(name) - 1] = '\0';
	}
	else
	{
		if (particleDescription->getParticleType() == ParticleDescription::PT_mesh)
		{
			snprintf(name, sizeof(name), "Particle Mesh (~b %s ~n)", text);
			name[sizeof(name) - 1] = '\0';
		}
		else
		{
			DEBUG_FATAL(true, ("Un-supported particle type."));
		}
	}

	return name;
}

//-----------------------------------------------------------------------------
std::string ParticleEffectListViewer::getParticleAttachmentName(char const *text)
{
	char name[256];
	sprintf(name, "Attachment (~p %s ~n)", text);

	return name;
}

//-----------------------------------------------------------------------------
void ParticleEffectListViewer::setSelectedListViewItem(QListViewItem *listViewItem)
{
	if (listViewItem != NULL)
	{
		// Save the newly selected list view item

		m_selectedListViewItem = dynamic_cast<ParticleEditorListViewItem *> (listViewItem);

		setSelected(m_selectedListViewItem, true);

		// Figure out which type of item was clicked

		if (m_rootListViewItem == listViewItem)
		{
			// Particle Effect Group

			emit signalUnDefine();
			emit showParticleEffectGroupAttributes();
		}
		else if (m_selectedListViewItem->getTag() == ParticleEditorListViewItem::T_particleEffect)
		{
			// Particle Effect

			ParticleEffectListViewItem *particleEffectListViewItem = safe_cast<ParticleEffectListViewItem *>(listViewItem);
			ParticleEffectDescription const &particleEffectDescription = particleEffectListViewItem->getParticleEffectDescription();

			emit signalUnDefine();
			emit showParticleEffectAttributes(particleEffectDescription);
		}
		else if (m_selectedListViewItem->getTag() == ParticleEditorListViewItem::T_particleEmitterGroup)
		{
			// Particle Emitter Group

			ParticleEmitterGroupListViewItem *particleEmitterGroupListViewItem = safe_cast<ParticleEmitterGroupListViewItem *>(listViewItem);
			ParticleEmitterGroupDescription const &particleEmitterGroupDescription = particleEmitterGroupListViewItem->getParticleEmitterGroupDescription();

			emit signalUnDefine();
			emit showParticleEmitterGroupAttributes(particleEmitterGroupDescription);
		}
		else if (m_selectedListViewItem->getTag() == ParticleEditorListViewItem::T_particleEmitter)
		{
			// Particle Emitter
			
			ParticleEmitterListViewItem *particleEmitterListViewItem = safe_cast<ParticleEmitterListViewItem *>(listViewItem);
			ParticleEmitterDescription const &particleEmitterDescription = particleEmitterListViewItem->getParticleEmitterDescription();

			emit showParticleEmitterAttributes(particleEmitterDescription);
		}
		else if (m_selectedListViewItem->getTag() == ParticleEditorListViewItem::T_particle)
		{
			// Particle

			ParticleListViewItem *particleListViewItem = safe_cast<ParticleListViewItem *>(listViewItem);
			ParticleDescription const &particleDescription = particleListViewItem->getParticleDescription();

			emit showParticleAttributes(particleDescription);
		}
		else if (m_selectedListViewItem->getTag() == ParticleEditorListViewItem::T_particleAttachment)
		{
			// Particle Attachment

			ParticleAttachmentListViewItem *particleAttachmentListViewItem = safe_cast<ParticleAttachmentListViewItem *>(listViewItem);
			ParticleAttachmentDescription const &particleAttachmentDescription = particleAttachmentListViewItem->getParticleAttachmentDescription();

			emit signalUnDefine();
			emit showParticleAttachmentAttributes(particleAttachmentDescription);
		}
	}
}

//-----------------------------------------------------------------------------
void ParticleEffectListViewer::contentsMouseReleaseEvent(QMouseEvent *mouseEvent)
{
	int button = mouseEvent->button();
	QListViewItem *selectedListViewItem = selectedItem();

	ParticleEditorListViewItem *particleEditorListViewItem = dynamic_cast<ParticleEditorListViewItem *>(selectedListViewItem);

	if ((selectedListViewItem != NULL) &&
	    (particleEditorListViewItem != NULL))
	{
		// Signal to change to the selected list view item

		setSelectedListViewItem(selectedListViewItem);

		// Figure out which type of item was clicked

		if (m_rootListViewItem == selectedListViewItem)
		{
			// Particle Effect was clicked

			if (button == Qt::RightButton)
			{
			}
		}
		else if (particleEditorListViewItem->getTag() == ParticleEditorListViewItem::T_particleEffect)
		{
			// An effect was clicked

			if (button == Qt::RightButton)
			{
				showParticleEffectPopUpMenu(mouseEvent->globalPos());
			}
		}
		else if (particleEditorListViewItem->getTag() == ParticleEditorListViewItem::T_particleEmitterGroup)
		{
			// An emitter group was clicked

			if (button == Qt::RightButton)
			{
				showParticleEmitterGroupPopUpMenu(mouseEvent->globalPos());
			}
		}
		else if (particleEditorListViewItem->getTag() == ParticleEditorListViewItem::T_particleEmitter)
		{
			// An emitter was clicked

			if (button == Qt::RightButton)
			{
				showParticleEmitterPopUpMenu(mouseEvent->globalPos());
			}
		}
		else if (particleEditorListViewItem->getTag() == ParticleEditorListViewItem::T_particle)
		{
			// A particle was clicked

			if (button == Qt::RightButton)
			{
				showParticlePopUpMenu(mouseEvent->globalPos());
			}
		}
		else if (particleEditorListViewItem->getTag() == ParticleEditorListViewItem::T_particleAttachment)
		{
			// A particle was clicked

			if (button == Qt::RightButton)
			{
				showParticleAttachmentPopUpMenu(mouseEvent->globalPos());
			}
		}
	}
}

//-----------------------------------------------------------------------------
void ParticleEffectListViewer::write(Iff &iff)
{
	NOT_NULL(m_rootListViewItem);

	m_rootListViewItem->write(iff);

	//if (m_selectedListViewItem != NULL)
	//{
	//	m_selectedListViewItem->write(iff);
	//}
}

//-----------------------------------------------------------------------------
void ParticleEffectListViewer::load(Iff &iff)
{
	NOT_NULL(m_rootListViewItem);

	// Remove the items from the list

	delete m_rootListViewItem;

	// Create a new root item

	m_rootListViewItem = new ParticleEffectGroupListViewItem(this, "Particle Effects");
	m_rootListViewItem->load(iff);

	// Select the root item

	setSelectedListViewItem(m_rootListViewItem);
}

//-----------------------------------------------------------------------------
void ParticleEffectListViewer::onAttributeNameChanged(const std::string &name)
{
	if (m_selectedListViewItem != NULL)
	{
		if (m_selectedListViewItem->getTag() == ParticleEditorListViewItem::T_particle)
		{
			ParticleListViewItem *particleListViewItem = safe_cast<ParticleListViewItem *>(m_selectedListViewItem);

			particleListViewItem->setName(name.c_str());
		}
		else if (m_selectedListViewItem->getTag() == ParticleEditorListViewItem::T_particleEmitter)
		{
			ParticleEmitterListViewItem *particleEmitterListViewItem = safe_cast<ParticleEmitterListViewItem *>(m_selectedListViewItem);

			particleEmitterListViewItem->setName(name.c_str());
		}
	}
}

//-----------------------------------------------------------------------------
void ParticleEffectListViewer::onParticleAttachmentDescriptionChanged(const ParticleAttachmentDescription &particleAttachmentDescription)
{
	ParticleAttachmentListViewItem *particleAttachmentListViewItem = dynamic_cast<ParticleAttachmentListViewItem *>(m_selectedListViewItem);

	if (particleAttachmentListViewItem != NULL)
	{
		particleAttachmentListViewItem->setDescription(particleAttachmentDescription);
	}
}

//-----------------------------------------------------------------------------
void ParticleEffectListViewer::onParticleDescriptionChanged(const ParticleDescription &particleDescription)
{
	ParticleListViewItem *particleListViewItem = dynamic_cast<ParticleListViewItem *>(m_selectedListViewItem);

	if (particleListViewItem != NULL)
	{
		particleListViewItem->setDescription(particleDescription);
	}
}

//-----------------------------------------------------------------------------
void ParticleEffectListViewer::onParticleEmitterDescriptionChanged(const ParticleEmitterDescription &particleEmitterDescription)
{
	ParticleEmitterListViewItem *particleEmitterListViewItem = dynamic_cast<ParticleEmitterListViewItem *>(m_selectedListViewItem);

	if (particleEmitterListViewItem != NULL)
	{
		particleEmitterListViewItem->setDescription(particleEmitterDescription);
	}
}

//-----------------------------------------------------------------------------
void ParticleEffectListViewer::onParticleEmitterGroupDescriptionChanged(const ParticleEmitterGroupDescription &particleEmitterGroupDescription)
{
	ParticleEmitterGroupListViewItem *particleEmitterGroupListViewItem = dynamic_cast<ParticleEmitterGroupListViewItem *>(m_selectedListViewItem);

	if (particleEmitterGroupListViewItem != NULL)
	{
		particleEmitterGroupListViewItem->setDescription(particleEmitterGroupDescription);
	}
}

//-----------------------------------------------------------------------------
void ParticleEffectListViewer::onParticleEffectDescriptionChanged(const ParticleEffectDescription &particleEffectDescription)
{
	ParticleEffectListViewItem *particleEffectListViewItem = dynamic_cast<ParticleEffectListViewItem *>(m_selectedListViewItem);

	if (particleEffectListViewItem != NULL)
	{
		particleEffectListViewItem->setDescription(particleEffectDescription);
	}
}

//-----------------------------------------------------------------------------
void ParticleEffectListViewer::setParticleEffectFileName(std::string const &fileName)
{
	m_rootListViewItem->setParticleEffectFileName(fileName, 0);
}

//-----------------------------------------------------------------------------
void ParticleEffectListViewer::showParticleEffectPopUpMenu(QPoint const &point)
{
	UNREF(point);

	return;
	//QPopupMenu addPopUpMenu(this, "ParticleEffectPopUpMenu");
	//
	//enum AddFieldItems
	//{
	//	AFI_flocking,
	//	AFI_gravity,
	//	AFI_turbulence,
	//	AFI_uniform,
	//	AFI_vortex
	//};
	//
	//addPopUpMenu.insertItem("Flocking", AFI_flocking);
	//addPopUpMenu.insertItem("Gravity", AFI_gravity);
	//addPopUpMenu.insertItem("Turbulence", AFI_turbulence);
	//addPopUpMenu.insertItem("Uniform", AFI_uniform);
	//addPopUpMenu.insertItem("Vortex", AFI_vortex);
	//
	//QPopupMenu popUpMenu(this, "ParticleEffectPopUpMenu");
	//popUpMenu.insertItem("Add Field", &addPopUpMenu);
	//
	//int result = popUpMenu.exec(point);
	//
	//switch (result)
	//{
	//	case AFI_flocking:
	//		{
	//		}
	//		break;
	//	case AFI_gravity:
	//		{
	//		}
	//		break;
	//	case AFI_turbulence:
	//		{
	//		}
	//		break;
	//	case AFI_uniform:
	//		{
	//		}
	//		break;
	//	case AFI_vortex:
	//		{
	//		}
	//		break;
	//	default:
	//		{
	//			if (result != -1)
	//			{
	//				DEBUG_FATAL(true, ("Unknown field selected."));
	//			}
	//		}
	//		break;
	//}
}

//-----------------------------------------------------------------------------
void ParticleEffectListViewer::showParticleEmitterGroupPopUpMenu(QPoint const &point)
{
	QPopupMenu emitterGroupPopUpMenu(this, "emitterGroupPopUpMenu");

	enum MenuItems
	{
		MI_addEmitter,
		MI_importEmitter
	};

	emitterGroupPopUpMenu.insertItem("Add Emitter", MI_addEmitter);
	emitterGroupPopUpMenu.insertSeparator();
	emitterGroupPopUpMenu.insertItem("Import Emitter", MI_importEmitter);

	// Disable delete if there is only one emitter left in the list

	ParticleEmitterGroupListViewItem *particleEmitterGroupListViewItem = dynamic_cast<ParticleEmitterGroupListViewItem *>(m_selectedListViewItem);

	// Get the result

	int result = emitterGroupPopUpMenu.exec(point);

	switch (result)
	{
		case MI_addEmitter:
			{
				if (particleEmitterGroupListViewItem != NULL)
				{
					// Add a new default emitter and particle

					particleEmitterGroupListViewItem->addChild(getParticleEmitterName("Default"))->addChild(getParticleName("Default", NULL));

					// Signal to rebuild the particle effect

					emit forceRebuild();
				}
			}
			break;
		case MI_importEmitter:
			{
				// Get the last used path
				
				QSettings settings;
				settings.insertSearchPath(QSettings::Windows, ParticleEditorUtility::getSearchPath());
				QString previousUsedPath(settings.readEntry("ParticleEffectFilePath", "c:\\"));
				
				// Get the file
				
				QString selectedFileName(QFileDialog::getOpenFileName(previousUsedPath, "Particle Effect files (*.prt)", this, "OpenFileDialog", "Import Particle Effect"));
				std::string path(!selectedFileName.isNull() ? static_cast<char const *>(selectedFileName) : "");
				
				// Make sure the specified file exists on disk
				
				if (FileNameUtils::isReadable(path))
				{
					// Save the path
				
					settings.writeEntry("ParticleEffectFilePath", path.c_str());

					// Make sure this is a valid particle effect Iff file

					if (ParticleEditorUtility::isValidIffFile(this, path.c_str(), ParticleEffectAppearanceTemplate::getTag()))
					{
						if (particleEmitterGroupListViewItem != NULL)
						{
							// Get the import particle effect iff
						
							Iff iff(path.c_str());
							ParticleEffectDescription particleEffectDescription;
							particleEffectDescription.load(iff);
							
							// Import all the emitters

							ParticleEffectDescription::ParticleEmitterGroupDescriptions::const_iterator currentParticleEmitterGroupDescription = particleEffectDescription.getParticleEmitterGroupDescriptions().begin();

							for (; currentParticleEmitterGroupDescription != particleEffectDescription.getParticleEmitterGroupDescriptions().end(); ++currentParticleEmitterGroupDescription)
							{
								ParticleEmitterGroupDescription::ParticleEmitterDescriptions::const_iterator currentParticleEmitterDescription = currentParticleEmitterGroupDescription->getParticleEmitterDescriptions().begin();

								for (; currentParticleEmitterDescription != currentParticleEmitterGroupDescription->getParticleEmitterDescriptions().end(); ++currentParticleEmitterDescription)
								{
									particleEmitterGroupListViewItem->addEmitter(*currentParticleEmitterDescription);
								}
							}
						}

						// Signal to rebuild the particle effect

						emit forceRebuild();
					}
				}
				else if (!path.empty())
				{
					std::string text("The selected file does not exist on disk: \"" + FileNameUtils::get(path, FileNameUtils::fileName | FileNameUtils::extension) + "\"");
					QMessageBox::warning(this, "File Does Not Exist", text.c_str(), "OK");
				}
			}
			break;
		default:
		{
			if (result != -1)
			{
				DEBUG_FATAL(true, ("Popup menu option not defined"));
			}
		}
	}
}

//-----------------------------------------------------------------------------
void ParticleEffectListViewer::showParticleEmitterPopUpMenu(QPoint const &point)
{
	QPopupMenu emitterPopUpMenu(this, "emitterPopUpMenu");

	enum MenuItems
	{
		   MI_reset
		,  MI_deleteMe
	};

	emitterPopUpMenu.insertItem("Reset", MI_reset);
	emitterPopUpMenu.insertSeparator();
	emitterPopUpMenu.insertItem("Delete Me", MI_deleteMe);

	// Disable delete if there is only one emitter left in the list

	ParticleEmitterListViewItem *particleEmitterListViewItem = dynamic_cast<ParticleEmitterListViewItem *>(m_selectedListViewItem);

	emitterPopUpMenu.setItemEnabled(MI_deleteMe, (particleEmitterListViewItem->parent()->childCount() > 1));

	// Get the result

	int result = emitterPopUpMenu.exec(point);

	// Do what is necessary

	switch (result)
	{
		case MI_reset:
			{
				ParticleEmitterListViewItem *particleEmitterListViewItem = dynamic_cast<ParticleEmitterListViewItem *>(m_selectedListViewItem);

				if (particleEmitterListViewItem != NULL)
				{
					// Create a default description

					ParticleEmitterDescription particleEmitterDescription;
					particleEmitterDescription.initializeDefault();

					// Reset to the default description

					particleEmitterListViewItem->setDescription(particleEmitterDescription);

					// Signal to show the new default description

					emit showParticleEmitterAttributes(particleEmitterListViewItem->getParticleEmitterDescription());

					// Signal to rebuild the particle effect

					emit forceRebuild();
				}
			}
			break;
		case MI_deleteMe:
			{
				// Save the parent item

				ParticleEmitterGroupListViewItem *particleEmitterGroupListViewItem = dynamic_cast<ParticleEmitterGroupListViewItem *>(m_selectedListViewItem->parent());
				
				// Remove the item from the description list

				particleEmitterGroupListViewItem->removeChild(m_selectedListViewItem);

				// Delete the list view item

				delete m_selectedListViewItem;

				// Set the selected item to the parent item

				m_selectedListViewItem = dynamic_cast<ParticleEditorListViewItem *>(particleEmitterGroupListViewItem);

				// Signal to rebuild the particle effect

				emit forceRebuild();
			}
			break;
		default:
		{
			if (result != -1)
			{
				DEBUG_FATAL(true, ("Popup menu option not defined"));
			}
		}
	}
}

//-----------------------------------------------------------------------------
void ParticleEffectListViewer::showParticlePopUpMenu(QPoint const &point)
{
	QPopupMenu popupMenu(this, "popupMenu");

	enum MenuItems
	{
		MI_reset,
		MI_attach
	};

	popupMenu.insertItem("Reset", MI_reset);
	popupMenu.insertSeparator();
	popupMenu.insertItem("Attach", MI_attach);

	// Only allow one attachment

	if (m_selectedListViewItem->childCount() > 0)
	{
		popupMenu.setItemEnabled(MI_attach, false);
	}

	// Get the result

	int result = popupMenu.exec(point);

	// Do what is necessary

	ParticleListViewItem *particleListViewItem = dynamic_cast<ParticleListViewItem *>(m_selectedListViewItem);

	switch (result)
	{
		case MI_reset:
			{
				if (particleListViewItem != NULL)
				{
					// Reset to the default description

					particleListViewItem->initializeDefault();

					// Signal to show the new default description

					emit showParticleAttributes(particleListViewItem->getParticleDescription());

					// Signal to rebuild the particle effect

					emit forceRebuild();
				}
			}
			break;
		case MI_attach:
			{
				if (particleListViewItem != NULL)
				{
					// Add a new default emitter and particle

					particleListViewItem->addChild(getParticleAttachmentName("Default"));

					// Signal to show the new particle attachment description

					emit showParticleAttributes(particleListViewItem->getParticleDescription());

					// Signal to rebuild the particle effect

					emit forceRebuild();
				}
			}
			break;
		default:
		{
			if (result != -1)
			{
				DEBUG_FATAL(true, ("Popup menu option not defined"));
			}
		}
	}
}

//-----------------------------------------------------------------------------
void ParticleEffectListViewer::showParticleAttachmentPopUpMenu(QPoint const &point)
{
	QPopupMenu popupMenu(this, "popupMenu");

	enum MenuItems
	{
		MI_delete
	};

	popupMenu.insertItem("Delete", MI_delete);

	// Get the result

	int result = popupMenu.exec(point);

	// Do what is necessary

	switch (result)
	{
		case MI_delete:
			{
				// Save the parent item

				ParticleListViewItem *particleListViewItem = dynamic_cast<ParticleListViewItem *>(m_selectedListViewItem->parent());
				
				// Remove the item from the description list

				particleListViewItem->removeChild(m_selectedListViewItem);

				// Delete the list view item

				delete m_selectedListViewItem;

				// Set the selected item to the parent item

				m_selectedListViewItem = dynamic_cast<ParticleEditorListViewItem *>(particleListViewItem);

				// Signal to rebuild the particle effect

				emit forceRebuild();
			}
			break;
		default:
		{
			if (result != -1)
			{
				DEBUG_FATAL(true, ("Popup menu option not defined"));
			}
		}
	}
}

//-----------------------------------------------------------------------------
void ParticleEffectListViewer::onTimerTimeout()
{
	if (m_rootListViewItem != NULL)
	{
		if (m_object != NULL)
		{
			ParticleEffectAppearance const * const particleEffectAppearance = ParticleEffectAppearance::asParticleEffectAppearance(m_object->getAppearance());

			if (particleEffectAppearance != NULL)
			{
				m_rootListViewItem->setParticleEffectFileName(particleEffectAppearance->getAppearanceTemplateName(), particleEffectAppearance->getParticleCount());
			}
		}
	}
}

// ============================================================================
