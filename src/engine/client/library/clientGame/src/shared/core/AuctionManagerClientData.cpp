//======================================================================
//
// AuctionManagerClientData.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/AuctionManagerClientData.h"

#include "UnicodeUtils.h"
#include "clientUserInterface/CuiManager.h"
#include "sharedRandom/Random.h"
#include <cstdio>

//======================================================================

AuctionManagerClientData::AuctionManagerClientData () :
hasDetails         (false),
receiptTime        (0),
header             (),
details            (),
locationId         ()
{
}

//----------------------------------------------------------------------

AuctionManagerClientData::Header AuctionManagerClientData::constructSimulationHeader ()
{
	static int64 s_id = 0;
	static bool  s_nameType = false;

	Header header;

//	header.auctionId   = ++s_id;

	char buf [128];
	if (s_nameType)
		_snprintf (buf, sizeof (buf), "Item number %d", static_cast<int>(s_id));
	else
		_snprintf (buf, sizeof (buf), "@obj_n:unknown_object");

	s_nameType = !s_nameType;

	header.itemName  = Unicode::narrowToWide (buf);
	header.highBid   = (int16)(Random::random (100, 10000));
	header.timer     = Random::random (0,   500);
	header.buyNowPrice = ((Random::random (0, 1) != 0) ? header.highBid : 0);

	if (header.buyNowPrice > 0)
	{
	}
	else
	{
		_snprintf (buf, sizeof (buf), "highBidder_%d", Random::random (0, 5));
		header.highBidderName = buf;
	}

	header.location = "tatooine.in.time.55";

	_snprintf (buf, sizeof (buf), "theOwner_%d", Random::random (0, 5));
	header.ownerName       = buf;

	header.maxProxyBid     = Random::random(0,1000);
	header.myBid           = Random::random(0,1000);

	static int64 fake_id = 600000000;
	header.itemId = NetworkId (fake_id++);

	return header;
}

//----------------------------------------------------------------------

void AuctionManagerClientData::simulateDetails     (const NetworkId & itemId)
{
	Details _details;

	_details.itemId = itemId;
	_details.userDescription = Unicode::narrowToWide ("this is the user description");
	_details.templateName    = "object/creature/player/shared_wookiee_female.iff";

	setDetails (_details);
}

//----------------------------------------------------------------------

void AuctionManagerClientData::constructLocalizedName (Unicode::String & str) const 
{
	if (header.itemName.empty ())
	{
		str.clear ();
		return;
	}

	str = StringId::decodeString (header.itemName);

	/*
	const size_t nullpos = header.itemName.find (uint16 (0));
	if (nullpos != std::string::npos && nullpos > 0 && nullpos < (header.itemName.size () - 1))
	{
		const Unicode::String & first  = header.itemName.substr (0, nullpos);
		const Unicode::String & second = header.itemName.substr (nullpos + 1);
		str = StringId::decodeString (first) + StringId::decodeString (second);
	}
	else
	{
		Unicode::String itemName = header.itemName;
		if (nullpos != std::string::npos)
		{
			itemName.erase(nullpos);
		}
		if (itemName [0] == '@')
			str = StringId::decodeString (itemName);
		else
			str = itemName;
	}
*/
}

//----------------------------------------------------------------------

void AuctionManagerClientData::setDetails (const Details & _details)
{
	hasDetails  = true;
	receiptTime = 0;
	details     = _details;
}

//----------------------------------------------------------------------

void AuctionManagerClientData::setHeader              (const Header & _header)
{
	header     = _header;
	
	const char * whitespace = ".";

	size_t endpos = 0;
	std::string token;

	typedef std::vector<std::string> StringVector;
	static StringVector sv;
	sv.clear ();

	while (endpos != Unicode::String::npos && Unicode::getFirstToken (header.location, endpos, endpos, token, whitespace))
	{
		sv.push_back (token);
		if (endpos != Unicode::String::npos)
			++endpos;
	}

	if (sv.size () != 4)
	{
		locationId = NetworkId::cms_invalid;
		WARNING (true, ("AuctionManagerClientData [%s] invalid location [%s]", header.itemId.getValueString ().c_str (), header.location.c_str ()));
	}
	else
	{
		locationId = NetworkId (sv [3]);
	}
}

//----------------------------------------------------------------------

int AuctionManagerClientData::decodeAuctionItemNameString(std::vector<Unicode::String> & undecodedComponents, std::vector<Unicode::String> & decodedComponents) const
{
	undecodedComponents.clear();
	decodedComponents.clear();

	if (header.itemName.empty())
		return 0;

	int numComponentsLocalized = 0;
	size_t pos = 0;

	//-- an auction item name can be a series of tokens, seperated by nulls
	//-- each token can be an encoded stringid ("@table:name") or a literal string

	while (pos != std::string::npos)
	{
		size_t nullpos = header.itemName.find(Unicode::unicode_char_t (0), pos);			

		if (pos == 0 && nullpos == std::string::npos)
		{
			undecodedComponents.push_back(header.itemName);

			if (header.itemName[0] == '@')
			{
				const StringId sid(Unicode::wideToNarrow(header.itemName));
				decodedComponents.push_back(sid.localize(true));
				++numComponentsLocalized;
			}
			else 
			{
				decodedComponents.push_back(header.itemName);
			}

			return numComponentsLocalized;
		}

		const Unicode::String & token = nullpos == std::string::npos ? (pos == 0 ? header.itemName : header.itemName.substr(pos)) : header.itemName.substr(pos, nullpos - pos);	

		if (token.empty())
			break;

		undecodedComponents.push_back(token);
		if (token[0] == '@')
		{
			const StringId sid (Unicode::wideToNarrow(token));
			decodedComponents.push_back(sid.localize(true));
			++numComponentsLocalized;
		}
		else
		{
			decodedComponents.push_back(token);
		}

		if (nullpos == std::string::npos)
			break;

		pos = nullpos + 1;
	}

	return numComponentsLocalized;
}

//======================================================================
