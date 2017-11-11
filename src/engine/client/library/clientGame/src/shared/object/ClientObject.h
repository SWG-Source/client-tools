// ======================================================================
//
// ClientObject.h
// Copyright 2001, 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef	INCLUDED_ClientObject_H
#define	INCLUDED_ClientObject_H

//-----------------------------------------------------------------------

#include "Archive/AutoDeltaVariableCallback.h"
#include "StringId.h"
#include "Unicode.h"
#include "clientGame/ClientWorld.h"
#include "localizationArchive/StringIdArchive.h"
#include "sharedObject/Object.h"
#include "sharedTerrain/TerrainGenerator.h"
#include "unicodeArchive/UnicodeArchive.h"

//-----------------------------------------------------------------------

class ArrangementDescriptor;
class BaselinesMessage;
class CellObject;
class ClientDataFile;
class ClientSynchronizedUi;
class CreatureObject;
class DeltasMessage;
class SharedObjectTemplate;
class ShipObject;
class StaticObject;
class TangibleObject;
class UpdatePostureMessage;
class VectorArgb;
class WeaponObject;

//-----------------------------------------------------------------------

class ClientObject : public Object
{
public:

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	struct Messages
	{
		typedef std::pair <ClientObject *, ClientObject *> ContainerMsg;
		struct AddedToContainer;
		struct RemovedFromContainer;
		struct ComplexityChange;
		struct BankBalance;
		struct CashBalance;

		struct NameChanged
		{
			typedef ClientObject Payload;
		};
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	typedef stdvector<Object*>::fwd  ObjectVector;

public:

	static void install (bool viewer);
	static ClientObject * findClientObjectByUniqueId (uint32 id);

public:	

	explicit ClientObject (const SharedObjectTemplate* newTemplate, const ObjectNotification &notification = ClientWorld::getTangibleNotification());
	virtual ~ClientObject();

	virtual ClientObject *         asClientObject();
	virtual ClientObject const *   asClientObject() const;
	virtual CreatureObject *       asCreatureObject();
	virtual CreatureObject const * asCreatureObject() const;
	virtual StaticObject *         asStaticObject();
	virtual StaticObject const *   asStaticObject() const;
	virtual TangibleObject *       asTangibleObject();
	virtual TangibleObject const * asTangibleObject() const;
	virtual ShipObject *           asShipObject();
	virtual ShipObject const *     asShipObject() const;
	virtual WeaponObject *         asWeaponObject();
	virtual WeaponObject const *   asWeaponObject() const;
	virtual CellObject *           asCellObject();
	virtual CellObject const *     asCellObject() const;

	virtual float          alter(float time);

	void                   applyBaselines(const BaselinesMessage & source);
	void                   applyDeltas(const DeltasMessage & source);
	void                   beginBaselines();
	virtual void           endBaselines();
	void                   updateContainment(NetworkId const &containerId, int slotArrangement);

	virtual void           createDefaultController();

	virtual bool           isInitialized() const;
	bool                   isSettingBaselines () const;
	virtual bool           isTargettable() const;

	const unsigned long    getAuthServerProcessId() const;
	const char            *getTemplateName(void) const; //lint !e1511 hiding non-virt member

	const Unicode::String &getLocalizedName() const;
	const Unicode::String &getLocalizedEnglishName() const;
	const Unicode::String  getLocalizedFirstName() const;
	const Unicode::String &getObjectName() const;
	const StringId &       getObjectNameStringId () const;
	const StringId &       getDescriptionStringId () const;
	const bool             getLocalizedDescription (Unicode::String & str) const;

	void                   setObjectName(const Unicode::String &newName);
	void                   setObjectNameStringId(const StringId & id);

	ClientObject*          getContainedBy() const;
	virtual int            getVolume() const;
	void                   setVolumeClientSideOnly(int newVolume);

	virtual void           setParentCell(CellProperty *cellProperty);

	virtual void           containedByModified(const NetworkId & oldValue, const NetworkId & newValue,bool isLocal);
	virtual void           arrangementModified(int oldValue, int newValue, bool isLocal);
	virtual void           addedToContainer(ClientObject &containedObject, int arrangement);
	virtual void           removedFromContainer(ClientObject &containedObject);

	const ClientSynchronizedUi  *getClientSynchronizedUi () const;
	ClientSynchronizedUi        *getClientSynchronizedUi ();

	void                   setTransferInProgress (bool b);

	float                  getComplexity () const;
	void                   setComplexity (float c);

	void                   setClientCached ();
	bool                   isClientCached () const;

	int                     getGameObjectType              () const;
	const std::string &     getGameObjectTypeName          () const;
	const StringId &        getGameObjectTypeStringId      () const;
	const Unicode::String & getGameObjectTypeLocalizedName () const;
	bool                    isGameObjectTypeOf             (int type) const;

	Object *                addFlyText                     (const Unicode::String & flyText, float fadeTime, const VectorArgb & color, float scale, int textType, bool fadeOut = true, bool renderWithChatBubbles = true, bool createInWorldspace = false);

	int                     getBankBalance                 () const;
	int                     getCashBalance                 () const;

	uint32                  getUniqueId                    () const;

	int                     getNumberOfAuxilliaryObjects   () const;
	void                    addAuxilliaryObject            (Object *object);
	Object *                getAuxilliaryObject            (int index);
	Object const *          getAuxilliaryObject            (int index) const;

	static const NetworkId  getNextFakeNetworkId           ();
	static const boolean    isFakeNetworkId                (const NetworkId& id);

	const ClientDataFile* getClientData () const;

	TerrainGenerator::Layer* getLayer () const;
	void                     setLayer(TerrainGenerator::Layer* layer);

	virtual void getObjectInfo(stdmap<std::string, stdmap<std::string, Unicode::String>::fwd >::fwd & propertyMap) const;

protected:

	void                   addClientOnlyVariable(Archive::AutoDeltaVariableBase & source);
	void                   addClientServerVariable(Archive::AutoDeltaVariableBase & source);
	void                   addClientServerVariable_np(Archive::AutoDeltaVariableBase & source);
	void                   addFirstParentClientServerVariable(Archive::AutoDeltaVariableBase & source);
	void                   addFirstParentClientServerVariable_np(Archive::AutoDeltaVariableBase & source);
	void                   addSharedVariable(Archive::AutoDeltaVariableBase & source);
	void                   addSharedVariable_np(Archive::AutoDeltaVariableBase & source);

	virtual ClientSynchronizedUi * createSynchronizedUi ();
	
	/*
	*/
	void                   setSynchronizedUi (ClientSynchronizedUi * sync);

	virtual void           filterLocalizedName (Unicode::String & localizedName) const;
	void                   setLocalizedNameDirty ();
	void                   updateLocalizedName () const;

private:

	static void remove ();

private:

	void                   setBeginBaselines(bool beginBaselines);
	void                   handleContainerChange();
	void                   depersistContainedBy(NetworkId const &oldParent, NetworkId const &newParent, bool isLocal);
	void                   handleContainerChangeWithWorld();

	// disabled
	ClientObject();
	ClientObject(const ClientObject & other);
	const ClientObject& operator=(const ClientObject & rhs);

private:

	//----------------------------------------------------------------------
	//-- AutoDeltaVariable Callbacks

	struct Callbacks
	{
		template <typename T, typename U> struct DefaultCallback
		{
			void modified (ClientObject & target, const U & old, const U & value, bool isLocal) const;
		};

		struct DummyMessagesName;
		struct DummyMessagesStringId;

		typedef DefaultCallback<Messages::NameChanged,       Unicode::String>  NameChange;
		typedef DefaultCallback<DummyMessagesStringId,       StringId>         StringIdChange;
		typedef DefaultCallback<Messages::ComplexityChange,  float>            ComplexityChange;
		typedef DefaultCallback<Messages::BankBalance,       int>              BankBalance;
		typedef DefaultCallback<Messages::CashBalance,       int>              CashBalance;
	};

	struct ContainedByChange
	{
		ContainedByChange(const NetworkId & old, const NetworkId & n, bool local);
		NetworkId m_oldParent;
		NetworkId m_newParent;
		bool      m_isLocal;
	};

	struct ArrangementChange
	{
		ArrangementChange(int const & old, int const & n, bool local);
		int       m_oldArrangement;
		int       m_newArrangement;
		bool      m_isLocal;
	};

	friend Callbacks::NameChange;
	friend Callbacks::StringIdChange;

protected:

	const ArrangementDescriptor     *m_arrangementDescriptor;
	Archive::AutoDeltaByteStream     m_authoritativeClientServerPackage;
	Archive::AutoDeltaByteStream     m_authoritativeClientServerPackage_np;
	Archive::AutoDeltaByteStream     m_firstParentAuthClientServerPackage;
	Archive::AutoDeltaByteStream     m_firstParentAuthClientServerPackage_np;
	Archive::AutoDeltaByteStream     m_sharedPackage;
	Archive::AutoDeltaByteStream     m_sharedPackage_np; //np refers to the not persisted version of this package

	ClientSynchronizedUi            *m_synchronizedUi;

	bool                             m_transferInProgress : 1;

	mutable bool                    m_localizedNameDirty : 1;

private:

#ifdef _DEBUG
	Transform m_startTransform_w;
#endif

	ContainedByChange*              m_containedByChange;
	ArrangementChange*              m_arrangementChange;

	bool                            m_beginBaselines : 1;
	bool                            m_initialized : 1;
	mutable Unicode::String         m_localizedName;
	mutable Unicode::String         m_localizedEnglishName;
	mutable bool                    m_lastFlyTextWasInvalidTarget : 1;               

	Archive::AutoDeltaVariable<int> 
		m_volume;

	Archive::AutoDeltaVariableCallback<Unicode::String, Callbacks::NameChange, ClientObject> 
		m_objectName;

	Archive::AutoDeltaVariableCallback<StringId, Callbacks::StringIdChange, ClientObject>
		m_nameStringId;

	Archive::AutoDeltaVariable<StringId> m_descriptionStringId;

	Archive::AutoDeltaVariableCallback<float, Callbacks::ComplexityChange, ClientObject>
		m_complexity;

	Archive::AutoDeltaVariable<unsigned long> m_authServerProcessId;

	Archive::AutoDeltaVariableCallback<int, Callbacks::CashBalance, ClientObject> m_cashBalance;
	Archive::AutoDeltaVariableCallback<int, Callbacks::BankBalance, ClientObject> m_bankBalance;

	bool                             m_clientCached : 1;
	uint32                           m_uniqueId;
	ObjectVector                    *m_auxilliaryObjectVector;

	float                            m_lastFlyTextTime;

#ifdef _DEBUG
	bool                             m_warnedNotInitialized : 1;
#endif

};

//-----------------------------------------------------------------------

inline const unsigned long ClientObject::getAuthServerProcessId() const
{
	return m_authServerProcessId.get();
}

//----------------------------------------------------------------------

inline const ClientSynchronizedUi * ClientObject::getClientSynchronizedUi () const
{
	return m_synchronizedUi;
}

//----------------------------------------------------------------------

inline ClientSynchronizedUi        *ClientObject::getClientSynchronizedUi ()
{
	return m_synchronizedUi;
}

//-----------------------------------------------------------------------

inline bool ClientObject::isTargettable() const
{
	//By default, object you see cannot be targetted.
	return false;
}

//----------------------------------------------------------------------

inline int ClientObject::getBankBalance () const
{
	return m_bankBalance.get ();
}

//----------------------------------------------------------------------

inline int ClientObject::getCashBalance () const
{
	return m_cashBalance.get ();
}

//----------------------------------------------------------------------

inline uint32 ClientObject::getUniqueId () const
{
	return m_uniqueId;
}

//-----------------------------------------------------------------------

#endif

