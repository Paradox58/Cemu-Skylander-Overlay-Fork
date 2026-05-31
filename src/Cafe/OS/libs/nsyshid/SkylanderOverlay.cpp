// Skylanders Portal overlay — ImGui 1.89.5
// F8 toggles the window while a game is running.

#include "SkylanderOverlay.h"
#include "Skylander.h"
#include "nsyshid.h"
#include "Common/FileStream.h"
#include "config/CemuConfig.h"
#include <imgui.h>
#include "imgui/imgui_extension.h"

#include <atomic>
#include <filesystem>
#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <cstdio>
#include <cctype>
#include <cstring>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <commdlg.h>
#include <shobjidl.h>
#endif

namespace fs = std::filesystem;

// ================================================================
//  Name table
// ================================================================
static const std::map<std::pair<uint16_t,uint16_t>,const char*> s_names = {
    {{0,0x0000},"Whirlwind"},{{0,0x1801},"Series 2 Whirlwind"},{{0,0x1C02},"Polar Whirlwind"},{{0,0x2805},"Horn Blast Whirlwind"},{{0,0x3810},"Eon's Elite Whirlwind"},
    {{1,0x0000},"Sonic Boom"},{{1,0x1801},"Series 2 Sonic Boom"},
    {{2,0x0000},"Warnado"},{{2,0x2206},"LightCore Warnado"},
    {{3,0x0000},"Lightning Rod"},{{3,0x1801},"Series 2 Lightning Rod"},
    {{4,0x0000},"Bash"},{{4,0x1801},"Series 2 Bash"},
    {{5,0x0000},"Terrafin"},{{5,0x1801},"Series 2 Terrafin"},{{5,0x2805},"Knockout Terrafin"},{{5,0x3810},"Eon's Elite Terrafin"},
    {{6,0x0000},"Dino Rang"},{{6,0x4810},"Eon's Elite Dino Rang"},
    {{7,0x0000},"Prism Break"},{{7,0x1801},"Series 2 Prism Break"},{{7,0x2805},"Hyper Beam Prism Break"},{{7,0x1206},"LightCore Prism Break"},
    {{8,0x0000},"Sunburn"},
    {{9,0x0000},"Eruptor"},{{9,0x1801},"Series 2 Eruptor"},{{9,0x2C02},"Volcanic Eruptor"},{{9,0x2805},"Lava Barf Eruptor"},{{9,0x1206},"LightCore Eruptor"},{{9,0x3810},"Eon's Elite Eruptor"},
    {{10,0x0000},"Ignitor"},{{10,0x1801},"Series 2 Ignitor"},{{10,0x1C03},"Legendary Ignitor"},
    {{11,0x0000},"Flameslinger"},{{11,0x1801},"Series 2 Flameslinger"},
    {{12,0x0000},"Zap"},{{12,0x1801},"Series 2 Zap"},
    {{13,0x0000},"Wham Shell"},{{13,0x2206},"LightCore Wham Shell"},
    {{14,0x0000},"Gill Grunt"},{{14,0x1801},"Series 2 Gill Grunt"},{{14,0x2805},"Anchors Away Gill Grunt"},{{14,0x3805},"Tidal Wave Gill Grunt"},{{14,0x3810},"Eon's Elite Gill Grunt"},
    {{15,0x0000},"Slam Bam"},{{15,0x1801},"Series 2 Slam Bam"},{{15,0x1C03},"Legendary Slam Bam"},{{15,0x4810},"Eon's Elite Slam Bam"},
    {{16,0x0000},"Spyro"},{{16,0x1801},"Series 2 Spyro"},{{16,0x2C02},"Dark Mega Ram Spyro"},{{16,0x2805},"Mega Ram Spyro"},{{16,0x3810},"Eon's Elite Spyro"},
    {{17,0x0000},"Voodood"},{{17,0x4810},"Eon's Elite Voodood"},
    {{18,0x0000},"Double Trouble"},{{18,0x1801},"Series 2 Double Trouble"},{{18,0x1C02},"Royal Double Trouble"},
    {{19,0x0000},"Trigger Happy"},{{19,0x1801},"Series 2 Trigger Happy"},{{19,0x2C02},"Springtime Trigger Happy"},{{19,0x2805},"Big Bang Trigger Happy"},{{19,0x3810},"Eon's Elite Trigger Happy"},
    {{20,0x0000},"Drobot"},{{20,0x1801},"Series 2 Drobot"},{{20,0x1206},"LightCore Drobot"},
    {{21,0x0000},"Drill Seargeant"},{{21,0x1801},"Series 2 Drill Seargeant"},
    {{22,0x0000},"Boomer"},{{22,0x4810},"Eon's Elite Boomer"},
    {{23,0x0000},"Wrecking Ball"},{{23,0x1801},"Series 2 Wrecking Ball"},
    {{24,0x0000},"Camo"},{{24,0x2805},"Thorn Horn Camo"},
    {{25,0x0000},"Zook"},{{25,0x1801},"Series 2 Zook"},{{25,0x4810},"Eon's Elite Zook"},
    {{26,0x0000},"Stealth Elf"},{{26,0x1801},"Series 2 Stealth Elf"},{{26,0x2C02},"Dark Stealth Elf"},{{26,0x1C03},"Legendary Stealth Elf"},{{26,0x2805},"Ninja Stealth Elf"},{{26,0x3810},"Eon's Elite Stealth Elf"},
    {{27,0x0000},"Stump Smash"},{{27,0x1801},"Series 2 Stump Smash"},
    {{28,0x0000},"Dark Spyro"},
    {{29,0x0000},"Hex"},{{29,0x1801},"Series 2 Hex"},{{29,0x1206},"LightCore Hex"},
    {{30,0x0000},"Chop Chop"},{{30,0x1801},"Series 2 Chop Chop"},{{30,0x2805},"Twin Blade Chop Chop"},{{30,0x3810},"Eon's Elite Chop Chop"},
    {{31,0x0000},"Ghost Roaster"},{{31,0x4810},"Eon's Elite Ghost Roaster"},
    {{32,0x0000},"Cynder"},{{32,0x1801},"Series 2 Cynder"},{{32,0x2805},"Phantom Cynder"},
    {{100,0x0000},"Jet Vac"},{{100,0x1403},"Legendary Jet Vac"},{{100,0x2805},"Turbo Jet Vac"},{{100,0x3805},"Full Blast Jet Vac"},{{100,0x1206},"LightCore Jet Vac"},
    {{101,0x0000},"Swarm"},{{102,0x0000},"Crusher"},{{102,0x1602},"Granite Crusher"},
    {{103,0x0000},"Flashwing"},{{103,0x1402},"Jade Flash Wing"},{{103,0x2206},"LightCore Flashwing"},
    {{104,0x0000},"Hot Head"},{{105,0x0000},"Hot Dog"},{{105,0x1402},"Molten Hot Dog"},{{105,0x2805},"Fire Bone Hot Dog"},
    {{106,0x0000},"Chill"},{{106,0x1603},"Legendary Chill"},{{106,0x2805},"Blizzard Chill"},{{106,0x1206},"LightCore Chill"},
    {{107,0x0000},"Thumpback"},
    {{108,0x0000},"Pop Fizz"},{{108,0x1402},"Punch Pop Fizz"},{{108,0x3C02},"Love Potion Pop Fizz"},{{108,0x2805},"Super Gulp Pop Fizz"},{{108,0x3805},"Fizzy Frenzy Pop Fizz"},{{108,0x1206},"LightCore Pop Fizz"},
    {{109,0x0000},"Ninjini"},{{109,0x1602},"Scarlet Ninjini"},
    {{110,0x0000},"Bouncer"},{{110,0x1603},"Legendary Bouncer"},
    {{111,0x0000},"Sprocket"},{{111,0x2805},"Heavy Duty Sprocket"},
    {{112,0x0000},"Tree Rex"},{{112,0x1602},"Gnarly Tree Rex"},
    {{113,0x0000},"Shroomboom"},{{113,0x3805},"Sure Shot Shroomboom"},{{113,0x1206},"LightCore Shroomboom"},
    {{114,0x0000},"Eye Brawl"},{{115,0x0000},"Fright Rider"},
    {{200,0x0000},"Anvil Rain"},{{201,0x0000},"Hidden Treasure"},{{201,0x2000},"Platinum Hidden Treasure"},
    {{202,0x0000},"Healing Elixir"},{{203,0x0000},"Ghost Pirate Swords"},{{204,0x0000},"Time Twist Hourglass"},
    {{205,0x0000},"Sky Iron Shield"},{{206,0x0000},"Winged Boots"},{{207,0x0000},"Sparx the Dragonfly"},
    {{208,0x0000},"Dragonfire Cannon"},{{208,0x1602},"Golden Dragonfire Cannon"},{{209,0x0000},"Scorpion Striker"},
    {{210,0x3002},"Biter's Bane"},{{210,0x3008},"Sorcerous Skull"},{{210,0x300B},"Axe of Illusion"},{{210,0x300E},"Arcane Hourglass"},{{210,0x3012},"Spell Slapper"},{{210,0x3014},"Rune Rocket"},
    {{211,0x3001},"Tidal Tiki"},{{211,0x3002},"Wet Walter"},{{211,0x3006},"Flood Flask"},{{211,0x3406},"Legendary Flood Flask"},{{211,0x3007},"Soaking Staff"},{{211,0x300B},"Aqua Axe"},{{211,0x3016},"Frost Helm"},
    {{212,0x3003},"Breezy Bird"},{{212,0x3006},"Drafty Decanter"},{{212,0x300D},"Tempest Timer"},{{212,0x3010},"Cloudy Cobra"},{{212,0x3011},"Storm Warning"},{{212,0x3018},"Cyclone Saber"},
    {{213,0x3004},"Spirit Sphere"},{{213,0x3404},"Legendary Spirit Sphere"},{{213,0x3008},"Spectral Skull"},{{213,0x3408},"Legendary Spectral Skull"},{{213,0x300B},"Haunted Hatchet"},{{213,0x300C},"Grim Gripper"},{{213,0x3010},"Spooky Snake"},{{213,0x3017},"Dream Piercer"},
    {{214,0x3000},"Tech Totem"},{{214,0x3007},"Automatic Angel"},{{214,0x3009},"Factory Flower"},{{214,0x300C},"Grabbing Gadget"},{{214,0x3016},"Makers Mana"},{{214,0x301A},"Topsy Techy"},
    {{215,0x3005},"Eternal Flame"},{{215,0x3009},"Fire Flower"},{{215,0x3011},"Scorching Stopper"},{{215,0x3012},"Searing Spinner"},{{215,0x3017},"Spark Spear"},{{215,0x301B},"Blazing Belch"},
    {{216,0x3000},"Banded Boulder"},{{216,0x3003},"Rock Hawk"},{{216,0x300A},"Slag Hammer"},{{216,0x300E},"Dust Of Time"},{{216,0x3013},"Spinning Sandstorm"},{{216,0x301A},"Rubble Trouble"},
    {{217,0x3003},"Oak Eagle"},{{217,0x3005},"Emerald Energy"},{{217,0x300A},"Weed Whacker"},{{217,0x3010},"Seed Serpent"},{{217,0x3018},"Jade Blade"},{{217,0x301B},"Shrub Shrieker"},
    {{218,0x3000},"Dark Dagger"},{{218,0x3014},"Shadow Spider"},{{218,0x301A},"Ghastly Grimace"},
    {{219,0x3000},"Shining Ship"},{{219,0x300F},"Heavenly Hawk"},{{219,0x301B},"Beam Scream"},
    {{220,0x301E},"Kaos Trap"},{{220,0x351F},"Ultimate Kaos Trap"},
    {{230,0x0000},"Hand of Fate"},{{230,0x3403},"Legendary Hand of Fate"},
    {{231,0x0000},"Piggy Bank"},{{232,0x0000},"Rocket Ram"},{{233,0x0000},"Tiki Speaky"},
    {{300,0x0000},"Dragon's Peak"},{{301,0x0000},"Empire of Ice"},{{302,0x0000},"Pirate Seas"},
    {{303,0x0000},"Darklight Crypt"},{{304,0x0000},"Volcanic Vault"},{{305,0x0000},"Mirror of Mystery"},
    {{306,0x0000},"Nightmare Express"},{{307,0x0000},"Sunscraper Spire"},{{308,0x0000},"Midnight Museum"},
    {{404,0x0000},"Legendary Bash"},{{416,0x0000},"Legendary Spyro"},{{419,0x0000},"Legendary Trigger Happy"},{{430,0x0000},"Legendary Chop Chop"},
    {{450,0x0000},"Gusto"},{{451,0x0000},"Thunderbolt"},{{452,0x0000},"Fling Kong"},{{453,0x0000},"Blades"},{{453,0x3403},"Legendary Blades"},
    {{454,0x0000},"Wallop"},{{455,0x0000},"Head Rush"},{{455,0x3402},"Nitro Head Rush"},{{456,0x0000},"Fist Bump"},{{457,0x0000},"Rocky Roll"},
    {{458,0x0000},"Wildfire"},{{458,0x3402},"Dark Wildfire"},{{459,0x0000},"Ka Boom"},{{460,0x0000},"Trail Blazer"},{{461,0x0000},"Torch"},
    {{462,0x3000},"Snap Shot"},{{462,0x3402},"Dark Snap Shot"},{{463,0x0000},"Lob Star"},{{463,0x3402},"Winterfest Lob-Star"},
    {{464,0x0000},"Flip Wreck"},{{465,0x0000},"Echo"},{{466,0x0000},"Blastermind"},{{467,0x0000},"Enigma"},
    {{468,0x0000},"Deja Vu"},{{468,0x3403},"Legendary Deja Vu"},{{469,0x0000},"Cobra Candabra"},{{469,0x3402},"King Cobra Cadabra"},
    {{470,0x0000},"Jawbreaker"},{{470,0x3403},"Legendary Jawbreaker"},{{471,0x0000},"Gearshift"},{{472,0x0000},"Chopper"},{{473,0x0000},"Tread Head"},
    {{474,0x0000},"Bushwack"},{{474,0x3403},"Legendary Bushwack"},{{475,0x0000},"Tuff Luck"},{{476,0x0000},"Food Fight"},{{476,0x3402},"Dark Food Fight"},
    {{477,0x0000},"High Five"},{{478,0x0000},"Krypt King"},{{478,0x3402},"Nitro Krypt King"},{{479,0x0000},"Short Cut"},{{480,0x0000},"Bat Spin"},{{481,0x0000},"Funny Bone"},
    {{482,0x0000},"Knight Light"},{{483,0x0000},"Spotlight"},{{484,0x0000},"Knight Mare"},{{485,0x0000},"Blackout"},
    {{502,0x0000},"Bop"},{{503,0x0000},"Spry"},{{504,0x0000},"Hijinx"},{{505,0x0000},"Terrabite"},{{506,0x0000},"Breeze"},
    {{507,0x0000},"Weeruptor"},{{507,0x3402},"Eggcellent Weeruptor"},{{508,0x0000},"Pet Vac"},{{508,0x3402},"Power Punch Pet Vac"},
    {{509,0x0000},"Small Fry"},{{510,0x0000},"Drobit"},{{514,0x0000},"Gill Runt"},{{519,0x0000},"Trigger Snappy"},{{526,0x0000},"Whisper Elf"},
    {{540,0x0000},"Barkley"},{{540,0x3402},"Gnarly Barkley"},{{541,0x0000},"Thumpling"},{{542,0x0000},"Mini-Jini"},{{543,0x0000},"Eye Small"},
    {{601,0x0000},"King Pen"},{{602,0x0000},"Tri-Tip"},{{603,0x0000},"Chopscotch"},{{604,0x0000},"Boom Bloom"},{{605,0x0000},"Pit Boss"},{{606,0x0000},"Barbella"},
    {{607,0x0000},"Air Strike"},{{608,0x0000},"Ember"},{{609,0x0000},"Ambush"},{{610,0x0000},"Dr. Krankcase"},{{611,0x0000},"Hood Sickle"},{{612,0x0000},"Tae Kwon Crow"},
    {{613,0x0000},"Golden Queen"},{{614,0x0000},"Wolfgang"},{{615,0x0000},"Pain-Yatta"},{{616,0x0000},"Mysticat"},{{617,0x0000},"Starcast"},{{618,0x0000},"Buckshot"},
    {{619,0x0000},"Aurora"},{{620,0x0000},"Flare Wolf"},{{621,0x0000},"Chompy Mage"},{{622,0x0000},"Bad Juju"},{{623,0x0000},"Grave Clobber"},{{624,0x0000},"Blaster-Tron"},
    {{625,0x0000},"Ro-Bow"},{{626,0x0000},"Chain Reaction"},{{627,0x0000},"Kaos"},{{628,0x0000},"Wild Storm"},{{629,0x0000},"Tidepool"},
    {{630,0x0000},"Crash Bandicoot"},{{631,0x0000},"Dr. Neo Cortex"},
    {{1000,0x0000},"Boom Jet (Bottom)"},{{1001,0x0000},"Free Ranger (Bottom)"},{{1001,0x2403},"Legendary Free Ranger (Bottom)"},
    {{1002,0x0000},"Rubble Rouser (Bottom)"},{{1003,0x0000},"Doom Stone (Bottom)"},{{1004,0x0000},"Blast Zone (Bottom)"},{{1004,0x2402},"Dark Blast Zone (Bottom)"},
    {{1005,0x0000},"Fire Kraken (Bottom)"},{{1005,0x2402},"Jade Fire Kraken (Bottom)"},{{1006,0x0000},"Stink Bomb (Bottom)"},{{1007,0x0000},"Grilla Drilla (Bottom)"},
    {{1008,0x0000},"Hoot Loop (Bottom)"},{{1008,0x2402},"Enchanted Hoot Loop (Bottom)"},{{1009,0x0000},"Trap Shadow (Bottom)"},
    {{1010,0x0000},"Magna Charge (Bottom)"},{{1010,0x2402},"Nitro Magna Charge (Bottom)"},{{1011,0x0000},"Spy Rise (Bottom)"},
    {{1012,0x0000},"Night Shift (Bottom)"},{{1012,0x2403},"Legendary Night Shift (Bottom)"},{{1013,0x0000},"Rattle Shake (Bottom)"},{{1013,0x2402},"Quick Draw Rattle Shake (Bottom)"},
    {{1014,0x0000},"Freeze Blade (Bottom)"},{{1014,0x2402},"Nitro Freeze Blade (Bottom)"},{{1015,0x0000},"Wash Buckler (Bottom)"},{{1015,0x2402},"Dark Wash Buckler (Bottom)"},
    {{2000,0x0000},"Boom Jet (Top)"},{{2001,0x0000},"Free Ranger (Top)"},{{2001,0x2403},"Legendary Free Ranger (Top)"},
    {{2002,0x0000},"Rubble Rouser (Top)"},{{2003,0x0000},"Doom Stone (Top)"},{{2004,0x0000},"Blast Zone (Top)"},{{2004,0x2402},"Dark Blast Zone (Top)"},
    {{2005,0x0000},"Fire Kraken (Top)"},{{2005,0x2402},"Jade Fire Kraken (Top)"},{{2006,0x0000},"Stink Bomb (Top)"},{{2007,0x0000},"Grilla Drilla (Top)"},
    {{2008,0x0000},"Hoot Loop (Top)"},{{2008,0x2402},"Enchanted Hoot Loop (Top)"},{{2009,0x0000},"Trap Shadow (Top)"},
    {{2010,0x0000},"Magna Charge (Top)"},{{2010,0x2402},"Nitro Magna Charge (Top)"},{{2011,0x0000},"Spy Rise (Top)"},
    {{2012,0x0000},"Night Shift (Top)"},{{2012,0x2403},"Legendary Night Shift (Top)"},{{2013,0x0000},"Rattle Shake (Top)"},{{2013,0x2402},"Quick Draw Rattle Shake (Top)"},
    {{2014,0x0000},"Freeze Blade (Top)"},{{2014,0x2402},"Nitro Freeze Blade (Top)"},{{2015,0x0000},"Wash Buckler (Top)"},{{2015,0x2402},"Dark Wash Buckler (Top)"},
    {{3000,0x0000},"Scratch"},{{3001,0x0000},"Pop Thorn"},{{3002,0x0000},"Slobber Tooth"},{{3002,0x2402},"Dark Slobber Tooth"},{{3003,0x0000},"Scorp"},
    {{3004,0x0000},"Fryno"},{{3004,0x3805},"Hog Wild Fryno"},{{3005,0x0000},"Smolderdash"},{{3005,0x2206},"LightCore Smolderdash"},
    {{3006,0x0000},"Bumble Blast"},{{3006,0x2402},"Jolly Bumble Blast"},{{3006,0x2206},"LightCore Bumble Blast"},{{3007,0x0000},"Zoo Lou"},{{3007,0x2403},"Legendary Zoo Lou"},
    {{3008,0x0000},"Dune Bug"},{{3009,0x0000},"Star Strike"},{{3009,0x2602},"Enchanted Star Strike"},{{3009,0x2206},"LightCore Star Strike"},
    {{3010,0x0000},"Countdown"},{{3010,0x2402},"Kickoff Countdown"},{{3010,0x2206},"LightCore Countdown"},{{3011,0x0000},"Wind Up"},{{3011,0x2404},"Gear Head VVind Up"},
    {{3012,0x0000},"Roller Brawl"},{{3013,0x0000},"Grim Creeper"},{{3013,0x2603},"Legendary Grim Creeper"},{{3013,0x2206},"LightCore Grim Creeper"},
    {{3014,0x0000},"Rip Tide"},{{3015,0x0000},"Punk Shock"},
    {{3200,0x0000},"Battle Hammer"},{{3201,0x0000},"Sky Diamond"},{{3202,0x0000},"Platinum Sheep"},{{3203,0x0000},"Groove Machine"},{{3204,0x0000},"UFO Hat"},
    {{3300,0x0000},"Sheep Wreck Island"},{{3301,0x0000},"Tower of Time"},{{3302,0x0000},"Fiery Forge"},{{3303,0x0000},"Arkeyan Crossbow"},
    {{3220,0x0000},"Jet Stream"},{{3221,0x0000},"Tomb Buggy"},{{3222,0x0000},"Reef Ripper"},{{3223,0x0000},"Burn Cycle"},
    {{3224,0x0000},"Hot Streak"},{{3224,0x4402},"Dark Hot Streak"},{{3224,0x4004},"E3 Hot Streak"},{{3224,0x441E},"Golden Hot Streak"},
    {{3225,0x0000},"Shark Tank"},{{3226,0x0000},"Thump Truck"},{{3227,0x0000},"Crypt Crusher"},{{3228,0x0000},"Stealth Stinger"},{{3228,0x4402},"Nitro Stealth Stinger"},
    {{3231,0x0000},"Dive Bomber"},{{3231,0x4402},"Spring Ahead Dive Bomber"},{{3232,0x0000},"Sky Slicer"},
    {{3233,0x0000},"Clown Cruiser (Nintendo Only)"},{{3233,0x4402},"Dark Clown Cruiser (Nintendo Only)"},
    {{3234,0x0000},"Gold Rusher"},{{3234,0x4402},"Power Blue Gold Rusher"},{{3235,0x0000},"Shield Striker"},
    {{3236,0x0000},"Sun Runner"},{{3236,0x4403},"Legendary Sun Runner"},{{3237,0x0000},"Sea Shadow"},{{3237,0x4402},"Dark Sea Shadow"},
    {{3238,0x0000},"Splatter Splasher"},{{3238,0x4402},"Power Blue Splatter Splasher"},{{3239,0x0000},"Soda Skimmer"},{{3239,0x4402},"Nitro Soda Skimmer"},
    {{3240,0x0000},"Barrel Blaster (Nintendo Only)"},{{3240,0x4402},"Dark Barrel Blaster (Nintendo Only)"},{{3241,0x0000},"Buzz Wing"},
    {{3400,0x0000},"Fiesta"},{{3400,0x4515},"Frightful Fiesta"},{{3401,0x0000},"High Volt"},{{3402,0x0000},"Splat"},{{3402,0x4502},"Power Blue Splat"},{{3406,0x0000},"Stormblade"},
    {{3411,0x0000},"Smash Hit"},{{3411,0x4502},"Steel Plated Smash Hit"},{{3412,0x0000},"Spitfire"},{{3412,0x4502},"Dark Spitfire"},
    {{3413,0x0000},"Hurricane Jet Vac"},{{3413,0x4503},"Legendary Hurricane Jet Vac"},
    {{3414,0x0000},"Double Dare Trigger Happy"},{{3414,0x4502},"Power Blue Double Dare Trigger Happy"},
    {{3415,0x0000},"Super Shot Stealth Elf"},{{3415,0x4502},"Dark Super Shot Stealth Elf"},{{3416,0x0000},"Shark Shooter Terrafin"},
    {{3417,0x0000},"Bone Bash Roller Brawl"},{{3417,0x4503},"Legendary Bone Bash Roller Brawl"},
    {{3420,0x0000},"Big Bubble Pop Fizz"},{{3420,0x450E},"Birthday Bash Big Bubble Pop Fizz"},
    {{3421,0x0000},"Lava Lance Eruptor"},{{3422,0x0000},"Deep Dive Gill Grunt"},
    {{3423,0x0000},"Turbo Charge Donkey Kong (Nintendo Only)"},{{3423,0x4502},"Dark Turbo Charge Donkey Kong (Nintendo Only)"},
    {{3424,0x0000},"Hammer Slam Bowser (Nintendo Only)"},{{3424,0x4502},"Dark Hammer Slam Bowser (Nintendo Only)"},
    {{3425,0x0000},"Dive-Clops"},{{3425,0x450E},"Missile-Tow Dive-Clops"},{{3426,0x0000},"Astroblast"},{{3426,0x4503},"Legendary Astroblast"},
    {{3427,0x0000},"Nightfall"},{{3428,0x0000},"Thrillipede"},{{3428,0x450D},"Eggcited Thrillipede"},
    {{3500,0x0000},"Sky Trophy"},{{3501,0x0000},"Land Trophy"},{{3502,0x0000},"Sea Trophy"},{{3503,0x0000},"Kaos Trophy"},
};

static const char* LookupName(uint16_t id, uint16_t var) {
    auto it = s_names.find({id,var});
    if (it != s_names.end()) return it->second;
    auto it2 = s_names.find({id,0x0000});
    return it2 != s_names.end() ? it2->second : nullptr;
}

// ================================================================
//  Enums + correct in-game element colors
// ================================================================
enum class SkyElem : uint8_t {
    Unknown=0,Air,Dark,Earth,Fire,Life,Light,Magic,Tech,Undead,Water,
    Kaos,   // Unique element — only Kaos (Imaginators sensei) uses this
    _Count
};
enum class SkyCat : uint8_t {
    Figure=0,Giant,SwapForce,TrapMaster,SuperCharger,Sensei,Mini,Trap,Vehicle,Item,AdventurePack,Trophy,_Count
};
enum class SkyGame : uint8_t {
    SSA=0,Giants,SwapForce,TrapTeam,SuperChargers,Imaginators,_Count
};

static const char* kElemName[(int)SkyElem::_Count] = {
    "?","Air","Dark","Earth","Fire","Life","Light","Magic","Tech","Undead","Water","Kaos"
};
static const char* kCatName[(int)SkyCat::_Count] = {
    "Figure","Giant","Swap Force","Trap Master","SuperCharger","Sensei","Mini","Trap Crystal","Vehicle","Magic Item","Adv. Pack","Trophy"
};
static const char* kGameName[(int)SkyGame::_Count] = {
    "Spyro's Adventure","Giants","Swap Force","Trap Team","SuperChargers","Imaginators"
};

// Correct Skylanders element colors
static const ImVec4 kElemCol[(int)SkyElem::_Count] = {
    {0.40f,0.40f,0.40f,1}, // Unknown  — mid gray
    {0.40f,0.78f,1.00f,1}, // Air      — sky blue
    {0.10f,0.06f,0.14f,1}, // Dark     — near black
    {0.62f,0.40f,0.14f,1}, // Earth    — brown
    {1.00f,0.22f,0.04f,1}, // Fire     — red
    {0.15f,0.75f,0.15f,1}, // Life     — green
    {1.00f,0.90f,0.20f,1}, // Light    — gold
    {0.76f,0.18f,0.68f,1}, // Magic    — pink-purple
    {1.00f,0.55f,0.02f,1}, // Tech     — orange
    {0.55f,0.55f,0.58f,1}, // Undead   — gray/silver
    {0.12f,0.45f,1.00f,1}, // Water    — deep blue
    {0.72f,0.08f,0.82f,1}, // Kaos     — vivid purple (unique to Kaos sensei)
};
static const ImVec4 kGameCol[(int)SkyGame::_Count] = {
    {0.48f,0.72f,0.28f,1},{0.68f,0.48f,0.16f,1},{0.28f,0.58f,0.88f,1},
    {0.78f,0.20f,0.20f,1},{0.82f,0.62f,0.06f,1},{0.52f,0.20f,0.78f,1},
};

// ================================================================
//  Metadata
// ================================================================
static SkyCat CatFromId(uint16_t id) {
    if (id>=210&&id<=220)   return SkyCat::Trap;
    if (id>=200&&id<=233)   return SkyCat::Item;
    if (id>=300&&id<=308)   return SkyCat::AdventurePack;
    if (id>=3300&&id<=3303) return SkyCat::AdventurePack;
    // Trap Masters: 2 per element (first pair in each group of 4).
    // Light (482-483) and Dark (484-485) are ALL Trap Masters.
    // IDs 452-453,456-457,460-461,464-465,468-469,472-473,476-477,480-481 are regular Trap Team figures.
    switch (id) {
        case 450: case 451: // Air:    Gusto, Thunderbolt
        case 454: case 455: // Earth:  Wallop, Head Rush
        case 458: case 459: // Fire:   Wildfire, Ka-Boom
        case 462: case 463: // Water:  Snap Shot, Lob-Star
        case 466: case 467: // Magic:  Blastermind, Enigma
        case 470: case 471: // Tech:   Jawbreaker, Gearshift
        case 474: case 475: // Life:   Bushwack, Tuff Luck
        case 478: case 479: // Undead: Krypt King, Short Cut
        case 482: case 483: // Light:  Knight Light, Spotlight
        case 484: case 485: // Dark:   Knight Mare, Blackout
            return SkyCat::TrapMaster;
    }
    if (id>=450&&id<=481)   return SkyCat::Figure;  // remaining Trap Team figures
    if (id>=500&&id<=599)   return SkyCat::Mini;
    if (id>=600&&id<=699)   return SkyCat::Sensei;  // Imaginators Senseis
    if (id>=1000&&id<=2015) return SkyCat::SwapForce;
    if (id>=3200&&id<=3204) return SkyCat::Item;
    if (id>=3220&&id<=3241) return SkyCat::Vehicle;
    if (id>=3400&&id<=3428) return SkyCat::SuperCharger;
    if (id>=3500&&id<=3503) return SkyCat::Trophy;
    return SkyCat::Figure;
}
static bool IsGiant(uint16_t id) {
    // The 8 Giants: Swarm, Crusher, Thumpback, Ninjini,
    //               Bouncer, Tree Rex, Eye-Brawl, Hot Head
    switch(id){
        case 101: // Swarm       (Air)
        case 102: // Crusher     (Earth)
        case 104: // Hot Head    (Fire) ← often missed
        case 107: // Thumpback   (Water)
        case 109: // Ninjini     (Magic)
        case 110: // Bouncer     (Tech)
        case 112: // Tree Rex    (Life)
        case 114: // Eye-Brawl   (Undead)
            return true;
    }
    return false;
}
static SkyGame GameFromId(uint16_t id) {
    // ── Spyro's Adventure ─────────────────────────────────────
    // Figures 0-32, magic items 200-209, adventure packs 300-304
    // (Dragon's Peak, Empire of Ice, Pirate Seas, Darklight Crypt,
    //  Volcanic Vault — exclusive battle arena for SSA),
    // legendary variants 404/416/419/430
    if (id<=32
     ||(id>=200&&id<=209)
     ||(id>=300&&id<=304)
     ||(id>=404&&id<=430))
        return SkyGame::SSA;

    // ── Giants ────────────────────────────────────────────────
    // Figures/giants 100-115, adventure-pack items 230-233
    // (Hand of Fate, Piggy Bank, Rocket Ram, Tiki Speaky)
    if ((id>=100&&id<=115)
     ||(id>=230&&id<=233))
        return SkyGame::Giants;

    // ── Swap Force ────────────────────────────────────────────
    // Swap-halves 1000-2015, SF figures 3000-3015,
    // SF magic items 3200-3204,
    // SF adventure packs 3300-3303
    // (Sheep Wreck Island, Tower of Time, Fiery Forge, Arkeyan Crossbow)
    if ((id>=1000&&id<=2015)
     ||(id>=3000&&id<=3015)
     ||(id>=3200&&id<=3204)
     ||(id>=3300&&id<=3303))
        return SkyGame::SwapForce;

    // ── Trap Team ─────────────────────────────────────────────
    // Trap Masters 450-485, trap crystals 210-220,
    // minis 500-599,
    // Trap Team adventure packs 305-308
    // (Mirror of Mystery, Nightmare Express,
    //  Sunscraper Spire, Midnight Museum)
    if ((id>=450&&id<=485)
     ||(id>=210&&id<=220)
     ||(id>=500&&id<=599)
     ||(id>=305&&id<=308))
        return SkyGame::TrapTeam;

    // ── SuperChargers ─────────────────────────────────────────
    // Vehicles 3220-3241, SC drivers 3400-3428, trophies 3500-3503
    if ((id>=3220&&id<=3241)
     ||(id>=3400&&id<=3428)
     ||(id>=3500&&id<=3503))
        return SkyGame::SuperChargers;

    // ── Imaginators ───────────────────────────────────────────
    if (id>=600&&id<=699)
        return SkyGame::Imaginators;

    return SkyGame::SSA;
}
static const std::unordered_map<uint16_t,SkyElem> s_elem = {
    {0,SkyElem::Air},{1,SkyElem::Air},{2,SkyElem::Air},{3,SkyElem::Air},
    {4,SkyElem::Earth},{5,SkyElem::Earth},{6,SkyElem::Earth},{7,SkyElem::Earth},
    {8,SkyElem::Fire},{9,SkyElem::Fire},{10,SkyElem::Fire},{11,SkyElem::Fire},
    {12,SkyElem::Water},{13,SkyElem::Water},{14,SkyElem::Water},{15,SkyElem::Water},
    {16,SkyElem::Magic},{17,SkyElem::Magic},{18,SkyElem::Magic},{23,SkyElem::Magic},{28,SkyElem::Magic},
    {19,SkyElem::Tech},{20,SkyElem::Tech},{21,SkyElem::Tech},{22,SkyElem::Tech},
    {24,SkyElem::Life},{25,SkyElem::Life},{26,SkyElem::Life},{27,SkyElem::Life},
    {29,SkyElem::Undead},{30,SkyElem::Undead},{31,SkyElem::Undead},{32,SkyElem::Undead},
    {404,SkyElem::Earth},{416,SkyElem::Magic},{419,SkyElem::Tech},{430,SkyElem::Undead},
    {100,SkyElem::Air},{101,SkyElem::Air},{102,SkyElem::Earth},{103,SkyElem::Earth},
    {104,SkyElem::Fire},{105,SkyElem::Fire},{106,SkyElem::Water},{107,SkyElem::Water},
    {108,SkyElem::Magic},{109,SkyElem::Magic},{110,SkyElem::Tech},{111,SkyElem::Tech},
    {112,SkyElem::Life},{113,SkyElem::Life},{114,SkyElem::Undead},{115,SkyElem::Undead},
    {200,SkyElem::Unknown},{201,SkyElem::Unknown},{202,SkyElem::Unknown},{203,SkyElem::Unknown},
    {204,SkyElem::Unknown},{205,SkyElem::Unknown},{206,SkyElem::Unknown},{207,SkyElem::Unknown},{208,SkyElem::Unknown},{209,SkyElem::Unknown},
    {210,SkyElem::Magic},{211,SkyElem::Water},{212,SkyElem::Air},{213,SkyElem::Undead},
    {214,SkyElem::Tech},{215,SkyElem::Fire},{216,SkyElem::Earth},{217,SkyElem::Life},{218,SkyElem::Dark},{219,SkyElem::Light},{220,SkyElem::Unknown},
    {230,SkyElem::Unknown},{231,SkyElem::Unknown},{232,SkyElem::Unknown},{233,SkyElem::Unknown},
    {300,SkyElem::Unknown},{301,SkyElem::Unknown},{302,SkyElem::Unknown},{303,SkyElem::Unknown},
    {304,SkyElem::Unknown},{305,SkyElem::Unknown},{306,SkyElem::Unknown},{307,SkyElem::Unknown},{308,SkyElem::Unknown},
    {450,SkyElem::Air},{451,SkyElem::Air},{452,SkyElem::Air},{453,SkyElem::Air},
    {454,SkyElem::Earth},{455,SkyElem::Earth},{456,SkyElem::Earth},{457,SkyElem::Earth},
    {458,SkyElem::Fire},{459,SkyElem::Fire},{460,SkyElem::Fire},{461,SkyElem::Fire},
    {462,SkyElem::Water},{463,SkyElem::Water},{464,SkyElem::Water},{465,SkyElem::Water},
    {466,SkyElem::Magic},{467,SkyElem::Magic},{468,SkyElem::Magic},{469,SkyElem::Magic},
    {470,SkyElem::Tech},{471,SkyElem::Tech},{472,SkyElem::Tech},{473,SkyElem::Tech},
    {474,SkyElem::Life},{475,SkyElem::Life},{476,SkyElem::Life},{477,SkyElem::Life},
    {478,SkyElem::Undead},{479,SkyElem::Undead},{480,SkyElem::Undead},{481,SkyElem::Undead},
    {482,SkyElem::Light},{483,SkyElem::Light},{484,SkyElem::Dark},{485,SkyElem::Dark},
    {502,SkyElem::Earth},{503,SkyElem::Magic},{504,SkyElem::Undead},{505,SkyElem::Earth},
    {506,SkyElem::Air},{507,SkyElem::Fire},{508,SkyElem::Air},{509,SkyElem::Fire},
    {510,SkyElem::Tech},{514,SkyElem::Water},{519,SkyElem::Tech},{526,SkyElem::Life},
    {540,SkyElem::Life},{541,SkyElem::Water},{542,SkyElem::Magic},{543,SkyElem::Undead},
    {601,SkyElem::Water},{602,SkyElem::Earth},{603,SkyElem::Life},{604,SkyElem::Life},
    {605,SkyElem::Earth},{606,SkyElem::Tech},{607,SkyElem::Air},{608,SkyElem::Fire},
    {609,SkyElem::Life},{610,SkyElem::Tech},{611,SkyElem::Undead},{612,SkyElem::Air},
    {613,SkyElem::Earth},{614,SkyElem::Undead},{615,SkyElem::Magic},{616,SkyElem::Magic},
    {617,SkyElem::Air},{618,SkyElem::Undead},{619,SkyElem::Light},{620,SkyElem::Fire},
    {621,SkyElem::Magic},{622,SkyElem::Dark},{623,SkyElem::Earth},{624,SkyElem::Tech},
    {625,SkyElem::Magic},{626,SkyElem::Tech},
    {627,SkyElem::Kaos},   // Kaos          — unique Kaos element
    {628,SkyElem::Air},
    {629,SkyElem::Water},
    {630,SkyElem::Earth},  // Crash Bandicoot — Earth sensei
    {631,SkyElem::Tech},   // Dr. Neo Cortex  — Tech sensei
    {1000,SkyElem::Air},{1001,SkyElem::Air},{1002,SkyElem::Earth},{1003,SkyElem::Earth},
    {1004,SkyElem::Fire},{1005,SkyElem::Fire},{1006,SkyElem::Life},{1007,SkyElem::Life},
    {1008,SkyElem::Magic},{1009,SkyElem::Magic},{1010,SkyElem::Tech},{1011,SkyElem::Tech},
    {1012,SkyElem::Undead},{1013,SkyElem::Undead},{1014,SkyElem::Water},{1015,SkyElem::Water},
    {2000,SkyElem::Air},{2001,SkyElem::Air},{2002,SkyElem::Earth},{2003,SkyElem::Earth},
    {2004,SkyElem::Fire},{2005,SkyElem::Fire},{2006,SkyElem::Life},{2007,SkyElem::Life},
    {2008,SkyElem::Magic},{2009,SkyElem::Magic},{2010,SkyElem::Tech},{2011,SkyElem::Tech},
    {2012,SkyElem::Undead},{2013,SkyElem::Undead},{2014,SkyElem::Water},{2015,SkyElem::Water},
    {3000,SkyElem::Air},{3001,SkyElem::Air},{3002,SkyElem::Earth},{3003,SkyElem::Earth},
    {3004,SkyElem::Fire},{3005,SkyElem::Fire},{3006,SkyElem::Life},{3007,SkyElem::Life},
    {3008,SkyElem::Earth},{3009,SkyElem::Magic},{3010,SkyElem::Tech},{3011,SkyElem::Tech}, // 3008=Dune Bug (Earth)
    {3012,SkyElem::Undead},{3013,SkyElem::Undead},{3014,SkyElem::Water},{3015,SkyElem::Water},
    {3200,SkyElem::Unknown},{3201,SkyElem::Unknown},{3202,SkyElem::Unknown},{3203,SkyElem::Unknown},{3204,SkyElem::Unknown},
    {3300,SkyElem::Unknown},{3301,SkyElem::Unknown},{3302,SkyElem::Unknown},{3303,SkyElem::Unknown},
    {3220,SkyElem::Air},{3221,SkyElem::Undead},{3222,SkyElem::Water},{3223,SkyElem::Fire},
    {3224,SkyElem::Fire},{3225,SkyElem::Earth},{3226,SkyElem::Earth},{3227,SkyElem::Undead},
    {3228,SkyElem::Life},{3231,SkyElem::Air},{3232,SkyElem::Air},{3233,SkyElem::Magic},
    {3234,SkyElem::Tech},{3235,SkyElem::Tech},{3236,SkyElem::Light},{3237,SkyElem::Dark},
    {3238,SkyElem::Water},{3239,SkyElem::Magic},{3240,SkyElem::Dark},{3241,SkyElem::Air},
    {3400,SkyElem::Undead},{3401,SkyElem::Tech},{3402,SkyElem::Magic},{3406,SkyElem::Air},
    {3411,SkyElem::Earth},{3412,SkyElem::Fire},{3413,SkyElem::Air},{3414,SkyElem::Tech},
    {3415,SkyElem::Life},{3416,SkyElem::Earth},{3417,SkyElem::Undead},{3420,SkyElem::Magic},
    {3421,SkyElem::Fire},{3422,SkyElem::Water},{3423,SkyElem::Earth},{3424,SkyElem::Fire},
    {3425,SkyElem::Water},{3426,SkyElem::Light},{3427,SkyElem::Dark},{3428,SkyElem::Life},
    {3500,SkyElem::Unknown},{3501,SkyElem::Unknown},{3502,SkyElem::Unknown},{3503,SkyElem::Unknown},
};
static SkyElem ElemOf(uint16_t id) {
    auto it=s_elem.find(id);
    return it!=s_elem.end()?it->second:SkyElem::Unknown;
}

// ================================================================
//  Data
// ================================================================
struct SlotState {
    bool active=false; uint8 portalSlot=0;
    uint16 skyId=0,skyVar=0;
    std::string name; SkyElem elem=SkyElem::Unknown;
};
static SlotState g_slots[nsyshid::MAX_SKYLANDERS];
static int g_selectedSlot = 0; // which slot is highlighted on the left panel

struct LibEntry {
    fs::path path; std::string name;
    SkyElem elem=SkyElem::Unknown; SkyCat cat=SkyCat::Figure; SkyGame game=SkyGame::SSA;
};
static std::vector<LibEntry> g_lib;
static bool g_libReady=false;
static char g_folder[512]={};

// Filters — 0 = no restriction
static uint32_t g_fGame=0, g_fCat=0, g_fElem=0;

static bool PassFilter(const LibEntry& e) {
    if (g_fGame && !(g_fGame&(1u<<(int)e.game))) return false;
    if (g_fCat  && !(g_fCat &(1u<<(int)e.cat ))) return false;
    if (g_fElem && !(g_fElem&(1u<<(int)e.elem))) return false;
    return true;
}
static bool AnyFilter(){return g_fGame||g_fCat||g_fElem;}

// ================================================================
//  Portal helpers
// ================================================================
static bool PortalOn(){return GetConfig().emulated_usb_devices.emulate_skylander_portal;}
static void EnablePortal(){
    GetConfig().emulated_usb_devices.emulate_skylander_portal=true;
    GetConfigHandle().Save();
}

// ================================================================
//  Async state
// ================================================================
// Wheel delta forwarded from the Win32 message handler (UI thread → render thread).
static std::mutex  g_wheelMtx;
static float       g_pendingWheel = 0.f;

void SkylanderOverlay_addWheelDelta(float delta) {
    std::lock_guard lk(g_wheelMtx);
    g_pendingWheel += delta;
}

void SkylanderOverlay_flushWheelDelta() {
    std::lock_guard lk(g_wheelMtx);
    if (g_pendingWheel != 0.f) {
        ImGui::GetIO().MouseWheel += g_pendingWheel;
        g_pendingWheel = 0.f;
    }
}

static bool g_visible=false,g_f8Prev=false;
static std::atomic<bool> g_fpRun{false},g_fpDone{false};
static int g_fpSlot=-1; static std::wstring g_fpPath; static std::mutex g_fpMtx;
static std::atomic<bool> g_fdRun{false},g_fdDone{false};
static std::wstring g_fdRes; static std::mutex g_fdMtx;

// ================================================================
//  String utils
// ================================================================
static std::string W2U(const std::wstring& w){
#ifdef _WIN32
    if(w.empty())return{};
    int n=WideCharToMultiByte(CP_UTF8,0,w.c_str(),-1,nullptr,0,nullptr,nullptr);
    if(n<=0)return{};std::string o(n-1,'\0');
    WideCharToMultiByte(CP_UTF8,0,w.c_str(),-1,o.data(),n,nullptr,nullptr);return o;
#else
    return{w.begin(),w.end()};
#endif
}
#ifdef _WIN32
static std::wstring U2W(const std::string& s){
    if(s.empty())return{};
    int n=MultiByteToWideChar(CP_UTF8,0,s.c_str(),-1,nullptr,0);
    if(n<=0)return{};std::wstring o(n-1,L'\0');
    MultiByteToWideChar(CP_UTF8,0,s.c_str(),-1,o.data(),n);return o;
}
#endif

// ================================================================
//  Pickers
// ================================================================
#ifdef _WIN32
static void OpenFilePicker(int slot){
    if(g_fpRun.exchange(true))return;
    g_fpSlot=slot;g_fpDone=false;
    std::thread([](){
        wchar_t buf[4096]={};OPENFILENAMEW o={};o.lStructSize=sizeof(o);
        o.lpstrFile=buf;o.nMaxFile=(DWORD)std::size(buf);
        o.lpstrFilter=L"Skylander files\0*.sky;*.bin;*.dump;*.dmp\0All files\0*.*\0";
        o.lpstrTitle=L"Open Skylander File";o.Flags=OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST|OFN_NOCHANGEDIR;
        bool ok=GetOpenFileNameW(&o);
        {std::lock_guard lk(g_fpMtx);g_fpPath=ok?buf:L"";}
        g_fpDone=true;g_fpRun=false;
    }).detach();
}
static void OpenFolderPicker(){
    if(g_fdRun.exchange(true))return;g_fdDone=false;
    std::thread([](){
        std::wstring res;HRESULT hr=CoInitializeEx(nullptr,COINIT_APARTMENTTHREADED);
        IFileOpenDialog* d=nullptr;
        if(SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog,nullptr,CLSCTX_INPROC_SERVER,IID_PPV_ARGS(&d)))){
            DWORD opts=0;d->GetOptions(&opts);d->SetOptions(opts|FOS_PICKFOLDERS|FOS_PATHMUSTEXIST);
            d->SetTitle(L"Select Skylander Library Folder");
            if(SUCCEEDED(d->Show(nullptr))){IShellItem*it=nullptr;
                if(SUCCEEDED(d->GetResult(&it))){PWSTR p=nullptr;
                    if(SUCCEEDED(it->GetDisplayName(SIGDN_FILESYSPATH,&p))){res=p;CoTaskMemFree(p);}
                    it->Release();}
            }d->Release();
        }if(SUCCEEDED(hr))CoUninitialize();
        {std::lock_guard lk(g_fdMtx);g_fdRes=res;}g_fdDone=true;g_fdRun=false;
    }).detach();
}
#else
static void OpenFilePicker(int){}
static void OpenFolderPicker(){}
#endif

// ================================================================
//  Portal slot ops
// ================================================================
static void LoadSlot(int ui, const fs::path& path){
    if(g_slots[ui].active){nsyshid::g_skyportal.RemoveSkylander(g_slots[ui].portalSlot);g_slots[ui]={};}
    std::unique_ptr<FileStream> f(FileStream::openFile2(path,true));if(!f)return;
    std::array<uint8,nsyshid::SKY_FIGURE_SIZE> buf{};
    if(f->readData(buf.data(),buf.size())!=buf.size())return;f->SetPosition(0);
    uint16 id=uint16(buf[0x11])<<8|uint16(buf[0x10]);
    uint16 var=uint16(buf[0x1D])<<8|uint16(buf[0x1C]);
    uint8 ps=nsyshid::g_skyportal.LoadSkylander(buf.data(),std::move(f));
    const char* nm=LookupName(id,var);
    g_slots[ui]={true,ps,id,var,nm?nm:"Unknown",ElemOf(id)};
}
static void ClearSlot(int ui){
    if(!g_slots[ui].active)return;
    nsyshid::g_skyportal.RemoveSkylander(g_slots[ui].portalSlot);g_slots[ui]={};
}
static int FirstFreeSlot(){
    for(int i=0;i<nsyshid::MAX_SKYLANDERS;i++)if(!g_slots[i].active)return i;
    return -1;
}

// ================================================================
//  Library scan
// ================================================================
static void DoScan(const std::string& utf8){
    g_lib.clear();
    if(utf8.empty())return;
    fs::path root;
#ifdef _WIN32
    root=U2W(utf8);
#else
    root=utf8;
#endif
    try{
        for(auto& de:fs::recursive_directory_iterator(root,fs::directory_options::skip_permission_denied)){
            if(!de.is_regular_file())continue;
            auto ext=de.path().extension().string();
            for(auto& c:ext)c=(char)std::tolower((uint8_t)c);
            if(ext!=".sky"&&ext!=".bin"&&ext!=".dump"&&ext!=".dmp")continue;
            uint16_t id=0,var=0;
            {uint8_t h[30]={};
#ifdef _WIN32
            FILE*fp=nullptr;_wfopen_s(&fp,de.path().c_str(),L"rb");
#else
            FILE*fp=std::fopen(de.path().c_str(),"rb");
#endif
            if(fp){if(std::fread(h,1,sizeof(h),fp)==sizeof(h)){
                id=uint16_t(h[0x11])<<8|uint16_t(h[0x10]);
                var=uint16_t(h[0x1D])<<8|uint16_t(h[0x1C]);}std::fclose(fp);}}
            const char* nm=LookupName(id,var);if(!nm)continue;
            SkyCat cat=CatFromId(id);if(cat==SkyCat::Figure&&IsGiant(id))cat=SkyCat::Giant;
            g_lib.push_back({de.path(),nm,ElemOf(id),cat,GameFromId(id)});
        }
    }catch(...){}
    std::sort(g_lib.begin(),g_lib.end(),[](const LibEntry&a,const LibEntry&b){
        if(a.game!=b.game)return(int)a.game<(int)b.game;
        if(a.cat!=b.cat)return(int)a.cat<(int)b.cat;
        if(a.elem!=b.elem)return(int)a.elem<(int)b.elem;
        return a.name<b.name;});
    g_libReady=true;
}
static void SaveScan(const std::string& u){
    GetConfig().emulated_usb_devices.skylander_library_path=u;
    GetConfigHandle().Save();DoScan(u);
}

// ================================================================
//  Draw helpers
// ================================================================

// Draw element dot via draw list — font-independent, always works.
// Returns width consumed so caller can SameLine after.

// Wrapping filter chip row.
// idBase: unique base ID per section (avoids ImGui hash collisions).
// bitOffset: first chip toggles bit (bitOffset), second toggles (bitOffset+1), etc.
static void FilterRow(const char** labels, int count, const ImVec4* colors,
                      uint32_t* mask, int idBase, int bitOffset=0)
{
    float maxX = ImGui::GetWindowPos().x + ImGui::GetContentRegionMax().x;

    for (int i=0;i<count;i++) {
        int bit = bitOffset + i;
        bool on = (*mask>>bit)&1;
        ImVec4 ac = colors ? colors[i] : ImVec4(0.20f,0.40f,0.75f,1);
        ImVec4 bg  = on ? ImVec4(ac.x*0.7f,ac.y*0.7f,ac.z*0.7f,1) : ImVec4(0.12f,0.13f,0.20f,1);
        ImVec4 hov = on ? ac : ImVec4(0.18f,0.20f,0.30f,1);
        ImVec4 txt = on ? ImVec4(1,1,1,1) : ImVec4(0.65f,0.68f,0.72f,1);
        float bw = ImGui::CalcTextSize(labels[i]).x + ImGui::GetStyle().FramePadding.x*2 + 6;

        if (i>0) {
            float nextX = ImGui::GetCursorScreenPos().x + 4 + bw;
            if (nextX < maxX) ImGui::SameLine(0,4);
        }

        ImGui::PushID(idBase+i);
        ImGui::PushStyleColor(ImGuiCol_Button,       bg);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,hov);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ac);
        ImGui::PushStyleColor(ImGuiCol_Text,         txt);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding,6);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,{5,2});
        if (ImGui::Button(labels[i],{bw,0})) *mask ^= (1u<<bit);
        ImGui::PopStyleVar(2);ImGui::PopStyleColor(4);
        ImGui::PopID();
    }
}

// ================================================================
//  Render
// ================================================================

void SkylanderOverlay_init(){}

// ----------------------------------------------------------------
// Helper: draw a filled coloured circle at the current cursor
// position, then advance the cursor past it (width = diameter+gap).
// Must be called before any item on the same line.
// ----------------------------------------------------------------
static void ColorDot(ImVec4 col, float radius = 0.f)
{
    if (radius <= 0.f) radius = ImGui::GetTextLineHeight() * 0.35f;
    ImVec2 p  = ImGui::GetCursorScreenPos();
    float  lh = ImGui::GetTextLineHeight();
    ImGui::GetWindowDrawList()->AddCircleFilled(
        { p.x + radius, p.y + lh * 0.5f },
        radius,
        ImGui::ColorConvertFloat4ToU32(col),
        12);
    // Reserve the dot's width so the next item doesn't overlap it
    ImGui::Dummy({ radius * 2.f + 4.f, lh });
    ImGui::SameLine(0, 0);
}

void SkylanderOverlay_render(bool isPadView)
{
    if (isPadView) return;

    // ── Consume async picker results ──────────────────────────
    if (g_fpDone.exchange(false)) {
        std::wstring p; { std::lock_guard lk(g_fpMtx); p = g_fpPath; }
        if (!p.empty() && g_fpSlot >= 0) LoadSlot(g_fpSlot, fs::path(p));
    }
    if (g_fdDone.exchange(false)) {
        std::wstring r; { std::lock_guard lk(g_fdMtx); r = g_fdRes; }
        if (!r.empty()) {
            std::string u = W2U(r);
            std::strncpy(g_folder, u.c_str(), sizeof(g_folder) - 1);
            SaveScan(u);
        }
    }

    // F8
#ifdef _WIN32
    bool f8=(GetAsyncKeyState(VK_F8)&0x8000)!=0;
    if(f8&&!g_f8Prev)g_visible=!g_visible;
    g_f8Prev=f8;
#else
    if(ImGui::IsKeyPressed(ImGuiKey_F8,false))g_visible=!g_visible;
#endif

    const ImGuiIO& io = ImGui::GetIO();
    bool enabled      = PortalOn();

    // ============================================================
    //  BADGE  (top-right corner, always visible)
    //  Two-window trick: display window has NoInputs so the game
    //  keeps its mouse focus; a transparent overlay on top catches
    //  clicks and calls InvisibleButton.
    // ============================================================
    {
        const float BW  = 84.f;
        const float PAD = 6.f;
        ImVec4 dotC = enabled
            ? ImVec4(0.10f,0.88f,0.10f,1.f)
            : ImVec4(1.00f,0.44f,0.00f,1.f);

        // ── Display window (NoInputs = never steals game mouse) ──
        ImGui::SetNextWindowPos({ io.DisplaySize.x - BW - PAD, PAD }, ImGuiCond_Always);
        ImGui::SetNextWindowSize({ BW, 0 }, ImGuiCond_Always);
        ImGui::SetNextWindowBgAlpha(0.88f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,  { 6.f, 5.f });
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 5.f);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,    { 3.f, 3.f });
        ImGui::Begin("##badge_vis", nullptr,
            ImGuiWindowFlags_NoMove            |
            ImGuiWindowFlags_NoDecoration      |
            ImGuiWindowFlags_AlwaysAutoResize  |
            ImGuiWindowFlags_NoSavedSettings   |
            ImGuiWindowFlags_NoFocusOnAppearing|
            ImGuiWindowFlags_NoNav             |
            ImGuiWindowFlags_NoInputs);         // <-- key: game keeps mouse focus

        ColorDot(dotC);                         // draws circle, advances cursor
        ImGui::TextUnformatted("Portal");
        ImGui::PushStyleColor(ImGuiCol_Text, { 0.50f,0.55f,0.65f,1.f });
        ImGui::TextUnformatted(g_visible ? "F8 · click close" : "F8 · click open");
        ImGui::PopStyleColor();

        float badgeH = ImGui::GetWindowHeight(); // measure before End()
        ImGui::End();
        ImGui::PopStyleVar(3);

        // ── Transparent hit-test overlay (receives clicks) ────────
        ImGui::SetNextWindowPos({ io.DisplaySize.x - BW - PAD, PAD }, ImGuiCond_Always);
        ImGui::SetNextWindowSize({ BW, badgeH }, ImGuiCond_Always);
        ImGui::SetNextWindowBgAlpha(0.f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.f, 0.f });
        ImGui::Begin("##badge_hit", nullptr,
            ImGuiWindowFlags_NoMove            |
            ImGuiWindowFlags_NoDecoration      |
            ImGuiWindowFlags_NoSavedSettings   |
            ImGuiWindowFlags_NoFocusOnAppearing|
            ImGuiWindowFlags_NoNav             |
            ImGuiWindowFlags_NoScrollbar);
        if (ImGui::InvisibleButton("##badgebtn", { BW, badgeH }))
            g_visible = !g_visible;
        ImGui::End();
        ImGui::PopStyleVar();
    }

    if (!g_visible) return;

    // ============================================================
    //  MAIN WINDOW
    // ============================================================
    int activeCnt = 0;
    for (int i = 0; i < nsyshid::MAX_SKYLANDERS; i++)
        if (g_slots[i].active) activeCnt++;

    // Initialise library folder from saved config (once)
    {
        static bool s_init = false;
        if (!s_init) {
            s_init = true;
            const std::string& sv =
                GetConfig().emulated_usb_devices.skylander_library_path.GetValue();
            if (!sv.empty()) {
                std::strncpy(g_folder, sv.c_str(), sizeof(g_folder) - 1);
                if (!g_libReady) DoScan(sv);
            }
        }
    }

    ImGui::SetNextWindowSize({ 860.f, 620.f }, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(
        { io.DisplaySize.x * 0.5f - 430.f, io.DisplaySize.y * 0.5f - 310.f },
        ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSizeConstraints({ 580.f, 480.f }, { 1600.f, 1080.f });
    ImGui::SetNextWindowBgAlpha(0.97f);

    // Style overrides for the main window
    ImGui::PushStyleColor(ImGuiCol_TitleBg,       { 0.03f,0.05f,0.18f,1.f });
    ImGui::PushStyleColor(ImGuiCol_TitleBgActive,  { 0.04f,0.11f,0.40f,1.f });
    ImGui::PushStyleColor(ImGuiCol_WindowBg,       { 0.03f,0.04f,0.12f,0.98f });
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 6.f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding,  4.f);

    bool windowOpen = true;
    bool collapsed  = !ImGui::Begin("Skylanders Portal Manager  [F8]",
                                    &windowOpen, ImGuiWindowFlags_NoCollapse);
    if (collapsed) {
        ImGui::End();
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(3);
        if (!windowOpen) g_visible = false;
        return;
    }

    // ── Status bar ───────────────────────────────────────────────
    ColorDot(enabled
        ? ImVec4(0.10f,0.88f,0.10f,1.f)
        : ImVec4(1.00f,0.44f,0.00f,1.f));

    if (enabled) {
        ImGui::Text("Portal active  —  %d / %d slots filled",
                    activeCnt, nsyshid::MAX_SKYLANDERS);
    } else {
        ImGui::TextColored({ 1.f,0.46f,0.f,1.f }, "Portal disabled");
        ImGui::SameLine(0, 8);
        ImGui::PushStyleColor(ImGuiCol_Button,        { 0.10f,0.32f,0.70f,1.f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.16f,0.44f,0.86f,1.f });
        if (ImGui::SmallButton(" Enable ")) EnablePortal();
        ImGui::PopStyleColor(2);
        ImGui::SameLine(0, 6);
        ImGui::TextDisabled("(restart game to take effect)");
    }

    ImGui::Separator();

    // ── Library folder bar ───────────────────────────────────────
    {
        float rowW = ImGui::GetContentRegionAvail().x;
        ImGui::SetNextItemWidth(rowW - 128.f);
        if (ImGui::InputText("##fldr", g_folder, sizeof(g_folder),
                             ImGuiInputTextFlags_EnterReturnsTrue))
            SaveScan(g_folder);

        ImGui::SameLine(0, 4);
        bool busy = (bool)g_fdRun;
        if (busy) ImGui::BeginDisabled();
        if (ImGui::Button("Browse", { 60.f, 0.f })) OpenFolderPicker();
        if (busy) ImGui::EndDisabled();

        ImGui::SameLine(0, 4);
        if (ImGui::Button("Scan", { 52.f, 0.f })) SaveScan(g_folder);
    }

    // ── Filter chips ─────────────────────────────────────────────
    // Each section uses a unique PushID range so identically-named
    // buttons (e.g. "Swap Force" appears in both Game and Type) don't
    // hash-collide in ImGui's ID stack.
    ImGui::Spacing();

    // Game row
    ImGui::PushStyleColor(ImGuiCol_Text, { 0.45f,0.57f,0.75f,1.f });
    ImGui::TextUnformatted("Game"); ImGui::PopStyleColor();
    ImGui::SameLine(0, 6);
    FilterRow(kGameName, (int)SkyGame::_Count, kGameCol, &g_fGame, /*idBase=*/100);

    // Type row
    ImGui::PushStyleColor(ImGuiCol_Text, { 0.45f,0.57f,0.75f,1.f });
    ImGui::TextUnformatted("Type"); ImGui::PopStyleColor();
    ImGui::SameLine(0, 6);
    {
        ImVec4 catColors[(int)SkyCat::_Count];
        for (int c = 0; c < (int)SkyCat::_Count; c++)
            catColors[c] = { 0.20f,0.38f,0.72f,1.f };
        FilterRow(kCatName, (int)SkyCat::_Count, catColors, &g_fCat, /*idBase=*/200);
    }

    // Element row (bitOffset=1 because Unknown=0 is skipped)
    ImGui::PushStyleColor(ImGuiCol_Text, { 0.45f,0.57f,0.75f,1.f });
    ImGui::TextUnformatted("Elem"); ImGui::PopStyleColor();
    ImGui::SameLine(0, 6);
    FilterRow(kElemName + 1, (int)SkyElem::_Count - 1,
              kElemCol  + 1, &g_fElem, /*idBase=*/300, /*bitOffset=*/1);

    // Clear button (only shown when a filter is active)
    if (AnyFilter()) {
        ImGui::SameLine(0, 12);
        ImGui::PushStyleColor(ImGuiCol_Button,        { 0.38f,0.07f,0.07f,1.f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.56f,0.12f,0.12f,1.f });
        if (ImGui::Button("Clear##fclr")) { g_fGame = g_fCat = g_fElem = 0; }
        ImGui::PopStyleColor(2);
    }

    ImGui::Separator();
    ImGui::Spacing();

    // ── Measure remaining height ONCE before any BeginChild ──────
    // This is the critical step: both children receive the same
    // explicit positive height so ImGui knows how tall to make them
    // and scroll works correctly (ref: ocornut/imgui#150).
    const float panelH = ImGui::GetContentRegionAvail().y;
    const float slotW  = 238.f;                              // left panel width


    // ============================================================
    //  LEFT PANEL — Portal Slots
    // ============================================================
    ImGui::PushStyleColor(ImGuiCol_ChildBg,           { 0.04f,0.06f,0.17f,0.95f });
    ImGui::PushStyleColor(ImGuiCol_TableBorderStrong, { 0.16f,0.26f,0.50f,0.90f });
    ImGui::PushStyleColor(ImGuiCol_TableBorderLight,  { 0.10f,0.15f,0.34f,0.70f });
    ImGui::PushStyleColor(ImGuiCol_TableRowBgAlt,     { 0.05f,0.08f,0.18f,0.55f });
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.f);

    ImGui::BeginChild("##slots", ImVec2(slotW, panelH), /*border=*/true);

    ImGui::PushStyleColor(ImGuiCol_Text, { 0.45f,0.57f,0.75f,1.f });
    ImGui::TextUnformatted("PORTAL SLOTS");
    ImGui::PopStyleColor();
    ImGui::Separator();
    ImGui::Spacing();

    // 4-column table: index | figure | Load | Unload
    const float kLW = 42.f, kUW = 52.f;
    if (ImGui::BeginTable("##slottbl", 4,
            ImGuiTableFlags_BordersInnerV |
            ImGuiTableFlags_RowBg         |
            ImGuiTableFlags_SizingFixedFit))
    {
        ImGui::TableSetupColumn("#",        ImGuiTableColumnFlags_WidthFixed,   22.f);
        ImGui::TableSetupColumn("Figure",   ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("##ldbtn",  ImGuiTableColumnFlags_WidthFixed,   kLW);
        ImGui::TableSetupColumn("##ulbtn",  ImGuiTableColumnFlags_WidthFixed,   kUW);

        for (int i = 0; i < nsyshid::MAX_SKYLANDERS; i++) {
            auto& s    = g_slots[i];
            bool  isSel = (g_selectedSlot == i);

            ImGui::TableNextRow();

            // Tint the selected row blue via TableSetBgColor
            if (isSel)
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0,
                                       IM_COL32(24, 58, 148, 115));

            // ── Col 0: slot index ──
            ImGui::TableSetColumnIndex(0);
            ImGui::PushStyleColor(ImGuiCol_Text,
                isSel ? ImVec4(0.86f,0.93f,1.f,1.f)
                      : ImVec4(0.38f,0.42f,0.50f,1.f));
            ImGui::Text("%2d", i + 1);
            ImGui::PopStyleColor();

            // ── Col 1: element dot + figure name ──
            // ColorDot draws a circle and advances the cursor (Dummy+SameLine).
            // Selectable("Name##id") renders the name starting from that cursor.
            // The "##" ID separator is standard ImGui: text before it is
            // the visible label, text after it is the unique widget ID.
            ImGui::TableSetColumnIndex(1);
            if (s.active) {
                ColorDot(kElemCol[(int)s.elem]);
                char label[288];
                snprintf(label, sizeof(label), "%s##ss%d", s.name.c_str(), i);
                if (ImGui::Selectable(label, isSel,
                        ImGuiSelectableFlags_None, ImVec2(0.f, 0.f)))
                    g_selectedSlot = i;
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort)) {
                    ImGui::BeginTooltip();
                    ColorDot(kElemCol[(int)s.elem]);
                    ImGui::TextUnformatted(s.name.c_str());
                    ImGui::EndTooltip();
                }
            } else {
                char label[32];
                snprintf(label, sizeof(label), "empty##ss%d", i);
                ImGui::PushStyleColor(ImGuiCol_Text, { 0.24f,0.26f,0.33f,1.f });
                if (ImGui::Selectable(label, isSel,
                        ImGuiSelectableFlags_None, ImVec2(0.f, 0.f)))
                    g_selectedSlot = i;
                ImGui::PopStyleColor();
            }

            // ── Col 2: Load ──
            ImGui::TableSetColumnIndex(2);
            ImGui::PushID(i * 10 + 0);
            bool picking = g_fpRun && g_fpSlot == i;
            if (!enabled || picking) ImGui::BeginDisabled();
            if (picking)
                ImGui::Button("···", { kLW, 0.f });
            else if (ImGui::Button("Load", { kLW, 0.f }))
                OpenFilePicker(i);
            if (!enabled || picking) ImGui::EndDisabled();
            ImGui::PopID();

            // ── Col 3: Unload ──
            ImGui::TableSetColumnIndex(3);
            ImGui::PushID(i * 10 + 1);
            if (!s.active || !enabled) {
                ImGui::BeginDisabled();
                ImGui::Button("Unload", { kUW, 0.f });
                ImGui::EndDisabled();
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button,
                    { 0.46f,0.06f,0.06f,1.f });
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                    { 0.68f,0.12f,0.12f,1.f });
                if (ImGui::Button("Unload", { kUW, 0.f }))
                    ClearSlot(i);
                ImGui::PopStyleColor(2);
            }
            ImGui::PopID();
        }

        ImGui::EndTable();
    }

    // Browse-file button pinned to bottom of slot panel
    ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
    if (!enabled) ImGui::BeginDisabled();
    ImGui::PushStyleColor(ImGuiCol_Button,        { 0.07f,0.20f,0.55f,1.f });
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.11f,0.32f,0.74f,1.f });
    char browseLbl[52];
    snprintf(browseLbl, sizeof(browseLbl), "Browse file  ->  slot %d", g_selectedSlot + 1);
    if (ImGui::Button(browseLbl, { -1.f, 0.f }))
        OpenFilePicker(g_selectedSlot);
    ImGui::PopStyleColor(2);
    if (!enabled) ImGui::EndDisabled();

    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(4);

    ImGui::SameLine(0, 6);

    // ============================================================
    //  RIGHT PANEL — Figure Library (scrollable)
    //
    //  BeginChild with:
    //    width  = 0   → fills all remaining horizontal space
    //    height = panelH → explicit positive value = scroll works
    //    border = true   → draws a frame around the region
    //
    //  Do NOT add ImGuiWindowFlags_NoScrollWithMouse; the default
    //  allows mouse-wheel scrolling when the child is hovered.
    // ============================================================
    ImGui::PushStyleColor(ImGuiCol_ChildBg,       { 0.02f,0.03f,0.10f,0.97f });
    ImGui::PushStyleColor(ImGuiCol_Header,        { 0.08f,0.18f,0.46f,0.90f });
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, { 0.12f,0.28f,0.64f,0.90f });
    ImGui::PushStyleColor(ImGuiCol_HeaderActive,  { 0.16f,0.36f,0.80f,1.00f });
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.f);

    ImGui::BeginChild("##lib", ImVec2(0.f, panelH), /*border=*/true);

    if (g_lib.empty()) {
        ImGui::Spacing();
        ImGui::TextDisabled("  No figures loaded.");
        ImGui::TextDisabled("  Enter a library folder path above and press Scan.");
    } else {
        const float kLoadW = 50.f;
        int lastGame = -1, lastCat = -1;
        int shown    = 0;

        for (int i = 0; i < (int)g_lib.size(); i++) {
            const LibEntry& e = g_lib[i];
            if (!PassFilter(e)) continue;
            shown++;

            // ── Game section header ──────────────────────────
            if ((int)e.game != lastGame) {
                lastGame = (int)e.game;
                lastCat  = -1;
                if (shown > 1) ImGui::Spacing();
                ImGui::PushStyleColor(ImGuiCol_Text, kGameCol[lastGame]);
                ImGui::Text(" %s", kGameName[lastGame]);
                ImGui::PopStyleColor();
            }

            // ── Category sub-header ──────────────────────────
            if ((int)e.cat != lastCat) {
                lastCat = (int)e.cat;
                ImGui::PushStyleColor(ImGuiCol_Text,      { 0.46f,0.58f,0.80f,1.f });
                ImGui::PushStyleColor(ImGuiCol_Separator, { 0.14f,0.24f,0.48f,0.5f });
                ImGui::Text("   %s", kCatName[lastCat]);
                ImGui::Separator();
                ImGui::PopStyleColor(2);
            }

            // ── Figure row ───────────────────────────────────
            //  Layout: [indent] [color dot] [Selectable(name)] [Load btn]
            //
            //  ColorDot advances the cursor by the dot's width via Dummy+SameLine.
            //  Selectable() with an explicit ImVec2 width clips the label to that
            //  width automatically — no manual text truncation needed.
            //  SameLine(0,4) then places the Load button right after.

            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10.f); // indent
            ColorDot(kElemCol[(int)e.elem]);                       // dot + advance

            float selW = ImGui::GetContentRegionAvail().x - kLoadW - 6.f;

            ImGui::PushID(3000 + i);

            // Selectable with the figure name — ImGui clips the label to selW
            bool rowClicked = ImGui::Selectable(
                e.name.c_str(), false,
                ImGuiSelectableFlags_AllowDoubleClick,
                ImVec2(selW, 0.f));

            if (rowClicked && ImGui::IsMouseDoubleClicked(0) && enabled) {
                int tgt = g_selectedSlot;
                if (g_slots[tgt].active) tgt = FirstFreeSlot();
                if (tgt >= 0) LoadSlot(tgt, e.path);
            }

            // Tooltip on hover
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort)) {
                ImGui::BeginTooltip();
                ColorDot(kElemCol[(int)e.elem]);
                ImGui::Text("%s  |  %s  |  %s",
                    kElemName[(int)e.elem],
                    kCatName[(int)e.cat],
                    kGameName[(int)e.game]);
                ImGui::TextDisabled("%s",
#ifdef _WIN32
                    W2U(e.path.wstring()).c_str()
#else
                    e.path.string().c_str()
#endif
                );
                ImGui::EndTooltip();
            }

            // Load button — placed immediately after the selectable
            ImGui::SameLine(0, 4);
            if (!enabled) ImGui::BeginDisabled();
            ImGui::PushStyleColor(ImGuiCol_Button,        { 0.08f,0.20f,0.54f,1.f });
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.12f,0.32f,0.72f,1.f });
            if (ImGui::Button("Load", { kLoadW, 0.f })) {
                int tgt = g_selectedSlot;
                if (g_slots[tgt].active) tgt = FirstFreeSlot();
                if (tgt >= 0) LoadSlot(tgt, e.path);
            }
            ImGui::PopStyleColor(2);
            if (!enabled) ImGui::EndDisabled();

            ImGui::PopID();
        }

        if (!shown) {
            ImGui::Spacing();
            ImGui::TextDisabled("  No figures match the current filters.");
        }
    }

    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(4);

    // ── End main window ──────────────────────────────────────────
    ImGui::End();
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(3);

    if (!windowOpen) g_visible = false;
}
