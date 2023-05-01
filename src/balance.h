/*--------------------------------------------------------------------------------
Quake2 Gloom Balance File
Created by Savvy
Use this file for the define statements used for most balancing
Note: I took some of the old #define's created by R1CH (Richard Stanway) and
various other coders that were scatered throughout the source code into this file
Made by Savvy in 2008, since GPL forbids copyright, just note i did this :)
Note: the song that the original gloom trailer used is Dragula by Rob Zombie
--------------------------------------------------------------------------------*/

/*---------------------
START GAMEPLAY SETTINGS
---------------------*/

/* -  - various messages -  - 
SPAWNQUEUEMESSAGE_CENTERPRINT define it for centerprinted spawnqueue messages
   -  - - - - ---- - - - -  -*/

//#define SPAWNQUEUEMESSAGE_CENTERPRINT

/* -  - building rules -  - 
NO_BUILDING_WHEN_OFF_GROUND define it for that annoying (in my opinion) "you must be on the ground to breed" message
NO_BUILDING_WHILE_INVUNERABLE define it to be able to breed while invunerable
  BUILDING_WHILE_INVUNERABLE_KILLS_INVUNERABLE only works if engineering or breeding while invunerable is enabled
NO_BUILDING_INSIDE_CORPSES define it so that you cannot build inside corpses (this was the way it was for the longest time)
   -  -  -  -  ---  -  -  - */

//#define NO_BUILDING_WHEN_OFF_GROUND
//#define NO_BUILDING_WHILE_INVUNERABLE
  #define BUILDING_WHILE_INVUNERABLE_KILLS_INVUNERABLE
//#define NO_BUILDING_INSIDE_CORPSES

/* -  - vote settings -  - 
VOTE_PLAYER_JOIN_TIME time for players to join, before you can start a vote
BAN_VOTE_VICTIM_ON_DISCONNECT if defined, players get banned if disconnecting as the victim of a vote kick, mute, or teamkick
   -  - - - - - - - - -  -*/
#define VOTE_PLAYER_JOIN_TIME 10
#define BAN_VOTE_VICTIM_ON_DISCONNECT


/* -  - upgrade settings -  -
UPGRADE_RANGE is the range for findradius() to find ents nearby for upgrade validation
   -  -  -  -  - - -  -  -  - */

#define UPGRADE_RANGE 150

/* -  - various settings -  - 
GUARDIAN_CANNOT_SEE_ITS_OWN_LIGHT if defined, then guardian cannot see it's (normally red) circular light
NEW_BODY_LUM if defined, uses the newer body lum. by me (Savvy)
SHOW_DEPTH_SLASH_HACK if defined, lines are shown and messages printed for each slash
   -  -  -  -  --  -  -  -  - */

//#define GUARDIAN_CANNOT_SEE_ITS_OWN_LIGHT
#define NEW_BODY_LUM
//#define SHOW_DEPTH_SLASH_HACK

/* -  - build points -  -
building costs and initial teambonus
   -  - - - -  - - - -   -*/
#define		TEAM_POINTS		300

#define		COST_EGG		  40
#define		COST_OBSTACLE	20
#define		COST_SPIKER		30
#define		COST_GASSER		20
#define		COST_HEALER		50

#define		COST_TELEPORT	40
#define		COST_TURRET		60
#define		COST_MGTURRET	40
#define		COST_MINE		  30
#define		COST_DETECTOR	20
#define		COST_DEPOT		60

/*-----------------------
END OF GAMEPLAY SETTINGS
START OF CLASS PROPERTIES
-----------------------*/

/*grouped slash settings*/
#define SLASH_RANGE_DRONE 105
#define SLASH_RANGE_WRAITH 90
#define SLASH_RANGE_STINGER 150
#define SLASH_RANGE_GUARDIAN 180
#define SLASH_RANGE_STALKER_ATTACK 120
#define SLASH_RANGE_STALKER_BITE 125


/*hatchling*/

/*transparent membrane - this is OLD, NOT IN USE*/
//#define HATCHLING_AMMO_TRANS_SPIKE_SPORES 1

/*enhancement*/
#define HATCHLING_ENHANCEMENT_MAX_HEALTH_ADDITION 20
#define HATCHLING_ENHANCEMENT_GRAPPLE_SPEED_MODIFIER 3

/*engineer*/

/*inventory*/
#define ENGI_AMMO_CELLS 50

/*biotech*/
#define BIOTECH_MAX_CELLS 750

/*inventory*/
/*normal*/
#define BIOTECH_AMMO_SCATTERHOT_CLIPS 1
#define BIOTECH_AMMO_FLASH_GRENADES 1
#define BIOTECH_AMMO_FLARES 4
#define BIOTECH_AMMO_9MM_CLIPS 2
#define BIOTECH_AMMO_HEALTH 1
#define BIOTECH_MAX_ACTIVE_FLARES BIOTECH_AMMO_FLARES

/*battletech*/
#define BIOTECH_AMMO_BATTLETECH_SCATTERHOT_CLIPS_ADD 1
#define BIOTECH_AMMO_BATTLETECH_FLASH_GRENADES_ADD 1
#define BIOTECH_AMMO_BATTLETECH_FLARES_ADD 2
#define BIOTECH_AMMO_BATTLETECH_9MM_CLIPS_ADD 1
#define BIOTECH_AMMO_BATTLETECH_HEALTH_ADD 1
/*this is old, we need to find flares for this and others via addition, so this is stupid
#define BIOTECH_BATTLETECH_MAX_ACTIVE_FLARES BIOTECH_AMMO_BATTLETECH_FLARES*/
#define BIOTECH_BATTLETECH_MAX_HEALTH_ADDITION 50

/*flare pack*/
#define BIOTECH_FLARE_PACK_ADD 6

/*drone*/
#define	DRONE_SPIT_MAX	5
#define DRONE_JUMP_HEIGHT_MODIFIER 1.5f

/*wrath*/
/*max spit*/
#define MAX_WRAITH_SPIT 10
/*health at which you can fly, if it goes below, you fall from the sky - NOTE: maximum health and health are stored elsewhere*/
#define WRAITH_MIN_FLY_HEALTH 75

/*mech*/
#define MAX_MECH_ROUNDS      120
#define MECH_REPAIR_DELAY    1
#define MECH_REPAIR_ROUNDS   10
/*DOOMSDAY
OLD_DOOMSDAY_C4 if defined, just makes 1 c4 and detonates, as old
NEW_DOOMSDAY_C4_COUNT only used if not old doomsday; the number of c4's to stack
NEW_DOOMSDAY_C4_INTERVAL only used if not old doomsday; the time, in seconds, between c4 explosions
NEW_DOOMSDAY_C4_DELAY only used if not old doomsday; the time, in seconds, before first c4 detonates
*/
//#define OLD_DOOMSDAY_C4
#define NEW_DOOMSDAY_C4_COUNT 5
#define NEW_DOOMSDAY_C4_INTERVAL 2
#define NEW_DOOMSDAY_C4_DELAY 5




//old #define MECH_MISSILE_COUNT      5
#define MECH_MISSILE_VALUE      10
#define MECH_MISSILE_MAX_QUEUED 5
//#define MECH_CANNOT_SHOOT_ROCKETS_WHILE_MOVING

#define MECH_ROCKET_DIRECT_DAMAGE 10
#define MECH_ROCKET_RADIUS        25
#define MECH_ROCKET_RADIUS_DAMAGE 50


/*stinger*/
#define	MAX_STINGER_GAS_SPORES 2
#define MAX_STINGER_FIRE 15

#define INFERNO_FIRE_MODIFIER_DAMAGE 2
#define INFERNO_FIRE_MODIFIER_RADIUS_DAMAGE 2
#define INFERNO_FIRE_MODIFIER_DAMAGE_RADIUS 2

/*commando*/
#define MAGNUM_AMMO_CONSUMPTION_EXPLOSIVELY_UPGRADED 2
#define MAGNUM_AMMO_CONSUMPTION_NORMAL 1

#define COMMANDO_C4_DROP_ON_DEATH_PROBABILITY 0.5 //5/10 chance that it will drop

/*exterminator*/
#define EXTERM_TAKEDAMAGE_PS_RESTART_DELAY 5
#define EXTERM_TAKEDAMAGE_PS_RESTART_MODIFIER 0.5f

/*guardian*/
#define ABILITY_GUARDIAN_INFEST_RANGE 150

/*stalker
STALKER_REGEN_RATE used to be 60 without define
*/
#define STALKER_REGEN_RATE 40

/*Eat Corpse functionality*/
/*ranges*/
#define STALKER_EATCORPSE_DISTANCE 100

/*overall settings*/
#define EATCORPSE_GIB_MIN_COUNT 2
#define EATCORPSE_GIB_MAX_COUNT 5



/*---------------------------------
     END OF CLASS PROPERTIES
START CLOSE COMBAT DAMAGE MODIFIERS
---------------------------------*/

/*aliens --> humans*/

#define BREEDER_TO_MECH_DAMAGE_MODIFIER           3.0f
#define HATCHLING_TO_MECH_DAMAGE_MODIFIER         1.75f
#define WRAITH_TO_MECH_DAMAGE_MODIFIER            2.0f
#define DRONE_TO_MECH_DAMAGE_MODIFIER             1.2f
#define KAMIKAZE_TO_MECH_DAMAGE_MODIFIER          1.5f
#define STINGER_TO_MECH_DAMAGE_MODIFIER           1.6f
#define GUARDIAN_TO_MECH_DAMAGE_MODIFIER          1.4f
#define STALKER_TO_MECH_DAMAGE_MODIFIER           1.0f

/*humans --> aliens*/

#define ENGINEER_TO_STALKER_DAMAGE_MODIFIER       3.0f
#define GRUNT_TO_STALKER_DAMAGE_MODIFIER          1.75f
#define SHOCK_TROOPER_TO_STALKER_DAMAGE_MODIFIER  1.1f
#define BIOTECH_TO_STALKER_DAMAGE_MODIFIER        1.5f
#define HEAVY_TROOPER_TO_STALKER_DAMAGE_MODIFIER  1.0f
#define COMMANDO_TO_STALKER_DAMAGE_MODIFIER       1.1f
#define EXTERMINATOR_TO_STALKER_DAMAGE_MODIFIER   1.1f
#define MECH_TO_STALKER_DAMAGE_MODIFIER           1.0f

/*class --> nonclient*/
#define GUARDIAN_TO_BUILDING_MODIFIER             3.0f
#define DRONE_TO_BUILDING_MODIFIER                1.0f


/*-------------------------------
END CLOSE COMBAT DAMAGE MODIFIERS
 START OF BUILDINGS AND MAP ENTS
-------------------------------*/

/*turrets*/
//#define HDEF1_ACCURACYTOLERANCE 8
#define HDEF1_VERTICALCAP       35

/*detectors*/
#define DETECTOR_DETECT_DISTANCE 350

/*depots
DEPOT_BEEPING when defined, beeps depot when aliens touch it
*/
#define DEPOT_BEEPING
#define DEPOT_HEALTH 150
//was 300

/*healers / healer ents*/
#define HEAL_NO_HEALTH	1
#define HEAL_NO_ARMOR	  2
// 4, 8, 16, 32, ...

/*healer
HEALER_HEALTH health a healer has at start
HEALER_GIB_HEALTH what is needed to gib
*/
#define	HEALER_HEALTH	120
#define	HEALER_GIB_HEALTH -100

/*spiker
SPIKER_HEALTH health that the spiker starts with
SPIKER_GIB_HEALTH health the spike gibs at
DEFAULT_SPIKER_SPEED the speed at which projectiles fly, if any projectiles
SPIKER_RETRIES number of times a spiker will shoot back, set high to prevent walking by easily

SPIKER_CORNER_RANGE only used if SPIKER_PREDICTION is defined; a float value for distance between spiker and target to fire, regardless of seeing or not

SPIKER_PREDICTION define to allow prediction around walls, this'll help to shoot at humans just as they are coming around the wall
COMMANDO_CAN_PASS_SPIKER define to let commandos crouch and move by spikers (when not IR_VISABLE
NERF_SPIKER define to use old settings
*/
#define	SPIKER_HEALTH 250
#define	SPIKER_GIB_HEALTH -100
#define DEFAULT_SPIKER_SPEED 200
#define SPIKER_RETRIES 5

#define SPIKER_CORNER_RANGE 150.00f

#define SPIKER_PREDICTION
//#define COMMANDO_CAN_PASS_SPIKER
//#define NERF_SPIKER


/*infested corpse
INFEST_HEALTH health of the infested corpse
*/
#define INFEST_HEALTH	250

/*HEAL RATES*/
#define HEALRATE_OBSTACLE 10
#define HEALRATE_OBSTACLE_AFTER_MAX 1
#define HEALRATE_OBSTACLE_HEALTH_CAP 2500

#define HEALRATE_SPIKER 10

#define HEALRATE_HEALER 10

/*-----------------------------
 END OF BUILDINGS AND MAP ENTS
START OF COMMON ITEM PROPERTIES
-----------------------------*/

/*medpacks
place MEDPACK_HALFED_BY_POISEN in a comment to make human medpacks not work, only remove poisen
*/
#define MEDPACK_HALFED_BY_POISEN
#define MEDPACK_VALUE 50

/*various grenades
place FRAG_GRENADE_EMIT_GLOW in a comment to remove the glow around frag grenades
place FRAG_GRENADE_RADIUS_GLOW in a comment to use FRAG_GRENADE_RENDER_FLAGS instead, i perfer using RF_SHELL_GREEN or similar to highlight the grenade
place FRAG_GRENADE_ATTACKABLE in a comment to remove the ability for FRAG_GRENADE_AFFECTED_BY to kill grenades
if this is true, FRAG_GRENADE_HEALTH is used
*/
#define FRAG_GRENADE_EMIT_GLOW
  #define FRAG_GRENADE_RADIUS_GLOW
  #define FRAG_GRENADE_RENDER_FLAGS (RF_SHELL_GREEN | RF_GLOW)
//#define FRAG_GRENADE_ATTACKABLE
#define FRAG_GRENADE_HEALTH 66
#define FRAG_GRENADE_AFFECTED_BY (WC_GRENADE | WC_ROCKET | WC_EXSHELL | WC_C4)
#define FRAG_GRENADE_DAMAGE 120
#define FRAG_GRENADE_DAMAGE_RADIUS 300

/*clips
ROCKETS HAVE *NOT* BEEN TESTED AS OF YET FOR MORE THAN 1 ROCKET PER CLIP - if it works out, you will probably have beep beep boom, beep beep boom, beep beep boom, reload, beep beep boom...
*/
#define CLIP_9MM_VALUE               12
#define CLIP_MAGNUM_VALUE            6 //for even 3 shots with upgrade
#define CLIP_NORMAL_SHELLS_VALUE     5
#define CLIP_EXPLOSIVE_SHELLS_VALUE  5
#define CLIP_SCATTERSHOT_VALUE       12
#define CLIP_AUTOGUN_VALUE           30
#define CLIP_ROCKETS_VALUE            1
#define CLIP_SUBMACHINEGUN_VALUE     60

/*---------------------------
END OF COMMON ITEM PROPERTIES
      START OF GRENADES
---------------------------*/

/*flares
FLARE_TLL time for flare to be alive
FLARE_HEALTH of a flare
FLARE_DETECTION_DISTANCE distance at which guardians are highlighted etc
*/
#define FLARE_TTL 225
#define FLARE_HEALTH 50
#define FLARE_DETECTION_DISTANCE 350

/*shock grenades
SHOCK_GRENADE_ALIEN_GLOW_TIME time an alien is highlighted
*/
#define SHOCK_GRENADE_ALIEN_GLOW_TIME 150

/*guardian spore / proxy spore / spiker dies / wraith special (was an idea)
SPORE_PROXY_HEALTH health of the spore that humans/aliens can destroy
SPORE_PROXY_SPIKETEST_TTL the time to live when 'spiketest' cvar is set
SPORE_PROXY_CHECKDELAY nexthink delay time, in seconds for checking for nearby humans
SPORE_PROXY_DETECT_RADIUS radius for finding humans, thus showering spike trops
SPORE_PROXY_MIN_VELOCITY minimum velocity for the proxy spore when tossed
SPORE_PROXY_VELOCITY_MODIFIER random number * this
SPORE_PROXY_AVELOCITY not sure
*/
#define SPORE_PROXY_HEALTH        20
#define SPORE_PROXY_SPIKETEST_TTL 30
#define SPORE_PROXY_CHECKDELAY    2
#define SPORE_PROXY_DETECT_RADIUS 150

#define SPORE_PROXY_MIN_VELOCITY      200
#define SPORE_PROXY_VELOCITY_MODIFIER 10.0
#define SPORE_PROXY_AVELOCITY         300

/*spike caltrop
SPIKE_TROP_HEALTH health of a single trop
SPIKE_TROP_VELOCITY_MODIFIER random number * this
SPIKE_TROP_ZGROUND_VELOCITY_MODIFIER random number * this - only for axis 2 (z axis?) when ent is a ground entity
*/
/*spike caltrop properties*/
#define SPIKE_TROP_HEALTH                    20
#define SPIKE_TROP_VELOCITY_MODIFIER         250
#define SPIKE_TROP_ZGROUND_VELOCITY_MODIFIER 450


/*stalker / whatever spike spore
SPORE_SPIKE_MIN_VELOCITY minimum velocity for tossing a spike spore
SPORE_SPIKE_MODIFIER random number * this
SPORE_SPIKE_AVELOCITY not sure
SPIKE_GRENADE_THROW_DELAY delay throwing??
SPIKE_GRENADE_EXPLODE_DELAY delay before "exploding"
SPIKE_GRENADE_SPIKE_SPEED spike speed
SPIKE_GRENADE_SPIKE_COUNT spike count
*/
/*spike spore properties*/
#define SPORE_SPIKE_MIN_VELOCITY      200
#define SPORE_SPIKE_VELOCITY_MODIFIER 10.0
#define SPORE_SPIKE_AVELOCITY         300

#define SPIKE_GRENADE_THROW_DELAY 1.5f
#define SPIKE_GRENADE_EXPLODE_DELAY 2
#define SPIKE_GRENADE_SPIKE_SPEED 700
#define SPIKE_GRENADE_SPIKE_COUNT 10

/*human mines
PROXY_MINES_EXPLODE_WHEN_TOO_MANY_OUT if defined, causes explosion of mine (like flares or guardian spores) when player has placed too many
if not defined, just removes the ent
PROXY_MINES_SPIN_WHEN_THROWN if defined, causes the proxy mine, when thrown, to spin (like a grenade)
PROXY_MINE_HAS_HEAT if defined, proxy mine is visible on infared
PROXY_MINE_DETECTS if defined, proxy mine detects in PROXY_MINE_DETECT_RADIUS around it for aliens, and explodes if any
if not defined, only way to explode is to get stepped on, or new mines are placed (may or may not explode, depending on PROXY_MINES_EXPLODE_WHEN_TOO_MANY_OUT)
PROXY_MINE_NO_GUARDIAN_TIPTOE if defined, guardians cannot walk over proxy mines
if not defined, guardians will trigger the proxy mine by walking over it
*/
#define PROXY_MINE_HEALTH 20
#define PROXY_MINE_CHECKDELAY 2
#define MAX_ACTIVE_PROXY_MINES 2
#define PROXY_MINE_DETECT_RADIUS 150

#define PROXY_MINES_EXPLODE_WHEN_TOO_MANY_OUT
//#define PROXY_MINES_SPIN_WHEN_THROWN
//#define PROXY_MINE_HAS_HEAT
//#define PROXY_MINE_DETECTS
//#define PROXY_MINE_NO_GUARDIAN_TIPTOE

/*wraith acid (spit)*/
/*old stuff
#define WRAITH_ACID_GRAVITY 1
#define WRAITH_ACID_VELOCITY 455
new*/
#define WRAITH_ACID_GRAVITY 0.75
#define WRAITH_ACID_VELOCITY 600


/*------------------
   END OF GRENADES
START OF PROJECTILES
------------------*/

/*flying spikes used by spikers, spike spores, proxy spores (old ones), and stalker
SPIKE_MISSILE_TTL time for the spike to live, it gets freed after this time
*/
/*spike properties*/
#define PROJECTILE_SPIKE_TTL 50

/*web (drone spit)
WEB_GRAVITY gravity of webs when fired, used for arcs
DRONE_WEB_POWER_SHIELD_REDUCE define if you want the following 2 statements to work, otherwise power shield does nothing
  EXTERM_POWER_ARMOR_LOSS_TO_WEB cells lost for each spit that hits power armor
  EXTERM_LASTMOVETIME_BY_WEB seconds i'm guessing for xt to get stuck... not sure 
MINIMUM_TURRET_SLOW_BY_WEB this is the minimum turret slowing by the spit
*/
/*web properties*/
#define PROJECTILE_WEB_GRAVITY           1.35
#define PROJECTILE_WEB_MIN_VELOCITY      200
#define PROJECTILE_WEB_VELOCITY_MODIFIER 10.0
#define PROJECTILE_WEB_AVELOCITY         300

/*effects of the web*/
//#define DRONE_WEB_POWER_SHIELD_REDUCE
#define EXTERM_POWER_ARMOR_LOSS_TO_WEB 15
#define EXTERM_LASTMOVETIME_BY_WEB     5

#define MINIMUM_TURRET_SLOW_BY_WEB     0.03f

/*shotgun shells used by shock trooper
SHOT_SHELL_SPREAD spread spread
SHOT_SHELL_DAMAGE damage of shells
SHELL_KICK kick induced by firing
*/
#define SHOT_SHELL_SPREAD 400
#define SHOT_SHELL_DAMAGE 13
#define SHELL_KICK	10

/*spread*/
#define DEFAULT_SHOTGUN_HSPREAD	1000
#define DEFAULT_SHOTGUN_VSPREAD	500
#define DEFAULT_SHOTGUN_COUNT	12
#define DEFAULT_SSHOTGUN_COUNT	20

/*explosive shotgun shells used by shock trooper
EX_SHELL_CROUCH_SPREAD spread when crouched
EX_SHELL_DEFAULT_SPREAD spread when standing
EX_SHELL_DAMAGE damage
EX_SHELL_CLIPS_START starting clips, 1 is usual
EX_SHELL_CLIPS_MAX this is used for the Exploding Shell Pack upgrade, for maximum explosive shells
*/
#define EX_SHELL_CROUCH_SPREAD 300
#define EX_SHELL_DEFAULT_SPREAD 600
#define EX_SHELL_DAMAGE	75

#define EX_SHELL_CLIPS_START 1
#define EX_SHELL_CLIPS_MAX 3

/*autogun (machinegun) used by grunt
*/
/*spread*/
#define DEFAULT_AUTOGUN_HSPREAD	250
#define DEFAULT_AUTOGUN_VSPREAD	250

/*commando C4 settings
CMDO_DROP_C4_FROM_FALL if defined, then cmdo dies if falling from high heights on ground contact (with explosion)
CMDO_DROP_C4_FROM_DAMAGE if defined, damage causes c4 drop
*/

#define CMDO_DROP_C4_FROM_FALL
#define CMDO_DROP_C4_FROM_DAMAGE

#define CMDO_DROP_C4_FALL_EXPLODE_DAMAGE 300
#define CMDO_DROP_C4_FALL_EXPLODE_RADIUS 200


/*hatchling radius damage settings
NOTE: AS OF THE MOMENT, EVERY SERVER FRAME HAS AN ATTACK AS LONG AS ENEMY IS WITHIN RADIUS
HATCHLING_DO_RADIUS_DAMAGE if defined, hatchling radius damage is on
HATCHLING_RADIUS_DAMAGE the damage that the hatchling radius damage gives to nearby foes
HATCHLING_DAMAGE_RADIUS the damage radius that the hatchling radius damage takes effect in
*/
#define HATCHLING_DO_RADIUS_DAMAGE
#define HATCHLING_RADIUS_DAMAGE 2.0
#define HATCHLING_DAMAGE_RADIUS 100.0
















































/*mathematics etc that you shouldn't touch*/

#ifdef RELEASE
/*we do NOT want debug stuff inside a release*/
  #ifdef SHOW_DEPTH_SLASH_HACK
  #undef SHOW_DEPTH_SLASH_HACK
  #endif
#endif

#define EX_SHELL_CLIPS_ADDITION (EX_SHELL_CLIPS_MAX - EX_SHELL_CLIPS_START)

/*Savvy - this was something i discovered i could do to help out with preventing idiots who use extremely loud volumes to find hatchlings (and ofc other aliens)*/
#define ALIEN_JUMP_ATTN ATTN_STATIC
#define HUMAN_JUMP_ATTN ATTN_IDLE

