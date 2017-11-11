//======================================================================
//
// CuiPersistentMessageManager.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiPersistentMessageManager_H
#define INCLUDED_CuiPersistentMessageManager_H

//======================================================================

struct ChatPersistentMessageToClientData;
class AttachmentData;
class ClientObject;
class WaypointDataBase;

//----------------------------------------------------------------------

class CuiPersistentMessageManager
{
public:

	typedef ChatPersistentMessageToClientData        Data;
	typedef stdvector<Data>::fwd                     DataVector;
	typedef stdvector<std::string>::fwd              StringVector;
	typedef stdvector<AttachmentData *>::fwd         Attachments;
	typedef stdmap<int, Attachments>::fwd            AttachmentMap;

	struct Messages
	{
		struct HeaderReceived
		{
			typedef Data Payload;
		};

		struct BodyReceived
		{
			typedef Data Payload;
		};

		struct MessageFailed
		{
			typedef Unicode::String Payload;
		};

		struct AttachObject
		{
			typedef ClientObject Payload;
		};

		struct StartComposing
		{
			//-- fromCharacterName contains the embedded recipients list
			typedef ChatPersistentMessageToClientData Payload;
		};
		struct EmailListChanged
		{
			typedef bool Payload;
		};
	};

	static void                install                  ();
	static void                remove                   ();
	static void                clear                    ();
	static void                update(float const deltaTime);

	static const Data *        getData                  (uint32 id);
	static const Attachments * getAttachments           (uint32 id);
	static void                getDataVector            (DataVector & dv);
	static void                sendMessage              (const std::string & recepient,   const Unicode::String & subject, const Unicode::String & body, const Unicode::String & outOfBand);
	static void                sendMessageToMultiple    (const std::string & recepients,  const Unicode::String & subject, const Unicode::String & body, const Unicode::String & outOfBand);
	static void                sendMessage              (const StringVector & recepients, const Unicode::String & subject, const Unicode::String & body, const Unicode::String & outOfBand);
	static stdset<std::string>::fwd const & getPredefinedRecipients();

	static void                receiveMessage           (const Data & data);

	static void                deleteMessage            (uint32 id);
	static void                emptyMail                (const bool confirmed);
	static void                requestMessageBody       (uint32 id);

	static bool                validateRecepientsString (const std::string & targetName);

	static bool                parseRecepientsString    (const std::string & recepients, StringVector & sv);

	static bool                hasNewMail               ();
	static void                setHasNewMail            (bool b);

	static void                checkNewMailStatus       (bool status);
	static void                processData              (Data & data);

	static void                saveToDatapad            (AttachmentData & ad);

	static int                 getCompositionWindowCount   ();
	static void                setCompositionWindowCount   (int count);
	static void                attachObjectToComposeWindow (ClientObject & obj);

	static void                startComposingNewMessage    (const std::string & recipients, const Unicode::String & subject, const Unicode::String & body, const Unicode::String & oob);

	static void                startWritingEmailsToDisk ();

private:
	static void                setAttachments           (int32 id, Attachments & attachments);
	static bool                checkDatapadSaveOk       (const WaypointDataBase & wd);

	static void                finishWritingEmailsToDisk ();

	static bool                ms_hasNewMail;

	static int                 ms_compositionWindowCount;
};

//----------------------------------------------------------------------

inline bool CuiPersistentMessageManager::hasNewMail               ()
{
	return ms_hasNewMail;
}

//----------------------------------------------------------------------

inline int CuiPersistentMessageManager::getCompositionWindowCount   ()
{
	return ms_compositionWindowCount;
}

//======================================================================

#endif
