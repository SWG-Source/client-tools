// GameConnection.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "FirstSwgLoadClient.h"
#include "Archive/ByteStream.h"
#include "Client.h"
#include "ConfigSwgLoadClient.h"
#include "GameConnection.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/Timer.h"
#include "sharedMath/Transform.h"
#include "sharedMath/Vector.h"
#include "sharedMath/Vector2d.h"
#include "sharedMathArchive/TransformArchive.h"
#include "sharedMathArchive/VectorArchive.h"
#include "sharedNetworkMessages/ClientLoginMessages.h"
#include "sharedNetworkMessages/ClientCentralMessages.h"
#include "sharedNetworkMessages/ClientPermissionsMessage.h"
#include "sharedNetworkMessages/CommandChannelMessages.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedNetworkMessages/MessageQueueCommandQueueEnqueue.h"
#include "sharedNetworkMessages/MessageQueueDataTransform.h"
#include "sharedNetworkMessages/MessageQueueDataTransformWithParent.h"
#include "sharedNetworkMessages/MessageQueueTeleportAck.h"
#include "sharedNetworkMessages/ObjectChannelMessages.h"
#include "sharedNetworkMessages/ShipUpdateTransformMessage.h"
#include "sharedNetworkMessages/UpdateContainmentMessage.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedRandom/Random.h"
#include "SwgLoadClient.h"
#include "UnicodeUtils.h"

//-----------------------------------------------------------------------

const unsigned long gs_defaultUnreliableSendRateMilliseconds = 250;
const unsigned long gs_maxUnreliableSendRateMilliseconds = 10000;
const float gs_shipTransformUpdateTime = 0.2f;
const float gs_shipTransformReliableUpdateTime = 4.f;

#if 0
// Old load client info with bad customization data, here for future troubleshooting of crash condition
const char *gs_newCharacterTemplate = "object/creature/player/human_male.iff";
const char *gs_newCharacterHairTemplate = "object/tangible/hair/human/hair_human_male_s06.iff";
const char *gs_newCharacterProfession = "combat_engineer";
const char *gs_newCharacterBodyCustomizationString = "@RD";
const char *gs_newCharacterHairCustomizationString = "2#0#1#private#15#1#index_color_1#1#e0#";
#else
// Load clients are purty zabrak female artisans
const char *gs_newCharacterTemplate = "object/creature/player/zabrak_female.iff";
const char *gs_newCharacterHairTemplate = "object/tangible/hair/zabrak/hair_zabrak_female_s18.iff";
const char *gs_newCharacterProfession = "crafting_artisan";
const char *gs_newCharacterBodyCustomizationString =
	"\x01\x27\xAB\x35\xFF\x01\x17\x38\x18\xFF\x01\x1C\x6B\x1B"
	"\xFF\x01\x05\xFF\x01\x1A\xFF\x01\x19\x6B\x0D\xFF\x01\x09"
	"\x3D\x12\xFF\x01\x13\xD1\x20\xFF\x01\x10\xFF\x01\x21\x57"
	"\x0F\xFF\x02\x14\x22\x11\xFF\x01\x0E\x97\x03\xD1\x0B\x80"
	"\x0C\xFF\x01\x06\xFF\x01\x08\xDC\x15\xFF\x02\x16\xFF\x01"
	"\x04\x7B\x07\x61\x0A\xFF\x01\x36\x19\x2C\x49\x25\xFF\x01"
	"\x2D\x01\x35\x03\x24\xFF\x01\x2A\xFF\x01\x01\x06\x22\xFF"
	"\x01\x2E\x15\xFF\x03";
const char *gs_newCharacterHairCustomizationString =
	"\x01\x02\x01\x04\x02\x09\xFF\x03";
#endif

//-----------------------------------------------------------------------


// chat table (taken from quotes file)
const char * chatText[] = {
"You won't read this and character creation won't work - CThurow",
"What, does everyone have a Calan filter on their email? CThurow",
"Multiserver is like an onion.  There are infinite layers, and each one makes you cry.  -JRandall",
"I loved the playstation - ACastoro to Ken Kutaragi",
"I wish I were German so I can have a name like HAAR. -JWatson ",
"How do you think programmers get laid?  -JRandall  F7 - DRubenfield",
"How long is 1/4 of a second?  -CMayer",
"50 is 1/2 of 100.  -CMayer",
"It's the fertile triangle of retail.  -JBrack",
"They stick horses heads in peoples beds as a really bad joke.  -DAlexander",
"Please refrain form using scary type to illustrate how important it is for us to get back to the way it was before you decided to change it.  -DBorth",
"Difficulties in selecting/deselecting weapons causes nudity.  -QWestcott",
"Woohoo, you made it a jpeg this time, I don't have to get Pfister to print it for me. You're my new friend, and I love you.  -MHigby",
"I'll pretty much call Joe right away and tell him not to make any happy frogs.  -JDonham",
"As far as the AI goes, if a Rancor decides to attack you, it should just attack, it shouldn't bother threatening you.  -JFreeman  Yeah, who's it trying to impress?  -TBailey",
"If you're playing the Starship Troopers Real Time Strategy game, and you're playing the bugs, you only have two options: \"Rush\" and \"Not Rush\".  And \"Not Rush\" is grayed out.  -CBarnes",
"You know, we crashed a lot less when we were naked.  -TBailey",
"There's nothing sadder, or scarier, than a shaved Wookie.  -CBarnes",
"Man, I'm glad I'm not married to Dan.  -RThornton",
"I got a segment fault core dump.  It that good news or bad news?  -CBarnes",
"AI will solve all our problems.  -CBarnes",
"That's a... Uh, slight, major data change.  -DRubenfield",
"My man clock must be ticking today.  -DAlexander",
"Customer service is required if you are going to have customers.  -CMayer",
"He's not the brightest bulb on the tree, but he ain't dumb.  -AFranklin",
"I think you’re incompetent enough to handle it.  -RVogel",
"Squeaky wheel gets the shaft. -JRandall  If you've got enough grease, the shaft is alright. -JWatson",
"It's always good to get paid the day before you have to spend a week at E3. -ASommers",
"hehe... I already did a cat. -JRandall",
"I’d do it if you pay for my Lasic.  -Asommers  If you ship in December I’ll pay for it myself.  -JSmedley",
"I just have a problem with commitment.  -CMayer",
"I’ve got bones like pretzels!  -DRubenfield  Mmmm...Delicious pretzels... -TBailey",
"Nothing says love like constipation.  -JWatson",
"Someone needs to get fired over this.  -ASommers",
"The client has no memory leaks!  -ASommers",
"We can’t support 30 creatures, because the server can’t handle it.  -ASommers",
"Well, my ass is clean now but my breath sure stinks! -JWatson",
"Perfection is the enemy of good.  -RKoster",
"I’m going to be the Dick--tator.  -RVogel",
"If you think we’ve over-promised, just wait until we under-deliver!  -JRodgers",
"<Fail!  Fail!  Fail!  Fail!  Fail!> -Screen display  That doesn’t mean it failed.  -NNewhard",
"We have more warnings than assets!  -JRodgers",
"Open mouths lead to closed ears.  -CMayer",
"Anything that can’t be written down with a spreadsheet is too complicated.  -CMayer",
"Dibs on the Beefaroni!!!  - JRoy",
"Well, if you want my opinion, it's just outright fucked - JWatson",
"Are we doing the build by remote control?  -RKoster",
"I just need to touch up his nuts a bit.  -JWatson",
"I suggest the people who think they can play ping pong, but can't go buy some balls.  -DBorth",
"The Jizz organ.  Where is the Jizz organ?  No one can find a picture of the Jizz organ!  -JBrack",
"Nobody built the Jizz.  -JShoopack",
"I won't have time to write wrapper functions for the hack.  -NNewhard",
"So, we need to talk about security.  -RKoster  We have Dan Borth.  -DRubenfield",
"Jeff Grills told me to tell you to quit checking in bugs.  -JDonham",
"How well do burgers travel?  -JBrack  Burgers travel well, fries don't.  <a beat>  Can I have one of those fries?  -JWatson",
"Everquest is not as cute as Britney Spears.  -RKoster",
"Oh, I'm not obsessed.  Obsessed is when you're Jeff Grills.  -CMayer",
"You slobs need to get your own dumbell! -ASommers",
"I'm a programmer so I can't estimate time accurately.  -JGrills",
"I am your father.  -Rvogel  You are not!  -JDonham",
"So the create npc script is just a script that creates npc's?  -TBailey",
"Hey! I just noticed something! If you remove John and Jake's first names, you get Roy Rodgers!  -CMayer",
"I've got this process down.  <a beat> Whoops.  -WKier",
"What's wrong with this picture?  -ASommers  Well, other than the bumpmapped man-ass, it looks fine.  -DRubenfield  <squints at picture>  That ass isn’t bumpmapped.  -JWatson",
"I hate Star Wars.  -HBlackman",
"I've found an interesting problem with collision and shaders...  -TFiala",
"Emacs is the antique.  -JWatson",
"To the extent the law allows, I explicitly DO NOT grant Sony permission to share any information about me with Microsoft without telling me first. - CThurow",
"What color is red?  -DWaldrip",
"I'm not gay!!!  -JRodgers",
"Tell you what, if you ship in September I’ll buy you a pony out of my own pocket.  -JSmedley",
"I can have needs, that's not illegal.  -JRodgers. ",
"Time to make Java my bitch.  - CMayer",
"I'm just going to start coming to work naked.  - CMayer",
"Sometimes we just have to be sad for a while. - CMayer",
"That's the kind of thing you show to someone like Gordon Walton, and they get a woodie over it.  -CThurow",
"How are we going to do animating lights?  -JDonham  I'm not.  -CBarnes",
"Unless we let the instruments move in and out of their face.  -JGrills  Yeah, I like that.  -CMayer",
"I think that just about covers it. Game art = Done.  -DBorth",
"No, it's not fun for the players, it's fun for us.  Screw the players.  -DRubenfield",
"I can't figure out how to force jump.  -TBaily  Let go with your feelings.  -JFreeman  I don't think that's going to work.  -TBailey  That is why you fail.  -JFreeman",
"Each person, like, has their own thing, and like, that's what they should do.  -JRodgers",
"Yeah, once I was at a bar in Bastrop and got punched by a redneck for no reason!  -DWaldrip   Did you have your daisy dukes on? -ACollins",
"I popped something off my stack and ended up off in la-la land.  -JGrills",
"What’s up with the girlfriend-like remark about the mouse?  -JRodgers",
"Perforce just fucked me.  -JBrack",
"That guy can be such a punk sometimes.  -CThurow",
"Jeff Grills, I love you.  -JDonham",
"Well, I'm *for* design.  -RVogel",
"There are no stupid questions, only stupid people.  Jake is stupid.  -JDonham",
"I can have the game done by Tuesday.  -TBailey  Well, if you can do that, you get a promotion.  -JDonham",
"I think this is fixed.  -CMayer  I’m fundamentally opposed to saying “I think this is fixed.” -JGrills  This is definitely fixed. -CMayer",
"Scott's going to get his little pee-pee whipped.  -RVogel",
"What kind of pizza do you want for your personal pleasure?  -MBogue",
"I'm like a chocoholic, but with alcohol.  -JWatson",
"I don't have a problem.  -RVogel",
"I buy my nipples in packs of 22.  -JWatson",
"My pickle is wet. -JDonham  Just be glad you have a pickle.  -AFranklin",
"When I get hungry, put meat in me.  -JWatson",
"Keeping me full of caffeine will ensure we ship on time.  -DRubenfield",
"When Rebo is scaled in the game could we scale his organ too? Or should I keep his organ small?  -DAlexander",
"It's the most honest fake we can do right now. -JRoy",
"You can't trust these programmers.  -JSiprelle",
"Get naked, let's see! - RVogel",
"My shadow shit is awesome! -ASommers",
"No, I will not bend over or get under your desk. -RPalacio",
"I'm not an idiot.  -JSmedley",
"Justin doesn't know how it works.  -JWatson  But he's implementing it!  -RKoster  Guess he'll know soon, then.  -JWatson",
"Can you get that done?  -JDonham  Not at my current salary.  -JWatson",
"You want me to do that too?  Holy fuck.  -TFiala",
"Is that done, or Arnie done?  -JRodgers",
"You talked to him about this?  -JRodgers   He doesn't know anything about *this*.  -JGrills",
"He saw Kelly fall to the floor and heard \"You are in command now, Admiral Smedley.\"  -JRodgers",
"You have to do it.  Kinda like going to Pottery Barn with your girlfriend.  -DBorth",
"If you see someone in the Bloodguard, just kill him and take his pants.  Then *you* are in the Bloodguard!  -BDaly",
"Artists aren't known for their great source control comments.  -JDobson",
"You find a chunk of special meat! -JFreeman (via a quest script)",
"300 Items, at $800/item = job I want.  -JDonham",
"Security through obfuscation.  -ACastoro",
"We need to have a metal/Otoh Gunga meeting.  -ASommers",
"Coffee is my bitter liquid god.  -JWatson",
"She's pretty good looking, for a girl.  -JJonas",
"Where else would you like me to touch you? - RVogel",
"I like my operating system to treat me like a bitch.  -JWatson",
"Great, now I have an unconscious rabbit threatening me.  -CMayer",
"What, was Friday no pants day?  -TBailey  Well, that's what Vogel told me.  -DRubenfield",
"The more you fuck up, the more we slip.  -JDonham",
"How can you not like sliced bread? - JShoopack",
"Sanity checking is scheduled, but it's low on my priority list. - SJakab",
"Every impossible system that is going to be developed is already completed . . . those aren’t impossible, just highly improbable. - Jgrills",
"It's strangely appropriate that that task is number 666.  -CThurow",
"You know what I see for swimming underwater for first playable is the ability to swim underwater.  -RVogel",
"They have a great sun, but they don't have Jesus Rays. -RVogel.  That's because Jesus is working on our project. -JRodgers",
"We will be the Sith Lords of the network.  -JRandall",
"Are you looking at my ass?  -JDonham   Are you trying to get me to look at your ass?  -JRodgers",
"I need to lube it, I have some stuff, I'll bring it in.  -RVogel",
"You got that right!  -ASommers",
"Shut up bitches, I'm done!  -CMayer",
"I fixed the last bug in it last night.  -JGrills",
"I've never seen the server stay up this long. -CMayer",
"You need to delete your Wedges, Andy. -ASommers",
"All of the world's problems will be fixed.  -ASommers",
"There's the first 90%, and then the second 90%.  -JGrills",
"Data hasn't changed from last week with the frame rate was at 30.  -RKoster",
"No, Joe is not John Roy.  -JDonham",
"That would be 40 hours on a bus!  That's like an entire work week!  -DBorth",
"I like my women at 72. -ASommers",
"I will set up the computer, that's what John and I get paid to do.  -RVogel",
"My secret santa gave me a lump of coal!  -RVogel",
"Boba Fat is looking unhappy, and it's most certainly because he can not eat with his Boba helmet on.  -WKier",
"Jeff, every time you fondle our base objects, our compilers go crazy!  -JRandall",
"Nice jacket!... Are you going sailing today?  -ACollins",
"Well... what if you went to Pluto and there where \"Space Rabbits\" there?  -ACollins",
"Really it's all like a big chunk of milk. -DWaldrip ",
"I love you, John Donham.  -KO'Hara",
"Put on the leather pants, J.  -HBlackman",
"Please for the love of god do not engage in combat on the build cluster.  It makes Jar Jar cry.  -CMayer",
"Okay, just ignore what you see here.  -ASommers",
"I enjoy being like a God.  No, really.  -MBouge",
"<On the phone with his wife> Haden is coming into town.  <silence>  I did so tell you he was coming!!!  -RVogel",
"There is something strangely erotic about naked aliens.  -JRoy",
"It's going to be a bad day when you can't figure out how to make a program dump core.  -CMayer",
"A present is best presented wrapped in love and tied with smile. -AJorgensen",
"Luckily we’re safe in this well secured, non-leaky, no-ant filled building!  -DRubenfield",
"I just fixed the last template system bug.  -SJakab",
"You guys will never finish this milestone with me on Valium.  -CBarnes",
"I just finished playing with myself.  -RVogel",
"I want you to make a building that is generic, yet distinctive.  -CBarnes",
"Matt, you've never touched me like that before!  -RThornton",
"Let's all get naked!  -RVogel",
"I fucking hate this project.  -ACastoro",
"Not to say I don't have a sense of humor, but I want to make sure that there really AREN'T sounds needed for the \"Klaatu\", \"Barada\", and \"Nikto\".  This IS a joke and not humorously named alien species, correct?  -WWallace",
"Brought to you by the Department of Redundancy Department.  -JWatson",
"I don't know that there is a good solution.  -JGrills",
"The terrain system will never talk to Maya or Perforce!  -ASommers",
"Where the fuck is the P.P.?  -JWatson",
"That's going to be the most fun game in which you can do whatever it is you do in that game.  -JFreeman",
"I'll do it...that way it will be a total cluster fuck.  -RVogel",
"Lady & Gentlemen, we have a winner!  -AFranklin",
"If you do your job right, and I'm blown away in December, you'll get your strippers.  -RVogel",
"Yes, but how many times during UO did I say that UO has a great object template system and we should use it for Star Wars Galaxies?  -DRubenfield",
"Damnit, I want to wear a dress!  -JWatson",
"I'm having a Windows moment.  -JWatson",
"Why did I hit abort when I meant retry? And why do they call it retry instead of debug? This is one of life's great mysteries.  -JWatson",
"I didn't know we were on a schedule.  -HBlackamn",
"It makes me hot just looking at you.  -RVogel",
"The math doesn't work.  -JGrills",
"You're the master, I'm the apprentice.  I'm just learning.  -ACastoro",
"That's a great car for hauling prostitutes, 'cause you can get more in there.  -ACastoro",
"Remind me to never have just one big tooth in the middle of my face.  -DWaldrip",
"Did Raph want them to blow glue?  -JGrills",
"Pookie is ready to be hammered by stuff.  -JBrack",
"Are banthas going to have 3 foot long tounges?  -SJakab  God I hope so.  -JWatson",
"Rich is powerful.  -TBailey",
"If they are bitching at each other, then they are paying us $15 a month.  -ACastoro",
"You found my home page!  -HBlackman",
"Somewhere in there.  -ASommers",
"It's not bad . . . it's just that editing particles with text pad slows us down a bit... -JRoy",
"My schedule will grow on its own - we don't need to help it. -JGrills",
"If they ship next year I'll eat my shoes.  -JDonham",
"Yeah, I found it in the street - of course I washed it before I put it on.  - AStapp",
"If somebody is wearing a purple box that you don't like, you can kill them with your purple box.  -AStapp",
"Lobsters are lettuce.  -JDonham",
"I used to be an artist before I flipped fractals around.  -JRoy",
"Yeah!  Server Rules!  -CMayer",
"My LOD system will take care of everything.  -TFiala",
"Evil ewoks are like teddy bears stuffed with garbage.  -WKier",
"I'll have a veggie burger with bacon, please.  -NNewhard",
"I'd like to talk about the animation crisis.  -BDaly",
"Does anyone get siezures anymore? -ASommers",
"I love Kevin O'Hara.  I love Kevin O'Hara. I love Kevin O'Hara. -JBrack",
"We don't have time to feel sorry for ourselves for being stupid.  -DWaldrip",
"You make my ass twitch. -NNewhard",
"Are you here to invade? Well, no, but I can give you an anal probe... -RKoster",
"You ruined Christmas!  -DRubenfield",
"They are looking for chairs because something is wrong with their asses! - AFranklin",
"You found a room of tasks!  -CBarnes",
"That's a trip to Vegas and 2 whores!  -CMayer",
"We need 5 gallons...Well, can you check with your supervisior?  -ASommers",
"Why is everone bouncing balls off my head?  -DRubenfield",
"It copied a pointer, which copied the class, which blah, blah, blah, blah, boom.  -RKoster",
"That is such a Raph task.  -DRubenfield",
"Jabba was a Hutt, not stupid.  -NNewhard",
"I'm about to make an ugly, ugly, ugly hack.  -CThurow",
"Red, Green, and Blue.  Too many colors.  -DRubenfield",
"It takes playing with yourself to a new high.  -RVogel",
"You made Ryan's nose throb! -DRubenfield",
"We all liked Bill's balls before you broke them by paddling them too hard!  -JRandall",
"No that is incorrect - most balls have been destroyed by players who have no business playing with those balls.  -DBorth",
"Just because it's right, doesn't mean it's perfect.  -JGrills",
"I really have no problem with core system bitch work.  -JRandall",
"Women are better at PvP. -RVogel",
"You are all fucking crazy. - JDonham",
"I'm doing something.  - RVogel",
"I have a gun, and face it, sometimes you need to kill people.  -JDonham",
"The only place they don't suck is urology!  -RKoster",
"It's not the size of the Visio doc, it's how you use it.  -ACastoro",
"I'd kill Rodia before the Millennium Falcon.  -CBarnes",
"This is the worst kind of anal.  -CBarnes",
"When we get 1 million combined subscribers, we are taking a trip to the Hilton in Atlanta, and spending more money than Richard G. -BMcQuaid",
"I just want an excuse to grab you.  -RVogel",
"It wouldn't be a moon walk, it would be an Endor moon walk.  -ACastoro",
"Now that's a quote! -ASommers",
"The willies are tangible.  -ASommers",
"I think that I, more than anyone else, know how to ship a game.  -ASommers",
"I'll hack that no problem! -JWatson",
"I really hate C++.  It fucking sucks! -JGrills",
"There is a difference between men and women. -RVogel",
"We can kill you, that's not a problem.  -ASommers",
"Mmmm....  maraboobies.msh... -CBarnes",
"We're cutting Dan! -NNewhard",
"I'd notice if my crotch was missing, I don't know about you. -ACastoro",
"That totally breaks our traffic ticket skill. -CBarnes",
"Use the fork! -CMayer",
"You're not an idiot. -JSmedley",
"Don't ever give a designer your wallet! -DRubenfield",
"I have peanuts. -CMayer",
"All I need to do is put it in.  -ASommers",
"It's dangerous to be a telephone pole in this city. -JGrills",
"Get me drunk, take me to a good band -- that's all I need. -CMayer",
"I'm deaf as a motherfucker. -CMayer",
"All it takes is 1/2 an inch. -RVogel",
"I'm going to turn my stripper on! -ASommers",
"This is the biggest hardware hack I've ever had to deal with!  Well, maybe not...  -JGrills",
"I'm really ready for a benevolent despot to take over. -HBlackman",
"I think I've got a really good hack that will solve that. -JGrills",
"I don't want to be a programmer anymore. -ASommers",
"Making games is hard. -Everyone",
"SOACD = Shit only Andy can do. -JBrack/JRodgers",
"That's the problem with these modelers... -AJorgensen",
"Ryan always has girl problems. -JRandall",
"RVDC = Rich Vogel Damage Control -DBorth",
"Producer = Shit Shield -JRandall",
"Death is asynchronous. -JRandall",
"I guess it's not going to be today...  -ASommers",
0};


//-----------------------------------------------------------------------

GameConnection::GameConnection(Client * o, const std::string & a, const unsigned short p, const NetworkId &characterId, const NetworkSetupData &setupData) :
	LoadConnection(a, p, setupData),
	m_characterObjectId(characterId),
	m_characterContainerId(NetworkId::cms_invalid),
	m_characterInShip(false),
	m_owner(o),
	m_transform(),
	m_readyToSimulate(false),
	m_sequenceNumber(0),
	m_updateTransformTimer(2.0f),
	m_velocity(),
	m_chatEventTimer(Random::randomReal(ConfigSwgLoadClient::getChatEventTimerMin(), ConfigSwgLoadClient::getChatEventTimerMax())),
	m_chatTextCount(0),
	m_socialEventTimer(Random::randomReal(ConfigSwgLoadClient::getSocialEventTimerMin(), ConfigSwgLoadClient::getSocialEventTimerMax())),
	m_timeOfLastUnreliableSendMilliseconds(0),
	m_timeOfLastReceiveMilliseconds(0),
	m_unreliableSendRateMilliseconds(gs_defaultUnreliableSendRateMilliseconds),
	m_sentThisFrame(false),
	m_receiveThisFrame(false),
	m_shipTransformUpdateTimer(gs_shipTransformUpdateTime),
	m_shipTransformReliableUpdateTimer(gs_shipTransformReliableUpdateTime)
{
	m_velocity = Vector(Random::randomReal(-1.0f, 1.0f), 0, Random::randomReal(-1.0f, 1.0f));
	m_velocity.normalize();
	m_velocity = m_velocity * (Random::randomReal(0.5f, 5.0f));

	for(int i = 0; chatText[i] != 0; ++i)
	{
		m_chatTextCount++;
	}
}

//-----------------------------------------------------------------------

GameConnection::~GameConnection()
{
	REPORT_LOG(true, ("[%s] Game connection closed\n", m_owner->getLoginId().c_str()));
	m_owner->onConnectionClosed(this);
}

//-----------------------------------------------------------------------

void GameConnection::onConnectionClosed()
{
}

//-----------------------------------------------------------------------

void GameConnection::chat(char const *text)
{
	++m_sequenceNumber;
	static unsigned long commandHash = Crc::normalizeAndCalculate("spatialChatInternal");

	Unicode::String params(Unicode::narrowToWide("0 0 0 0 0 "));
	params += Unicode::narrowToWide(text);

	MessageQueueCommandQueueEnqueue msg(m_sequenceNumber, commandHash, NetworkId::cms_invalid, params);
	ObjControllerMessage message(m_characterObjectId, CM_commandQueueEnqueue, 0.0f, GameControllerMessageFlags::SEND | GameControllerMessageFlags::DEST_SERVER, &msg);
	send(message, true);

	REPORT_LOG(true, ("[%s] Chats : \"%s\"\n", m_owner->getLoginId().c_str(), text));
}

//-----------------------------------------------------------------------

static int socialTypes[] = {328, 78};

void GameConnection::social()
{
	++m_sequenceNumber;
	int socialEntry = socialTypes[Random::random(0, sizeof(socialTypes) / sizeof(int) - 1)];
	
	static unsigned long commandHash = Crc::normalizeAndCalculate("socialInternal");
	static NetworkId targetId;
	char buf[256] = {"\0"};
	snprintf(buf, sizeof (buf), "0 %d ", socialEntry);
	Unicode::String params(Unicode::narrowToWide(buf));

	MessageQueueCommandQueueEnqueue msg(m_sequenceNumber, commandHash, targetId, params);
	ObjControllerMessage message(m_characterObjectId, CM_commandQueueEnqueue, 0.0f, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE |GameControllerMessageFlags::DEST_AUTH_SERVER, &msg);
	send(message, true);

	REPORT_LOG(true, ("[%s] plays a social\n", m_owner->getLoginId().c_str()));
}

//-----------------------------------------------------------------------

const Vector & GameConnection::getVelocity() const
{
	return m_velocity;
}

//-----------------------------------------------------------------------

const bool GameConnection::getReadyToSimulate() const
{
	return m_readyToSimulate;
}

//-----------------------------------------------------------------------

void GameConnection::onConnectionOpened()
{
	const LoginClientToken * token = m_owner->getLoginClientToken();

	ClientIdMsg l(token->getToken(), token->getTokenSize(), 0);
	send(l, true);
}

//-----------------------------------------------------------------------

void GameConnection::onReceive(const Archive::ByteStream & data)
{
	// adjust send rate if necessary
	if(! m_receiveThisFrame)
	{
		unsigned long timeMs = Clock::timeMs();
		if(timeMs < m_timeOfLastReceiveMilliseconds + m_unreliableSendRateMilliseconds)
		{
			if(m_unreliableSendRateMilliseconds / 2 >= gs_defaultUnreliableSendRateMilliseconds)
			{
				m_unreliableSendRateMilliseconds = m_unreliableSendRateMilliseconds / 2;
			}
			else
			{
				m_unreliableSendRateMilliseconds = gs_defaultUnreliableSendRateMilliseconds;
			}	
		}
		m_receiveThisFrame = true;
		m_timeOfLastReceiveMilliseconds = timeMs;
	}

	static Archive::ReadIterator ri;
	ri = data.begin();
	GameNetworkMessage msg(ri);
	
	ri = data.begin();

	if(msg.isType("ClientPermissionsMessage"))
	{
		REPORT_LOG(true, ("[%s] received ClientPermissionMessage : ", m_owner->getLoginId().c_str()));
		ClientPermissionsMessage cpm(ri);
		if(! cpm.getCanLogin())
		{
			REPORT_LOG(true, ("CAN'T LOG IN! ABORTING!\n"));
			SwgLoadClient::quit();
		}
		else
		{
			REPORT_LOG(true, ("can login "));
			if(cpm.getCanCreateRegularCharacter())
			{
				REPORT_LOG(true, ("and can create character "));
				if(m_characterObjectId == NetworkId::cms_invalid)
				{
					REPORT_LOG(true, ("and need to create character. Sending ClientCreateCharacter message\n"));
					ClientCreateCharacter c(
						Unicode::narrowToWide(m_owner->getLoginId()),
						gs_newCharacterTemplate,
						1.0f,
						ConfigSwgLoadClient::getCreationLocation(),
						gs_newCharacterBodyCustomizationString,
						gs_newCharacterHairTemplate,
						gs_newCharacterHairCustomizationString,
						gs_newCharacterProfession,
						false,
						Unicode::String(),
						false, 
						"smuggler_2a", 
						"combat_brawler_2handmelee_01");
					send(c, true);
				}
				else
				{
					REPORT_LOG(true, ("and I will send a SelectCharacter message\n"));
					SelectCharacter s(m_characterObjectId);
					send(s, true);
				}
			}
			else
			{
				REPORT_LOG(true, ("and I will send a SelectCharacter message\n"));
				SelectCharacter s(m_characterObjectId);
				send(s, true);
			}
		}
	}
	else if(msg.isType("ClientCreateCharacterSuccess"))
	{
		REPORT_LOG(true, ("[%s] received ClientCreateCharacterSuccess message\n", m_owner->getLoginId().c_str()));
		ClientCreateCharacterSuccess cccs(ri);
		m_characterObjectId = cccs.getNetworkId();
		SelectCharacter s(m_characterObjectId);
		send(s, true);
	}
	else if(msg.isType("CmdStartScene"))
	{
		REPORT_LOG(true, ("[%s] received CmdStartScene message : ", m_owner->getLoginId().c_str()));
		CmdStartScene start(ri);
		if(! ConfigSwgLoadClient::getResetStart())
		{
			REPORT_LOG(true, ("and starting at the given coordinates of %f, %f, %f\n", start.getStartPosition().x, start.getStartPosition().y, start.getStartPosition().z));
			m_transform.setPosition_p(start.getStartPosition());
		}
		else
		{
			REPORT_LOG(true, ("and warping to new coordinates of %f, %f\n", ConfigSwgLoadClient::getStartX(), ConfigSwgLoadClient::getStartZ()));
			Vector start(ConfigSwgLoadClient::getStartX(), 0.0f, ConfigSwgLoadClient::getStartZ());
			m_transform.setPosition_p(start);
		}
		REPORT_LOG(true, ("[%s] My character object ID is %s\n", m_owner->getLoginId().c_str(), m_characterObjectId.getValueString().c_str()));
		DEBUG_FATAL(m_characterObjectId != start.getObjectId(), ("Got a CmdStartScene for an object we didn't log in with?"));
		m_characterContainerId = NetworkId::cms_invalid;
		m_characterInShip = false;
		m_readyToSimulate = true;
		CmdSceneReady s;
		send(s, true);
		chat(ConfigSwgLoadClient::getScriptSetupText());
	}
	else if(msg.isType("UpdateContainmentMessage"))
	{
		UpdateContainmentMessage o(ri);
		if (o.getNetworkId() == m_characterObjectId)
		{
			m_characterContainerId = o.getContainerId();
			m_characterInShip = o.getSlotArrangement() != -1;
		}
	}
	else if(msg.isType("ObjControllerMessage"))
	{
		ObjControllerMessage c(ri);
		if (   c.getNetworkId() == m_characterObjectId
		    || (   m_characterInShip
				    && c.getNetworkId() == m_characterContainerId
						&& c.getMessage() == CM_netUpdateTransform))
		{
			// acknowledge teleports
			if (c.getMessage() == CM_netUpdateTransform)
			{
				MessageQueueDataTransform const *messageData = dynamic_cast<MessageQueueDataTransform const *>(c.getData());
				if (messageData)
				{
					if (m_characterInShip && c.getNetworkId() == m_characterContainerId)
						m_transform = messageData->getTransform(); // the ship is teleporting
					MessageQueueTeleportAck ackData(messageData->getSequenceNumber());
					ObjControllerMessage const msg(
						c.getNetworkId(),
						CM_teleportAck,
						0.0f,
						GameControllerMessageFlags::SEND | 
						GameControllerMessageFlags::RELIABLE |
						GameControllerMessageFlags::DEST_AUTH_SERVER,
						&ackData);
					send(msg, true);
				}
			}			
			if (c.getMessage() == CM_netUpdateTransformWithParent)
			{
				MessageQueueDataTransformWithParent const *messageData = dynamic_cast<MessageQueueDataTransformWithParent const *>(c.getData());
				if (messageData)
				{
					MessageQueueTeleportAck ackData(messageData->getSequenceNumber());
					ObjControllerMessage const msg(
						m_characterObjectId,
						CM_teleportAck,
						0.0f,
						GameControllerMessageFlags::SEND | 
						GameControllerMessageFlags::RELIABLE |
						GameControllerMessageFlags::DEST_AUTH_SERVER,
						&ackData);
					send(msg, true);
				}
			}
		}
	}
}

//-----------------------------------------------------------------------

const bool GameConnection::tryToSendUnreliable()
{
	bool result = false;
	unsigned long timeMs = Clock::timeMs();
	if(m_sentThisFrame || timeMs > m_timeOfLastUnreliableSendMilliseconds + m_unreliableSendRateMilliseconds)
	{
		result = true;
		m_sentThisFrame = true;
		m_timeOfLastUnreliableSendMilliseconds = timeMs;
	}
	else
	{
		result = false;
	}
	return result;
}

// ----------------------------------------------------------------------

void GameConnection::update()
{
	float frameTime = Clock::frameTime();

	if (m_characterInShip)
	{
		// If we would reach our goal position this frame, pick a new goal.
		// The new goal is a random point around the origin, either in a sphere
		// or cube depending on configuration.

		float const shipSpeed = ConfigSwgLoadClient::getShipSpeed();
		float const shipYawRate = 20.f*PI_OVER_180;
		float const shipPitchRate = 20.f*PI_OVER_180;

		if (m_shipGoalPosition.magnitudeBetweenSquared(m_transform.getPosition_p()) <= sqr(shipSpeed*frameTime))
		{
			if (ConfigSwgLoadClient::getShipLoiterInCube())
				m_shipGoalPosition = Vector::randomCube(ConfigSwgLoadClient::getShipLoiterRadius());
			else
				m_shipGoalPosition = Vector::randomUnit() * Random::randomReal(0.f, ConfigSwgLoadClient::getShipLoiterRadius());
			m_shipGoalPosition.x += ConfigSwgLoadClient::getShipLoiterCenterX();
			m_shipGoalPosition.y += ConfigSwgLoadClient::getShipLoiterCenterY();
			m_shipGoalPosition.z += ConfigSwgLoadClient::getShipLoiterCenterZ();
		}

		Vector const direction_o = m_transform.rotateTranslate_p2l(m_shipGoalPosition);

		float const desiredYaw = direction_o.theta();
		float const maxYaw = shipYawRate * frameTime;
		float const actualYaw = clamp(-maxYaw, desiredYaw, maxYaw);
		m_transform.yaw_l(actualYaw);

		float const desiredPitch = direction_o.phi();
		float const maxPitch = shipPitchRate * frameTime;
		float const actualPitch = clamp(-maxPitch, desiredPitch, maxPitch);
		m_transform.pitch_l(actualPitch);
		
		m_transform.move_l(Vector::unitZ * shipSpeed * frameTime);			
		m_velocity = m_transform.getLocalFrameK_p() * shipSpeed;

		if (m_shipTransformUpdateTimer.updateZero(frameTime))
		{
			bool const reliable = m_shipTransformReliableUpdateTimer.updateZero(frameTime);
			ShipUpdateTransformMessage const msg(
				0,
				m_transform,
				m_velocity,
				0.f,
				0.f,
				0.f,
				getServerSyncStampLong());
			send(msg, reliable);
		}
	}
	else
	{
		Vector v = m_velocity;
		v.normalize();
		Vector face = m_transform.rotate_p2l(v);		
		m_transform.yaw_l(face.theta());
		m_transform.move_l(m_velocity * frameTime);
		if(tryToSendUnreliable())
		{
			if(m_updateTransformTimer.updateZero(frameTime))
			{
				m_velocity = Vector(Random::randomReal(-1.0f, 1.0f), 0, Random::randomReal(-1.0f, 1.0f));
				m_velocity.normalize();
				m_velocity = m_velocity * (Random::randomReal(0.5f, 0.5f));
				MessageQueueDataTransform data(0, ++m_sequenceNumber, m_transform, 0.0f, 0.0f, false);
				ObjControllerMessage message(m_characterObjectId, CM_netUpdateTransform, 0.0f, GameControllerMessageFlags::SEND | GameControllerMessageFlags::DEST_SERVER, &data);
				send(message, true);
			}
		}
	}
	
	if(m_chatEventTimer.updateZero(frameTime))
	{
		chat(chatText[Random::random(m_chatTextCount - 1)]);
		m_chatEventTimer.setExpireTime(Random::randomReal(ConfigSwgLoadClient::getChatEventTimerMin(), ConfigSwgLoadClient::getChatEventTimerMax()));
	}
	
	if(m_socialEventTimer.updateZero(frameTime))
	{
		social();
		m_socialEventTimer.setExpireTime(Random::randomReal(ConfigSwgLoadClient::getSocialEventTimerMin(), ConfigSwgLoadClient::getSocialEventTimerMax()));
	}

	if(m_sentThisFrame)
	{
		// adjust rate
		unsigned long timeMs = Clock::timeMs();
		if (timeMs > m_timeOfLastReceiveMilliseconds + m_unreliableSendRateMilliseconds && m_unreliableSendRateMilliseconds < gs_maxUnreliableSendRateMilliseconds)
		{
			m_unreliableSendRateMilliseconds = m_unreliableSendRateMilliseconds * 2;
		}
		m_sentThisFrame = false;
	}
	m_receiveThisFrame = false;
}

//-----------------------------------------------------------------------

