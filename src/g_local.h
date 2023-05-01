/*
    Quake II Glooom, a total conversion mod for Quake II
    Copyright (C) 1999-2007  Gloom Developers

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// g_local.h -- local definitions for game module

#include "q_shared.h"

// define GAME_INCLUDE so that game.h does not define the
// short, server-visible gclient_t and edict_t structures,
// because we define the full size ones in this file
#define	GAME_INCLUDE
#include "game.h"

#include "p_menu.h"

/*Savvy custom file for balancing*/
#include "balance.h"

// the "gameversion" client command will print this plus compile date

//CHANGE THESE BEFORE COMPILING!!

#define	GAMEVERSION	"gloom"
#define GLOOMVERSION "Gloom edit by Savvy v0.7"
//#define GLOOMVERSION "Gloom Source"
#define	GLOOMVERSIONNO	  "1.s"

#ifdef RELEASE
/*Savvy checked, and this is annoying ;\
#error Remember to check all compile settings before releasing a DLL
*/
#endif

// undef this on linux/unix
#ifdef WIN32
/*Savvy this is used for a dll override realtime i presume, reeeeally dumb, i don't see a point, i'm leaving it undefined for my win32 dlls*/
//#define ADMINDLLHACK 1
#endif

// define this if you want ZZLIB adds
#ifdef WIN32
#define DEBUGBREAKPOINT _asm int 3
#else
#define DEBUGBREAKPOINT asm ("int $3")
#endif

// only allow gods to give, god, notarget and noclip
//#define ONLYGODSCANCHEAT 1

// plug GMD on level change
#ifndef RELEASE
#ifdef ONECALL
#define GMDPLUG 1
#endif
#endif

// dynamic rotation compatibility mode
// uses thresholds for min_players and loads *_maps rotations
// without this, only default_maps is loaded

// use client sound caching (works with base 1.3 install too)
#define CACHE_CLIENTSOUNDS 1

// use enhanced client anim code (requires r1q2 client)
//#define CLIENT_ANIM 1

// add vote extend code
// r1: warning, bad choice of name. VOTE_EXTEND is defined later as 7 for voting :/
//#define VOTE_EXTEND 1

// if you want guardian armor to regen near nest
#define GUARDIAN_REGEN_NEAR_NEST 1

// if you want two breeder models on random
//#define TWOBREEDERS 1

// define this if you want lpbeven code included
// NOTE: currently the dll lags too much for this to be useful
// WARNING: this is probably broken currently.
//#define LPBEVEN 1

#define DEFAULT_VELOCITY 150

// protocol bytes that can be directly added to messages
#define	svc_muzzleflash		1
#define	svc_muzzleflash2	2
#define	svc_temp_entity		3
#define	svc_layout			4
#define	svc_inventory		5
#define svc_sound 			9
#define svc_print			10
#define	svc_stufftext		11
#define svc_centerprint		15

#define FrameReference(self, frame_anim) (int)(classlist[(self)->client->resp.class_type].frames[(frame_anim)])
#define GetItemByIndex(index) ((const gitem_t *)&itemlist[index])

// hardoptz, _*Index doesn't check for already fetched index
//#define ModelIndex(n)	(modelcache[n].index ? modelcache[n].index : _ModelIndex(n)
#define ImageIndex(n)	imagecache[n] ? imagecache[n] : _ImageIndex(n)
#define SoundIndex(n)	soundcache[n] ? soundcache[n] : (unsigned int)_SoundIndex(n) //FV7 typecast _SoundIndex to unsigned int

#ifdef UNDEFINED
void temp_point(int type, vec3_t origin);
void temp_line(int type, vec3_t origin, vec3_t tr_endpos);
void temp_lightning(int dest_entnum, int entnum, vec3_t dest_origin, vec3_t origin);
#endif

#if 1
// temp ent macros
#define temp_point(type, origin) \
do { \
	gi.WriteByte(svc_temp_entity); \
	gi.WriteByte(type); \
	gi.WritePosition(origin); \
} while (0)

#define temp_line(type, origin, tr_endpos) \
do { \
	gi.WriteByte(svc_temp_entity); \
	gi.WriteByte(type); \
	gi.WritePosition(origin); \
	gi.WritePosition(tr_endpos); \
} while (0)

#define temp_impact(type, origin, movedir) \
do { \
	gi.WriteByte(svc_temp_entity); \
	gi.WriteByte(type); \
	gi.WritePosition(origin); \
	gi.WriteDir(movedir); \
} while (0)

// special ones
#define temp_splash(type, count, origin, movedir, style) \
do { \
	gi.WriteByte(svc_temp_entity); \
	gi.WriteByte(type); \
	gi.WriteByte(count); \
	gi.WritePosition(origin); \
	gi.WriteDir(movedir); \
	gi.WriteByte(style); \
} while (0)

#define temp_attack(type, entnum, origin, tr_endpos) \
	do { gi.WriteByte(svc_temp_entity); gi.WriteByte(type); \
	gi.WriteShort(entnum); gi.WritePosition(origin); gi.WritePosition(tr_endpos); } while (0)
#define temp_grapple(entnum, origin, tr_endpos, pos1) \
	do { gi.WriteByte(svc_temp_entity); gi.WriteByte(TE_GRAPPLE_CABLE); \
	gi.WriteShort(entnum); gi.WritePosition(origin); gi.WritePosition(tr_endpos); gi.WritePosition(pos1); } while (0)
#define temp_lightning(dest_entnum, entnum, dest_origin, origin) \
	do { gi.WriteByte(svc_temp_entity); gi.WriteByte(TE_LIGHTNING); \
	gi.WriteShort(dest_entnum); gi.WriteShort(entnum); gi.WritePosition(dest_origin); gi.WritePosition(origin); } while (0)
#define temp_flashlight(origin, flash_entnum) \
	do { gi.WriteByte(svc_temp_entity); gi.WriteByte(TE_FLASHLIGHT); \
	gi.WritePosition(origin); gi.WriteShort(flash_entnum); } while (0)
#define temp_forcewall(origin, tr_endpos, style) \
	do { gi.WriteByte(svc_temp_entity); gi.WriteByte(TE_FORCEWALL); \
	gi.WritePosition(origin); gi.WritePosition(tr_endpos); gi.WriteShort(style); } while (0)
#define temp_steam(nextid, count, origin, movedir, style, plat2flags, wait) \
	do { gi.WriteByte(svc_temp_entity); gi.WriteByte(TE_STEAM); \
	gi.WriteShort(nextid); gi.WriteByte(count); gi.WritePosition(origin); gi.WriteDir(movedir); \
	gi.WriteByte(style); gi.WriteShort(plat2flags); if (nextid != -1) gi.WriteLong(wait); } while (0)
#define temp_widow(type, origin) do { gi.WriteByte(svc_temp_entity); gi.WriteByte(TE_WIDOWBEAMOUT); \
	gi.WriteShort(type); gi.WritePosition(origin); } while (0)

#else

void temp_point (int type, vec3_t origin);
void temp_line (int type, vec3_t origin, vec3_t tr_endpos);
void temp_impact (int type, vec3_t origin, vec3_t movedir);
void temp_splash (int type, int count, vec3_t origin, vec3_t movedir, int style);

#endif

#define	MAX_IPFILTERS		1024

#define CHASE_VIEWS			5

#define MAPS_DEFAULT 0
#define	MAPS_TINY	1
#define	MAPS_SMALL	2
#define MAPS_MEDIUM	3
#define MAPS_LARGE	4

extern	int	oldtimelimit;

extern	int	currentmapindex;
extern	int	currentmapmode;

typedef enum ctfgrapplestate_e {
	CTF_GRAPPLE_STATE_FLY,
	CTF_GRAPPLE_STATE_PULL,
	CTF_GRAPPLE_STATE_HANG
} ctfgrapplestate_t;

typedef enum entitytype_e {
	ENT_NULL, //we don't want 0 being used as a meaningful value.
	ENT_DRONEWEB,
	ENT_SPIKE,
	ENT_TURRET,
	ENT_MGTURRET,
	ENT_TURRETBASE,
	ENT_DETECTOR,
	ENT_TRIPWIRE_BOMB,
	ENT_TRIPWIRE_LASER,
	ENT_AMMO_DEPOT,
	ENT_TELEPORTER_U, //U = unbuilt/layed
	ENT_TELEPORTER_D, //D = disabled
	ENT_TELEPORTER,   //  = normal
	ENT_COCOON_U,
	ENT_COCOON_D,
	ENT_COCOON,
	ENT_SPIKER,
	ENT_OBSTACLE,
	ENT_GASSER,
	ENT_HEALER,
	ENT_WRAITHBOMB,
	ENT_ROCKET,
	ENT_GRENADE,
	ENT_FLASH_GRENADE,
	ENT_SMOKE_GRENADE,
	ENT_C4,
	ENT_GAS_SPORE,
	ENT_SPIKE_SPORE,
	ENT_PROXY_SPORE,
	ENT_FLARE,
	ENT_INFEST,
	ENT_CORPSE,
	ENT_HUMAN_BODY,
	ENT_FEMALE_BODY,
	ENT_FLAMES,
	ENT_GASSER_GAS,
	ENT_BLASTER,
	ENT_NOBLOOD_BODY,
	ENT_AREAPORTAL,
	ENT_FUNC_DOOR,
	ENT_TARGET_BLASTER,
	ENT_PROXYSPIKE,
	ENT_FUNC_EXPLOSIVE,
	ENT_FUNC_WALL,
	ENT_EVIL_HEALER,
	ENT_SD_CAUSER,
/*Savvy custom ents
ENT_MAP_ prefixed ones are map ents that need to be identified
*/
  ENT_MAP_TRIGGER_REPAIRER,
  ENT_MAP_TRIGGER_HEALER,
#ifdef SAVVY_CONTENT
  ENT_COLDBLAST,
  ENT_PROXY_MINE,
#endif
  ENT_SPBOT
} entitytype_t;

typedef enum image_cache_e {
	i_null, //magic :)
	i_health,
	a_bullets,
	a_acid,
	a_blaster,
	c4_i,
	ggren_i,
	gren_i,
	flare_i,
	i_9mmclip,
	i_acammo,
	i_autoclip,
	i_autogun,
	i_autoshotgun,
	i_cocoon,
	turtle,
	i_magclip,
	i_magnum,
	i_missile,
	i_plasmagun,
	i_pistol,
	i_rl,
	i_shellclip,
	i_shellex,
	i_spas,
	i_spore,
	i_sspore,
	i_subclip,
	i_submachinegun,
	i_tele,
	a_shells,
	a_cells,
	inventory,
	teammenu,
	alienmenu,
	humanmenu,
	breedermenu,
	i_powershield,
	i_combatarmor,
	i_jacketarmor,
	i_bodyarmor,
	engiemenu,
	k_security,
	i_airstrike,
	k_pyramid,
	k_datacd,
	k_redkey,
	k_powercube,
#ifdef SAVVY_CONTENT
/*Savvy custom icons needed*/
  i_proxy_mine,
#endif
	MAX_IMAGE_CACHE
} image_cache_t;

typedef enum sound_cache_e {
	sound_null,
	items_s_health,
	misc_keytry,
	items_protect4,
	misc_lasfly,
	weapons_noammo,
	player_fry,
	weapons_machgf1b,
	weapons_up_end,
	weapons_rg_hum,
	misc_fhit3,
	voice_toastie,
	misc_activate,
	weapons_laser2,
	weapons_bfg_hum,
	victory_human,
	weapons_keyboard,
	misc_power1,
	plats_pt1_strt,
	misc_power2,
	weapons_acid,
	weapons_pistgf1,
	weapons_gg_on,
	weapons_subfire,
	mutant_mutatck1,
	cocoon_drud1,
	mutant_mutatck2,
	player_watr_out,
	switches_butn2,
	misc_spawn1,
	alien_watr_in,
	world_electro,
	weapons_grenlb1b,
	weapons_flame,
	world_blackhole,
	alien_drown1,
	player_gasp1,
	player_watr_in,
	player_gasp2,
	misc_windfly,
	weapons_railgf1a,
	weapons_hgrenc1b,
	weapons_grapple_grreset,
	weapons_grenlf1a,
	misc_bdeath,
	tank_thud,
	organ_organe2,
	world_force1,
	organ_organe3,
	plats_pt1_mid,
	organ_organe4,
	alien_watr_out,
	world_quake,
	misc_h2ohit1,
	turret_Tlasexplode,
	//world_land,
	weapons_targbeep,
	makron_laf4,
	weapons_chomp,
	alien_watr_un,
	world_spark1,
	world_amb10,
	weapons_hgrent1a,
	world_spark3,
	world_ric1,
	player_watr_un,
	misc_udeath,
	items_damage3,
	world_laser,
	player_drown1,
	victory_alien,
	weapons_hgrenb1a,
	plats_pt1_end,
	berserk_attack,
	weapons_pulsfire,
	weapons_rockfly,
	misc_ssdie,
	items_l_health,
	weapons_pulseout,
	weapons_magshot,
	weapons_c4,
	items_damage,
	items_m_health,
	misc_secret,
	world_klaxon2,
	alien_gurp1,
	world_mov_watr,
	alien_gurp2,
	misc_comp_up,
	world_10_0,
	weapons_grapple_grfire,
	misc_talk,
	doors_dr1_mid,
	misc_ar1_pkup,
	organ_twang,
	world_stp_watr,
	weapons_hgrenb2a,
	weapons_tick2,
	alien_gasp1,
	alien_gasp2,
	misc_talk1,
	weapons_rocklx1a,
	weapons_davelas,
	wraith_fly,
	weapons_mechbeep,
	weapons_refill,
	weapons_up,
	detector_alarm1,
	detector_alarm2,
	detector_alarm3,
	misc_keyuse,
	misc_turrethum,
	doors_dr1_end,
	player_burn1,
	doors_dr1_strt,
	weapons_rocklf1a,
	player_burn2,
	weapons_webshot1,
	weapons_webshot2,
	player_lava_in,
	weapons_shotweb1,
	organ_healer1,
	weapons_misload,
	ambient_humamb,
	organ_growegg2,
	misc_5min,
	organ_growegg,

#ifdef CACHE_CLIENTSOUNDS
	player_land1, // used by EV_FALLSHORT

	// we need to repeat everything if we want to stay compatible
	// new pak structure can remove LOT of this shiz
	grunt_jump,
	grunt_pain100_1, grunt_pain100_2, grunt_pain75_1, grunt_pain75_2,
	grunt_pain50_1, grunt_pain50_2, grunt_pain25_1, grunt_pain25_2,
	grunt_death1, grunt_death2, grunt_death3, grunt_death4,
	grunt_fall1, grunt_fall2,
	// special voicegroups like grunt just require entrys in line to stay together
	grunt_voice1, grunt_voice2, grunt_voice3,
	grunt_voice4, grunt_voice5,
	grunt_voice6, grunt_voice7, grunt_voice8, grunt_voice9,
	grunt_voice10, grunt_voice11, grunt_voice12,
	grunt_voice13, grunt_voice14, grunt_voice15,
	grunt_voice16, grunt_voice17, grunt_voice18,
	grunt_voice19, grunt_voice20, grunt_voice21,

	hatch_jump, 
	hatch_pain100_1, hatch_pain100_2, hatch_pain75_1, hatch_pain75_2,
	hatch_pain50_1, hatch_pain50_2, hatch_pain25_1, hatch_pain25_2,
	hatch_death1, hatch_death2, hatch_death3, hatch_death4,
	hatch_fall1, hatch_fall2,
	// generic voices are referenced by *_spawn, so change the ordering => change *Voice func
	hatch_spawn,
	hatch_scream1, hatch_scream2, hatch_scream3, hatch_scream4, 
	hatch_scream5, hatch_scream6,

	kamikaze_jump, 
	kamikaze_pain100_1, kamikaze_pain100_2, kamikaze_pain75_1, kamikaze_pain75_2,
	kamikaze_pain50_1, kamikaze_pain50_2, kamikaze_pain25_1, kamikaze_pain25_2,
	kamikaze_death1, kamikaze_death2, kamikaze_death3, kamikaze_death4,
	kamikaze_fall1, kamikaze_fall2,
	kamikaze_spawn,
	kamikaze_scream1, kamikaze_scream2, kamikaze_scream3, kamikaze_scream4, 
	kamikaze_scream5, kamikaze_scream6,

	heavy_jump, 
	heavy_pain100_1, heavy_pain100_2, heavy_pain75_1, heavy_pain75_2,
	heavy_pain50_1, heavy_pain50_2, heavy_pain25_1, heavy_pain25_2,
	heavy_death1, heavy_death2, heavy_death3, heavy_death4,
	heavy_fall1, heavy_fall2,
	heavy_spawn,
	heavy_voice1, heavy_voice2, heavy_voice3, heavy_voice4, 
	heavy_voice5, heavy_voice6,

	commando_jump, 
	commando_pain100_1, commando_pain100_2, commando_pain75_1, commando_pain75_2,
	commando_pain50_1, commando_pain50_2, commando_pain25_1, commando_pain25_2,
	commando_death1, commando_death2, commando_death3, commando_death4,
	commando_fall1, commando_fall2,
	commando_spawn,
	commando_voice1, commando_voice2, commando_voice3, commando_voice4, 
	commando_voice5, commando_voice6,

	drone_jump, 
	drone_pain100_1, drone_pain100_2, drone_pain75_1, drone_pain75_2,
	drone_pain50_1, drone_pain50_2, drone_pain25_1, drone_pain25_2,
	drone_death1, drone_death2, drone_death3, drone_death4,
	drone_fall1, drone_fall2,
	drone_spawn,
	drone_scream1, drone_scream2, drone_scream3, drone_scream4, 
	drone_scream5, drone_scream6,

	mech_jump,
	mech_pain100_1, mech_pain100_2, mech_pain75_1, mech_pain75_2,
	mech_pain50_1, mech_pain50_2, mech_pain25_1, mech_pain25_2,
	mech_death1, mech_death2, mech_death3, mech_death4,
	mech_fall1, mech_fall2,
	mech_spawn,
	mech_voice1, mech_voice2, mech_voice3, mech_voice4, 
	mech_voice5, mech_voice6,
	mech_step,
	mech_shot,

	shock_jump,
	shock_pain100_1, shock_pain100_2, shock_pain75_1, shock_pain75_2,
	shock_pain50_1, shock_pain50_2, shock_pain25_1, shock_pain25_2,
	shock_death1, shock_death2, shock_death3, shock_death4,
	shock_fall1, shock_fall2,
	shock_spawn,
	shock_voice1, shock_voice2, shock_voice3, shock_voice4, 
	shock_voice5, shock_voice6,

	stalker_jump,
	stalker_pain100_1, stalker_pain100_2, stalker_pain75_1, stalker_pain75_2,
	stalker_pain50_1, stalker_pain50_2, stalker_pain25_1, stalker_pain25_2,
	stalker_death1, stalker_death2, stalker_death3, stalker_death4,
	stalker_fall1, stalker_fall2,
	stalker_spawn,
	stalker_scream1, stalker_scream2, stalker_scream3, stalker_scream4, 
	stalker_scream5, stalker_scream6,

	breeder_jump,
	breeder_pain100_1, breeder_pain100_2, breeder_pain75_1, breeder_pain75_2,
	breeder_pain50_1, breeder_pain50_2, breeder_pain25_1, breeder_pain25_2,
	breeder_death1, breeder_death2, breeder_death3, breeder_death4,
	breeder_fall1, breeder_fall2,
	breeder_spawn,
	breeder_scream1, breeder_scream2, breeder_scream3, breeder_scream4, 
	breeder_scream5, breeder_scream6,

	engineer_jump,
	engineer_pain100_1, engineer_pain100_2, engineer_pain75_1, engineer_pain75_2,
	engineer_pain50_1, engineer_pain50_2, engineer_pain25_1, engineer_pain25_2,
	engineer_death1, engineer_death2, engineer_death3, engineer_death4,
	engineer_fall1, engineer_fall2,
	engineer_spawn,
	engineer_voice1, engineer_voice2, engineer_voice3, engineer_voice4, 
	engineer_voice5, engineer_voice6,

	guardian_jump,
	guardian_pain100_1, guardian_pain100_2, guardian_pain75_1, guardian_pain75_2,
	guardian_pain50_1, guardian_pain50_2, guardian_pain25_1, guardian_pain25_2,
	guardian_death1, guardian_death2, guardian_death3, guardian_death4,
	guardian_fall1, guardian_fall2,
	guardian_spawn,
	guardian_scream1, guardian_scream2, guardian_scream3, guardian_scream4, 
	guardian_scream5, guardian_scream6,

	exterm_jump,
	exterm_pain100_1, exterm_pain100_2, exterm_pain75_1, exterm_pain75_2,
	exterm_pain50_1, exterm_pain50_2, exterm_pain25_1, exterm_pain25_2,
	exterm_death1, exterm_death2, exterm_death3, exterm_death4,
	exterm_fall1, exterm_fall2,
	exterm_spawn,
	exterm_voice1, exterm_voice2, exterm_voice3, exterm_voice4, 
	exterm_voice5, exterm_voice6,
	exterm_step,

	stinger_jump,
	stinger_pain100_1, stinger_pain100_2, stinger_pain75_1, stinger_pain75_2,
	stinger_pain50_1, stinger_pain50_2, stinger_pain25_1, stinger_pain25_2,
	stinger_death1, stinger_death2, stinger_death3, stinger_death4,
	stinger_fall1, stinger_fall2,
	stinger_spawn,
	stinger_scream1, stinger_scream2, stinger_scream3, stinger_scream4, 
	stinger_scream5, stinger_scream6,

	wraith_jump,
	wraith_pain100_1, wraith_pain100_2, wraith_pain75_1, wraith_pain75_2,
	wraith_pain50_1, wraith_pain50_2, wraith_pain25_1, wraith_pain25_2,
	wraith_death1, wraith_death2, wraith_death3, wraith_death4,
	wraith_fall1, wraith_fall2,
	wraith_spawn,
	wraith_scream1, wraith_scream2, wraith_scream3, wraith_scream4, 
	wraith_scream5, wraith_scream6,

	biotech_jump,
	biotech_pain100_1, biotech_pain100_2, biotech_pain75_1, biotech_pain75_2,
	biotech_pain50_1, biotech_pain50_2, biotech_pain25_1, biotech_pain25_2,
	biotech_death1, biotech_death2, biotech_death3, biotech_death4,
	biotech_fall1, biotech_fall2,
	biotech_spawn,
	biotech_voice1, biotech_voice2,	biotech_voice3,
	biotech_voice4,
	biotech_voice5,
	biotech_voice6,
	biotech_voice7,
	biotech_voice8, biotech_voice9,

#endif

	MAX_SOUND_CACHE
} sound_cache_t;

/*Savvy - edited grapple here
original stuff when i got code:
#define CTF_GRAPPLE_SPEED 750 // speed of grapple in flight
#define CTF_GRAPPLE_PULL_SPEED 600	// speed player is pulled at (chamfix, was 650)
*/
#define CTF_GRAPPLE_SPEED 1000
#define CTF_GRAPPLE_PULL_SPEED 600

#define NUM_PLAYER_FRAMES	36

#define FRAME_STAND_S		0
#define FRAME_STAND_E		1
#define FRAME_RUN_S			2
#define FRAME_RUN_E			3
#define FRAME_ATTACK1_S		4
#define FRAME_ATTACK1_E		5
#define FRAME_ATTACK2_S		6
#define FRAME_ATTACK2_E		7
#define FRAME_PAIN1_S		8
#define FRAME_PAIN1_E		9
#define FRAME_PAIN2_S		10
#define FRAME_PAIN2_E		11
#define FRAME_TAUNT1_S		12
#define FRAME_TAUNT1_E		13
#define FRAME_TAUNT2_S		14
#define FRAME_TAUNT2_E		15
#define FRAME_TAUNT3_S		16
#define FRAME_TAUNT3_E		17
#define FRAME_DIE1_S		18
#define FRAME_DIE1_E		19
#define FRAME_DIE2_S		20
#define FRAME_DIE2_E		21
#define FRAME_DIE3_S		22
#define FRAME_DIE3_E		23
#define FRAME_DUCK_S		24
#define FRAME_DUCK_E		25
#define FRAME_DUCKRUN_S		26
#define FRAME_DUCKRUN_E		27
#define FRAME_DUCKPAIN_S	28
#define FRAME_DUCKPAIN_E	29
#define FRAME_DUCKDIE_S		30
#define FRAME_DUCKDIE_E		31
#define FRAME_PAIN3_S		32
#define FRAME_PAIN3_E		33
#define FRAME_JUMP_S		34
#define FRAME_JUMP_E		35

#define FRAME_WRAITH_FLYING_DEATH_S	FRAME_DIE3_S
#define FRAME_WRAITH_FLYING_DEATH_E	FRAME_DIE3_E
#define FRAME_PAIN_FLYING_S	FRAME_PAIN3_S
#define FRAME_PAIN_FLYING_E	FRAME_PAIN3_E
#define FRAME_FLY_S			FRAME_DUCKRUN_S
#define FRAME_FLY_E			FRAME_DUCKRUN_E
#define FRAME_FLY_FALL_S	FRAME_DUCKPAIN_S
#define	FRAME_FLY_FALL_E	FRAME_DUCKPAIN_E

// client sound caching
#define MAX_DEATH_SOUNDS 4
#define MAX_FALL_SOUNDS 2
#define MAX_PAIN_SOUNDS 8

#define SOUND_JUMP	0
#define SOUND_PAIN	1
#define SOUND_DEATH (SOUND_PAIN+MAX_PAIN_SOUNDS)
#define SOUND_FALL	(SOUND_DEATH+MAX_DEATH_SOUNDS)

#define NUM_CLASS_SOUNDS (SOUND_FALL+MAX_FALL_SOUNDS)

#define CONTENTS_NOBUILD	(CONTENTS_AUX|CONTENTS_MIST)

/*Savvy added into balance.h
// buildable defines
#define		TEAM_POINTS		300

#define		COST_EGG		40
#define		COST_OBSTACLE	20
#define		COST_SPIKER		30
#define		COST_GASSER		20
#define		COST_HEALER		50

#define		COST_TELEPORT	40
#define		COST_TURRET		60
#define		COST_MGTURRET	40
#define		COST_MINE		30
#define		COST_DETECTOR	20
#define		COST_DEPOT		60
*/

// class defines
/*Savvy horribly stupid
#define		CLASS_GRUNT		0
#define		CLASS_HATCHLING	1
#define		CLASS_HEAVY		2
#define		CLASS_COMMANDO	3
#define		CLASS_DRONE		4
#define		CLASS_MECH		5
#define		CLASS_SHOCK		6
#define		CLASS_STALKER	7
#define		CLASS_BREEDER	8
#define		CLASS_ENGINEER	9
#define		CLASS_GUARDIAN	10
#define		CLASS_KAMIKAZE	11
#define		CLASS_EXTERM	12
#define		CLASS_STINGER	13
#define		CLASS_WRAITH	14
#define		CLASS_BIO		15
#define		CLASS_OBSERVER	16
*/

enum {
CLASS_GRUNT,
CLASS_HATCHLING,
CLASS_HEAVY,
CLASS_COMMANDO,
CLASS_DRONE,
CLASS_MECH,
CLASS_SHOCK,
CLASS_STALKER,
CLASS_BREEDER,
CLASS_ENGINEER,
CLASS_GUARDIAN,
CLASS_KAMIKAZE,
CLASS_EXTERM,
CLASS_STINGER,
CLASS_WRAITH,
CLASS_BIO,
CLASS_OBSERVER
};

#define NUMCLASSES	16 // don't include observer here

//yuck for stats
#define		SPAWNKILLS		16
#define		PLAYERDEATHS	17

#define		TEAM_NONE		0
#define		TEAM_ALIEN		1
#define		TEAM_HUMAN		2
#define		MAXTEAMS		3

// special winning mode
#define		MODE_NORMAL		0
#define		MODE_HUMANWIN	1
#define		MODE_ALIENWIN	2

//	BC means Bit Class, btw
#define BC_ENGINEER		0x00000001
#define BC_GRUNT		0x00000002
#define BC_SHOCK		0x00000004
#define BC_HEAVY		0x00000008
#define BC_BIO			0x00000010
#define BC_COMMANDO		0x00000020
#define BC_EXTERM		0x00000040
#define BC_MECH			0x00000080

#define BC_BREEDER		0x00010000
#define BC_HATCHLING	0x00020000
#define BC_DRONE		0x00040000
#define BC_KAMI			0x00080000
#define BC_WRAITH		0x00100000
#define BC_STINGER		0x00200000
#define BC_GUARDIAN		0x00400000
#define BC_STALKER		0x00800000

//weapon classes (what CAN hurt this ent)
#define WC_AUTOGUN 		0x00000002
#define WC_POKE			0x00000004
#define WC_PISTOL		0x00000008
#define WC_SHOTGUN		0x00000010
#define WC_SCATTER		0x00000020
#define WC_SMG 			0x00000040
#define WC_MAGNUM 		0x00000080
#define WC_PULSE 		0x00000100
#define WC_ROCKET 		0x00000200
#define WC_GRENADE 		0x00000400
#define WC_C4 			0x00000800
#define WC_EXSHELL 		0x00001000
#define WC_DISCHARGE 	0x00002000
#define WC_AUTOCANNON 	0x00004000
#define WC_JUMPATTACK 	0x00008000
#define WC_MELEE 		0x00010000
#define WC_SPIKESPORE 	0x00020000
#define WC_SPIKES 		0x00040000
#define WC_GAS			0x00080000
#define WC_FIRE			0x00100000
#define WC_WRAITHSPIT 	0x00200000
#define WC_KAMI	 		0x00400000
#define WC_FLASHGRENADE 0x00800000
#define	WC_NO_SPARKS	0x80000000

//damn you cataclaw.
#define DF_INFINITE_AMMO	0x00002000	// 8192

/*Savvy sorry r1, moving it to a better spot :)

//r1chs hax0red constants
#define	HEALER_HEALTH	60
#define	HEALER_GIB_HEALTH -100

#define	SPIKER_HEALTH 250
#define	SPIKER_GIB_HEALTH -100

#define INFEST_HEALTH	250

#define	DRONE_SPIT_MAX	5

#define SHOT_SHELL_SPREAD 400
#define SHOT_SHELL_DAMAGE 13
#define SHELL_KICK	10
#define EX_SHELL_CROUCH_SPREAD 300
#define EX_SHELL_DEFAULT_SPREAD 600
#define EX_SHELL_DAMAGE	75

#define SPIKE_GRENADE_THROW_DELAY 1.5f
#define SPIKE_GRENADE_EXPLODE_DELAY 2
#define SPIKE_GRENADE_SPIKE_SPEED 700
#define SPIKE_GRENADE_SPIKE_COUNT 10

#define BIOTECH_MAX_CELLS 750
*/

// this converts from CLASS_ to BC_
extern long classtypebit[16];

// human voices
#define		VOICE_SPAWN		0
#define		VOICE_HELP		1
#define		VOICE_ORDER		2
#define		VOICE_AFFIRM	3
#define		VOICE_DEFEND	4
#define		VOICE_NOAMMO	5
#define		VOICE_TAUNT		6


// view pitching times
#define DAMAGE_TIME		0.5f
#define	FALL_TIME		0.3f


// edict->flags, user-defined
#define	FL_FLY					0x00000001
#define	FL_SWIM					0x00000002	// implied immunity to drowining
#define FL_IMMUNE_LASER			0x00000004
#define	FL_INWATER				0x00000008
#define	FL_GODMODE				0x00000010
#define	FL_NOTARGET				0x00000020
#define FL_IMMUNE_SLIME			0x00000040
#define FL_IMMUNE_LAVA			0x00000080
#define	FL_PARTIALGROUND		0x00000100	// not all corners are valid
#define	FL_WATERJUMP			0x00000200	// player jumping out of water
#define	FL_TEAMSLAVE			0x00000400	// not the first on the team
#define FL_NO_KNOCKBACK			0x00000800
#define FL_POWER_ARMOR			0x00001000	// power armor (if any) is active
#define FL_NOWATERNOISE			0x00002000
#define	FL_NO_HURTSPARKS		0x00004000
#define	FL_SELFGLOW				0x00008000	// self glow (alien navigation system?)

#define FL_MAP_SPAWNED			0x00010000	// map spawned (used for teles and eggs with xminrule)
#define	FL_CLIPPING				0x00020000	// special clipping for humie structs

#define	FRAMETIME		0.1f

// memory tags to allow dynamic memory to be cleaned up
#define	TAG_GAME	765		// clear when unloading the dll
#define	TAG_LEVEL	766		// clear when loading a new level


#define MELEE_DISTANCE	80

#define BODY_QUEUE_SIZE		24

typedef enum {
	ITEM_NONE,
	ITEM_ARMOR_BODY,
	ITEM_ARMOR_MECH,
	ITEM_ARMOR_COMBAT,
	ITEM_ARMOR_JACKET,
	ITEM_ARMOR_POWERSHIELD,
	ITEM_WEAPON_PISTOL,
	ITEM_WEAPON_MAGNUM,
	ITEM_WEAPON_SHOTGUN,
	ITEM_WEAPON_SCATTERGUN,
	ITEM_WEAPON_AUTOGUN,
	ITEM_WEAPON_SUBGUN,
	ITEM_WEAPON_ROCKET_LAUNCHER,
	ITEM_WEAPON_PULSE_LASER,
	ITEM_AMMO_SHELLS,
	ITEM_AMMO_EXSHELLS,
	ITEM_AMMO_SCATTERSHOT,
	ITEM_AMMO_BULLETS,
	ITEM_AMMO_MAGNUM,
	ITEM_AMMO_AUTOSHOT,
	ITEM_AMMO_SUBSHOT,
	ITEM_AMMO_SPIT,
	ITEM_AMMO_MECHCANNON,
	ITEM_AMMO_SPIKES,
	ITEM_AMMO_CELLS,
	ITEM_AMMO_ROCKETS,
	ITEM_AMMO_MINIMISSILES,
	ITEM_AMMO_9MM,
	ITEM_AMMO_MAGCLIP,
	ITEM_AMMO_SHELLCLIP,
	ITEM_AMMO_EXSHELLCLIP,
	ITEM_AMMO_SCATTERCLIP,
	ITEM_AMMO_AUTOCLIP,
	ITEM_AMMO_ROCKETCLIP,
	ITEM_AMMO_SUBGUNCLIP,
	ITEM_GRENADE,
	ITEM_GAS_GRENADE,
	ITEM_FLASH_GRENADE,
	ITEM_FLARE,
	ITEM_C4,
	ITEM_SPORE,
	ITEM_SPIKESPORE,
	ITEM_HEALTH,
	ITEM_KEY_PASS,
	ITEM_KEY_AIRSTRIKE,
	ITEM_KEY_PYRAMID,
	ITEM_KEY_CUBE,
	ITEM_KEY_DATACD,
	ITEM_KEY_REDKEY,
#ifdef SAVVY_CONTENT
	ITEM_PROXY_MINE,
#endif
	ITEM_NULL_EOL
} itemindexcache_e;

typedef enum
{
	DAMAGE_NO,
	DAMAGE_YES,			// will take damage if hit
	DAMAGE_AIM			// auto targeting recognizes this
} damage_t;

typedef enum
{
	WEAPON_READY,
	WEAPON_ACTIVATING,
	WEAPON_DROPPING,
	WEAPON_RELOADING,
	WEAPON_FIRING
} weaponstate_t;

//deadflag
#define DEAD_NO					0
#define DEAD_DYING				1
#define DEAD_DEAD				2
//#define DEAD_RESPAWNABLE		3

#define GOD_NORMAL				1
#define	GOD_ADMIN				2

//range
#define RANGE_MELEE				0
#define RANGE_NEAR				1
#define RANGE_MID				2
#define RANGE_FAR				3

//gib types
#define GIB_ORGANIC				0
#define GIB_METALLIC			1
#define GIB_GREEN				2

#define AI_HOLD_FRAME			0x00000080
#define AI_NOSTEP				0x00000400

// armor types
#define ARMOR_NONE				0
#define ARMOR_JACKET			1
#define ARMOR_COMBAT			2
#define ARMOR_BODY				3
#define ARMOR_FULL				4

// power armor types
#define POWER_ARMOR_NONE		0
#define POWER_ARMOR_SCREEN		1
#define POWER_ARMOR_SHIELD		2

// handedness values
#define RIGHT_HANDED			0
#define LEFT_HANDED				1
#define CENTER_HANDED			2

// game.serverflags values
#define SFL_CROSS_TRIGGER_1		0x00000001
#define SFL_CROSS_TRIGGER_2		0x00000002
#define SFL_CROSS_TRIGGER_3		0x00000004
#define SFL_CROSS_TRIGGER_4		0x00000008
#define SFL_CROSS_TRIGGER_5		0x00000010
#define SFL_CROSS_TRIGGER_6		0x00000020
#define SFL_CROSS_TRIGGER_7		0x00000040
#define SFL_CROSS_TRIGGER_8		0x00000080
#define SFL_CROSS_TRIGGER_MASK	0x000000ff

// edict->movetype values
typedef enum movetype_e
{
	MOVETYPE_NONE,			// never moves
	MOVETYPE_NOCLIP,		// origin and angles change with no interaction
	MOVETYPE_PUSH,			// no clip to world, push on box contact
	MOVETYPE_STOP,			// no clip to world, stops on box contact

	MOVETYPE_WALK,			// gravity
	MOVETYPE_STEP,			// gravity, special edge handling
	MOVETYPE_FLY,
	MOVETYPE_TOSS,			// gravity
	MOVETYPE_FLYMISSILE,	// extra size to monsters
	MOVETYPE_BOUNCE
} movetype_t;

typedef struct
{
	int		base_count;
	int		max_count;
	float	normal_protection;
	float	energy_protection;
	int		armor;
} gitem_armor_t;

// gitem_t->flags
#define	IT_WEAPON		1		// use makes active weapon
#define	IT_AMMO			2
#define IT_ARMOR		4
#define IT_STAY_COOP	8
#define IT_KEY			16
#define IT_POWERUP		32
#define	IT_CLIP			64

// ammo types for clips
enum {
	AMMO_NONE,
	AMMO_SHELLS,
	AMMO_EXSHELLS,
	AMMO_SCATTER,
	AMMO_BULLETS,
	AMMO_MAGNUM,
	AMMO_AUTO,
	AMMO_SUB,
	AMMO_FUEL,
	AMMO_CANNON,
	AMMO_SPIKES,
	AMMO_ROCKETS,
	AMMO_CELLS,
	AMMO_GRENADE,
	AMMO_C4,
	AMMO_SPORE,
/*Savvy human mine*/
	AMMO_PMINE
};

// clip types for weapons
enum {
	CLIP_NONE,
	CLIP_SHELLS,
	CLIP_SCATTER,
	CLIP_BULLETS,
	CLIP_MAGNUM,
	CLIP_AUTO,
	CLIP_SUB,
	CLIP_FUEL,
	CLIP_CANNON,
	CLIP_SPIKES,
	CLIP_ROCKETS,
	CLIP_CELLS,
	CLIP_GRENADE,
	CLIP_C4,
	CLIP_SPORE
};

typedef struct gitem_s
{
	char		*classname;	// spawning name
	qboolean	(*pickup)(struct edict_s *ent, struct edict_s *other);
	void		(*use)(struct edict_s *ent, const struct gitem_s *item);
	void		(*drop)(struct edict_s *ent, const struct gitem_s *item);
	void		(*weaponthink)(struct edict_s *ent);
	char		*pickup_sound;
	char		*active_sound;
	char		*world_model;
	int			world_model_flags;
	char		*view_model;

	// client side info
	char		*icon;
	char		*pickup_name;	// for printing on pickup
	int			count_width;		// number of digits to display by icon

	int			quantity;		// for ammo how much, for weapons how much is used per shot

	int			clip_type;		// for weapons
	int			flags;			// IT_* flags

	void		*info;
	int			tag;
	//int			size;			// Players are limited to a certain carrying capacity
	char		*precaches;		// string of all models, sounds, and images this item will use
	int			item_id;		//for image cache
} gitem_t;

#define	FAILED_VOTES_LIMIT	2 // number of failed votes allowed to init per map

#define		VOTE_INVALID	0 // keep this on 0
#define		VOTE_KICK		1
#define		VOTE_MAP		2
#define		VOTE_NEXTMAP	3
#define		VOTE_TEAMKICK	4
#define		VOTE_MUTE		5
#define		VOTE_ENDMAP		6
#define		VOTE_EXTEND		7
#define		VOTE_SD			8

// tumu: resp.voted status on players, bitmask
// VOTE_INVALID used for 0
#define		VOTE_NO		1
#define		VOTE_YES	2
#define		VOTE_NOT_ELIGIBLE 4
#define		VOTE_VETO	8
#define		VOTE_ABSTAIN	16

typedef struct vote_s
{
	edict_t	*starter;				// person who started vote
	unsigned int target_index;		// index of person being kicked/muted or a map
	unsigned int lastmap_index[2];	// anti same map vote spam
	unsigned int type;				// type of vote (see above)
	unsigned int voters;			// num of ppl in the game when the vote started
	unsigned int timer;				// time vote ends
	//char	iplog[MAX_CLIENTS][16];	// voted IPs
} vote_t;

typedef struct fragsave_s
{
	int		inuse;
	unsigned int ghostcode;
	float	expiretime;
	int		frags;
	int		team;
	int		oldclass;
	int		total_score;
	int		starttime;
	vec3_t	oldorigin;
	vec3_t	oldangles;
	char	netname[16];
} fragsave_t;

typedef struct reconnect_s
{
	char		ip[22];
	char		message[16];
	char		value[16];
	float		time;
	int			token;
	qboolean	completed;
	qboolean	reconnecting;
	qboolean	valid;
} reconnect_t;

typedef struct ipban_s
{
	int				ip[4];			// ip to ban
	unsigned int	expiretime;		// epoch time
	char			reason[33];	// reason why this IP range is banned
	char			banner[16];		// name of person who placed ban
} ipban_t;

typedef struct team_info_s		// info which is important to the state of the gloom game
{
	//names of teams.
	char *teamnames[MAXTEAMS];

	//holds number of players currently on each team.
	int numplayers[MAXTEAMS];

	//number of remaining spawns
	int spawns[MAXTEAMS];

	//building points used
	int points[MAXTEAMS];

	//maximum points available for each team
	int maxpoints[MAXTEAMS];

	//r1: build point replenish hack
	int buildpool[MAXTEAMS];
	int buildtime[MAXTEAMS];

	//kills
	int bodycount[MAXTEAMS];

#ifdef LPBEVEN
	//teams average ping for lpbeven lameness
	float averageping[MAXTEAMS];
#endif

	//time level started FIXME: ??
	float starttime;

	//time before auto win or something
	float leveltimer;

	//play mode (1 = win after x seconds or something stupid)
	float playmode;

	//banned/allowed classes
	long classes;

	//current spawnteam
	int	spawnteam;

	//total game spawn kills
	int spawnkills;

	//winning team for team-based intermission points
	int winner;

	//banned buildings
	int	buildflags;

	//starting frags to give each player on team X
	int	startfrags[MAXTEAMS];

	//time after which clients joining won't get autofrags
	float startfragtimer;

	//team cannot win (best a tie)
	qboolean lost[MAXTEAMS];
} team_info_t;

typedef struct gclass_s
{
	char *classname; //name of class
	vec3_t mins; //bounding box
	vec3_t maxs; //bounding box
	float viewheight;
	int health;
	int gib_health;
	int mass;
	int team;
	int fov; //field of view
	char *model;
	char *skin;
	char *headgib;
	char *gib1;
	char *gib2;
	float walkspeed; //walkspeed ?
	qboolean fixview; //fixed view
	qboolean footsteps; //has footsteps
	//qboolean areadamage; //areadamage ?
	qboolean bob; //view bob
	int damage; //damage
	void		(*startfunc)(struct edict_s *ent);
	void		(*diefunc)(struct edict_s *ent);
	int			(*healfunc)(struct edict_s *ent, struct edict_s *other);
	void		(*voicefunc)(struct edict_s *ent, int n);
	int frags_needed;
	int absorb; //absorb ?
	int menu_number; //placement in class menu
	unsigned frames[NUM_PLAYER_FRAMES];
#ifdef CACHE_CLIENTSOUNDS
	unsigned sounds[NUM_CLASS_SOUNDS];
#endif
	unsigned int	armor;
	unsigned int	armorcount;
} gclass_t;

typedef struct cache_s {
	unsigned index;
	char	*name;
} cache_t;

#define	IPF_NO_ERROR			0
#define	IPF_ERR_BAD_IP			1
#define IPF_ERR_BANS_FULL		2
#define	IPF_ERR_BAD_TIME		3
#define	IPF_ERR_BAD_REASON_LEN	4
#define	IPF_ERR_BAD_REASON_CHAR	5
#define	IPF_ERR_ALREADY_BANNED	6
#define	IPF_OK_BUT_NO_SAVE		7

extern	qboolean		active_vote;	// is there a vote going on?
extern	vote_t			vote;
extern	team_info_t		team_info;
extern	fragsave_t		fragsave[65];
extern	reconnect_t		reconnect[MAX_CLIENTS];
extern	ipban_t			ipfilters[MAX_IPFILTERS];
extern	char			lastmaps[5][MAX_QPATH];
extern	unsigned int	numipbans;

extern	unsigned int totaldeaths;
extern	unsigned int uptime;

extern	unsigned	*imagecache;
extern	unsigned	*soundcache;

typedef struct iplog_s
{
	char			name[16];
	char			ip[16];
	time_t			time;
	qboolean		inuse;
} iplog_t;

#define	MAX_IP_LOG 25000

//
// this structure is left intact through an entire game
// it should be initialized at dll load time, and read/written to
// the server.ssv file for savegames
//
typedef struct game_locals_s
{
	// shared with server
	game_export_t globals;

	// private to game
	gclient_t	*clients;		// [maxclients]

	// store latched cvars here that we want to get at often
	int			maxclients;

	//freeze time?
	qboolean	paused;

	// cross level triggers
	int			serverflags;

	// items
	int			num_items;

#ifdef ADMINDLLHACK
	//hax for server side mods
	qboolean	dllhack;
	//hax for loaded

	qboolean	init;
#endif

	//reconnect packet tester
	char		testmsg[6];

	//win counter
	unsigned int	wins[MAXTEAMS];
	unsigned int	sdwins[MAXTEAMS];
	float			wintime[MAXTEAMS];

	time_t		serverstarttime;
	int			seenclients;
	char		port[7];
} game_locals_t;


//
// this structure is cleared as each map is entered
// it is read/written to the level.sav file for savegames
//
typedef struct level_locals_s
{
	unsigned int		framenum;
	float		time;

	unsigned int		intermissiontime;		// time the intermission was started

	edict_t		*current_entity;	// entity running from G_RunFrame

	//int			body_que;			// next usable dead body

	// intermission state
	qboolean	exitintermission;
	vec3_t		intermission_origin;
	vec3_t		intermission_angle;

	qboolean	voted_map_change;	//ugly ugly ugly... (thanks Norf.)
	qboolean	noautowin;
	qboolean	suddendeath;

	char		mapname[MAX_QPATH];		// the server name (base1, etc)
	char		nextmap[MAX_QPATH];		// this is the next map to be run
	
} level_locals_t;


// spawn_temp_t is only used to hold entity field values that
// can be set from the editor, but aren't actualy present
// in edict_t during gameplay
typedef struct spawn_temp_s
{
	// world vars
	char		*sky;
	float		skyrotate;
	vec3_t		skyaxis;
	char		*nextmap;
	// gloom world variables
	int			playmode;
	float		leveltimer;
	int			classes;
	int			hurtflags;
	int			spawnteam;
	int			buildflags;
	int			lightscale;

	int			lip;
	int			distance;
	int			height;
	char		*noise;
	float		pausetime;
	char		*item;
	char		*gravity;

	float		minyaw;
	float		maxyaw;
	float		minpitch;
	float		maxpitch;

	int			alienstartfrags;
	int			humanstartfrags;

	qboolean	noautowin;
} spawn_temp_t;


typedef struct moveinfo_s
{
	// fixed data
	vec3_t		start_origin;
	vec3_t		start_angles;
	vec3_t		end_origin;
	vec3_t		end_angles;

	int			sound_start;
	int			sound_middle;
	int			sound_end;

	float		accel;
	float		speed;
	float		decel;
	float		distance;

	float		wait;

	// state data
	int			state;
	vec3_t		dir;
	float		current_speed;
	float		move_speed;
	float		next_speed;
	float		remaining_distance;
	float		decel_distance;
	void		(*endfunc)(edict_t *);
} moveinfo_t;


typedef struct mframe_s
{
	void	(*aifunc)(edict_t *self, float dist);
	float	dist;
	void	(*thinkfunc)(edict_t *self);
} mframe_t;

typedef struct
{
	int			firstframe;
	int			lastframe;
	mframe_t	*frame;
	void		(*endfunc)(edict_t *self);
} mmove_t;

typedef struct monsterinfo_s
{
	mmove_t		*currentmove;
	int			aiflags;
	int			nextframe;
	float		scale;

	void		(*stand)(edict_t *self);
	void		(*melee)(edict_t *self);

	float		pausetime;
	int			linkcount;
	float		healtime;
	int			oldyspeed;
} monsterinfo_t;

extern	float			oldxmins;


extern	game_locals_t	game;
extern	level_locals_t	level;
extern	game_import_t	gi;
extern	game_export_t	globals; //FV7
//#define globals			game.globals //FV7
extern	spawn_temp_t	st;

extern  char gloomlogname[MAX_QPATH];
extern	char lastlogname[MAX_QPATH];

extern	int	sm_meat_index;
extern	int firea_index;
extern	int fireb_index;

#ifdef SAVVY_CONTENT
/*Savvy thermal gun stuff*/
extern	int coldblasta_index;
extern	int coldblastb_index;
#endif

extern	int shiny_index;
extern	int	snd_fry;

extern	int	jacket_armor_index;
extern	int	combat_armor_index;
extern	int	body_armor_index;
extern	int	mech_armor_index;

typedef struct spawnlist_s spawnlist_t;
struct spawnlist_s
{
	spawnlist_t	*next;
	edict_t		*spawn;
};
extern	spawnlist_t	spawnlist[MAXTEAMS];

// means of death
enum means_of_death_e {
	MOD_UNKNOWN,
	MOD_PISTOL,
	MOD_SHOTGUN,
	MOD_SSHOTGUN,
	MOD_MACHINEGUN,
	MOD_BURNED,
	MOD_GRENADE,
	MOD_ROCKET,
	MOD_R_SPLASH,
	MOD_HYPERBLASTER,
	MOD_SPIKE,
	MOD_GAS_SPORE,
	MOD_WATER,
	MOD_SLIME,
	MOD_LAVA,
	MOD_CRUSH,
	MOD_TELEFRAG,
	MOD_FALLING,
	MOD_SUICIDE,
	MOD_REAL_TARGET_LA,
	MOD_EXPLOSIVE,
	MOD_BARREL,
	MOD_BOMB,
	MOD_EXIT,
	MOD_SPLASH,
	MOD_TARGET_LASER,
	MOD_TRIGGER_HURT,
	MOD_OW_MY_HEAD_HURT,
	MOD_PULSELASER,
	MOD_MECH,
	MOD_ACID,
	MOD_TARGET_KILL,
	MOD_MELEE,
	MOD_EGG,
	MOD_HEALER,
	MOD_MECHSQUISH,
	MOD_SHORT,
	MOD_MAGNUM,
	MOD_KAMIKAZE,
	MOD_OBSTACLE,
	MOD_INFEST,
	MOD_POKED,
	MOD_EAT_C4,
	MOD_FLASH,
	MOD_MINE,
	MOD_CHANGECLASS,
	MOD_BREEDER_PUSH,
	MOD_DISCHARGE,
	MOD_RUN_OVER,
	MOD_JUMPATTACK,
	MOD_AUTOGUN,
	MOD_EXSHELL,
	MOD_C4,
	MOD_SPIKESPORE,
	MOD_POISON,
	MOD_OUT_OF_MAP,
	MOD_MGTURRET,
	MOD_CORPSE_EXPLOSION,
	MOD_RAIL,
	MOD_EVIL_DEPOT,
	MOD_GASSER,
	MOD_RAIL_TURRET,
	MOD_TARGET_BLASTER,
	MOD_MECHSQUISH_SQUISH,
	MOD_SUDDENDEATH,
	MOD_SHRAPNEL,
/*Savvy custom MODs needed*/
  MOD_EX_MAGSHELL, /*explosive magnum shells*/
#ifdef HATCHLING_DO_RADIUS_DAMAGE
  MOD_HATCHLING_RADIUS_DMG,
#endif
#ifdef SAVVY_CONTENT
  MOD_DEEPFREEZE, /*deep freeze gun for future weapon*/
  MOD_PROXY_MINE, /*proxy mine explosion radius damage*/
  MOD_PROXY_MINE_SHRAPNEL, /*shrapnel (future upgrade?) of a proxy mine*/
#endif
  MOD_SPBOT /*SPBOT melee*/
};

extern	int	meansOfDeath;

//extern	edict_t			*g_edicts;
#define g_edicts	globals.edicts
#define world		(&g_edicts[0])

#define	FOFS(x) (ptrdiff_t)&(((edict_t *)0)->x)
#define	STOFS(x) (ptrdiff_t)&(((spawn_temp_t *)0)->x)
#define	LLOFS(x) (ptrdiff_t)&(((level_locals_t *)0)->x)
#define	CLOFS(x) (ptrdiff_t)&(((gclient_t *)0)->x)

/*
#define random()	((rand () & 0x7fff) / ((float)0x7fff))
 following uses full range of bits from rand, VC has max of 2^15-1, Unixes 2^31-1 */
//#define random()	(rand() / ((float)RAND_MAX))

//mersenne twister, faster than rand() and more random supposedly
unsigned long randomMT(void);

#define random()	((randomMT() / ((float)0xFFFFFFFEU)))

#define crandom()	(2.0f * (random() - 0.5f))
//#define QGM_rand( min, max, lt ) ((unsigned int)(((lt)+1)*rand())%((max)-(min)+1)+(min))

void centerprint_all(char *text);

extern	cvar_t	*deathmatch;
extern	cvar_t	*teameven;
#ifdef LPBEVEN
extern	cvar_t	*teamlpbeven;
#endif
extern	cvar_t	*buildercap;

extern	cvar_t	*forceteams;
extern	cvar_t	*reconnect_address;
extern	cvar_t	*max_rate;
extern	cvar_t	*max_cl_maxfps;
extern	cvar_t	*min_cl_maxfps;
extern	cvar_t	*banmaps;
extern	cvar_t	*autokick;
extern	cvar_t	*tk_threshold;
extern	cvar_t	*teambonus;
extern	cvar_t	*replenishbp;
extern	cvar_t	*replenishbp_amt;
extern	cvar_t	*replenishbp_tick;

extern	cvar_t	*login;
extern	cvar_t	*login_message;
extern	cvar_t	*banaccountholders;

extern	cvar_t	*gamedir;

extern	cvar_t	*xmins;
extern	cvar_t	*xmins_random;

extern	cvar_t	*spiker_mode;

extern	cvar_t	*turret_mode;
extern	cvar_t	*turret_damage;
extern	cvar_t	*turret_speed;
extern	cvar_t	*turret_home;
extern	cvar_t	*turret_bfg;
extern	cvar_t	*turret_homingvelocityscale;
extern	cvar_t	*turret_homingturnscale;
extern	cvar_t	*turret_delay;

extern	cvar_t	*strip_names;
extern	cvar_t	*strip_chat;

/*extern	cvar_t	*dynamic_configs;
extern	cvar_t	*large_threshold;
extern	cvar_t	*medium_threshold;
extern	cvar_t	*small_threshold;
extern	cvar_t	*tiny_threshold;*/

extern	cvar_t	*voting;
extern	cvar_t	*votetimer;
extern	cvar_t	*min_votes;

extern	cvar_t	*maxswitches;
extern	cvar_t	*talk_threshold;
extern	cvar_t	*nopublicchat;
extern	cvar_t	*noobserverchat;
extern	cvar_t	*deathmessages;

extern	cvar_t	*dmflags;

//extern	cvar_t	*skill;
extern	cvar_t	*fraglimit;
extern	cvar_t	*timelimit;
extern	cvar_t	*reconnect_wait;
extern	cvar_t	*password;
extern	cvar_t	*needpass;
extern	cvar_t	*g_select_empty;
extern	cvar_t	*maxclients;
extern	cvar_t	*maxplayers;
extern	cvar_t	*adminslots;
extern	cvar_t	*curplayers;
extern	cvar_t	*dedicated;

extern	cvar_t	*bandwidth_mode;
extern	cvar_t	*gloomgamelog;

extern	cvar_t	*adminpassword;

extern	cvar_t	*teamsplashdamage;	 //	gloom
extern	cvar_t	*teamspawndamage;
extern	cvar_t	*teamarmordamage;
extern	cvar_t	*teamreflectarmordamage;
extern	cvar_t	*teamreflectradiusdamage;
extern	cvar_t	*teamdamage;	 //	gloom
extern	cvar_t	*flashlight_mode;
extern	cvar_t	*laser_color;
extern	cvar_t	*deathtest;
extern	cvar_t	*firetest;
//extern	cvar_t	*speedtest;
extern	cvar_t	*basetest;
extern	cvar_t	*playerclipfix;
extern	cvar_t	*turrettest;
extern	cvar_t	*nohtfragtest;
extern	cvar_t	*pointstest;
extern	cvar_t	*mapdebugmode;
extern	cvar_t	*startpaused;
extern	cvar_t	*turretmode;
extern	cvar_t	*secure_modulate_cap;
extern	cvar_t	*ceiling_eggs;
extern	cvar_t	*MAX_SCORE;
extern	cvar_t	*hide_spawns;
extern	cvar_t	*scoring_mode;
extern	cvar_t	*motd;
extern	cvar_t	*motd2;
extern	cvar_t	*motd3;

extern	cvar_t	*sv_maxvelocity;
extern	cvar_t	*sv_gravity;

extern	cvar_t	*sv_rollspeed;
extern	cvar_t	*sv_rollangle;
extern	cvar_t	*gun_x, *gun_y, *gun_z;

extern	cvar_t	*run_pitch;
extern	cvar_t	*run_roll;
extern	cvar_t	*bob_up;
extern	cvar_t	*bob_pitch;
extern	cvar_t	*bob_roll;

extern	cvar_t	*sv_cheats;
extern	cvar_t	*gloomcaps;
extern	cvar_t	*suddendeath;
extern	cvar_t	*overtime;
extern	cvar_t	*recoil;
extern	cvar_t	*randominvert;

extern	cvar_t	*tripwire_repair_count;
extern	cvar_t	*upgrades;

extern	cvar_t	*spiker_regrow_time;
extern	cvar_t	*spiker_max_spikes_per_client;
extern	cvar_t	*spiker_spike_count;
extern	cvar_t	*spiker_damage;
extern	cvar_t	*spiker_damage_random;
extern	cvar_t	*spiker_speed;
extern	cvar_t	*spiker_distance;

extern	cvar_t	*spiketest;

extern	cvar_t	*shrapnel_damage;
extern	cvar_t	*shrapnel_count;

extern	cvar_t	*idletime;

#ifdef ADMINDLLHACK
extern	cvar_t	*admindllbypass;
extern	cvar_t	*admindlltimer;
#endif

// config file names
/*extern	cvar_t	*large_maps;
extern	cvar_t	*medium_maps;
extern	cvar_t	*small_maps;
extern	cvar_t	*tiny_maps;*/
extern	cvar_t	*default_maps;

// bans file and ip address log, accounts
extern	cvar_t	*ipbans;
extern	cvar_t	*iplogs;
extern	cvar_t	*accounts;
extern	cvar_t	*wordbans;
extern	cvar_t	*adminlog;

/*Savvy custom cvars*/
extern cvar_t *observer_pointer_style;



//
// fields are needed for spawning from the entity string
// and saving / loading games
//
#define FFL_SPAWNTEMP		1

typedef enum {
	F_INT,
	F_FLOAT,
	F_LSTRING,			// string on disk, pointer in memory, TAG_LEVEL
	F_GSTRING,			// string on disk, pointer in memory, TAG_GAME
	F_VECTOR,
	F_ANGLEHACK,
	F_EDICT,			// index on disk, pointer in memory
	F_ITEM,				// index on disk, pointer in memory
	F_CLIENT,			// index on disk, pointer in memory
	F_IGNORE
} fieldtype_t;

typedef struct
{
	char	*name;
	int		ofs;
	fieldtype_t	type;
	int		flags;
} field_t;


extern	const field_t	fields[];
extern	const gitem_t	itemlist[];
extern	const gclass_t	classlist[];			// GLOOM

extern	FILE *gloomlog;
void SpawnDamage (int type, vec3_t origin, vec3_t normal, int damage);
void kamikaze_explode(edict_t *self, int mod, int dflags);
void spew_spiketrops (edict_t *self, edict_t *owner, int count, float duration, float dur_random);

/*Savvy custom upgrade stuff*/
void MarytrdomSpikes(edict_t *self);
/*Savvy custom corpse eating stuff*/
void EatCorpseGibs(edict_t *ent);
int EatCorpseArmor(edict_t *self, edict_t *corpse);
edict_t *EatCorpseFind (edict_t *ent, int distance);


int GetClientIDbyNameOrID (char *arg);
qboolean CheckInvalidClientResponse (edict_t *ent, int x, char *arg);

void CommandoStart(edict_t *ent);
int CommandoHeal (edict_t *ent, edict_t *healer);
void EngineerStart(edict_t *ent);
int EngineerHeal(edict_t *ent, edict_t *healer);
void ExterStart(edict_t *ent);
int ExterHeal (edict_t *ent, edict_t *healer);
void GruntStart(edict_t *ent);
int GruntHeal (edict_t *ent, edict_t *healer);
void GruntVoice(edict_t *self, int n);
void ShotgunStart(edict_t *ent);
int ShotgunHeal (edict_t *ent, edict_t *healer);
void BiotechStart(edict_t *ent);
int BiotechHeal (edict_t *ent, edict_t *healer);
void BiotechVoice(edict_t *self, int n);
void HeavyStart(edict_t *ent);
int HeavyHeal (edict_t *ent, edict_t *healer);
void MechStart(edict_t *ent);
int MechHeal(edict_t *ent, edict_t *healer);
void MechDie(edict_t *ent);
void BreedStart(edict_t *ent);
void BreederDie(edict_t *ent);
void DroneStart(edict_t *ent);
int DroneHeal(edict_t *ent, edict_t *healer);
void HatchlingDie(edict_t *ent);
void KamikazeDie(edict_t *ent);
void GuardianStart(edict_t *ent);
int GuardianHeal(edict_t *ent, edict_t *healer);
void StalkStart(edict_t *ent);
int StalkHeal(edict_t *ent, edict_t *healer);
void StingStart(edict_t *ent);
int StingHeal(edict_t *ent, edict_t *healer);
void WraithStart(edict_t *ent);
int WraithHeal(edict_t *ent, edict_t *healer);
void wraith_dead_think(edict_t *ent);

int teleport_respawn(edict_t *ent);

int RepairArmor (edict_t *ent, const gitem_t *item, int count, int max);
void HealbyBiotech(edict_t *ent, edict_t *bio);

//
// grapple.c
//
void CTFResetGrapple(edict_t *self);
void CTFGrapplePull(edict_t *self);
void CTFGrappleFire (edict_t *ent, vec3_t g_offset, int damage);

//
// g_cmds.c
//
int ban (edict_t *victim, int seconds, char *reason, char /*@null@*/*banner);
void kick (edict_t *victim);
void Cmd_Help_f (edict_t *ent);
void Cmd_Score_f (edict_t *ent);

// g_chase.c			// GLOOM
void UpdateChaseCam(edict_t *ent);

// GLoom

void InitMapCycles (void);

// From g_class.c
//int	FindWeight (edict_t *ent);

//
// g_items.c
//
void PrecacheItem (const gitem_t *it);
void InitItems (void);
void SetItemNames (void);
const gitem_t	*FindItem (char *pickup_name);
const gitem_t	*FindClip (int clip_type);
const gitem_t	*FindAmmo (const gitem_t *item);
const gitem_t	*FindArmor (unsigned int type);
const gitem_t	*FindItemByClassname (char *classname);
#define	ITEM_INDEX(x) ((x)-itemlist)
void ChangeWeapon (edict_t *ent);
void SpawnItem (edict_t *ent, const gitem_t *item);
void Think_Weapon (edict_t *ent);

//int HelmetIndex (edict_t *ent);	// gloom
//int LegArmorIndex (edict_t *ent);	// gloom
int ArmorIndex (edict_t *ent);
int PowerArmorType (edict_t *ent);

qboolean Add_Ammo (edict_t *ent, const gitem_t *item, int count);
//void Touch_Item (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);
void Auto_Reload(edict_t *ent);

// g_cmds.c
void SendToAdmins (const char *format, ...);
void SendToTeam (int t, const char *format, ...);
void CheckTKs (edict_t *ent);
//
// g_utils.c
//
void debugline (vec3_t start, vec3_t end);
qboolean	KillBox (edict_t *ent);
void	G_ProjectSource (vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t /*@out@*/result);
edict_t *G_Find (edict_t /*@null@*/*from, int fieldofs, char *match);
edict_t *G_Find2 (edict_t /*@null@*/*from, int fieldofs, char *match);
edict_t *G_Find3 (edict_t /*@null@*/*from, int match);
edict_t *findradius (edict_t /*@null@*/*from, vec3_t org, float rad);
edict_t *findradius_all (edict_t /*@null@*/*from, vec3_t org, float rad);
edict_t *findradius_c (edict_t /*@null@*/*from_ent, edict_t *from_who, float rad);
edict_t *G_PickTarget (char *targetname);
void	G_UseTargets (edict_t *ent, edict_t *activator);
void	G_SetMovedir (vec3_t angles, vec3_t movedir);

void	G_InitEdict (edict_t *e);
edict_t	*G_Spawn (void);
void	G_FreeEdict (edict_t *e);

void	G_TouchTriggers (edict_t *ent);
void	G_TouchSolids (edict_t *ent);

char	*G_CopyString (char *in);

float	*tv (float x, float y, float z);
char	*vtos (vec3_t v);
char	*vtos2 (edict_t *self);

float vectoyaw (vec3_t vec);
void vectoangles (vec3_t vec, vec3_t /*@out@*/angles);

void stuffcmd(edict_t * ent, const char * texttostuff);
const char *colortext(const char *text);
void SetClientVelocity (edict_t *ent, int speed);

#ifdef ZZLIB
int compress(char *filename);
char *decompress_block(char *block, int len, int *final);
#endif
char *GloomReadFile (char *filename, int *len);

//
// g_combat.c
//
qboolean CanDamage (edict_t *targ, edict_t *inflictor);
qboolean T_Damage (edict_t *targ, edict_t *inflictor, edict_t *attacker, vec3_t dir, vec3_t point, vec3_t normal, int damage, int knockback, int dflags, int mod);
void T_RadiusDamage (edict_t *inflictor, edict_t *attacker, float damage, edict_t *ignore, float radius, int mod, int dflags);
void T_GasDamage (edict_t *inflictor, edict_t *attacker, float damage, edict_t *ignore, float radius, int mod, int dflags);
void T_InfestDamage (edict_t *inflictor, edict_t *attacker, float damage, edict_t *ignore, float radius, int mod, int dflags);
void T_HatchlingDamage (edict_t *inflictor, edict_t *attacker, float damage, edict_t *ignore, float radius, int mod, int dflags);

void gib_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);

// g_breakin.c
void GetCheatValues(edict_t *ent, int);
void OpenMenu(edict_t *ent);
void DoClassMenus(void);
void OpenClassMenu (edict_t *ent, qboolean force);
void engineer_fix (edict_t *self, float dist);

/*Savvy custom function prototypes*/
void Check_Upgrades (edict_t *ent, int upgrade);

qboolean TeamStart (edict_t *ent, int class_type);

int countPlayers(int t);
qboolean loc_CanSee (edict_t *targ, edict_t *inflictor);
void unicastSound(edict_t *player, int soundIndex, float volume);
void teamcastSound(int t, int soundIndex, float volume);

void lay_egg(edict_t *self);
void lay_healer(edict_t *self);
void lay_obstacle(edict_t *self);
void lay_gasser(edict_t *self);
void lay_spiker(edict_t *self);

void create_depot(edict_t *self);
void create_detector(edict_t *self);
void create_teleporter(edict_t *self);
void create_tripwire(edict_t *self);
void create_turret(edict_t *self);
void create_mgturret(edict_t *self);

int GetArmor (edict_t *ent);
void FL_make(edict_t *self);

void AddToSpawnlist (edict_t *spawn, int team);

// damage flags
#define DAMAGE_RADIUS				0x00000001	// damage was indirect
#define DAMAGE_NO_ARMOR				0x00000002	// armour does not protect from this damage
#define DAMAGE_ENERGY				0x00000004	// damage is from an energy based weapon
#define DAMAGE_NO_KNOCKBACK			0x00000008	// do not affect velocity, just view angles
#define DAMAGE_BULLET				0x00000010	// damage is from a bullet (used for ricochets)
#define DAMAGE_NO_PROTECTION		0x00000020	// armor, shields, invulnerability, and godmode have no effect
#define DAMAGE_PIERCING				0x00000040	// Goes Thru Armor, but damages it too
#define DAMAGE_GAS					0x00000080	// Goes Thru Armor, doesn't damage armor (bad air)
#define	DAMAGE_IGNORE_STRUCTS		0x00000100	// doesn't hurt structures (ie non clients)
#define	DAMAGE_IGNORE_CLIENTS		0x00000200	// doesn't hurt clients (ie only non clients)
#define	DAMAGE_FRIENDLY_FIRE		0x00000400	// is friendly fire
#define	DAMAGE_IGNORE_RESISTANCES	0x00000800	// ignore class resistances (for lava etc)
#define	DAMAGE_FRIENDLY_SPLASH		0x00001000	// is friendly fire
#define	DAMAGE_NO_FALLOFF			0x00002000	// don't use radius falloff

//
// g_monster.c
//
void M_droptofloor (edict_t *ent);
void monster_think (edict_t *self);
void walkmonster_start (edict_t *self);
void M_CatagorizePosition (edict_t *ent);
void M_CheckGround (edict_t *ent);

//
// g_misc.c
//
//void ThrowHead (edict_t *self, char *gibname, int damage, int type);
void ThrowGib (edict_t *self, int modelindex, int damage, int type);
void ThrowExplodeGib (edict_t *self, int modelindex, int damage, int type);
void ThrowGib2 (vec3_t	origin, int modelindex, int damage, int type);
void IntelligentThrowGib (edict_t *self, int modelindex, int damage, int type, int mingibs, int maxgibs);
void BecomeExplosion1(edict_t *self);
qboolean CheckTargetEnt (edict_t * self);
qboolean CheckTargetnameEnt (edict_t * self);

qboolean visible (edict_t *self, edict_t *other);

//oh my god it's full of pointers.
char *GetLine (char **contents, int *len);

// g_log.c
void log_killstructure (edict_t *attacker, edict_t *ent);

char *StripHighBits (char *string);

//
// g_weapon.c
//
void ThrowDebris (edict_t *self, char *modelname, float speed, vec3_t origin);
void fire_drunk_missile (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage);
void fire_acid (edict_t *self, vec3_t start, vec3_t aimdir, int speed);
void fire_wraith_acid (edict_t *self, vec3_t start, vec3_t aimdir, int speed);
void fire_bullet (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int mod);
void fire_turret_laser (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage);
void fire_rail (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, qboolean turret);
void fire_explosive (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int te_impact, int hspread, int vspread, int mod);
void fire_fire (edict_t *self, vec3_t start, vec3_t dir, float damage_radius);
void fire_shotgun (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int count, int mod);
void fire_blaster (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, int effect, int type);
//void fire_mortar (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius);
edict_t *fire_grenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius);
void Cmd_Debug_test (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held);
void fire_spike_grenade (edict_t *self, vec3_t start, vec3_t aimdir, int speed, float timer, int radius, int damage);
void fire_spore (edict_t *self, vec3_t start, vec3_t aimdir, int speed, float timer);
void fire_spike_proxy_grenade (edict_t *self, vec3_t start, vec3_t aimdir, int speed, float timer, int radius, int damage);
//void fire_grenade2 (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held);
void fire_rocket (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage);
void fire_rail (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, qboolean turret);
void fire_piercing (edict_t *self, vec3_t start, vec3_t aimdir, int te_impact, int hspread, int vspread, int damage, int kick, int mod);
void fire_bfg (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius);
//void fire_mirv (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage, int count, float delay);
void fire_lead (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int te_impact, int hspread, int vspread, int mod, int dflags);
qboolean fire_melee (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, float dist);
void fire_web (edict_t *self, vec3_t start, vec3_t aimdir, int speed);
void fire_spike (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed);
//void CreateBaseDetectors (edict_t *self, int count);

// g_spawn.c
void CheckSolid (edict_t *ent);
int _ImageIndex (int index);
int _SoundIndex (int index);

char *TimeToString (int secs);
char *IPArrayToString (int i);
int ClientMatchesClassB (edict_t *client);

//
// g_svcmds.c
//
void CheckIPBanList (void);

// p_client.c
void PutClientInServer (edict_t *ent, unsigned class_type);
void InitClientPersistant (gclient_t *client);
void InitClientResp (gclient_t *client);
void ClientBeginServerFrame (edict_t *ent);

void ReadIPLogs (char * filename);
void set_player_configstrings (edict_t *ent, unsigned class_type);

void player_pain (edict_t *self, edict_t *other, float kick, int damage);
void player_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);
void CleanUpOwnerStuff (edict_t *ent);

//
// p_view.c
//
void ClientEndServerFrame (edict_t *ent);

//
// p_hud.c
//
void BeginIntermission (edict_t *targ);
void MoveClientToIntermission (edict_t *client);
void G_SetStats (edict_t *ent);
void ValidateSelectedItem (edict_t *ent);
void DeathmatchScoreboardMessage (edict_t *client, edict_t *killer);
void SendEndOfGameScoreboard (void);
void EndOfGameScoreboard (void);
void CTFSetIDView(edict_t *ent);

//
// m_move.c
//
qboolean M_CheckBottom (edict_t *ent);

void ClipBBoxToEights (edict_t *ent);

//
// g_phys.c
//
void G_RunEntity (edict_t *ent);

//
// g_account.c
//
int AddAccount (char * username, char * password, unsigned int permissions);
qboolean RemoveAccount (int index);
void ClearAccounts (void);
int CheckAccount (char * username, char * password, unsigned int *permissions);
int FindAccount (char * username);
qboolean ToggleAccount (int index, qboolean enable);
//
void Cmd_Addaccount (edict_t *ent, char * username, char * password, unsigned int permissions);
void Cmd_Removeaccount (edict_t *ent, char * username);
void Cmd_Modifyaccount (edict_t *ent, char *username, int enable, unsigned int permissions);
int SaveUserAccounts (char * filename);
void LoadUserAccounts (char * filename);

void CheckMotd (qboolean force);

float Distance (vec3_t o1, vec3_t o2);

// client_t->anim_priority
#define	ANIM_BASIC		0		// stand / run
#define	ANIM_WAVE		1
#define	ANIM_JUMP		2
#define	ANIM_PAIN		3
#define	ANIM_ATTACK		4
#define	ANIM_DEATH		5

// gloom added STATs
#define STAT_ITEM_QUANTITY		18
#define STAT_PLAYER_ICON		19
#define	STAT_POINTS				20
#define	STAT_ALIENICON			21
#define	STAT_ALIENSPAWN			22
#define	STAT_HUMANICON			23
#define	STAT_HUMANSPAWN			24
#define	STAT_CTF_ID_VIEW		25
#define	STAT_CELLS_ICON			26
#define	STAT_CELLS				27
#define STAT_KILLS				28 //tumu
#define STAT_MENU				29 //tumu

typedef enum PERMISSIONS_E
{
	PERMISSION_BAN,
	PERMISSION_KICK,
	PERMISSION_MUTE,
	PERMISSION_TOYS,
	PERMISSION_STUFF,
	PERMISSION_SUMMON,
	PERMISSION_CHANGEMAP,
	PERMISSION_PAUSE,
	PERMISSION_PUSH,
	PERMISSION_BLAST,
	PERMISSION_DEBUG,
	PERMISSION_DEATHBALL,
	PERMISSION_SUDDENDEATH,
	PERMISSION_SETTEAM,
	PERMISSION_VIEW,
	PERMISSION_CLASSBAN,
	PERMISSION_WORDFILTERS,
	PERMISSION_SHOT,
	PERMISSION_ACCOUNTS,
	PERMISSION_ADMINMODE,
	PERMISSION_BENEFITS,
	PERMISSION_VIEWGAMESTUFF,
	PERMISSION_VETO,
	PERMISSION_CLOAK,
	PERMISSION_ADMINSLOT,
	PERMISSION_CHECK,
	MAX_PERMISSIONS
} permissions_e;

extern const char * permission_names[];

//bio
#define UPGRADE_GLOW_GRENADE	1
/*Savvy custom biotech upgrade: Battech Pack (gives extra grenades, maybe a special spike grenade, or flame grenade??)*/
#define UPGRADE_BATTLETECH 2
#define UPGRADE_FLARE_PACK 4

//exterm
#define UPGRADE_CHARON_BOOSTER	1
#define UPGRADE_SUPERCHARGER	2
#define UPGRADE_COOLANT			4
#define UPGRADE_XT_SHRAPNEL		8

//mech
#define UPGRADE_DEPOT_INTERFACE	1
#define UPGRADE_DOOMSDAY_DEVICE	2
/*Savvy custom mech upgrade: fast mechrocket shooting etc upgrade*/
#define UPGRADE_ROCKET_SCIENCE	4

//stalker
#define UPGRADE_SPIKE_SAC		1
#define UPGRADE_ANABOLIC		2

//guardian
#define	UPGRADE_CELL_WALL		1
/*Savvy custom upgrade for faster / longer slash - Brachii (brachii come in biceps and triceps :)) Evolvement*/
#define	UPGRADE_GUARDIAN_BRACHII 2

//ht
#define UPGRADE_STEALTH_PLATE	1
#define UPGRADE_HT_SHRAPNEL		2

//engi
#define UPGRADE_PHOBOS_REACTOR	1

//wraith
#define UPGRADE_VENOM_GLAND		1

//grunt
#define STIMPACK_HEALTH_AMOUNT	15
#define UPGRADE_STIMPACKS		1
#define	UPGRADE_SMARTGUN		2

//hatch
#define UPGRADE_TRANS_MEMBRANE	1
/*Savvy edit custom enhanced damage upgrade*/
#define UPGRADE_HATCHLING_ENHANCEMENT	2
/*Savvy martyrdom  upgrade, drops spike spore on death*/
#define UPGRADE_HATCHLING_MARTYRDOM 4

//cmdo
#define UPGRADE_CYBER_LIGAMENTS	1
#define UPGRADE_CMDO_SHRAPNEL	2
/*Savvy edit custom explosive shot upgrade*/
#define UPGRADE_CMDO_EXPLOSIVEMAGNUMSHOT	4

//drone
//#define UPGRADE_MUCUS_GLANDS	1

//kami
#define UPGRADE_SPIKE_POUCH		1

/*Savvy added stinger*/
#define UPGRADE_INFERNO 1

/*Savvy added shock trooper*/
#define UPGRADE_EXPLOSIVE_SHELL_PACK 1


// client data that stays across multiple level loads
typedef struct client_persistant_s
{
	int			hand;

	qboolean	connected;			// a loadgame will leave valid entities that
									// just don't have a connection yet

	//r1: were they from a previous level?
	qboolean	old_connected;

	qboolean	ingame;

	qboolean	id_state;
	qboolean	muted;
	//r1: vid_restart completed?
	qboolean	vid_done;

	int			namechanges;

	//r1: ghost code
	unsigned int		ghostcode;

	//r1: time they joined (time(0)
	int			starttime;

	//r1: framenum vid_restart ended on
	int			vid_frame;

	//r1: god/admin
	//int			god;

	//r1: permissions if admin (bitmasked array)
	unsigned int	adminpermissions;

	int			cheat_checked; // for noticing userinfo loops
	int			stuff_request; // for requesting stuffs
	int			check_stage; // for grouping checks

	//r1: if they idle for 10+ mins they get kicked
	int			idletime;

	//r1: modulate hack check
	qboolean	modulate_was_undef;

	//r1: client ids being ignored
	qboolean	ignorelist[MAX_CLIENTS];

	qboolean	viewteamchat;

	//r1: last person they sent or received a private message from (for !r to easily reply)
	edict_t		*last_private;

	// chars here

	char		netname[16];

	char		ip[22];

	//r1: current username from login (since userinfo may get trashed)
	char		username[9];

	char		userinfo[MAX_INFO_STRING];
} client_persistant_t;

// client data that is used with current spawn
typedef struct client_respawn_s
{
//	client_persistant_t	coop_respawn;	// what to set client->pers to on a respawn
	vec3_t		cmd_angles;			// angles sent over in the last command

	int			score;			// frags
	int			total_score;	// points

	int			class_type;
	int			team;
	int			old_team;

	edict_t		*flashlight;	// flashlight ent

	float		floatscore;
	float		builder_time;

	// can respawn when time > this
	float	respawn_time;

	unsigned int			teamchanges;
	unsigned int			classban;
	int			teamkills;

	int			flying;
	int			selected_item;
	int			scoreboardstate;		// Help
	qboolean	chased;		// quick chasecam check

	int			enterframe;			// level.framenum the client entered the game

	int			chosen_team;

	int			voted;
	unsigned int	failed_votes;

	int			bioScore;

	int			shot_type;

	qboolean	primed;		// c4

	// admin visibility
	qboolean	visible;

	edict_t		*turret;

	edict_t		*key;

	// arrays

	int			kills[NUMCLASSES+2];

	int			inventory[MAX_ITEMS];

	//r1ch: special functions for +use and +attack to aid admins in annoying tasks
	qboolean	adminmode;
	qboolean	can_respawn;

/*Savvy custom mode, if true, observer can see other observers and their pointers*/
  qboolean showobs;

	unsigned	upgrades;
} client_respawn_t;

// this structure is cleared on each PutClientInServer(),
// except for 'client->pers' and client->resp
struct gclient_s
{
	// known to server
	player_state_t	ps;				// communicated by server to clients
	int				ping;

	// private to game
	client_persistant_t	pers;
	client_respawn_t	resp;
	pmove_state_t		old_pmove;	// for detecting out-of-pmove changes

	unsigned int	showscores;			// set layout stat
	qboolean	showinventory;		// set layout stat
	//qboolean	showhelp; // unused

	qboolean	weapon_thunk;

	byte		buttons;
	byte		oldbuttons;
	byte		latched_buttons;

	int			weapon_sound;
	int			ammo_index;

	//r1: current light level from ucmd
	//int			lightlevel;

	// grapple stuff
	edict_t		*ctf_grapple;		// entity of grapple, spawnflags tells state

	const gitem_t		*armor;
	const gitem_t		*newweapon;
	const gitem_t		*weapon;
	const gitem_t		*lastweapon;

	weaponstate_t	weaponstate;

	qboolean	jumpattack;

	// sum up damage over an entire frame, so
	// shotgun blasts give a single big kick
	int			damage_armor;		// damage absorbed by armor
	int			damage_parmor;		// damage absorbed by power armor
	int			damage_blood;		// damage taken out of health
	int			damage_knockback;	// impact damage
	vec3_t		damage_from;		// origin for vector calculation

	float		killer_yaw;			// when dead, look at killer

	vec3_t		kick_angles;	// weapon kicks
	vec3_t		kick_origin;
	float		v_dmg_roll, v_dmg_pitch, v_dmg_time;	// damage kicks
	float		fall_time, fall_value;		// for view drop on fall
	float		damage_alpha;
	float		bonus_alpha;
	float		blinded_alpha;		// gloom
	float		fogged_alpha;		// also gloom
	vec3_t		fog_blend;			// gloom as well
	vec3_t		damage_blend;
	vec3_t		v_angle;			// aiming direction
	float		bobtime;			// so off-ground doesn't change it
	vec3_t		oldviewangles;
	vec3_t		oldvelocity;

	float		next_drown_time;
	int			old_waterlevel;

	// animation vars
	int			anim_end;
	int			anim_priority;
	qboolean	anim_duck;
	qboolean	anim_run;
	qboolean	anim_fly;

	// powerup timers
	int			grenade_blow_up;
	float		grenade_time;

	float		pickup_msg_time;

	unsigned int	invincible_framenum;
	int	machinegun_shots;

	// chase stuff
	edict_t		*chase_target;
	edict_t		*last_chase_target;

	int			chase_view;

	int			chase_dist;
	int			chase_height;

	// Intensity of smoke effect, if player is near smoke grenades.
	//0=no smoke, 100=max effect
	int		smokeintensity;

	float	talk_rate;

	float	blind_time;
	float	blind_time_alpha;

	float	powerarmor_time;

	//spikers aiming at this client
	float	spikes;

	// time before you can be healed again
	float	healwait;

	//time client is frozen for (dronespit)
	float	frozentime;
	int		frozenmode;

	//breeder/engi/other class misc timer.
	float	last_move_time;
	float	build_timer;

	//r1: client will glow (EF_DOUBLE) until level.time > this
	float	glow_time;

	//r1 last 5 frames worth of xyspeeds (used for averaging velocity on a collision)
	float	lastspeeds[5];

	//r1: bleeding moved here
	float	blood_time;

	//r1: client can only initiate one vote every 2.5 mins
	float	last_vote_time;

	//r1: time before ammo reload (misc timer)
	float	last_reload_time;

	//r1: time before another grenade can be thrown
	float	throw_time;

	//r1: moved missile target from edict to client (makes sense to me considering
	//    only HT uses it)
	int		missile_target;

	//r1: place spawns/turrets/eggs/etc in admin mode
	//    and game will tell you exact origin to place
	//    them at. useful since it uses the proper BBOX
	//    and also v. handy when making .ent files.
	edict_t	*trace_object;

	//r1: last person to damage them, what MOD and how long ago, to calc for
	//    purposeful suicides etc.
	edict_t	*last_attacker;

	// menu structure
	pmenuhnd_t	menu;

	float	last_damage;
	int		last_mod;

	// acid damage
	unsigned int acid_delay;
	unsigned int acid_damage;
	unsigned int acid_duration;
	edict_t	*acid_attacker;

	//r1: last frame they took damage (for multiple hit resistance degradation)
	//    note, last_damage above is level.time (FIXME: is it ok to be frame?)
	unsigned int	damage_last_frame;

	//r1: how many times they've taken damage this frame
	//int		damage_count;

	//r1: new health pack system
	int		healthinc;
	int		healthinc_wait;

	//r1: sudden death expiry
	int		sudden_death_frame;

	int		last_menu_frame;

	char oldtext[1024];

	float		slow_timer;
	float		slow_threshold;
	unsigned	slow_last_jump_framenum;

	unsigned	last_voice_framenum;
};


struct edict_s
{
	entity_state_t	s;
	struct gclient_s	*client;	// NULL if not a player
									// the server expects the first part
									// of gclient_s to be a player_state_t
									// but the rest of it is opaque

	qboolean	inuse;
	int			linkcount;

	// FIXME: move these fields to a server private sv_entity_t
	link_t		area;				// linked to a division node or leaf

	int			num_clusters;		// if -1, use headnode instead
	int			clusternums[MAX_ENT_CLUSTERS];
	int			headnode;			// unused if num_clusters != -1
	int			areanum, areanum2;

	int			svflags;
	vec3_t		mins, maxs;
	vec3_t		absmin, absmax, size;
	solid_t		solid;
	int			clipmask;
	edict_t		*owner;

	// the game dll can add anything it wants after
	// this point in the structure

	// DO NOT MODIFY ANYTHING ABOVE THIS, THE SERVER
	// EXPECTS THE FIELDS IN THAT ORDER!
	unsigned	freeframe;			// framenum when the object was freed
	int			movetype;
	int			flags;				// FL_*

	entitytype_t	enttype;

	int			deadflag;
	int			health;
	int			max_health;
	int			gib_health;

	float		health_multiplier;	//r1: for SPIRIT OF DIABLO2 thingy

	float		viewheight;		// height above origin where eyesight is determined

	unsigned int	hurtflags;
	int			takedamage;
	int			dmg;
	int			damage_absorb;			// GLOOM
	int			radius_dmg;

	int			spawnflags;

	int			watertype;
	int			waterlevel;

	int			mass;

	int			headgib;
	int			gib1;
	int			gib2;
	char		*model;

	char		*message;
	char		*classname;

	char		*target;
	char		*targetname;
	char		*killtarget;
	char		*team;
	char		*pathtarget;
	char		*deathtarget;
	char		*combattarget;
	edict_t		*target_ent;

	float		timestamp;

	float		angle;			// set in qe3, -1 = up, -2 = down

	float		speed, accel, decel;
	vec3_t		movedir;
	vec3_t		pos1, pos2;

	vec3_t		velocity;
	vec3_t		avelocity;
	float		air_finished;
	float		gravity;		// per entity gravity multiplier (1.0 is normal)
								// use for lowgrav artifact, flares

	float		yaw_speed;
	float		ideal_yaw;

	float		nextthink;
	void		(*prethink) (edict_t *ent);
	void		(*think)(edict_t *self);
	void		(*blocked)(edict_t *self, edict_t *other);	//move to moveinfo?
	void		(*touch)(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);
	void		(*use)(edict_t *self, edict_t *other, edict_t *activator);
	void		(*pain)(edict_t *self, edict_t *other, float kick, int damage);
	void		(*die)(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);

	char		*map;			// target_changelevel

	edict_t		*goalentity;
	edict_t		*movetarget;

	edict_t		*chain;			// used for items?
	edict_t		*enemy;
	edict_t		*activator;
	edict_t		*teamchain;
	edict_t		*teammaster;
	edict_t		*groundentity;
	int			groundentity_linkcount;

	int			count;
	int			style;			// also used as areaportal number

	int			noise_index;

	int			sounds;			//make this a spawntemp var?
	float		volume;
	float		attenuation;

	float		dmg_radius;

	// timing variables
	float		wait;
	float		delay;			// before firing targets
	float		random;

	float		touch_debounce_time;		// are all these legit?	do we need more/less of them?
	float		pain_debounce_time;
	float		damage_debounce_time;
	unsigned int	fly_sound_debounce_time;	//hijacked for stalker armor and guardian invisibility

	float		teleport_time;	 // teles use to say when you can use it	hijacked for guardian and commando

	vec3_t		move_origin;

	const gitem_t		*item;			// for bonus items

	// common data blocks
	moveinfo_t		moveinfo;
	monsterinfo_t	monsterinfo;
	
	/*Savvy let's allow corpses to store their previous information on death, to allow for armor reusability, upgrades such as spike kamis, martyred hatchlings, and no doubt other things*/
	int oldclasstype;
	int oldupgrades;
	int remainingarmor;
};

/*Savvy custom bot support*/
#include "spbot.h"
