package script.terminal;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.library.ai_lib;
import script.library.armor;
import script.library.beast_lib;
import script.library.buff;
import script.library.callable;
import script.library.chat;
import script.library.consumable;
import script.library.craftinglib;
import script.library.create;
import script.library.expertise;
import script.library.factions;
import script.library.gm;
import script.library.healing;
import script.library.incubator;
import script.library.instance;
import script.library.jedi;
import script.library.money;
import script.library.pet_lib;
import script.library.player_stomach;
import script.library.prose;
import script.library.resource;
import script.library.respec;
import script.library.skill;
import script.library.skill_template;
import script.library.space_crafting;
import script.library.space_flags;
import script.library.space_skill;
import script.library.space_transition;
import script.library.space_utils;
import script.library.static_item;
import script.library.stealth;
import script.library.sui;
import script.library.utils;
import script.library.weapons;


public class terminal_character_builder extends script.base_script
{
	public terminal_character_builder()
	{
	}
	public static final int CASH_AMOUNT = 100000000;
	public static final int AMT = 1000000;
	public static final int FACTION_AMT = 250000;
	
	public static final float WEAPON_SPEED = 1.0f;
	public static final float WEAPON_DAMAGE = 1.0f;
	public static final float WEAPON_ELEMENTAL = 1.0f;
	public static final float WEAPON_EFFECIENCY = 1.0f;
	
	public static final float CONDITION = 1.0f;
	public static final float GENERAL_PROTECTION = 0.94f;
	public static final String SKILL_TBL = "datatables/skill/skills.iff";
	public static final String HEROIC_JEWELRY_SETS = "datatables/skill/character_builder_heroic_jewelry.iff";
	public static final String SKILL_LOADOUT_TBL = "datatables/skill/loadout.iff";
	public static final String EXOTIC_SKILL_MODS = "datatables/crafting/reverse_engineering_mods.iff";
	
	public static final String SHIP_CHASSIS_TBL = "datatables/ship/components/character_builder/frog_chassis.iff";
	public static final String SHIPCOMPONENT_ARMOR_TBL = "datatables/ship/components/character_builder/frog_armor.iff";
	public static final String SHIPCOMPONENT_BOOSTER_TBL = "datatables/ship/components/character_builder/frog_booster.iff";
	public static final String SHIPCOMPONENT_DROIDINTERFACE_TBL = "datatables/ship/components/character_builder/frog_droid_interface.iff";
	public static final String SHIPCOMPONENT_ENGINE_TBL = "datatables/ship/components/character_builder/frog_engine.iff";
	public static final String SHIPCOMPONENT_REACTOR_TBL = "datatables/ship/components/character_builder/frog_reactor.iff";
	public static final String SHIPCOMPONENT_SHIELD_TBL = "datatables/ship/components/character_builder/frog_shield.iff";
	public static final String SHIPCOMPONENT_WEAPON_TBL = "datatables/ship/components/character_builder/frog_weapon.iff";
	public static final String SHIPCOMPONENT_CAPACITOR_TBL = "datatables/ship/components/character_builder/frog_capacitor.iff";
	
	public static final String GENERIC_PROMPT = "Select the desired option";
	public static final String GENERIC_TITLE = "Character Builder Terminal";
	
	public static final string_id SID_TERMINAL_PROMPT = new string_id("skill_teacher","skill_terminal_prompt");
	public static final string_id SID_TERMINAL_TITLE = new string_id("skill_teacher","skill_terminal_title");
	public static final string_id SID_TERMINAL_DISABLED = new string_id("skill_teacher","skill_terminal_disabled");
	public static final string_id SID_TERMINAL_DENIED = new string_id("skill_teacher","skill_terminal_denied");
	public static final string_id SID_TERMINAL_MAX_SKILLS = new string_id("skill_teacher","skill_terminal_max_skills");
	public static final string_id PROSE_GRANT_SKILL = new string_id("skill_teacher","skill_terminal_grant");
	public static final string_id PROSE_GRANT_SKILL_FAIL = new string_id("skill_teacher","skill_terminal_grant_fail");
	
	public static final String[] CHARACTER_BUILDER_OPTIONS =
	{
		"Weapons",
		"Armor Sets",
		"Roadmap Skill Builder",
		"Resources",
		"Credits",
		"Faction",
		"Vehicles/Mounts/Beasts",
		"Ships",
		"Crafting Items",
		"Structure Deeds",
		"PA Halls",
		"Misc Items/Medicines",
		"Jedi Options",
		"Best Resource",
		"Flag for All Heroic Instances",
		"Draft Schematics"
	};
	public static final String[] RESOURCE_TYPES =
	{
		"Creature Resources",
		"Flora Resources",
		"Chemical",
		"Water",
		"Mineral",
		"Gas",
		"Energy",
		"Asteroid"
	};
	
	public static final String[] BEST_RESOURCE_TYPES =
	{
		"Creature Resources",
		"Flora Resources",
		"Chemical",
		"Water",
		"Mineral",
		"Gas",
		"Energy",
		"Asteroid",
		"Filter by specific attribute"
	};
	
	public static final String[] SPACE_RESOURCE_LOCALIZED = 
	{
		"@resource/resource_names:space_chemical_acid",
		"@resource/resource_names:space_chemical_cyanomethanic",
		"@resource/resource_names:space_chemical_petrochem",
		"@resource/resource_names:space_chemical_sulfuric",
		"@resource/resource_names:space_gas_methane",
		"@resource/resource_names:space_gas_organometallic",
		"@resource/resource_names:space_gem_crystal",
		"@resource/resource_names:space_gem_diamond",
		"@resource/resource_names:space_metal_carbonaceous",
		"@resource/resource_names:space_metal_ice",
		"@resource/resource_names:space_metal_iron",
		"@resource/resource_names:space_metal_obsidian",
		"@resource/resource_names:space_metal_silicaceous"
	};
	
	public static final String[] SPACE_RESOURCE_CONST = 
	{
		"space_chemical_acid",
		"space_chemical_cyanomethanic",
		"space_chemical_petrochem",
		"space_chemical_sulfuric",
		"space_gas_methane",
		"space_gas_organometallic",
		"space_gem_crystal",
		"space_gem_diamond",
		"space_metal_carbonaceous",
		"space_metal_ice",
		"space_metal_iron",
		"space_metal_obsidian",
		"space_metal_silicaceous"
	};
	
	public static final String[] RESOURCE_BASE_TYPES =
	{
		"creature_resources",
		"flora_resources",
		"chemical",
		"water",
		"mineral",
		"gas",
		"energy",
		"Asteroid"
	};
	public static final String[] VEHICLE_MOUNT_OPTIONS =
	{
		"Vehicles",
		"Mounts",
		"Beasts"
	};
	public static final String[] VEHICLE_OPTIONS =
	{
		"Swoop",
		"Speederbike",
		"X34",
		"AB1",
		"V35",
		"XP38",
		"Barc Speeder",
		"AV21",
		"X31",
		"Flash Speeder"
	};
	public static final String[] MOUNT_OPTIONS =
	{
		"Carrion Spat",
		"Kaduu",
		"Dewback",
		"Bol",
		"Falumpaset",
		"Brackaset",
		"Cu Pa - Creature Handler Version",
		"Bantha - Creature Handler Version"
	};
	
	public static final String[] BEAST_OPTIONS_FOR_PLAYERS =
	{
		"angler",
		"bageraset",
		"bantha",
		"bark_mite",
		"bearded_jax",
		"boar_wolf",
		"bocatt",
		"bolle_bol",
		"bordok",
		"borgle",
		"blurrg",
		"capper_spineflap",
		"choku",
		"chuba",
		"condor_dragon",
		"dalyrake",
		"durni",
		"fambaa",
		"gronda",
		"gurk",
		"ikopi",
		"mamien",
		"mynock",
		"quenker",
		"reptilian_flier"
	};
	
	public static final String[] BEAST_OPTIONS =
	{
		"acklay",
		"angler",
		"bageraset",
		"bantha",
		"bark_mite",
		"baz_nitch",
		"bearded_jax",
		"blistmok",
		"blurrg",
		"boar_wolf",
		"bocatt",
		"bol",
		"bolle_bol",
		"bolma",
		"bolotaur",
		"bordok",
		"borgle",
		"brackaset",
		"capper_spineflap",
		"carrion_spat",
		"choku",
		"chuba",
		"condor_dragon",
		"corellian_butterfly",
		"corellian_sand_panther",
		"corellian_slice_hound",
		"crystal_snake",
		"cu_pa",
		"dalyrake",
		"dewback",
		"dune_lizard",
		"durni",
		"dwarf_nuna",
		"eopie",
		"falumpaset",
		"fambaa",
		"fanned_rawl",
		"flewt",
		"flit",
		"fynock",
		"gackle_bat",
		"gaping_spider",
		"gnort",
		"graul",
		"gronda",
		"gualama",
		"gubbur",
		"guf_drolg",
		"gulginaw",
		"gurk",
		"gurnaset",
		"gurreck",
		"hanadak",
		"hermit_spider",
		"horned_krevol",
		"horned_rasp",
		"huf_dun",
		"huurton",
		"ikopi",
		"jundak",
		"kaadu",
		"kai_tok",
		"kashyyyk_bantha",
		"kima",
		"kimogila",
		"kittle",
		"kliknik",
		"krahbu",
		"kubaza_beetle",
		"kusak",
		"kwi",
		"langlatch",
		"lantern_bird",
		"lava_flea",
		"malkloc",
		"mamien",
		"mawgax",
		"merek",
		"minstyngar",
		"mott",
		"mouf",
		"murra",
		"mutated_acklay",
		"mutated_boar",
		"mutated_borgax",
		"mutated_cat",
		"mutated_chuba_fly",
		"mutated_cu_pa",
		"mutated_dewback",
		"mutated_griffon",
		"mutated_jax",
		"mutated_quenker",
		"mutated_rancor",
		"mutated_slice_hound",
		"mutated_varasquactyl",
		"mynock",
		"narglatch",
		"nerf",
		"nuna",
		"peko_peko",
		"perlek",
		"pharple",
		"piket",
		"plumed_rasp",
		"pugoriss",
		"purbole",
		"quenker",
		"rancor",
		"remmer",
		"reptilian_flier",
		"roba",
		"rock_mite",
		"ronto",
		"salt_mynock",
		"sharnaff",
		"shaupaut",
		"shear_mite",
		"skreeg",
		"snorbal",
		"spined_puc",
		"spined_snake",
		"squall",
		"squill",
		"stintaril",
		"swirl_prong",
		"tanc_mite",
		"tanray",
		"tauntaun",
		"thune",
		"torton",
		"tulrus",
		"tusk_cat",
		"tybis",
		"uller",
		"varactyl",
		"veermok",
		"verne",
		"vesp",
		"vir_vur",
		"voritor_lizard",
		"vynock",
		"walluga",
		"wampa",
		"webweaver",
		"whisper_bird",
		"womp_rat",
		"woolamander",
		"worrt",
		"xandank",
		"zucca_boar"
	};
	public static final String[] SHIP_OPTIONS =
	{
		"Rebel",
		"Imperial",
		"Freelance",
		"Other",
		"Parts",
		"Pilot Skills"
	};
	public static final String[] REBEL_SHIP_OPTIONS =
	{
		"Incom X4 Gunship",
		"Z95",
		"Y-Wing",
		"Y-Wing Longprobe",
		"X-Wing",
		"Advanced X-Wing",
		"A-Wing",
		"B-Wing",
		"YKL-37R Nova Courier"
	};
	public static final String[] REBEL_SHIP_TYPES =
	{
		"gunship_rebel",
		"z95",
		"ywing",
		"ywing_longprobe",
		"xwing",
		"advanced_xwing",
		"awing",
		"bwing",
		"ykl37r"
	};
	public static final String[] IMPERIAL_SHIP_OPTIONS =
	{
		"Imperial YE-4 Gunship",
		"TIE Fighter (Light Duty)",
		"TIE Fighter",
		"TIE/In",
		"TIE Interceptor",
		"TIE Bomber",
		"TIE Advanced",
		"TIE Aggressor",
		"TIE Interceptor (Imperial Guard)",
		"VT-49 Decimator"
	};
	public static final String[] IMPERIAL_SHIP_TYPES =
	{
		"gunship_imperial",
		"tie_light_duty",
		"tiefighter",
		"tie_in",
		"tieinterceptor",
		"tiebomber",
		"tieadvanced",
		"tieaggressor",
		"tieinterceptor_imperial_guard",
		"decimator"
	};
	public static final String[] FREELANCE_SHIP_OPTIONS =
	{
		"Blacksun AEG-77 Vigo Gunship",
		"Naboo N-1",
		"Scyk",
		"Dunelizard",
		"Kimogila",
		"Kihraxz",
		"Ixiyen",
		"Rihkxyrk",
		"Vaksai",
		"Krayt",
	};
	public static final String[] FREELANCE_SHIP_TYPES =
	{
		"gunship_neutral",
		"naboo_n1",
		"hutt_light_s01",
		"hutt_medium_s01",
		"hutt_heavy_s01",
		"blacksun_light_s01",
		"blacksun_medium_s01",
		"blacksun_heavy_s01",
		"blacksun_vaksai",
		"hutt_turret_ship",
	};
	public static final String[] OTHER_SHIP_OPTIONS =
	{
		"Sorosuub",
		"Eta-2 Actis (Jedi Starfighter)",
		"Belbullab-22 (Grievous' Starship)",
		"ARC-170",
		"KSE Firespray",
		"Y-8 Mining Ship",
		"YT-1300",
		"YT-2400"
	};
	public static final String[] OTHER_SHIP_TYPES =
	{
		"sorosuub_space_yacht",
		"jedi_starfighter",
		"grievous_starship",
		"arc170",
		"firespray",
		"y8_mining_ship",
		"yt1300",
		"yt2400"
	};
	public static final String[] MAIN_SHIP_OPTIONS =
	{
		"Ship Components",
		"Ship Chassis",
		"Gunship Collection Reward Schematics",
		"Component Schematics",
		"Pilot Skills"
	};
	
	public static final String[] PILOT_SKILLS =
	{
		"Master Imperial Pilot",
		"Master Rebel Pilot",
		"Neutral Pilot",
		"Revoke Pilot"
	};
	
	public static final String[] DEED_CRAFTING_OPTIONS =
	{
		"Deeds",
		"Crafting Items"
	};
	public static final String[] DEED_OPTIONS =
	{
		"Clothing Factory",
		"Food Factory",
		"Equipment Factory",
		"Structure Factory",
		"Generic House",
		"Small Tatooine House",
		"Small Naboo House",
		"Small Corellia House",
		"Merchant Tent",
		"Heavy Mineral Harvester",
		"Heavy Flora Harvester",
		"Heavy Gas Harvester",
		"Heavy Chemical Harvester",
		"Heavy Moisture Vaporator",
		"Fusion Reactor",
		"Corellia City Pack",
		"Naboo City Pack",
		"Tatooine City Pack"

	};
	
	public static final String[] CRAFTING_OPTIONS =
	{
		"Weapon Crafting Station",
		"Structure Crafting Station",
		"Clothing Crafting Station",
		"Food Crafting Station",
		"Generic Crafting Tool",
		"Weapon Crafting Tool",
		"Structure Crafting Tool",
		"Clothing Crafting Tool",
		"Food Crafting Tool",
		"Ship Crafting Tool",
		"Ship Crafting Station"

	};
	public static final String[] PA_OPTIONS =
	{
		"Generic PA Hall",
		"Tatooine PA Hall",
		"Naboo PA Hall",
		"Corellia PA Hall",
		"Tatooine City Hall",
		"Naboo City Hall",
		"Corellia City Hall"

	};
	public static final String[] WEAPON_OPTIONS =
	{
		"Pistols",
		"Carbines",
		"Rifles",
		"Heavy Weapons",
		"Unarmed Weapons",
		"One-Handed Weapons",
		"Two-Handed Weapons",
		"Polearm Weapons",
		"Battlefield Weapons"

	};
	public static final String[] PISTOL_OPTIONS =
	{
		"CDEF Pistol",
		"Scout Blaster",
		"Geonosian Sonic Blaster",
		"Republic Blaster",
		"Launcher",
		"High Capacity Scatter Pistol",
		"Scatter Pistol",
		"SR Combat",
		"Striker",
		"Tangle",
		"Power5",
		"FWG5",
		"DX2",
		"DL44 XT",
		"DL44",
		"DH17",
		"D18",
		"Alliance Disruptor",
		"Deathhammer",
		"Flare Pistol",
		"Flechette Pistol",
		"Intimidator",
		"Jawa Pistol",
		"Renegade Pistol",
		"DE-10",
		"DL44 Metal",
		"Disruptor Pistol",
		"Ion Relic Pistol",
		"Pistol Lamprey"
	};
	
	public static final String[] CARBINE_OPTIONS =
	{
		"CDEF Carbine",
		"DH 17 Carbine",
		"DH 17 Snubnose",
		"E11 Carbine",
		"E11 Carbine Mark 2",
		"Laser",
		"DxR 6",
		"EE 3",
		"Elite Carbine",
		"Nym Slugthrower",
		"Alliance Needler",
		"Bothan Bola",
		"E 5 Carbine",
		"Geonosian Carbine",
		"Proton Carbine",
		"Czerka Dart Carbine",
		"Sfor Carbine",
		"Whistler Modified"

	};
	public static final String[] RIFLE_OPTIONS =
	{
		"BWDL19 Rifle (Death Trooper)",
		"TC-22 Blaster Rifle Replica (GCW Rifle)",
		"Jinkins J-1 Rifle (Nym's Themepark)",
		"CDEF",
		"DLT 20",
		"DLT 20a",
		"Tusken",
		"Advanced Laser Rifle",
		"SG 82",
		"Spraystick",
		"E 11",
		"Jawa Ion",
		"T 21",
		"Tenloss DxR 6",
		"Beserker",
		"Light Bowcaster",
		"Recon Bowcaster",
		"Assault Bowcaster",
		"Beam Rifle",
		"Acid Beam Rifle",
		"High Velocity Sniper Rifle",
		"LD 1 Rifle",
		"Massassi Ink Rifle",
		"Proton Rifle",
		"Lightning Rifle",
		"Laser Rifle",
		"Heavy Lightning Rifle",
		"DP-23 Rifle",
		"Mustafar Disruptor Rifle",
		"Tusken Elite"

	};
	public static final String[] HEAVY_WEAPON_OPTIONS =
	{
		"Rocket Launcher",
		"Plasma Flame Thrower",
		"Acid Beam",
		"Light Lightning Canon",
		"Heavy Acid Beam",
		"Heavy Lighting Beam",
		"Heavy Particle Beam",
		"Flame Thrower",
		"Heavy Republic Flame Thrower",
		"Lava Cannon",
		"Crusader M-XX Heavy Rifle",
		"C-M 'Frag Storm' Heavy Shotgun",
		"Devastator Acid Launcher",
		"CC-V 'Legion' Cryo Projector"
	};
	public static final String[] UNARMED_OPTIONS =
	{
		"Vibroknuckler",
		"Massasiknuckler",
		"Razorknuckler",
		"Basterfist",
		"Guardian Blaster Fist"

	};
	public static final String[] ONEHANDED_OPTIONS =
	{
		"Survival Knife",
		"'Twilek' Dagger",
		"Sword",
		"Curved Sword",
		"Gaderiffi Baton",
		"Vibroblade",
		"Ryyk Blade",
		"Rantok Blade",
		"Stun Baton",
		"Acid Sword",
		"Junta Mace",
		"Marauder Sword",
		"Massassi Sword",
		"RSF SWORD",
		"Stone Knife",
		"Janta Knife",
		"Donkuwa Knife",
		"Nyax Curved Sword",
		"Obsidian Sword",
		"Mustafar Bandit Sword",
		"Gaderiffi Elite"

	};
	public static final String[] TWOHANDED_OPTIONS =
	{
		"Axe",
		"Battleaxe",
		"Katana",
		"Vibroaxe",
		"Cleaver",
		"Power Hammer",
		"Scythe",
		"Kashyyk Sword",
		"Sith Sword",
		"Nyax Sword",
		"Black Sun Executioner's Hack",
		"Tulrus Sword",
		"Obsidian 2h Sword",
		"Massassi Enforcer Blade"

	};
	public static final String[] POLEARM_OPTIONS =
	{
		"1H Wood Staff",
		"Metal Staff",
		"Reinforced Staff",
		"Vibro Lance",
		"Long Vibro Axe",
		"Lance",
		"Kaminoan Great Staff",
		"Massassi Lance",
		"Shocklance",
		"Cryo Lance",
		"Kashyyk BladeStick",
		"Electric Polearm",
		"Nightsister Energy Lance",
		"Nightsister Lance",
		"Obsidian Lance",
		"Xandank Lance",
		"Acidic Paragon Vibro Axe"
	};
	public static final String[] GRENADE_OPTIONS =
	{
		"Light Fragmentation Grenade",
		"Fragmentation Grenade",
		"Imperial Detonator",
		"Proton Grenade",
		"Thermal Detonator",
		"Glop Detonator",
		"Cryoban"

	};
	
	public static final String[] BATTLEFIELD_WEAPON_OPTIONS =
	{
		"Westar-34 Blaster Pistol",
		"NGant-Zarvel 9118 Carbine",
		"Westar-M5 Blaster Rifle",
		"CR-1 Blast Cannon",
		"Buzz-Knuck",
		"Sith Sword",
		"Vibrosword",
		"Magnaguard Electrostaff",
		"One-Handed Sith-Saber Hilt",
		"Two-Handed Mysterious Lightsaber Hilt",
		"Double-Bladed Darth Phobos Lightsaber Hilt"
	};
	
	public static final String[] ARMOR_OPTIONS =
	{
		"Assault Armor, For Classes: Commando, Bounty Hunter",
		"Battle Armor, For Classes: Spy, Officer",
		"Reconnaissance Armor, For Classes: Medic, Smuggler",
		"Personal Shield Generators",
		"Combat Enhancement Items",
		"PvP Sets",
		"Heroic Jewelry"
	};
	
	public static final String[] ARMOR_ASSAULT_OPTIONS =
	{
		"Composite Armor Set",
		"Marauder Assault Armor Set",
		"Chitin Armor Set",
		"Crafted Bounty Hunter Armor Set",
		"Kashyyykian Hunting Armor Set",
		"Ithorian Sentinel Armor Set",
		"Shocktrooper (Imperial Factional) Armor Set",
		"Rebel Assault (Rebel Factional) Armor Set"
	};
	public static final String[] ARMOR_BATTLE_OPTIONS =
	{
		"Padded Armor Set",
		"Marauder Battle Armor Set",
		"RIS Armor Set",
		"Bone Armor Set",
		"Kashyyykian Black Mountain Armor Set",
		"Ithorian Defender Armor Set",
		"Stormtrooper (Imperial Factional) Armor Set",
		"Rebel Battle (Rebel Factional) Armor Set",
		"Imperial Snowtrooper Armor Set"
	};
	public static final String[] ARMOR_RECON_OPTIONS =
	{
		"Tantel Armor Set",
		"Ubese Armor Set",
		"Mabari Armor Set",
		"Recon Marauder Set",
		"Kashyyykian Ceremonial Armor Set",
		"Ithorian Guardian Armor Set",
		"Scout Trooper (Imperial Factional) Armor Set",
		"Marine Rebel (Rebel Factional) Armor Set",
		"Alliance Cold Weather Armor set"
	};
	public static final String[] ARMOR_PROTECTION_AMOUNT =
	{
		"Basic",
		"Standard",
		"Advanced"
	};
	public static final String[] ARMOR_PSG_OPTIONS =
	{
		"MARK I",
		"MARK II",
		"MARK III"
	};
	public static final String[] ARMOR_ENHANCEMENT_OPTIONS =
	{
		"Combat Enhancement Ring"

	};
	
	public static final String[] ARMOR_PVP_SETS =
	{
		"Imperial Black PvP",
		"Imperial White PvP",
		"Rebel Grey PvP",
		"Rebel Green PvP",
		"Galactic Marine Armor Set",
		"Rebel SpecForce Armor Set"
	};
	
	public static final String[] MISCITEM_OPTIONS =
	{
		"Misc Items",
		"Medicines",
		"Power Ups",
		"Exotic Attachments",
		"Socketed Clothing",
		"Basic Armor Attachments",
		"Basic Clothing Attachments",
		"Crafting Suit",
		"Aurilia Crystals"
	};
	public static final String[] CLOTHING_OPTIONS =
	{
		"Socketed Shirt",
		"Socketed Gloves",
		"Socketed Hat",
		"Socketed Pants",
		"Socketed Jacket",
		"Socketed Boots",
	};
	
	public static final String[] ATTACHMENT_OPTIONS =
	{
		"Weapon",
		"Armor",
		"Chest Armor",
		"Shirt"
	};
	
	public static final String[] MISC_OPTIONS =
	{
		"Spec-Ops Pack",
		"Krayt Skull Pack",
		"Pilot Ace Pack",
		"Jedi Belt Pouch",
		"Snowtrooper Backpack",
		"Alliance Cold Weather Backpack",
		"Arakyd Probe Droid",
		"Seeker Droid",
		"Master Crafted EE3 Schematic",
		"Master Crafted DC-15 Schematic",
		"Vet 30k Resource Deed"

	};
	public static final String[] MEDICINE_OPTIONS =
	{
		"High Charge Stimpack-A",
		"High Charge Stimpack-B",
		"High Charge Stimpack-C",
		"High Charge Stimpack-D"

		
	};
	public static final String[] HEAL_OPTIONS =
	{
		"Heal Wounds",
		"Heal Battle Fatigue"
	};
	
	public static final String[] SMUGGLER_TOOLS_OPTIONS =
	{
		"Simple Toolkit",
		"Finely Crafted Toolset",
		"Trandoshan Interframe",
		"Delicate Trigger",
		"Illegal Core Booster",
		"Mandalorian Interframe",
		"Micro Plating"
	};
	
	public static final String[] FACTION_OPTIONS =
	{
		"Receive Faction Points",
		"Declare Faction Rebel",
		"Declare Faction Imperial",
		
		"Gain One Faction Rank",
		"Lose One Faction Rank",
		"Resign From Current Faction"
	};
	
	public static final String[] ROADMAP_SKILL_OPTIONS =
	{
		"Select Roadmap",
		"Earn Current Skill",
		"Set Combat Level",
		"Level 90 - Reset respecs"
	};
	
	public static final String[] JEDI_OPTIONS =
	{
		"Saber Crystals",
		"Light Sabers",
		"Jedi Robes"

	};
	public static final String[] PUBLISH_OPTIONS =
	{
		"Heavy Weapons",
		"Jedi Options",
		"Traps",
		"Death Watch Bunker Entrance Flag",
		"Spy Camouflage Gear"
	};
	
	public static final String[] CRYSTAL_OPTIONS =
	{
		"Color Crystals",
		"Lava Crystal",
		"Power Crystals",
		"Ancient Krayt Pearls"
	};
	
	public static final String[] SABER_OPTIONS =
	{
		"Training Saber",
		"Generation One Sabers",
		"Generation Two Sabers",
		"Generation Three Sabers",
		"Generation Four Sabers",
		"Generation Five Sabers"
	};
	public static final String[] JEDI_PLAYTEST_OPTIONS =
	{
		"Level 26 Playtest",
		"Level 86 Playtest"
	};
	public static final String[] ROBE_OPTIONS =
	{
		"Padawan Robe",
		"(40)Light Acolyte Robe",
		"(40)Dark Acolyte Robe",
		"(60)Light Apprentice Robe",
		"(60)Dark Apprentice Robe",
		"(80)Light Jedi Knight Robe",
		"(80)Dark Jedi Knight Robe",
		"(80)Jedi Master Cloak Brown",
		"(80)Jedi Master Cloak Black",
		"Reset Jedi Statue Slots for Master Jedi Cloaks Collection"
	};
	public static final String ARMOR_SET_PREFIX = "object/tangible/wearables/armor/";
	public static final String[] ARMOR_SET_ASSAULT_1 =
	{
		"composite/armor_composite_bicep_l.iff",
		"composite/armor_composite_chest_plate.iff",
		"composite/armor_composite_bicep_r.iff",
		"composite/armor_composite_gloves.iff",
		"composite/armor_composite_boots.iff",
		"composite/armor_composite_helmet.iff",
		"composite/armor_composite_bracer_l.iff",
		"composite/armor_composite_leggings.iff",
		"composite/armor_composite_bracer_r.iff"
	};
	public static final String[] ARMOR_SET_ASSAULT_2 =
	{
		"chitin/armor_chitin_s01_bicep_l.iff",
		"chitin/armor_chitin_s01_bicep_r.iff",
		"chitin/armor_chitin_s01_boots.iff",
		"chitin/armor_chitin_s01_bracer_l.iff",
		"chitin/armor_chitin_s01_bracer_r.iff",
		"chitin/armor_chitin_s01_chest_plate.iff",
		"chitin/armor_chitin_s01_gloves.iff",
		"chitin/armor_chitin_s01_helmet.iff",
		"chitin/armor_chitin_s01_leggings.iff"
	};
	public static final String[] ARMOR_SET_ASSAULT_3 =
	{
		"marauder/armor_marauder_s03_bicep_l.iff",
		"marauder/armor_marauder_s03_bicep_r.iff",
		"marauder/armor_marauder_s03_boots.iff",
		"marauder/armor_marauder_s03_bracer_l.iff",
		"marauder/armor_marauder_s03_bracer_r.iff",
		"marauder/armor_marauder_s03_chest_plate.iff",
		"marauder/armor_marauder_s03_gloves.iff",
		"marauder/armor_marauder_s03_helmet.iff",
		"marauder/armor_marauder_s03_leggings.iff"
	};
	public static final String[] ARMOR_SET_ASSAULT_4 =
	{
		"bounty_hunter/armor_bounty_hunter_crafted_bicep_l.iff",
		"bounty_hunter/armor_bounty_hunter_crafted_bicep_r.iff",
		"bounty_hunter/armor_bounty_hunter_crafted_boots.iff",
		"bounty_hunter/armor_bounty_hunter_crafted_bracer_l.iff",
		"bounty_hunter/armor_bounty_hunter_crafted_bracer_r.iff",
		"bounty_hunter/armor_bounty_hunter_crafted_chest_plate.iff",
		"bounty_hunter/armor_bounty_hunter_crafted_gloves.iff",
		"bounty_hunter/armor_bounty_hunter_crafted_helmet.iff",
		"bounty_hunter/armor_bounty_hunter_crafted_leggings.iff"
	};
	public static final String[] ARMOR_SET_BATTLE_1 =
	{
		"bone/armor_bone_s01_bicep_l.iff",
		"bone/armor_bone_s01_bicep_r.iff",
		"bone/armor_bone_s01_boots.iff",
		"bone/armor_bone_s01_bracer_l.iff",
		"bone/armor_bone_s01_bracer_r.iff",
		"bone/armor_bone_s01_chest_plate.iff",
		"bone/armor_bone_s01_gloves.iff",
		"bone/armor_bone_s01_helmet.iff",
		"bone/armor_bone_s01_leggings.iff"
	};
	public static final String[] ARMOR_SET_BATTLE_2 =
	{
		"marauder/armor_marauder_s01_bicep_l.iff",
		"marauder/armor_marauder_s01_bicep_r.iff",
		"marauder/armor_marauder_s01_boots.iff",
		"marauder/armor_marauder_s01_bracer_l.iff",
		"marauder/armor_marauder_s01_bracer_r.iff",
		"marauder/armor_marauder_s01_chest_plate.iff",
		"marauder/armor_marauder_s01_gloves.iff",
		"marauder/armor_marauder_s01_helmet.iff",
		"marauder/armor_marauder_s01_leggings.iff"
	};
	public static final String[] ARMOR_SET_BATTLE_3 =
	{
		"padded/armor_padded_s01_bicep_l.iff",
		"padded/armor_padded_s01_bicep_r.iff",
		"padded/armor_padded_s01_boots.iff",
		"padded/armor_padded_s01_bracer_l.iff",
		"padded/armor_padded_s01_bracer_r.iff",
		"padded/armor_padded_s01_chest_plate.iff",
		"padded/armor_padded_s01_gloves.iff",
		"padded/armor_padded_s01_helmet.iff",
		"padded/armor_padded_s01_leggings.iff"
	};
	public static final String[] ARMOR_SET_BATTLE_4 =
	{
		"ris/armor_ris_bicep_l.iff",
		"ris/armor_ris_bracer_l.iff",
		"ris/armor_ris_gloves.iff",
		"ris/armor_ris_bicep_r.iff",
		"ris/armor_ris_bracer_r.iff",
		"ris/armor_ris_helmet.iff",
		"ris/armor_ris_boots.iff",
		"ris/armor_ris_chest_plate.iff",
		"ris/armor_ris_leggings.iff"
	};
	public static final String[] ARMOR_SET_RECON_1 =
	{
		"zam/armor_zam_wesell_helmet.iff",
		"zam/armor_zam_wesell_boots.iff",
		"zam/armor_zam_wesell_chest_plate.iff",
		"zam/armor_zam_wesell_pants.iff",
		"zam/armor_zam_wesell_gloves.iff"
	};
	public static final String[] ARMOR_SET_RECON_2 =
	{
		"marauder/armor_marauder_s02_bicep_l.iff",
		"marauder/armor_marauder_s02_bicep_r.iff",
		"marauder/armor_marauder_s02_boots.iff",
		"marauder/armor_marauder_s02_bracer_l.iff",
		"marauder/armor_marauder_s02_bracer_r.iff",
		"marauder/armor_marauder_s02_chest_plate.iff",
		"marauder/armor_marauder_s02_gloves.iff",
		"marauder/armor_marauder_s02_helmet.iff",
		"marauder/armor_marauder_s02_leggings.iff"
	};
	public static final String[] ARMOR_SET_RECON_3 =
	{
		"tantel/armor_tantel_skreej_bicep_l.iff",
		"tantel/armor_tantel_skreej_chest_plate.iff",
		"tantel/armor_tantel_skreej_bicep_r.iff",
		"tantel/armor_tantel_skreej_boots.iff",
		"tantel/armor_tantel_skreej_gloves.iff",
		"tantel/armor_tantel_skreej_helmet.iff",
		"tantel/armor_tantel_skreej_bracer_l.iff",
		"tantel/armor_tantel_skreej_bracer_r.iff",
		"tantel/armor_tantel_skreej_leggings.iff"
	};
	public static final String[] ARMOR_SET_RECON_4 =
	{
		"ubese/armor_ubese_boots.iff",
		"ubese/armor_ubese_jacket.iff",
		"ubese/armor_ubese_bracer_l.iff",
		"ubese/armor_ubese_bracer_r.iff",
		"ubese/armor_ubese_pants.iff",
		"ubese/armor_ubese_gloves.iff",
		"ubese/armor_ubese_helmet.iff",
		"ubese/armor_ubese_shirt.iff"
	};
	public static final String[] ARMOR_SET_ASSAULT_ITHORIAN =
	{
		"ithorian_sentinel/ith_armor_s03_bicep_l.iff",
		"ithorian_sentinel/ith_armor_s03_chest_plate.iff",
		"ithorian_sentinel/ith_armor_s03_bicep_r.iff",
		"ithorian_sentinel/ith_armor_s03_gloves.iff",
		"ithorian_sentinel/ith_armor_s03_boots.iff",
		"ithorian_sentinel/ith_armor_s03_helmet.iff",
		"ithorian_sentinel/ith_armor_s03_bracer_l.iff",
		"ithorian_sentinel/ith_armor_s03_leggings.iff",
		"ithorian_sentinel/ith_armor_s03_bracer_r.iff"
	};
	public static final String[] ARMOR_SET_BATTLE_ITHORIAN =
	{
		"ithorian_defender/ith_armor_s01_bicep_l.iff",
		"ithorian_defender/ith_armor_s01_chest_plate.iff",
		"ithorian_defender/ith_armor_s01_bicep_r.iff",
		"ithorian_defender/ith_armor_s01_gloves.iff",
		"ithorian_defender/ith_armor_s01_boots.iff",
		"ithorian_defender/ith_armor_s01_helmet.iff",
		"ithorian_defender/ith_armor_s01_bracer_l.iff",
		"ithorian_defender/ith_armor_s01_leggings.iff",
		"ithorian_defender/ith_armor_s01_bracer_r.iff"
	};
	public static final String[] ARMOR_SET_RECON_ITHORIAN =
	{
		"ithorian_guardian/ith_armor_s02_bicep_l.iff",
		"ithorian_guardian/ith_armor_s02_chest_plate.iff",
		"ithorian_guardian/ith_armor_s02_bicep_r.iff",
		"ithorian_guardian/ith_armor_s02_gloves.iff",
		"ithorian_guardian/ith_armor_s02_boots.iff",
		"ithorian_guardian/ith_armor_s02_helmet.iff",
		"ithorian_guardian/ith_armor_s02_bracer_l.iff",
		"ithorian_guardian/ith_armor_s02_leggings.iff",
		"ithorian_guardian/ith_armor_s02_bracer_r.iff"
	};
	public static final String[] ARMOR_SET_ASSAULT_WOOKIEE =
	{
		"kashyyykian_hunting/armor_kashyyykian_hunting_bicep_l.iff",
		"kashyyykian_hunting/armor_kashyyykian_hunting_bicep_r.iff",
		"kashyyykian_hunting/armor_kashyyykian_hunting_bracer_l.iff",
		"kashyyykian_hunting/armor_kashyyykian_hunting_bracer_r.iff",
		"kashyyykian_hunting/armor_kashyyykian_hunting_chestplate.iff",
		"kashyyykian_hunting/armor_kashyyykian_hunting_leggings.iff"
	};
	public static final String[] ARMOR_SET_BATTLE_WOOKIEE =
	{
		"kashyyykian_black_mtn/armor_kashyyykian_black_mtn_bicep_l.iff",
		"kashyyykian_black_mtn/armor_kashyyykian_black_mtn_bicep_r.iff",
		"kashyyykian_black_mtn/armor_kashyyykian_black_mtn_bracer_l.iff",
		"kashyyykian_black_mtn/armor_kashyyykian_black_mtn_bracer_r.iff",
		"kashyyykian_black_mtn/armor_kashyyykian_black_mtn_chestplate.iff",
		"kashyyykian_black_mtn/armor_kashyyykian_black_mtn_leggings.iff"
	};
	public static final String[] ARMOR_SET_RECON_WOOKIEE =
	{
		"kashyyykian_ceremonial/armor_kashyyykian_ceremonial_bicep_l.iff",
		"kashyyykian_ceremonial/armor_kashyyykian_ceremonial_bicep_r.iff",
		"kashyyykian_ceremonial/armor_kashyyykian_ceremonial_bracer_l.iff",
		"kashyyykian_ceremonial/armor_kashyyykian_ceremonial_bracer_r.iff",
		"kashyyykian_ceremonial/armor_kashyyykian_ceremonial_chestplate.iff",
		"kashyyykian_ceremonial/armor_kashyyykian_ceremonial_leggings.iff"
	};
	public static final String[] ARMOR_SET_ASSAULT_IMPERIAL =
	{
		"assault_trooper/armor_assault_trooper_chest_plate.iff",
		"assault_trooper/armor_assault_trooper_leggings.iff",
		"assault_trooper/armor_assault_trooper_helmet.iff",
		"assault_trooper/armor_assault_trooper_bicep_l.iff",
		"assault_trooper/armor_assault_trooper_bicep_r.iff",
		"assault_trooper/armor_assault_trooper_bracer_l.iff",
		"assault_trooper/armor_assault_trooper_bracer_r.iff",
		"assault_trooper/armor_assault_trooper_boots.iff",
		"assault_trooper/armor_assault_trooper_gloves.iff"
	};
	public static final String[] ARMOR_SET_BATTLE_IMPERIAL =
	{
		"stormtrooper/armor_stormtrooper_chest_plate.iff",
		"stormtrooper/armor_stormtrooper_leggings.iff",
		"stormtrooper/armor_stormtrooper_helmet.iff",
		"stormtrooper/armor_stormtrooper_bicep_l.iff",
		"stormtrooper/armor_stormtrooper_bicep_r.iff",
		"stormtrooper/armor_stormtrooper_bracer_l.iff",
		"stormtrooper/armor_stormtrooper_bracer_r.iff",
		"stormtrooper/armor_stormtrooper_boots.iff",
		"stormtrooper/armor_stormtrooper_gloves.iff"
	};
	public static final String[] ARMOR_SET_RECON_IMPERIAL =
	{
		"scout_trooper/armor_scout_trooper_chest_plate.iff",
		"scout_trooper/armor_scout_trooper_leggings.iff",
		"scout_trooper/armor_scout_trooper_helmet.iff",
		"scout_trooper/armor_scout_trooper_bicep_l.iff",
		"scout_trooper/armor_scout_trooper_bicep_r.iff",
		"scout_trooper/armor_scout_trooper_bracer_l.iff",
		"scout_trooper/armor_scout_trooper_bracer_r.iff",
		"scout_trooper/armor_scout_trooper_boots.iff",
		"scout_trooper/armor_scout_trooper_gloves.iff"
	};
	public static final String[] ARMOR_SET_ASSAULT_REBEL =
	{
		"rebel_assault/armor_rebel_assault_chest_plate.iff",
		"rebel_assault/armor_rebel_assault_leggings.iff",
		"rebel_assault/armor_rebel_assault_helmet.iff",
		"rebel_assault/armor_rebel_assault_bicep_l.iff",
		"rebel_assault/armor_rebel_assault_bicep_r.iff",
		"rebel_assault/armor_rebel_assault_bracer_l.iff",
		"rebel_assault/armor_rebel_assault_bracer_r.iff",
		"rebel_assault/armor_rebel_assault_boots.iff",
		"rebel_assault/armor_rebel_assault_gloves.iff"
	};
	public static final String[] ARMOR_SET_BATTLE_REBEL =
	{
		"rebel_battle/armor_rebel_battle_chest_plate.iff",
		"rebel_battle/armor_rebel_battle_leggings.iff",
		"rebel_battle/armor_rebel_battle_helmet.iff",
		"rebel_battle/armor_rebel_battle_bicep_l.iff",
		"rebel_battle/armor_rebel_battle_bicep_r.iff",
		"rebel_battle/armor_rebel_battle_bracer_l.iff",
		"rebel_battle/armor_rebel_battle_bracer_r.iff",
		"rebel_battle/armor_rebel_battle_boots.iff",
		"rebel_battle/armor_rebel_battle_gloves.iff"
	};
	public static final String[] ARMOR_SET_BATTLE_SNOWTROOPER =
	{
		"snowtrooper/armor_snowtrooper_chest_plate.iff",
		"snowtrooper/armor_snowtrooper_leggings.iff",
		"snowtrooper/armor_snowtrooper_helmet.iff",
		"snowtrooper/armor_snowtrooper_bicep_l.iff",
		"snowtrooper/armor_snowtrooper_bicep_r.iff",
		"snowtrooper/armor_snowtrooper_bracer_l.iff",
		"snowtrooper/armor_snowtrooper_bracer_r.iff",
		"snowtrooper/armor_snowtrooper_boots.iff",
		"snowtrooper/armor_snowtrooper_gloves.iff",
		"snowtrooper/armor_snowtrooper_belt.iff"
	};
	public static final String[] ARMOR_SET_RECON_REBEL =
	{
		"marine/armor_marine_chest_plate_rebel.iff",
		"marine/armor_marine_leggings.iff",
		"marine/armor_marine_helmet.iff",
		"marine/armor_marine_bicep_l.iff",
		"marine/armor_marine_bicep_r.iff",
		"marine/armor_marine_bracer_l.iff",
		"marine/armor_marine_bracer_r.iff",
		"marine/armor_marine_boots.iff",
		"marine/armor_marine_gloves.iff"
	};
	
	public static final String[] ARMOR_SET_REBEL_SNOW =
	{
		"rebel_snow/armor_rebel_snow_chest_plate.iff",
		"rebel_snow/armor_rebel_snow_leggings.iff",
		"rebel_snow/armor_rebel_snow_helmet.iff",
		"rebel_snow/armor_rebel_snow_bicep_l.iff",
		"rebel_snow/armor_rebel_snow_bicep_r.iff",
		"rebel_snow/armor_rebel_snow_bracer_l.iff",
		"rebel_snow/armor_rebel_snow_bracer_r.iff",
		"rebel_snow/armor_rebel_snow_boots.iff",
		"rebel_snow/armor_rebel_snow_gloves.iff",
		"rebel_snow/armor_rebel_snow_belt.iff"
	};
	
	public static final String[] ARMOR_SET_PVP_IMPERIAL_BLACK =
	{
		"armor_pvp_spec_ops_imperial_black_bicep_l_05_01",
		"armor_pvp_spec_ops_imperial_black_bicep_r_05_01",
		"armor_pvp_spec_ops_imperial_black_boots_05_01",
		"armor_pvp_spec_ops_imperial_black_bracer_l_05_01",
		"armor_pvp_spec_ops_imperial_black_bracer_r_05_01",
		"armor_pvp_spec_ops_imperial_black_chest_plate_orange_pad_05_01",
		"armor_pvp_spec_ops_imperial_black_chest_plate_red_pad_05_01",
		"armor_pvp_spec_ops_imperial_black_chest_plate_yellow_pad_05_01",
		"armor_pvp_spec_ops_imperial_black_chest_plate_blue_pad_05_01",
		"armor_pvp_spec_ops_imperial_black_chest_plate_white_pad_05_01",
		"armor_pvp_spec_ops_imperial_black_chest_plate_black_pad_05_01",
		"armor_pvp_spec_ops_imperial_black_gloves_05_01",
		"armor_pvp_spec_ops_imperial_black_helmet_05_01",
		"armor_pvp_spec_ops_imperial_black_leggings_05_01"
	};
	
	public static final String[] ARMOR_SET_PVP_IMPERIAL_WHITE =
	{
		"armor_pvp_spec_ops_imperial_white_bicep_l_05_01",
		"armor_pvp_spec_ops_imperial_white_bicep_r_05_01",
		"armor_pvp_spec_ops_imperial_white_boots_05_01",
		"armor_pvp_spec_ops_imperial_white_bracer_l_05_01",
		"armor_pvp_spec_ops_imperial_white_bracer_r_05_01",
		"armor_pvp_spec_ops_imperial_white_chest_plate_orange_pad_05_01",
		"armor_pvp_spec_ops_imperial_white_chest_plate_red_pad_05_01",
		"armor_pvp_spec_ops_imperial_white_chest_plate_yellow_pad_05_01",
		"armor_pvp_spec_ops_imperial_white_chest_plate_blue_pad_05_01",
		"armor_pvp_spec_ops_imperial_white_chest_plate_white_pad_05_01",
		"armor_pvp_spec_ops_imperial_white_chest_plate_black_pad_05_01",
		"armor_pvp_spec_ops_imperial_white_gloves_05_01",
		"armor_pvp_spec_ops_imperial_white_helmet_05_01",
		"armor_pvp_spec_ops_imperial_white_leggings_05_01"
	};
	
	public static final String[] ARMOR_SET_PVP_REBEL_GREY =
	{
		"armor_pvp_spec_ops_rebel_black_grey_bicep_l_05_01",
		"armor_pvp_spec_ops_rebel_black_grey_bicep_r_05_01",
		"armor_pvp_spec_ops_rebel_black_grey_boots_05_01",
		"armor_pvp_spec_ops_rebel_black_grey_bracer_l_05_01",
		"armor_pvp_spec_ops_rebel_black_grey_bracer_r_05_01",
		"armor_pvp_spec_ops_rebel_black_grey_gloves_05_01",
		"armor_pvp_spec_ops_rebel_black_grey_helmet_05_01",
		"armor_pvp_spec_ops_rebel_black_grey_leggings_05_01",
		"armor_pvp_spec_ops_rebel_black_grey_chest_plate_05_01",
		"armor_pvp_spec_ops_rebel_black_black_chest_plate_05_01"
	};
	
	public static final String[] ARMOR_SET_PVP_REBEL_GREEN =
	{
		"armor_pvp_spec_ops_rebel_black_green_bicep_l_05_01",
		"armor_pvp_spec_ops_rebel_black_green_bicep_r_05_01",
		"armor_pvp_spec_ops_rebel_black_green_boots_05_01",
		"armor_pvp_spec_ops_rebel_black_green_bracer_l_05_01",
		"armor_pvp_spec_ops_rebel_black_green_bracer_r_05_01",
		"armor_pvp_spec_ops_rebel_black_green_gloves_05_01",
		"armor_pvp_spec_ops_rebel_black_green_helmet_05_01",
		"armor_pvp_spec_ops_rebel_black_green_leggings_05_01",
		"armor_pvp_spec_ops_rebel_black_green_chest_plate_05_01",
		"armor_pvp_spec_ops_rebel_black_black_chest_plate_05_01"
	};
	
	public static final String[] ARMOR_SET_ASSUALT_GALACTIC_MARINE =
	{
		"armor_galactic_marine_bicep_l",
		"armor_galactic_marine_bicep_r",
		"armor_galactic_marine_boots",
		"armor_galactic_marine_bracer_l",
		"armor_galactic_marine_bracer_r",
		"armor_galactic_marine_chest_plate",
		"armor_galactic_marine_gloves",
		"armor_galactic_marine_helmet",
		"armor_galactic_marine_leggings",
		"armor_galactic_marine_belt"
	};
	
	public static final String[] ARMOR_SET_ASSUALT_REBEL_SPECFORCE =
	{
		"armor_rebel_spec_force_bicep_l",
		"armor_rebel_spec_force_bicep_r",
		"armor_rebel_spec_force_boots",
		"armor_rebel_spec_force_bracer_l",
		"armor_rebel_spec_force_bracer_r",
		"armor_rebel_spec_force_chest_plate",
		"armor_rebel_spec_force_gloves",
		"armor_rebel_spec_force_helmet",
		"armor_rebel_spec_force_leggings",
		"armor_rebel_spec_force_belt"
	};
	
	public static final String[][] ARMOR_SETS_ASSAULT =
	{
		ARMOR_SET_ASSAULT_1,
		ARMOR_SET_ASSAULT_2,
		ARMOR_SET_ASSAULT_3
	};
	public static final String[][] ARMOR_SETS_BATTLE =
	{
		ARMOR_SET_BATTLE_1,
		ARMOR_SET_BATTLE_2,
		ARMOR_SET_BATTLE_3,
		ARMOR_SET_BATTLE_4
	};
	public static final String[][] ARMOR_SETS_RECON =
	{
		ARMOR_SET_RECON_1,
		ARMOR_SET_RECON_2,
		ARMOR_SET_RECON_3,
		ARMOR_SET_RECON_4
	};
	
	public static final String[] CYBERNETIC_ITEMS =
	{
		"object/tangible/wearables/cybernetic/s02/cybernetic_s02_arm_l.iff",
		"object/tangible/wearables/cybernetic/s02/cybernetic_s02_arm_r.iff",
		"object/tangible/wearables/cybernetic/s02/cybernetic_s02_legs.iff",
		"object/tangible/wearables/cybernetic/s03/cybernetic_s03_arm_l.iff",
		"object/tangible/wearables/cybernetic/s03/cybernetic_s03_arm_r.iff",
		"object/tangible/wearables/cybernetic/s05/cybernetic_s05_arm_l.iff",
		"object/tangible/wearables/cybernetic/s05/cybernetic_s05_arm_r.iff",
		"object/tangible/wearables/cybernetic/s05/cybernetic_s05_legs.iff"
	};
	
	public static final String[] PUB27_HEAVYPACK =
	{
		"object/weapon/ranged/heavy/ep3_loot_void.iff",
		"object/weapon/ranged/heavy/heavy_rocket_launcher.iff",
		"weapon_tow_heavy_rocket_launcher_05_01",
		"object/weapon/ranged/rifle/rifle_odararissl.iff",
		"object/weapon/ranged/rifle/rifle_proton.iff",
		"object/weapon/ranged/heavy/som_lava_cannon.iff",
		"weapon_tow_cannon_03_02",
		"object/weapon/ranged/heavy/som_republic_flamer.iff",
		"weapon_tow_flamer_01_01",
		"weapon_publish_gift_27_04_01",
		"object/weapon/ranged/pistol/pistol_launcher.iff",
		"object/weapon/ranged/pistol/pistol_launcher_elite.iff",
		"object/weapon/ranged/pistol/pistol_launcher_medium.iff",
		"object/weapon/ranged/pistol/quest_pistol_launcher.iff",
		"object/weapon/ranged/rifle/rifle_acid_beam.iff",
		"object/weapon/ranged/rifle/rifle_flame_thrower.iff",
		"object/weapon/ranged/rifle/rifle_flame_thrower_light.iff"
	};
	
	public static final String[] PUB27_TRAPS =
	{
		"prx_trapCaltrop",
		"tmr_trapCaltrop",
		"rmt_trapCaltrop",
		"prx_trapFlashbang",
		"tmr_trapFlashbang",
		"rmt_trapFlashbang",
		"prx_trapHX2",
		"tmr_trapHX2",
		"rmt_trapHX2",
		"prx_trapKamino",
		"tmr_trapKamino",
		"rmt_trapKamino"
	};
	
	public static final String[] PUB27_CAMOSTUFF =
	{
		"item_clothing_spy_stealth_shirt_02_01",
		"item_clothing_spy_stealth_pants_02_01",
		"item_clothing_spy_stealth_duster_02_01",
		"item_clothing_spy_stealth_gloves_02_01",
		"item_clothing_spy_stealth_boots_02_01"
	};
	
	public static final String[] SPACE_LOOT_CATEGORIES = 
	{
		"armor",
		"booster",
		"droid_interface",
		"engine",
		"reactor",
		"shield_generator",
		"weapon",
		"weapon_capacitor"
	};
	
	public static final String[] CRAFTING_PROFESSIONS =
	{
		"Domestics",
		"Engineer",
		"Structure",
		"Weaponsmith"
	};
	
	public static final String[] CRAFTING_SKILL_TEMPLATES =
	{
		"trader_1a",
		"trader_1d",
		"trader_1b",
		"trader_1c"
	};
	
	public static final String[] COLLECTION_COMPONENT_SCHEMS =
	{
		"Collection Boosters",
		"Collection Capacitors",
		"Collection Engines",
		"Collection Reactors",
		"Nova Orion Engines"
	};
	
	public static final String[] BASIC_MOD_STRINGS =
	{
		"precision_modified",
		"strength_modified",
		"agility_modified",
		"stamina_modified",
		"constitution_modified",
		"luck_modified",
		"camouflage",
		"combat_block_value"
	};
	
	public static final String[] BASIC_MOD_LIST =
	{
		"Precision",
		"Strength",
		"Agility",
		"Stamina",
		"Constitution",
		"Luck",
		"Camouflage",
		"Block Value"
	};
	
	public static final String[] HEROIC_JEWELRY_LIST =
	{
		"Bounty Hunter Enforcer (DPS)",
		"Bounty Hunter Flawless (Utility A)",
		"Bounty Hunter Dire Fate (PvP)",
		"Medic Striker's (DPS)",
		"Medic First Responder's (Healing)",
		"Medic Blackbar's Doom (PvP)",
		"Jedi Duelist (Saber DPS)",
		"Jedi Dark Fury (Force Power DPS)",
		"Jedi Guardian (Tanking)",
		"Commando Grenadier (Grenade DPS)",
		"Commando Frontman (Tanking)",
		"Commando Juggernaut (Weapon DPS)",
		"Smuggler Scoundrel (DPS)",
		"Smuggler Rogue (PvP)",
		"Smuggler Gambler's (PvE)",
		"Spy Assassin's (DPS)",
		"Spy Ghost (PvP)",
		"Spy Razor Cat (DPS)",
		"Officer Dead Eye (DPS)",
		"Officer Hellstorm (AoE DPS)",
		"Officer General's (Group PvE)",
		"Heroism (Stats)"
	};
	public static final String[] AURILIA_CRYSTALS =
	{
		"Small Aurilian Crystal",
		"Medium Aurilian Crystal",
		"Large Aurilian Crystal"
	};
	
	public static final String[] CRAFTING_SUIT =
	{
		"Blix's Ultra Crafting Suit"
	};
	
	
	public boolean checkConfigSetting(String configString) throws InterruptedException
	{
		String enabled = toLower(getConfigSetting("CharacterBuilder", configString));
		if (enabled == null)
		{
			return false;
		}
		
		if (enabled.equals("true") || enabled.equals("1"))
		{
			return true;
		}
		
		return false;
	}
	
	
	public int OnObjectMenuRequest(obj_id self, obj_id player, menu_info mi) throws InterruptedException
	{
		menu_info_data data = mi.getMenuItemByType (menu_info_types.ITEM_USE);
		
		if (data != null)
		{
			data.setServerNotify (true);
		}
		else
		{
			mi.addRootMenu (menu_info_types.ITEM_USE, new string_id("",""));
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuSelect(obj_id self, obj_id player, int item) throws InterruptedException
	{
		if (isGod(player) || checkConfigSetting("builderEnabled"))
		{
			
			if (item == menu_info_types.ITEM_USE)
			{
				startCharacterBuilder(player);
			}
			return SCRIPT_CONTINUE;
		}
		else
		{
			sendSystemMessageTestingOnly(player, "Only authorized users may access this terminal.");
			return SCRIPT_CONTINUE;
		}
		
	}
	
	
	public void startCharacterBuilder(obj_id player) throws InterruptedException
	{
		obj_id self = getSelf();
		
		String prompt = "Select the desired character option";
		String title = "Character Builder Terminal";
		
		closeOldWindow(player);
		
		int pid = sui.listbox(self, player, prompt, sui.OK_CANCEL, title, CHARACTER_BUILDER_OPTIONS, "handleOptionSelect", true, false);
		setWindowPid(player, pid);
		
	}
	
	
	public int handleOptionSelect(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		if (idx == -1 || idx > CHARACTER_BUILDER_OPTIONS.length)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		switch(idx)
		{
			
			case 0:  
			if (isGod(player) || checkConfigSetting("weaponsEnabled"))
			{
				handleWeaponOption(player);
			}
			else
			{
				sendSystemMessageTestingOnly(player, "This Option is Currently Disabled.");
				return SCRIPT_CONTINUE;
			}
			break;
			case 1:  
			if (isGod(player) || checkConfigSetting("armorEnabled"))
			{
				handleArmorOption(player);
			}
			else
			{
				sendSystemMessageTestingOnly(player, "This Option is Currently Disabled.");
				return SCRIPT_CONTINUE;
			}
			break;
			case 2:  
			if (isGod(player) || checkConfigSetting("skillsEnabled"))
			{
				handleRoadmapSkills(player);
			}
			else
			{
				sendSystemMessageTestingOnly(player, "The Roadmap Skill Builder option is DISABLED.");
				return SCRIPT_CONTINUE;
			}
			break;
			
			case 3:  
			if (isGod(player) || checkConfigSetting("resourcesEnabled"))
			{
				handleResourceOption(player);
			}
			else
			{
				sendSystemMessageTestingOnly(player, "This Option is Currently Disabled.");
				return SCRIPT_CONTINUE;
			}
			break;
			case 4:  
			if (isGod(player) || checkConfigSetting("creditsEnabled"))
			{
				handleCreditOption(player);
			}
			else
			{
				sendSystemMessageTestingOnly(player, "This Option is Currently Disabled.");
				return SCRIPT_CONTINUE;
			}
			break;
			
			case 5:  
			if (isGod(player) || checkConfigSetting("factionEnabled"))
			{
				handleFactionOption(player);
			}
			else
			{
				sendSystemMessageTestingOnly(player, "This Option is Currently Disabled.");
				return SCRIPT_CONTINUE;
			}
			break;
			case 6:  
			handleVehicleOption(player);
			break;
			case 7:  
			if (isGod(player) || checkConfigSetting("shipsEnabled"))
			{
				handleShipMenuSelect(player);
			}
			else
			{
				sendSystemMessageTestingOnly(player, "This Option is Currently Disabled.");
				return SCRIPT_CONTINUE;
			}
			break;
			case 8:  
			if (isGod(player) || checkConfigSetting("craftingEnabled"))
			{
				handleCraftingOption(player);
			}
			else
			{
				sendSystemMessageTestingOnly(player, "This Option is Currently Disabled.");
				return SCRIPT_CONTINUE;
			}
			break;
			case 9:  
			if (isGod(player) || checkConfigSetting("deedsEnabled"))
			{
				handleDeedOption(player);
			}
			else
			{
				sendSystemMessageTestingOnly(player, "This Option is Currently Disabled.");
				return SCRIPT_CONTINUE;
			}
			break;
			case 10:  
			if (isGod(player) || checkConfigSetting("pahallEnabled"))
			{
				handlePAOption(player);
			}
			else
			{
				sendSystemMessageTestingOnly(player, "This Option is Currently Disabled.");
				return SCRIPT_CONTINUE;
			}
			break;
			case 11:  
			if (isGod(player) || checkConfigSetting("miscitemEnabled"))
			{
				handleMiscOption(player);
			}
			else
			{
				sendSystemMessageTestingOnly(player, "This Option is Currently Disabled.");
				return SCRIPT_CONTINUE;
			}
			break;
			case 12:  
			if (isGod(player) || checkConfigSetting("jediEnabled"))
			{
				handleJediOption(player);
			}
			else
			{
				sendSystemMessageTestingOnly(player, "You Are Not Authorized to use the Jedi Options!.");
				return SCRIPT_CONTINUE;
			}
			break;
			case 13:  
			if (isGod(player))
			{
				handleBestResourceOption(player);
			}
			else
			{
				sendSystemMessageTestingOnly(player, "This is for developers only.");
				return SCRIPT_CONTINUE;
			}
			break;
			
			case 14:  
			if (isGod(player))
			{
				flagAllHeroicInstances(player);
			}
			else
			{
				sendSystemMessageTestingOnly(player, "This is for developers only.");
				return SCRIPT_CONTINUE;
			}
			break;
			case 15: 
			if (isGod(player))
			{
				handleDraftSchematicsOption(player);
			}
			else
			{
				sendSystemMessageTestingOnly(player, "This is for developers only.");
				return SCRIPT_CONTINUE;
			}
			break;
			
			default:
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void cleanScriptVars(obj_id player) throws InterruptedException
	{
		obj_id self = getSelf();
		
		utils.removeScriptVarTree(player, "character_builder");
		utils.removeScriptVarTree(self, "character_builder");
		
		setObjVar(player, "character_builder", true);
	}
	
	
	public void closeOldWindow(obj_id player) throws InterruptedException
	{
		String playerPath = "character_builder.";
		if (utils.hasScriptVar(player, "character_builder.pid"))
		{
			int oldpid = utils.getIntScriptVar(player, "character_builder.pid");
			forceCloseSUIPage (oldpid);
			utils.removeScriptVar(player, "character_builder.pid");
		}
	}
	
	
	public void setWindowPid(obj_id player, int pid) throws InterruptedException
	{
		if (pid > -1)
		{
			utils.setScriptVar(player, "character_builder.pid", pid);
		}
	}
	
	
	public String[] convertSkillListNames(String[] skillList) throws InterruptedException
	{
		for (int i = 0; i < skillList.length; i++)
		{
			testAbortScript();
			skillList[i] = "@skl_n:" + skillList[i];
		}
		
		return skillList;
	}
	
	
	public void handleSkillLoadoutOption(obj_id player) throws InterruptedException
	{
		
		if (!isIdValid(player))
		{
			return;
		}
		obj_id inventoryId = utils.getInventoryContainer(player);
		if (!isIdValid(inventoryId))
		{
			return;
		}
		obj_id[] inventoryArray = getInventoryAndEquipment(player);
		if (inventoryArray == null)
		{
			inventoryArray = new obj_id[0];
		}
		java.util.HashSet inventoryLookup = new java.util.HashSet(inventoryArray.length > 8 ? inventoryArray.length * 2 : 16);
		for (int i = 0; i < inventoryArray.length; ++i)
		{
			testAbortScript();
			String itemTemplate = getTemplateName(inventoryArray[i]);
			if (itemTemplate != null)
			{
				inventoryLookup.add(itemTemplate);
			}
		}
		
		int itemCount = 0;
		int shipCount = 0;
		String[] skills = getSkillListingForPlayer(player);
		if (skills != null)
		{
			for (int i = 0; i < skills.length; ++i)
			{
				testAbortScript();
				if (space_transition.isPlayerBelowShipLimit(player))
				{
					obj_id shipId = null;
					if (skills[i].equals("pilot_rebel_navy_master"))
					{
						shipId = space_utils.createShipControlDevice(player, "advanced_xwing", false);
					}
					else if (skills[i].equals("pilot_imperial_navy_master"))
					{
						shipId = space_utils.createShipControlDevice(player, "tieinterceptor_imperial_guard", false);
					}
					else if (skills[i].equals("pilot_neutral_master"))
					{
						shipId = space_utils.createShipControlDevice(player, "yt1300", false);
					}
					else if (skills[i].equals("pilot_rebel_navy_novice"))
					{
						shipId = space_utils.createShipControlDevice(player, "z95", false);
					}
					else if (skills[i].equals("pilot_imperial_navy_novice"))
					{
						shipId = space_utils.createShipControlDevice(player, "tie_light_duty", false);
					}
					else if (skills[i].equals("pilot_neutral_novice"))
					{
						shipId = space_utils.createShipControlDevice(player, "basic_hutt_light", false);
					}
					if (isValidId(shipId))
					{
						++shipCount;
					}
				}
				
				dictionary items = dataTableGetRow(SKILL_LOADOUT_TBL, skills[i]);
				if (items != null)
				{
					
					java.util.Enumeration keys = items.keys();
					while (keys.hasMoreElements())
					{
						testAbortScript();
						String key = (String)(keys.nextElement());
						if (key.equals("skill"))
						{
							continue;
						}
						
						Object value = items.get(key);
						if (value != null && value instanceof String && ((String)value).length() > 0)
						{
							if (key.equals("armor"))
							{
								
								String armorSetName = (String)(value);
								String armorCategoryName = armorSetName.substring(0, armorSetName.length()-1);
								String armorLevelName = armorSetName.substring(armorSetName.length()-1, armorSetName.length());
								if ((!armorCategoryName.equals("assault") && !armorCategoryName.equals("battle") && !armorCategoryName.equals("recon")))
								{
									sendSystemMessageTestingOnly(player, "Invalid armor category "+ armorCategoryName +
									" for skill entry "+ skills[i]);
									continue;
								}
								if ((!armorLevelName.equals("1") && !armorLevelName.equals("2") && !armorLevelName.equals("3")))
								{
									sendSystemMessageTestingOnly(player, "Invalid armor level "+ armorLevelName +
									" for skill entry "+ skills[i]);
									continue;
								}
								int armorCategory = AC_none;
								int armorLevel = Integer.parseInt(armorLevelName) - 1;
								int playerSpecies = getSpecies(player);
								String[] armorSet = null;
								if (armorCategoryName.equals("assault"))
								{
									armorCategory = AC_assault;
									if (playerSpecies == SPECIES_WOOKIEE)
									{
										armorSet = ARMOR_SET_ASSAULT_WOOKIEE;
									}
									else if (playerSpecies == SPECIES_ITHORIAN)
									{
										armorSet = ARMOR_SET_ASSAULT_ITHORIAN;
									}
									else
									{
										armorSet = ARMOR_SETS_ASSAULT[rand(0, ARMOR_SETS_ASSAULT.length-1)];
									}
								}
								else if (armorCategoryName.equals("battle"))
								{
									armorCategory = AC_battle;
									if (playerSpecies == SPECIES_WOOKIEE)
									{
										armorSet = ARMOR_SET_BATTLE_WOOKIEE;
									}
									else if (playerSpecies == SPECIES_ITHORIAN)
									{
										armorSet = ARMOR_SET_BATTLE_ITHORIAN;
									}
									else
									{
										armorSet = ARMOR_SETS_BATTLE[rand(0, ARMOR_SETS_BATTLE.length-1)];
									}
								}
								else
								{
									armorCategory = AC_reconnaissance;
									if (playerSpecies == SPECIES_WOOKIEE)
									{
										armorSet = ARMOR_SET_RECON_WOOKIEE;
									}
									else if (playerSpecies == SPECIES_ITHORIAN)
									{
										armorSet = ARMOR_SET_RECON_ITHORIAN;
									}
									else
									{
										armorSet = ARMOR_SETS_RECON[rand(0, ARMOR_SETS_RECON.length-1)];
									}
								}
								if (armorSet == null)
								{
									sendSystemMessageTestingOnly(player, "Unable to get armor set for armor category "+ armorCategoryName);
									continue;
								}
								for (int j = 0; j < armorSet.length; ++j)
								{
									testAbortScript();
									String armorTemplate = ARMOR_SET_PREFIX + armorSet[j];
									if (!inventoryLookup.contains(armorTemplate))
									{
										obj_id newItem = createObject(armorTemplate, inventoryId, "");
										if (isIdValid(newItem))
										{
											
											if (!isGameObjectTypeOf(newItem, GOT_armor_foot) && !isGameObjectTypeOf(newItem, GOT_armor_hand))
											{
												armor.setArmorDataPercent(newItem, armorLevel, armorCategory, GENERAL_PROTECTION, CONDITION);
											}
											inventoryLookup.add(armorTemplate);
											++itemCount;
										}
									}
								}
							}
							else
							{
								if (!inventoryLookup.contains(value))
								{
									String itemTemplate = (String)(value);
									obj_id newItem = null;
									int itemGot = getGameObjectTypeFromTemplate(itemTemplate);
									if (isGameObjectTypeOf(itemGot, GOT_weapon))
									{
										newItem = weapons.createWeapon(itemTemplate, inventoryId, weapons.VIA_TEMPLATE, WEAPON_SPEED, WEAPON_DAMAGE, WEAPON_EFFECIENCY, WEAPON_ELEMENTAL);
									}
									else
									{
										newItem = createObject(itemTemplate, inventoryId, "");
									}
									if (isIdValid(newItem))
									{
										inventoryLookup.add(itemTemplate);
										++itemCount;
									}
								}
							}
						}
					}
				}
			}
		}
		
		if (itemCount > 0 || shipCount > 0)
		{
			if (shipCount > 0)
			{
				sendSystemMessageTestingOnly(player, shipCount + " ships were placed in your datapad.");
			}
			if (itemCount > 0)
			{
				sendSystemMessageTestingOnly(player, itemCount + " items were placed in your inventory.");
			}
			sendSystemMessageTestingOnly(player, "Have fun storming the castle!");
		}
		else
		{
			sendSystemMessageTestingOnly(player, "No new items were placed in your inventory. You've got everything you need.");
		}
		
		startCharacterBuilder(player);
	}
	
	
	public void handleCreditOption(obj_id player) throws InterruptedException
	{
		
		if (getCashBalance(player) < 100000000)
		{
			dictionary d = new dictionary();
			d.put("payoutTarget", player);
			money.systemPayout(money.ACCT_BETA_TEST, player, CASH_AMOUNT, "handlePayoutToPlayer", d);
		}
		else
		{
			sendSystemMessageTestingOnly(player, "You already have 100,000,000+ credits. Why do you need any more money?");
		}
		startCharacterBuilder(player);
		
	}
	
	
	public int handlePayoutToPlayer(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id player = params.getObjId("payoutTarget");
		if (!isIdValid(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		int retCode = params.getInt(money.DICT_CODE);
		if (retCode == money.RET_SUCCESS)
		{
			String terminal = "Character Builder Terminal";
			sendSystemMessageTestingOnly(player, "You receive "+ CASH_AMOUNT + " credits from the "+ terminal);
		}
		else if (retCode == money.RET_FAIL)
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void handleBuffOption(obj_id player) throws InterruptedException
	{
		buff.applyBuff(player, "frogBuff");
		sendSystemMessageTestingOnly(player, "Buffs applied!");
		startCharacterBuilder(player);
	}
	
	
	public void handleResourceOption(obj_id player) throws InterruptedException
	{
		obj_id self = getSelf();
		
		refreshMenu ( player, "Select the desired resource category", "Character Builder Terminal", RESOURCE_TYPES, "handleCategorySelection", false);
		
		return;
	}
	
	
	public int handleCategorySelection(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		if (btn == sui.BP_REVERT)
		{
			startCharacterBuilder(player);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		if (idx == -1)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		location loc = getLocation(player);
		String planet = "current";
		
		String[] resourceList = buildAvailableResourceTree(self, loc, RESOURCE_BASE_TYPES[idx]);
		
		refreshMenu ( player, "Select the desired resource category", "Character Builder Terminal", resourceList, "handleResourceSelection", false);
		if (resourceList[0].startsWith("@resource/resource_names"))
		{
			utils.setScriptVar(player, "character_builder.resourceList", SPACE_RESOURCE_CONST);
		}
		else
		{
			utils.setScriptVar(player, "character_builder.resourceList", resourceList);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleResourceSelection(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		if (btn == sui.BP_REVERT)
		{
			refreshMenu ( player, "Select the desired resource category", "Character Builder Terminal", RESOURCE_TYPES, "handleCategorySelection", false);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		if (idx == -1)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		String[] resourceList = utils.getStringArrayScriptVar(player, "character_builder.resourceList");
		
		String resourceName = resourceList[idx].trim();
		if (resourceName.startsWith("\\#"))
		{
			resourceName = resourceName.substring(13, resourceName.length()-3);
		}
		
		if (resourceName.startsWith("space"))
		{
			makeSpaceResource(player, resourceName);
			return SCRIPT_CONTINUE;
		}
		
		obj_id resourceId = getResourceTypeByName(resourceName);
		
		if (isIdNull(resourceId))
		{
			resourceId = pickRandomNonDepeletedResource(resourceName);
		}
		
		obj_id inv = utils.getInventoryContainer(player);
		obj_id resourceCrate = createResourceCrate(resourceId, AMT, inv);
		
		if (isIdNull(resourceCrate))
		{
			sendSystemMessageTestingOnly(player, "Resource grant failed. It is likely your inventory is full.");
		}
		else
		{
			resourceName = " \\#pcontrast1 "+getResourceName(resourceId)+"\\#. a type of "+ getClassString(getResourceClass(resourceId));
			prose_package proseSuccess = prose.getPackage(resource.SID_SAMPLE_LOCATED, resourceName, AMT);
			sendSystemMessageProse(player, proseSuccess);
			
		}
		
		refreshMenu ( player, "Select the desired resource category", "Character Builder Terminal", resourceList, "handleResourceSelection", false);
		return SCRIPT_CONTINUE;
	}
	
	
	public void makeSpaceResource(obj_id self, String rclass) throws InterruptedException
	{
		obj_id[] rtypes = getResourceTypes(rclass);
		sendSystemMessageTestingOnly(self, "Types are..."+ rtypes[0].toString());
		obj_id rtype = rtypes[0];
		if (!isIdValid(rtype))
		{
			sendSystemMessageTestingOnly(self, "No id found");
			sendSystemMessageTestingOnly(self, "Type was "+rclass);
			return;
		}
		
		String crateTemplate = getResourceContainerForType(rtype);
		
		if (!crateTemplate.equals(""))
		{
			obj_id pInv = utils.getInventoryContainer(self);
			if (isIdValid(pInv))
			{
				obj_id crate = createObject(crateTemplate, pInv, "");
				if (addResourceToContainer (crate, rtype, AMT, self))
				{
					sendSystemMessageTestingOnly(self, "Resource of class "+rclass+" added");
				}
			}
		}
	}
	
	
	public String[] buildAvailableResourceTree(obj_id self, location loc, String topParent) throws InterruptedException
	{
		Vector allResources = new Vector();
		allResources.setSize(0);
		
		if (topParent.equals("Asteroid"))
		{
			return SPACE_RESOURCE_LOCALIZED;
			
		}
		
		if (hasObjVar(self, "allPlanetResources"))
		{
			String[] planetNames =
			{
				"corellia", "dantooine", "lok", "naboo", "rori", "talus", "tatooine"
			};
			
			for (int i = 0; i < planetNames.length; i++)
			{
				testAbortScript();
				loc.area = planetNames[i];
				
				resource_density[] resources = requestResourceList(loc, 0.0f, 1.0f, topParent);
				
				for (int j = 0; j < resources.length; j++)
				{
					testAbortScript();
					allResources.add(resources[j]);
				}
			}
		}
		else
		{
			resource_density[] resources = requestResourceList(loc, 0.0f, 1.0f, topParent);
			
			for (int j = 0; j < resources.length; j++)
			{
				testAbortScript();
				allResources.add(resources[j]);
			}
		}
		
		String[] resourceTree = buildSortedResourceTree(allResources, topParent, 0);
		
		return resourceTree;
	}
	
	
	public String[] buildSortedResourceTree(Vector resources, String topParent, int branchLevel) throws InterruptedException
	{
		Vector resourceTree = new Vector();
		resourceTree.setSize(0);
		
		if (resources != null)
		{
			for (int i = 0; i < resources.size(); i++)
			{
				testAbortScript();
				if (!isResourceDerivedFrom(((resource_density)(resources.get(i))).getResourceType(), topParent))
				{
					continue;
				}
				
				String parent = getResourceClass(((resource_density)(resources.get(i))).getResourceType());
				String child = null;
				
				if (parent == null)
				{
					continue;
				}
				
				while (!parent.equals(topParent))
				{
					testAbortScript();
					child = parent;
					parent = getResourceParentClass(child);
				}
				
				if (child == null)
				{
					child = "\\#pcontrast1 "+getResourceName(((resource_density)(resources.get(i))).getResourceType())+"\\#.";
				}
				
				for (int j = 0; j < branchLevel; j++)
				{
					testAbortScript();
					child = " "+ child;
				}
				
				if (resourceTree.indexOf(child) == -1)
				{
					resourceTree.add(child);
				}
			}
		}
		
		for (int i = 0; i < resourceTree.size(); i++)
		{
			testAbortScript();
			String parent = ((String)(resourceTree.get(i))).trim();
			String[] childBranch = buildSortedResourceTree(resources, parent, branchLevel+1);
			
			for (int j = 0; j < childBranch.length; j++)
			{
				testAbortScript();
				resourceTree.add(++i, childBranch[j]);
			}
		}
		
		return (String[])resourceTree.toArray(new String[0]);
	}
	
	
	public String getClassString(String className) throws InterruptedException
	{
		final String resourceTable = "datatables/resource/resource_tree.iff";
		
		String classString = "";
		
		int row = dataTableSearchColumnForString(className, 1, resourceTable);
		int column = 2;
		while ((classString == null || classString.length() == 0) && column <= 9)
		{
			testAbortScript();
			classString = dataTableGetString(resourceTable, row, column);
			column++;
		}
		
		return classString;
	}
	
	
	public void handleBestResourceOption(obj_id player) throws InterruptedException
	{
		obj_id self = getSelf();
		
		refreshMenu(player, "Select the desired resource category", "Character Builder Terminal", BEST_RESOURCE_TYPES, "handleBestCategorySelection", false);
		
		return;
	}
	
	
	public int handleBestCategorySelection(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		if (btn == sui.BP_REVERT)
		{
			startCharacterBuilder(player);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		if (idx == -1)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		if (idx > RESOURCE_BASE_TYPES.length - 1)
		{
			utils.setScriptVar(player, "character_builder.specificFilter", -1);
			refreshMenu(player, "Select the desired resource category", "Character Builder Terminal", RESOURCE_TYPES, "handleBestCategorySelection", false);
			return SCRIPT_CONTINUE;
		}
		
		location loc = getLocation(player);
		String planet = "current";
		
		String[] resourceList = getResourceChildClasses(RESOURCE_BASE_TYPES[idx]);
		
		int goodResources = 0;
		
		for (int i = 0; i < resourceList.length; ++i)
		{
			testAbortScript();
			if (!hasResourceType(resourceList[i]))
			{
				resourceList[i] = null;
			}
			else
			{
				++goodResources;
			}
		}
		
		String[] temp = new String[goodResources];
		goodResources = 0;
		
		for (int i = 0; i < resourceList.length; ++i)
		{
			testAbortScript();
			if (resourceList[i] != null)
			{
				temp[goodResources++] = resourceList[i];
			}
		}
		
		resourceList = temp;
		temp = null;
		
		refreshMenu(player, "Select the desired resource category", "Character Builder Terminal", resourceList, "handleBestResourceSelection", false);
		utils.setScriptVar(player, "character_builder.resourceList", resourceList);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleBestResourceSelection(obj_id self, dictionary params) throws InterruptedException
	{
		if (params == null || params.isEmpty())
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		if (btn == sui.BP_REVERT)
		{
			startCharacterBuilder(player);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		if (idx == -1)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		String[] resourceList = utils.getStringArrayScriptVar(player, "character_builder.resourceList");
		
		if (utils.hasScriptVar(player, "character_builder.specificFilter"))
		{
			String[] attribs = craftinglib.getAttribNamesByResourceClass(resourceList[idx]);
			
			if (attribs == null)
			{
				debugSpeakMsg(player, "attribs null");
				return SCRIPT_CONTINUE;
			}
			
			utils.setScriptVar(player, "character_builder.resourceIndex", idx);
			
			refreshMenu(player, "Select the desired attribute", "Character Builder Terminal", attribs, "handleBestResourceSelectionWithAttribute", false);
			
			return SCRIPT_CONTINUE;
		}
		
		craftinglib.makeBestResource(player, resourceList[idx], AMT);
		
		refreshMenu(player, "Select the desired resource category", "Character Builder Terminal", resourceList, "handleBestResourceSelection", false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleBestResourceSelectionWithAttribute(obj_id self, dictionary params) throws InterruptedException
	{
		if (params == null || params.isEmpty())
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		if (btn == sui.BP_REVERT)
		{
			startCharacterBuilder(player);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		if (idx == -1)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		String[] resourceList = utils.getStringArrayScriptVar(player, "character_builder.resourceList");
		int resourceListIndex = utils.getIntScriptVar(player, "character_builder.resourceIndex");
		
		String[] attribs = craftinglib.getAttribNamesByResourceClass(resourceList[resourceListIndex]);
		
		utils.removeScriptVar(player, "character_builder.specificFilter");
		utils.removeScriptVar(player, "character_builder.resourceIndex");
		
		craftinglib.makeBestResourceByAttribute(player, resourceList[resourceListIndex], attribs[idx], AMT);
		
		refreshMenu(player, "Select the desired resource category", "Character Builder Terminal", BEST_RESOURCE_TYPES, "handleBestCategorySelection", false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void handleVehicleOption(obj_id player) throws InterruptedException
	{
		obj_id self = getSelf();
		
		refreshMenu ( player, "Select the desired vehicle or mount option", "Character Builder Terminal", VEHICLE_MOUNT_OPTIONS, "handleVehicleOptions", false);
	}
	
	
	public int handleVehicleOptions(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		if (btn == sui.BP_REVERT)
		{
			startCharacterBuilder(player);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		if (idx == -1 || idx > VEHICLE_MOUNT_OPTIONS.length)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(player))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		String prompt = "Select the desired option";
		String title = "Character Builder Terminal";
		int pid = 0;
		
		switch(idx)
		{
			
			case 0:
			if (isGod(player) || checkConfigSetting("vehiclesEnabled"))
			{
				refreshMenu ( player, prompt, title, VEHICLE_OPTIONS, "handleVehicleSelect", false);
			}
			else
			{
				sendSystemMessageTestingOnly(player, "This Option is Currently Disabled.");
				return SCRIPT_CONTINUE;
			}
			
			break;
			
			case 1:
			if (isGod(player) || checkConfigSetting("vehiclesEnabled"))
			{
				refreshMenu ( player, prompt, title, MOUNT_OPTIONS, "handleMountSelect", false);
			}
			else
			{
				sendSystemMessageTestingOnly(player, "This Option is Currently Disabled.");
				return SCRIPT_CONTINUE;
			}
			break;
			
			case 2:
			if (!isGod(player))
			{
				refreshMenu(player, prompt, title, BEAST_OPTIONS_FOR_PLAYERS, "handleBeastSelect", false);
			}
			else
			{
				refreshMenu(player, prompt, title, BEAST_OPTIONS, "handleBeastSelect", false);
			}
			break;
			
			default:
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
			
		}
		
		return SCRIPT_CONTINUE;
		
	}
	
	
	public int handleVehicleSelect(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		obj_id pInv = utils.getInventoryContainer(player);
		
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		if (btn == sui.BP_REVERT)
		{
			handleVehicleOption(player);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		if (idx == -1 || idx > VEHICLE_OPTIONS.length)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(player))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (!isIdValid(pInv))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (getVolumeFree(pInv) <= 0)
		{
			sendSystemMessageTestingOnly(player, "Your Inventory is Full, please make room and try again.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		switch(idx)
		{
			case 0:  
			createObject("object/tangible/deed/vehicle_deed/speederbike_swoop_deed.iff", pInv, "");
			sendSystemMessageTestingOnly(player, "Swoop Deed Issued.");
			break;
			
			case 1:  
			createObject("object/tangible/deed/vehicle_deed/speederbike_deed.iff", pInv, "");
			sendSystemMessageTestingOnly(player, "Speederbike Deed Issued.");
			break;
			case 2:  
			createObject("object/tangible/deed/vehicle_deed/landspeeder_x34_deed.iff", pInv, "");
			sendSystemMessageTestingOnly(player, "X34 Landspeeder Deed Issued.");
			break;
			case 3: 
			createObject("object/tangible/deed/vehicle_deed/landspeeder_ab1_deed.iff",pInv, "");
			sendSystemMessageTestingOnly(player, "AB1 Deed Issued.");
			break;
			case 4: 
			createObject("object/tangible/deed/vehicle_deed/landspeeder_v35_deed.iff", pInv, "");
			sendSystemMessageTestingOnly(player, "V-35 SoroSuub Carrier Deed Issued.");
			break;
			case 5: 
			createObject("object/tangible/deed/vehicle_deed/landspeeder_xp38_deed.iff", pInv, "");
			sendSystemMessageTestingOnly(player, "XP38 Landspeeder Deed Issued.");
			break;
			case 6: 
			if (isGod(player))
			{
				createObject("object/tangible/deed/vehicle_deed/barc_speeder_deed.iff", pInv, "");
				sendSystemMessageTestingOnly(player, "Barc Speeder Deed Issued.");
			}
			else
			{
				sendSystemMessageTestingOnly(player, "You Must be in God Mode to enjoy the BARC Speeder!");
			}
			break;
			case 7: 
			if (isGod(player))
			{
				createObject("object/tangible/deed/vehicle_deed/landspeeder_av21_deed.iff", pInv, "");
				sendSystemMessageTestingOnly(player, "AV21 Deed Issued.");
			}
			else
			{
				sendSystemMessageTestingOnly(player, "You Must be in God Mode to enjoy the AV21 Speeder!");
			}
			break;
			case 8: 
			createObject("object/tangible/deed/vehicle_deed/landspeeder_x31_deed.iff", pInv, "");
			sendSystemMessageTestingOnly(player, "X31 Deed Issued.");
			break;
			case 9: 
			if (isGod(player))
			{
				createObject("object/tangible/deed/vehicle_deed/speederbike_flash_deed.iff", pInv, "");
				sendSystemMessageTestingOnly(player, "Flash Speeder Deed Issued.");
			}
			else
			{
				sendSystemMessageTestingOnly(player, "You Must be in God Mode to enjoy the Flash Speeder!");
			}
			break;
			default:
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
			
		}
		refreshMenu ( player, "Select the desired option", "Character Builder Terminal", VEHICLE_OPTIONS, "handleVehicleSelect", false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleMountSelect(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		obj_id pInv = utils.getInventoryContainer(player);
		
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		if (btn == sui.BP_REVERT)
		{
			handleVehicleOption(player);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		if (idx == -1 || idx > MOUNT_OPTIONS.length)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(player))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		switch(idx)
		{
			case 0:  
			handleMount(player, "carrion_spat");
			break;
			case 1:  
			handleMount(player, "kaadu_motley");
			break;
			case 2:  
			handleMount(player, "lesser_dewback");
			break;
			case 3:  
			handleMount(player, "bol_lesser_plains");
			break;
			case 4:  
			handleMount(player, "falumpaset_plodding");
			break;
			case 5:  
			handleMount(player, "brackaset_lowlands");
			break;
			case 6:  
			handleMount(player, "cu_pa");
			break;
			case 7:  
			handleMount(player, "dwarf_bantha");
			break;
			default:
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
			
		}
		refreshMenu ( player, "Select the desired option", "Character Builder Terminal", MOUNT_OPTIONS, "handleMountSelect", false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleBeastSelect(obj_id self, dictionary params) throws InterruptedException
	{
		if (params == null || params.isEmpty())
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		obj_id pInv = utils.getInventoryContainer(player);
		
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		if (btn == sui.BP_REVERT)
		{
			handleVehicleOption(player);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		String[] beasts = (!isGod(player) ? BEAST_OPTIONS_FOR_PLAYERS : BEAST_OPTIONS);
		
		if (idx == -1 || idx > beasts.length)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(player))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		String creatureName = "bm_"+ beasts[idx];
		obj_id inv = utils.getInventoryContainer(player);
		
		obj_id egg = createObject("object/tangible/item/beast/bm_egg.iff", inv, "");
		
		int hashCreatureType = incubator.getHashType(creatureName);
		
		incubator.setUpEggWithDummyData(player, egg, hashCreatureType);
		
		refreshMenu(player, "Select the desired option", "Character Builder Terminal", beasts, "handleBeastSelect", false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void handleMount(obj_id player, String mountType) throws InterruptedException
	{
		location spawnLoc = getLocation(player);
		spawnLoc.x += 2;
		spawnLoc.z += 2;
		
		int petType = pet_lib.getPetType(mountType);
		int level = dataTableGetInt(pet_lib.CREATURE_TABLE, mountType, "level");
		int chance = pet_lib.getChanceToTame(level, petType, player);
		
		if (chance <= 0)
		{
			
			if (petType == pet_lib.PET_TYPE_AGGRO)
			{
				
				sendSystemMessage(player, pet_lib.SID_SYS_LACK_SKILL);
				return;
			}
			else
			{
				if (level > 10)
				{
					
					sendSystemMessage(player, pet_lib.SID_SYS_LACK_SKILL);
					return;
				}
			}
		}
		
		obj_id mount = create.object(mountType, spawnLoc);
		
		if (!hasScript(player, "ai.pet_master"))
		{
			attachScript(player, "ai.pet_master");
		}
		
		if (!createNewMount(player, mount))
		{
			destroyObject(mount);
		}
		else
		{
			if (getMountsEnabled())
			{
				if (couldPetBeMadeMountable(mount) == 0)
				{
					if (makePetMountable(mount))
					{
						obj_id petControlDevice = callable.getCallableCD(mount);
						setObjVar(petControlDevice, "ai.pet.trainedMount", 1);
						setObjVar(petControlDevice, "ai.pet.type", pet_lib.getPetType(mount));
					}
				}
				else
				{
					sendSystemMessageTestingOnly(player, "For some reason, the creature spawned can NOT be turned into a mount. Might be out of SCALE, could be several other things.");
				}
			}
		}
	}
	
	
	public boolean createNewMount(obj_id master, obj_id pet) throws InterruptedException
	{
		String creatureName = ai_lib.getCreatureName(pet);
		
		if (creatureName == null || creatureName.equals(""))
		{
			return false;
		}
		
		int petSpecies = ai_lib.aiGetSpecies(pet);
		
		if (petSpecies == -1)
		{
			return false;
		}
		
		if (callable.hasCallable(master, callable.CALLABLE_TYPE_RIDEABLE))
		{
			return false;
		}
		
		if (!pet_lib.hasMaxStoredPetsOfType(master, pet_lib.PET_TYPE_MOUNT))
		{
			obj_id petControlDevice = null;
			
			if (!callable.hasCallableCD(pet))
			{
				obj_id datapad = utils.getPlayerDatapad(master);
				
				if (!isIdValid(datapad))
				{
					return false;
				}
				
				String controlTemplate = "object/intangible/pet/"+ utils.dataTableGetString(create.CREATURE_TABLE, creatureName, "template");
				
				if (!controlTemplate.endsWith(".iff"))
				{
					controlTemplate = pet_lib.PET_CTRL_DEVICE_TEMPLATE;
				}
				
				petControlDevice = createObject(controlTemplate, datapad, "");
				
				if (!isIdValid(petControlDevice))
				{
					petControlDevice = createObject(pet_lib.PET_CTRL_DEVICE_TEMPLATE, datapad, "");
					
					if (!isIdValid(petControlDevice))
					{
						sendSystemMessage(master, pet_lib.SID_SYS_TOO_MANY_STORED_PETS);
						return false;
					}
				}
				
				setObjVar(petControlDevice, "ai.pet.trainedMount", 1);
				
				pet_lib.setUpPetControlDevice(petControlDevice, pet);
			}
			
			petControlDevice = callable.getCallableCD(pet);
			
			callable.setCallableLinks(master, petControlDevice, pet);
			
			dictionary params = new dictionary();
			params.put("pet", pet);
			params.put("master", master);
			messageTo(pet, "handleAddMaster", params, 1, false);
			
			sendSystemMessageTestingOnly(master, creatureName + " Issued and Stored on Your Datapad.");
			
			return true;
		}
		
		sendSystemMessage( master, pet_lib.SID_SYS_TOO_MANY_STORED_PETS );
		
		return false;
	}
	
	
	public void handleShipOption(obj_id player) throws InterruptedException
	{
		obj_id self = getSelf();
		
		refreshMenu ( player, "Select the desired ship option", "Character Builder Terminal", SHIP_OPTIONS, "handleShipOptions", false);
	}
	
	
	public int handleShipOptions(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		if (btn == sui.BP_REVERT)
		{
			startCharacterBuilder(player);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		if (idx == -1 || idx > SHIP_OPTIONS.length)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(player))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		String prompt = "Select the desired option";
		String title = "Character Builder Terminal";
		int pid = 0;
		
		switch(idx)
		{
			
			case 0:
			refreshMenu ( player, prompt, title, REBEL_SHIP_OPTIONS, "handleRebelShipSelect", false);
			break;
			case 1:
			refreshMenu ( player, prompt, title, IMPERIAL_SHIP_OPTIONS, "handleImperialShipSelect", false);
			break;
			case 2:
			refreshMenu ( player, prompt, title, FREELANCE_SHIP_OPTIONS, "handleFreelanceShipSelect", false);
			break;
			case 3:
			refreshMenu ( player, prompt, title, OTHER_SHIP_OPTIONS, "handleOtherShipSelect", false);
			break;
			case 4:
			refreshMenu ( player, prompt, title, MAIN_SHIP_OPTIONS, "handlePartShipSelect", false);
			break;
			case 5:
			if (isGod(player))
			{
				refreshMenu(player, prompt, title, PILOT_SKILLS, "handlePilotSkillSelect", false);
			}
			else
			{
				sendSystemMessageTestingOnly(player, "This Option is Currently Disabled.");
				cleanScriptVars(player);
			}
			break;
			default:
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
			
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean revokePilotingSkills(obj_id player) throws InterruptedException
	{
		if (hasSkill(player, "pilot_rebel_navy_novice") || hasSkill(player, "pilot_imperial_navy_novice") || hasSkill(player, "pilot_neutral_novice"))
		{
			String pilotFaction = "";
			
			if (!utils.hasScriptVar(player, "revokePilotSkill"))
			{
				utils.setScriptVar(player, "revokePilotSkill", 1);
			}
			
			if (hasSkill(player, "pilot_rebel_navy_novice"))
			{
				pilotFaction = "rebel_navy";
			}
			else if (hasSkill(player, "pilot_imperial_navy_novice"))
			{
				pilotFaction = "imperial_navy";
			}
			else if (hasSkill(player, "pilot_neutral_novice"))
			{
				pilotFaction = "neutral";
			}
			
			if (!pilotFaction.equals(""))
			{
				for (int i = 0; i < space_skill.SKILL_NAMES.length; i++)
				{
					testAbortScript();
					
					skill.revokeSkill(player, "pilot_"+ pilotFaction + space_skill.SKILL_NAMES[i]);
				}
				
				utils.removeScriptVar(player, "revokePilotSkill");
				sendSystemMessageTestingOnly(player, "Pilot Skills Revoked.");
				return true;
			}
		}
		return false;
	}
	
	
	public void handlePilotSkillSelect(obj_id player) throws InterruptedException
	{
		refreshMenu(player, GENERIC_PROMPT, GENERIC_TITLE, PILOT_SKILLS, "handlePilotSkillSelect", false);
	}
	
	
	public int handlePilotSkillSelect(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		if (btn == sui.BP_REVERT)
		{
			handleShipMenuSelect(player);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		if (idx == -1 || idx > PILOT_SKILLS.length)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(player))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		boolean pilotRevoked = true;
		
		if (space_skill.hasSpaceSkills(player))
		{
			pilotRevoked = revokePilotingSkills(player);
		}
		
		if (!pilotRevoked)
		{
			sendSystemMessageTestingOnly(player, "The system was unable to revoke your pilot skills.");
			return SCRIPT_OVERRIDE;
		}
		
		if (idx == 0)
		{
			skill.grantSkill(player, "pilot_imperial_navy_novice");
			skill.grantSkill(player, "pilot_imperial_navy_starships_01");
			skill.grantSkill(player, "pilot_imperial_navy_starships_02");
			skill.grantSkill(player, "pilot_imperial_navy_starships_03");
			skill.grantSkill(player, "pilot_imperial_navy_starships_04");
			skill.grantSkill(player, "pilot_imperial_navy_weapons_01");
			skill.grantSkill(player, "pilot_imperial_navy_weapons_02");
			skill.grantSkill(player, "pilot_imperial_navy_weapons_03");
			skill.grantSkill(player, "pilot_imperial_navy_weapons_04");
			skill.grantSkill(player, "pilot_imperial_navy_procedures_01");
			skill.grantSkill(player, "pilot_imperial_navy_procedures_02");
			skill.grantSkill(player, "pilot_imperial_navy_procedures_03");
			skill.grantSkill(player, "pilot_imperial_navy_procedures_04");
			skill.grantSkill(player, "pilot_imperial_navy_droid_01");
			skill.grantSkill(player, "pilot_imperial_navy_droid_02");
			skill.grantSkill(player, "pilot_imperial_navy_droid_03");
			skill.grantSkill(player, "pilot_imperial_navy_droid_04");
			skill.grantSkill(player, "pilot_imperial_navy_master");
			sendSystemMessageTestingOnly(player, "Master Imperial Pilot skills received.");
		}
		else if (idx == 1)
		{
			skill.grantSkill(player, "pilot_rebel_navy_novice");
			skill.grantSkill(player, "pilot_rebel_navy_starships_01");
			skill.grantSkill(player, "pilot_rebel_navy_starships_02");
			skill.grantSkill(player, "pilot_rebel_navy_starships_03");
			skill.grantSkill(player, "pilot_rebel_navy_starships_04");
			skill.grantSkill(player, "pilot_rebel_navy_weapons_01");
			skill.grantSkill(player, "pilot_rebel_navy_weapons_02");
			skill.grantSkill(player, "pilot_rebel_navy_weapons_03");
			skill.grantSkill(player, "pilot_rebel_navy_weapons_04");
			skill.grantSkill(player, "pilot_rebel_navy_procedures_01");
			skill.grantSkill(player, "pilot_rebel_navy_procedures_02");
			skill.grantSkill(player, "pilot_rebel_navy_procedures_03");
			skill.grantSkill(player, "pilot_rebel_navy_procedures_04");
			skill.grantSkill(player, "pilot_rebel_navy_droid_01");
			skill.grantSkill(player, "pilot_rebel_navy_droid_02");
			skill.grantSkill(player, "pilot_rebel_navy_droid_03");
			skill.grantSkill(player, "pilot_rebel_navy_droid_04");
			skill.grantSkill(player, "pilot_rebel_navy_master");
			sendSystemMessageTestingOnly(player, "Master Rebel Pilot skills received.");
		}
		else if (idx == 2)
		{
			skill.grantSkill(player, "pilot_neutral_novice");
			skill.grantSkill(player, "pilot_neutral_starships_01");
			skill.grantSkill(player, "pilot_neutral_starships_02");
			skill.grantSkill(player, "pilot_neutral_starships_03");
			skill.grantSkill(player, "pilot_neutral_starships_04");
			skill.grantSkill(player, "pilot_neutral_weapons_01");
			skill.grantSkill(player, "pilot_neutral_weapons_02");
			skill.grantSkill(player, "pilot_neutral_weapons_03");
			skill.grantSkill(player, "pilot_neutral_weapons_04");
			skill.grantSkill(player, "pilot_neutral_procedures_01");
			skill.grantSkill(player, "pilot_neutral_procedures_02");
			skill.grantSkill(player, "pilot_neutral_procedures_03");
			skill.grantSkill(player, "pilot_neutral_procedures_04");
			skill.grantSkill(player, "pilot_neutral_droid_01");
			skill.grantSkill(player, "pilot_neutral_droid_02");
			skill.grantSkill(player, "pilot_neutral_droid_03");
			skill.grantSkill(player, "pilot_neutral_droid_04");
			skill.grantSkill(player, "pilot_neutral_master");
			sendSystemMessageTestingOnly(player, "Master Privateer Pilot skills received.");
		}
		
		handlePilotSkillSelect(player);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleRebelShipSelect(obj_id self, dictionary params) throws InterruptedException
	{
		handleShipSelect(params, REBEL_SHIP_OPTIONS, REBEL_SHIP_TYPES, "handleRebelShipSelect");
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleImperialShipSelect(obj_id self, dictionary params) throws InterruptedException
	{
		handleShipSelect(params, IMPERIAL_SHIP_OPTIONS, IMPERIAL_SHIP_TYPES, "handleImperialShipSelect");
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleFreelanceShipSelect(obj_id self, dictionary params) throws InterruptedException
	{
		handleShipSelect(params, FREELANCE_SHIP_OPTIONS, FREELANCE_SHIP_TYPES, "handleFreelanceShipSelect");
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleOtherShipSelect(obj_id self, dictionary params) throws InterruptedException
	{
		handleShipSelect(params, OTHER_SHIP_OPTIONS, OTHER_SHIP_TYPES, "handleOtherShipSelect");
		return SCRIPT_CONTINUE;
	}
	
	
	public void handleShipSelect(dictionary params, String[] options, String[] types, String message) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			
			return;
		}
		
		obj_id player = sui.getPlayerId(params);
		
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		if (btn == sui.BP_REVERT)
		{
			handleShipOption(player);
			return;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return;
		}
		
		if (idx == -1 || idx > options.length)
		{
			cleanScriptVars(player);
			return;
		}
		
		if (!isIdValid(player))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return;
		}
		
		if (space_transition.isPlayerBelowShipLimit(player))
		{
			obj_id shipId = space_utils.createShipControlDevice(player, types[idx], false);
			if (isIdValid(shipId))
			{
				sendSystemMessageTestingOnly(player, "Created ship ("+ options[idx] + ") in datapad.");
			}
			else
			{
				sendSystemMessageTestingOnly(player, "Failed to create ship.");
			}
		}
		else
		{
			sendSystemMessageTestingOnly(player, "Failed to create ship. No room in datapad.");
		}
		
		refreshMenu ( player, "Select the desired option", "Character Builder Terminal", options, message, false);
	}
	
	
	public void handleShipMenuSelect(obj_id player) throws InterruptedException
	{
		
		refreshMenu ( player, "Select the desired deed option", "Character Builder Terminal", MAIN_SHIP_OPTIONS, "handleShipMenuSelect", false);
	}
	
	
	public int handleShipMenuSelect(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		obj_id pInv = utils.getInventoryContainer(player);
		
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		if (btn == sui.BP_REVERT)
		{
			startCharacterBuilder(player);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		if (idx == -1 || idx > MAIN_SHIP_OPTIONS.length)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(player))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (!isIdValid(pInv))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (getVolumeFree(pInv) <= 0)
		{
			sendSystemMessageTestingOnly(player, "Your Inventory is Full, please make room and try again.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		switch(idx)
		{
			case 0:  
			refreshMenu(player, GENERIC_PROMPT, GENERIC_TITLE, SPACE_LOOT_CATEGORIES, "handleShipComponentSelection", false);
			break;
			case 1:  
			String[] shipChassisArray = dataTableGetStringColumn(SHIP_CHASSIS_TBL, "name");;
			refreshMenu(player, GENERIC_PROMPT, GENERIC_TITLE, shipChassisArray, "handleShipChasisSelection", false);
			break;
			case 2:  
			static_item.createNewItemFunction("item_gunship_imperial_schematic",pInv);
			static_item.createNewItemFunction("item_gunship_rebel_schematic",pInv);
			static_item.createNewItemFunction("item_gunship_neutral_schematic",pInv);
			sendSystemMessageTestingOnly(player, "Gunship Schematics Issued.");
			handleShipMenuSelect(player);
			break;
			case 3: 
			refreshMenu(player, GENERIC_PROMPT, GENERIC_TITLE, COLLECTION_COMPONENT_SCHEMS, "handleCollectionComponentSelect", false);
			break;
			case 4: 
			refreshMenu(player, GENERIC_PROMPT, GENERIC_TITLE, PILOT_SKILLS, "handlePilotSkillSelect", false);
			break;
			default:
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public void handleCollectionComponentSelect(obj_id player) throws InterruptedException
	{
		refreshMenu(player, GENERIC_PROMPT, GENERIC_TITLE, COLLECTION_COMPONENT_SCHEMS, "handleCollectionComponentSelect", false);
	}
	
	
	public int handleCollectionComponentSelect(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		obj_id pInv = utils.getInventoryContainer(player);
		
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		if (btn == sui.BP_REVERT)
		{
			handleShipMenuSelect(player);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		int dataTableLength = dataTableGetNumRows(SHIP_CHASSIS_TBL);
		if (idx == -1 || idx > dataTableLength)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(player))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (!isIdValid(pInv))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (getVolumeFree(pInv) <= 0)
		{
			sendSystemMessageTestingOnly(player, "Your Inventory is Full, please make room and try again.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		switch(idx)
		{
			case 0:  
			static_item.createNewItemFunction("item_collection_reward_booster_01_mk5_schematic",pInv);
			static_item.createNewItemFunction("item_collection_reward_booster_01_mk4_schematic",pInv);
			static_item.createNewItemFunction("item_collection_reward_booster_01_mk3_schematic",pInv);
			static_item.createNewItemFunction("item_collection_reward_booster_01_mk2_schematic",pInv);
			static_item.createNewItemFunction("item_collection_reward_booster_01_mk1_schematic",pInv);
			sendSystemMessageTestingOnly(player, "Component Schematics Issued.");
			handleCollectionComponentSelect(player);
			break;
			case 1:  
			static_item.createNewItemFunction("item_collection_reward_capacitor_01_mk5_schematic",pInv);
			static_item.createNewItemFunction("item_collection_reward_capacitor_01_mk4_schematic",pInv);
			static_item.createNewItemFunction("item_collection_reward_capacitor_01_mk3_schematic",pInv);
			static_item.createNewItemFunction("item_collection_reward_capacitor_01_mk2_schematic",pInv);
			static_item.createNewItemFunction("item_collection_reward_capacitor_01_mk1_schematic",pInv);
			sendSystemMessageTestingOnly(player, "Component Schematics Issued.");
			handleCollectionComponentSelect(player);
			break;
			case 2:  
			static_item.createNewItemFunction("item_collection_reward_engine_01_mk5_schematic",pInv);
			static_item.createNewItemFunction("item_collection_reward_engine_01_mk4_schematic",pInv);
			static_item.createNewItemFunction("item_collection_reward_engine_01_mk3_schematic",pInv);
			static_item.createNewItemFunction("item_collection_reward_engine_01_mk2_schematic",pInv);
			static_item.createNewItemFunction("item_collection_reward_engine_01_mk1_schematic",pInv);
			sendSystemMessageTestingOnly(player, "Component Schematics Issued.");
			handleCollectionComponentSelect(player);
			break;
			case 3: 
			static_item.createNewItemFunction("item_collection_reward_reactor_01_mk5_schematic",pInv);
			static_item.createNewItemFunction("item_collection_reward_reactor_01_mk4_schematic",pInv);
			static_item.createNewItemFunction("item_collection_reward_reactor_01_mk3_schematic",pInv);
			static_item.createNewItemFunction("item_collection_reward_reactor_01_mk2_schematic",pInv);
			static_item.createNewItemFunction("item_collection_reward_reactor_01_mk1_schematic",pInv);
			sendSystemMessageTestingOnly(player, "Component Schematics Issued.");
			handleCollectionComponentSelect(player);
			break;
			case 4: 
			static_item.createNewItemFunction("item_reward_orion_engine_schematic_01_01",pInv);
			static_item.createNewItemFunction("item_reward_nova_engine_schematic_01_01",pInv);
			sendSystemMessageTestingOnly(player, "Component Schematics Issued.");
			handleCollectionComponentSelect(player);
			break;
			default:
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		handleCollectionComponentSelect(player);
		return SCRIPT_CONTINUE;
	}
	
	
	public void handleShipChasisSelection(obj_id player) throws InterruptedException
	{
		String[] shipChassisArray = dataTableGetStringColumn(SHIP_CHASSIS_TBL, "name");;
		refreshMenu(player, GENERIC_PROMPT, GENERIC_TITLE, shipChassisArray, "handleShipChasisSelection", false);
	}
	
	
	public int handleShipChasisSelection(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		obj_id pInv = utils.getInventoryContainer(player);
		
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		if (btn == sui.BP_REVERT)
		{
			handleShipMenuSelect(player);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		int dataTableLength = dataTableGetNumRows(SHIP_CHASSIS_TBL);
		if (idx == -1 || idx > dataTableLength)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(player))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (!isIdValid(pInv))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (getVolumeFree(pInv) <= 0)
		{
			sendSystemMessageTestingOnly(player, "Your Inventory is Full, please make room and try again.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		issueShipChassis(player, idx);
		
		handleShipChasisSelection(player);
		return SCRIPT_CONTINUE;
	}
	
	
	public void handleShipComponentSelection(obj_id player) throws InterruptedException
	{
		refreshMenu(player, GENERIC_PROMPT, GENERIC_TITLE, SPACE_LOOT_CATEGORIES, "handleShipComponentSelection", false);
	}
	
	
	public int handleShipComponentSelection(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		obj_id pInv = utils.getInventoryContainer(player);
		
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		if (btn == sui.BP_REVERT)
		{
			handleShipMenuSelect(player);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		if (idx == -1 || idx > SPACE_LOOT_CATEGORIES.length)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(player))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (!isIdValid(pInv))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (getVolumeFree(pInv) <= 0)
		{
			sendSystemMessageTestingOnly(player, "Your Inventory is Full, please make room and try again.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		switch(idx)
		{
			case 0:  
			String armorMenuArray[] = dataTableGetStringColumn(SHIPCOMPONENT_ARMOR_TBL, "strName");
			refreshMenu(player, GENERIC_PROMPT, GENERIC_TITLE, armorMenuArray, "handleShipComponentArmorSelection", false);
			break;
			case 1:  
			String boosterMenuArray[] = dataTableGetStringColumn(SHIPCOMPONENT_BOOSTER_TBL, "strName");
			refreshMenu(player, GENERIC_PROMPT, GENERIC_TITLE, boosterMenuArray, "handleShipComponentBoosterSelection", false);
			break;
			case 2:  
			String droidInterfaceMenuArray[] = dataTableGetStringColumn(SHIPCOMPONENT_DROIDINTERFACE_TBL, "strName");
			refreshMenu(player, GENERIC_PROMPT, GENERIC_TITLE, droidInterfaceMenuArray, "handleShipComponentDroidInterfaceSelection", false);
			break;
			case 3:  
			String engineMenuArray[] = dataTableGetStringColumn(SHIPCOMPONENT_ENGINE_TBL, "strName");
			refreshMenu(player, GENERIC_PROMPT, GENERIC_TITLE, engineMenuArray, "handleShipComponentEngineSelection", false);
			break;
			case 4:  
			String reactorMenuArray[] = dataTableGetStringColumn(SHIPCOMPONENT_REACTOR_TBL, "strName");
			refreshMenu(player, GENERIC_PROMPT, GENERIC_TITLE, reactorMenuArray, "handleShipComponentReactorSelection", false);
			break;
			case 5:  
			String shieldMenuArray[] = dataTableGetStringColumn(SHIPCOMPONENT_SHIELD_TBL, "strName");
			refreshMenu(player, GENERIC_PROMPT, GENERIC_TITLE, shieldMenuArray, "handleShipComponentShieldSelection", false);
			break;
			case 6:  
			String weaponMenuArray[] = dataTableGetStringColumn(SHIPCOMPONENT_WEAPON_TBL, "strName");
			refreshMenu(player, GENERIC_PROMPT, GENERIC_TITLE, weaponMenuArray, "handleShipComponentWeaponSelection", false);
			break;
			case 7:  
			String capacitorMenuArray[] = dataTableGetStringColumn(SHIPCOMPONENT_CAPACITOR_TBL, "strName");
			refreshMenu(player, GENERIC_PROMPT, GENERIC_TITLE, capacitorMenuArray, "handleShipComponentCapacitorSelection", false);
			break;
			default:
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public void handleShipComponentArmorSelection(obj_id player) throws InterruptedException
	{
		String armorMenuArray[] = dataTableGetStringColumn(SHIPCOMPONENT_ARMOR_TBL, "strName");
		refreshMenu(player, GENERIC_PROMPT, GENERIC_TITLE, armorMenuArray, "handleShipComponentArmorSelection", false);
	}
	
	
	public int handleShipComponentArmorSelection(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		obj_id pInv = utils.getInventoryContainer(player);
		
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		if (btn == sui.BP_REVERT)
		{
			handleShipComponentSelection(player);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		int dataTableLength = dataTableGetNumRows(SHIPCOMPONENT_ARMOR_TBL);
		if (idx == -1 || idx > dataTableLength)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(player))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (!isIdValid(pInv))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (getVolumeFree(pInv) <= 0)
		{
			sendSystemMessageTestingOnly(player, "Your Inventory is Full, please make room and try again.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		String objString = dataTableGetString(SHIPCOMPONENT_ARMOR_TBL, idx, "strType");
		obj_id objectOID = createObject(objString, pInv, "");
		
		if (!isIdValid(objectOID))
		{
			sendSystemMessage(player, "The component could not be created", null);
			return SCRIPT_CONTINUE;
		}
		
		tweakSpaceShipComponent(objectOID);
		
		handleShipComponentArmorSelection(player);
		return SCRIPT_CONTINUE;
	}
	
	
	public void handleShipComponentBoosterSelection(obj_id player) throws InterruptedException
	{
		String boosterMenuArray[] = dataTableGetStringColumn(SHIPCOMPONENT_BOOSTER_TBL, "strName");
		refreshMenu(player, GENERIC_PROMPT, GENERIC_TITLE, boosterMenuArray, "handleShipComponentBoosterSelection", false);
	}
	
	
	public int handleShipComponentBoosterSelection(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		obj_id pInv = utils.getInventoryContainer(player);
		
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		if (btn == sui.BP_REVERT)
		{
			handleShipComponentSelection(player);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		int dataTableLength = dataTableGetNumRows(SHIPCOMPONENT_BOOSTER_TBL);
		if (idx == -1 || idx > dataTableLength)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(player))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (!isIdValid(pInv))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (getVolumeFree(pInv) <= 0)
		{
			sendSystemMessageTestingOnly(player, "Your Inventory is Full, please make room and try again.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		String objectString = dataTableGetString(SHIPCOMPONENT_BOOSTER_TBL, idx, "strType");
		obj_id objectOID = createObject(objectString, pInv, "");
		
		if (!isIdValid(objectOID))
		{
			sendSystemMessage(player, "The component could not be created", null);
			return SCRIPT_CONTINUE;
		}
		
		tweakSpaceShipComponent(objectOID);
		
		handleShipComponentBoosterSelection(player);
		return SCRIPT_CONTINUE;
	}
	
	
	public void handleShipComponentCapacitorSelection(obj_id player) throws InterruptedException
	{
		String menuArray[] = dataTableGetStringColumn(SHIPCOMPONENT_CAPACITOR_TBL, "strName");
		refreshMenu(player, GENERIC_PROMPT, GENERIC_TITLE, menuArray, "handleShipComponentCapacitorSelection", false);
	}
	
	
	public int handleShipComponentCapacitorSelection(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		obj_id pInv = utils.getInventoryContainer(player);
		
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		if (btn == sui.BP_REVERT)
		{
			handleShipComponentSelection(player);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		int dataTableLength = dataTableGetNumRows(SHIPCOMPONENT_CAPACITOR_TBL);
		if (idx == -1 || idx > dataTableLength)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(player))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (!isIdValid(pInv))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (getVolumeFree(pInv) <= 0)
		{
			sendSystemMessageTestingOnly(player, "Your Inventory is Full, please make room and try again.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		String objectString = dataTableGetString(SHIPCOMPONENT_CAPACITOR_TBL, idx, "strType");
		obj_id objectOID = createObject(objectString, pInv, "");
		
		if (!isIdValid(objectOID))
		{
			sendSystemMessage(player, "The component could not be created", null);
			return SCRIPT_CONTINUE;
		}
		
		tweakSpaceShipComponent(objectOID);
		
		handleShipComponentCapacitorSelection(player);
		return SCRIPT_CONTINUE;
	}
	
	
	public void handleShipComponentDroidInterfaceSelection(obj_id player) throws InterruptedException
	{
		String menuArray[] = dataTableGetStringColumn(SHIPCOMPONENT_DROIDINTERFACE_TBL, "strName");
		refreshMenu(player, GENERIC_PROMPT, GENERIC_TITLE, menuArray, "handleShipComponentDroidInterfaceSelection", false);
	}
	
	
	public int handleShipComponentDroidInterfaceSelection(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		obj_id pInv = utils.getInventoryContainer(player);
		
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		if (btn == sui.BP_REVERT)
		{
			handleShipComponentSelection(player);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		int dataTableLength = dataTableGetNumRows(SHIPCOMPONENT_DROIDINTERFACE_TBL);
		if (idx == -1 || idx > dataTableLength)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(player))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (!isIdValid(pInv))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (getVolumeFree(pInv) <= 0)
		{
			sendSystemMessageTestingOnly(player, "Your Inventory is Full, please make room and try again.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		String objectString = dataTableGetString(SHIPCOMPONENT_DROIDINTERFACE_TBL, idx, "strType");
		obj_id objectOID = createObject(objectString, pInv, "");
		
		if (!isIdValid(objectOID))
		{
			sendSystemMessage(player, "The component could not be created", null);
			return SCRIPT_CONTINUE;
		}
		
		tweakSpaceShipComponent(objectOID);
		
		handleShipComponentDroidInterfaceSelection(player);
		return SCRIPT_CONTINUE;
	}
	
	
	public void handleShipComponentEngineSelection(obj_id player) throws InterruptedException
	{
		String menuArray[] = dataTableGetStringColumn(SHIPCOMPONENT_ENGINE_TBL, "strName");
		refreshMenu(player, GENERIC_PROMPT, GENERIC_TITLE, menuArray, "handleShipComponentEngineSelection", false);
	}
	
	
	public int handleShipComponentEngineSelection(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		obj_id pInv = utils.getInventoryContainer(player);
		
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		if (btn == sui.BP_REVERT)
		{
			handleShipComponentSelection(player);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		int dataTableLength = dataTableGetNumRows(SHIPCOMPONENT_ENGINE_TBL);
		if (idx == -1 || idx > dataTableLength)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(player))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (!isIdValid(pInv))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (getVolumeFree(pInv) <= 0)
		{
			sendSystemMessageTestingOnly(player, "Your Inventory is Full, please make room and try again.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		String objectString = dataTableGetString(SHIPCOMPONENT_ENGINE_TBL, idx, "strType");
		obj_id objectOID = createObject(objectString, pInv, "");
		
		if (!isIdValid(objectOID))
		{
			sendSystemMessage(player, "The component could not be created", null);
			return SCRIPT_CONTINUE;
		}
		
		tweakSpaceShipComponent(objectOID);
		
		handleShipComponentEngineSelection(player);
		return SCRIPT_CONTINUE;
	}
	
	
	public void handleShipComponentReactorSelection(obj_id player) throws InterruptedException
	{
		String menuArray[] = dataTableGetStringColumn(SHIPCOMPONENT_REACTOR_TBL, "strName");
		refreshMenu(player, GENERIC_PROMPT, GENERIC_TITLE, menuArray, "handleShipComponentReactorSelection", false);
	}
	
	
	public int handleShipComponentReactorSelection(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		obj_id pInv = utils.getInventoryContainer(player);
		
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		if (btn == sui.BP_REVERT)
		{
			handleShipComponentSelection(player);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		int dataTableLength = dataTableGetNumRows(SHIPCOMPONENT_REACTOR_TBL);
		if (idx == -1 || idx > dataTableLength)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(player))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (!isIdValid(pInv))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (getVolumeFree(pInv) <= 0)
		{
			sendSystemMessageTestingOnly(player, "Your Inventory is Full, please make room and try again.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		String objectString = dataTableGetString(SHIPCOMPONENT_REACTOR_TBL, idx, "strType");
		obj_id objectOID = createObject(objectString, pInv, "");
		
		if (!isIdValid(objectOID))
		{
			sendSystemMessage(player, "The component could not be created", null);
			return SCRIPT_CONTINUE;
		}
		
		tweakSpaceShipComponent(objectOID);
		
		handleShipComponentReactorSelection(player);
		return SCRIPT_CONTINUE;
	}
	
	
	public void handleShipComponentShieldSelection(obj_id player) throws InterruptedException
	{
		String menuArray[] = dataTableGetStringColumn(SHIPCOMPONENT_SHIELD_TBL, "strName");
		refreshMenu(player, GENERIC_PROMPT, GENERIC_TITLE, menuArray, "handleShipComponentShieldSelection", false);
	}
	
	
	public int handleShipComponentShieldSelection(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		obj_id pInv = utils.getInventoryContainer(player);
		
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		if (btn == sui.BP_REVERT)
		{
			handleShipComponentSelection(player);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		int dataTableLength = dataTableGetNumRows(SHIPCOMPONENT_SHIELD_TBL);
		if (idx == -1 || idx > dataTableLength)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(player))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (!isIdValid(pInv))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (getVolumeFree(pInv) <= 0)
		{
			sendSystemMessageTestingOnly(player, "Your Inventory is Full, please make room and try again.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		String objectString = dataTableGetString(SHIPCOMPONENT_SHIELD_TBL, idx, "strType");
		obj_id objectOID = createObject(objectString, pInv, "");
		
		if (!isIdValid(objectOID))
		{
			sendSystemMessage(player, "The component could not be created", null);
			return SCRIPT_CONTINUE;
		}
		
		tweakSpaceShipComponent(objectOID);
		
		handleShipComponentShieldSelection(player);
		return SCRIPT_CONTINUE;
	}
	
	public void handleShipComponentWeaponSelection(obj_id player) throws InterruptedException
	{
		String menuArray[] = dataTableGetStringColumn(SHIPCOMPONENT_WEAPON_TBL, "strName");
		refreshMenu(player, GENERIC_PROMPT, GENERIC_TITLE, menuArray, "handleShipComponentWeaponSelection", false);
	}
	
	
	public int handleShipComponentWeaponSelection(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		obj_id pInv = utils.getInventoryContainer(player);
		
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		if (btn == sui.BP_REVERT)
		{
			handleShipComponentSelection(player);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		int dataTableLength = dataTableGetNumRows(SHIPCOMPONENT_WEAPON_TBL);
		if (idx == -1 || idx > dataTableLength)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(player))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (!isIdValid(pInv))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (getVolumeFree(pInv) <= 0)
		{
			sendSystemMessageTestingOnly(player, "Your Inventory is Full, please make room and try again.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		String objectString = dataTableGetString(SHIPCOMPONENT_WEAPON_TBL, idx, "strType");
		obj_id objectOID = createObject(objectString, pInv, "");
		
		if (!isIdValid(objectOID))
		{
			sendSystemMessage(player, "The component could not be created", null);
			return SCRIPT_CONTINUE;
		}
		
		tweakSpaceShipComponent(objectOID);
		
		handleShipComponentWeaponSelection(player);
		return SCRIPT_CONTINUE;
	}
	
	
	public void handleDeedOption(obj_id player) throws InterruptedException
	{
		
		refreshMenu ( player, "Select the desired deed option", "Character Builder Terminal", DEED_OPTIONS, "handleDeedSelect", false);
	}
	
	
	public int handleDeedSelect(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		obj_id pInv = utils.getInventoryContainer(player);
		
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		if (btn == sui.BP_REVERT)
		{
			startCharacterBuilder(player);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		if (idx == -1 || idx > DEED_OPTIONS.length)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(player))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (!isIdValid(pInv))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (getVolumeFree(pInv) <= 0)
		{
			sendSystemMessageTestingOnly(player, "Your Inventory is Full, please make room and try again.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		switch(idx)
		{
			case 0:  
			createObject("object/tangible/deed/factory_deed/clothing_factory_deed.iff", pInv, "");
			sendSystemMessageTestingOnly(player, "Clothing Factory Deed Issued.");
			break;
			case 1:  
			createObject("object/tangible/deed/factory_deed/food_factory_deed.iff", pInv, "");
			sendSystemMessageTestingOnly(player, "Food Factory Deed Issued.");
			break;
			case 2:  
			createObject("object/tangible/deed/factory_deed/item_factory_deed.iff", pInv, "");
			sendSystemMessageTestingOnly(player, "Item Factory Deed Issued.");
			break;
			case 3:  
			createObject("object/tangible/deed/factory_deed/structure_factory_deed.iff", pInv, "");
			sendSystemMessageTestingOnly(player, "Structure Factory Deed Issued.");
			break;
			case 4:  
			createObject("object/tangible/deed/player_house_deed/generic_house_small_deed.iff", pInv, "");
			sendSystemMessageTestingOnly(player, "Generic House Deed Issued.");
			break;
			case 5:  
			createObject("object/tangible/deed/player_house_deed/tatooine_house_small_deed.iff", pInv, "");
			sendSystemMessageTestingOnly(player, "Small Tatooine House Deed Issued.");
			break;
			case 6:  
			createObject("object/tangible/deed/player_house_deed/naboo_house_small_deed.iff", pInv, "");
			sendSystemMessageTestingOnly(player, "Small Naboo House Deed Issued.");
			break;
			case 7:  
			createObject("object/tangible/deed/player_house_deed/corellia_house_small_deed.iff", pInv, "");
			sendSystemMessageTestingOnly(player, "Small Corellia House Deed Issued.");
			break;
			case 8:  
			createObject("object/tangible/deed/player_house_deed/merchant_tent_style_03_deed.iff", pInv, "");
			sendSystemMessageTestingOnly(player, "Merchant Tent Deed Issued.");
			break;
			case 9:  
			obj_id mineral = createObject("object/tangible/deed/harvester_deed/ore_harvester_heavy_deed.iff", pInv, "");
			sendSystemMessageTestingOnly(player, "Heavy Mineral Harvester Issued.");
			if (isIdValid(mineral))
			{
				setObjVar (mineral, "player_structure.deed.maxExtractionRate", 13);
				setObjVar (mineral, "player_structure.deed.currentExtractionRate", 13);
			}
			break;
			case 10:  
			obj_id flora = createObject("object/tangible/deed/harvester_deed/flora_harvester_deed_heavy.iff", pInv, "");
			sendSystemMessageTestingOnly(player, "Heavy Flora Harvester Issued.");
			if (isIdValid(flora))
			{
				setObjVar (flora, "player_structure.deed.maxExtractionRate", 13);
				setObjVar (flora, "player_structure.deed.currentExtractionRate", 13);
			}
			break;
			case 11:  
			obj_id gas = createObject("object/tangible/deed/harvester_deed/gas_harvester_deed_heavy.iff", pInv, "");
			sendSystemMessageTestingOnly(player, "Heavy Gas Harvester Issued.");
			if (isIdValid(gas))
			{
				setObjVar (gas, "player_structure.deed.maxExtractionRate", 13);
				setObjVar (gas, "player_structure.deed.currentExtractionRate", 13);
			}
			break;
			case 12:  
			obj_id chemical = createObject("object/tangible/deed/harvester_deed/liquid_harvester_deed_heavy.iff", pInv, "");
			sendSystemMessageTestingOnly(player, "Heavy Chemical Harvester Issued.");
			if (isIdValid(chemical))
			{
				setObjVar (chemical, "player_structure.deed.maxExtractionRate", 13);
				setObjVar (chemical, "player_structure.deed.currentExtractionRate", 13);
			}
			break;
			case 13:  
			obj_id moisture = createObject("object/tangible/deed/harvester_deed/moisture_harvester_deed_heavy.iff", pInv, "");
			sendSystemMessageTestingOnly(player, "Heavy Moisture Vaporator Issued.");
			if (isIdValid(moisture))
			{
				setObjVar (moisture, "player_structure.deed.maxExtractionRate", 13);
				setObjVar (moisture, "player_structure.deed.currentExtractionRate", 13);
			}
			break;
			case 14:  
			obj_id fusion = createObject("object/tangible/deed/generator_deed/power_generator_fusion_deed.iff", pInv, "");
			sendSystemMessageTestingOnly(player, "Fusion Reactor Issued.");
			if (isIdValid(fusion))
			{
				setObjVar (fusion, "player_structure.deed.maxExtractionRate", 16);
				setObjVar (fusion, "player_structure.deed.currentExtractionRate", 16);
			}
			break;
			case 15:
			createObject("object/tangible/deed/city_deed/cityhall_corellia_deed.iff", pInv, "");
			createObject("object/tangible/deed/city_deed/cloning_corellia_deed.iff", pInv, "");
			createObject("object/tangible/deed/city_deed/bank_corellia_deed.iff", pInv, "");
			createObject("object/tangible/deed/city_deed/garage_corellia_deed.iff", pInv, "");
			createObject("object/tangible/deed/city_deed/garden_corellia_lrg_01_deed.iff", pInv, "");
			createObject("object/tangible/deed/city_deed/garden_corellia_med_01_deed.iff", pInv, "");
			createObject("object/tangible/deed/city_deed/garden_corellia_sml_01_deed.iff", pInv, "");
			sendSystemMessageTestingOnly(player, "Corellia City Pack Created");
			break;
			case 16:
			createObject("object/tangible/deed/city_deed/cityhall_naboo_deed.iff", pInv, "");
			createObject("object/tangible/deed/city_deed/cloning_naboo_deed.iff", pInv, "");
			createObject("object/tangible/deed/city_deed/bank_naboo_deed.iff", pInv, "");
			createObject("object/tangible/deed/city_deed/garage_naboo_deed.iff", pInv, "");
			createObject("object/tangible/deed/city_deed/garden_naboo_lrg_01_deed.iff", pInv, "");
			createObject("object/tangible/deed/city_deed/garden_naboo_med_01_deed.iff", pInv, "");
			createObject("object/tangible/deed/city_deed/garden_naboo_sml_01_deed.iff", pInv, "");
			sendSystemMessageTestingOnly(player, "Naboo City Pack Created");
			break;
			case 17:
			createObject("object/tangible/deed/city_deed/cityhall_tatooine_deed.iff", pInv, "");
			createObject("object/tangible/deed/city_deed/cloning_tatooine_deed.iff", pInv, "");
			createObject("object/tangible/deed/city_deed/bank_tatooine_deed.iff", pInv, "");
			createObject("object/tangible/deed/city_deed/garage_tatooine_deed.iff", pInv, "");
			createObject("object/tangible/deed/city_deed/garden_tatooine_lrg_01_deed.iff", pInv, "");
			createObject("object/tangible/deed/city_deed/garden_tatooine_med_01_deed.iff", pInv, "");
			createObject("object/tangible/deed/city_deed/garden_tatooine_sml_01_deed.iff", pInv, "");
			sendSystemMessageTestingOnly(player, "Tatooine City Pack Created");
			break;
			default:
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
			
		}
		
		handleDeedOption(player);
		return SCRIPT_CONTINUE;
	}
	
	
	public void handleCraftingOption(obj_id player) throws InterruptedException
	{
		
		refreshMenu ( player, "Select the desired deed option", "Character Builder Terminal", CRAFTING_OPTIONS, "handleCraftingSelect", false);
	}
	
	
	public int handleCraftingSelect(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		obj_id pInv = utils.getInventoryContainer(player);
		
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		if (btn == sui.BP_REVERT)
		{
			startCharacterBuilder(player);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		if (idx == -1 || idx > CRAFTING_OPTIONS.length)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(player))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (!isIdValid(pInv))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (getVolumeFree(pInv) <= 0)
		{
			sendSystemMessageTestingOnly(player, "Your Inventory is Full, please make room and try again.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		switch(idx)
		{
			case 0:  
			createObject("object/tangible/crafting/station/weapon_station.iff", pInv, "");
			sendSystemMessageTestingOnly(player, "Weapon Crafting Station Issued.");
			break;
			case 1:  
			createObject("object/tangible/crafting/station/structure_station.iff", pInv, "");
			sendSystemMessageTestingOnly(player, "Structure Crafting Station Issued.");
			break;
			case 2:  
			createObject("object/tangible/crafting/station/clothing_station.iff", pInv, "");
			sendSystemMessageTestingOnly(player, "Clothing Crafting Station Issued.");
			break;
			case 3:  
			createObject("object/tangible/crafting/station/food_station.iff", pInv, "");
			sendSystemMessageTestingOnly(player, "Food Crafting Station Issued.");
			break;
			case 4:  
			createObject("object/tangible/crafting/station/generic_tool.iff", pInv, "");
			sendSystemMessageTestingOnly(player, "Generic Crafting Tool Issued.");
			break;
			case 5:  
			createObject("object/tangible/crafting/station/weapon_tool.iff", pInv, "");
			sendSystemMessageTestingOnly(player, "Weapon Crafting Tool Issued.");
			break;
			case 6:  
			createObject("object/tangible/crafting/station/structure_tool.iff", pInv, "");
			sendSystemMessageTestingOnly(player, "Structure Crafting Tool Issued.");
			break;
			case 7:  
			createObject("object/tangible/crafting/station/clothing_tool.iff", pInv, "");
			sendSystemMessageTestingOnly(player, "Clothing Crafting Tool Issued.");
			break;
			case 8:  
			createObject("object/tangible/crafting/station/food_tool.iff", pInv, "");
			sendSystemMessageTestingOnly(player, "Food Crafting Tool Issued.");
			break;
			case 9:
			createObject("object/tangible/crafting/station/space_tool.iff", pInv, "");
			sendSystemMessageTestingOnly(player, "Ship Crafting Tool Issued.");
			break;
			case 10:
			createObject("object/tangible/crafting/station/space_station.iff", pInv, "");
			sendSystemMessageTestingOnly(player, "Ship Crafting Station Issued.");
			break;
			default:
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
			
		}
		
		handleCraftingOption(player);
		return SCRIPT_CONTINUE;
		
	}
	
	
	public void handlePAOption(obj_id player) throws InterruptedException
	{
		refreshMenu ( player, "Select the desired deed option", "Character Builder Terminal", PA_OPTIONS, "handlePASelect", false);
	}
	
	
	public int handlePASelect(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			
			return SCRIPT_CONTINUE;
		}
		obj_id player = sui.getPlayerId(params);
		obj_id pInv = utils.getInventoryContainer(player);
		
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		if (btn == sui.BP_REVERT)
		{
			startCharacterBuilder(player);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		if (idx == -1 || idx > PA_OPTIONS.length)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(player))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (!isIdValid(pInv))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (getVolumeFree(pInv) <= 0)
		{
			sendSystemMessageTestingOnly(player, "Your Inventory is Full, please make room and try again.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		switch(idx)
		{
			case 0:  
			createObject("object/tangible/deed/guild_deed/generic_guild_deed.iff", pInv, "");
			sendSystemMessageTestingOnly(player, "Generic PA Hall Deed Issued.");
			break;
			case 1:  
			createObject("object/tangible/deed/guild_deed/tatooine_guild_deed.iff", pInv, "");
			sendSystemMessageTestingOnly(player, "Tatooine PA Hall Deed Issued.");
			break;
			case 2:  
			createObject("object/tangible/deed/guild_deed/naboo_guild_deed.iff", pInv, "");
			sendSystemMessageTestingOnly(player, "Naboo PA Hall Deed Issued.");
			break;
			case 3:  
			createObject("object/tangible/deed/guild_deed/corellia_guild_deed.iff", pInv, "");
			sendSystemMessageTestingOnly(player, "Corellia PA Hall Deed Issued.");
			break;
			case 4:  
			createObject("object/tangible/deed/city_deed/cityhall_tatooine_deed.iff", pInv, "");
			sendSystemMessageTestingOnly(player, "Tatooine City Hall Deed Issued.");
			break;
			case 5:  
			createObject("object/tangible/deed/city_deed/cityhall_naboo_deed.iff", pInv, "");
			sendSystemMessageTestingOnly(player, "Naboo City Hall Deed Issued.");
			break;
			case 6:  
			createObject("object/tangible/deed/city_deed/cityhall_corellia_deed.iff", pInv, "");
			sendSystemMessageTestingOnly(player, "Corellia City Hall Deed Issued.");
			break;
			default:
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
			
		}
		
		handlePAOption(player);
		return SCRIPT_CONTINUE;
	}
	
	
	public void handleWeaponOption(obj_id player) throws InterruptedException
	{
		
		refreshMenu (player, "Select the desired weapon option", "Character Builder Terminal", WEAPON_OPTIONS, "handleWeaponOptions", false);
		
	}
	
	public int handleWeaponOptions(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		if (btn == sui.BP_REVERT)
		{
			refreshMenu (player, "Select the desired character option", "Character Builder Terminal", CHARACTER_BUILDER_OPTIONS, "handleOptionSelect", true);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		if (idx == -1 || idx > WEAPON_OPTIONS.length)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(player))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		switch(idx)
		{
			
			case 0:
			refreshMenu (player, "Select the desired weapon option", "Character Builder Terminal", PISTOL_OPTIONS, "handlePistolSelect", false);
			break;
			case 1:
			refreshMenu (player, "Select the desired weapon option", "Character Builder Terminal", CARBINE_OPTIONS, "handleCarbineSelect", false);
			break;
			case 2:
			refreshMenu (player, "Select the desired weapon option", "Character Builder Terminal", RIFLE_OPTIONS, "handleRifleSelect", false);
			break;
			case 3:
			refreshMenu (player, "Select the desired weapon option", "Character Builder Terminal", HEAVY_WEAPON_OPTIONS, "handleHeavySelect", false);
			break;
			case 4:
			refreshMenu (player, "Select the desired weapon option", "Character Builder Terminal", UNARMED_OPTIONS, "handleUnarmedSelect", false);
			break;
			case 5:
			refreshMenu (player, "Select the desired weapon option", "Character Builder Terminal", ONEHANDED_OPTIONS, "handleOneHandedSelect", false);
			break;
			case 6:
			refreshMenu (player, "Select the desired weapon option", "Character Builder Terminal", TWOHANDED_OPTIONS, "handleTwoHandedSelect", false);
			break;
			case 7:
			refreshMenu (player, "Select the desired weapon option", "Character Builder Terminal", POLEARM_OPTIONS, "handlePolearmSelect", false);
			break;
			case 8:
			refreshMenu (player, "Select the desired weapon option", "Character Builder Terminal", BATTLEFIELD_WEAPON_OPTIONS, "handleBattlefieldSelect", false);
			break;
			
			default:
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
			
		}
		
		return SCRIPT_CONTINUE;
	}
	
	public int handlePistolSelect(obj_id self, dictionary params) throws InterruptedException
	{
		
		if ((params == null) || (params.isEmpty()))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		obj_id pInv = utils.getInventoryContainer(player);
		
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		if (btn == sui.BP_REVERT)
		{
			refreshMenu (player, "Select the desired weapon option", "Character Builder Terminal", WEAPON_OPTIONS, "handleWeaponOptions", false);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		if (idx == -1 || idx > PISTOL_OPTIONS.length)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(player))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (!isIdValid(pInv))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (getVolumeFree(pInv) <= 0)
		{
			sendSystemMessageTestingOnly(player, "Your Inventory is Full, please make room and try again.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		String weapon = "";
		
		switch(idx)
		{
			case 0:  
			weapon = "object/weapon/ranged/pistol/pistol_cdef.iff";
			break;
			case 1:  
			weapon = "object/weapon/ranged/pistol/pistol_scout_blaster.iff";
			break;
			case 2:  
			weapon = "object/weapon/ranged/pistol/pistol_geonosian_sonic_blaster_loot.iff";
			break;
			case 3:  
			weapon = "object/weapon/ranged/pistol/pistol_republic_blaster.iff";
			break;
			case 4:  
			weapon = "object/weapon/ranged/pistol/pistol_launcher.iff";
			break;
			case 5:  
			weapon = "object/weapon/ranged/pistol/pistol_scatter.iff";
			break;
			case 6:  
			weapon = "object/weapon/ranged/pistol/pistol_scatter_light.iff";
			break;
			case 7:  
			weapon = "object/weapon/ranged/pistol/pistol_srcombat.iff";
			break;
			case 8:  
			weapon = "object/weapon/ranged/pistol/pistol_striker.iff";
			break;
			case 9:  
			weapon = "object/weapon/ranged/pistol/pistol_tangle.iff";
			break;
			case 10:  
			weapon = "object/weapon/ranged/pistol/pistol_power5.iff";
			break;
			case 11:  
			weapon = "object/weapon/ranged/pistol/pistol_fwg5.iff";
			break;
			case 12:  
			weapon = "object/weapon/ranged/pistol/pistol_dx2.iff";
			break;
			case 13:  
			weapon = "object/weapon/ranged/pistol/pistol_dl44_metal.iff";
			break;
			case 14:  
			weapon = "object/weapon/ranged/pistol/pistol_dl44.iff";
			break;
			case 15:  
			weapon = "object/weapon/ranged/pistol/pistol_dh17.iff";
			break;
			case 16:  
			weapon = "object/weapon/ranged/pistol/pistol_d18.iff";
			break;
			case 17:  
			weapon = "object/weapon/ranged/pistol/pistol_alliance_disruptor.iff";
			break;
			case 18:  
			weapon = "object/weapon/ranged/pistol/pistol_deathhammer.iff";
			break;
			case 19:  
			weapon = "object/weapon/ranged/pistol/pistol_flare.iff";
			break;
			case 20:  
			weapon = "object/weapon/ranged/pistol/pistol_flechette.iff";
			break;
			case 21:  
			weapon = "object/weapon/ranged/pistol/pistol_intimidator.iff";
			break;
			case 22:  
			weapon = "object/weapon/ranged/pistol/pistol_jawa.iff";
			break;
			case 23:  
			weapon = "object/weapon/ranged/pistol/pistol_renegade.iff";
			break;
			case 24: 
			weapon = "object/weapon/ranged/pistol/pistol_de_10.iff";
			break;
			case 25: 
			weapon = "object/weapon/ranged/pistol/pistol_dl44_metal_light.iff";
			break;
			case 26: 
			weapon = "object/weapon/ranged/pistol/som_disruptor_pistol.iff";
			break;
			case 27: 
			weapon = "object/weapon/ranged/pistol/som_ion_relic_pistol.iff";
			break;
			case 28: 
			weapon = "object/weapon/ranged/pistol/pistol_heroic_sd.iff";
			break;
			default:
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		if (weapon != null && !weapon.equals(""))
		{
			createSnowFlakeFrogWeapon(player, weapon);
		}
		
		refreshMenu (player, "Select the desired weapon option", "Character Builder Terminal", PISTOL_OPTIONS, "handlePistolSelect", false);
		return SCRIPT_CONTINUE;
	}
	
	public int handleCarbineSelect(obj_id self, dictionary params) throws InterruptedException
	{
		
		if ((params == null) || (params.isEmpty()))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		obj_id pInv = utils.getInventoryContainer(player);
		
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		if (btn == sui.BP_REVERT)
		{
			refreshMenu (player, "Select the desired weapon option", "Character Builder Terminal", WEAPON_OPTIONS, "handleWeaponOptions", false);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		if (idx == -1 || idx > CARBINE_OPTIONS.length)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(player))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (!isIdValid(pInv))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (getVolumeFree(pInv) <= 0)
		{
			sendSystemMessageTestingOnly(player, "Your Inventory is Full, please make room and try again.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		String weapon = "";
		
		switch(idx)
		{
			case 0:  
			weapon = "object/weapon/ranged/carbine/carbine_cdef.iff";
			break;
			case 1:  
			weapon = "object/weapon/ranged/carbine/carbine_dh17.iff";
			break;
			case 2:  
			weapon = "object/weapon/ranged/carbine/carbine_dh17_snubnose.iff";
			break;
			case 3:  
			weapon = "object/weapon/ranged/carbine/carbine_e11.iff";
			break;
			case 4:  
			weapon = "object/weapon/ranged/carbine/carbine_e11_mark2.iff";
			break;
			case 5:  
			weapon = "object/weapon/ranged/carbine/carbine_laser.iff";
			break;
			case 6:  
			weapon = "object/weapon/ranged/carbine/carbine_dxr6.iff";
			break;
			case 7:  
			weapon = "object/weapon/ranged/carbine/carbine_ee3.iff";
			break;
			case 8:  
			weapon = "object/weapon/ranged/carbine/carbine_elite.iff";
			break;
			case 9:  
			weapon = "object/weapon/ranged/carbine/carbine_nym_slugthrower.iff";
			break;
			case 10:  
			weapon = "object/weapon/ranged/carbine/carbine_alliance_needler.iff";
			break;
			case 11:  
			weapon = "object/weapon/ranged/carbine/carbine_bothan_bola.iff";
			break;
			case 12:  
			weapon ="object/weapon/ranged/carbine/carbine_e5.iff";
			break;
			case 13:  
			weapon = "object/weapon/ranged/carbine/carbine_geo.iff";
			break;
			case 14:  
			weapon = "object/weapon/ranged/carbine/carbine_proton.iff";
			break;
			case 15:  
			weapon = "object/weapon/ranged/carbine/carbine_czerka_dart.iff";
			break;
			case 16:  
			weapon = "object/weapon/ranged/carbine/som_carbine_republic_sfor.iff";
			break;
			case 17:  
			weapon = "object/weapon/ranged/carbine/carbine_heroic_sd.iff";
			break;
			default:
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		if (weapon != null && !weapon.equals(""))
		{
			createSnowFlakeFrogWeapon(player, weapon);
		}
		
		refreshMenu (player, "Select the desired weapon option", "Character Builder Terminal", CARBINE_OPTIONS, "handleCarbineSelect", false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleRifleSelect(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		obj_id pInv = utils.getInventoryContainer(player);
		
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		if (btn == sui.BP_REVERT)
		{
			refreshMenu (player, "Select the desired weapon option", "Character Builder Terminal", WEAPON_OPTIONS, "handleWeaponOptions", false);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		if (idx == -1 || idx > RIFLE_OPTIONS.length)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(player))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (!isIdValid(pInv))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (getVolumeFree(pInv) <= 0)
		{
			sendSystemMessageTestingOnly(player, "Your Inventory is Full, please make room and try again.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		String weapon = "";
		
		switch(idx)
		{
			case 0:  
			weapon = "item_outbreak_deathrooper_rifle";
			break;
			case 1:  
			weapon = "weapon_gcw_tc22_rifle_03_01";
			break;
			case 2:  
			weapon = "weapon_jinkins_j1_01_01";
			break;
			case 3:  
			weapon = "object/weapon/ranged/rifle/rifle_cdef.iff";
			break;
			case 4:  
			weapon = "object/weapon/ranged/rifle/rifle_dlt20.iff";
			break;
			case 5:  
			weapon = "object/weapon/ranged/rifle/rifle_dlt20a.iff";
			break;
			case 6:  
			weapon = "object/weapon/ranged/rifle/rifle_tusken.iff";
			break;
			case 7:  
			weapon = "object/weapon/ranged/rifle/rifle_laser.iff";
			break;
			case 8:  
			weapon = "object/weapon/ranged/rifle/rifle_sg82.iff";
			break;
			case 9:  
			weapon = "object/weapon/ranged/rifle/rifle_spraystick.iff";
			break;
			case 10:  
			weapon = "object/weapon/ranged/rifle/rifle_e11.iff";
			break;
			case 11:  
			weapon = "object/weapon/ranged/rifle/rifle_jawa_ion.iff";
			break;
			case 12:  
			weapon = "object/weapon/ranged/rifle/rifle_t21.iff";
			break;
			case 13:  
			weapon = "object/weapon/ranged/rifle/rifle_tenloss_dxr6_disruptor_loot.iff";
			break;
			case 14:  
			weapon = "object/weapon/ranged/rifle/rifle_berserker.iff";
			break;
			case 15:  
			weapon = "object/weapon/ranged/rifle/rifle_bowcaster.iff";
			break;
			case 16:  
			weapon = "object/weapon/ranged/rifle/rifle_bowcaster_medium.iff";
			break;
			case 17:  
			weapon = "object/weapon/ranged/rifle/rifle_bowcaster_heavy.iff";
			break;
			case 18:  
			weapon = "object/weapon/ranged/rifle/rifle_beam.iff";
			break;
			case 19:  
			weapon = "object/weapon/ranged/rifle/rifle_acid_beam.iff";
			break;
			case 20:  
			weapon = "object/weapon/ranged/rifle/rifle_adventurer.iff";
			break;
			case 21:  
			weapon = "object/weapon/ranged/rifle/rifle_ld1.iff";
			break;
			case 22:  
			weapon = "object/weapon/ranged/rifle/rifle_massassi_ink.iff";
			break;
			case 23:  
			weapon = "object/weapon/ranged/rifle/rifle_proton.iff";
			break;
			case 24:  
			weapon = "object/weapon/ranged/rifle/rifle_lightning_light.iff";
			break;
			case 25:  
			weapon = "object/weapon/ranged/rifle/rifle_laser_light.iff";
			break;
			case 26:  
			weapon = "object/weapon/ranged/rifle/rifle_lightning_heavy.iff";
			break;
			case 27:  
			weapon = "object/weapon/ranged/rifle/som_rifle_dp23.iff";
			break;
			case 28:  
			weapon = "object/weapon/ranged/rifle/som_rifle_mustafar_disruptor.iff";
			break;
			case 29:  
			weapon = "object/weapon/ranged/rifle/rifle_tusken_elite.iff";
			break;
			default:
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		if (weapon != null && !weapon.equals(""))
		{
			if (static_item.isStaticItem(weapon))
			{
				static_item.createNewItemFunction(weapon, pInv);
			}
			else
			{
				createSnowFlakeFrogWeapon(player, weapon);
			}
		}
		refreshMenu (player, "Select the desired weapon option", "Character Builder Terminal", RIFLE_OPTIONS, "handleRifleSelect", false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleHeavySelect(obj_id self, dictionary params) throws InterruptedException
	{
		
		if ((params == null) || (params.isEmpty()))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		obj_id pInv = utils.getInventoryContainer(player);
		
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		if (btn == sui.BP_REVERT)
		{
			refreshMenu (player, "Select the desired weapon option", "Character Builder Terminal", WEAPON_OPTIONS, "handleWeaponOptions", false);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		if (idx == -1 || idx > HEAVY_WEAPON_OPTIONS.length)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(player))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (!isIdValid(pInv))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (getVolumeFree(pInv) <= 0)
		{
			sendSystemMessageTestingOnly(player, "Your Inventory is Full, please make room and try again.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		String weapon = "";
		
		switch(idx)
		{
			case 0:  
			weapon = "object/weapon/ranged/heavy/heavy_rocket_launcher.iff";
			break;
			case 1:  
			weapon = "object/weapon/ranged/rifle/rifle_flame_thrower.iff";
			break;
			case 2:  
			weapon = "object/weapon/ranged/rifle/rifle_acid_beam.iff";
			break;
			case 3:  
			weapon = "object/weapon/ranged/rifle/rifle_lightning.iff";
			break;
			case 4:  
			weapon = "object/weapon/ranged/heavy/heavy_acid_beam.iff";
			break;
			case 5:  
			weapon = "object/weapon/ranged/heavy/heavy_lightning_beam.iff";
			break;
			case 6:  
			weapon = "object/weapon/ranged/heavy/heavy_particle_beam.iff";
			break;
			case 7:  
			weapon = "object/weapon/ranged/rifle/rifle_flame_thrower_light.iff";
			break;
			case 8:  
			weapon = "object/weapon/ranged/heavy/som_republic_flamer.iff";
			break;
			case 9:  
			weapon = "object/weapon/ranged/heavy/som_lava_cannon.iff";
			break;
			case 10:
			static_item.createNewItemFunction("weapon_mandalorian_heavy_04_01",pInv);
			sendSystemMessageTestingOnly(player, "Crusader M-XX Heavy Rifle Issued.");
			break;
			case 11:
			static_item.createNewItemFunction("weapon_rebel_heavy_04_01",pInv);
			sendSystemMessageTestingOnly(player, "C-M 'Frag Storm' Heavy Shotgun Issued.");
			break;
			case 12:
			static_item.createNewItemFunction("weapon_tow_heavy_acid_beam_04_01",pInv);
			sendSystemMessageTestingOnly(player, "Devastator Acid Launcher Issued.");
			break;
			case 13:  
			weapon = "object/weapon/ranged/heavy/heavy_carbonite_rifle.iff";
			break;
			default:
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		if (weapon != null && !weapon.equals(""))
		{
			createSnowFlakeFrogWeapon(player, weapon);
		}
		
		refreshMenu (player, "Select the desired weapon option", "Character Builder Terminal", HEAVY_WEAPON_OPTIONS, "handleHeavySelect", false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleUnarmedSelect(obj_id self, dictionary params) throws InterruptedException
	{
		
		if ((params == null) || (params.isEmpty()))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		obj_id pInv = utils.getInventoryContainer(player);
		
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		if (btn == sui.BP_REVERT)
		{
			refreshMenu (player, "Select the desired weapon option", "Character Builder Terminal", WEAPON_OPTIONS, "handleWeaponOptions", false);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		if (idx == -1 || idx > UNARMED_OPTIONS.length)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(player))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (!isIdValid(pInv))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (getVolumeFree(pInv) <= 0)
		{
			sendSystemMessageTestingOnly(player, "Your Inventory is Full, please make room and try again.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		String weapon = "";
		
		switch(idx)
		{
			case 0:  
			weapon = "object/weapon/melee/special/vibroknuckler.iff";
			break;
			case 1:  
			weapon = "object/weapon/melee/special/massassiknuckler.iff";
			break;
			case 2:  
			weapon = "object/weapon/melee/special/blacksun_razor.iff";
			break;
			case 3:  
			weapon = "object/weapon/melee/special/blasterfist.iff";
			break;
			
			case 4:  
			static_item.createNewItemFunction("weapon_tow_blasterfist_04_01",pInv);
			sendSystemMessageTestingOnly(player, "Guardian Blaster Fist");
			break;
			default:
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		if (weapon != null && !weapon.equals(""))
		{
			createSnowFlakeFrogWeapon(player, weapon);
		}
		
		refreshMenu (player, "Select the desired weapon option", "Character Builder Terminal", UNARMED_OPTIONS, "handleUnarmedSelect", false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleOneHandedSelect(obj_id self, dictionary params) throws InterruptedException
	{
		
		if ((params == null) || (params.isEmpty()))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		obj_id pInv = utils.getInventoryContainer(player);
		
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		if (btn == sui.BP_REVERT)
		{
			refreshMenu (player, "Select the desired weapon option", "Character Builder Terminal", WEAPON_OPTIONS, "handleWeaponOptions", false);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		if (idx == -1 || idx > ONEHANDED_OPTIONS.length)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(player))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (!isIdValid(pInv))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (getVolumeFree(pInv) <= 0)
		{
			sendSystemMessageTestingOnly(player, "Your Inventory is Full, please make room and try again.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		String weapon = "";
		
		switch(idx)
		{
			case 0:  
			weapon = "object/weapon/melee/knife/knife_survival.iff";
			break;
			case 1:  
			weapon = "object/weapon/melee/knife/knife_dagger.iff";
			break;
			case 2:  
			weapon = "object/weapon/melee/sword/sword_01.iff";
			break;
			case 3:  
			weapon = "object/weapon/melee/sword/sword_02.iff";
			break;
			case 4:  
			weapon = "object/weapon/melee/baton/baton_gaderiffi.iff";
			break;
			case 5:  
			weapon = "object/weapon/melee/knife/knife_vibroblade.iff";
			break;
			case 6:  
			weapon = "object/weapon/melee/sword/sword_blade_ryyk.iff";
			break;
			case 7:  
			weapon = "object/weapon/melee/sword/sword_rantok.iff";
			break;
			case 8:  
			weapon = "object/weapon/melee/baton/baton_stun.iff";
			break;
			case 9:  
			weapon = "object/weapon/melee/sword/sword_acid.iff";
			break;
			case 10:  
			weapon = "object/weapon/melee/sword/sword_mace_junti.iff";
			break;
			case 11:  
			weapon = "object/weapon/melee/sword/sword_marauder.iff";
			break;
			case 12:  
			weapon = "object/weapon/melee/sword/sword_massassi.iff";
			break;
			case 13:  
			weapon = "object/weapon/melee/sword/sword_rsf.iff";
			break;
			case 14: 
			weapon = "object/weapon/melee/knife/knife_stone.iff";
			break;
			case 15: 
			weapon = "object/weapon/melee/knife/knife_janta.iff";
			break;
			case 16:
			weapon = "object/weapon/melee/knife/knife_donkuwah.iff";
			break;
			case 17: 
			weapon = "object/weapon/melee/sword/sword_curved_nyax.iff";
			break;
			case 18: 
			weapon = "object/weapon/melee/sword/som_sword_obsidian.iff";
			break;
			case 19: 
			weapon = "object/weapon/melee/sword/som_sword_mustafar_bandit.iff";
			break;
			case 20: 
			weapon = "object/weapon/melee/baton/baton_gaderiffi_elite.iff";
			break;
			default:
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		if (weapon != null && !weapon.equals(""))
		{
			createSnowFlakeFrogWeapon(player, weapon);
		}
		
		refreshMenu (player, "Select the desired weapon option", "Character Builder Terminal", ONEHANDED_OPTIONS, "handleOneHandedSelect", false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleTwoHandedSelect(obj_id self, dictionary params) throws InterruptedException
	{
		
		if ((params == null) || (params.isEmpty()))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		obj_id pInv = utils.getInventoryContainer(player);
		
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		if (btn == sui.BP_REVERT)
		{
			refreshMenu (player, "Select the desired weapon option", "Character Builder Terminal", WEAPON_OPTIONS, "handleWeaponOptions", false);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		if (idx == -1 || idx > TWOHANDED_OPTIONS.length)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(player))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (!isIdValid(pInv))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (getVolumeFree(pInv) <= 0)
		{
			sendSystemMessageTestingOnly(player, "Your Inventory is Full, please make room and try again.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		String weapon = "";
		
		switch(idx)
		{
			case 0:  
			weapon = "object/weapon/melee/axe/axe_heavy_duty.iff";
			break;
			case 1:  
			weapon = "object/weapon/melee/2h_sword/2h_sword_battleaxe.iff";
			break;
			case 2:  
			weapon = "object/weapon/melee/2h_sword/2h_sword_katana.iff";
			break;
			case 3:  
			weapon = "object/weapon/melee/axe/axe_vibroaxe.iff";
			break;
			case 4:  
			weapon = "object/weapon/melee/2h_sword/2h_sword_cleaver.iff";
			break;
			case 5:  
			weapon = "object/weapon/melee/2h_sword/2h_sword_maul.iff";
			break;
			case 6:  
			weapon = "object/weapon/melee/2h_sword/2h_sword_scythe.iff";
			break;
			case 7:  
			weapon = "object/weapon/melee/2h_sword/2h_sword_kashyyk.iff";
			break;
			case 8:  
			weapon = "object/weapon/melee/2h_sword/2h_sword_sith.iff";
			break;
			case 9: 
			weapon = "object/weapon/melee/sword/sword_nyax.iff";
			break;
			case 10: 
			weapon = "object/weapon/melee/2h_sword/2h_sword_blacksun_hack.iff";
			break;
			case 11: 
			weapon = "object/weapon/melee/2h_sword/som_2h_sword_tulrus.iff";
			break;
			case 12: 
			weapon = "object/weapon/melee/2h_sword/som_2h_sword_obsidian.iff";
			break;
			case 13: 
			weapon = "object/weapon/melee/2h_sword/2h_sword_kun_massassi.iff";
			break;
			default:
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		if (weapon != null && !weapon.equals(""))
		{
			createSnowFlakeFrogWeapon(player, weapon);
		}
		
		refreshMenu (player, "Select the desired weapon option", "Character Builder Terminal", TWOHANDED_OPTIONS, "handleTwoHandedSelect", false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handlePolearmSelect(obj_id self, dictionary params) throws InterruptedException
	{
		
		if ((params == null) || (params.isEmpty()))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		obj_id pInv = utils.getInventoryContainer(player);
		
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		if (btn == sui.BP_REVERT)
		{
			refreshMenu (player, "Select the desired weapon option", "Character Builder Terminal", WEAPON_OPTIONS, "handleWeaponOptions", false);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		if (idx == -1 || idx > POLEARM_OPTIONS.length)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(player))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (!isIdValid(pInv))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (getVolumeFree(pInv) <= 0)
		{
			sendSystemMessageTestingOnly(player, "Your Inventory is Full, please make room and try again.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		String weapon = "";
		
		switch(idx)
		{
			case 0:  
			weapon = "object/weapon/melee/polearm/lance_staff_wood_s1.iff";
			break;
			case 1:  
			weapon = "object/weapon/melee/polearm/lance_staff_metal.iff";
			break;
			case 2:  
			weapon = "object/weapon/melee/polearm/lance_staff_wood_s2.iff";
			break;
			case 3:  
			weapon = "object/weapon/melee/polearm/lance_vibrolance.iff";
			break;
			case 4:  
			weapon = "object/weapon/melee/polearm/polearm_vibro_axe.iff";
			break;
			case 5:  
			weapon = "object/weapon/melee/polearm/lance_controllerfp.iff";
			break;
			case 6:  
			weapon = "object/weapon/melee/polearm/lance_kaminoan.iff";
			break;
			case 7:  
			weapon = "object/weapon/melee/polearm/lance_massassi.iff";
			break;
			case 8:  
			weapon = "object/weapon/melee/polearm/lance_shock.iff";
			break;
			case 9:  
			weapon = "object/weapon/melee/polearm/lance_cryo.iff";
			break;
			case 10:  
			weapon = "object/weapon/melee/polearm/lance_kashyyk.iff";
			break;
			case 11:  
			weapon = "object/weapon/melee/polearm/polearm_lance_electric_polearm.iff";
			break;
			case 12:  
			weapon = "object/weapon/melee/polearm/lance_nightsister.iff";
			break;
			case 13:  
			weapon = "object/weapon/melee/polearm/lance_controllerfp_nightsister.iff";
			break;
			case 14:  
			weapon = "object/weapon/melee/polearm/som_lance_obsidian.iff";
			break;
			case 15:  
			weapon = "object/weapon/melee/polearm/som_lance_xandank.iff";
			break;
			case 16:  
			weapon = "object/weapon/melee/polearm/polearm_heroic_sd.iff";
			break;
			default:
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		if (weapon != null && !weapon.equals(""))
		{
			createSnowFlakeFrogWeapon(player, weapon);
		}
		
		refreshMenu (player, "Select the desired weapon option", "Character Builder Terminal", POLEARM_OPTIONS, "handlePolearmSelect", false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleBattlefieldSelect(obj_id self, dictionary params) throws InterruptedException
	{
		
		if ((params == null) || (params.isEmpty()))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		obj_id pInv = utils.getInventoryContainer(player);
		
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		if (btn == sui.BP_REVERT)
		{
			refreshMenu (player, "Select the desired weapon option", "Character Builder Terminal", WEAPON_OPTIONS, "handleWeaponOptions", false);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		if (idx == -1 || idx > BATTLEFIELD_WEAPON_OPTIONS.length)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(player))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (!isIdValid(pInv))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (getVolumeFree(pInv) <= 0)
		{
			sendSystemMessageTestingOnly(player, "Your Inventory is Full, please make room and try again.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		String weapon = "";
		
		switch(idx)
		{
			
			case 0:
			static_item.createNewItemFunction("weapon_westar_pistol_04_01",pInv);
			sendSystemMessageTestingOnly(player, "Westar-34 Blaster Pistol Issued.");
			break;
			case 1:
			static_item.createNewItemFunction("weapon_carbine_ngant_zarvel_04_01",pInv);
			sendSystemMessageTestingOnly(player, "NGant-Zarvel 9118 Carbine Issued.");
			break;
			case 2:
			static_item.createNewItemFunction("weapon_westar_rifle_04_01",pInv);
			sendSystemMessageTestingOnly(player, "Westar-M5 Blaster Rifle Issued.");
			break;
			case 3:
			static_item.createNewItemFunction("weapon_heavy_cr1_04_01",pInv);
			sendSystemMessageTestingOnly(player, "CR-1 Blast Cannon Issued.");
			break;
			
			case 4:
			static_item.createNewItemFunction("weapon_knuckler_buzz_knuck",pInv);
			sendSystemMessageTestingOnly(player, "Buzz-Knuck Issued.");
			break;
			case 5:
			static_item.createNewItemFunction("weapon_sword_1h_pvp_04_01",pInv);
			sendSystemMessageTestingOnly(player, "Sith Sword Issued.");
			break;
			case 6:
			static_item.createNewItemFunction("weapon_sword_2h_pvp_04_01",pInv);
			sendSystemMessageTestingOnly(player, "Vibrosword Issued.");
			break;
			case 7:
			static_item.createNewItemFunction("weapon_magna_guard_polearm_04_01",pInv);
			sendSystemMessageTestingOnly(player, "Magnaguard Electrostaff Issued.");
			break;
			case 8:
			static_item.createNewItemFunction("item_schematic_pvp_bf_saber_03_01",pInv);
			sendSystemMessageTestingOnly(player, "One-Handed Sith-Saber Hilt Schematic Issued.");
			break;
			case 9:
			static_item.createNewItemFunction("item_schematic_pvp_bf_saber_03_02",pInv);
			sendSystemMessageTestingOnly(player, "Two-Handed Mysterious Lightsaber Hilt Schematic Issued.");
			break;
			case 10:
			static_item.createNewItemFunction("item_schematic_pvp_bf_saber_03_03",pInv);
			sendSystemMessageTestingOnly(player, "Double-Bladed Darth Phobos Lightsaber Hilt Schematic Issued.");
			break;
			
			default:
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		refreshMenu (player, "Select the desired weapon option", "Character Builder Terminal", BATTLEFIELD_WEAPON_OPTIONS, "handleBattlefieldSelect", false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleGrenadeSelect(obj_id self, dictionary params) throws InterruptedException
	{
		
		if ((params == null) || (params.isEmpty()))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		obj_id pInv = utils.getInventoryContainer(player);
		
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		if (btn == sui.BP_REVERT)
		{
			refreshMenu (player, "Select the desired weapon option", "Character Builder Terminal", WEAPON_OPTIONS, "handleWeaponOptions", false);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		if (idx == -1 || idx > GRENADE_OPTIONS.length)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(player))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (!isIdValid(pInv))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (getVolumeFree(pInv) <= 0)
		{
			sendSystemMessageTestingOnly(player, "Your Inventory is Full, please make room and try again.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		obj_id item = null;
		switch(idx)
		{
			case 0:  
			item = createObject("object/weapon/ranged/grenade/grenade_fragmentation_light.iff", pInv, "");
			setCount(item, 500);
			sendSystemMessageTestingOnly(player, "Light Fragmentation Grenade with many charges Issued.");
			break;
			case 1:  
			item = createObject("object/weapon/ranged/grenade/grenade_fragmentation.iff", pInv, "");
			setCount(item, 500);
			sendSystemMessageTestingOnly(player, "Fragmentation Grenade with many charges Issued.");
			break;
			case 2:  
			item = createObject("object/weapon/ranged/grenade/grenade_imperial_detonator.iff", pInv, "");
			setCount(item, 500);
			sendSystemMessageTestingOnly(player, "Imperial Detonator with many charges Issued.");
			break;
			case 3:  
			item = createObject("object/weapon/ranged/grenade/grenade_proton.iff", pInv, "");
			setCount(item, 500);
			sendSystemMessageTestingOnly(player, "Proton Grenade with many charges Issued.");
			break;
			case 4:  
			item = createObject("object/weapon/ranged/grenade/grenade_thermal_detonator.iff", pInv, "");
			setCount(item, 500);
			sendSystemMessageTestingOnly(player, "Thermal Detonator with many charges Issued.");
			break;
			case 5:  
			item = createObject("object/weapon/ranged/grenade/grenade_glop.iff", pInv, "");
			setCount(item, 500);
			sendSystemMessageTestingOnly(player, "Glop Grenade with many charges Issued.");
			break;
			case 6:  
			item = createObject("object/weapon/ranged/grenade/grenade_cryoban.iff", pInv, "");
			setCount(item, 500);
			sendSystemMessageTestingOnly(player, "Cryoban Grenade with many charges Issued.");
			break;
			
			default:
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		refreshMenu (player, "Select the desired weapon option", "Character Builder Terminal", GRENADE_OPTIONS, "handleGrenadeSelect", false);
		return SCRIPT_CONTINUE;
	}
	
	
	public void handleArmorOption(obj_id player) throws InterruptedException
	{
		refreshMenu (player, "Select the desired armor option", "Character Builder Terminal", ARMOR_OPTIONS, "handleArmorSelect", false);
	}
	
	
	public int handleArmorSelect(obj_id self, dictionary params) throws InterruptedException
	{
		
		if ((params == null) || (params.isEmpty()))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		obj_id pInv = utils.getInventoryContainer(player);
		
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		if (btn == sui.BP_REVERT)
		{
			refreshMenu (player, "Select the desired character option", "Character Builder Terminal", CHARACTER_BUILDER_OPTIONS, "handleOptionSelect", true);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		if (idx == -1 || idx > ARMOR_OPTIONS.length)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(player))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		String prompt = "Select the desired armor option";
		String title = "Character Builder Terminal";
		int pid = 0;
		
		switch(idx)
		{
			
			case 0:
			refreshMenu (player, prompt, title, ARMOR_PROTECTION_AMOUNT, "handleProtectionAmount", false);
			utils.setScriptVar(player, "character_builder.armorType", 2);
			break;
			case 1:
			refreshMenu (player, prompt, title, ARMOR_PROTECTION_AMOUNT, "handleProtectionAmount", false);
			utils.setScriptVar(player, "character_builder.armorType", 1);
			break;
			case 2:
			refreshMenu (player, prompt, title, ARMOR_PROTECTION_AMOUNT, "handleProtectionAmount", false);
			utils.setScriptVar(player, "character_builder.armorType", 0);
			break;
			case 3:
			refreshMenu (player, prompt, title, ARMOR_PSG_OPTIONS, "handlePsgSelect", false);
			break;
			case 4:
			refreshMenu (player, prompt, title, ARMOR_ENHANCEMENT_OPTIONS, "handleEnhancementSelect", false);
			break;
			case 5:
			refreshMenu (player, prompt, title, ARMOR_PVP_SETS, "handlePvPSelect", false);
			break;
			case 6:
			refreshMenu (player, prompt, title, HEROIC_JEWELRY_LIST, "handleHeroicJewelrySelect", false);
			break;
			default:
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
			
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleProtectionAmount(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		obj_id pInv = utils.getInventoryContainer(player);
		
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		if (btn == sui.BP_REVERT)
		{
			refreshMenu (player, "Select the desired armor option", "Character Builder Terminal", ARMOR_OPTIONS, "handleArmorSelect", false);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		if (idx == -1 || idx > ARMOR_PROTECTION_AMOUNT.length)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(player))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (!isIdValid(pInv))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (getVolumeFree(pInv) <= 9)
		{
			sendSystemMessageTestingOnly(player, "Your Inventory is Full! Please free up at least 9 inventory slots and try again.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		utils.setScriptVar(player, "character_builder.armorLevel", idx +1);
		int type = utils.getIntScriptVar(player, "character_builder.armorType");
		String[] options = new String[0];
		String handler = "";
		int pid = 0;
		String prompt = "Select the desired armor level option";
		String title = "Character Builder Terminal";
		switch(type)
		{
			case 0:  
			options = ARMOR_RECON_OPTIONS;
			handler = "handleReconSelect";
			break;
			case 1:  
			options = ARMOR_BATTLE_OPTIONS;
			handler = "handleBattleSelect";
			break;
			case 2:  
			options = ARMOR_ASSAULT_OPTIONS;
			handler = "handleAssaultSelect";
			break;
			default:
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		utils.setScriptVar(player, "character_builder.armorOptions", options);
		utils.setScriptVar(player, "character_builder.armorHandler", handler);
		refreshMenu (player, prompt, title, options, handler, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleAssaultSelect(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		obj_id pInv = utils.getInventoryContainer(player);
		
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		if (btn == sui.BP_REVERT)
		{
			refreshMenu (player, "Select the desired armor option", "Character Builder Terminal", ARMOR_PROTECTION_AMOUNT, "handleProtectionAmount", false);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		if (idx == -1 || idx > ARMOR_ASSAULT_OPTIONS.length)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(player))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (!isIdValid(pInv))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (getVolumeFree(pInv) <= 9)
		{
			sendSystemMessageTestingOnly(player, "Your Inventory is Full! Please free up at least 9 inventory slots and try again.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		switch(idx)
		{
			case 0:  
			issueAssaultArmorSet(player,ARMOR_SET_ASSAULT_1);
			sendSystemMessageTestingOnly(player, "Composite Armor Set Issued.");
			break;
			case 1:  
			issueAssaultArmorSet(player,ARMOR_SET_ASSAULT_3);
			sendSystemMessageTestingOnly(player, "Marauder Assault Armor Set Issued.");
			break;
			case 2:  
			issueAssaultArmorSet(player,ARMOR_SET_ASSAULT_2);
			sendSystemMessageTestingOnly(player, "Chitin Armor Set Issued.");
			break;
			case 3:  
			issueAssaultArmorSet(player,ARMOR_SET_ASSAULT_4);
			sendSystemMessageTestingOnly(player, "Crafted Bounty Hunter Armor Set Issued.");
			break;
			case 4:  
			issueAssaultArmorSet(player,ARMOR_SET_ASSAULT_WOOKIEE);
			sendSystemMessageTestingOnly(player, "Kashyyykian Hunting Armor Set Issued.");
			break;
			case 5:  
			issueAssaultArmorSet(player,ARMOR_SET_ASSAULT_ITHORIAN);
			sendSystemMessageTestingOnly(player, "Ithorian Sentinel Armor Set Issued.");
			break;
			case 6:  
			issueAssaultArmorSet(player,ARMOR_SET_ASSAULT_IMPERIAL);
			sendSystemMessageTestingOnly(player, "Shocktrooper Armor Set Issued.");
			break;
			case 7:  
			issueAssaultArmorSet(player,ARMOR_SET_ASSAULT_REBEL);
			sendSystemMessageTestingOnly(player, "Rebel Assault Armor Set Issued.");
			break;
			default:
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		String[] options = utils.getStringArrayScriptVar(player, "character_builder.armorOptions");
		String handler = utils.getStringScriptVar(player, "character_builder.armorHandler");
		refreshMenu (player, "Select the desired armor level option", "Character Builder Terminal", options, handler, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleBattleSelect(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		obj_id pInv = utils.getInventoryContainer(player);
		
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		if (btn == sui.BP_REVERT)
		{
			refreshMenu (player, "Select the desired armor option", "Character Builder Terminal", ARMOR_PROTECTION_AMOUNT, "handleProtectionAmount", false);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		if (idx == -1 || idx > ARMOR_BATTLE_OPTIONS.length)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(player))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (!isIdValid(pInv))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (getVolumeFree(pInv) <= 9)
		{
			sendSystemMessageTestingOnly(player, "Your Inventory is Full! Please free up at least 9 inventory slots and try again.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		switch(idx)
		{
			case 0:  
			issueBattleArmorSet(player,ARMOR_SET_BATTLE_3);
			sendSystemMessageTestingOnly(player, "Padded Armor Set Issued.");
			break;
			case 1:  
			issueBattleArmorSet(player,ARMOR_SET_BATTLE_2);
			sendSystemMessageTestingOnly(player, "Marauder Battle Armor Set Issued.");
			break;
			case 2:  
			issueBattleArmorSet(player,ARMOR_SET_BATTLE_4);
			sendSystemMessageTestingOnly(player, "RIS Armor Set Issued.");
			break;
			case 3:  
			issueBattleArmorSet(player,ARMOR_SET_BATTLE_1);
			sendSystemMessageTestingOnly(player, "Bone Armor Set Issued.");
			break;
			case 4:  
			issueBattleArmorSet(player,ARMOR_SET_BATTLE_WOOKIEE);
			sendSystemMessageTestingOnly(player, "Kashyyykian Black Mountain Armor Set Issued.");
			break;
			case 5:  
			issueBattleArmorSet(player,ARMOR_SET_BATTLE_ITHORIAN);
			sendSystemMessageTestingOnly(player, "Ithorian Defender Armor Set Issued.");
			break;
			case 6:  
			issueBattleArmorSet(player,ARMOR_SET_BATTLE_IMPERIAL);
			sendSystemMessageTestingOnly(player, "Stormtrooper Armor Set Issued.");
			break;
			case 7:  
			issueBattleArmorSet(player,ARMOR_SET_BATTLE_REBEL);
			sendSystemMessageTestingOnly(player, "Rebel Battle Armor Set Issued.");
			break;
			case 8:  
			issueBattleArmorSet(player,ARMOR_SET_BATTLE_SNOWTROOPER);
			sendSystemMessageTestingOnly(player, "Imperial Snowtrooper Set Issued.");
			break;
			default:
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		String[] options = utils.getStringArrayScriptVar(player, "character_builder.armorOptions");
		String handler = utils.getStringScriptVar(player, "character_builder.armorHandler");
		refreshMenu (player, "Select the desired armor level option", "Character Builder Terminal", options, handler, false);
		return SCRIPT_CONTINUE;
	}
	
	public int handleReconSelect(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		obj_id pInv = utils.getInventoryContainer(player);
		
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		if (btn == sui.BP_REVERT)
		{
			refreshMenu (player, "Select the desired armor option", "Character Builder Terminal", ARMOR_PROTECTION_AMOUNT, "handleProtectionAmount", false);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		if (idx == -1 || idx > ARMOR_RECON_OPTIONS.length)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(player))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (!isIdValid(pInv))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (getVolumeFree(pInv) <= 9)
		{
			sendSystemMessageTestingOnly(player, "Your Inventory is Full! Please free up at least 9 inventory slots and try again.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		switch(idx)
		{
			case 0:  
			issueReconArmorSet(player,ARMOR_SET_RECON_3);
			sendSystemMessageTestingOnly(player, "Tantel Armor Set Issued.");
			break;
			case 1:  
			issueReconArmorSet(player,ARMOR_SET_RECON_4);
			sendSystemMessageTestingOnly(player, "Ubese Armor Set Issued.");
			break;
			case 2:  
			issueReconArmorSet(player,ARMOR_SET_RECON_1);
			sendSystemMessageTestingOnly(player, "Mabari Armor Set Issued.");
			break;
			case 3:  
			issueReconArmorSet(player,ARMOR_SET_RECON_2);
			sendSystemMessageTestingOnly(player, "Recon Marauder Armor Set Issued.");
			break;
			case 4:  
			issueReconArmorSet(player,ARMOR_SET_RECON_WOOKIEE);
			sendSystemMessageTestingOnly(player, "Kashyyykian Ceremonial Armor Set Issued.");
			break;
			case 5:  
			issueReconArmorSet(player,ARMOR_SET_RECON_ITHORIAN);
			sendSystemMessageTestingOnly(player, "Ithorian Guardian Armor Set Issued.");
			break;
			case 6:  
			issueReconArmorSet(player,ARMOR_SET_RECON_IMPERIAL);
			sendSystemMessageTestingOnly(player, "Scout Trooper Armor Set Issued.");
			break;
			case 7:  
			issueReconArmorSet(player,ARMOR_SET_RECON_REBEL);
			sendSystemMessageTestingOnly(player, "Rebel Marine Armor Set Issued.");
			break;
			case 8: 
			issueReconArmorSet(player,ARMOR_SET_REBEL_SNOW);
			sendSystemMessageTestingOnly(player, "Rebel Snow Armor Set Issued.");
			break;
			default:
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		String[] options = utils.getStringArrayScriptVar(player, "character_builder.armorOptions");
		String handler = utils.getStringScriptVar(player, "character_builder.armorHandler");
		refreshMenu (player, "Select the desired armor level option", "Character Builder Terminal", options, handler, false);
		return SCRIPT_CONTINUE;
	}
	
	public int handlePsgSelect(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		obj_id pInv = utils.getInventoryContainer(player);
		
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		if (btn == sui.BP_REVERT)
		{
			refreshMenu (player, "Select the desired armor option", "Character Builder Terminal", ARMOR_OPTIONS, "handleArmorSelect", false);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		if (idx == -1 || idx > ARMOR_PSG_OPTIONS.length)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(player))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (!isIdValid(pInv))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (getVolumeFree(pInv) <= 2)
		{
			sendSystemMessageTestingOnly(player, "Your Inventory is Full! Please free up at least 9 inventory slots and try again.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		obj_id item = null;
		switch(idx)
		{
			case 0:  
			item = createObject("object/tangible/component/armor/shield_generator_personal.iff", pInv, "");
			armor.initializePsg(item, 2.5f, 500, 10000);
			sendSystemMessageTestingOnly(player, "PSG Mark I Issued.");
			break;
			case 1:  
			item = createObject("object/tangible/component/armor/shield_generator_personal_b.iff", pInv, "");
			armor.initializePsg(item, 2.5f, 1000, 10000);
			sendSystemMessageTestingOnly(player, "PSG Mark II Issued.");
			break;
			case 2:  
			item = createObject("object/tangible/component/armor/shield_generator_personal_c.iff", pInv, "");
			armor.initializePsg(item, 2.5f, 2000, 10000);
			setSocketsUp(item);
			sendSystemMessageTestingOnly(player, "PSG Mark III Issued.");
			break;
			default:
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		refreshMenu (player, "Select the desired armor option", "Character Builder Terminal", ARMOR_PSG_OPTIONS, "handlePsgSelect", false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handlePvPSelect(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		obj_id pInv = utils.getInventoryContainer(player);
		
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		if (btn == sui.BP_REVERT)
		{
			refreshMenu (player, "Select the desired armor option", "Character Builder Terminal", ARMOR_PROTECTION_AMOUNT, "handleProtectionAmount", false);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		if (idx == -1 || idx > ARMOR_ASSAULT_OPTIONS.length)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(player))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (!isIdValid(pInv))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (getVolumeFree(pInv) <= 9)
		{
			sendSystemMessageTestingOnly(player, "Your Inventory is Full! Please free up at least 9 inventory slots and try again.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		switch(idx)
		{
			case 0:
			issuePvPSet(player, ARMOR_SET_PVP_IMPERIAL_BLACK);
			sendSystemMessageTestingOnly(player, "Imperial Black PvP Set Issued.");
			break;
			case 1:
			issuePvPSet(player,ARMOR_SET_PVP_IMPERIAL_WHITE);
			sendSystemMessageTestingOnly(player, "Imperial White PvP Set Issued.");
			break;
			case 2:
			issuePvPSet(player,ARMOR_SET_PVP_REBEL_GREY);
			sendSystemMessageTestingOnly(player, "Rebel Grey PvP Set Issued.");
			break;
			case 3:
			issuePvPSet(player,ARMOR_SET_PVP_REBEL_GREEN);
			sendSystemMessageTestingOnly(player, "Rebel Green PvP Set Issued.");
			break;
			case 4:  
			issueAssaultArmorSet(player,ARMOR_SET_ASSUALT_GALACTIC_MARINE);
			sendSystemMessageTestingOnly(player, "Galactic Marine Armor Set Issued.");
			break;
			case 5:  
			issueAssaultArmorSet(player,ARMOR_SET_ASSUALT_REBEL_SPECFORCE);
			sendSystemMessageTestingOnly(player, "Rebel SpecForce Armor Set Issued.");
			break;
			default:
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		String[] options = utils.getStringArrayScriptVar(player, "character_builder.armorOptions");
		String handler = utils.getStringScriptVar(player, "character_builder.armorHandler");
		refreshMenu (player, "Select the desired armor level option", "Character Builder Terminal", options, handler, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public void issuePvPSet(obj_id player, String[] armorPieces) throws InterruptedException
	{
		obj_id pInv = utils.getInventoryContainer(player);
		for (int i = 0; i < armorPieces.length; i++)
		{
			testAbortScript();
			static_item.createNewItemFunction(armorPieces[i], pInv);
		}
	}
	
	
	public void issueAssaultArmorSet(obj_id player, String[] armorPieces) throws InterruptedException
	{
		obj_id pInv = utils.getInventoryContainer(player);
		for (int j = 0; j < armorPieces.length; ++j)
		{
			testAbortScript();
			if (static_item.isStaticItem(armorPieces[j]))
			{
				obj_id armorItem = static_item.createNewItemFunction(armorPieces[j], pInv);
				
				if (hasScript(armorItem, "npc.faction_recruiter.biolink_item"))
				{
					setBioLink(armorItem, player);
				}
			}
			else
			{
				String armorTemplate = ARMOR_SET_PREFIX + armorPieces[j];
				obj_id armorItem = createObject(armorTemplate, pInv, "");
				if (isIdValid(armorItem))
				{
					if (!isGameObjectTypeOf(armorItem, GOT_armor_foot) && !isGameObjectTypeOf(armorItem, GOT_armor_hand))
					{
						armor.setArmorDataPercent(armorItem, 2, 2, utils.getIntScriptVar(player,"character_builder.armorLevel")*.33f, CONDITION);
						armor.setArmorSpecialProtectionPercent(armorItem, armor.DATATABLE_ASSAULT_LAYER, 1.0f);
					}
					setSocketsUp(armorItem);
				}
			}
			
		}
	}
	
	
	public void issueBattleArmorSet(obj_id player, String[] armorPieces) throws InterruptedException
	{
		obj_id pInv = utils.getInventoryContainer(player);
		for (int j = 0; j < armorPieces.length; ++j)
		{
			testAbortScript();
			if (static_item.isStaticItem(armorPieces[j]))
			{
				obj_id armorItem = static_item.createNewItemFunction(armorPieces[j], pInv);
				
				if (hasScript(armorItem, "npc.faction_recruiter.biolink_item"))
				{
					setBioLink(armorItem, player);
				}
			}
			else
			{
				String armorTemplate = ARMOR_SET_PREFIX + armorPieces[j];
				obj_id armorItem = createObject(armorTemplate, pInv, "");
				if (isIdValid(armorItem))
				{
					if (!isGameObjectTypeOf(armorItem, GOT_armor_foot) && !isGameObjectTypeOf(armorItem, GOT_armor_hand))
					{
						armor.setArmorDataPercent(armorItem, 2, 1, utils.getIntScriptVar(player,"character_builder.armorLevel")*.33f, CONDITION);
					}
					setSocketsUp(armorItem);
				}
			}
		}
	}
	
	
	public void issueReconArmorSet(obj_id player, String[] armorPieces) throws InterruptedException
	{
		obj_id pInv = utils.getInventoryContainer(player);
		for (int j = 0; j < armorPieces.length; ++j)
		{
			testAbortScript();
			if (static_item.isStaticItem(armorPieces[j]))
			{
				obj_id armorItem = static_item.createNewItemFunction(armorPieces[j], pInv);
				
				if (hasScript(armorItem, "npc.faction_recruiter.biolink_item"))
				{
					setBioLink(armorItem, player);
				}
			}
			else
			{
				String armorTemplate = ARMOR_SET_PREFIX + armorPieces[j];
				obj_id armorItem = createObject(armorTemplate, pInv, "");
				if (isIdValid(armorItem))
				{
					if (!isGameObjectTypeOf(armorItem, GOT_armor_foot) && !isGameObjectTypeOf(armorItem, GOT_armor_hand))
					{
						armor.setArmorDataPercent(armorItem, 2, 0, utils.getIntScriptVar(player,"character_builder.armorLevel")*.33f, CONDITION);
						armor.setArmorSpecialProtectionPercent(armorItem, armor.DATATABLE_RECON_LAYER, 1.0f);
					}
					setSocketsUp(armorItem);
				}
			}
		}
	}
	
	
	public void handleEnhancementSelect(obj_id player) throws InterruptedException
	{
		
		refreshMenu ( player, "Select the desired deed option", "Character Builder Terminal", ARMOR_ENHANCEMENT_OPTIONS, "handleEnhancementSelect", false);
	}
	
	
	public int handleEnhancementSelect(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		obj_id pInv = utils.getInventoryContainer(player);
		
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		if (btn == sui.BP_REVERT)
		{
			startCharacterBuilder(player);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		if (idx == -1 || idx > ARMOR_ENHANCEMENT_OPTIONS.length)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(player))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (!isIdValid(pInv))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (getVolumeFree(pInv) <= 0)
		{
			sendSystemMessageTestingOnly(player, "Your Inventory is Full, please make room and try again.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		switch(idx)
		{
			case 0:  
			if (isGod(player))
			{
				static_item.createNewItemFunction("item_development_combat_test_ring_06_01",pInv);
				sendSystemMessageTestingOnly(player, "Combat Enhancement Ring Issued");
			}
			else
			{
				sendSystemMessageTestingOnly(player, "You Must be in God Mode to enjoy the Combat Ring!");
			}
			break;
			default:
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		handleEnhancementSelect(player);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleHeroicJewelrySelect(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		obj_id pInv = utils.getInventoryContainer(player);
		
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		if (btn == sui.BP_REVERT)
		{
			startCharacterBuilder(player);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		if (idx == -1 || idx > HEROIC_JEWELRY_LIST.length)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(player))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (!isIdValid(pInv))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (getVolumeFree(pInv) <= 5)
		{
			sendSystemMessageTestingOnly(player, "Your Inventory is Full, please make room and try again.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		String column = "";
		
		switch(idx)
		{
			case 0:
			column = "set_bh_dps";
			break;
			case 1:
			column = "set_bh_utility_a";
			break;
			case 2:
			column = "set_bh_utility_b";
			break;
			case 3:
			column = "set_medic_dps";
			break;
			case 4:
			column = "set_medic_utility_a";
			break;
			case 5:
			column = "set_medic_utility_b";
			break;
			case 6:
			column = "set_jedi_dps";
			break;
			case 7:
			column = "set_jedi_utility_a";
			break;
			case 8:
			column = "set_jedi_utility_b";
			break;
			case 9:
			column = "set_commando_dps";
			break;
			case 10:
			column = "set_commando_utility_a";
			break;
			case 11:
			column = "set_commando_utility_b";
			break;
			case 12:
			column = "set_smuggler_dps";
			break;
			case 13:
			column = "set_smuggler_utility_a";
			break;
			case 14:
			column = "set_smuggler_utility_b";
			break;
			case 15:
			column = "set_spy_dps";
			break;
			case 16:
			column = "set_spy_utility_a";
			break;
			case 17:
			column = "set_spy_utility_b";
			break;
			case 18:
			column = "set_officer_dps";
			break;
			case 19:
			column = "set_officer_utility_a";
			break;
			case 20:
			column = "set_officer_utility_b";
			break;
			case 21:
			column = "set_hero";
			break;
			default:
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		if (column != null && !column.equals(""))
		{
			String[] itemSet = dataTableGetStringColumn(HEROIC_JEWELRY_SETS, column);
			
			if ((itemSet != null) && (itemSet.length != 0))
			{
				for (int i = 0; i < itemSet.length; i++)
				{
					testAbortScript();
					static_item.createNewItemFunction(itemSet[i], pInv);
				}
			}
		}
		
		refreshMenu( player, "Select the desired armor option", "Character Builder Terminal", HEROIC_JEWELRY_LIST, "handleHeroicJewelrySelect", false);
		return SCRIPT_CONTINUE;
		
	}
	
	
	public void handleMiscOption(obj_id player) throws InterruptedException
	{
		refreshMenu ( player, "Select the desired item option", "Character Builder Terminal", MISCITEM_OPTIONS, "handleMiscOptions", false);
	}
	
	
	public int handleMiscOptions(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		if (btn == sui.BP_REVERT)
		{
			startCharacterBuilder(player);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		if (idx == -1 || idx > MISCITEM_OPTIONS.length)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(player))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		String prompt = "Select the desired option";
		String promptTwo = "Select FIRST Skill Modifier, You will need to pick Three total times!";
		String title = "Character Builder Terminal";
		int pid = 0;
		
		switch(idx)
		{
			
			case 0:
			refreshMenu ( player, prompt, title, MISC_OPTIONS, "handleMiscSelect", false);
			break;
			case 1:
			refreshMenu ( player, prompt, title, MEDICINE_OPTIONS, "handleMedicineSelect", false);
			break;
			
			case 2:
			refreshMenu ( player, prompt, title, getExoticMods(), "handlePowerUpSelect", false);
			break;
			case 3:
			refreshMenu ( player, promptTwo, title, getExoticMods(), "handleAttachmentOneSelect", false);
			break;
			case 4:
			refreshMenu ( player, prompt, title, CLOTHING_OPTIONS, "handleClothingSelect", false);
			break;
			case 5:
			refreshMenu ( player, promptTwo, title, BASIC_MOD_LIST, "handleBasicArmorAttachmentOne", false);
			utils.setScriptVar(player, "character_builder.attachment", "object/tangible/gem/armor.iff");
			break;
			case 6:
			refreshMenu ( player, promptTwo, title, BASIC_MOD_LIST, "handleBasicArmorAttachmentOne", false);
			utils.setScriptVar(player, "character_builder.attachment", "object/tangible/gem/clothing.iff");
			break;
			case 7:
			refreshMenu ( player, prompt, title, CRAFTING_SUIT, "handleCraftingSuitSelect", false);
			break;
			case 8:
			refreshMenu ( player, prompt, title, AURILIA_CRYSTALS, "handleBuffCrystalSelect", false);
			break;
			
			default:
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
			
		}
		
		return SCRIPT_CONTINUE;
		
	}
	
	
	public int handleMiscSelect(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		obj_id pInv = utils.getInventoryContainer(player);
		
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		if (btn == sui.BP_REVERT)
		{
			handleMiscOption(player);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		if (idx == -1 || idx > MISC_OPTIONS.length)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(player))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (!isIdValid(pInv))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (getVolumeFree(pInv) <= 0)
		{
			sendSystemMessageTestingOnly(player, "Your Inventory is Full, please make room and try again.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		switch(idx)
		{
			case 0:  
			createObject("object/tangible/wearables/backpack/backpack_s06.iff", pInv, "");
			sendSystemMessageTestingOnly(player, "Spec-Ops Pack Issued.");
			break;
			case 1:  
			createObject("object/tangible/wearables/backpack/backpack_krayt_skull.iff", pInv, "");
			sendSystemMessageTestingOnly(player, "Krayt Pack Issued.");
			break;
			case 2:  
			createObject("object/tangible/wearables/backpack/backpack_s09.iff", pInv, "");
			sendSystemMessageTestingOnly(player, "Pilot Ace Pack Issued.");
			break;
			case 3:  
			static_item.createNewItemFunction("item_fannypack_04_01", pInv);
			sendSystemMessageTestingOnly(player, "Jedi Belt of Master Bodo Baas Issued.");
			break;
			case 4:
			static_item.createNewItemFunction("armor_snowtrooper_backpack", pInv);
			sendSystemMessageTestingOnly(player, "Snowtrooper Backpack Issued.");
			break;
			case 5:
			static_item.createNewItemFunction("armor_rebel_snow_backpack", pInv);
			sendSystemMessageTestingOnly(player, "Alliance Cold Weather Backpack Issued.");
			break;
			case 6:  
			createObject("object/tangible/mission/mission_bounty_droid_probot.iff", pInv, "");
			sendSystemMessageTestingOnly(player, "Arakyd Probe Droid Issued.");
			break;
			case 7:  
			createObject("object/tangible/mission/mission_bounty_droid_seeker.iff", pInv, "");
			sendSystemMessageTestingOnly(player, "Seeker Droid Issued.");
			break;
			case 8:  
			static_item.createNewItemFunction("item_limited_use_schematic_bounty_ee3_04_01", pInv);
			sendSystemMessageTestingOnly(player, "Master Crafted EE3 Draft Schematic Issued.");
			break;
			case 9:  
			static_item.createNewItemFunction("item_limited_use_schematic_bounty_dc15_04_01", pInv);
			sendSystemMessageTestingOnly(player, "Master Crafted DC-15 Draft Schematic Issued.");
			break;
			case 10:  
			createObject("object/tangible/veteran_reward/resource.iff", pInv, "");
			sendSystemMessageTestingOnly(player, "Resource Kit Issued");
			default:
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
			
		}
		
		refreshMenu ( player, "Select the desired option", "Character Builder Terminal", MISC_OPTIONS, "handleMiscSelect", false);
		return SCRIPT_CONTINUE;
		
	}
	
	
	public int handleCraftingSuitSelect(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		obj_id pInv = utils.getInventoryContainer(player);
		
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		if (btn == sui.BP_REVERT)
		{
			handleMiscOption(player);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		if (idx == -1 || idx > CRAFTING_SUIT.length)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(player))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (!isIdValid(pInv))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (getVolumeFree(pInv) <= 0)
		{
			sendSystemMessageTestingOnly(player, "Your Inventory is Full, please make room and try again.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		switch(idx)
		{
			case 0:  
			obj_id suit = static_item.createNewItemFunction("item_god_craftingsuit_06_01", pInv);
			if (isIdValid(suit))
			{
				sendSystemMessageTestingOnly(player, "Blix's Ultra Crafting Suit Issued, May you see nothing but Amazing Crafts!");
			}
			break;
			
			default:
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
			
		}
		
		refreshMenu ( player, "Select the desired option", "Character Builder Terminal", CRAFTING_SUIT, "handleCraftingSuitSelect", false);
		return SCRIPT_CONTINUE;
		
	}
	
	
	public int handleSmugglerSelect(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		obj_id pInv = utils.getInventoryContainer(player);
		
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		if (btn == sui.BP_REVERT)
		{
			handleMiscOption(player);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		if (idx == -1 || idx > SMUGGLER_TOOLS_OPTIONS.length)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(player))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (!isIdValid(pInv))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (getVolumeFree(pInv) <= 0)
		{
			sendSystemMessageTestingOnly(player, "Your Inventory is Full, please make room and try again.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		switch(idx)
		{
			case 0:  
			createObject("object/tangible/smuggler/simple_toolkit.iff", pInv, "");
			sendSystemMessageTestingOnly(player, "Simple Toolkit Issued.");
			break;
			case 1:  
			createObject("object/tangible/smuggler/finely_crafted_toolset.iff", pInv, "");
			sendSystemMessageTestingOnly(player, "Finely Crafted Toolset Issued.");
			break;
			case 2:  
			createObject("object/tangible/smuggler/trandoshan_interframe.iff", pInv, "");
			sendSystemMessageTestingOnly(player, "Trandoshan Interframe Issued.");
			break;
			case 3:  
			createObject("object/tangible/smuggler/delicate_trigger_assembly.iff", pInv, "");
			sendSystemMessageTestingOnly(player, "Delicate Trigger Issued.");
			break;
			case 4:  
			createObject("object/tangible/smuggler/illegal_core_booster.iff", pInv, "");
			sendSystemMessageTestingOnly(player, "Illegal Core Booster Issued.");
			break;
			case 5:  
			createObject("object/tangible/smuggler/mandalorian_interframe.iff", pInv, "");
			sendSystemMessageTestingOnly(player, "Mandalorian Interframe Issued.");
			break;
			case 6:  
			createObject("object/tangible/smuggler/reactive_micro_plating.iff", pInv, "");
			sendSystemMessageTestingOnly(player, "Micro Plating Issued.");
			break;
			default:
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
			
		}
		
		refreshMenu ( player, "Select the desired option", "Character Builder Terminal", SMUGGLER_TOOLS_OPTIONS, "handleSmugglerSelect", false);
		return SCRIPT_CONTINUE;
		
	}
	
	
	public String[] getExoticMods() throws InterruptedException
	{
		String[] skillMods = dataTableGetStringColumn(EXOTIC_SKILL_MODS, "name");
		
		for (int i = 0; i < skillMods.length; i++)
		{
			testAbortScript();
			skillMods[i] = utils.packStringId(new string_id("stat_n", skillMods[i]));
		}
		
		return skillMods;
	}
	
	
	public int handlePowerUpSelect(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		obj_id pInv = utils.getInventoryContainer(player);
		
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		String[] skillMods = dataTableGetStringColumn(EXOTIC_SKILL_MODS, "name");
		
		if (btn == sui.BP_REVERT)
		{
			handleMiscOption(player);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		if (idx == -1 || idx > skillMods.length)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(player))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (!isIdValid(pInv))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (getVolumeFree(pInv) <= 0)
		{
			sendSystemMessageTestingOnly(player, "Your Inventory is Full, please make room and try again.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		String skillMod = dataTableGetString(EXOTIC_SKILL_MODS, idx, "name");
		int powerRatio = dataTableGetInt(EXOTIC_SKILL_MODS, idx, "ratio");
		int power = 75;
		
		obj_id armorPower = static_item.createNewItemFunction("item_reverse_engineering_powerup_armor_02_01", pInv);
		obj_id shirtPower = static_item.createNewItemFunction("item_reverse_engineering_powerup_clothing_02_01", pInv);
		obj_id weaponPower = static_item.createNewItemFunction("item_reverse_engineering_powerup_weapon_02_01", pInv);
		
		if (isIdValid(armorPower))
		{
			setObjVar(armorPower, "reverse_engineering.reverse_engineering_modifier", skillMod);
			setObjVar(armorPower, "reverse_engineering.reverse_engineering_ratio", powerRatio);
			setObjVar(armorPower, "reverse_engineering.reverse_engineering_power", power);
			setCount(armorPower, 350);
			sendSystemMessageTestingOnly(player, "Armor Power Up Issued");
		}
		
		if (isIdValid(shirtPower))
		{
			setObjVar(shirtPower, "reverse_engineering.reverse_engineering_modifier", skillMod);
			setObjVar(shirtPower, "reverse_engineering.reverse_engineering_ratio", powerRatio);
			setObjVar(shirtPower, "reverse_engineering.reverse_engineering_power", power);
			setCount(shirtPower, 350);
			sendSystemMessageTestingOnly(player, "Shirt Power Up Issued");
		}
		
		if (isIdValid(weaponPower))
		{
			setObjVar(weaponPower, "reverse_engineering.reverse_engineering_modifier", skillMod);
			setObjVar(weaponPower, "reverse_engineering.reverse_engineering_ratio", powerRatio);
			setObjVar(weaponPower, "reverse_engineering.reverse_engineering_power", power);
			setCount(weaponPower, 350);
			sendSystemMessageTestingOnly(player, "Weapon Power Up Issued");
		}
		
		refreshMenu ( player, "Select the desired option", "Character Builder Terminal", getExoticMods(), "handlePowerUpSelect", false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleAttachmentOneSelect(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		obj_id pInv = utils.getInventoryContainer(player);
		
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		String[] skillMods = dataTableGetStringColumn(EXOTIC_SKILL_MODS, "name");
		
		String prompt = "Select Second Skill Modifier, You will need to pick one more time!";
		String title = "Character Builder Terminal";
		
		if (btn == sui.BP_REVERT)
		{
			handleMiscOption(player);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		if (idx == -1 || idx > skillMods.length)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		utils.setScriptVar(player, "character_builder.modTypeOne", idx);
		
		refreshMenu ( player, prompt, title, getExoticMods(), "handleAttachmentTwoSelect", false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleAttachmentTwoSelect(obj_id self, dictionary params) throws InterruptedException
	{
		
		if ((params == null) || (params.isEmpty()))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		obj_id pInv = utils.getInventoryContainer(player);
		
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		String[] skillMods = dataTableGetStringColumn(EXOTIC_SKILL_MODS, "name");
		
		String prompt = "Select Final Skill Modifier";
		String title = "Character Builder Terminal";
		
		if (btn == sui.BP_REVERT)
		{
			handleMiscOption(player);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		if (idx == -1 || idx > skillMods.length)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		utils.setScriptVar(player, "character_builder.modTypeTwo", idx);
		
		refreshMenu ( player, prompt, title, getExoticMods(), "handleAttachmentsSelect", false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleAttachmentsSelect(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		obj_id pInv = utils.getInventoryContainer(player);
		
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		String[] skillMods = dataTableGetStringColumn(EXOTIC_SKILL_MODS, "name");
		
		if (btn == sui.BP_REVERT)
		{
			handleMiscOption(player);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		if (idx == -1 || idx > skillMods.length)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		utils.setScriptVar(player, "character_builder.modTypeThree", idx);
		
		if (!isIdValid(player))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (!isIdValid(pInv))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (getVolumeFree(pInv) <= 0)
		{
			sendSystemMessageTestingOnly(player, "Your Inventory is Full, please make room and try again.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		int skillModOne = utils.getIntScriptVar(player, "character_builder.modTypeOne");
		int skillModTwo = utils.getIntScriptVar(player, "character_builder.modTypeTwo");
		int skillModThree = utils.getIntScriptVar(player, "character_builder.modTypeThree");
		
		String stringSkillModOne = dataTableGetString(EXOTIC_SKILL_MODS, skillModOne, "name");
		String stringSkillModTwo = dataTableGetString(EXOTIC_SKILL_MODS, skillModTwo, "name");
		String stringSkillModThree = dataTableGetString(EXOTIC_SKILL_MODS, skillModThree, "name");
		
		int powerRatioOne = dataTableGetInt(EXOTIC_SKILL_MODS, skillModOne, "ratio");
		int powerRatioTwo = dataTableGetInt(EXOTIC_SKILL_MODS, skillModTwo, "ratio");
		int powerRatioThree = dataTableGetInt(EXOTIC_SKILL_MODS, skillModThree, "ratio");
		
		int power = 35;
		int powerOne = power/powerRatioOne;
		int powerTwo = power/powerRatioTwo;
		int powerThree = power/powerRatioThree;
		
		obj_id armorPower = createObject("object/tangible/gem/bp_armor_only.iff", pInv, "");
		obj_id shirtPower = createObject("object/tangible/gem/shirt_only.iff", pInv, "");
		obj_id weaponPower = createObject("object/tangible/gem/weapon.iff", pInv, "");
		
		if (isIdValid(armorPower))
		{
			setObjVar(armorPower, "skillmod.bonus."+ stringSkillModOne, powerOne);
			setObjVar(armorPower, "skillmod.bonus."+ stringSkillModTwo, powerTwo);
			setObjVar(armorPower, "skillmod.bonus."+ stringSkillModThree, powerThree);
			setObjVar(armorPower, "reverse_engineering.attachment_level", 2);
			sendSystemMessageTestingOnly(player, "Exotic Armor Attachment Issued");
		}
		
		if (isIdValid(shirtPower))
		{
			setObjVar(shirtPower, "skillmod.bonus."+ stringSkillModOne, powerOne);
			setObjVar(shirtPower, "skillmod.bonus."+ stringSkillModTwo, powerTwo);
			setObjVar(shirtPower, "skillmod.bonus."+ stringSkillModThree, powerThree);
			setObjVar(shirtPower, "reverse_engineering.attachment_level", 2);
			sendSystemMessageTestingOnly(player, "Exotic Shirt Attachment Issued");
		}
		
		if (isIdValid(weaponPower))
		{
			setObjVar(weaponPower, "skillmod.bonus."+ stringSkillModOne, powerOne);
			setObjVar(weaponPower, "skillmod.bonus."+ stringSkillModTwo, powerTwo);
			setObjVar(weaponPower, "skillmod.bonus."+ stringSkillModThree, powerThree);
			setObjVar(weaponPower, "reverse_engineering.attachment_level", 2);
			sendSystemMessageTestingOnly(player, "Exotic Weapon Attachment Issued");
		}
		
		refreshMenu ( player, "Select the desired option", "Character Builder Terminal", MISCITEM_OPTIONS, "handleMiscOptions", false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleClothingSelect(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		obj_id pInv = utils.getInventoryContainer(player);
		
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		if (btn == sui.BP_REVERT)
		{
			handleMiscOption(player);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		if (idx == -1 || idx > CLOTHING_OPTIONS.length)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(player))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (!isIdValid(pInv))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (getVolumeFree(pInv) <= 0)
		{
			sendSystemMessageTestingOnly(player, "Your Inventory is Full, please make room and try again.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		String clothing = "";
		
		switch(idx)
		{
			case 0:  
			clothing = "object/tangible/wearables/shirt/npe_shirt.iff";
			break;
			case 1:  
			clothing = "object/tangible/wearables/gloves/gloves_s14.iff";
			break;
			case 2:  
			clothing = "object/tangible/wearables/hat/hat_s02.iff";
			break;
			case 3:  
			clothing = "object/tangible/wearables/pants/pants_s07.iff";
			break;
			case 4:  
			clothing = "object/tangible/wearables/jacket/jacket_s13.iff";
			break;
			case 5:  
			clothing = "object/tangible/wearables/boots/boots_s05.iff";
			break;
			default:
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		if (clothing != null && !clothing.equals(""))
		{
			obj_id clothingObject = createObject(clothing, pInv, "");
			
			if (isIdValid(clothingObject))
			{
				setSocketsUp(clothingObject);
				sendSystemMessageTestingOnly(player, "Clothing Issued");
			}
		}
		
		refreshMenu ( player, "Select the desired option", "Character Builder Terminal", CLOTHING_OPTIONS, "handleClothingSelect", false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleBasicArmorAttachmentOne(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		obj_id pInv = utils.getInventoryContainer(player);
		
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		String prompt = "Select Second Skill Modifier, You will need to pick one more time!";
		String title = "Character Builder Terminal";
		
		if (btn == sui.BP_REVERT)
		{
			handleMiscOption(player);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		if (idx == -1 || idx > BASIC_MOD_LIST.length)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		utils.setScriptVar(player, "character_builder.basicModTypeOne", idx);
		
		refreshMenu ( player, prompt, title, BASIC_MOD_LIST, "handleBasicArmorAttachmentTwo", false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleBasicArmorAttachmentTwo(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		obj_id pInv = utils.getInventoryContainer(player);
		
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		String prompt = "Select Final Skill Modifier";
		String title = "Character Builder Terminal";
		
		if (btn == sui.BP_REVERT)
		{
			handleMiscOption(player);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		if (idx == -1 || idx > BASIC_MOD_LIST.length)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		utils.setScriptVar(player, "character_builder.basicModTypeTwo", idx);
		
		refreshMenu ( player, prompt, title, BASIC_MOD_LIST, "handleBasicArmorAttachment", false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleBasicArmorAttachment(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		obj_id pInv = utils.getInventoryContainer(player);
		
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		if (btn == sui.BP_REVERT)
		{
			handleMiscOption(player);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		if (idx == -1 || idx > BASIC_MOD_LIST.length)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		utils.setScriptVar(player, "character_builder.basicModTypeThree", idx);
		
		if (!isIdValid(player))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (!isIdValid(pInv))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (getVolumeFree(pInv) <= 0)
		{
			sendSystemMessageTestingOnly(player, "Your Inventory is Full, please make room and try again.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		int skillModOne = utils.getIntScriptVar(player, "character_builder.basicModTypeOne");
		int skillModTwo = utils.getIntScriptVar(player, "character_builder.basicModTypeTwo");
		int skillModThree = utils.getIntScriptVar(player, "character_builder.basicModTypeThree");
		
		String stringSkillModOne = BASIC_MOD_STRINGS[skillModOne];
		String stringSkillModTwo = BASIC_MOD_STRINGS[skillModTwo];
		String stringSkillModThree = BASIC_MOD_STRINGS[skillModThree];
		
		int powerRatio = 1;
		int power = 35;
		power = power/powerRatio;
		
		String attachment = utils.getStringScriptVar(player, "character_builder.attachment");
		obj_id armorPower = createObject(attachment, pInv, "");
		
		if (isIdValid(armorPower))
		{
			setObjVar(armorPower, "skillmod.bonus."+ stringSkillModOne, power);
			setObjVar(armorPower, "skillmod.bonus."+ stringSkillModTwo, power);
			setObjVar(armorPower, "skillmod.bonus."+ stringSkillModThree, power);
			setObjVar(armorPower, "reverse_engineering.attachment_level", 2);
			sendSystemMessageTestingOnly(player, "Basic Armor Attachment Issued");
			cleanScriptVars(player);
		}
		
		refreshMenu ( player, "Select the desired option", "Character Builder Terminal", MISCITEM_OPTIONS, "handleMiscOptions", false);
		return SCRIPT_CONTINUE;
		
	}
	
	
	public int handleBuffCrystalSelect(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		obj_id pInv = utils.getInventoryContainer(player);
		
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		if (btn == sui.BP_REVERT)
		{
			handleMiscOption(player);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		if (idx == -1 || idx > AURILIA_CRYSTALS.length)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(player))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (!isIdValid(pInv))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (getVolumeFree(pInv) <= 0)
		{
			sendSystemMessageTestingOnly(player, "Your Inventory is Full, please make room and try again.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		String crystal = "";
		
		switch(idx)
		{
			case 0:
			crystal = "item_aurilia_buff_crystal_03_01";
			break;
			case 1:
			crystal = "item_aurilia_buff_crystal_03_02";
			break;
			case 2:
			crystal = "item_aurilia_buff_crystal_03_03";
			break;
			default:
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		if (crystal != null && !crystal.equals(""))
		{
			if (isGod(player))
			{
				static_item.createNewItemFunction(crystal, pInv);
			}
			else
			{
				sendSystemMessageTestingOnly(player, "You Must be in God Mode to enjoy the Aurilia Crystals!");
			}
		}
		
		refreshMenu ( player, "Select the desired option", "Character Builder Terminal", AURILIA_CRYSTALS, "handleBuffCrystalSelect", false);
		return SCRIPT_CONTINUE;
		
	}
	
	
	public int handleMedicineSelect(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		obj_id pInv = utils.getInventoryContainer(player);
		
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		if (btn == sui.BP_REVERT)
		{
			handleMiscOption(player);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		if (idx == -1 || idx > MEDICINE_OPTIONS.length)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(player))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (!isIdValid(pInv))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (getVolumeFree(pInv) <= 0)
		{
			sendSystemMessageTestingOnly(player, "Your Inventory is Full, please make room and try again.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		switch(idx)
		{
			case 0:  
			obj_id stima = createObject("object/tangible/medicine/instant_stimpack/stimpack_a.iff", pInv, "");
			if (isIdValid(stima))
			{
				setCount(stima, 350);
				setObjVar(stima, "healing.power", 250);
				sendSystemMessageTestingOnly(player, "High Charge Instant Stimpack-A Issued!");
			}
			else
			{
				sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction, Invalid Stim.");
			}
			break;
			case 1:  
			obj_id stimb = createObject("object/tangible/medicine/instant_stimpack/stimpack_b.iff", pInv, "");
			if (isIdValid(stimb))
			{
				setCount(stimb, 350);
				setObjVar(stimb, "healing.power", 400);
				sendSystemMessageTestingOnly(player, "High Charge Instant Stimpack-B Issued!");
			}
			else
			{
				sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction, Invalid Stim.");
			}
			break;
			case 2:  
			obj_id stimc = createObject("object/tangible/medicine/instant_stimpack/stimpack_c.iff", pInv, "");
			if (isIdValid(stimc))
			{
				setCount(stimc, 350);
				setObjVar(stimc, "healing.power", 700);
				sendSystemMessageTestingOnly(player, "High Charge Instant Stimpack-C Issued!");
			}
			else
			{
				sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction, Invalid Stim.");
			}
			break;
			case 3:  
			obj_id stimd = createObject("object/tangible/medicine/instant_stimpack/stimpack_d.iff", pInv, "");
			if (isIdValid(stimd))
			{
				setCount(stimd, 350);
				setObjVar(stimd, "healing.power", 1200);
				sendSystemMessageTestingOnly(player, "High Charge Instant Stimpack-D Issued!");
			}
			else
			{
				sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction, Invalid Stim.");
			}
			break;
			
			default:
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		refreshMenu ( player, "Select the desired option", "Character Builder Terminal", MEDICINE_OPTIONS, "handleMedicineSelect", false);
		return SCRIPT_CONTINUE;
	}
	
	
	public void handleHealOption(obj_id player) throws InterruptedException
	{
		refreshMenu ( player, "Select the desired item option", "Character Builder Terminal", HEAL_OPTIONS, "handleHealOptions", false);
	}
	
	
	public int handleHealOptions(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		if (btn == sui.BP_REVERT)
		{
			refreshMenu (player, "Select the desired character option", "Character Builder Terminal", CHARACTER_BUILDER_OPTIONS, "handleOptionSelect", true);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		if (idx == -1 || idx > HEAL_OPTIONS.length)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(player))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		switch(idx)
		{
			
			case 0:
			{
				sendSystemMessageTestingOnly(player, "You have been licked by a Frog! All wounds have been Healed!");
			}
			break;
			case 1:
			healShockWound(player, 1000);
			sendSystemMessageTestingOnly(player, "You have been licked by a Frog! All Battle Fatigue has been Healed!");
			break;
			default:
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
			
		}
		
		refreshMenu (player, "Select the desired option", "Character Builder Terminal", HEAL_OPTIONS, "handleHealOptions", false);
		return SCRIPT_CONTINUE;
	}
	
	
	public void handleSpaceMiningOption(obj_id player) throws InterruptedException
	{
		obj_id self = getSelf();
		
		skill.grantSkill(player, "pilot_rebel_navy_novice");
		skill.grantSkill(player, "pilot_rebel_navy_starships_01");
		skill.grantSkill(player, "pilot_rebel_navy_starships_02");
		skill.grantSkill(player, "pilot_rebel_navy_starships_03");
		skill.grantSkill(player, "pilot_rebel_navy_starships_04");
		skill.grantSkill(player, "pilot_rebel_navy_weapons_01");
		skill.grantSkill(player, "pilot_rebel_navy_weapons_02");
		skill.grantSkill(player, "pilot_rebel_navy_weapons_03");
		skill.grantSkill(player, "pilot_rebel_navy_weapons_04");
		skill.grantSkill(player, "pilot_rebel_navy_procedures_01");
		skill.grantSkill(player, "pilot_rebel_navy_procedures_02");
		skill.grantSkill(player, "pilot_rebel_navy_procedures_03");
		skill.grantSkill(player, "pilot_rebel_navy_procedures_04");
		skill.grantSkill(player, "pilot_rebel_navy_droid_01");
		skill.grantSkill(player, "pilot_rebel_navy_droid_02");
		skill.grantSkill(player, "pilot_rebel_navy_droid_03");
		skill.grantSkill(player, "pilot_rebel_navy_droid_04");
		skill.grantSkill(player, "pilot_rebel_navy_master");
		
		attachScript(player, "wwallace.space_mining_test");
		
		obj_id objInventory = utils.getInventoryContainer(player);
		sendSystemMessageTestingOnly(player, "Granting a mining vessel...and launching you to spaaaace!");
		obj_id weapon1 = createObjectOverloaded("object/tangible/ship/components/weapon/wpn_mining_laser_mk2.iff", objInventory);
		obj_id weapon2 = createObjectOverloaded("object/tangible/ship/components/weapon/wpn_tractor_pulse_gun.iff", objInventory);
		obj_id cargoHold = createObjectOverloaded("object/tangible/ship/components/cargo_hold/crg_starfighter_large.iff", objInventory);
		
		obj_id newship = space_utils.createShipControlDevice(player, "xwing", true);
		
		if (isIdValid(newship))
		{
			obj_id ship = space_transition.getShipFromShipControlDevice(newship);
			obj_id comp1 = shipUninstallComponent(player, ship , ship_chassis_slot_type.SCST_weapon_0, objInventory);
			destroyObject(comp1);
			shipInstallComponent(player, ship, ship_chassis_slot_type.SCST_weapon_0, weapon1);
			obj_id comp2 = shipUninstallComponent(player, ship , ship_chassis_slot_type.SCST_weapon_1, objInventory);
			destroyObject(comp2);
			shipInstallComponent(player, ship, ship_chassis_slot_type.SCST_weapon_1, weapon2);
			shipInstallComponent(player, ship, ship_chassis_slot_type.SCST_cargo_hold, cargoHold);
			
			location space = new location(0, 0, 0, "space_tatooine");
			location ground = getLocation(player);
			
			launch(player, ship, null, space, ground);
		}
	}
	
	
	public void handleFactionOption(obj_id player) throws InterruptedException
	{
		obj_id self = getSelf();
		
		refreshMenu ( player, "Select the desired faction option", "Character Builder Terminal", FACTION_OPTIONS, "handleFactionOptions", false);
	}
	
	
	public int handleFactionOptions(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		if (idx == -1 || idx > FACTION_OPTIONS.length)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(player))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		String prompt = "Select the desired option";
		String title = "Character Builder Terminal";
		int pid = 0;
		String factionName = factions.getFaction(player);
		int current_rank = pvpGetCurrentGcwRank(player);
		switch(idx)
		{
			
			case 0:
			if (factionName == null)
			{
				sendSystemMessageTestingOnly(player, "You must declare a Faction before receiving Points!");
				return SCRIPT_OVERRIDE;
			}
			int standing = (int)factions.getFactionStanding(player, factionName);
			if (standing < 25000)
			{
				factions.addFactionStanding(player, factionName, FACTION_AMT);
			}
			else
			{
				sendSystemMessageTestingOnly(player, "You already have a large amount of unspent faction points. Why do you need any more?");
			}
			break;
			case 1:
			if (space_flags.isImperialPilot(player))
			{
				sendSystemMessageTestingOnly(player, "You are an Imperial Pilot! You must surrender your current space faction before you become a Rebel!");
				return SCRIPT_OVERRIDE;
			}
			if (factionName == null)
			{
				pvpSetAlignedFaction(player, (370444368));
				pvpMakeCovert(player);
				sendSystemMessageTestingOnly(player, "Faction Set. You are now a Covert Rebel!");
			}
			else if (factionName.equals("Imperial"))
			{
				sendSystemMessageTestingOnly(player, "You are an Imperial! You must surrender your current faction before you become a Rebel!");
			}
			else if (factionName.equals("Rebel"))
			{
				sendSystemMessageTestingOnly(player, "You are already a Rebel!");
			}
			break;
			case 2:
			if (space_flags.isRebelPilot(player))
			{
				sendSystemMessageTestingOnly(player, "You are a Rebel Pilot! You must surrender your current space faction before you become an Imperial!");
				return SCRIPT_OVERRIDE;
			}
			if (factionName == null)
			{
				pvpSetAlignedFaction(player, (-615855020));
				pvpMakeCovert(player);
				sendSystemMessageTestingOnly(player, "Faction Set. You are now a Covert Imperial!");
			}
			else if (factionName.equals("Rebel"))
			{
				sendSystemMessageTestingOnly(player, "You are a Rebel! You must surrender your current faction before you become an Imperial!");
			}
			else if (factionName.equals("Imperial"))
			{
				sendSystemMessageTestingOnly(player, "You are already an Imperial!");
			}
			break;
			
			case 3:
			if (factionName == null)
			{
				sendSystemMessageTestingOnly(player, "You must declare a Faction before receiving Rank");
				return SCRIPT_OVERRIDE;
			}
			else
			{
				factions.setRank(player, current_rank + 1);
			}
			break;
			
			case 4:
			if (factionName == null)
			{
				sendSystemMessageTestingOnly(player, "You must declare a Faction before losing Rank");
				return SCRIPT_OVERRIDE;
			}
			else
			{
				factions.setRank(player, current_rank - 1);
			}
			break;
			case 5:
			if (hasObjVar(player, "intChangingFactionStatus"))
			{
				removeObjVar(player, "intChangingFactionStatus");
			}
			pvpMakeNeutral(player);
			pvpSetAlignedFaction(player, 0);
			factions.setFactionStanding(player, factionName, 0);
			factions.unequipFactionEquipment(player, false);
			factions.releaseFactionHirelings(player);
			sendSystemMessageTestingOnly(player, "You are now Neutral.");
			break;
			default:
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
			
		}
		refreshMenu ( player, "Select the desired faction option", "Character Builder Terminal", FACTION_OPTIONS, "handleFactionOptions", false);
		return SCRIPT_CONTINUE;
	}
	
	
	public void handleRoadmapSkills(obj_id player) throws InterruptedException
	{
		refreshMenu ( player, "Select the desired Roadmap option", "Character Builder Terminal", ROADMAP_SKILL_OPTIONS, "handleRoadmapSelect", false);
	}
	
	
	public int handleRoadmapSelect(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		if (btn == sui.BP_REVERT)
		{
			startCharacterBuilder(player);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		if (idx == -1 || idx > ROADMAP_SKILL_OPTIONS.length)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(player))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		String prompt = "Select the desired roadmap skill option";
		String title = "Character Builder Terminal";
		int pid = 0;
		
		switch(idx)
		{
			case 0:
			if (isGod(player))
			{
				handleRoadmapChoice(player);
			}
			else
			{
				sendSystemMessageTestingOnly(player, "You cannot access that option.");
			}
			break;
			case 1:
			String skillName = getWorkingSkill(player);
			
			dictionary xpReqs = getSkillPrerequisiteExperience(skillName);
			if (xpReqs == null || xpReqs.isEmpty())
			{
				sendSystemMessageTestingOnly(player, "Current working skill is invalid.");
				return SCRIPT_CONTINUE;
			}
			
			java.util.Enumeration e = xpReqs.keys();
			String xpType = (String)(e.nextElement());
			int xpCost = xpReqs.getInt(xpType);
			int curXP = getExperiencePoints(player, xpType);
			if (curXP < xpCost)
			{
				grantExperiencePoints(player, xpType, xpCost - curXP);
			}
			skill_template.earnWorkingSkill(player);
			handleRoadmapSkills(player);
			break;
			case 2:
			String template = getSkillTemplate(player);
			sui.inputbox(self, player, "Enter your desired level.", "handleAutoLevelSelect");
			break;
			case 3:
			respec.revokeAllSkillsAndExperience(player);
			
			int currentCombatXp = getExperiencePoints(player, "combat_general");
			grantExperiencePoints(player, "combat_general", -currentCombatXp);
			skill.recalcPlayerPools(player, true);
			
			respec.autoLevelPlayer(player, 90, false);
			
			utils.fullExpertiseReset(player, true);
			
			skill.setPlayerStatsForLevel(player, 90);
			
			removeObjVar(player, "expertise_reset");
			removeObjVar(player, "respecsBought");
			sendSystemMessageTestingOnly(player, "Respecced to level 90 and respecs cleared.");
			cleanScriptVars(player);
			break;
			default:
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleAutoLevelSelect(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id player = sui.getPlayerId(params);
		String text = sui.getInputBoxText(params);
		
		int level = utils.stringToInt(text);
		
		if (level < 1 || level > 90)
		{
			sendSystemMessageTestingOnly(player, "Invalid level entered!");
		}
		else
		{
			respec.autoLevelPlayer(player, level, false);
		}
		
		utils.fullExpertiseReset(player, true);
		
		expertise.autoAllocateExpertiseByLevel(player, false);
		handleRoadmapSkills(player);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void handleRoadmapChoice(obj_id player) throws InterruptedException
	{
		obj_id self = getSelf();
		String[] roadmapList = getRoadmapList();
		
		if (roadmapList == null || roadmapList.length == 0)
		{
			sendSystemMessage(player, SID_TERMINAL_DENIED);
			cleanScriptVars(player);
			return;
		}
		else
		{
			closeOldWindow(player);
			utils.setBatchScriptVar(player, "character_builder.roadmap_list", roadmapList);
		}
		refreshMenu ( player, "Select a skill roadmap.", "Character Builder Terminal", convertRoadmapNames(roadmapList), "handleRoadmapChoiceSelection", false);
	}
	
	
	public int handleRoadmapChoiceSelection(obj_id self, dictionary params) throws InterruptedException
	{
		int idx = sui.getListboxSelectedRow(params);
		obj_id player = sui.getPlayerId(params);
		int btn = sui.getIntButtonPressed(params);
		
		if (btn == sui.BP_REVERT)
		{
			refreshMenu (player, "Select the desired character option", "Character Builder Terminal", CHARACTER_BUILDER_OPTIONS, "handleOptionSelect", true);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		if (idx == -1)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		boolean levelNinety = false;
		
		if (idx % 2 == 1)
		{
			levelNinety = true;
		}
		
		idx = idx / 2;
		
		String[] roadmapList = utils.getStringBatchScriptVar(player, "character_builder.roadmap_list");
		
		if (exists(player))
		{
			if (!levelNinety)
			{
				handleRoadmapSkillProgression(player, roadmapList[idx]);
			}
			else
			{
				handleProfessionLevelToNinety(player, roadmapList[idx]);
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public String[] getRoadmapList() throws InterruptedException
	{
		
		return gm.getRoadmapList();
	}
	
	
	public String[] convertRoadmapNames(String[] list) throws InterruptedException
	{
		String[] newList = new String[list.length * 2];
		
		for (int i = 0; i < newList.length; i += 2)
		{
			testAbortScript();
			char branch = list[i / 2].charAt(list[i / 2].length() - 1);
			branch -= 49;
			
			String roadmapName = "@ui_roadmap:title_"+ list[i / 2].substring(0, list[i / 2].lastIndexOf('_'));
			String branchName = "@ui_roadmap:track_title_"+ list[i / 2].substring(0, list[i / 2].lastIndexOf('_')) + "_"+ branch;
			
			newList[i] = roadmapName + " - " + branchName;
			
			newList[i + 1] = roadmapName + " - Level 90";
		}
		
		return newList;
	}
	
	
	public void handleProfessionLevelToNinety(obj_id player, String roadmap) throws InterruptedException
	{
		revokeAllSkills(player);
		
		int currentCombatXp = getExperiencePoints(player, "combat_general");
		grantExperiencePoints(player, "combat_general", -currentCombatXp);
		skill.recalcPlayerPools(player, true);
		
		setSkillTemplate(player, roadmap);
		
		respec.autoLevelPlayer(player, 90, false);
		
		utils.fullExpertiseReset(player, true);
		
		skill.setPlayerStatsForLevel(player, 90);
		
		expertise.autoAllocateExpertiseByLevel(player, false);
		handleRoadmapSkills(player);
	}
	
	
	public void handleRoadmapSkillProgression(obj_id player, String roadmap) throws InterruptedException
	{
		obj_id self = getSelf();
		
		String templateSkills = dataTableGetString(skill_template.TEMPLATE_TABLE, roadmap, "template");
		String[] skillList = split(templateSkills, ',');
		
		if (skillList == null || skillList.length == 0)
		{
			sendSystemMessage(player, SID_TERMINAL_DENIED);
			cleanScriptVars(player);
			return;
		}
		else
		{
			
			closeOldWindow(player);
			utils.setScriptVar(player, "character_builder.skill_template", roadmap);
			utils.setBatchScriptVar(player, "character_builder.roadmap_skills", skillList);
		}
		
		refreshMenu ( player, "Select a the working skill in the roadmap.", "Character Builder Terminal", convertSkillListNames(skillList), "handleRoadmapSkillSelection", false);
	}
	
	
	public int handleRoadmapSkillSelection(obj_id self, dictionary params) throws InterruptedException
	{
		int idx = sui.getListboxSelectedRow(params);
		obj_id player = sui.getPlayerId(params);
		int btn = sui.getIntButtonPressed(params);
		
		if (btn == sui.BP_REVERT)
		{
			refreshMenu (player, "Select the desired character option", "Character Builder Terminal", CHARACTER_BUILDER_OPTIONS, "handleOptionSelect", true);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		if (idx == -1)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		if (exists(player) && !outOfRange(self , player, false))
		{
			sendSystemMessageTestingOnly(player, "Revoking all old skills.");
			revokeAllSkills(player);
			
			int currentCombatXp = getExperiencePoints(player, "combat_general");
			grantExperiencePoints(player, "combat_general", -currentCombatXp);
			skill.recalcPlayerPools(player, true);
			
			String skillTemplate = utils.getStringScriptVar(player, "character_builder.skill_template");
			setSkillTemplate(player, skillTemplate);
			
			String[] roadmapSkills = utils.getStringBatchScriptVar(player, "character_builder.roadmap_skills");
			
			for (int i = 0; i < idx; i++)
			{
				testAbortScript();
				skill.grantSkillToPlayer(player, roadmapSkills[i]);
			}
			
			setWorkingSkill(player, roadmapSkills[idx]);
			
			utils.fullExpertiseReset(player, true);
			
			expertise.autoAllocateExpertiseByLevel(player, false);
			skill.recalcPlayerPools(player, true);
		}
		
		refreshMenu (player, "Select the desired character option", "Character Builder Terminal", CHARACTER_BUILDER_OPTIONS, "handleOptionSelect", true);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void revokeAllSkills(obj_id player) throws InterruptedException
	{
		String[] skillList = getSkillListingForPlayer(player);
		int attempts = skillList.length;
		
		if ((skillList != null) && (skillList.length != 0))
		{
			while (skillList.length > 0 && attempts > 0)
			{
				testAbortScript();
				for (int i = 0; i < skillList.length; i++)
				{
					testAbortScript();
					String skillName = skillList[i];
					if ((!skillName.startsWith ("species_")&& !skillName.startsWith("social_language_")&& !skillName.startsWith("utility_")&& !skillName.startsWith("common_")&& !skillName.startsWith("demo_")&& !skillName.startsWith("force_title_")&& !skillName.startsWith("force_sensitive_")&& !skillName.startsWith("combat_melee_basic")&& !skillName.startsWith("pilot_")&& !skillName.startsWith("internal_expertise_") && !skillName.startsWith("class_chronicles_") && !skillName.startsWith("combat_ranged_weapon_basic")))
					{
						skill.revokeSkillSilent(player, skillName);
					}
				}
				
				skillList = getSkillListingForPlayer(player);
				
				--attempts;
			}
		}
		
		utils.fullExpertiseReset(player, false);
		
		skill.recalcPlayerPools(player, true);
	}
	
	
	public void handlePetAbilityOption(obj_id player) throws InterruptedException
	{
		int[] abilityList = dataTableGetIntColumn(pet_lib.PET_ABILITY_TABLE, "abilityCrc");
		String[] abilityNames = dataTableGetStringColumn(pet_lib.PET_ABILITY_TABLE, "abilityName");
		
		for (int i = 0; i < abilityNames.length; i++)
		{
			testAbortScript();
			abilityNames[i] = utils.packStringId(new string_id("pet/pet_ability", abilityNames[i]));
		}
		
		refreshMenu ( player, "Select the desired Pet Ability", "Character Builder Terminal", abilityNames, "handlePetAbilitySelection", false);
		utils.setScriptVar(player, "character_builder.petAbilityList", abilityList);
		utils.setScriptVar(player, "character_builder.petAbilityNames", abilityNames);
	}
	
	
	public int handlePetAbilitySelection(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		if (btn == sui.BP_REVERT)
		{
			startCharacterBuilder(player);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		if (idx == -1)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		int[] petAbilityList = utils.getIntArrayScriptVar(player, "character_builder.petAbilityList");
		String[] petAbilityNames = utils.getStringArrayScriptVar(player, "character_builder.petAbilityNames");
		
		int[] chAbilityList = getIntArrayObjVar(player, "ch.petAbility.abilityList");
		int[] newAbilityList = null;
		
		if (chAbilityList != null && chAbilityList.length > 0)
		{
			if (utils.getElementPositionInArray(chAbilityList, petAbilityList[idx]) > -1)
			{
				refreshMenu ( player, "Select the desired Pet Ability", "Character Builder Terminal", petAbilityNames, "handlePetAbilitySelection", false);
				return SCRIPT_CONTINUE;
			}
			
			newAbilityList = new int[chAbilityList.length + 1];
			for (int i = 0; i < chAbilityList.length; i++)
			{
				testAbortScript();
				newAbilityList[i] = chAbilityList[i];
			}
			
			newAbilityList[newAbilityList.length - 1] = petAbilityList[idx];
		}
		else
		{
			newAbilityList = new int[1];
			newAbilityList[0] = petAbilityList[idx];
		}
		setObjVar(player, "ch.petAbility.abilityList", newAbilityList);
		
		refreshMenu ( player, "Select the desired Pet Ability", "Character Builder Terminal", petAbilityNames, "handlePetAbilitySelection", false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void handleJediOption(obj_id player) throws InterruptedException
	{
		refreshMenu ( player, "Select the desired Jedi option", "Character Builder Terminal", JEDI_OPTIONS, "handleJediSelect", false);
	}
	
	
	public int handleJediSelect(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		obj_id pInv = utils.getInventoryContainer(player);
		
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		if (btn == sui.BP_REVERT)
		{
			startCharacterBuilder(player);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		if (idx == -1 || idx > JEDI_OPTIONS.length)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(player))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		String prompt = "Select the desired armor option";
		String title = "Character Builder Terminal";
		int pid = 0;
		
		switch(idx)
		{
			case 0:
			refreshMenu( player, prompt, title, CRYSTAL_OPTIONS, "handleCrystalSelect", false);
			break;
			case 1:
			refreshMenu( player, prompt, title, SABER_OPTIONS, "handleSaberSelect", false);
			break;
			case 2:
			refreshMenu( player, prompt, title, ROBE_OPTIONS, "handleRobeSelect", false);
			break;
			
			default:
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		return SCRIPT_CONTINUE;
	}
	
	public int handleCrystalSelect(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		obj_id pInv = utils.getInventoryContainer(player);
		
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		if (btn == sui.BP_REVERT)
		{
			handleJediOption(player);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		if (idx == -1 || idx > CRYSTAL_OPTIONS.length)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(player))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (!isIdValid(pInv))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (getVolumeFree(pInv) <= 4)
		{
			sendSystemMessageTestingOnly(player, "Your Inventory is Full! Please free up at least 4 inventory slots and try again.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		switch(idx)
		{
			
			case 0:
			for (int i = 0; i < 4; i++)
			{
				testAbortScript();
				jedi.createColorCrystal(pInv, rand(0, 11));
			}
			sendSystemMessageTestingOnly(player, "Color Crystals Issued!");
			break;
			case 1:
			if (isGod(player))
			{
				static_item.createNewItemFunction("item_tow_lava_crystal_06_01",pInv);
				sendSystemMessageTestingOnly(player, "Lava Crystal Issued!");
			}
			else
			{
				sendSystemMessageTestingOnly(player, "You Must be in God Mode to enjoy the Lava Crystal!");
			}
			break;
			case 2:
			for (int i = 0; i < 4; i++)
			{
				testAbortScript();
				static_item.createNewItemFunction("item_power_crystal_04_15", pInv);
			}
			sendSystemMessageTestingOnly(player, "Power Crystals Issued!");
			break;
			case 3:
			if (isGod(player))
			{
				for (int i = 0; i < 4; i++)
				{
					testAbortScript();
					static_item.createNewItemFunction("item_krayt_pearl_04_20", pInv);
					
				}
				sendSystemMessageTestingOnly(player, "Ancient Krayt Pearls Issued!");
			}
			else
			{
				sendSystemMessageTestingOnly(player, "You Must be in God Mode to enjoy the Ancient Krayt Pearls Issued!");
			}
			break;
			default:
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
			
		}
		
		refreshMenu( player, "Select the desired armor option", "Character Builder Terminal", CRYSTAL_OPTIONS, "handleCrystalSelect", false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleSaberSelect(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		obj_id pInv = utils.getInventoryContainer(player);
		
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		if (btn == sui.BP_REVERT)
		{
			handleJediOption(player);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		if (idx == -1 || idx > SABER_OPTIONS.length)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(player))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (!isIdValid(pInv))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (getVolumeFree(pInv) <= 4)
		{
			sendSystemMessageTestingOnly(player, "Your Inventory is Full! Please free up at least 4 inventory slots and try again.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		switch(idx)
		{
			
			case 0:
			{
				float[] weaponMinDamage =
				{
					145f
				};
				float[] weaponMaxDamage =
				{
					300f
				};
				float[] weaponAttackSpeed =
				{
					1f
				};
				float weaponWoundChance = 1.0f;
				float weaponForceCost = 0.0f;
				float weaponAttackCost = 100f;
				generateGenerationSabers(0, player, pInv, weaponMinDamage, weaponMaxDamage, weaponAttackSpeed, weaponWoundChance,weaponForceCost,weaponAttackCost);
				jedi.createColorCrystal(pInv, rand(0, 11));
				sendSystemMessageTestingOnly(player, "Training Saber Issued!");
			}
			break;
			case 1:
			{
				float[] weaponMinDamage =
				{
					300f, 300f, 300f
				};
				float[] weaponMaxDamage =
				{
					611f, 611f, 611f
				};
				float[] weaponAttackSpeed =
				{
					1f, 1f, 1f
				};
				float weaponWoundChance = 1.0f;
				float weaponForceCost = 0.0f;
				float weaponAttackCost = 1f;
				generateGenerationSabers(1, player, pInv, weaponMinDamage, weaponMaxDamage, weaponAttackSpeed, weaponWoundChance,weaponForceCost,weaponAttackCost);
				jedi.createColorCrystal(pInv, rand(0, 11));
				sendSystemMessageTestingOnly(player, "Generation One Sabers Issued!");
			}
			break;
			case 2:
			{
				float[] weaponMinDamage =
				{
					360f, 360f, 360f
				};
				float[] weaponMaxDamage =
				{
					740f, 740f, 740f
				};
				float[] weaponAttackSpeed =
				{
					1f, 1f, 1f
				};
				float weaponWoundChance = 1.0f;
				float weaponForceCost = 0.0f;
				float weaponAttackCost = 1f;
				generateGenerationSabers(2, player, pInv, weaponMinDamage, weaponMaxDamage, weaponAttackSpeed, weaponWoundChance,weaponForceCost,weaponAttackCost);
				jedi.createColorCrystal(pInv, rand(0, 11));
				sendSystemMessageTestingOnly(player, "Generation Two Sabers Issued!");
				
			}
			break;
			case 3:
			{
				float[] weaponMinDamage =
				{
					500f, 500f, 500f
				};
				float[] weaponMaxDamage =
				{
					1000f, 1000f, 1000f
				};
				float[] weaponAttackSpeed =
				{
					1f, 1f, 1f
				};
				float weaponWoundChance = 1.0f;
				float weaponForceCost = 0.0f;
				float weaponAttackCost = 1f;
				generateGenerationSabers(3, player, pInv, weaponMinDamage, weaponMaxDamage, weaponAttackSpeed, weaponWoundChance,weaponForceCost,weaponAttackCost);
				jedi.createColorCrystal(pInv, rand(0, 11));
				sendSystemMessageTestingOnly(player, "Generation Three Sabers Issued!");
			}
			break;
			case 4:
			{
				float[] weaponMinDamage =
				{
					597f, 597f, 597f
				};
				float[] weaponMaxDamage =
				{
					1193f, 1193f, 1193f
				};
				float[] weaponAttackSpeed =
				{
					1f, 1f, 1f
				};
				float weaponWoundChance = 1.0f;
				float weaponForceCost = 0.0f;
				float weaponAttackCost = 1f;
				generateGenerationSabers(4, player, pInv, weaponMinDamage, weaponMaxDamage, weaponAttackSpeed, weaponWoundChance,weaponForceCost,weaponAttackCost);
				jedi.createColorCrystal(pInv, rand(0, 11));
				sendSystemMessageTestingOnly(player, "Generation Four Sabers Issued!");
			}
			break;
			case 5:
			{
				float[] weaponMinDamage =
				{
					660f, 660f, 660f
				};
				float[] weaponMaxDamage =
				{
					1350f, 1350f, 1350f
				};
				float[] weaponAttackSpeed =
				{
					1f, 1f, 1f
				};
				float weaponWoundChance = 1.0f;
				float weaponForceCost = 0.0f;
				float weaponAttackCost = 1f;
				generateGenerationSabers(5, player, pInv, weaponMinDamage, weaponMaxDamage, weaponAttackSpeed, weaponWoundChance,weaponForceCost,weaponAttackCost);
				jedi.createColorCrystal(pInv, rand(0, 11));
				sendSystemMessageTestingOnly(player, "Generation Five Sabers Issued!");
			}
			break;
			default:
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
			
		}
		
		refreshMenu( player, "Select the desired armor option", "Character Builder Terminal", SABER_OPTIONS, "handleSaberSelect", false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleRobeSelect(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		obj_id pInv = utils.getInventoryContainer(player);
		
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		if (btn == sui.BP_REVERT)
		{
			handleJediOption(player);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		if (idx == -1 || idx > ROBE_OPTIONS.length)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(player))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (!isIdValid(pInv))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (getVolumeFree(pInv) <= 2)
		{
			sendSystemMessageTestingOnly(player, "Your Inventory is Full! Please free up at least 2 inventory slots and try again.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		switch(idx)
		{
			
			case 0:
			{
				static_item.createNewItemFunction("item_jedi_robe_padawan_04_01", pInv);
				sendSystemMessageTestingOnly(player, "Padawan Robe Issued!");
			}
			break;
			case 1:
			{
				static_item.createNewItemFunction("item_jedi_robe_light_03_01", pInv);
				sendSystemMessageTestingOnly(player, "Light Acolyte Robe Issued!");
			}
			break;
			case 2:
			{
				static_item.createNewItemFunction("item_jedi_robe_dark_03_01", pInv);
				sendSystemMessageTestingOnly(player, "Dark Acolyte Robe Issued!");
			}
			break;
			case 3:
			{
				static_item.createNewItemFunction("item_jedi_robe_light_03_02", pInv);
				sendSystemMessageTestingOnly(player, "Light Apprentice Robe Issued!");
			}
			break;
			case 4:
			{
				static_item.createNewItemFunction("item_jedi_robe_dark_03_02", pInv);
				sendSystemMessageTestingOnly(player, "Dark Apprentice Robe Issued!");
			}
			break;
			case 5:
			{
				static_item.createNewItemFunction("item_jedi_robe_light_03_03", pInv);
				sendSystemMessageTestingOnly(player, "Light Jedi Knight Robe Issued!");
			}
			break;
			case 6:
			{
				static_item.createNewItemFunction("item_jedi_robe_dark_03_03", pInv);
				sendSystemMessageTestingOnly(player, "Dark Jedi Knight Robe Issued!");
			}
			break;
			case 7:
			{
				static_item.createNewItemFunction("item_jedi_robe_06_01", pInv);
				sendSystemMessageTestingOnly(player, "Jedi Master Cloak Issued!");
			}
			break;
			case 8:
			{
				static_item.createNewItemFunction("item_jedi_robe_06_02", pInv);
				sendSystemMessageTestingOnly(player, "Jedi Master Cloak Issued!");
			}
			break;
			case 9:
			{
				if (hasCompletedCollectionSlot(player, "jedi_robe_01_07"))
				{
					modifyCollectionSlotValue(player, "jedi_robe_01_07", -1);
				}
				if (hasCompletedCollectionSlot(player, "jedi_robe_01_08"))
				{
					modifyCollectionSlotValue(player, "jedi_robe_01_08", -1);
				}
				sendSystemMessageTestingOnly(player, "Statue slots in Master Jedi Cloak collection reset!");
			}
			default:
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
			
		}
		
		refreshMenu( player, "Select the desired armor option", "Character Builder Terminal", ROBE_OPTIONS, "handleRobeSelect", false);
		return SCRIPT_CONTINUE;
	}
	
	
	public void generateGenerationSabers(int generation, obj_id player, obj_id pInv, float[] weaponMinDamage, float[] weaponMaxDamage, float[] weaponAttackSpeed, float weaponWoundChance, float weaponForceCost, float weaponAttackCost) throws InterruptedException
	{
		String[] saber;
		if (generation < 1 || generation > 5)
		{
			saber = new String[1];
			saber[0] = "object/weapon/melee/sword/crafted_saber/sword_lightsaber_training.iff";
		}
		else
		{
			saber = new String[3];
			saber[0] = "object/weapon/melee/sword/crafted_saber/sword_lightsaber_one_handed_gen"+ generation +".iff";
			saber[1] = "object/weapon/melee/2h_sword/crafted_saber/sword_lightsaber_two_handed_gen"+ generation +".iff";
			saber[2] = "object/weapon/melee/polearm/crafted_saber/sword_lightsaber_polearm_gen"+ generation +".iff";
		}
		if (saber.length != weaponMinDamage.length || saber.length != weaponMaxDamage.length || saber.length != weaponAttackSpeed.length)
		{
			return;
		}
		
		for (int i = 0; i < saber.length; i++)
		{
			testAbortScript();
			obj_id saberObj = createObject(saber[i], pInv, "");
			if (!isIdValid (saberObj))
			{
				continue;
			}
			
			setWeaponMinDamage(saberObj, (int)(weaponMinDamage[i]));
			setWeaponMaxDamage(saberObj, (int)(weaponMaxDamage[i]));
			setWeaponAttackSpeed(saberObj, weaponAttackSpeed[i]);
			setWeaponAttackCost(saberObj, (int)weaponAttackCost);
			setWeaponWoundChance(saberObj, weaponWoundChance);
			
			setConversionId(saberObj,weapons.CONVERSION_VERSION);
			
			setObjVar(saberObj, jedi.VAR_SABER_DEFAULT_STATS + "."+ jedi.VAR_MIN_DMG, (int)weaponMinDamage[i]);
			setObjVar(saberObj, jedi.VAR_SABER_DEFAULT_STATS + "."+ jedi.VAR_MAX_DMG, (int)weaponMaxDamage[i]);
			setObjVar(saberObj, jedi.VAR_SABER_DEFAULT_STATS + "."+ jedi.VAR_SPEED, weaponAttackSpeed[i]);
			
			setObjVar(saberObj, jedi.VAR_SABER_DEFAULT_STATS + "."+ jedi.VAR_WOUND, weaponWoundChance);
			setObjVar(saberObj, jedi.VAR_SABER_DEFAULT_STATS + "."+ jedi.VAR_RADIUS, getWeaponDamageRadius (saberObj));
			setObjVar(saberObj, jedi.VAR_SABER_DEFAULT_STATS + "."+ jedi.VAR_ATTACK_COST, getWeaponAttackCost(saberObj));
			setObjVar(saberObj, jedi.VAR_SABER_DEFAULT_STATS + "."+ jedi.VAR_ACCURACY, getWeaponAccuracy (saberObj));
			
			setWeaponRangeInfo(saberObj, 0.0f, 5.0f);
			setWeaponDamageType (saberObj, DAMAGE_ENERGY);
			setCraftedId(saberObj, saberObj);
			setCrafter(saberObj, player);
			setSocketsUp(saberObj);
			weapons.setWeaponData(saberObj);
		}
	}
	
	
	public void launch(obj_id player, obj_id ship, obj_id[] membersApprovedByShipOwner, location warpLocation, location groundLoc) throws InterruptedException
	{
		
		space_transition.clearOvertStatus(ship);
		Vector groupMembersToWarp = utils.addElement(null, player);
		Vector groupMemberStartIndex = utils.addElement(null, 0);
		utils.setScriptVar(player, "strLaunchPointName", "launching");
		Vector shipStartLocations = space_transition.getShipStartLocations(ship);
		if (shipStartLocations != null && shipStartLocations.size() > 0)
		{
			int startIndex = 0;
			location playerLoc = getLocation(player);
			if (isIdValid(playerLoc.cell))
			{
				obj_id group = getGroupObject(player);
				if (isIdValid(group))
				{
					obj_id[] groupMembers = getGroupMemberIds(group);
					for (int i = 0; i < groupMembers.length; ++i)
					{
						testAbortScript();
						if ((groupMembers[i] != player && exists(groupMembers[i]) && getLocation(groupMembers[i]).cell == playerLoc.cell && groupMemberApproved(membersApprovedByShipOwner, groupMembers[i])))
						{
							
							startIndex = getNextStartIndex(shipStartLocations, startIndex);
							if (startIndex <= shipStartLocations.size())
							{
								groupMembersToWarp = utils.addElement(groupMembersToWarp, groupMembers[i]);
								groupMemberStartIndex = utils.addElement(groupMemberStartIndex, startIndex);
							}
							
							else
							{
								string_id strSpam = new string_id("space/space_interaction", "no_space_expansion");
								sendSystemMessage(groupMembers[i], strSpam);
							}
						}
					}
				}
			}
		}
		
		for (int i = 0; i < groupMembersToWarp.size(); ++i)
		{
			testAbortScript();
			space_transition.setLaunchInfo(((obj_id)(groupMembersToWarp.get(i))), ship, ((Integer)(groupMemberStartIndex.get(i))).intValue(), groundLoc);
			warpPlayer(
			((obj_id)(groupMembersToWarp.get(i))),
			warpLocation.area,
			warpLocation.x, warpLocation.y, warpLocation.z,
			null,
			warpLocation.x, warpLocation.y, warpLocation.z);
		}
		
	}
	
	
	public int getNextStartIndex(Vector shipStartLocations, int lastStartIndex) throws InterruptedException
	{
		int startIndex = lastStartIndex+1;
		
		if (startIndex > shipStartLocations.size())
		{
			for (startIndex = 1; startIndex <= shipStartLocations.size(); ++startIndex)
			{
				testAbortScript();
				if (((location)(shipStartLocations.get(startIndex-1))).cell != null)
				{
					break;
				}
			}
		}
		
		return startIndex;
	}
	
	
	public boolean groupMemberApproved(obj_id[] membersApprovedByShipOwner, obj_id memberToTest) throws InterruptedException
	{
		for (int i = 0; i < membersApprovedByShipOwner.length; ++i)
		{
			testAbortScript();
			if (membersApprovedByShipOwner[i] == memberToTest)
			{
				return true;
			}
		}
		return false;
	}
	
	
	public void handleCyberneticsOption(obj_id player) throws InterruptedException
	{
		obj_id inv = utils.getInventoryContainer(player);
		if (!isIdValid( inv ))
		{
			return;
		}
		
		for (int i = 0; i < CYBERNETIC_ITEMS.length; ++i)
		{
			testAbortScript();
			createObject(CYBERNETIC_ITEMS[i], inv, "");
		}
		sendSystemMessageTestingOnly( player, "Cybernetics issued. Pay a cybernetic Engineer to install the items");
		sendSystemMessageTestingOnly( player, "Locate the cybernetic engineer on the 2nd floor of a medical center");
		location warpLocation = getLocation( player );
		warpLocation.area = "tatooine";
		warpLocation.x = 1305f;
		warpLocation.y = 7f;
		warpLocation.z = 3261f;
		warpPlayer( player, warpLocation.area, warpLocation.x, warpLocation.y, warpLocation.z, null, 0.0f, 0.0f, 0.0f);
	}
	
	
	public void refreshMenu(obj_id player, String prompt, String title, String[] options, String myHandler, boolean draw) throws InterruptedException
	{
		obj_id self = getSelf();
		closeOldWindow(player);
		
		if (outOfRange(self , player, true))
		{
			cleanScriptVars(player);
			return;
		}
		
		if (draw == false)
		{
			int pid = sui.listbox(self, player, prompt, sui.OK_CANCEL_REFRESH, title, options, myHandler, false, false);
			sui.listboxUseOtherButton(pid, "Back");
			sui.showSUIPage(pid);
			setWindowPid(player, pid);
			
		}
		else
		{
			int pid = sui.listbox(self, player, prompt, sui.OK_CANCEL, title, options, myHandler, true, false);
			sui.showSUIPage(pid);
			setWindowPid(player, pid);
		}
	}
	
	
	public boolean outOfRange(obj_id self, obj_id player, boolean message) throws InterruptedException
	{
		
		if (isGod(player))
		{
			return false;
		}
		
		location a = getLocation(self);
		location b = getLocation(player);
		if (a.cell == b.cell && a.distance(b) < 8.0f)
		{
			return false;
		}
		
		if (message)
		{
			sendSystemMessageTestingOnly(player, "Out of Range of Character Builder Terminal.");
		}
		
		return true;
	}
	
	
	public void handlePublishOption(obj_id player) throws InterruptedException
	{
		refreshMenu ( player, "Select the desired Publish 27 option", "Character Builder Terminal", PUBLISH_OPTIONS, "handlePublishOptions", false);
	}
	
	
	public int handlePublishOptions(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		obj_id pInv = utils.getInventoryContainer(player);
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		if (btn == sui.BP_REVERT)
		{
			refreshMenu (player, "Select the desired character option", "Character Builder Terminal", CHARACTER_BUILDER_OPTIONS, "handleOptionSelect", true);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		if (idx == -1 || idx > PUBLISH_OPTIONS.length)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(player))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (!isIdValid(pInv))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		if (getVolumeFree(pInv) <= 12)
		{
			sendSystemMessageTestingOnly(player, "Your Inventory has less than 12 slots, please make room and try again.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		switch(idx)
		{
			
			case 0:
			{
				for (int i = 0; i < PUB27_HEAVYPACK.length; i++)
				{
					testAbortScript();
					if (PUB27_HEAVYPACK[i].startsWith("object"))
					{
						weapons.createWeapon(PUB27_HEAVYPACK[i], pInv, weapons.VIA_TEMPLATE, WEAPON_SPEED, WEAPON_DAMAGE, WEAPON_EFFECIENCY, WEAPON_ELEMENTAL);
					}
					else
					{
						static_item.createNewItemFunction(PUB27_HEAVYPACK[i], pInv);
					}
				}
				
				String template = getSkillTemplate(player);
				if (!template.startsWith("trader") && !template.startsWith("entertainer"))
				{
					if (!isGod(player))
					{
						respec.autoLevelPlayer(player, 88, false);
					}
				}
			}
			
			sendSystemMessageTestingOnly(player, "Heavy Weapons Pack Issued!");
			break;
			case 1:
			{
				float[] weaponMinDamage =
				{
					239f, 239f, 239f
				};
				float[] weaponMaxDamage =
				{
					477f, 477f, 477f
				};
				float[] weaponAttackSpeed =
				{
					.5f, .5f, .5f
				};
				float weaponWoundChance = 1.0f;
				float weaponForceCost = 0.0f;
				float weaponAttackCost = 1f;
				generateGenerationSabers(4, player, pInv, weaponMinDamage, weaponMaxDamage, weaponAttackSpeed, weaponWoundChance,weaponForceCost,weaponAttackCost);
				jedi.createColorCrystal(pInv, rand(0, 11));
				
				for (int i = 0; i < 4; i++)
				{
					testAbortScript();
					static_item.createNewItemFunction("item_krayt_pearl_04_16", pInv);
				}
				
				static_item.createNewItemFunction("item_jedi_robe_dark_03_03", pInv);
				static_item.createNewItemFunction("item_jedi_robe_light_03_03", pInv);
				
				revokeAllSkills(player);
				int currentCombatXp = getExperiencePoints(player, "combat_general");
				grantExperiencePoints(player, "combat_general", -currentCombatXp);
				setSkillTemplate(player, "force_sensitive_1a");
				String templateSkills = dataTableGetString(skill_template.TEMPLATE_TABLE, "force_sensitive_1a", "template");
				String[] skillList = split(templateSkills, ',');
				setWorkingSkill(player, skillList[0]);
				
				respec.autoLevelPlayer(player, 88, false);
				skill.recalcPlayerPools(player, true);
				sendSystemMessageTestingOnly(player, "Level 78 Gear Issued!");
			}
			break;
			case 2:
			{
				for (int i = 0; i < PUB27_TRAPS.length; i++)
				{
					testAbortScript();
					stealth.createRangerLoot(100, PUB27_TRAPS[i], pInv, 100);
				}
				
				sendSystemMessageTestingOnly(player, "Traps Issued!");
				
			}
			break;
			case 3:
			{
				setObjVar(player, "mand.acknowledge", true);
				sendSystemMessageTestingOnly(player, "Death Watch Bunker Access Granted!");
			}
			break;
			case 4:
			for (int i = 0; i < PUB27_CAMOSTUFF.length; i++)
			{
				testAbortScript();
				static_item.createNewItemFunction(PUB27_CAMOSTUFF[i],pInv);
			}
			sendSystemMessageTestingOnly(player, "Spy Gear Issued!");
			break;
			default:
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
			
		}
		
		refreshMenu (player, "Select the desired option", "Character Builder Terminal", PUBLISH_OPTIONS, "handlePublishOptions", false);
		return SCRIPT_CONTINUE;
	}
	
	
	public void flagAllHeroicInstances(obj_id player) throws InterruptedException
	{
		String[] instanceFlags = dataTableGetStringColumn(instance.INSTANCE_DATATABLE, "key_required");
		if (instanceFlags != null && instanceFlags.length > 0)
		{
			for (int i = 0; i < instanceFlags.length; i++)
			{
				testAbortScript();
				String flag = instanceFlags[i];
				if (flag != null && flag.length() > 0)
				{
					instance.flagPlayerForInstance(player, flag);
				}
			}
			
			sendSystemMessageTestingOnly(player, "All Heroic Instances Flagged");
		}
		else
		{
			sendSystemMessageTestingOnly(player, "No Instance Flags Found.");
		}
	}
	
	
	public obj_id tweakSpaceShipComponent(obj_id objComponent) throws InterruptedException
	{
		String strComponentType = space_crafting.getShipComponentStringType(objComponent);
		
		if (strComponentType == null)
		{
			LOG("tweakSpaceShipComponent", "MAJOR MESSUP! "+ objComponent + " is 9 kinds of messed up");
			setName(objComponent, "BAD COMPONENT: "+ getTemplateName(objComponent) + " : PLEASE REPORT BUG");
			return null;
		}
		
		if (!strComponentType.equals(""))
		{
			
			dictionary dctParams = dataTableGetRow("datatables/ship/components/character_builder/frog_"+ strComponentType + ".iff", getTemplateName(objComponent));
			
			if (dctParams == null)
			{
				LOG("tweakSpaceShipComponent", "TEMPLATE OF TYPE "+ getTemplateName(objComponent) + " HAS BEEN PASSED TO SETUP SPACE COMPONENT. THIS DOES NOT EXIST IN THE DATATBLE of "+ "datatables/ship/components/"+ strComponentType + ".iff");
				return null;
			}
			
			if (strComponentType.equals("armor"))
			{
				float fltMaximumHitpoints = dctParams.getFloat("fltMaximumHitpoints");
				space_crafting.setComponentCurrentHitpoints(objComponent, fltMaximumHitpoints);
				space_crafting.setComponentMaximumHitpoints(objComponent, fltMaximumHitpoints);
				
				float fltMaximumArmorHitpoints = dctParams.getFloat("fltMaximumArmorHitpoints");
				space_crafting.setComponentCurrentArmorHitpoints(objComponent, fltMaximumHitpoints);
				space_crafting.setComponentMaximumArmorHitpoints(objComponent, fltMaximumHitpoints);
				
				float fltMass = dctParams.getFloat("fltMass");
				space_crafting.setComponentMass(objComponent, fltMass);
				
				setComponentObjVar(objComponent, "character.builder", 1);
				String newName = dctParams.getString("strName");
				setName(objComponent, newName);
			}
			else if (strComponentType.equals("booster"))
			{
				LOG("tweakSpaceShipComponent", "BOOSTER"+ getTemplateName(objComponent));
				
				float fltMaximumEnergy = dctParams.getFloat("fltMaximumEnergy");
				space_crafting.setBoosterMaximumEnergy(objComponent, fltMaximumEnergy);
				space_crafting.setBoosterCurrentEnergy(objComponent, fltMaximumEnergy);
				
				float fltRechargeRate = dctParams.getFloat("fltRechargeRate");
				space_crafting.setBoosterEnergyRechargeRate(objComponent, fltRechargeRate);
				
				float fltConsumptionRate = dctParams.getFloat("fltConsumptionRate");
				space_crafting.setBoosterEnergyConsumptionRate(objComponent, fltConsumptionRate);
				
				float fltAcceleration = dctParams.getFloat("fltAcceleration");
				space_crafting.setBoosterAcceleration(objComponent, fltAcceleration);
				
				float fltMaxSpeed = dctParams.getFloat("fltMaxSpeed");
				space_crafting.setBoosterMaximumSpeed(objComponent, fltMaxSpeed);
				
				float fltMaximumHitpoints = dctParams.getFloat("fltMaximumHitpoints");
				space_crafting.setComponentCurrentHitpoints(objComponent, fltMaximumHitpoints);
				space_crafting.setComponentMaximumHitpoints(objComponent, fltMaximumHitpoints);
				
				float fltMaximumArmorHitpoints = dctParams.getFloat("fltMaximumArmorHitpoints");
				space_crafting.setComponentCurrentArmorHitpoints(objComponent, fltMaximumHitpoints);
				space_crafting.setComponentMaximumArmorHitpoints(objComponent, fltMaximumHitpoints);
				
				float fltEnergyMaintenance = dctParams.getFloat("fltEnergyMaintenance");
				space_crafting.setComponentEnergyMaintenance(objComponent, fltEnergyMaintenance);
				
				float fltMass = dctParams.getFloat("fltMass");
				space_crafting.setComponentMass(objComponent, fltMass);
				
				setComponentObjVar(objComponent, "character.builder", 1);
				String newName = dctParams.getString("strName");
				setName(objComponent, newName);
			}
			else if (strComponentType.equals("cargo_hold"))
			{
				LOG("tweakSpaceShipComponent", "CARGO HOLD"+ getTemplateName(objComponent));
				
				int intCargoHoldCapacity = dctParams.getInt("intCargoHoldCapacity");
				space_crafting.setCargoHoldMaxCapacity(objComponent, intCargoHoldCapacity);
				
				setComponentObjVar(objComponent, "character.builder", 1);
				String newName = dctParams.getString("strName");
				setName(objComponent, newName);
			}
			else if (strComponentType.equals("droid_interface"))
			{
				LOG("tweakSpaceShipComponent", "DROID INTERFACE"+ getTemplateName(objComponent));
				
				float fltCommandSpeed = dctParams.getFloat("fltCommandSpeed");
				space_crafting.setDroidInterfaceCommandSpeed(objComponent, fltCommandSpeed);
				
				float fltMaximumHitpoints = dctParams.getFloat("fltMaximumHitpoints");
				space_crafting.setComponentCurrentHitpoints(objComponent, fltMaximumHitpoints);
				space_crafting.setComponentMaximumHitpoints(objComponent, fltMaximumHitpoints);
				
				float fltMaximumArmorHitpoints = dctParams.getFloat("fltMaximumArmorHitpoints");
				space_crafting.setComponentCurrentArmorHitpoints(objComponent, fltMaximumHitpoints);
				space_crafting.setComponentMaximumArmorHitpoints(objComponent, fltMaximumHitpoints);
				
				float fltEnergyMaintenance = dctParams.getFloat("fltEnergyMaintenance");
				space_crafting.setComponentEnergyMaintenance(objComponent, fltEnergyMaintenance);
				
				float fltMass = dctParams.getFloat("fltMass");
				space_crafting.setComponentMass(objComponent, fltMass);
				
				setComponentObjVar(objComponent, "character.builder", 1);
				String newName = dctParams.getString("strName");
				setName(objComponent, newName);
			}
			else if (strComponentType.equals("engine"))
			{
				LOG("tweakSpaceShipComponent", "ENGINE"+ getTemplateName(objComponent));
				
				float fltMaxSpeed = dctParams.getFloat("fltMaxSpeed");
				space_crafting.setEngineMaximumSpeed(objComponent, fltMaxSpeed);
				
				float fltMaxPitch = dctParams.getFloat("fltMaxPitch");
				space_crafting.setEngineMaximumPitch(objComponent, fltMaxPitch);
				
				float fltMaxRoll = dctParams.getFloat("fltMaxRoll");
				space_crafting.setEngineMaximumRoll(objComponent, fltMaxRoll);
				
				float fltMaxYaw = dctParams.getFloat("fltMaxYaw");
				space_crafting.setEngineMaximumYaw(objComponent, fltMaxYaw);
				
				float fltMaximumHitpoints = dctParams.getFloat("fltMaximumHitpoints");
				space_crafting.setComponentCurrentHitpoints(objComponent, fltMaximumHitpoints);
				space_crafting.setComponentMaximumHitpoints(objComponent, fltMaximumHitpoints);
				
				float fltMaximumArmorHitpoints = dctParams.getFloat("fltMaximumArmorHitpoints");
				space_crafting.setComponentCurrentArmorHitpoints(objComponent, fltMaximumHitpoints);
				space_crafting.setComponentMaximumArmorHitpoints(objComponent, fltMaximumHitpoints);
				
				float fltEnergyMaintenance = dctParams.getFloat("fltEnergyMaintenance");
				space_crafting.setComponentEnergyMaintenance(objComponent, fltEnergyMaintenance);
				
				float fltMass = dctParams.getFloat("fltMass");
				space_crafting.setComponentMass(objComponent, fltMass);
				
				setComponentObjVar(objComponent, "character.builder", 1);
				String newName = dctParams.getString("strName");
				setName(objComponent, newName);
			}
			else if (strComponentType.equals("reactor"))
			{
				LOG("tweakSpaceShipComponent", "REACTOR"+ getTemplateName(objComponent));
				
				float fltEnergyGeneration = dctParams.getFloat("fltEnergyGeneration");
				space_crafting.setReactorEnergyGeneration(objComponent, fltEnergyGeneration);
				
				float fltMaximumHitpoints = dctParams.getFloat("fltMaximumHitpoints");
				space_crafting.setComponentCurrentHitpoints(objComponent, fltMaximumHitpoints);
				space_crafting.setComponentMaximumHitpoints(objComponent, fltMaximumHitpoints);
				
				float fltMaximumArmorHitpoints = dctParams.getFloat("fltMaximumArmorHitpoints");
				space_crafting.setComponentCurrentArmorHitpoints(objComponent, fltMaximumHitpoints);
				space_crafting.setComponentMaximumArmorHitpoints(objComponent, fltMaximumHitpoints);
				
				float fltMass = dctParams.getFloat("fltMass");
				space_crafting.setComponentMass(objComponent, fltMass);
				
				setComponentObjVar(objComponent, "character.builder", 1);
				String newName = dctParams.getString("strName");
				setName(objComponent, newName);
			}
			else if (strComponentType.equals("shield"))
			{
				LOG("tweakSpaceShipComponent", "SHIELD"+ getTemplateName(objComponent));
				
				float fltShieldHitpointsMaximumFront = dctParams.getFloat("fltShieldHitpointsMaximumFront");
				float fltShieldHitpointsMaximumBack = dctParams.getFloat("fltShieldHitpointsMaximumFront");
				space_crafting.setShieldGeneratorCurrentFrontHitpoints(objComponent, 0f);
				space_crafting.setShieldGeneratorCurrentBackHitpoints(objComponent, 0f);
				space_crafting.setShieldGeneratorMaximumFrontHitpoints(objComponent, fltShieldHitpointsMaximumFront);
				space_crafting.setShieldGeneratorMaximumBackHitpoints(objComponent, fltShieldHitpointsMaximumBack);
				
				float fltShieldRechargeRate = dctParams.getFloat("fltShieldRechargeRate");
				space_crafting.setShieldGeneratorRechargeRate(objComponent, fltShieldRechargeRate);
				
				float fltMaximumHitpoints = dctParams.getFloat("fltMaximumHitpoints");
				space_crafting.setComponentCurrentHitpoints(objComponent, fltMaximumHitpoints);
				space_crafting.setComponentMaximumHitpoints(objComponent, fltMaximumHitpoints);
				
				float fltMaximumArmorHitpoints = dctParams.getFloat("fltMaximumArmorHitpoints");
				space_crafting.setComponentCurrentArmorHitpoints(objComponent, fltMaximumHitpoints);
				space_crafting.setComponentMaximumArmorHitpoints(objComponent, fltMaximumHitpoints);
				
				float fltEnergyMaintenance = dctParams.getFloat("fltEnergyMaintenance");
				space_crafting.setComponentEnergyMaintenance(objComponent, fltEnergyMaintenance);
				
				float fltMass = dctParams.getFloat("fltMass");
				space_crafting.setComponentMass(objComponent, fltMass);
				
				setComponentObjVar(objComponent, "character.builder", 1);
				String newName = dctParams.getString("strName");
				setName(objComponent, newName);
			}
			else if (strComponentType.equals("weapon"))
			{
				float fltMinDamage = dctParams.getFloat("fltMinDamage");
				space_crafting.setWeaponMinimumDamage(objComponent, fltMinDamage);
				
				float fltMaxDamage = dctParams.getFloat("fltMaxDamage");
				space_crafting.setWeaponMaximumDamage(objComponent, fltMaxDamage);
				
				float fltShieldEffectiveness = dctParams.getFloat("fltShieldEffectiveness");
				space_crafting.setWeaponShieldEffectiveness(objComponent, fltShieldEffectiveness);
				
				float fltArmorEffectiveness = dctParams.getFloat("fltArmorEffectiveness");
				space_crafting.setWeaponArmorEffectiveness(objComponent, fltArmorEffectiveness);
				
				float fltEnergyPerShot = dctParams.getFloat("fltEnergyPerShot");
				space_crafting.setWeaponEnergyPerShot(objComponent, fltEnergyPerShot);
				
				float fltRefireRate = dctParams.getFloat("fltRefireRate");
				space_crafting.setWeaponRefireRate(objComponent, fltRefireRate);
				
				float fltMaximumHitpoints = dctParams.getFloat("fltMaximumHitpoints");
				space_crafting.setComponentCurrentHitpoints(objComponent, fltMaximumHitpoints);
				space_crafting.setComponentMaximumHitpoints(objComponent, fltMaximumHitpoints);
				
				float fltMaximumArmorHitpoints = dctParams.getFloat("fltMaximumArmorHitpoints");
				space_crafting.setComponentCurrentArmorHitpoints(objComponent, fltMaximumHitpoints);
				space_crafting.setComponentMaximumArmorHitpoints(objComponent, fltMaximumHitpoints);
				
				float fltEnergyMaintenance = dctParams.getFloat("fltEnergyMaintenance");
				space_crafting.setComponentEnergyMaintenance(objComponent, fltEnergyMaintenance);
				
				float fltMass = dctParams.getFloat("fltMass");
				space_crafting.setComponentMass(objComponent, fltMass);
				
				setComponentObjVar(objComponent, "character.builder", 1);
				String newName = dctParams.getString("strName");
				setName(objComponent, newName);
			}
			else if (strComponentType.equals("capacitor"))
			{
				LOG("tweakSpaceShipComponent", "CAPACITOR"+ getTemplateName(objComponent));
				
				float fltMaxEnergy = dctParams.getFloat("fltMaxEnergy");
				space_crafting.setWeaponCapacitorMaximumEnergy(objComponent, fltMaxEnergy);
				space_crafting.setWeaponCapacitorCurrentEnergy(objComponent, fltMaxEnergy);
				
				float fltRechargeRate = dctParams.getFloat("fltRechargeRate");
				space_crafting.setWeaponCapacitorRechargeRate(objComponent, fltRechargeRate);
				float fltMaximumHitpoints = dctParams.getFloat("fltMaximumHitpoints");
				space_crafting.setComponentCurrentHitpoints(objComponent, fltMaximumHitpoints);
				space_crafting.setComponentMaximumHitpoints(objComponent, fltMaximumHitpoints);
				
				float fltMaximumArmorHitpoints = dctParams.getFloat("fltMaximumArmorHitpoints");
				space_crafting.setComponentCurrentArmorHitpoints(objComponent, fltMaximumHitpoints);
				space_crafting.setComponentMaximumArmorHitpoints(objComponent, fltMaximumHitpoints);
				
				float fltEnergyMaintenance = dctParams.getFloat("fltEnergyMaintenance");
				space_crafting.setComponentEnergyMaintenance(objComponent, fltEnergyMaintenance);
				
				float fltMass = dctParams.getFloat("fltMass");
				space_crafting.setComponentMass(objComponent, fltMass);
				
				setComponentObjVar(objComponent, "character.builder", 1);
				String newName = dctParams.getString("strName");
				setName(objComponent, newName);
			}
			return objComponent;
		}
		else
		{
			LOG("tweakSpaceShipComponent", "MAJOR MESSUP! "+ objComponent + " could not be modified as it was not valid");
			return null;
		}
	}
	
	
	public boolean setComponentObjVar(obj_id objComponent, String objVarName, float fltValue) throws InterruptedException
	{
		return setObjVar(objComponent, objVarName, fltValue);
	}
	
	
	public boolean issueShipChassis(obj_id player, int idx) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			return false;
		}
		
		LOG("issueShipChassis", "idx = "+ idx);
		
		if (idx < 0)
		{
			return false;
		}
		
		LOG("issueShipChassis", "issueShipChassis function");
		
		String type = dataTableGetString(SHIP_CHASSIS_TBL, idx, "shipType");
		
		String skill = dataTableGetString(SHIP_CHASSIS_TBL, type, "skill");
		
		float mass = dataTableGetFloat(SHIP_CHASSIS_TBL, idx, "fltMass");
		
		float hp = dataTableGetFloat(SHIP_CHASSIS_TBL, idx, "fltHitPoints");
		
		LOG("issueShipChassis", "SKILL: "+ skill);
		
		if (type.equals("firespray") || type.equals("yt2400"))
		{
			if (hasSkill(player, "pilot_rebel_navy_master") || !hasSkill(player, "pilot_imperial_navy_master") || !hasSkill(player, "pilot_neutral_master"))
			{
				giveShipChassis(player, type, mass, hp);
				return true;
			}
			sendSystemMessageTestingOnly(player, "You cannot use the Firespray due to certification requirements. Skill Required is: "+ skill);
			return false;
		}
		
		if (!hasSkill(player, skill))
		{
			sendSystemMessageTestingOnly(player, "You cannot use this ship due to certification requirements. Skill Required is: "+ skill);
			return false;
		}
		
		giveShipChassis(player, type, mass, hp);
		return true;
	}
	
	
	public boolean giveShipChassis(obj_id player, String type, float mass, float hp) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			return false;
		}
		
		if (type == null || type.equals(""))
		{
			return false;
		}
		
		if (mass < 0 || hp < 0)
		{
			return false;
		}
		
		obj_id pInv = utils.getInventoryContainer(player);
		
		obj_id pcd = space_crafting.createDeedFromBlueprints(player, type, pInv, mass, hp);
		if (!isIdValid(pcd))
		{
			
			sendSystemMessageTestingOnly(player, "The ship was not created due to error.");
			LOG("issueShipChassis", "CHASSIS"+ type + " IS BAD");
			
			return false;
		}
		
		sendSystemMessageTestingOnly(player, "Chassis Issued.");
		return true;
	}
	
	
	public void handleDraftSchematicsOption(obj_id player) throws InterruptedException
	{
		refreshMenu(player, "Select the desired crafting profession", "Character Builder Terminal", CRAFTING_PROFESSIONS, "handleDraftSchematicsList", false);
	}
	
	
	public int handleDraftSchematicsList(obj_id self, dictionary params) throws InterruptedException
	{
		if (params == null || params.isEmpty())
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		if (btn == sui.BP_REVERT)
		{
			startCharacterBuilder(player);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		if (idx == -1 || idx > CRAFTING_SKILL_TEMPLATES.length)
		{
			cleanScriptVars(player);
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(player) || !isGod(player))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		String[] skillList = skill_template.getSkillTemplateSkillsByTemplateName(CRAFTING_SKILL_TEMPLATES[idx]);
		
		String[] schematicGroups = craftinglib.getDraftSchematicGroupsFromSkills(skillList);
		
		String[] schematics = craftinglib.getDraftSchematicsFromGroups(schematicGroups);
		
		Arrays.sort(schematics);
		
		utils.setScriptVar(player, "character_builder.schematicsList", schematics);
		
		float craftPercentage = 100.0f;
		
		if (!utils.hasScriptVar(player, "character_builder.qualityPercentage"))
		{
			utils.setScriptVar(player, "character_builder.qualityPercentage", 100.0f);
		}
		else
		{
			craftPercentage = utils.getFloatScriptVar(player, "character_builder.qualityPercentage");
		}
		
		if (utils.hasScriptVar(player, "character_builder.qualityPercentagePID"))
		{
			int oldpid = utils.getIntScriptVar(player, "character_builder.qualityPercentagePID");
			sui.closeSUI(player, oldpid);
		}
		
		refreshMenu(player, "Select a profession draft schematic. Schematics that require items in addition to resources may not be crafted properly (armor, weapons, droids). Instead, use this to select the items required for their schematic and then use the crafting tool.", "Character Builder Terminal", schematics, "handleSchematicSelect", false);
		int pid = sui.inputbox(self, player, "A high crafting percentage can result in a crafted item that players cannot create legally.", sui.OK_CANCEL, "Crafting Percentage", sui.INPUT_NORMAL, null, "handleCraftQualityPercentage", null);
		
		utils.setScriptVar(player, "character_builder.qualityPercentagePID", pid);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleCraftQualityPercentage(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id player = sui.getPlayerId(params);
		String stringPercentage = sui.getInputBoxText(params);
		int btn = sui.getIntButtonPressed(params);
		
		if (btn == sui.BP_CANCEL)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(player) || !isGod(player))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			return SCRIPT_OVERRIDE;
		}
		
		float floatPercentage = utils.stringToFloat(stringPercentage);
		
		if (floatPercentage > 100.0f || floatPercentage < 0.0f)
		{
			sendSystemMessageTestingOnly(player, "Bad Crafting Percentage.");
			return SCRIPT_OVERRIDE;
		}
		
		utils.setScriptVar(player, "character_builder.qualityPercentage", floatPercentage);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleSchematicSelect(obj_id self, dictionary params) throws InterruptedException
	{
		if (params == null || params.isEmpty())
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		if (btn == sui.BP_REVERT)
		{
			handleDraftSchematicsOption(player);
			return SCRIPT_CONTINUE;
		}
		
		if (btn == sui.BP_CANCEL)
		{
			cleanScriptVars(player);
			closeOldWindow(player);
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(player) || !isGod(player))
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		String[] schematics = utils.getStringArrayScriptVar(player, "character_builder.schematicsList");
		
		if (schematics == null || schematics.length <= 0)
		{
			sendSystemMessageTestingOnly(player, "The system is unable to complete the transaction.");
			cleanScriptVars(player);
			return SCRIPT_OVERRIDE;
		}
		
		float craftPercentage = 100.0f;
		
		if (utils.hasScriptVar(player, "character_builder.qualityPercentage"))
		{
			craftPercentage = utils.getFloatScriptVar(player, "character_builder.qualityPercentage");
		}
		
		obj_id container = utils.getInventoryContainer(player);
		
		obj_id craftedItem = makeCraftedItem(schematics[idx], craftPercentage, container);
		
		if (isIdValid(craftedItem))
		{
			sendSystemMessageTestingOnly(player, "Making: "+ getName(craftedItem));
		}
		else
		{
			sendSystemMessageTestingOnly(player, "Failed to make: "+ schematics[idx]);
		}
		
		if (utils.hasScriptVar(player, "character_builder.qualityPercentagePID"))
		{
			int oldpid = utils.getIntScriptVar(player, "character_builder.qualityPercentagePID");
			sui.closeSUI(player, oldpid);
		}
		
		refreshMenu(player, "Select the draft schematic. Schematics that require items in addition to resources may not be crafted properly (armor, weapons, droids). Instead, use this to select the items required for their schematic and then use the crafting tool.", "Character Builder Terminal", schematics, "handleSchematicSelect", false);
		int pid = sui.inputbox(self, player, "A high crafting percentage can result in a crafted item that players cannot create legally.", sui.OK_CANCEL, "A high crafting percentage ", sui.INPUT_NORMAL, null, "handleCraftQualityPercentage", null);
		
		utils.setScriptVar(player, "character_builder.qualityPercentagePID", pid);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void setSocketsUp(obj_id item) throws InterruptedException
	{
		
		setSkillModSockets(item, 1);
		setCondition(item, CONDITION_MAGIC_ITEM);
		
	}
	
	
	public void createSnowFlakeFrogWeapon(obj_id player, String weaponName) throws InterruptedException
	{
		obj_id pInv = utils.getInventoryContainer(player);
		
		obj_id weaponObject = weapons.createWeapon(weaponName, pInv, weapons.VIA_TEMPLATE, WEAPON_SPEED, WEAPON_DAMAGE, WEAPON_EFFECIENCY, WEAPON_ELEMENTAL);
		if (isIdValid(weaponObject))
		{
			setSocketsUp(weaponObject);
			sendSystemMessageTestingOnly(player, "Weapon Issued!");
		}
		
	}
}
