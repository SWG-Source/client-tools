// ======================================================================
//
// ObjectEditor.h
// copyright(c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ObjectEditor_H
#define INCLUDED_ObjectEditor_H

// ======================================================================

#include "BaseObjectEditor.h"

// ======================================================================
class ClientObject;
namespace MessageDispatch
{
	class Callback;
}

struct UpdateObjects;
//-----------------------------------------------------------------

/**
* The ObjectEditor allows the user to browse and modify all object properties.
*
* @todo: this should use QTables, not QListViews
* @todo: this should allow the user to modify properties
* @todo: ObjVar view/edit/add support should be coded
*/
class ObjectEditor : public BaseObjectEditor, public MessageDispatch::Receiver
{
	Q_OBJECT; //lint !e1516 !e19 !e1924 !e1762 various deficiencies in the Qt macro

public:
	explicit ObjectEditor(QWidget*theParent=0, const char*theName=0);
	virtual ~ObjectEditor();

public slots:
	void refreshObjects();
	void updateObjectData();
	void onAttributeRenamed(QListViewItem* item, int col, const QString &text) const;
	void onCreatureSkillsContextMenuRequested(QListBoxItem *, const QPoint &);
	void onRevokeCreatureSkill();
	void onRemoveScript();
	void onRemoveObjvar();
	void onSetObjvar();
	void onScriptsListContextMenuRequested(QListViewItem *, const QPoint &, int);
	void onObjvarListContextMenuRequested(QListViewItem *, const QPoint &, int);
public:
	virtual void receiveMessage(const MessageDispatch::Emitter& source, const MessageDispatch::MessageBase& message);

protected:
	void dragEnterEvent(QDragEnterEvent* event);
	void dropEvent     (QDropEvent* event);

private:
	//disabled
	ObjectEditor(const ObjectEditor& rhs);
	ObjectEditor& operator=(const ObjectEditor& rhs);

	void updateObjects(const UpdateObjects *);

private:
	struct PropertiesMenuItems
	{
		struct Client
		{
			struct Transform
			{
				QListViewItem*   translation;
				QListViewItem*     translateX;
				QListViewItem*     translateY;
				QListViewItem*     translateZ;
				QListViewItem*   rotation;
				QListViewItem*     pitch;
				QListViewItem*     yaw;
				QListViewItem*     roll;
				QListViewItem*   scale;
				QListViewItem*     scaleX;
				QListViewItem*     scaleY;
				QListViewItem*     scaleZ;
			};

			struct General
			{
				QListViewItem*   networkId;
				QListViewItem*   authoritative;
				QListViewItem*   watchable;
				QListViewItem*   active;
				QListViewItem*   appearance;
				QListViewItem*   objectTemplate;
				QListViewItem*   volume;
				QListViewItem*   script;
				QListViewItem*   slotType;
				QListViewItem*   name;
				QListViewItem*   container;
			};

			Transform         transform;
			General           general;

			QListViewItem*   transformItem;
			QListViewItem*   generalItem;
		};

		struct Tangible
		{
			QListViewItem*   weight;
			QListViewItem*   HP;
			QListViewItem*   maxHP;
			QListViewItem*   visible;
			QListViewItem*   popupHelp;
			QListViewItem*   armorEffectiveness;
			QListViewItem*   frozen;
			QListViewItem*   squelch;
			QListViewItem*   resourceList;

			QListViewItem*   tangibleItem;
		};

		struct Creature
		{
			struct Attributes
			{
				QListViewItem*    health;
				QListViewItem*    strength;
				QListViewItem*    constitution;
				QListViewItem*    action;
				QListViewItem*    quickness;
				QListViewItem*    stamina;
				QListViewItem*    mind;
				QListViewItem*    focus;
				QListViewItem*    willpower;
			};

			Attributes         attribs;
			Attributes         maxAttribs;

			QListViewItem*    mood;
			QListViewItem*    sayMode;
			QListViewItem*    gender;
			QListViewItem*    attribsItem;
			QListViewItem*    maxAttribsItem;

			QListViewItem*    creatureItem;
		};

		Client   client;
		Tangible tangible;
		Creature creature;

		struct Trigger
		{
			stdvector<QListViewItem*>::fwd triggers;
		};

		Trigger trigger;
	};

	PropertiesMenuItems     m_pmi;
	ClientObject*           m_obj;
	MessageDispatch::Callback * m_callback;
	void addInfoClientObject();
	void addInfoTangibleObject();
	void addInfoCreatureObject();
};

// ======================================================================

#endif
