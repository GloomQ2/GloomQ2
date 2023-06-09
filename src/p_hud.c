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

#include "g_local.h"

/*

INTERMISSION

*/
void MoveClientToIntermission (edict_t *ent)
{
	int i;

	ent->svflags |= SVF_NOCLIENT;
	VectorCopy (level.intermission_origin, ent->s.origin);
	VectorClear (ent->velocity);

	VectorClear (ent->client->ps.viewoffset);
	VectorClear (ent->client->ps.kick_angles);

	VectorCopy (level.intermission_angle, ent->client->ps.viewangles);
	VectorCopy (level.intermission_angle, ent->s.angles);
	VectorCopy (level.intermission_angle, ent->client->v_angle);

	// if delta_angles isn't set correctly, the final fov will be wrong (we're stopping normal fov calcs)
	for (i=0 ; i<3 ; i++)
		ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(ent->s.angles[i] - ent->client->resp.cmd_angles[i]);

	//ent->client->ps.pmove.pm_type = PM_FREEZE;
	ent->client->ps.gunindex = 0;

	ent->client->ps.blend[3] = 0;

	ent->health = 0;
	ent->client->ps.rdflags &= ~RDF_UNDERWATER;
	ent->client->ps.fov = 90;

	ent->client->invincible_framenum = 0;
	ent->client->grenade_blow_up = 0;
	ent->client->grenade_time = 0;
	ent->client->resp.old_team = ent->client->resp.team;

	ent->viewheight = 0;
	ent->s.modelindex = 0;
	ent->s.modelindex2 = 0;
	ent->s.modelindex3 = 0;
	ent->s.modelindex4 = 0;
	ent->s.effects = 0;
	ent->s.sound = 0;
	ent->solid = SOLID_NOT;
	ent->takedamage = DAMAGE_NO;

	if ( ent->client->resp.flashlight )
	{
		G_FreeEdict(ent->client->resp.flashlight);
		ent->client->resp.flashlight = NULL;
	}

	PMenu_Close(ent);

	memset (&ent->client->ps.stats, 0, sizeof (ent->client->ps.stats));

	// add the layout
	if (deathmatch->value)
	{
		ent->client->showscores = 0;
	}
}

void BeginIntermission (edict_t *targ)
{
	int		i;//, n;
	edict_t	*ent, *client;

	if (level.intermissiontime != level.framenum)
		return;		// already activated

//	game.autosaved = false;

	// take map from given ent, or search for one
	if (!level.voted_map_change) {
		if (targ) {
			strncpy(level.nextmap, targ->map, sizeof(level.nextmap)-1);
		} else {
			// check for target_changelevel, if it should be used in timelimit/fraglimit situations
			ent = G_Find (NULL, FOFS(classname), "target_changelevel");
			if (ent) {
				strncpy(level.nextmap, ent->map, sizeof(level.nextmap)-1);
			}
		}
	}

	if (!deathmatch->value)
	{
		level.exitintermission = true;		// go immediately to the next level
		return;
	}

	EndOfGameScoreboard();

	if (team_info.winner == TEAM_ALIEN) {
		// find an intermission spot
		ent = G_Find (NULL, FOFS(classname), "info_player_intermission_a");
		if (!ent)
		{	// the map creator forgot to put in an intermission point...
			ent = G_Find (NULL, FOFS(classname), "info_player_intermission");
			if (!ent) {
				gi.dprintf ("WARNING: couldn't find an info_player_intermission!!\n");
				ent = G_Find2 (NULL, FOFS(classname), "info_player_start");
			}
		}
		else
		{	// chose one of four spots
			i = randomMT() & 3;
			while (i--)
			{
				ent = G_Find (ent, FOFS(classname), "info_player_intermission_a");
				if (!ent)	// wrap around the list
					ent = G_Find (ent, FOFS(classname), "info_player_intermission_a");
			}
		}
	
	} else if (team_info.winner == TEAM_HUMAN) {
		// find an intermission spot
		ent = G_Find (NULL, FOFS(classname), "info_player_intermission_h");
		if (!ent)
		{	// the map creator forgot to put in an intermission point...
			ent = G_Find (NULL, FOFS(classname), "info_player_intermission");
			if (!ent) {
				gi.dprintf ("WARNING: couldn't find an info_player_intermission!!\n");
				ent = G_Find2 (NULL, FOFS(classname), "info_player_start");
			}
		}
		else
		{	// chose one of four spots
			i = randomMT() & 3;
			while (i--)
			{
				ent = G_Find (ent, FOFS(classname), "info_player_intermission_h");
				if (!ent)	// wrap around the list
					ent = G_Find (ent, FOFS(classname), "info_player_intermission_h");
			}
		}
	} else {
		// find an intermission spot
		ent = G_Find (NULL, FOFS(classname), "info_player_intermission");
		if (!ent)
		{	// the map creator forgot to put in an intermission point...
			gi.dprintf ("WARNING: couldn't find an info_player_intermission!!\n");
			ent = G_Find (NULL, FOFS(classname), "info_player_start");
		}
		else
		{	// chose one of four spots
			i = randomMT() & 3;
			while (i--)
			{
				ent = G_Find (ent, FOFS(classname), "info_player_intermission");
				if (!ent)	// wrap around the list
					ent = G_Find (ent, FOFS(classname), "info_player_intermission");
			}
		}
	}

	if (!ent) {
		gi.dprintf ("WARNING: couldn't find any suitable intermission entity!!!!\n");
		return;
	}

	VectorCopy (ent->s.origin, level.intermission_origin);
	VectorCopy (ent->s.angles, level.intermission_angle);

	// move all clients to the intermission point
	for (i=0 ; i < game.maxclients ; i++)
	{
		client = g_edicts + 1 + i;
		if (!client->inuse)
			continue;
		MoveClientToIntermission (client);
	}

	//r1: reset client teams to avoid bugging up team_info array due to disconnections
	//at start of next map (causes negative clients to show in team select and messes
	//up scoreboard quite nicely)

	//note: we want to do it here otherwise peoples scoreboards would be fubar
	//in previous for loop.
	for (i=0 ; i < game.maxclients ; i++)
	{
		client = g_edicts + 1 + i;
		if (!client->inuse)
			continue;

		client->client->resp.class_type = CLASS_OBSERVER;
		client->client->resp.team = TEAM_NONE;
		client->client->resp.score = 0;
		client->client->resp.total_score = 0;
	}
}

/*
DeathmatchScoreboardMessage
*/
void Scoreboard_Large (edict_t *ent);
void Scoreboard_Team_Iconed (edict_t *ent);
void Scoreboard_leet (edict_t *ent);
void DeathmatchScoreboardMessage (edict_t *ent, edict_t *killer)
{
	int		big = 0;

	if (level.intermissiontime || team_info.numplayers[ent->client->resp.team] > 16)
		big=1;

	if (!big){	// small

		if (ent->client->resp.scoreboardstate)
			Scoreboard_Large (ent);
		else
			Scoreboard_Team_Iconed (ent);
//			Scoreboard_leet (ent);

	}else{	// big

		Scoreboard_Large (ent);
	}

}

/* other ideas:

	split mode: 6vs6 icons
	hybrid iconed: few iconed and few text
*/

void Scoreboard_Team_Iconed (edict_t *ent)
{
	char	entry[1024];
	char	string[1400];
	size_t	stringlength=0;
	int	i, k;
	int sorted[MAX_CLIENTS];
	int sortedscores[MAX_CLIENTS];
	int	score, total=0;
	gclient_t	*cl;
	edict_t		*cl_ent;
	int	x,y;
	char	*tag;
	size_t	j;

	for (i=0 ; i<game.maxclients ; i++)
	{
		cl_ent = g_edicts + 1 + i;

		if (!cl_ent->inuse)
			continue;

		if (game.clients[i].resp.team != ent->client->resp.team || !game.clients[i].resp.visible)
			continue;

		score = game.clients[i].resp.total_score;

		for (j=0 ; j<total ; j++)
		{
			if (score > sortedscores[j])
				break;
		}
		for (k=total ; k>j ; k--)
		{
			sorted[k] = sorted[k-1];
			sortedscores[k] = sortedscores[k-1];
		}
		sorted[j] = i;
		sortedscores[j] = score;
		total++;
	}

	if (team_info.numplayers[ent->client->resp.team] > 12)
		y=-32; // if over 12 players, make y's start at 0 below --tumu
	else
		y=32;

	if (total) {
		// print it
		for (i=0 ; i<total ; i++)
		{
			cl = &game.clients[sorted[i]];
			cl_ent = g_edicts + 1 + sorted[i];

			// scoreboard size hack: orders players by score in rows --tumu
			if (!(i & 1)) {
				x=0;
				y+=32;
			} else
				x=160;

			if (level.suddendeath)
			{
				tag = NULL;
			}
			else
			{
				if (cl_ent == ent)
					tag = "tag1";
				else if (cl_ent->health < 1 && cl->resp.team != TEAM_NONE)
					tag = "tag2";
				else
					tag = NULL;
			}

			if (tag)
			{
				Com_sprintf (entry, sizeof(entry),
					" xv %i yv %i picn %s",x+32, y, tag);
				j = strlen(entry);
				if (stringlength + j > 1024)
					break;
				strcpy (string + stringlength, entry);
				stringlength += j;
			}

			score = cl->resp.total_score;

			Com_sprintf (entry, sizeof(entry),
				" client %i %i %i %i %i %i",
				x, y, sorted[i],
				score,
				cl->ping,
				(int)((level.framenum - cl->resp.enterframe)/600));

			j = strlen(entry);
			if (stringlength + j > 1024)
				break;
			strcpy (string + stringlength, entry);
			stringlength += j;
		}

		// send the layout
	} else {
		Com_sprintf (entry, sizeof(entry),
			" client %i %i %i %i %i %i",
			0, y, 0,
			0,
			0,
			0);

		strcpy (string + stringlength, entry);
	}

	gi.WriteByte (svc_layout);
	gi.WriteString (string);

}


#if 0
void Scoreboard_leet (edict_t *ent)
{
	char	entry[1024] = "\0";
	char	string[1400] = "\0";
	int		stringlength = 0;
	int		i, j, k;

	int		sorted[MAX_CLIENTS];
	int		sortedscores[MAX_CLIENTS];

	int		score, total=0;
	gclient_t	*cl;
	edict_t		*cl_ent;
	int	x,y;
	char	*tag;

	int classorder[] = {CLASS_BREEDER, CLASS_HATCHLING, CLASS_DRONE,
						CLASS_WRAITH, CLASS_KAMIKAZE, CLASS_STINGER, CLASS_GUARDIAN, CLASS_STALKER,
						CLASS_ENGINEER, CLASS_GRUNT, CLASS_SHOCK, CLASS_BIO, CLASS_HEAVY,
						CLASS_COMMANDO, CLASS_EXTERM, CLASS_MECH, CLASS_OBSERVER};

	int classes[NUMCLASSES+1] = {0};

	for (i=0 ; i<game.maxclients ; i++)
	{
		cl_ent = g_edicts + i + 1;

		if (!cl_ent->inuse)
			continue;

		if (game.clients[i].resp.team != ent->client->resp.team || !game.clients[i].resp.visible)
			continue;

		score = game.clients[i].resp.total_score;

		for (j=0 ; j<total ; j++)
		{
			if (score > sortedscores[j])
				break;
		}
		for (k=total ; k>j ; k--)
		{
			sorted[k] = sorted[k-1];
			sortedscores[k] = sortedscores[k-1];
		}
		sorted[j] = i;
		sortedscores[j] = score;

		// count classes for formatting
		classes[game.clients[i].resp.class_type]++;

		total++;
	}

/*	if (team_info.numplayers[ent->client->resp.team] > 12)
		y=-32; // if over 12 players, make y's start at 0 below --tumu
	else
		y=32;
*/

/*

minimum of 4 lines per class icon of text fits

4*32 pixel icons = 128 pixels
7*8 spacers between icons = 56 pixels

total of 184 pixels if put in single column (virtual is 320x200)

every class icon can have 4 playernames alongside, total of 32 players

<----32+8*8+15*8=216 pixels--->

        {score name ping}
[breed] 500 351 Tuomari Ankka
[er   ] 250  50 Tuomari Grue
[icon ]
[     ]

[hatch] 500 351 Tuomari UhRaK
[ icon] 250  50 Tuomari iddqd
[     ] 50   10 Tuomari Grue
[     ]

[drone] 234 233 ProPut Sp
[drone]
[drone]
[drone]

[wrait] 500 351 Tuomari Kromi
[h    ] 250  50 Tuomari Enigma
[ icon] 50   10 bassokoira
[     ]

[kami ] x
[kami ]
[kami ]
[kami ]

[sting] y
[er   ]
[     ]
[     ]

[guard] z 
[ian  ]
[     ]
[     ]

[stalk] xyz
[er   ]
[     ]
[     ]


xv %d yv %d picn %s string


*/
	if (total) {
		char *s;
		int classcounter;
//		qboolean even=true;

		classes[CLASS_BREEDER] = 2;
		classes[CLASS_HATCHLING] = 6;
		classes[CLASS_DRONE] = 2;
		classes[CLASS_WRAITH] = 1;
		classes[CLASS_KAMIKAZE] = 1;
		classes[CLASS_STINGER] = 2;
		classes[CLASS_GUARDIAN] = 1;
		classes[CLASS_STALKER] = 1;

		// first print the icons
		y = 12;

		Q_snprintf(entry, sizeof(entry), "xv 40 yv 0 string \"Sc   Ping   Name\" ");

		k = strlen (entry);
		if (stringlength + k > 1024)
			return;
		strcpy (string + stringlength, entry);
		stringlength += k;

		// then the texts
		// roll over classes
		for (classcounter = 0 ; classcounter<17 ; classcounter++) {

			i = classorder[classcounter];

			if (classes[i] > 0) {

				// ugly workaround, fix!
				switch (i) {
				case CLASS_BREEDER:
					s = "breeder";
					break;
				case CLASS_HATCHLING:
					s = "kami";
					break;
				case CLASS_DRONE:
					s = "drone";
					break;
				case CLASS_WRAITH:
					s = "wraith";
					break;
				case CLASS_KAMIKAZE:
					s = "kami";
					break;
				case CLASS_STINGER:
					s = "stinger";
					break;
				case CLASS_GUARDIAN:
					s = "guardian";
					break;
				case CLASS_STALKER:
					s = "stalker";
					break;
				default:
					s = "obs";
					break;
				}

//				Q_snprintf(entry, sizeof(entry), "xv %i yv %i picn %s xv %i ", even ? 0 : 168,  y, s, even ? 40 : 0);
				Com_sprintf(entry, sizeof(entry), "xv %i yv %i picn %s xv %i ", 0,  y, s, 40);

				k = strlen (entry);
				if (stringlength + k > 1024)
					break;
				strcpy (string + stringlength, entry);
				stringlength += k;
			}

			// roll over clients
			for (j = 0 ; j < classes[i] ; j++) { // s/classes/total

				// insert proper client inputs
				cl = &game.clients[sorted[0]];
				cl_ent = g_edicts + 1 + sorted[0];

				Q_snprintf (entry, sizeof(entry),
					"yv %i string%s \"%3i%4i %s\" ",
					y+j*8,
					cl_ent == ent ? "2" : "",
					cl->resp.total_score, cl->ping, cl->pers.netname);

				k = strlen(entry);
				if (stringlength + k > 1024)
					break;
				strcpy (string + stringlength, entry);
				stringlength += k;
			}

//			even ^= true;

			if (classes[i] > 1 && classes[i] < 5)
				y += 32 + 8;
			else if (classes[i] > 4)
				y += 32 + 8 + 8 * (classes[i] - 4);
		}

	}

	gi.WriteByte (svc_layout);
	gi.WriteString (string);

	gi.dprintf("%i | %s\n", stringlength, string);
}
#endif
void Scoreboard_Large (edict_t *ent)
{
	char	entry[1024];
	char	string[1400];
	size_t	stringlength=0;
	int	i, k;
	int	sorted[MAX_CLIENTS];
	int sortedscores[MAX_CLIENTS];
	int	score, total=0;
	int	team_y[MAXTEAMS];
	gclient_t	*cl;
	edict_t		*cl_ent;
	char	*tag, teamname;
	size_t		j;

	for (i=0 ; i<game.maxclients ; i++)
	{
		cl_ent = g_edicts + 1 + i;

		if (!cl_ent->inuse)
			continue;

		if (!cl_ent->client->resp.visible)
			continue;

		score = game.clients[i].resp.total_score;

		for (j=0 ; j<total ; j++)
		{
			if (score > sortedscores[j])
				break;
		}
		for (k=total ; k>j ; k--)
		{
			sorted[k] = sorted[k-1];
			sortedscores[k] = sortedscores[k-1];
		}
		sorted[j] = i;
		sortedscores[j] = score;
		total++;
	}

	// hacky way to determine right starting Y positions
	if (ent->client->resp.team == TEAM_ALIEN || ent->client->resp.team == TEAM_NONE){
		team_y[TEAM_ALIEN] = 8 + 4;
		team_y[TEAM_HUMAN] = team_y[TEAM_ALIEN] + team_info.numplayers[TEAM_ALIEN]*8 + 4;
		team_y[TEAM_NONE] = team_y[TEAM_ALIEN] + team_info.numplayers[TEAM_ALIEN]*8 + 4 +
				    team_info.numplayers[TEAM_HUMAN]*8 + 4;

	} else if (ent->client->resp.team == TEAM_HUMAN) {
		team_y[TEAM_HUMAN] = 8 + 4;
		team_y[TEAM_ALIEN] = team_y[TEAM_HUMAN] + team_info.numplayers[TEAM_HUMAN]*8 + 4;
		team_y[TEAM_NONE] = team_y[TEAM_HUMAN] + team_info.numplayers[TEAM_HUMAN]*8 + 4 +
				    team_info.numplayers[TEAM_ALIEN]*8 + 4;
	}

	//was 24, is 160-(17*8/2)
	Com_sprintf(entry,sizeof(entry),"xv 92 yv 0 string2 \"  Scr Png Tm Name\"");
	j = strlen(entry);
	strcpy (string + stringlength, entry);
	stringlength += j;

	for (i=0 ; i<total ; i++)
	{
		cl = &game.clients[sorted[i]];
		cl_ent = g_edicts + 1 + sorted[i];

		if (cl->resp.team==TEAM_ALIEN)
			teamname = (cl_ent->health <= 0) ? 'a' : 'A';
		else if (cl->resp.team == TEAM_HUMAN)
			teamname = (cl_ent->health <= 0) ? 'h' : 'H';
		else
			teamname = 'O';

		if (level.suddendeath && ent->client->resp.team)
			teamname = toupper(teamname);

		// highlight player
		if (cl_ent == ent)
			tag = "string";
		else
			tag = "string2";

		score = cl->resp.total_score;

		Com_sprintf(entry,sizeof(entry)," yv %i %s \"%c.%-3i%4i %2i %s\"",
			team_y[cl->resp.team], tag, teamname, score, cl->ping, 
			(int)((level.framenum - cl->resp.enterframe)/600),
			cl->pers.netname);

		j = strlen(entry);
		if (stringlength + j > 1024)
			break;

		team_y[cl->resp.team] += 8;

		strcpy (string + stringlength, entry);
		stringlength += j;
	}

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
}

static char	scoreboard_string[1400];

void SendEndOfGameScoreboard (void)
{
	gi.WriteByte (svc_layout);
	gi.WriteString (scoreboard_string);
	gi.multicast (vec3_origin, MULTICAST_ALL_R);
}

void EndOfGameScoreboard (void)
{
	char	entry[1024];
	size_t	stringlength=0;
	int	i, k;
	int	sorted[MAX_CLIENTS];
	int sortedscores[MAX_CLIENTS];
	int	score, total=0;
	int	team_y[MAXTEAMS];
	gclient_t	*cl;
	edict_t		*cl_ent;
	char	*tag, *teamname;
	size_t		j;

	for (i=0 ; i<game.maxclients ; i++)
	{
		cl_ent = g_edicts + 1 + i;

		if (!cl_ent->inuse)
			continue;

		if (!cl_ent->client->resp.visible)
			continue;

		score = game.clients[i].resp.total_score;

		for (j=0 ; j<total ; j++)
		{
			if (score > sortedscores[j])
				break;
		}
		for (k=total ; k>j ; k--)
		{
			sorted[k] = sorted[k-1];
			sortedscores[k] = sortedscores[k-1];
		}
		sorted[j] = i;
		sortedscores[j] = score;
		total++;
	}

	// hacky way to determine right starting Y positions
	//if (ent->client->resp.team == TEAM_ALIEN || ent->client->resp.team == TEAM_NONE){
		team_y[TEAM_ALIEN] = 8 + 4;
		team_y[TEAM_HUMAN] = team_y[TEAM_ALIEN] + team_info.numplayers[TEAM_ALIEN]*8 + 4;
		team_y[TEAM_NONE] = team_y[TEAM_ALIEN] + team_info.numplayers[TEAM_ALIEN]*8 + 4 +
				    team_info.numplayers[TEAM_HUMAN]*8 + 4;

/*	} else if (ent->client->resp.team == TEAM_HUMAN) {
		team_y[TEAM_HUMAN] = 8 + 4;
		team_y[TEAM_ALIEN] = team_y[TEAM_HUMAN] + team_info.numplayers[TEAM_HUMAN]*8 + 4;
		team_y[TEAM_NONE] = team_y[TEAM_HUMAN] + team_info.numplayers[TEAM_HUMAN]*8 + 4 +
				    team_info.numplayers[TEAM_ALIEN]*8 + 4;
	}*/

	//was 24, is 160-(17*8/2)
	Com_sprintf(entry,sizeof(entry),"xv 92 yv 0 string2 \"  Scr Png Tm Name\"");
	j = strlen(entry);
	strcpy (scoreboard_string + stringlength, entry);
	stringlength += j;

	for (i=0 ; i<total ; i++)
	{
		cl = &game.clients[sorted[i]];
		cl_ent = g_edicts + 1 + sorted[i];

		if (cl->resp.team==TEAM_ALIEN)
			teamname = (cl_ent->deadflag) ? "a" : "A";
		else if (cl->resp.team == TEAM_HUMAN)
			teamname = (cl_ent->deadflag) ? "h" : "H";
		else
			teamname = "O";

		// highlight player
		//if (cl_ent == ent)
			tag = "string";
		//else
		//	tag = "string2";

		score = cl->resp.total_score;

		Com_sprintf(entry,sizeof(entry)," yv %i %s \"%s.%-3i%4i %2i %s\"",
			team_y[cl->resp.team], tag, teamname, score, cl->ping, 
			(int)((level.framenum - cl->resp.enterframe)/600),
			cl->pers.netname);

		j = strlen(entry);
		if (stringlength + j > 1024)
			break;

		team_y[cl->resp.team] += 8;

		strcpy (scoreboard_string + stringlength, entry);
		stringlength += j;
	}
}


/*
DeathmatchScoreboard

Draw instead of help message.
Note that it isn't that hard to overflow the 1400 byte message limit!
*/
void DeathmatchScoreboard (edict_t *ent)
{
	ent->client->showscores = level.framenum + 50;
	DeathmatchScoreboardMessage (ent, ent->enemy);
	gi.unicast (ent, true);
}

/*
Cmd_Score_f

Display the scoreboard
*/
void Cmd_Score_f (edict_t *ent)
{
	ent->client->showinventory = false;
	//ent->client->showhelp = false;

	if (ent->client->menu.entries)
	{
		PMenu_Close(ent);
		return;
	}

	if (!deathmatch->value)
		return;

	if (level.intermissiontime)
		return;

	if (ent->client->showscores)
	{
		ent->client->showscores = 0;
		return;
	}

	if (ent->client->last_menu_frame == level.framenum)
		return;

	ent->client->last_menu_frame = level.framenum;

	ent->client->showscores = 1;
	ent->client->resp.scoreboardstate = 0; // force iconed
	DeathmatchScoreboard (ent);
}

/*
Cmd_Help_f

Display the current help message
*/
void Cmd_Help_f (edict_t *ent)
{
	// this is for backwards compatability
	if (deathmatch->value)
	{
		Cmd_Score_f (ent);
		return;
	}
}

/*
G_SetStats
*/
void G_SetStats (edict_t *ent)
{
	const gitem_t		*item;
	int			index, cells = 0;
	int			power_armor_type;

	if (level.intermissiontime && (level.intermissiontime < level.framenum))
		return;

	//r1: just copy from chasee
	if (ent->client->chase_target)
	{
		int old_menu = ent->client->ps.stats[STAT_MENU];
		memcpy (ent->client->ps.stats, ent->client->chase_target->client->ps.stats, sizeof(ent->client->ps.stats));

		//only copy in 1st person
		if (ent->client->chase_view == 0)
			memcpy (ent->client->ps.kick_angles, ent->client->chase_target->client->ps.kick_angles, sizeof(ent->client->ps.kick_angles));

		//don't allow some things for non-admins
		if (!(ent->client->pers.adminpermissions & (1 << PERMISSION_VIEWGAMESTUFF)))
		{
			if (!level.suddendeath)
				ent->client->ps.stats[STAT_POINTS] = 0;

			ent->client->ps.stats[STAT_ALIENICON] =
			ent->client->ps.stats[STAT_ALIENSPAWN] =
			ent->client->ps.stats[STAT_HUMANICON] =
			ent->client->ps.stats[STAT_HUMANSPAWN] = 0;
		}

		ent->client->ps.stats[STAT_LAYOUTS] = 0;
		ent->client->ps.stats[STAT_MENU] = old_menu;

		if (ent->client->showscores)
			ent->client->ps.stats[STAT_LAYOUTS] |= 1;

		if (ent->client->showinventory && ent->health > 0)
			ent->client->ps.stats[STAT_LAYOUTS] |= 2;

		ent->client->ps.stats[STAT_CTF_ID_VIEW] = CS_GENERAL + (ent->client->chase_target - g_edicts - 1);

		return;
	}

	/*
	 * health
	 */
	if (ent->client->resp.class_type != CLASS_OBSERVER) {
		ent->client->ps.stats[STAT_HEALTH_ICON] = imagecache[i_health];
		ent->client->ps.stats[STAT_HEALTH] = ent->health & 65535;
	} else {
		if (ent->client->chase_target) {
			ent->client->ps.stats[STAT_HEALTH_ICON] = imagecache[i_health];
			ent->client->ps.stats[STAT_HEALTH] = ent->client->chase_target->health & 65535;
		} else {
			ent->client->ps.stats[STAT_HEALTH_ICON] =
			ent->client->ps.stats[STAT_HEALTH] = 0;
		}
	}

	/*
	 * ammo
	 */
	if(ent->client->chase_target)
	{
		item = &itemlist[ent->client->chase_target->client->ammo_index];
		ent->client->ps.stats[STAT_AMMO_ICON] = imagecache[item->item_id];
		ent->client->ps.stats[STAT_AMMO] = ent->client->chase_target->client->resp.inventory[ent->client->chase_target->client->ammo_index];
	}
	else
	{
		if (ent->client->ammo_index && !ent->client->resp.turret) {
			item = &itemlist[ent->client->ammo_index];
			ent->client->ps.stats[STAT_AMMO_ICON] = imagecache[item->item_id];
			ent->client->ps.stats[STAT_AMMO] = ent->client->resp.inventory[ent->client->ammo_index];
		} else {
			ent->client->ps.stats[STAT_AMMO_ICON] =
			ent->client->ps.stats[STAT_AMMO] = 0;
		}
	}

	/*
	 * armor
	 */
	if (ent->client->chase_target)
	{
		if (ent->client->chase_target->client->armor)
		{
			index = ITEM_INDEX(ent->client->chase_target->client->armor);

			item = ent->client->chase_target->client->armor;
			ent->client->ps.stats[STAT_ARMOR_ICON] = imagecache[item->item_id];
			ent->client->ps.stats[STAT_ARMOR] = ent->client->chase_target->client->resp.inventory[index];
		} else
		{
			ent->client->ps.stats[STAT_ARMOR_ICON] =
			ent->client->ps.stats[STAT_ARMOR] = 0;
		}

	} else {
		int armor = GetArmor (ent);;
		power_armor_type = PowerArmorType (ent);
		if (power_armor_type)
		{
			cells = ent->client->resp.inventory[ITEM_AMMO_CELLS];
			if (cells == 0 && ent->client->resp.class_type != CLASS_EXTERM)
			{	// ran out of cells for power armor
				ent->flags &= ~FL_POWER_ARMOR;
				gi.sound(ent, CHAN_AUTO, SoundIndex (misc_power2), 1, ATTN_IDLE, 0);
				power_armor_type = 0;
			}
		}

		if (power_armor_type && (!armor || (level.framenum & 8) ) )
		{	// flash between power armor and other armor icon
			ent->client->ps.stats[STAT_ARMOR_ICON] = imagecache[i_powershield];
			ent->client->ps.stats[STAT_ARMOR] = cells;
		}
		else if (ent->client->armor)
		{
			ent->client->ps.stats[STAT_ARMOR_ICON] = imagecache[ent->client->armor->item_id];
			ent->client->ps.stats[STAT_ARMOR] = armor;
		}
		else
		{
			ent->client->ps.stats[STAT_ARMOR_ICON] =
			ent->client->ps.stats[STAT_ARMOR] = 0;
		}
	}

	/*
	 * pickup message
	 */
	if (level.time > ent->client->pickup_msg_time)
	{
		ent->client->ps.stats[STAT_PICKUP_ICON] =
		ent->client->ps.stats[STAT_PICKUP_STRING] = 0;
	}

	/*
	 * selected item
	 */
	if (ent->client->resp.selected_item == 0 || ent->client->resp.selected_item == -1 || ent->client->resp.turret) {
		ent->client->ps.stats[STAT_SELECTED_ICON] =
		ent->client->ps.stats[STAT_SELECTED_ITEM] =
		ent->client->ps.stats[STAT_ITEM_QUANTITY] = 0;
	} else {
		ent->client->ps.stats[STAT_SELECTED_ICON] = imagecache[itemlist[ent->client->resp.selected_item].item_id];
		ent->client->ps.stats[STAT_SELECTED_ITEM] = ent->client->resp.selected_item;
		ent->client->ps.stats[STAT_ITEM_QUANTITY] = ent->client->resp.inventory[ent->client->resp.selected_item];
	}

	/*
	 * layouts
	 */
	ent->client->ps.stats[STAT_LAYOUTS] = 0;

	if (ent->client->showscores)
		ent->client->ps.stats[STAT_LAYOUTS] |= 1;

	if (ent->client->showinventory && ent->health > 0)
		ent->client->ps.stats[STAT_LAYOUTS] |= 2;

	/*
	 * frags
	 */
	ent->client->ps.stats[STAT_FRAGS] = (int)ent->client->resp.total_score;
	if (ent->client->chase_target)
		ent->client->ps.stats[STAT_KILLS] = (int)ent->client->chase_target->client->resp.total_score;
	else
		ent->client->ps.stats[STAT_KILLS] = (int)ent->client->resp.score;

	/*
	 * help icon / current weapon if not shown
	 */
	//if (ent->client->resp.helpchanged && (level.framenum&8) )
		//ent->client->ps.stats[STAT_HELPICON] = imagecache[i_help");
	if ( (ent->client->pers.hand == CENTER_HANDED) && ent->client->weapon && (ent->client->resp.team==TEAM_HUMAN) && (ent->client->resp.class_type!=CLASS_ENGINEER) && !ent->client->resp.turret) {
		ent->client->ps.stats[STAT_HELPICON] = imagecache[ent->client->weapon->item_id];
	} else {
		if (ent->client->resp.turret) 
			ent->client->ps.stats[STAT_AMMO_ICON] = imagecache[i_missile];
		else
			ent->client->ps.stats[STAT_HELPICON] = 0;
	}

	ent->client->ps.stats[STAT_TIMER_ICON] =
	ent->client->ps.stats[STAT_TIMER] =
	ent->client->ps.stats[STAT_CELLS_ICON] = 0;

	if(team_info.starttime > level.time)
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = imagecache[turtle];
		ent->client->ps.stats[STAT_TIMER] = (int)(team_info.starttime - level.time);
	}

	if (ent->client->resp.team == TEAM_HUMAN) {
		if (ent->client->build_timer > level.time) {
			ent->client->ps.stats[STAT_TIMER_ICON] = imagecache[i_tele];
			ent->client->ps.stats[STAT_TIMER] = (int)(ent->client->build_timer - level.time);
		} else if(ent->client->resp.class_type == CLASS_BIO) {
			item = FindItem("cells");
			cells = ITEM_INDEX(item);
			ent->client->ps.stats[STAT_TIMER_ICON] = imagecache[a_cells];
			ent->client->ps.stats[STAT_TIMER] = ent->client->resp.inventory[cells];
		} else if (ent->client->resp.class_type == CLASS_EXTERM && ent->client->last_move_time > level.time) {
			ent->client->ps.stats[STAT_TIMER_ICON] = imagecache[a_blaster];
			ent->client->ps.stats[STAT_TIMER] = (int)(ent->client->last_move_time - level.time);
		}
	} else if (ent->client->resp.team == TEAM_ALIEN) {
		if (ent->client->build_timer > level.time) {
			ent->client->ps.stats[STAT_TIMER_ICON] = imagecache[i_cocoon];
			ent->client->ps.stats[STAT_TIMER] = (int)(ent->client->build_timer - level.time);
		} else if (ent->client->resp.class_type == CLASS_GUARDIAN && ent->client->grenade_blow_up) {
			ent->client->ps.stats[STAT_TIMER_ICON] = imagecache[c4_i];
			ent->client->ps.stats[STAT_TIMER] = (ent->client->grenade_time - level.time);
		}
	}

	//observers can't see spawn/build point counts till they really join
	if (ent->client->resp.class_type != CLASS_OBSERVER) { 

		if (!level.suddendeath) {
			if(ent->client->resp.team)
				ent->client->ps.stats[STAT_POINTS] = (int)team_info.maxpoints[ent->client->resp.team] - team_info.points[ent->client->resp.team];
			else
				ent->client->ps.stats[STAT_POINTS] = 0;
		} else {
			if (ent->health > 0)
				ent->client->ps.stats[STAT_POINTS] = 1 + (ent->client->sudden_death_frame - level.framenum) / 10;
			else
				ent->client->ps.stats[STAT_POINTS] = 0;
		}

		if (level.suddendeath) {
			ent->client->ps.stats[STAT_ALIENICON] =
			ent->client->ps.stats[STAT_ALIENSPAWN] =
			ent->client->ps.stats[STAT_HUMANICON] =
			ent->client->ps.stats[STAT_HUMANSPAWN] = 0;
		} else {
			if (hide_spawns->value) {
				if (ent->client->resp.team == TEAM_HUMAN) {
					ent->client->ps.stats[STAT_ALIENICON] = 0;
					ent->client->ps.stats[STAT_HUMANICON] = imagecache[i_tele];

					ent->client->ps.stats[STAT_ALIENSPAWN] = 0;
					ent->client->ps.stats[STAT_HUMANSPAWN] = team_info.spawns[TEAM_HUMAN];
				} else if (ent->client->resp.team == TEAM_ALIEN) {
					ent->client->ps.stats[STAT_ALIENICON] = imagecache[i_cocoon];
					ent->client->ps.stats[STAT_HUMANICON] = 0;

					ent->client->ps.stats[STAT_ALIENSPAWN] = team_info.spawns[TEAM_ALIEN];
					ent->client->ps.stats[STAT_HUMANSPAWN] = 0;
				} else {
					ent->client->ps.stats[STAT_ALIENICON] =
					ent->client->ps.stats[STAT_ALIENSPAWN] =
					ent->client->ps.stats[STAT_HUMANICON] =
					ent->client->ps.stats[STAT_HUMANSPAWN] = 0;
				}
			} else {
					ent->client->ps.stats[STAT_ALIENICON] = imagecache[i_cocoon];
					ent->client->ps.stats[STAT_HUMANICON] = imagecache[i_tele];

					ent->client->ps.stats[STAT_ALIENSPAWN] = team_info.spawns[TEAM_ALIEN];
					ent->client->ps.stats[STAT_HUMANSPAWN] = team_info.spawns[TEAM_HUMAN];
			}
		}
	} else {
		ent->client->ps.stats[STAT_POINTS] =
		ent->client->ps.stats[STAT_ALIENICON] =
		ent->client->ps.stats[STAT_ALIENSPAWN] =
		ent->client->ps.stats[STAT_HUMANICON] =
		ent->client->ps.stats[STAT_HUMANSPAWN] = 0;
	}

	// id view update
	CTFSetIDView(ent);
}

//r1: updated so engi/breeder can see their teams structure health
//r1: updated so engi can see teammates armor count
//r1: updated to trace uses a small mins/maxs since the trace code
//    if it hits means the cpu intensive bit doesnt execute.

void CTFSetIDView(edict_t *ent)
{
	vec3_t	forward;
	trace_t	tr;
	vec3_t start;
	vec3_t mins = {-4,-4,-4};
	vec3_t maxs = {4,4,4};

	if(ent->client->pers.id_state == false && ent->client->resp.flashlight == NULL)
		return;

	ent->client->ps.stats[STAT_CTF_ID_VIEW] = 0;

	VectorCopy (ent->s.origin,start);
	start[2] += ent->viewheight;

	AngleVectors(ent->client->v_angle, forward, NULL, NULL);

	VectorScale(forward, 4096, forward);
	VectorAdd(ent->s.origin, forward, forward);

	tr = gi.trace(start, mins, maxs, forward, ent, CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_DEADMONSTER);

	// flashlight update code starts
	if (ent->client->resp.flashlight) {

		if (tr.ent && tr.ent->client && (tr.ent->client->resp.class_type == CLASS_GUARDIAN) &&
			(tr.ent->s.modelindex == 0) && ent->client->resp.team == TEAM_HUMAN) {
			VectorCopy (tr.endpos, start);
			tr = gi.trace (start,mins,maxs, forward, tr.ent,CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_DEADMONSTER);
		}

		// backoff a bit as flashlight always hits something, and even if it didn't we don't care
		//VectorMA(tr.endpos,-4,forward,endp);
		//VectorCopy(endp,tr.endpos);

		// sw mode fix?
		if ((tr.ent->svflags & SVF_MONSTER) || (tr.ent->client))
		{
			if ((tr.ent->takedamage) && (tr.ent != ent->client->resp.flashlight->owner))
			{
				ent->client->resp.flashlight->s.skinnum = 1;
			}
		}
		else
			ent->client->resp.flashlight->s.skinnum = 0;

		vectoangles(tr.plane.normal,ent->client->resp.flashlight->s.angles);
		VectorCopy(tr.endpos,ent->client->resp.flashlight->s.origin);
		// following removes lagging from flashlight when toggling it
		//VectorCopy(tr.endpos,self->s.old_origin);

		gi.linkentity (ent->client->resp.flashlight);
		//gi.bprintf (PRINT_HIGH, "trace hit %s at %s\n", tr.ent->classname, vtos(tr.endpos));
	}
	// flashlight update code ends

	// return now if no id
	if(ent->client->pers.id_state == false)
		return;

	// we hit something
	if (tr.ent && tr.ent->health > 0 && tr.ent != ent) {
		if (tr.ent->client) {
			// player
			if (!tr.ent->client->resp.visible)
				return;

			if (ent->client->resp.class_type == CLASS_BIO && tr.ent->client->resp.team == TEAM_ALIEN && ent->health > 0)
			{
				//can't glow invis guard
				if (tr.ent->s.modelindex)
				{
					//don't override
					if (tr.ent->client->glow_time <= level.time)
						tr.ent->client->glow_time = level.time + .5f;
				}
			}

			// check for same team
			if (ent->client->resp.team != TEAM_NONE && (ent->client->resp.team != tr.ent->client->resp.team))
				return;

			ent->client->ps.stats[STAT_CTF_ID_VIEW] = CS_GENERAL + (tr.ent - g_edicts - 1);

			if (ent->client->resp.class_type == CLASS_ENGINEER) {
				if (tr.ent->client->armor) {
					ent->client->ps.stats[STAT_TIMER_ICON] = imagecache[tr.ent->client->armor->item_id];
					ent->client->ps.stats[STAT_TIMER] = tr.ent->client->resp.inventory[ITEM_INDEX(tr.ent->client->armor)];
				}
			} else if (ent->client->resp.class_type == CLASS_BIO) {
				ent->client->ps.stats[STAT_TIMER_ICON] = imagecache[i_health];
				ent->client->ps.stats[STAT_TIMER] = tr.ent->health;
			} else if (ent->client->resp.class_type == CLASS_BREEDER) {
				ent->client->ps.stats[STAT_TIMER_ICON] = imagecache[i_health];
				ent->client->ps.stats[STAT_TIMER] = tr.ent->health;
			}
		} else {
			// ent (probably buildable)
			if (ent->client->resp.class_type == CLASS_BREEDER && tr.ent->svflags & SVF_MONSTER && ent->client->build_timer < level.time) {
				ent->client->ps.stats[STAT_TIMER_ICON] = imagecache[i_health];
				ent->client->ps.stats[STAT_TIMER] = tr.ent->health;
			} else if (ent->client->resp.class_type == CLASS_ENGINEER && tr.ent->flags & FL_CLIPPING && ent->client->build_timer < level.time) {
				ent->client->ps.stats[STAT_TIMER_ICON] = imagecache[i_health];
				ent->client->ps.stats[STAT_TIMER] = (int)(((float)tr.ent->health / (float)tr.ent->max_health) * 100.0);
			}
		}
	} else if (ent->client->resp.class_type == CLASS_BIO && ent->health > 0) {
		edict_t	*who, *best;
		vec3_t dir;
		float	bd = 0, d;
		// trace was unsuccessful, try guessing
		AngleVectors(ent->client->v_angle, forward, NULL, NULL);
		best = NULL;
		for (who = g_edicts+1; who->client; who++) {

			if (!who->inuse)
				continue;

			if (who->client->resp.team != TEAM_ALIEN)
				continue;

			if (who->health <= 0)
				continue;

			if (who == ent)
				continue;

			VectorSubtract(who->s.origin, ent->s.origin, dir);
			VectorNormalize(dir);
			d = DotProduct(forward, dir);
			if (d > bd && loc_CanSee(ent, who)) {
				if (who->client->resp.class_type == CLASS_GUARDIAN && who->s.modelindex == 0) {
					gi.WriteByte(svc_muzzleflash);
					gi.WriteShort(who - g_edicts);
					gi.WriteByte(MZ_NUKE1 | MZ_SILENCED);
					gi.unicast(ent, false);
				}
				
				bd = d;
				best = who;
			}
		}

		if (best && bd > 0.95)
		{
			//can't glow invis guard
			if (best->s.modelindex)
			{
				//don't override
				if (best->client->glow_time <= level.time)
					best->client->glow_time = level.time + .5f;
			}
		}
	}
}
