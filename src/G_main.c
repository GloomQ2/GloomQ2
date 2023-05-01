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

// #include <stdio.h> //FV7
#include "g_local.h"
#include "g_map.h"
#include "g_wordfilter.h"

team_info_t		team_info;
game_locals_t	game;
level_locals_t	level;
game_import_t	gi;
game_export_t	globals; //FV7
spawn_temp_t	st;

int	sm_meat_index;
int firea_index;
int fireb_index;
int	shiny_index;

#ifdef SAVVY_CONTENT
/*Savvy thermal gun stuff*/
int coldblasta_index;
int coldblastb_index;
#endif


int	changemap_spincount = 0;

int meansOfDeath;
#ifdef VOTE_EXTEND
int oldtimelimit = 0;
#endif
int	currentmapindex;

cvar_t	*maxentities;
cvar_t	*deathmatch;
cvar_t	*teameven;
#ifdef LPBEVEN
cvar_t	*teamlpbeven;
#endif
cvar_t	*buildercap;
cvar_t	*forceteams;
cvar_t	*reconnect_address;
cvar_t	*max_rate;
cvar_t	*max_cl_maxfps;
cvar_t	*min_cl_maxfps;
cvar_t	*banmaps;

cvar_t	*autokick;
cvar_t	*tk_threshold;
cvar_t	*teambonus;
cvar_t	*replenishbp;
cvar_t	*replenishbp_amt;
cvar_t	*replenishbp_tick;

cvar_t	*login;
cvar_t	*login_message;
cvar_t	*banaccountholders;

cvar_t	*gamedir;
cvar_t	*xmins;
cvar_t	*xmins_random;

cvar_t	*spiker_mode;

cvar_t	*turret_mode;
cvar_t	*turret_damage;
cvar_t	*turret_speed;
cvar_t	*turret_home;
cvar_t	*turret_bfg;
cvar_t	*turret_homingvelocityscale;
cvar_t	*turret_homingturnscale;
cvar_t	*turret_delay;

cvar_t *strip_names;
cvar_t *strip_chat;

cvar_t	*idletime;

/*cvar_t	*dynamic_configs;
cvar_t	*large_threshold;
cvar_t	*medium_threshold;
cvar_t	*small_threshold;
cvar_t	*tiny_threshold;*/

cvar_t	*voting;
cvar_t	*min_votes;
cvar_t	*votetimer;

cvar_t	*maxswitches;
cvar_t	*talk_threshold;
cvar_t	*nopublicchat;
cvar_t	*noobserverchat;
cvar_t	*deathmessages;
cvar_t	*dmflags;
cvar_t	*fraglimit;
cvar_t	*timelimit;
cvar_t	*reconnect_wait;
cvar_t	*password;
cvar_t	*needpass;
cvar_t	*g_select_empty;
cvar_t	*maxclients;
cvar_t	*maxplayers;
cvar_t	*adminslots;
cvar_t	*curplayers;
cvar_t	*dedicated;
cvar_t	*bandwidth_mode;
cvar_t	*gloomgamelog;
cvar_t	*adminpassword;
cvar_t	*teamsplashdamage;
cvar_t	*teamspawndamage;
cvar_t	*teamarmordamage;
cvar_t	*teamreflectarmordamage;
cvar_t	*teamreflectradiusdamage;
cvar_t	*teamdamage;
cvar_t	*flashlight_mode;
cvar_t	*laser_color;
cvar_t	*deathtest;
cvar_t	*firetest;
//cvar_t	*speedtest;
cvar_t	*basetest;
cvar_t	*playerclipfix;
cvar_t	*turrettest;
cvar_t	*nohtfragtest;
cvar_t	*pointstest;
cvar_t	*mapdebugmode;
cvar_t	*turretmode;
cvar_t	*secure_modulate_cap;
cvar_t	*ceiling_eggs;
cvar_t	*MAX_SCORE;
cvar_t	*hide_spawns;
cvar_t	*scoring_mode;
cvar_t	*motd;
cvar_t	*motd2;
cvar_t	*motd3;
cvar_t	*sv_maxvelocity;
cvar_t	*sv_gravity;
cvar_t	*sv_rollspeed;
cvar_t	*sv_rollangle;
cvar_t	*gun_x;
cvar_t	*gun_y;
cvar_t	*gun_z;
cvar_t	*run_pitch;
cvar_t	*run_roll;
cvar_t	*bob_up;
cvar_t	*bob_pitch;
cvar_t	*bob_roll;
cvar_t	*sv_cheats;
cvar_t	*gloomcaps;
cvar_t	*suddendeath;
cvar_t	*overtime;
cvar_t	*recoil;
cvar_t	*randominvert;

cvar_t	*tripwire_repair_count;
cvar_t	*upgrades;

cvar_t	*spiker_regrow_time;
cvar_t	*spiker_max_spikes_per_client;
cvar_t	*spiker_spike_count;
cvar_t	*spiker_damage;
cvar_t	*spiker_damage_random;
cvar_t	*spiker_speed;
cvar_t	*spiker_distance;

cvar_t	*spiketest;

cvar_t	*shrapnel_damage;
cvar_t	*shrapnel_count;

#ifdef ADMINDLLHACK
cvar_t	*admindllbypass;
cvar_t	*admindlltimer;
#endif

// config file names
/*cvar_t	*large_maps;
cvar_t	*medium_maps;
cvar_t	*small_maps;
cvar_t	*tiny_maps;*/
cvar_t	*default_maps;

// bans file and ip address log, accounts
cvar_t	*ipbans;
cvar_t	*iplogs;
cvar_t	*accounts;
cvar_t	*wordbans;
cvar_t	*adminlog;


/*Savvy custom cvars*/
cvar_t *observer_pointer_style;


int		playertimes[6] = {0,0,0,0,0,0};
int		currentmapmode = MAPS_DEFAULT;

// cache pointers
unsigned *imagecache;
unsigned *soundcache;

// exports
void EXPORT SpawnEntities (char *mapname, char *entities, char *spawnpoint);
void EXPORT ClientThink (edict_t *ent, usercmd_t *cmd);
qboolean EXPORT ClientConnect (edict_t *ent, char *userinfo);
void EXPORT ClientUserinfoChanged (edict_t *ent, char *userinfo);
void EXPORT ClientDisconnect (edict_t *ent);
void EXPORT ClientBegin (edict_t *ent);
void EXPORT ClientCommand (edict_t *ent);
/*void WriteGame (char *filename, qboolean autosave);
void ReadGame (char *filename);
void WriteLevel (char *filename);
void ReadLevel (char *filename);*/
void EXPORT InitGame (void);
void EXPORT G_RunFrame (void);
void EXPORT ServerCommand (void);
// enf of exports
void CheckVotes(void);
void RunEntity (edict_t *ent);

int SV_Cmd_WriteIP_f (void);
void SaveIPLog (char * filename);
void CloseAdminLog (void);

#ifdef ZZLIB
int compress(char *filename);
#endif
void EXPORT ShutdownGame (void)
{
	int records;

	gi.dprintf ("==== ShutdownGame Start ====\n");
	if (gloomlog) {
		#ifdef ZZLIB
		char oldname[MAX_QPATH];
		char newname[MAX_QPATH];
		#endif
		fclose (gloomlog);
		#ifdef ZZLIB
		compress (va("%s/%s/logs/%s", gamedir->string, game.port, gloomlogname));
		sprintf (oldname, "%s/%s/logs/%s", gamedir->string, game.port, gloomlogname);
		sprintf (newname, "%s/%s/logs/%s.zz", gamedir->string, game.port, gloomlogname);
		rename (oldname, newname);
		#endif
	}

	if (oldxmins)
		gi.cvar_set ("xmins", va("%g", oldxmins));

	if (accounts->string[0])
		SaveUserAccounts(accounts->string);

	if (iplogs->string[0])
		SaveIPLog(iplogs->string);

	if (wordbans->string[0])
		WriteWordFilters(wordbans->string);

	CloseAdminLog();

	// HACK (hmm?)
	if (ipbans->string[0]) {
		records = SV_Cmd_WriteIP_f();
		if (records >= 0) {
			gi.dprintf (" + Wrote %d bans to %s\n", records, ipbans->string);
		} else {
			gi.dprintf (" - Couldn't save bans to %s\n", ipbans->string);
		}
	}

	gi.dprintf ("==== ShutdownGame End ======\n");

	//warning, don't put any code after these...
 	gi.FreeTags (TAG_LEVEL);
	gi.FreeTags (TAG_GAME);
}

void EXPORT WriteGame (char *filename __attribute__((unused)), qboolean autosave __attribute__((unused))) //FV7 add attributes
{
}
void EXPORT WriteLevel (char *filename __attribute__((unused))) //FV7 add attribute
{
}
void EXPORT ReadGame (char *filename __attribute__((unused))) //FV7 add attribute
{
	gi.error ("Can't load single player games in Gloom.\n");
}
void EXPORT ReadLevel (char *filename __attribute__((unused))) //FV7 add attribute
{
	gi.error ("Can't load single player games in Gloom.\n");
}
/*
GetGameAPI

Returns a pointer to the structure with all entry points
and global variables
*/

// game_export_t * IMPORT GetGameAPI (game_import_t *import) //FV7
__attribute__((visibility("default"))) game_export_t *GetGameAPI (game_import_t *import)
{
	gi = *import;

	globals.apiversion = GAME_API_VERSION;
	globals.Init = InitGame;
	globals.Shutdown = ShutdownGame;
	globals.SpawnEntities = SpawnEntities;

	globals.WriteGame = WriteGame;
	globals.ReadGame = ReadGame;
	globals.WriteLevel = WriteLevel;
	globals.ReadLevel = ReadLevel;

	globals.ClientThink = ClientThink;
	globals.ClientConnect = ClientConnect;
	globals.ClientUserinfoChanged = ClientUserinfoChanged;
	globals.ClientDisconnect = ClientDisconnect;
	globals.ClientBegin = ClientBegin;
	globals.ClientCommand = ClientCommand;

	globals.RunFrame = G_RunFrame;

	globals.ServerCommand = ServerCommand;

	globals.edict_size = sizeof(edict_t);

	return &globals;
}

char *GetGameStats (void)
{
	size_t	i;
	edict_t	*ent;

	static char stats[1024];
	char	*str;

	int tks;
	int maxtks;

	int kills;
	int maxkills;

	int spawnkills;
	int maxspawnkills;

	int	deaths;
	int	maxdeaths;

	edict_t *maxtksby;
	edict_t *maxkillsby;
	edict_t *maxspawnkillsby;
	edict_t	*maxdeathsby;

	tks = maxtks = 0;
	kills = maxkills = 0;
	spawnkills = maxspawnkills = 0;
	deaths = maxdeaths = 0;

	maxtksby = maxkillsby = maxspawnkillsby = maxdeathsby = NULL;

	stats[0] = '\0';

	for (int i = 0; i < game.maxclients ; i++) //FV7 typecast `i` to int
	{
		ent = g_edicts + 1 + i;
		if (!ent->inuse || !ent->client)
			continue;

		tks = 0;

		if (ent->client->resp.team == TEAM_ALIEN) {
			tks = ent->client->resp.kills[CLASS_BREEDER] + ent->client->resp.kills[CLASS_HATCHLING] + ent->client->resp.kills[CLASS_DRONE] + ent->client->resp.kills[CLASS_WRAITH] + ent->client->resp.kills[CLASS_KAMIKAZE] + ent->client->resp.kills[CLASS_STINGER] + ent->client->resp.kills[CLASS_GUARDIAN] + ent->client->resp.kills[CLASS_STALKER];
		} else if (ent->client->resp.team == TEAM_HUMAN || (level.intermissiontime && ent->client->resp.old_team == TEAM_HUMAN))  {
			tks = ent->client->resp.kills[CLASS_ENGINEER] + ent->client->resp.kills[CLASS_GRUNT] + ent->client->resp.kills[CLASS_SHOCK] + ent->client->resp.kills[CLASS_BIO] + ent->client->resp.kills[CLASS_HEAVY] + ent->client->resp.kills[CLASS_COMMANDO] + ent->client->resp.kills[CLASS_EXTERM] + ent->client->resp.kills[CLASS_MECH];
		} else {
			if (ent->client->resp.old_team == TEAM_HUMAN) {
				tks = ent->client->resp.kills[CLASS_ENGINEER] + ent->client->resp.kills[CLASS_GRUNT] + ent->client->resp.kills[CLASS_SHOCK] + ent->client->resp.kills[CLASS_BIO] + ent->client->resp.kills[CLASS_HEAVY] + ent->client->resp.kills[CLASS_COMMANDO] + ent->client->resp.kills[CLASS_EXTERM] + ent->client->resp.kills[CLASS_MECH];
			} else {
				tks = ent->client->resp.kills[CLASS_BREEDER] + ent->client->resp.kills[CLASS_HATCHLING] + ent->client->resp.kills[CLASS_DRONE] + ent->client->resp.kills[CLASS_WRAITH] + ent->client->resp.kills[CLASS_KAMIKAZE] + ent->client->resp.kills[CLASS_STINGER] + ent->client->resp.kills[CLASS_GUARDIAN] + ent->client->resp.kills[CLASS_STALKER];
			}
		}

		if (tks > maxtks) {
			maxtks = tks;
			maxtksby = ent;
		}

		kills = 0;

		if (ent->client->resp.team == TEAM_ALIEN) {
			kills = ent->client->resp.kills[CLASS_ENGINEER] + ent->client->resp.kills[CLASS_GRUNT] + ent->client->resp.kills[CLASS_SHOCK] + ent->client->resp.kills[CLASS_BIO] + ent->client->resp.kills[CLASS_HEAVY] + ent->client->resp.kills[CLASS_COMMANDO] + ent->client->resp.kills[CLASS_EXTERM] + ent->client->resp.kills[CLASS_MECH];
		} else if (ent->client->resp.team == TEAM_HUMAN || (level.intermissiontime && ent->client->resp.old_team == TEAM_HUMAN))  {
			kills = ent->client->resp.kills[CLASS_BREEDER] + ent->client->resp.kills[CLASS_HATCHLING] + ent->client->resp.kills[CLASS_DRONE] + ent->client->resp.kills[CLASS_WRAITH] + ent->client->resp.kills[CLASS_KAMIKAZE] + ent->client->resp.kills[CLASS_STINGER] + ent->client->resp.kills[CLASS_GUARDIAN] + ent->client->resp.kills[CLASS_STALKER];
		} else {
			if (ent->client->resp.old_team == TEAM_HUMAN) {
				kills = ent->client->resp.kills[CLASS_BREEDER] + ent->client->resp.kills[CLASS_HATCHLING] + ent->client->resp.kills[CLASS_DRONE] + ent->client->resp.kills[CLASS_WRAITH] + ent->client->resp.kills[CLASS_KAMIKAZE] + ent->client->resp.kills[CLASS_STINGER] + ent->client->resp.kills[CLASS_GUARDIAN] + ent->client->resp.kills[CLASS_STALKER];
			} else {
				kills = ent->client->resp.kills[CLASS_ENGINEER] + ent->client->resp.kills[CLASS_GRUNT] + ent->client->resp.kills[CLASS_SHOCK] + ent->client->resp.kills[CLASS_BIO] + ent->client->resp.kills[CLASS_HEAVY] + ent->client->resp.kills[CLASS_COMMANDO] + ent->client->resp.kills[CLASS_EXTERM] + ent->client->resp.kills[CLASS_MECH];
			}
		}

		if (kills > maxkills) {
			maxkills = kills;
			maxkillsby = ent;
		}

		deaths = ent->client->resp.kills[PLAYERDEATHS];
		if (deaths > maxdeaths) {
			maxdeaths = deaths;
			maxdeathsby = ent;
		}

		spawnkills = ent->client->resp.kills[SPAWNKILLS];
		if (spawnkills > maxspawnkills) {
			maxspawnkills = spawnkills;
			maxspawnkillsby = ent;
		}
	}

	i = 0;
	if (maxkills) {
		str = va("Most kills: %s (%d) ",maxkillsby->client->pers.netname, maxkills);
		strcpy (stats + i, str);
		i += strlen (str);
	}
	if (maxspawnkills) {
		str = va("Most spawn kills: %s (%d) ",maxspawnkillsby->client->pers.netname, maxspawnkills);
		strcpy (stats + i, str);
		i += strlen (str);
	}
	if (maxtks) {
		str = va("Most teamkills: %s (%d) ",maxtksby->client->pers.netname, maxtks);
		strcpy (stats + i, str);
		i += strlen (str);
	}
	if (maxdeaths) {
		str = va("Most feed: %s (%d)", maxdeathsby->client->pers.netname, maxdeaths);
		strcpy (stats + i, str);
		i += strlen (str);
	}

	//if (stats[0])
	//	strcpy (stats + i, "\n");

	return stats;
}

/*
EndDMLevel

The timelimit or fraglimit has been exceeded
*/
void ClearSpawnQueue (void);
void EndDMLevel (void)
{
	edict_t		*ent = NULL;
	//int		j=0;
	unsigned int		nextmap=0;
	char	*map = NULL;
	qboolean looped=false;

	ClearSpawnQueue ();

	// trigger win ents
	if (team_info.winner == TEAM_ALIEN) {

		ent=G_Find (NULL, FOFS(classname), "on_aliens_win");

	} else if (team_info.winner == TEAM_HUMAN) {

		ent=G_Find (NULL, FOFS(classname), "on_humans_win");

	} else {
		ent=G_Find (NULL, FOFS(classname), "on_tie_game");
	}

	if (ent && !level.voted_map_change)
	{
		if (ent->map)
			strncpy(level.nextmap,ent->map, sizeof(level.nextmap));
		G_UseTargets ( ent, ent->activator);
	}

	if (!level.voted_map_change) {
		if (level.suddendeath) {
			game.sdwins[team_info.winner]++;
		} else {
			game.wins[team_info.winner]++;
		}
		game.wintime[team_info.winner] += level.time / 60;
	}

	// print statistics
	totaldeaths += (team_info.bodycount[TEAM_HUMAN] + team_info.bodycount[TEAM_ALIEN]);
	gi.bprintf (PRINT_HIGH, "The body count was %d humans and %d spiders (%u).\nGame lasted %d minutes and there were %d spawn kills.\n%s\n",
		team_info.bodycount[TEAM_HUMAN], team_info.bodycount[TEAM_ALIEN],
		totaldeaths,(int)(level.time / 60),team_info.spawnkills, GetGameStats());

	ent = NULL;

	// nextmap is currentmap
	nextmap = currentmapindex;

	/*if (!level.voted_map_change) {
		// no votes, bump up rotation position

		int i;
		int max = -1;
		int min = 256;

		for (i = 5;i >= 0;i--) {
			if (playertimes[i] > max)
				max = playertimes[i];
			if (playertimes[i] < min)
				min = playertimes[i];
			average += playertimes[i];
		}
		average -= max;
		average -= min;
		average /= 4;

		gi.dprintf("EndDMLevel: average players for nextmap = %f\n", average);

		//nextmap++;
	}*/

	// if there was a succesful vote, skip rotation
	// if there is a nextmap, skip rotation (trigger or worldspawn does this)
	if (!level.nextmap[0]) {
		unsigned int totalplayers = team_info.numplayers[TEAM_NONE] + team_info.numplayers[TEAM_HUMAN] + team_info.numplayers[TEAM_ALIEN];
		// traverse thru map rotation for next map, exclude mission maps (_*)
		// don't exclude too many maps, you might get into random cycles with small rotations

		MAPSEARCH: nextmap = NextByMagicAlgorithm(totalplayers);

		if (nextmap == 0 && !looped) {
			nextmap = 0;
			looped = true;
			goto MAPSEARCH;
		} else if (nextmap == 0 && looped) {
			// did a loop on maplist and didn't find suitable map
			gi.dprintf("EndDMLevel: No map found for current players, taking same map.\n");
			// check for worldspawn set nextmap first before forcing
			nextmap = currentmapindex;
		}

		map = MapnameByIndex(nextmap);
/*		MAPSEARCH: while ((map = MapnameByIndex(nextmap)))
		{
			foundit=true;

			// skip mission maps (they don't need to be in rotation anyway)
			if (map[0] == '_')
				foundit=false;

			// HACK: don't take currently played one
			if (!Q_stricmp(level.mapname, map))
				foundit=false;

			if (foundit)
				break;

			nextmap++;
		}

		// didn't find suitable map or maplist exhausted, loop from beginning
		if (!map && !looped) {
			j = nextmap; // j has the totals
			nextmap = 1;
			looped = true;
			goto MAPSEARCH;
		} else if (!map && looped) {
			// did a loop on maplist and didn't find suitable map, randomize
			gi.dprintf("EndDMLevel: Maplist looped twice, taking random map.\n");
			// check for worldspawn set nextmap first before forcing
			nextmap = (randomMT() + 1) % j;
			map = MapnameByIndex(nextmap);
		}*/
	}

	// do the actual level change

	// worldspawn forced map change?
	if (!level.nextmap[0])
	{
		if (map)
			strncpy(level.nextmap, map, sizeof(level.nextmap)-1);
		else
			strncpy(level.nextmap, level.mapname, sizeof(level.nextmap)-1);
	}

	currentmapindex = nextmap;
}

void CheckNeedPass (void)
{
	int need;

	// if password or spectator_password has changed, update needpass
	// as needed
	if (password->modified) // || spectator_password->modified)
	{
			password->modified = false; //spectator_password->modified = false;

			need = 0;

			if (*password->string && Q_stricmp(password->string, "none"))
					need |= 1;
			//if (*spectator_password->string && Q_stricmp(spectator_password->string, "none"))
			//		need |= 2;

			gi.cvar_set("needpass", va("%d", need));
	}
}

void CheckMotd (qboolean force)
{
	static char motd_string[26];
	static char motd_string2[26];
	static char motd_string3[26];

	// set team_menu motd

	if (motd->modified || force)
	{
		strncpy(motd_string, motd->string, sizeof(motd_string) - 1);

		motd->modified = false;
		team_menu[11].text = motd_string;
	}

	if (motd2->modified || force)
	{
		strncpy(motd_string2, motd2->string, sizeof(motd_string2) - 1);

		motd2->modified = false;
		team_menu[12].text = motd_string2;
	}

	if (motd3->modified || force)
	{
		strncpy(motd_string3, motd3->string, sizeof(motd_string3) - 1);

		motd3->modified = false;
		team_menu[13].text = motd_string3;
	}

}

void CheckDMRules (void)
{
	edict_t *ent;
	static qboolean xmins_issued;

	if (level.intermissiontime)
	{
		if (level.intermissiontime == level.framenum) {
			//r1: the map finished properly.
			if (team_info.winner)
				UpdateCurrentMapFinishCount(1);

			for (ent = g_edicts + 1; ent < g_edicts + 1 + game.maxclients; ent++) {
				if (ent->inuse) {
					ent->client->ps.stats[STAT_LAYOUTS] = 0;
					ent->client->showscores = 0;
				}
			}
			xmins_issued = false;
			EndDMLevel();
			BeginIntermission(NULL);
		} else if (level.intermissiontime + 10 == level.framenum) {
			SendEndOfGameScoreboard ();
			for (ent = g_edicts + 1; ent < g_edicts + 1 + game.maxclients; ent++) {
				if (ent->inuse)
					ent->client->ps.stats[STAT_LAYOUTS] |= 1;
			}
		} else if (level.intermissiontime + 110 == level.framenum) {
			gi.bprintf (PRINT_HIGH, "Next map: %s\n", level.nextmap);
		} else if (level.intermissiontime + 120 == level.framenum) {
			level.exitintermission = true;
		}

		return;
	}

	if (!deathmatch->value)
		return;

	if (!xmins_issued && xmins->value && level.framenum >= 10*60*xmins->value) {
		edict_t *spot = NULL;
		const char	*colors = colortext("***");
		gi.sound (world,CHAN_AUTO|CHAN_RELIABLE, SoundIndex (misc_talk1), 1, ATTN_NONE,0);
		gi.bprintf(PRINT_MEDIUM,"%s %g minutes is up! Starting spawns vulnerable! %s\n",colors, xmins->value, colors);

		while ((spot = G_Find (spot, FOFS(classname), "target_xmins")) != NULL) {
			G_UseTargets (spot, spot);
			if (spot)
				G_FreeEdict (spot);
		}
		xmins_issued = true;
	}
}


void ResetVoteList (void)
{
	edict_t *ent;

	// clear votes
	for (ent = g_edicts+1; ent < &g_edicts[game.maxclients+1]; ent++)
	{
		if (!ent->inuse)
			continue;

		ent->client->resp.voted = VOTE_INVALID;
	}

	//reset vote clients
	vote.timer = 0;

	//r1: need to zero this or vote endmap or vote extend (or anything that doesn't use it)
	//    will ban the last player who had a vote targetted against them if they disconnect
	vote.target_index = 0;

	vote.starter = NULL;
	active_vote = false;
}

/*Savvy edited CauseSuddenDeath for some new conditions for healers, depots, etc*/

int CauseSuddenDeath (void) {
	int alive = 0;
	edict_t *ent;

	for (ent = g_edicts+1; ent < &g_edicts[game.maxclients+1]; ent++) {
		if (!ent->inuse) { continue; }
		if (ent->health < 1) { continue; }

		if (ent->client->resp.class_type == CLASS_ENGINEER || ent->client->resp.class_type == CLASS_BREEDER) {
		T_Damage (ent, world, world, vec3_origin, vec3_origin, vec3_origin, 10000, 0, DAMAGE_NO_PROTECTION, MOD_OUT_OF_MAP);
		}
    else {
    alive++;
		//r1: sudden death: everyone dies after 2 mins if they don't kill anyone
		ent->client->sudden_death_frame = level.framenum + 1200;
		}
	}

ent = NULL;

	for (ent = g_edicts + game.maxclients; ent < &g_edicts[globals.num_edicts]; ent++) {
		if (!ent->inuse) { continue; }

		switch (ent->enttype) {
			//FV7 add block
			case ENT_NULL:
			case ENT_DRONEWEB:
			case ENT_SPIKE:
			case ENT_TURRETBASE:
			case ENT_TRIPWIRE_LASER:
			case ENT_TELEPORTER_U:
			case ENT_TELEPORTER_D:
			case ENT_COCOON_U:
			case ENT_COCOON_D:
			case ENT_WRAITHBOMB:
			case ENT_ROCKET:
			case ENT_GRENADE:
			case ENT_FLASH_GRENADE:
			case ENT_SMOKE_GRENADE:
			case ENT_C4:
			case ENT_GAS_SPORE:
			case ENT_SPIKE_SPORE:
			case ENT_PROXY_SPORE:
			case ENT_FLARE:
			case ENT_CORPSE:
			case ENT_HUMAN_BODY:
			case ENT_FEMALE_BODY:
			case ENT_FLAMES:
			case ENT_GASSER_GAS:
			case ENT_BLASTER:
			case ENT_NOBLOOD_BODY:
			case ENT_AREAPORTAL:
			case ENT_FUNC_DOOR:
			case ENT_TARGET_BLASTER:
			case ENT_PROXYSPIKE:
			case ENT_FUNC_EXPLOSIVE:
			case ENT_FUNC_WALL:
			case ENT_EVIL_HEALER:
			case ENT_SD_CAUSER:
			case ENT_MAP_TRIGGER_REPAIRER:
			case ENT_MAP_TRIGGER_HEALER:
			case ENT_SPBOT:
			//FV7 end block
			case ENT_TELEPORTER:
			case ENT_COCOON:
			case ENT_SPIKER:
			case ENT_TURRET:
			case ENT_MGTURRET:
			case ENT_OBSTACLE:
			case ENT_DETECTOR:
			case ENT_AMMO_DEPOT:
			case ENT_HEALER:
			case ENT_TRIPWIRE_BOMB:
			case ENT_GASSER:
			case ENT_INFEST:
				ent->hurtflags = 0;
				ent->style = 0;
				ent->flags = 0;
				T_Damage (ent, world, world, vec3_origin, vec3_origin, vec3_origin, 10000, 0, DAMAGE_NO_PROTECTION, MOD_OUT_OF_MAP);
				if (ent->health > 0 || ent->takedamage) { BecomeExplosion1 (ent); }
				break;
		}

		if (ent->classname && (!Q_stricmp (ent->classname, "trigger_repair") || !Q_stricmp (ent->classname, "trigger_healer"))) { G_FreeEdict (ent); }
	}

level.suddendeath = true;
return alive;
}

void vote_extend (edict_t *ent);
void centerprint_all(char *text);
void Use_Target_Speaker (edict_t *ent, edict_t *other, edict_t *activator);

void CheckTeamRules (void) {
qboolean	humans_lost=false, aliens_lost=false;
int	i, score1=0, score2=0;

	if (level.intermissiontime || !deathmatch->value) { return; }

	//FIXME: move these to event-based? (eg on spawn kill, on player kill)
	if(team_info.spawns[TEAM_ALIEN] == 0 && countPlayers(TEAM_ALIEN) == 0) { aliens_lost = true; }
	if(team_info.spawns[TEAM_HUMAN] == 0 && countPlayers(TEAM_HUMAN) == 0) { humans_lost = true; }

	if (fraglimit->value && !aliens_lost && !humans_lost) {
	gclient_t	*cl;

		for (i=0 ; i<game.maxclients ; i++) {
			if (!g_edicts[i+1].inuse) {continue; }

			cl = game.clients + i;

			if (cl->resp.total_score < 0) {continue; }

			if(cl->resp.team == TEAM_ALIEN) { score1 += cl->resp.total_score; }
			else if(cl->resp.team == TEAM_HUMAN) { score2 += cl->resp.total_score; }
		}

		if (score1 >= fraglimit->value) {
		aliens_lost = true;
		gi.bprintf (PRINT_HIGH, "Fraglimit hit.\n");
		}

		if (score2 >= fraglimit->value) {
		humans_lost = true;
		gi.bprintf (PRINT_HIGH, "Fraglimit hit.\n");
		}
	}

	if (timelimit->value) {
		if (suddendeath->value) {
			if (level.framenum == (unsigned int)(timelimit->value*600)) { //FV7 unsigned
				int remaining = CauseSuddenDeath();
				if (remaining) { centerprint_all ("Sudden Death!"); }
			}
			else if (level.framenum == (unsigned int)((timelimit->value - 1)*600))   { centerprint_all ("1 Minute Remaining\nUntil Sudden Death"); } //FV7 unsigned
			else if (level.framenum == (unsigned int)((timelimit->value - 0.5)*600)) { centerprint_all ("30 Seconds Remaining\nUntil Sudden Death"); } //FV7 unsigned
			else if (!level.suddendeath && level.framenum % 10 == 0 && (level.framenum >= (timelimit->value * 600) - (10 * 10))) { centerprint_all (va ("%d", ((int)(timelimit->value * 600) - level.framenum) / 10)); }
		}
		else {
			if (level.framenum == (unsigned int)((timelimit->value - 5)*600)) { centerprint_all ("5 Minutes Remaining"); } //FV7 unsigned
			else if (level.framenum == (unsigned int)((timelimit->value - 1)*600)) { centerprint_all ("1 Minute Remaining"); } //FV7 unsigned
		}

		if (level.framenum >= (unsigned int)((timelimit->value + suddendeath->value)* 600)) { //FV7 unsigned
		gi.bprintf (PRINT_HIGH, "Timelimit hit: ");
		/*Savvy edited this so that 1 team will win due to frags*/
      if (score1 >= score2) {
      aliens_lost = true;
      gi.bprintf (PRINT_HIGH, "Humans won.\n");
      }
      else if (score1 <= score2) {
      humans_lost = true;
      gi.bprintf (PRINT_HIGH, "Aliens won.\n");
      }
      else {
      aliens_lost = true;
      humans_lost = true;
      gi.bprintf (PRINT_HIGH, "Tie.\n");
      }
		}
		else if (level.framenum == (unsigned int)((overtime->value)* 600)) { //FV7 unsigned
		team_info.buildflags = 0xFFFFFFFFU;
    gi.bprintf (PRINT_HIGH, "Entering overtime mode, no further building is possible. %d\n", (int)overtime->value); //FV7 %d
		}
		else if (level.framenum + 3000 == (unsigned int)(timelimit->value*600)) { //FV7 unsigned
		gi.sound (world, CHAN_AUTO, SoundIndex(misc_5min), 1.0, ATTN_NONE, 0);
#ifdef VOTE_EXTEND
		}
    else if (level.framenum + 1500 == (unsigned int)(timelimit->value*600)) { //FV7 unsigned
			if ((int)voting->value & 64) {
			vote_extend (NULL);
				if (active_vote) {
				edict_t *vent;
					for (vent = g_edicts+1; vent < &g_edicts[game.maxclients+1]; vent++) {
						if (!(vent->inuse && vent->client)) { continue; }
						if (!vent->client->resp.team) { vent->client->resp.voted = VOTE_ABSTAIN; }
						else { vent->client->resp.voted = VOTE_INVALID; }
					}
				vote.timer = level.framenum + (votetimer->value * 10);
				vote.type = VOTE_EXTEND;
				vote.starter = NULL;
				centerprint_all (va("Server started a vote to\n%s the timelimit\nYou have %i seconds to vote\nType vote <yes|no> in console to vote", colortext("extend"),(int)(vote.timer - level.framenum)/10));
				gi.sound (world, CHAN_AUTO, SoundIndex (misc_secret), 1.0, ATTN_NONE, 0);
				gi.cprintf (NULL,PRINT_HIGH, "[vote] Server started a vote to extend the timelimit.\n");
				}
			}
#endif
		}
	}

	// use_humanwin & use_alienwin uses this, UGLY!
	if (team_info.leveltimer > 0 && level.time > team_info.leveltimer*60.0) {
		if (team_info.playmode == MODE_HUMANWIN) { aliens_lost = true; }
		else if (team_info.playmode == MODE_ALIENWIN) { humans_lost = true; }
		else { humans_lost = aliens_lost = true; }
	}

	//check we are actually allowed to win
	if (humans_lost && team_info.lost[TEAM_ALIEN]) { aliens_lost = true; }
	else if (aliens_lost && team_info.lost[TEAM_HUMAN]) { humans_lost = true; }

	if (humans_lost && aliens_lost) {
	edict_t *speaker = NULL;
	speaker = G_Find (NULL,FOFS(classname),"on_humans_win");
		if (speaker && speaker->message) { gi.bprintf(PRINT_HIGH, "%s", speaker->message); }
		else { gi.bprintf (PRINT_HIGH, "Tie game.\n"); }
	team_info.winner = TEAM_NONE;
	level.intermissiontime = level.framenum + 10*10;
	//r1: abort votes, no point continuing them past this point.
	ResetVoteList();
	}
  else if (aliens_lost && !humans_lost) {
	/*human victory*/
	edict_t *speaker = NULL;
	speaker = G_Find (speaker, FOFS(classname), "target_speaker_h");
		if (speaker) { Use_Target_Speaker (speaker, speaker, speaker); }
		else { gi.sound(world, CHAN_AUTO|CHAN_RELIABLE, SoundIndex (victory_human), 1, ATTN_NONE, 0); }
	team_info.winner = TEAM_HUMAN;
	speaker = G_Find (NULL, FOFS(classname), "on_humans_win");

		if (speaker && speaker->message) { gi.bprintf(PRINT_HIGH, "%s", speaker->message); }
		else { gi.bprintf (PRINT_HIGH, "%s Win!\n", team_info.teamnames[TEAM_HUMAN]); }
	//r1: abort votes, no point continuing them past this point.
	ResetVoteList();

		level.intermissiontime = level.framenum + 10*10;
	}
  else if(humans_lost && !aliens_lost) {
	/*alien victory*/
	edict_t *speaker = NULL;
	speaker = G_Find (speaker, FOFS(classname), "target_speaker_a");
		if (speaker) { Use_Target_Speaker (speaker, speaker, speaker); }
		else { gi.sound(world, CHAN_AUTO|CHAN_RELIABLE, SoundIndex (victory_alien), 1, ATTN_NONE, 0); }
	team_info.winner = TEAM_ALIEN;
	speaker = G_Find (NULL, FOFS(classname), "on_aliens_win");

		if (speaker && speaker->message) { gi.bprintf(PRINT_HIGH, "%s", speaker->message); }
		else { gi.bprintf (PRINT_HIGH, "%s Win!\n", team_info.teamnames[TEAM_ALIEN]); }

	//r1: abort votes, no point continuing them past this point.
	ResetVoteList();

	level.intermissiontime = level.framenum + 10*10;
	}
}

void ClientEndServerFrames (void);
void ExitLevel (void) {
char	command [256];

	//r1: detect maps in the map cycle which don't exist on the server - if this gets called
	//    more than once we assume the request to change map didn't work for some reason (eg
	//    map doesn't exist or something)

	if (changemap_spincount++ == 1) {
		char *map = MapnameByIndex(NextByIndex (currentmapindex, 0));
		if (!map) {
			map = MapnameByIndex(NextByIndex (0, 0));
			if (!map) {
				gi.error ("ExitLevel: Changemap to '%s' failed, no other maps available.", level.nextmap);
			}
		}
		RemoveFromMaplist (IndexByMapname (level.nextmap, 0));
		gi.bprintf (PRINT_HIGH, "WARNING: Request to change map to '%s' failed. Trying '%s'...\n", level.nextmap, map);
		strcpy (level.nextmap, map);
	}


#ifdef GMDPLUG
	gi.bprintf (PRINT_HIGH,"\nDon't have the next map '%s'?\nGet it at the Gloom Map Depository\n%s\n",level.nextmap,colortext("http://www.planetquake.com/gmd/"));
#endif
	Com_sprintf (command, sizeof(command), "gamemap \"%s\"\n", level.nextmap);
	gi.AddCommandString (command);
	//level.exitintermission = false;

	//r1: is this even needed?
	ClientEndServerFrames ();
}

/*void CheckMapCycle (void)
{
	int i;
	int players = 0;

	for (i = 5;i > 0;i--) {
		playertimes[i] = playertimes[i-1];
	}

	for (i=0 ; i<MAXTEAMS ; i++)
		players += team_info.numplayers[i];

	playertimes[0] = players;
}*/

/*
G_RunFrame

Advances the world by 0.1 seconds
*/
void CheckReconnectEnts (void);
void ProcessSpawnQueue (void);
void EXPORT G_RunFrame (void) {

	if (game.paused) { return; }

level.framenum++;
level.time = level.framenum * FRAMETIME;

	// exit intermissions
	if (level.exitintermission) {
	ExitLevel ();
	return;
	}

	/*
	 * treat each object in turn
	 * even the world gets a chance to think
	 */
	// some movement code uses level.current_entity for teammaster identifying
level.current_entity = g_edicts;
	do {
		if (level.current_entity->inuse) {
			VectorCopy (level.current_entity->s.origin, level.current_entity->s.old_origin);

			// if the ground entity moved, make sure we are still on it
			if ((level.current_entity->groundentity) && (level.current_entity->groundentity->linkcount != level.current_entity->groundentity_linkcount)) {
				level.current_entity->groundentity = NULL;
				if ( !(level.current_entity->flags & (FL_SWIM|FL_FLY)) && (level.current_entity->svflags & SVF_MONSTER) ) {
				M_CheckGround (level.current_entity);
				}
			}

			if (level.current_entity->client) { ClientBeginServerFrame (level.current_entity); }
			else { G_RunEntity (level.current_entity); }
		}

		level.current_entity++;
	} while (level.current_entity < g_edicts+globals.num_edicts);

	// build the playerstate_t structures for all players
	ClientEndServerFrames ();

	/* misc stuff that doesn't belong to the actual game */
	CheckDMRules ();

	if (active_vote) {
		CheckVotes();
	}

	CheckTeamRules ();

	ProcessSpawnQueue ();

	// FIXME: set the uptime variable on initgame to time() and calculate the uptime from there
	if (level.framenum % 600 == 0)
	{
		int days = 0;
		int hours = 0;
		int mins = uptime + 1;

		/*static clock_t start, finish;
		double duration;

		if (!start)
			start = clock();
		else {
			finish = clock();
			duration = (double)(finish - start) / CLOCKS_PER_SEC;
			gi.dprintf ("%2.4f seconds\n", duration );
			start = clock();
		}*/

		uptime++;

		CheckNeedPass ();
		CheckMotd (false);

		/*if (*reconnect_address->string)
			CheckReconnectEnts();*/

		//unnecessary
		//CheckIPBanList();

		while (mins/60/24 >= 1) {
			days++;
			mins -= 60*24;
		}

		while (mins/60 >= 1) {
			hours++;
			mins -= 60;
		}

		gi.AddCommandString (va("set uptime \"%ddays, %dhrs, %dmins\" s\n",days, hours, mins));
	}
}

void think_cause_sd (edict_t *ent)
{
	ent->count--;

	if (level.intermissiontime)
	{
		G_FreeEdict (ent);
		return;
	}

	if (ent->count && ent->count <= 10)
		centerprint_all (va("%d\n", ent->count));

	if (ent->count == 30)
		centerprint_all ("Sudden Death in\n30 Seconds\n");

	if (ent->count == 15)
		centerprint_all ("Sudden Death in\n15 Seconds\n");

	if (!ent->count)
	{
		centerprint_all ("Sudden Death!\n");
		CauseSuddenDeath ();
		G_FreeEdict (ent);
		return;
	}

	ent->nextthink = level.time + 1;
}

void CheckVotes (void){
	edict_t *ent;
	int invalid = 0;
	int votes=0;
	unsigned int yes=0, no=0;

	if (active_vote && (level.framenum >= vote.timer)) {

		active_vote=false;
		vote.voters=0;

		for (ent = g_edicts+1; ent < &g_edicts[game.maxclients+1]; ent++)
		{
			if (!(ent->inuse && ent->client && ent->client->pers.connected))
				continue;

			if (ent->client->resp.voted == VOTE_INVALID) {
				if (vote.type == VOTE_TEAMKICK) {
					if (ent->client->resp.team == vote.starter->client->resp.team) {
						invalid++;
					}
				} else {
					invalid++;
				}
			}


			if (ent->client->resp.voted & VOTE_YES) {
				yes++;
			} else if (ent->client->resp.voted & VOTE_NO) {
				if (vote.type == VOTE_TEAMKICK) {
					if (ent->client->resp.team == vote.starter->client->resp.team) {
						no++;
					}
				} else {
					no++;
				}
			}

			// veto right for gods, useful for preventing votes or kick people out
			if (ent->client->resp.voted & VOTE_VETO) {

				yes=no=0;
				vote.voters=1;
				invalid = 0;

				if (ent->client->resp.voted & VOTE_YES)
					yes++;
				else
					no++;

				gi.bprintf(PRINT_HIGH, va("[vote] %s vetoed the vote.\n", ent->client->pers.netname));
				break;
			}

			if (vote.type == VOTE_TEAMKICK) {
				if (ent->client->resp.team == vote.starter->client->resp.team) {
					vote.voters++;
				}
			} else {
				vote.voters++;
			}

		}

		no += (invalid/2);
		votes = yes + no;

		//if (votes<vote.voters/2 || (vote.type==VOTE_NEXTMAP && votes<vote.voters/4)){
		//	gi.bprintf(PRINT_HIGH, "Voting is over, not enough people voted!\n");
		//} else {
		gi.bprintf(PRINT_HIGH, "[vote] Voting is over, %d voted yes, %d voted no.\n", yes, no);
		if (yes > no) {
			if (vote.type == VOTE_KICK || vote.type == VOTE_TEAMKICK) {
				//gi.bprintf(PRINT_HIGH, "[vote] %s was kicked.\n", vote.ent->client->pers.netname);
				ban(&g_edicts[vote.target_index],(int)banmaps->value, va("vote kick by %s", vote.starter->client->pers.netname), NULL);
				kick(&g_edicts[vote.target_index]);
			}else if (vote.type == VOTE_MUTE){
				gi.bprintf(PRINT_HIGH, "[vote] Muting %s!\n", g_edicts[vote.target_index].client->pers.netname);
				g_edicts[vote.target_index].client->pers.muted = true;
				g_edicts[vote.target_index].client->pers.namechanges = 4; //no nick changing for j00.
			}else if (vote.type == VOTE_MAP){
				char *mapname=NULL;
				mapname = MapnameByIndex (vote.target_index);
				if (mapname) {
					gi.bprintf(PRINT_HIGH, "[vote] Changing the map to %s!\n", mapname);
					level.voted_map_change = true;
					strncpy(level.nextmap, mapname, sizeof(level.nextmap)-1);
					level.intermissiontime = level.framenum + 1;
				} else {
					gi.bprintf(PRINT_HIGH, "[vote] Error finding map index %d in maplist!\n", vote.target_index);
				}

			}else if (vote.type == VOTE_NEXTMAP){
				char *mapname=NULL;
				mapname = MapnameByIndex (vote.target_index);
				if (mapname) {
					gi.bprintf(PRINT_HIGH, "[vote] The next map will be %s!\n", mapname);
					level.voted_map_change = true;
					strncpy(level.nextmap, mapname, sizeof(level.nextmap)-1);
				} else {
					gi.bprintf(PRINT_HIGH, "[vote] Error finding map index %d in maplist!\n", vote.target_index);
				}
			} else if (vote.type == VOTE_ENDMAP) {
				if ((float)yes / (float)votes < 0.66) {
					gi.bprintf (PRINT_HIGH,"[vote] Need at least 66%% yes vote for end map.\n");
				} else {
					gi.bprintf (PRINT_HIGH,"[vote] The map is aborted!\n");
					level.intermissiontime = level.framenum + 1;
					//r1: spam the finished count extra low so its not likely to come up again
					UpdateCurrentMapFinishCount (-5);
				}
#ifdef VOTE_EXTEND
			} else if (vote.type == VOTE_EXTEND) {
				gi.bprintf (PRINT_HIGH, "[vote] The timelimit is extended by 30 minutes!\n");
				//r1: moved oldtimelimit to spawnentities, or else we could end up with the 
				//old timelimit being an already extended value.
				gi.cvar_set ("timelimit", va("%d", (int)timelimit->value + 30));
#endif
			} else if (vote.type == VOTE_SD) {
				edict_t *spawner = G_Spawn ();
				spawner->count = 60;
				spawner->nextthink = level.time + 1;
				spawner->think = think_cause_sd;
				spawner->enttype = ENT_SD_CAUSER;
				spawner->classname = "sdcauser";
				spawner->svflags |= SVF_NOCLIENT;
				gi.linkentity (spawner);
				gi.bprintf (PRINT_HIGH, "[vote] Sudden Death in 60 seconds!!\n");
				centerprint_all ("Sudden Death In\n60 Seconds!\n");
			}
		} else {
			if (vote.type == VOTE_KICK || vote.type == VOTE_TEAMKICK) {
				gi.bprintf(PRINT_HIGH, "[vote] %s gets to stay!\n", g_edicts[vote.target_index].client->pers.netname);
			} else if (vote.type == VOTE_MAP) {
				char *mapname=NULL;
				mapname = MapnameByIndex (vote.target_index);
				if (mapname) {
					gi.bprintf(PRINT_HIGH, "[vote] The map will not be changed to %s!\n", mapname);
					vote.lastmap_index[0] = vote.lastmap_index[1];
					vote.lastmap_index[1] = vote.target_index;
				}
			} else if (vote.type == VOTE_NEXTMAP) {
				char *mapname=NULL;
				mapname = MapnameByIndex (vote.target_index);
				if (mapname) {
					gi.bprintf(PRINT_HIGH, "[vote] The next map will not be changed to %s!\n", mapname);
				}
			} else if (vote.type == VOTE_MUTE) {
				gi.bprintf (PRINT_HIGH, "[vote] %s gets to annoy everyone!\n",g_edicts[vote.target_index].client->pers.netname);
			} else if (vote.type == VOTE_ENDMAP) {
				gi.bprintf (PRINT_HIGH, "[vote] The map continues!\n");
#ifdef VOTE_EXTEND
			} else if (vote.type == VOTE_EXTEND) {
				gi.bprintf (PRINT_HIGH, "[vote] The timelimit remains at %d minutes!\n", (int)timelimit->value);
#endif
			} else if (vote.type == VOTE_SD) {
				gi.bprintf (PRINT_HIGH, "[vote] The map continues!\n");
			}
			if (vote.starter)
				vote.starter->client->resp.failed_votes++;
		}

		//r1ch: "dumb" votes get instant quota
		if (vote.starter)
		{
			if (votes >= 5 && yes < 3) {
				vote.starter->client->resp.failed_votes = FAILED_VOTES_LIMIT;
			}

			vote.starter->client->last_vote_time = level.time + 180;
		}

		ResetVoteList();

	} // votes<vote.voters<2 ...
}
