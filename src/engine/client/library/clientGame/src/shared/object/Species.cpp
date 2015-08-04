//======================================================================
//
// Species.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/Species.h"

#include <map>

//======================================================================

namespace SpeciesNamespace
{
	typedef stdmap<SharedCreatureObjectTemplate::Species, StringId>::fwd        TypeStringIdMap;
	typedef stdmap<SharedCreatureObjectTemplate::Species, std::string>::fwd     TypeStringMap;
	typedef stdmap<SharedCreatureObjectTemplate::Species, Unicode::String>::fwd TypeUnicodeStringMap;

	TypeStringIdMap       s_typeStringIds;
	TypeStringMap         s_typeStrings;
	TypeUnicodeStringMap  s_typeUnicodeStrings;

	bool                  s_installed = false;
	std::string const     s_stringTableName("species");
	std::string const     s_emptyString("unknown");

	void install()
	{
		DEBUG_FATAL(s_installed, ("already installed"));

		s_installed = true;

#define MAKE_NAME(a) s_typeStrings[SharedCreatureObjectTemplate::SP_##a] = Unicode::toLower(#a);

		MAKE_NAME(human);
		MAKE_NAME(rodian);
		MAKE_NAME(trandoshan);
		MAKE_NAME(monCalamari);
		MAKE_NAME(wookiee);
		MAKE_NAME(bothan);
		MAKE_NAME(twilek);
		MAKE_NAME(zabrak);
		MAKE_NAME(abyssin);
		MAKE_NAME(aqualish);
		MAKE_NAME(arcona);
		MAKE_NAME(askajian);
		MAKE_NAME(bith);
		MAKE_NAME(bomarrMonk);
		MAKE_NAME(chadraFan);
		MAKE_NAME(chevin);
		MAKE_NAME(dantari);
		MAKE_NAME(devaronian);
		MAKE_NAME(drall);
		MAKE_NAME(dug);
		MAKE_NAME(duros);
		MAKE_NAME(elomin);
		MAKE_NAME(ewok);
		MAKE_NAME(feeorin);
		MAKE_NAME(frogDog);
		MAKE_NAME(gamorrean);
		MAKE_NAME(gorax);
		MAKE_NAME(gotal);
		MAKE_NAME(gran);
		MAKE_NAME(gungan);
		MAKE_NAME(gupin);
		MAKE_NAME(hutt);
		MAKE_NAME(ishiTib);
		MAKE_NAME(ithorian);
		MAKE_NAME(jawa);
		MAKE_NAME(kiffu);
		MAKE_NAME(kitonak);
		MAKE_NAME(klatooinian);
		MAKE_NAME(kowakianMonkeyLizard);
		MAKE_NAME(kubaz);
		MAKE_NAME(marauder);
		MAKE_NAME(massassiWarrior);
		MAKE_NAME(nikto);
		MAKE_NAME(ortolan);
		MAKE_NAME(palowick);
		MAKE_NAME(phlog);
		MAKE_NAME(quarren);
		MAKE_NAME(selonian);
		MAKE_NAME(shistavanen);
		MAKE_NAME(sullustan);
		MAKE_NAME(talz);
		MAKE_NAME(teek);
		MAKE_NAME(tulgah);
		MAKE_NAME(toydarian);
		MAKE_NAME(tuskenRaider);
		MAKE_NAME(weequay);
		MAKE_NAME(whiffid);
		MAKE_NAME(wistie);
		MAKE_NAME(yuzzum);
		MAKE_NAME(angler);
		MAKE_NAME(bageraset);
		MAKE_NAME(bantha);
		MAKE_NAME(barkMite);
		MAKE_NAME(bazNitch);
		MAKE_NAME(beardedJax);
		MAKE_NAME(blackfish);
		MAKE_NAME(blistmok);
		MAKE_NAME(bluefish);
		MAKE_NAME(blurrg);
		MAKE_NAME(boarWolf);
		MAKE_NAME(bocatt);
		MAKE_NAME(bol);
		MAKE_NAME(bolleBol);
		MAKE_NAME(bolma);
		MAKE_NAME(bordok);
		MAKE_NAME(borgle);
		MAKE_NAME(brackaset);
		MAKE_NAME(capperSpineflap);
		MAKE_NAME(carrionSpat);
		MAKE_NAME(choku);
		MAKE_NAME(chuba);
		MAKE_NAME(coloClawFish);
		MAKE_NAME(condorDragon);
		MAKE_NAME(corellianSandPanther);
		MAKE_NAME(corellianSliceHound);
		MAKE_NAME(crownedRasp);
		MAKE_NAME(crystalSnake);
		MAKE_NAME(cuPa);
		MAKE_NAME(dalyrake);
		MAKE_NAME(dewback);
		MAKE_NAME(duneLizard);
		MAKE_NAME(durni);
		MAKE_NAME(dwarfNuna);
		MAKE_NAME(eopie);
		MAKE_NAME(faa);
		MAKE_NAME(falumpaset);
		MAKE_NAME(fambaa);
		MAKE_NAME(fannedRawl);
		MAKE_NAME(flewt);
		MAKE_NAME(flit);
		MAKE_NAME(fliteRasp);
		MAKE_NAME(fynock);
		MAKE_NAME(gackleBat);
		MAKE_NAME(gapingSpider);
		MAKE_NAME(gekk);
		MAKE_NAME(gnort);
		MAKE_NAME(graul);
		MAKE_NAME(greatGrassPlainsTuskCat);
		MAKE_NAME(gronda);
		MAKE_NAME(gualama);
		MAKE_NAME(gubbur);
		MAKE_NAME(gufDrolg);
		MAKE_NAME(gulginaw);
		MAKE_NAME(gurk);
		MAKE_NAME(gurnaset);
		MAKE_NAME(gurrek);
		MAKE_NAME(hanadak);
		MAKE_NAME(hermitSpider);
		MAKE_NAME(hornedKrevol);
		MAKE_NAME(hornedRasp);
		MAKE_NAME(hufDun);
		MAKE_NAME(huurton);
		MAKE_NAME(ikopi);
		MAKE_NAME(jellyfish);
		MAKE_NAME(kaadu);
		MAKE_NAME(kaitok);
		MAKE_NAME(kima);
		MAKE_NAME(kimogila);
		MAKE_NAME(kittle);
		MAKE_NAME(kliknik);
		MAKE_NAME(krahbu);
		MAKE_NAME(kraytDragon);
		MAKE_NAME(kupernug);
		MAKE_NAME(kusak);
		MAKE_NAME(kwi);
		MAKE_NAME(laa);
		MAKE_NAME(langlatch);
		MAKE_NAME(lanternBird);
		MAKE_NAME(malkloc);
		MAKE_NAME(mamien);
		MAKE_NAME(mawgax);
		MAKE_NAME(merek);
		MAKE_NAME(mott);
		MAKE_NAME(murra);
		MAKE_NAME(mynock);
		MAKE_NAME(narglatch);
		MAKE_NAME(nerf);
		MAKE_NAME(nuna);
		MAKE_NAME(opeeSeaKiller);
		MAKE_NAME(predatorialButterfly);
		MAKE_NAME(pekoPeko);
		MAKE_NAME(perlek);
		MAKE_NAME(pharple);
		MAKE_NAME(piket);
		MAKE_NAME(plumedRasp);
		MAKE_NAME(pufferfish);
		MAKE_NAME(pugoriss);
		MAKE_NAME(purbole);
		MAKE_NAME(quenker);
		MAKE_NAME(qurvel);
		MAKE_NAME(rancor);
		MAKE_NAME(ray);
		MAKE_NAME(remmer);
		MAKE_NAME(reptilianFlyer);
		MAKE_NAME(roba);
		MAKE_NAME(rockMite);
		MAKE_NAME(ronto);
		MAKE_NAME(saltMynock);
		MAKE_NAME(sarlacc);
		MAKE_NAME(scurrier);
		MAKE_NAME(sharnaff);
		MAKE_NAME(shaupaut);
		MAKE_NAME(shearMite);
		MAKE_NAME(skreeg);
		MAKE_NAME(snorbal);
		MAKE_NAME(spinedPuc);
		MAKE_NAME(spinedSnake);
		MAKE_NAME(squall);
		MAKE_NAME(squill);
		MAKE_NAME(stintaril);
		MAKE_NAME(stripedFish);
		MAKE_NAME(swirlProng);
		MAKE_NAME(tancMite);
		MAKE_NAME(taunTaun);
		MAKE_NAME(tesselatedArborealBinjinphant);
		MAKE_NAME(thune);
		MAKE_NAME(torton);
		MAKE_NAME(tybis);
		MAKE_NAME(veermok);
		MAKE_NAME(verne);
		MAKE_NAME(vesp);
		MAKE_NAME(virVur);
		MAKE_NAME(vlutore);
		MAKE_NAME(vogEel);
		MAKE_NAME(voritorLizard);
		MAKE_NAME(vynock);
		MAKE_NAME(whisperBird);
		MAKE_NAME(wingedOrnith);
		MAKE_NAME(wompRat);
		MAKE_NAME(woolamander);
		MAKE_NAME(worrt);
		MAKE_NAME(zuccaBoar);
		MAKE_NAME(assassinDroid);
		MAKE_NAME(astromech);
		MAKE_NAME(bartenderDroid);
		MAKE_NAME(bugDroid);
		MAKE_NAME(darktrooper);
		MAKE_NAME(demolitionmech);
		MAKE_NAME(doorDroid);
		MAKE_NAME(droideka);
		MAKE_NAME(interrogator);
		MAKE_NAME(jediTrainer);
		MAKE_NAME(loadLifter);
		MAKE_NAME(mouseDroid);
		MAKE_NAME(powerDroid);
		MAKE_NAME(probot);
		MAKE_NAME(protocolDroid);
		MAKE_NAME(repairDroid);
		MAKE_NAME(spiderDroid);
		MAKE_NAME(surgicalDroid);
		MAKE_NAME(tattletaleDroid);
		MAKE_NAME(trackerDroid);
		MAKE_NAME(treadwell);
		MAKE_NAME(ev9d9);
		MAKE_NAME(maulProbeDroid);
		MAKE_NAME(atst);
		MAKE_NAME(atat);

#undef MAKE_NAME

		for (TypeStringMap::const_iterator it = s_typeStrings.begin (); it != s_typeStrings.end (); ++it)
		{
			const SharedCreatureObjectTemplate::Species species = (*it).first;
			const std::string & str  = (*it).second;
			s_typeStringIds.insert(std::make_pair(species, StringId(s_stringTableName, Unicode::toLower (str))));
		}
	}
}

using namespace SpeciesNamespace;

//----------------------------------------------------------------------

const StringId & Species::getStringId(SharedCreatureObjectTemplate::Species const species)
{
	if (!s_installed)
	{
		install();
	}

	TypeStringIdMap::const_iterator iterTypeStringIds = s_typeStringIds.find(species);

	if (iterTypeStringIds != s_typeStringIds.end())
	{
		return iterTypeStringIds->second;
	}

	return StringId::cms_invalid;
}

//----------------------------------------------------------------------

const Unicode::String & Species::getLocalizedName(SharedCreatureObjectTemplate::Species const species)
{
	if (!s_installed)
		install();

	const TypeUnicodeStringMap::iterator it = s_typeUnicodeStrings.find (species);
	if (it != s_typeUnicodeStrings.end ())
		return (*it).second;

	const StringId & stringId = getStringId (species);
	if (stringId.isValid ())
		return s_typeUnicodeStrings [species] = stringId.localize ();
	else
		return s_typeUnicodeStrings [species];
}

//======================================================================
