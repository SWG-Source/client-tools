//======================================================================
//
// DraftSchematicInfo.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_DraftSchematicInfo_H
#define INCLUDED_DraftSchematicInfo_H

//======================================================================

class ClientObject;
class MessageQueueDraftSlotsData;
class MessageQueueDraftSlotsDataOption;
class ManufactureSchematicObject;
class NetworkId;
class StringId;
class ResourceContainerObject;

//----------------------------------------------------------------------


class DraftSchematicInfo
{
public:

	struct Messages
	{		
		struct Changed
		{
			typedef DraftSchematicInfo Payload;
		};
	};

	friend class DraftSchematicManager;

	typedef MessageQueueDraftSlotsData           Slot;
	typedef MessageQueueDraftSlotsDataOption     Option;
	typedef stdvector<Slot>::fwd                 SlotVector;
	typedef std::pair<int, int>                  Weight;
	typedef stdvector<Weight>::fwd               WeightVector;
	typedef std::pair<int, WeightVector>         AttribWeight;
	typedef stdvector<AttribWeight>::fwd         AttribWeightVector;

	ClientObject *                    getClientObject                  () const;
	const SlotVector &                getSlots                         () const;
	const Unicode::String &           getLocalizedName                 () const;
	const std::pair<uint32, uint32> & getDraftSchematicTemplate        () const;
	uint32                            getServerDraftSchematicTemplate  () const;
	uint32                            getSharedDraftSchematicTemplate  () const;

	int                            getLastFrameUpdate               () const;

	bool                           formatDescription                (Unicode::String & header, Unicode::String & desc, Unicode::String & attribs, bool minimalAttribs, ResourceContainerObject* considerResource = NULL) const;
	void                           formatAttribs                    (Unicode::String & attribs, bool minimal, ResourceContainerObject* considerResource = NULL) const;

	                               DraftSchematicInfo               ();
	                               DraftSchematicInfo               (uint32 serverCrc, uint32 sharedCrc);
	explicit                       DraftSchematicInfo               (const std::pair<uint32, uint32> & crc);

	void                           setSlots                         (const SlotVector & sv);

	static StringId                convertNameStringIdToDescription (const StringId & id);
	static void                    createSlotDescription            (const Slot & slot, const ManufactureSchematicObject * manf_schem, Unicode::String & one, Unicode::String & two, Unicode::String & three);
	static bool                    createOptionDescription          (const Slot & slot, const ManufactureSchematicObject * manf_schem, int optionIndex, Unicode::String & str, bool printName, bool printDescription);

	void                           requestDataIfNeeded              () const;

	void                           setComplexity                    (int complexity);
	int                            getComplexity                    () const;

	void                           setManufactureSchematicVolume    (int volume);
	int                            getManufactureSchematicVolume    () const;

	void                           setResourceWeights               (const AttribWeightVector & weights, const AttribWeightVector & resourceMaxWeights);

	bool                           formatDraftAttribWeights         (Unicode::String & draftAttribs, const Unicode::String & expNameFilter, bool showWeights = true, ResourceContainerObject* considerResource = NULL) const;

	float                          calculateResourceMatch           (const NetworkId &resourceTypeId, int slot) const;

private:

	                               DraftSchematicInfo        (const DraftSchematicInfo & rhs);
	DraftSchematicInfo &           operator=                 (const DraftSchematicInfo & rhs);

	                              ~DraftSchematicInfo        ();
	void                           createClientObject        () const;

	std::pair<uint32, uint32>      m_draftSchematicTemplate;
	std::string                    m_craftedSharedTemplate;
	mutable ClientObject *         m_clientObject;
	SlotVector *                   m_slots;
	mutable int                    m_lastFrameUpdate;
	mutable bool                   m_slotsRequested;
	mutable bool                   m_slotsReceived;
	int                            m_complexity;
	int                            m_manufactureSchematicVolume;

	struct CachedAttribWeights;

	//-- map of attribute name to cached weight info
	typedef stdmap<Unicode::String, CachedAttribWeights>::fwd   CachedAttribWeightMap;

	//-- map of experimental attribs to cached attribute weight maps
	typedef stdmap<Unicode::String, CachedAttribWeightMap>::fwd CachedExpAttribWeightMap;

	CachedExpAttribWeightMap *     m_cachedExpAttribWeightMap;
	AttribWeightVector *           m_assemblyWeights;
	AttribWeightVector *           m_resourceMaxWeights;

	mutable bool                   m_weightsRequested;
	mutable bool                   m_weightsReceived;
};

//----------------------------------------------------------------------

inline ClientObject * DraftSchematicInfo::getClientObject () const
{
	if (!m_clientObject)	
		createClientObject ();

	return m_clientObject;
}

//----------------------------------------------------------------------

inline const DraftSchematicInfo::SlotVector & DraftSchematicInfo::getSlots () const
{
	return *NON_NULL (m_slots);
}

//----------------------------------------------------------------------

inline const std::pair<uint32, uint32> & DraftSchematicInfo::getDraftSchematicTemplate() const
{
	return m_draftSchematicTemplate;
}

inline uint32 DraftSchematicInfo::getServerDraftSchematicTemplate () const
{
	return m_draftSchematicTemplate.first;
}

//----------------------------------------------------------------------

inline uint32 DraftSchematicInfo::getSharedDraftSchematicTemplate () const
{
	return m_draftSchematicTemplate.second;
}

//----------------------------------------------------------------------

inline int DraftSchematicInfo::getLastFrameUpdate        () const
{
	return m_lastFrameUpdate;
}

//----------------------------------------------------------------------

inline int DraftSchematicInfo::getComplexity                    () const
{
	return m_complexity;
}

//----------------------------------------------------------------------

inline int DraftSchematicInfo::getManufactureSchematicVolume                        () const
{
	return m_manufactureSchematicVolume;
}

//======================================================================

#endif
