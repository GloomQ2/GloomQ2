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
#include "m_player.h"
#include "g_log.h"

#include <stdio.h>

void PMenu_Close(edict_t *ent);

int SV_FilterPacket (char *from);
int SV_GetTimeLeft (char *from);

#define EPSILON 0.0001	 // Define your own tolerance
#define FLOAT_EQ(x,v) (((v - EPSILON) < x) && (x <( v + EPSILON)))

#define MAX_CHEAT_CHECKING_ATTEMPTS 35

reconnect_t  reconnect[MAX_CLIENTS];

void CopyToBodyQue (edict_t *ent);

void wraith_dead_think (edict_t *ent);
void Updateteam_menu(void);

void SP_misc_teleporter_dest (edict_t *ent);

int stringContains(char *buff1, char *buff2);

/*QUAKED info_player_start (1 0 0) (-16 -16 -24) (16 16 32)
The normal starting point for a level.
*/
void SP_info_player_start(edict_t *self)
{
	self->svflags |= SVF_NOCLIENT;
}

/*void disconnect_byte (edict_t *ent)
{
	if (!ent->inuse)
		return;

	gi.WriteByte (0x07);
	gi.unicast (ent, true);
}*/

void CheckTKs (edict_t *ent)
{
	if (ent->client->resp.teamkills == -1 || level.intermissiontime)
		return;

	if ((ent->client->resp.teamkills >= tk_threshold->value || ent->client->resp.total_score < -25 || ent->client->resp.score < -5) && autokick->value) {
		int tks;
		tks = 0;

		if (ent->client->resp.team == TEAM_HUMAN)
			tks = ent->client->resp.kills[CLASS_ENGINEER] + ent->client->resp.kills[CLASS_GRUNT] + ent->client->resp.kills[CLASS_SHOCK] + ent->client->resp.kills[CLASS_BIO] + ent->client->resp.kills[CLASS_HEAVY] + ent->client->resp.kills[CLASS_COMMANDO] + ent->client->resp.kills[CLASS_EXTERM] + ent->client->resp.kills[CLASS_MECH];
		else
			tks = ent->client->resp.kills[CLASS_BREEDER] + ent->client->resp.kills[CLASS_HATCHLING] + ent->client->resp.kills[CLASS_DRONE] + ent->client->resp.kills[CLASS_WRAITH] + ent->client->resp.kills[CLASS_KAMIKAZE] + ent->client->resp.kills[CLASS_STINGER] + ent->client->resp.kills[CLASS_GUARDIAN] + ent->client->resp.kills[CLASS_STALKER];

		gi.bprintf (PRINT_HIGH, "%s was banned for teamkilling (%d tks - %d counter)\n", ent->client->pers.netname, tks, (int)ent->client->resp.teamkills);
		ban(ent, (int)banmaps->value, "auto-ban for teamkilling", NULL);
		ent->client->resp.teamkills = -1;
		kick (ent);
	}
}

void SendToTeam (int t, const char *format, ...)
{
	edict_t *ent = NULL;
	va_list		argptr;
	static char		string[1024];

	if (level.intermissiontime)
		return;

	va_start (argptr, format);
	Q_vsnprintf (string, sizeof(string)-1, format, argptr);
	va_end (argptr);

	for (ent=g_edicts+1 ; ent->client ; ent++)
	{
		if (ent->inuse && ent->client->pers.connected && ent->client->resp.team == t)
			gi.cprintf (ent, PRINT_HIGH, "%s", string);
	}
}

/*QUAKED info_player_deathmatch (1 0 1) (-16 -16 -24) (16 16 32)
potential spawning position for deathmatch games
*/
void spawnspot_die(edict_t *self, edict_t *inflictor __attribute__((unused)), edict_t *attacker, int damage __attribute__((unused)), vec3_t point __attribute__((unused))) //FV7 attributes
{

	// always allow killing ourselves and some MoDs always kill us
	if (attacker != self && (meansOfDeath != MOD_TRIGGER_HURT && meansOfDeath != MOD_CRUSH && meansOfDeath != MOD_LAVA)) {

		// if xmins, don't allow killing of map spawned teles
		if (self->enttype == ENT_TELEPORTER && (self->flags & FL_MAP_SPAWNED) && level.time < (60*xmins->value) && meansOfDeath != MOD_OUT_OF_MAP) {
			self->health = self->max_health;
			return;
		}
	}
	// **same type of code used in M_cocoon.c die**

	if (self->enttype == ENT_TELEPORTER && !(self->spawnflags & 1024) && !level.intermissiontime) {

		// active teleporter getting wasted

		if(attacker && attacker->client)
		{
			log_killstructure (attacker, self);

			gi.bprintf (PRINT_HIGH, "Human spawn destroyed by %s!\n", attacker->client->pers.netname);

			team_info.spawnkills++;

			if(attacker->client->resp.team == TEAM_HUMAN)
			{
				attacker->client->resp.score--;
				if (pointstest->value == 0)
					attacker->client->resp.total_score -= 10;
				attacker->client->resp.teamkills += 33;
				CheckTKs (attacker);
			}
			else if(attacker->client->resp.team == TEAM_ALIEN)
			{
				attacker->client->resp.kills[SPAWNKILLS]++;
				if(++attacker->client->resp.score > MAX_SCORE->value) {
					team_info.points[TEAM_ALIEN] -= 10;
					attacker->client->resp.score = MAX_SCORE->value;
				}
				if (pointstest->value == 0)
					attacker->client->resp.total_score += 10;
			}
		}
		else {
			SendToTeam (TEAM_HUMAN, "Human spawn destroyed!\n");
		}

		// no minus teles from unfinished
		if (!(self->spawnflags & 1024))
			team_info.spawns[TEAM_HUMAN]--;
	}

	//always display fireworks
	gi.WriteByte (svc_temp_entity);

	if (self->groundentity && attacker && !(attacker->flags & FL_CLIPPING))
		gi.WriteByte (TE_GRENADE_EXPLOSION);
	else
		gi.WriteByte (TE_ROCKET_EXPLOSION);

	gi.WritePosition (self->s.origin);
	gi.multicast (self->s.origin, MULTICAST_PVS);

	// give points back (unless special)
	if (!(self->spawnflags & 16384))
	{
		if (attacker && attacker->client && attacker->client->resp.team == TEAM_ALIEN)
			team_info.buildpool[TEAM_HUMAN] += COST_TELEPORT;
		else
			team_info.points[TEAM_HUMAN] -= COST_TELEPORT;
	}

	if (self->target)
		G_UseTargets (self, attacker);

	G_FreeEdict (self);
}

void spawn_spark (edict_t *ent)
{
	int fx = (randomMT() % 3) + 5;
	vec3_t temp;
	VectorCopy (ent->s.origin, temp);
	temp[2] -= 8;

	gi.sound (ent, CHAN_AUTO, gi.soundindex(va("world/spark%d.wav", fx)), 1, ATTN_IDLE, 0);

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_SPARKS);
	gi.WritePosition (temp);
	gi.WriteDir (ent->velocity);
	gi.multicast (temp, MULTICAST_PVS);

	/*gi.WriteByte(svc_temp_entity);
	gi.WriteByte(TE_WELDING_SPARKS);
	gi.WriteByte(24);
	gi.WritePosition(ent->s.origin);
	gi.WriteDir(ent->velocity);
	gi.WriteByte(226);
	gi.multicast(ent->s.origin, MULTICAST_PVS);*/

	ent->nextthink = level.time + 3 + random() * 2 + crandom()*3;
}

void spawn_pain (edict_t *self, edict_t *other __attribute__((unused)), float kick __attribute__((unused)), int damage __attribute__((unused))) //FV7 attributes
{
	// no pain to map spawns during xmins
	if (self->enttype == ENT_TELEPORTER && (self->flags & FL_MAP_SPAWNED) && (level.time < 60*xmins->value))
		return;

	self->teleport_time = level.time + 5;

	// optimization: check for think
	if (!self->think && self->health < self->max_health)
	{
		self->s.effects = 0;	
		self->think = spawn_spark;
		self->nextthink = level.time + 5 + crandom() * 4;
	}
}

void SP_info_player_deathmatch_use (edict_t *self, edict_t *other __attribute__((unused)), edict_t *activator __attribute__((unused))) //FV7 attributes
{
	if (self->enttype == ENT_TELEPORTER_D) {
		gi.unlinkentity (self);

		//quickly hack it so a killbox will actually kill stuff
		VectorSet (self->maxs, 24, 24, 16);

		//splat
		KillBox (self);

		//back to normal thank you...
		VectorSet (self->maxs, 24, 24, -16);

		self->takedamage = DAMAGE_YES;
		self->teleport_time = level.time;

		self->s.effects = EF_TELEPORTER;
		self->s.renderfx |= RF_IR_VISIBLE;

		self->enttype = ENT_TELEPORTER;

		self->svflags &= ~SVF_NOCLIENT;

		team_info.spawns[TEAM_HUMAN]++;

		AddToSpawnlist(self, TEAM_HUMAN);

	} else {

		if (self->spawnflags & 16) {
			self->takedamage = DAMAGE_YES;
		} else {
			self->takedamage = DAMAGE_NO;
		}

		self->s.effects = 0;
		self->s.renderfx = 0;

		self->enttype = ENT_TELEPORTER_D;

		self->svflags |= SVF_NOCLIENT;

		team_info.spawns[TEAM_HUMAN]--;
	}

	gi.linkentity (self);
}

void SP_info_player_deathmatch (edict_t *self)
{
	if (st.spawnteam && team_info.spawnteam && !(team_info.spawnteam & st.spawnteam)){
		G_FreeEdict(self);
		return;
	}

	if (st.classes)
		self->style = st.classes;

	if (st.hurtflags)
		self->hurtflags = st.hurtflags;

	gi.setmodel (self, "models/objects/dmspot/tris.md2");
	self->flags |= FL_CLIPPING;
	self->solid = SOLID_BBOX;
	self->movetype = MOVETYPE_STEP;
	self->pain = spawn_pain;
	self->max_health = self->health = 215;
	self->die = spawnspot_die;
	self->mass = 800;

	if (Q_stricmp (level.mapname, "chaos") && Q_stricmp (level.mapname, "palpha") && !(gi.pointcontents (self->s.origin) & (CONTENTS_AUX|CONTENTS_MIST)))
		self->s.origin[2] += 16;

	VectorSet (self->mins, -24, -24, -24);
	VectorSet (self->maxs, 24, 24, -16);

	if (self->spawnflags & 4) {
		self->damage_absorb = 2000;
		self->flags |= FL_NO_KNOCKBACK;
	} else {
		self->damage_absorb = 30;
	}

	self->use = SP_info_player_deathmatch_use;

	CheckSolid (self);

	self->s.skinnum = 1;

	if (self->spawnflags & 8) {
		// disabled tele

		self->enttype = ENT_TELEPORTER_D;

		self->svflags |= SVF_NOCLIENT;

		if (self->spawnflags & 16) {
			self->takedamage = DAMAGE_YES;
		} else {
			self->takedamage = DAMAGE_NO;
		}
	} else {
		self->takedamage = DAMAGE_YES;
		self->s.effects = EF_TELEPORTER;
		self->s.renderfx |= RF_IR_VISIBLE;

		if (!self->enttype) {
			self->flags |= FL_MAP_SPAWNED;
			self->classname = "info_player_deathmatch";

			if (!level.framenum)
			{
				team_info.maxpoints[TEAM_HUMAN] -= COST_TELEPORT;

				//randomize when players can spawn at the start of the game, prevents
				//a whole ton of configstrings simultaneously.

				//FIXME: maybe after they choose team set configstring to most likely class?
				self->teleport_time = level.time + reconnect_wait->value + random() * 5;
			}
		}

		self->enttype = ENT_TELEPORTER;

		AddToSpawnlist(self, TEAM_HUMAN);

		team_info.spawns[TEAM_HUMAN]++;
	}

	gi.linkentity (self);
}

/*QUAKED info_player_intermission (1 0 1) (-16 -16 -24) (16 16 32)
The deathmatch intermission point will be at one of these
Use 'angles' instead of 'angle', so you can set pitch or roll as well as yaw.	'pitch yaw roll'
*/
void SP_info_player_intermission(edict_t *self)
{
	CheckSolid (self);
}

void player_pain (edict_t *self __attribute__((unused)), edict_t *other __attribute__((unused)), float kick __attribute__((unused)), int damage __attribute__((unused))) //FV7 attributes
{
	// player pain is handled at the end of the frame in P_DamageFeedback
}

void Bprintf (const char *format, ...)
{
	edict_t	*ent;
	va_list		argptr;
	static char		string[1024];

	va_start (argptr, format);
	Q_vsnprintf (string, sizeof(string)-1, format, argptr);
	va_end (argptr);

	for (ent=g_edicts+1 ; ent->client ; ent++)
	{
		if (ent->inuse && ent->client->pers.connected && ent->client->pers.vid_done) {
			if (atoi(Info_ValueForKey(ent->client->pers.userinfo, "msg")) == 1) {
				if (stringContains (string, ent->client->pers.netname)) {
					gi.cprintf (ent, PRINT_HIGH, "%s", string);
				}
			} else {
				gi.cprintf (ent, PRINT_HIGH, "%s", string);
			}
		}
	}

	//show console
	if (dedicated->value)
		gi.cprintf (NULL, PRINT_MEDIUM, "%s", string);
}

static const char *obits[] = {
"%s died.\n",
"%s was capped by %s.\n",
"%s was gunned down by %s.\n",
"%s was blown away by %s's scattergun.\n",
"%s was machinegunned by %s.\n",
"%s felt the heat of %s fire.\n",
"%s was shredded by %s's shrapnel.\n",
"%s ate %s explosion.\n",
"%s almost dodged %s's blast.\n",
"%s was gibified by %s's autocannon.\n",
"%s was spiked by %s.\n",
"%s gagged on %s's gas.\n",
"%s sank like a rock.\n",
"%s melted.\n",
"%s visits the Volcano God.\n",
"%s was squished.\n",
"%s was telefragged by %s.\n",
"%s cratered.\n",
"%s suicides.\n",
"%s got lasered.\n",
"%s blew up.\n",
"%s blew up.\n",
"%s blew up.\n",
"%s found a way out.\n",
"%s was in the wrong place.\n", //MOD_SPLASH
"%s saw the light.\n",
"%s was in the wrong place.\n", //MOD_TRIGGER_HURT
"%s was squished by %s.\n",
"%s was disintegrated by %s's laser.\n",
"%s broke his mech.\n",
"%s was spit upon by %s.\n",
"%s died.\n",
"%s was slashed by %s.\n",
"%s sat on an egg.\n",
"%s got healed.\n",
"%s was electrocuted by %s's mech.\n",
"%s short circuited.\n",
"%s got a taste of %s's magnum.\n",
"%s was caught in %s's blast.\n",
"%s didn't watch where %s was going.\n", // he,she,it
"%s got infested.\n",
"%s was poked to death by %s.\n",
"%s felt %s's revenge.\n",
"%s was shocked by %s.\n",
"%s blew up.\n",
"",
"%s got owned by %s's breeder.\n",
"%s blew his power pack.\n",
"%s was mown down by %s.\n",
"%s was slashed by %s.\n",
"%s was machinegunned by %s.\n",
"%s ate %s's explosive death.\n",
"%s enters orbit thanks to %s's C4.\n",
"%s was spiked by %s.\n",
"%s was poisoned by %s.\n",
"%s was vacuum-packaged.\n",
"%s was shot to bits.\n",
"%s step on wrong bug.\n", //"%s felt %s's aftermath",
"%s was atomized by %s's laser pulse.\n",
"%s felt the wrath of the evil depot.\n",
"%s got gassed.\n",
"%s was railed.\n",
"%s got blasted.\n",
"%s was squished by %s's mech.\n",
"",
"%s ate some of %s's shrapnel.\n",
/*Savvy custom MODs required*/
"%s ate %s's big magnum.\n", /*explosive magnum shells*/
#ifdef HATCHLING_DO_RADIUS_DAMAGE
"%s was slashed by %s\n", /*hatchling radius damage*/
#endif
#ifdef SAVVY_CONTENT
"%s was frozen in time by %s.\n", /*deep freeze gun from new class*/
"%s stepped on %s's mine.\n", /*proxy mine radius damage*/
"%s got hit by %s's shrapnel\n" /*proxy mine shrapnel*/
#endif

};

void ClientObituary (edict_t *self, edict_t *inflictor __attribute__((unused)), edict_t *attacker) { //FV7 attribute
	int			mod;
	const char	*message=NULL;
	char		*message2;

	//r1: ignore frags, messages, etc during intermission
	if (level.intermissiontime)
		return;

	if (((!attacker->client || attacker == self) && meansOfDeath != MOD_INFEST) && level.time - self->client->last_damage <= 1) {
		attacker = self->client->last_attacker;
		meansOfDeath = self->client->last_mod;
	}

	mod = meansOfDeath;

	if (!attacker->client || attacker == self) {
		// killed by normal ent or self

		// if player can kill himself with a weapon he has,
		// this switch cause needs to have it handled or
		// obit breakage will happen
		switch (mod)
		{
		case MOD_SUDDENDEATH:
			message = "%s expired.\n";
			break;
		case MOD_TARGET_KILL: // former mod_map
		case MOD_TRIGGER_HURT:
		case MOD_EXPLOSIVE: // used in few places for world generated explosion
			if (attacker->message && attacker->message[0])
				Bprintf ("%s %s.\n", self->client->pers.netname, attacker->message);
			else
				message = obits[mod];
			break;
		/** start of some target_blaster obits **/
		case MOD_BURNED:
			message = "%s got a little too hot.\n";
			break;
		case MOD_ROCKET:
			message = "%s caught a rocket.\n";
			break;
		case MOD_TARGET_BLASTER:
			message = "%s got blasted.\n";
			break;
		case MOD_ACID:
			message = "%s was slimed.\n";
			break;
		// these also used for spikers
		case MOD_SPIKESPORE:
		case MOD_SPIKE:
			if (!attacker->client)
				message = "%s was spiked.\n";
			else
				message = "%s spiked himself.\n";
			break;
		case MOD_HYPERBLASTER: // also used for mech shooting himself in foot
			message = obits[MOD_MECH];
			break;
		// in addition: mod_mgturret, mod_explosive, mod_grenade, mod_r_splash
		case MOD_GRENADE:
			if (attacker == self) {
				if (self->client->resp.class_type != CLASS_BIO)
					message = "%s tripped on his own grenade.\n";
				else
					message = "%s tripped on her own grenade.\n";
			} else {
				message = "%s was shredded to a grenade.\n";
			}
			break;
		case MOD_TELEFRAG:
			message = "%s was telefragged.\n";
			break;
		case MOD_R_SPLASH:
			if (attacker == self) {
				if (self->client->resp.class_type != CLASS_BIO)
					message = "%s blew himself up.\n";
				else
					message = "%s blew herself up.\n";
			} else {
				message = "%s was caught in rocket explosion.\n";
			}
			break;
		/** end of some target_blaster obits **/
		case MOD_POISON: // like the acid drops in speedse
			message = "%s was poisoned.\n";
			break;
		case MOD_EAT_C4:

			if (randomMT()&1)
				message = "%s sacrificed itself for the swarm.\n";
			else
				message = "%s ate a spicy meatball.\n";

			break;
		case MOD_OBSTACLE:
			if (self->client->resp.class_type == CLASS_BIO)
				message2 = "she";
			else
				message2 = "he";

			Bprintf (obits[mod], self->client->pers.netname, message2);
			break;
		case MOD_CHANGECLASS:
			// actually, reduce pts if changing class to same if alive
			// and grunt
			return;
		case MOD_C4:
			message = "%s couldn't escape from his own C4.\n";
			break;
		case MOD_KAMIKAZE:
			message = "%s exploded itself.\n";
			break;
		case MOD_FLASH:
			message = "%s saw a bright light.\n";
			break;
/*Savvy custom bot message, when it kills you with melee*/
		case MOD_SPBOT:
		  message = "%s got a little too close to a bot\n";
		  break;
		case MOD_SUICIDE:
			self->client->resp.total_score -= (classlist[self->client->resp.class_type].frags_needed + 1);
			//fall through
		default:
			message = obits[mod];
			break;
		}

		if (deathmessages->value && self->client->resp.visible && !level.suddendeath) {
			if (message)
				Bprintf (message, self->client->pers.netname);
				//gi.bprintf (PRINT_LOW, "%s %s.\n", self->client->pers.netname, message);
		}

		//r1: ignore during intermission
		if (!level.intermissiontime) {
			//self->client->resp.kills[PLAYERDEATHS]++;
			log_death (attacker, self, mod);
			team_info.bodycount[self->client->resp.team]++;
		}

		//moved c4 guardian eat code here, give frags on death rather than at eat so
		//you dont' end up with 12 or whatever. only 1 frag due to kamiguard fun :)

		if (pointstest->value)
			self->client->resp.total_score -= classlist[self->client->resp.class_type].frags_needed * 2;

		if ((mod == MOD_R_SPLASH || mod == MOD_GRENADE || mod == MOD_SUICIDE || mod == MOD_C4) && attacker->client) {
			if (deathtest->value) {
				self->client->resp.score -= classlist[self->client->resp.class_type].frags_needed;
			} else {
				self->client->resp.score--;
				self->client->resp.total_score--;
			}
		}

		self->enemy = NULL;

	} else {

		// killed by other client
		self->enemy = attacker;
		switch (mod)
		{

		//ass hack
		case MOD_INFEST:
			//FIXED: shouldn't break; or you will add to score. return needed.
			attacker->client->resp.total_score++; // special case
			if (deathmessages->value && self->client->resp.visible && attacker->client->resp.visible && !level.suddendeath)
				Bprintf (obits[mod], self->client->pers.netname);
			return;

		case MOD_CORPSE_EXPLOSION:
			// special case
			if (deathmessages->value && self->client->resp.visible && attacker->client->resp.visible && !level.suddendeath)
				Bprintf ("%s felt %s's aftermath.\n", self->client->pers.netname, attacker->client->pers.netname);
			if (self->client->resp.team == TEAM_HUMAN) {
				attacker->client->resp.score++;
				if(attacker->client->resp.score > MAX_SCORE->value)
					attacker->client->resp.score = MAX_SCORE->value;
			}
			return;

		default:
			message = obits[mod];
			break;
		}

		if (deathmessages->value && self->client->resp.visible && attacker->client->resp.visible && !level.suddendeath) {
			if (message)
				Bprintf (message, self->client->pers.netname, attacker->client->pers.netname);
		}

		//r1: ignore during intermission
		if (!level.intermissiontime) {
			attacker->client->resp.kills[self->client->resp.class_type]++;
			team_info.bodycount[self->client->resp.team]++;
			log_death (attacker, self, mod);
		}

		if (pointstest->value)
			self->client->resp.total_score -= classlist[self->client->resp.class_type].frags_needed;


		if (attacker->client->resp.team == self->client->resp.team) {

			// idea: just reduce personal pts from TK and when they go too low,
			// kick the player
			// result: you can TK, but it reduces your points
			// extra: reduce based on class' TK ability (intentional or not)

			if (mod != MOD_TELEFRAG) {
				// no TK checking if full teamdamage
				if (teamdamage->value < 2) {
					int tkcost;

					switch (classlist[self->client->resp.class_type].frags_needed) {
						case 1:
						case 2:
						case 3:
						case 4:
						case 5:
							tkcost = classlist[self->client->resp.class_type].frags_needed * 10;
							break;
						default:
							tkcost = 5;
							break;
					}

					attacker->client->sudden_death_frame -= 150;

					attacker->client->resp.teamkills += tkcost;
					attacker->client->resp.total_score -= (classlist[self->client->resp.class_type].frags_needed + 1) * 2;

					if (classlist[self->client->resp.class_type].frags_needed) {
						if (attacker->client->resp.score > 0)
							attacker->client->resp.score -= 2;
						else
							attacker->client->resp.score --;//= classlist[self->client->resp.class_type].frags_needed;
					}

					if (self->client->resp.visible)
						gi.bprintf (PRINT_HIGH, "%s teamkilled!\n", attacker->client->pers.netname);

					CheckTKs (attacker);
				}
			}
		} else {
			// gloom: no frags as stalker or mech
			if (self->client->resp.class_type == CLASS_GUARDIAN && self->client->grenade_blow_up > 0)
				attacker->client->resp.score += self->client->grenade_blow_up;

			if ( (attacker->client->resp.class_type != CLASS_MECH &&
				attacker->client->resp.class_type != CLASS_STALKER) || classlist[self->client->resp.class_type].frags_needed >= 3)
				//(classlist[attacker->client->resp.class_type].frags_needed ==
				//classlist[self->client->resp.class_type].frags_needed))
			{
				if (!(self->client->resp.class_type == CLASS_HATCHLING && nohtfragtest->value &&
					mod == MOD_R_SPLASH))
				{
					self->client->resp.kills[PLAYERDEATHS]++;
					if (scoring_mode->value == 0)
						attacker->client->resp.score++;
					else if (scoring_mode->value == 1)
						attacker->client->resp.score += classlist[self->client->resp.class_type].frags_needed + 1;
					else if (scoring_mode->value == 2) {
						attacker->client->resp.floatscore += (int)(((((float)classlist[self->client->resp.class_type].frags_needed + 1.0) / ((float)classlist[attacker->client->resp.class_type].frags_needed + 1.0))));
						if (attacker->client->resp.floatscore >= 1.0) {
							attacker->client->resp.score = (int)attacker->client->resp.floatscore;
							attacker->client->resp.floatscore -= attacker->client->resp.score;
						}
					}
					if (self->client->resp.total_score > 0)
						self->client->resp.total_score--;
				}
			}

/*Savvy sudden death area can go here, the part for kills anyways*/

			attacker->client->sudden_death_frame += 300;

			//r1: killing enemies is good, reduce tk count
			attacker->client->resp.teamkills -= 6;
			if (attacker->client->resp.teamkills < 0)
				attacker->client->resp.teamkills = 0;


			if (pointstest->value)
				//your score is increase by [enemy frag cost +1] divided by [your frag cost +1]
				attacker->client->resp.total_score += (int)((float)(classlist[self->client->resp.class_type].frags_needed+1) / (float)(classlist[attacker->client->resp.class_type].frags_needed+1));
			else
				attacker->client->resp.total_score += classlist[self->client->resp.class_type].frags_needed + 1;

			// add to builder pts if max score
			if(attacker->client->resp.score > MAX_SCORE->value) {
				attacker->client->resp.score = MAX_SCORE->value;
				if (attacker->client->resp.class_type != CLASS_MECH && attacker->client->resp.class_type != CLASS_STALKER)
				{
					if (classlist[self->client->resp.class_type].frags_needed < 1)
						team_info.points[attacker->client->resp.team]-= 2;
					else if (classlist[self->client->resp.class_type].frags_needed < 3)
						team_info.points[attacker->client->resp.team]-= 4;
					else
						team_info.points[attacker->client->resp.team]-= 6;

				}
			}
		}
	}
}

//void Touch_Item (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);

/*
LookAtKiller
*/
void LookAtKiller (edict_t *self, edict_t *inflictor, edict_t *attacker)
{
	vec3_t		dir;

	if (attacker && attacker != world && attacker != self)
	{
		VectorSubtract (attacker->s.origin, self->s.origin, dir);
	}
	else if (inflictor && inflictor != world && inflictor != self)
	{
		VectorSubtract (inflictor->s.origin, self->s.origin, dir);
	}
	else
	{
		self->client->killer_yaw = self->s.angles[YAW];
		return;
	}
	self->client->killer_yaw = 180/M_PI*atan2(dir[1], dir[0]);

	// set death viewangles
	self->client->ps.viewangles[ROLL] = 40;
	self->client->ps.viewangles[PITCH] = -15;
	self->client->ps.viewangles[YAW] = self->client->killer_yaw;
}

char *TimeToString (int secs)
{
	int days = 0;
	int hours = 0;
	int mins = 0;
	int	years = 0;
	int weeks = 0;
	static int index;
	static char bannmsg[4][64];
	char	*banmsg;

	// use an array so that multiple vtos won't collide
	banmsg = bannmsg[index];
	index = (index + 1)&3;

	banmsg[0] = '\0';

	if (secs < 0) {
		strcpy (banmsg, "eternity");
		return banmsg;
	}

	while (secs/60/60/24/365 >= 1) {
		years++;
		secs -= 60*60*24*365;
	}

	if (years > 0) {
		strcat (banmsg, va("%d year",years));
		if (years != 1)
			strcat (banmsg, "s");
	}

	while (secs/60/60/24/7 >= 1) {
		weeks++;
		secs -= 60*60*24*7;
	}

	if (weeks > 0) {
		if (banmsg[0])
			strcat (banmsg, va(", %d week",weeks));
		else
			strcat (banmsg, va("%d week", weeks));

		if (weeks != 1)
			strcat (banmsg, "s");
	}

	while (secs/60/60/24 >= 1) {
		days++;
		secs -= 60*60*24;
	}

	if (days > 0) {
		if (banmsg[0])
			strcat (banmsg, va(", %d day",days));
		else
			strcat (banmsg, va("%d day", days));

		if (days != 1)
			strcat (banmsg, "s");
	}

	while (secs/60/60 >= 1) {
		hours++;
		secs -= 60*60;
	}
	if (hours > 0) {
		if (banmsg[0])
			strcat (banmsg, va(", %d hour",hours));
		else
			strcat (banmsg, va("%d hour", hours));

		if (hours != 1)
			strcat (banmsg, "s");
	}

	while (secs/60 >= 1) {
		mins++;
		secs -= 60;
	}
	if (mins > 0) {
		if (banmsg[0])
			strcat (banmsg, va(", %d min",mins));
		else
			strcat (banmsg, va("%d min", mins));

		if (mins != 1)
			strcat (banmsg, "s");
	}

	if (secs > 0) {
		if (banmsg[0])
			strcat (banmsg, va(", %d sec",secs));
		else
			strcat (banmsg, va("%d sec", secs));

		if (secs != 1)
			strcat (banmsg, "s");
	}

	if (!*banmsg)
		strcpy (banmsg, "0 secs");

	return banmsg;
}

void UnlinkClientEnt (edict_t *self, int damage __attribute__((unused))) //FV7 attribute
{
	VectorClear (self->mins);
	VectorClear (self->maxs);

	self->s.frame = 0;
	self->s.skinnum = 0;

	self->takedamage = DAMAGE_NO;
	self->clipmask = MASK_SOLID;
	self->solid = SOLID_NOT;

	self->movetype = MOVETYPE_BOUNCE;

	self->velocity[0] += crandom() * 50;
	self->velocity[1] += crandom() * 50;
	self->velocity[2] += crandom() * 50;

	if (self->client)
	{
		self->client->anim_priority = ANIM_DEATH;
		self->client->anim_end = self->s.frame;
	}

	self->svflags |= SVF_NOCLIENT;
	self->nextthink = 0;
	self->think = NULL;
	self->touch = NULL;
	gi.unlinkentity (self);
}

/*
player_die
*/
void C4_Timer (edict_t *c4);
void C4_Explode (edict_t* self);
edict_t *C4_Arm (edict_t* self);
void c4_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf); /*Savvy needed for "new" c4*/

//void CopyToBodyQue (edict_t *ent);
void player_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point __attribute__((unused))) //FV7 attribute
{
//	int		count;
	int		gib_health;
	int		gibtype;
	vec3_t		start;
	qboolean	wasflying=false;
	//edict_t *e;

	//player isn't dead at all, first call (or should be)
	if (!self->deadflag)
	{
		//leave this, prevents multideath loop bug
		self->deadflag = DEAD_DEAD;

		PMenu_Close(self);

		// clear angle velocities
		VectorClear (self->avelocity);

		// reset default speeds in prep for respawn
		SetClientVelocity (self, DEFAULT_VELOCITY);
		self->client->slow_timer = 0;
		self->client->slow_last_jump_framenum = 0;
		stuffcmd (self, va("set cl_upspeed %d\n", DEFAULT_VELOCITY));

		// server visible stuff starts
		self->clipmask = MASK_SHOT; // seems not have any effect
		self->svflags = SVF_DEADMONSTER;

		// level the body
		self->s.angles[0] = 0;
		self->s.angles[2] = 0;

		// server visible stuff ends
		self->movetype = MOVETYPE_TOSS;
		self->client->last_move_time = 0;
		self->client->build_timer = 0;

		// clear spikers aiming at client
		self->client->spikes = 0;

		//stop them doing damage
		self->touch = NULL;

		//shut up
		self->client->weapon_sound = 0;

		//how long before they can respawn
		self->client->resp.respawn_time = level.time;

		//flying causes different death anim
		wasflying = (self->client->resp.flying) ? true : false; /*used to be 1 : 0, IDIOTS, LOOK AT DATA TYPES*/
		self->client->resp.flying = 0;

		// remove powerups
		self->client->invincible_framenum = 0;

		// drop c4 armed
		if (self->client->resp.primed && level.time > self->client->throw_time)
		{
		/*Savvy edit to let the coder decide easier
    if (random() < .25)
    */
			if (random() <= COMMANDO_C4_DROP_ON_DEATH_PROBABILITY)
			{
				edict_t *c4ent;
				const gitem_t *c4 = FindItem ("C4 Explosive");
				self->client->resp.inventory[ITEM_INDEX(c4)]--;
				c4ent = C4_Arm (self);

				//it goes flying off as he falls dropping it
				c4ent->velocity[0] = crandom() * 300;
				c4ent->velocity[1] = crandom() * 300;
				c4ent->velocity[2] = random() * 200;

				c4ent->avelocity[0] = crandom() * 180;
				c4ent->avelocity[1] = crandom() * 180;
				c4ent->avelocity[2] = crandom() * 180;
			}
			self->client->resp.primed = 0;
		}
		
		/*Savvy martyrdom upgrade - it goes off if you don't suicide or changeclass*/
		if (((meansOfDeath != MOD_SUICIDE) && (meansOfDeath != MOD_CHANGECLASS)) && (self->client->resp.class_type == CLASS_HATCHLING) && (self->client->resp.upgrades & UPGRADE_HATCHLING_MARTYRDOM)) {
		MarytrdomSpikes(self);
		}

		// remove weapon
		self->client->newweapon = NULL;
		ChangeWeapon (self);

		// free flashlight
		if (self->client->resp.flashlight) {
		G_FreeEdict(self->client->resp.flashlight);
		self->client->resp.flashlight = NULL;
		}

		//clear any poison
		self->client->acid_damage = self->client->acid_duration = 0;
		self->client->acid_attacker = NULL;

		// reset grapple
		if (self->client->ctf_grapple)
			CTFResetGrapple(self->client->ctf_grapple);

		// change view
		LookAtKiller (self, inflictor, attacker);

		// display message, update scores, check tks.
		ClientObituary (self, inflictor, attacker);

		// no items
		self->client->resp.selected_item = 0;

		//r1ch: drop any key
		if (self->client->resp.key)
			self->client->resp.key->item->drop (self, self->client->resp.key->item);

		self->client->resp.key = NULL;

		// prevents guardian blowing detpack with 'kill'
		if (meansOfDeath == MOD_SUICIDE)
			self->client->grenade_blow_up = 0;
		
		//save enemy
		self->enemy = attacker;

		// detonates c4ed guard if shot
		if(self->client->grenade_blow_up > 0)
		{
			T_RadiusDamage(self,self,280*(self->client->grenade_blow_up),self,50*(self->client->grenade_blow_up),MOD_EAT_C4, 0);
			T_RadiusDamage(self,self,180*(self->client->grenade_blow_up),self,180*(self->client->grenade_blow_up),MOD_EAT_C4, 0);
			T_RadiusDamage(self,self,25*(self->client->grenade_blow_up),self,500*(self->client->grenade_blow_up),MOD_EAT_C4, 0);
			VectorCopy(self->s.origin, start);
			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_EXPLOSION2);
			gi.WritePosition (start);
			gi.multicast (start, MULTICAST_PVS);
			gi.sound(self,CHAN_AUTO,SoundIndex (voice_toastie),1,ATTN_NORM,0);
			//gi.sound(self,CHAN_AUTO,SoundIndex (voice_toastie),1,ATTN_NORM,0);
			//gi.sound(self,CHAN_AUTO,SoundIndex (voice_toastie),1,ATTN_NORM,0);
			self->health = (classlist[self->client->resp.class_type].gib_health - random()*50);
			self->client->grenade_blow_up = 0;
		}

		if (meansOfDeath != MOD_SUICIDE && meansOfDeath != MOD_CHANGECLASS && self->client->resp.class_type == CLASS_MECH && (self->client->resp.upgrades & UPGRADE_DOOMSDAY_DEVICE)) {
/*Savvy old doomsday was a bit lame*/
#ifdef OLD_DOOMSDAY_C4
		edict_t *c4 = G_Spawn();
		c4->think = C4_Timer;
		c4->owner = self;
		VectorCopy (self->s.origin, c4->s.origin);
		C4_Explode (c4);
#else
/*Savvy i added in this lovely section, i took most of the code from a special c4.c ?*/
    int i;
    int timer;
    trace_t tr;
    edict_t *c4;
    edict_t *warnwho;
    vec3_t mins = {-8,-8,-6};
    vec3_t maxs = {8,8,8};
    vec_t oldspot;
    
      for (i = 0; i < NEW_DOOMSDAY_C4_COUNT; i++) {
      /*Savvy used to be 10, let's make them each detonate, 1 second after each, so constantly for a while*/
      timer = (NEW_DOOMSDAY_C4_DELAY + (i * NEW_DOOMSDAY_C4_INTERVAL));
      
      c4 = G_Spawn();
      VectorCopy(self->s.origin, c4->s.origin);
      
      /*Savvy gives it a nice stack*/
      c4->velocity[2] = (25 + (i * 25));
      c4->avelocity[YAW] = i+50;
      
      /*Savvy make sure we don't spawn in other c4*/
      oldspot = c4->s.origin[2];
      c4->s.origin[2] += ((maxs[2] + mins[2]) / 2);
      
      /*Savvy make sure not in a ceiling / ent, by going down, doesn't matter if we are within another ent at that point*/
      tr = gi.trace (c4->s.origin, mins, maxs, c4->s.origin, c4, MASK_ALL);
        if (tr.startsolid) { c4->s.origin[2] = oldspot; }
      
      c4->owner = self;
      c4->s.modelindex = gi.modelindex("models/objects/c4/tris.md2");
      c4->flags |= FL_CLIPPING;
      c4->touch = c4_touch;
      c4->movetype = MOVETYPE_STEP;
      c4->solid=SOLID_BBOX;
      VectorCopy(mins,c4->mins);
      VectorCopy(maxs,c4->maxs);
      c4->think = C4_Timer;
      c4->count = timer;
      c4->nextthink = level.time + FRAMETIME;
      c4->s.renderfx |= RF_IR_VISIBLE;
      c4->classname = "c4";
      //c4->dmg_radius = 800;
      //c4->radius_dmg = 700;
      c4->takedamage = DAMAGE_NO;
      
      c4->teamchain = G_Spawn();
      VectorCopy(c4->s.origin, c4->teamchain->s.origin);
      c4->teamchain->owner=self;
      c4->teamchain->touch=c4_touch;
      c4->teamchain->movetype = MOVETYPE_NOCLIP;
      c4->teamchain->solid = SOLID_NOT;
      VectorCopy(mins,c4->teamchain->mins);
      VectorCopy(maxs,c4->teamchain->maxs);
      c4->teamchain->classname = "spinninglights";
      c4->teamchain->s.modelindex = shiny_index;
      //c4->teamchain->s.skinnum = 0;
      c4->teamchain->s.effects |= EF_SPINNINGLIGHTS;
      c4->teamchain->think = G_FreeEdict;
      c4->teamchain->nextthink = (level.time + (timer + 1));

      c4->enttype = ENT_C4;

      gi.linkentity(c4);
      gi.linkentity(c4->teamchain);
      }
    /*Savvy acoording to the function, we could skip a bunch of players if we specify parameter 1*/
      while ((warnwho = findradius_c(NULL, c4, 250))!=NULL) {
      gi.cprintf(warnwho,PRINT_HIGH,"%s\n", colortext("RUN!"));
      }
    gi.bprintf(PRINT_HIGH, "%s stacked the oods against the aliens\n", colortext(self->client->pers.netname));
#endif
		}

		if (self->client->resp.class_type == CLASS_ENGINEER && (self->client->resp.upgrades & UPGRADE_PHOBOS_REACTOR)) {
		self->dmg = 150;
		self->dmg_radius = 180;
		}

		self->client->resp.upgrades = 0;

		gib_health = classlist[self->client->resp.class_type].gib_health;

		//r1: we don't want any bodies in high activity pvs areas
		//ankka: this method is slow as hell => dll lags
		//ankka: small opt: globals.num_edicts
		/*e = NULL;
		count = 0;
		for (e = g_edicts + game.maxclients + 1; e < &g_edicts[globals.num_edicts+1]; e++) {
			if (!e->inuse)
				continue;
			if (e->svflags & SVF_NOCLIENT)
				continue;
			if (e->s.modelindex == 0)
				continue;
			if (gi.inPVS (e->s.origin, self->s.origin))
				count++;
		}*/

		// make sure kamikaze_explode runs gibcode
		if ( (self->client->resp.class_type == CLASS_KAMIKAZE && meansOfDeath == MOD_KAMIKAZE) ||
			 (meansOfDeath == MOD_INFEST && random() > 0.666))
			self->health = gib_health - random() * 20;

		// prevent hatch/kami gibbing anyone with jump attack
		// leaves more bodies to guards
		/*if (meansOfDeath == MOD_JUMPATTACK && attacker->client && 
			(attacker->client->resp.class_type == CLASS_HATCHLING ||
			 attacker->client->resp.class_type == CLASS_KAMIKAZE))
			gib_health = self->health-1;*/

		//if (self->health <= gib_health || meansOfDeath == MOD_INFEST || (count >= 25 && self->deadflag != DEAD_DEAD)) {
		if (meansOfDeath == MOD_CHANGECLASS)
		{
			UnlinkClientEnt (self, 0);
		}
		else
		{
			if (self->health <= gib_health)
			{
				// gib the player, were DEAD. further calls to player_die should never happen
				self->deadflag = DEAD_DEAD;

				if ((self->client->resp.class_type==CLASS_ENGINEER)||(self->client->resp.class_type==CLASS_MECH))
					gibtype=GIB_METALLIC;
				else
					gibtype=GIB_ORGANIC;

				//clienthead is a gib already
				//but clienthead has no l33t blood trail :D
				if (self->client->resp.class_type != CLASS_KAMIKAZE)
				{
					IntelligentThrowGib (self, self->headgib, damage, gibtype, 1, 2);
					IntelligentThrowGib (self, self->gib1, damage, gibtype, 0, 1);
					IntelligentThrowGib (self, self->gib2, damage, gibtype, 0, 1);
				}

				if(bandwidth_mode->value)
					ThrowGib (self, self->gib1, damage, gibtype);

				if(bandwidth_mode->value == 2.0)
					ThrowGib (self, self->gib2, damage, gibtype);

				if (self->client->resp.class_type == CLASS_KAMIKAZE && meansOfDeath != MOD_KAMIKAZE)
				{
					classlist[self->client->resp.class_type].diefunc(self);
				}

				//gibs doesn't sound right with engi.. need a little explosion (YES TWO SOUNDS SO SUE ME... ONES A TEMPENT :P)
				if (self->client->resp.class_type == CLASS_ENGINEER)
				{
	#ifdef CACHE_CLIENTSOUNDS
					gi.sound (self, CHAN_AUTO, SoundIndex(classlist[self->client->resp.class_type].sounds[SOUND_DEATH+3]), 1, ATTN_NORM, 0);
	#else
					gi.sound (self, CHAN_AUTO, gi.soundindex("*death4.wav"), 1, ATTN_NORM, 0);
	#endif
					temp_point (TE_EXPLOSION1, self->s.origin);
					gi.multicast (self->s.origin, MULTICAST_PVS);
				}
				else
				{
					gi.sound (self, CHAN_AUTO, SoundIndex (misc_udeath), 1, ATTN_NORM, 0);
				}

				//r1: no clienthead since it becomes stupid and float in air etc
				UnlinkClientEnt (self, 0);
			}
			//END GIB
			else //they didn't gib therefore run normal anims
			{	//BEGIN NORMAL DEATH ANIMS
				int i,start_frame,end_frame;

				//gi.dprintf("player_die: normal death\n");

				// be sure in player model
				self->s.modelindex = 255;

				i = randomMT()%3;

				//they're DYING... not fully dead yet since still in anims.
				self->deadflag = DEAD_DYING;
				self->client->damage_last_frame = level.framenum;

				if (self->client->resp.class_type==CLASS_WRAITH)
				{
					i=0;
					if (wasflying)
					{
						i=2;
						self->think=wraith_dead_think;
						self->nextthink=level.time+FRAMETIME;
					}
				}
				else if (self->client->ps.pmove.pm_flags & PMF_DUCKED)
					i=3;

				switch (i)
				{
				case 0:
					start_frame = FRAME_DIE1_S;
					end_frame = FRAME_DIE1_E;
					break;
				case 1:
					start_frame = FRAME_DIE2_S;
					end_frame = FRAME_DIE2_E;
					break;
				case 3:
					start_frame = FRAME_DUCKDIE_S;
					end_frame = FRAME_DUCKDIE_E;
					break;
				default:
					start_frame = FRAME_DIE3_S;
					end_frame = FRAME_DIE3_E;
					break;
				}

				self->client->anim_priority = ANIM_DEATH;
				self->s.frame = FrameReference(self, start_frame)-1;
				self->client->anim_end = FrameReference(self, end_frame);

				if(self->s.modelindex == 255 && (classlist[self->client->resp.class_type].diefunc))
					classlist[self->client->resp.class_type].diefunc(self);

				if (meansOfDeath != MOD_CHANGECLASS)
	#ifdef CACHE_CLIENTSOUNDS
					gi.sound (self, CHAN_AUTO, SoundIndex(classlist[self->client->resp.class_type].sounds[SOUND_DEATH+(randomMT()%4)]), 1, ATTN_NORM, 0);
	#else
					gi.sound (self, CHAN_AUTO, gi.soundindex(va("*death%i.wav", (randomMT()%4)+1)), 1, ATTN_NORM, 0);
	#endif
				// we're changing solidity, link!
				// FIXME ?? we are? where?
				gi.linkentity (self);
			}
		}
	}
	else if (self->deadflag == DEAD_DYING && self->health < classlist[self->client->resp.class_type].gib_health)
	{
		//the players dying body was shot to pieces so they "died" again. gib em.
		//yes, evil duplicated code. but i hate the current error.

		//gi.dprintf ("player_die: dying during normal death.\n");

		// gib the player, were DEAD. further calls to player_die should never happen
		self->deadflag = DEAD_DEAD;

		if ((self->client->resp.class_type==CLASS_ENGINEER)||(self->client->resp.class_type==CLASS_MECH))
			gibtype=GIB_METALLIC;
		else
			gibtype=GIB_ORGANIC;

		if (self->client->resp.class_type != CLASS_KAMIKAZE) {
			IntelligentThrowGib (self, self->headgib, damage, gibtype, 0, 2);
			IntelligentThrowGib (self, self->gib1, damage, gibtype, 0, 1);
			IntelligentThrowGib (self, self->gib2, damage, gibtype, 0, 1);
		}

		if(bandwidth_mode->value)
			ThrowGib (self, self->gib1, damage, gibtype);

		if(bandwidth_mode->value == 2.0)
			ThrowGib (self, self->gib2, damage, gibtype);

		if (self->client->resp.class_type == CLASS_KAMIKAZE && meansOfDeath != MOD_KAMIKAZE) {
			classlist[self->client->resp.class_type].diefunc(self);
		}

		gi.sound (self, CHAN_AUTO, SoundIndex (misc_udeath), 1, ATTN_NORM, 0);

		UnlinkClientEnt (self, 0);
	}
}

/*
InitClientPersistant

Only called at ClientConnect, thus settings are retained thru
every death and level changes.
*/
void InitClientPersistant (gclient_t *client)
{
	memset (&client->pers, 0, sizeof(client->pers));

	client->pers.starttime = time(0);
	client->pers.id_state = true;

	if (secure_modulate_cap->value) {
		client->pers.vid_done = false;
		client->pers.vid_frame = 1800;
	} else {
		client->pers.vid_done = true;
		client->pers.vid_frame = 80;
	}
}



// Called at ClientBeginDeathMatch, settings are for current level
void InitClientResp (gclient_t *client)
{
	memset (&client->resp, 0, sizeof(client->resp));

//	client->resp.coop_respawn = client->pers;

	// these aren't 0
	client->resp.enterframe = level.framenum;
	client->resp.class_type = CLASS_OBSERVER;

	client->resp.visible = true;
}


/*
SelectSpawnPoint

Chooses a player start, deathmatch start, coop start, etc
*/
void	SelectSpawnPoint (edict_t *ent __attribute__((unused)), vec3_t origin, vec3_t angles) //FV7 attribute
{
	edict_t	*spot = NULL;

	// find a single player start spot
	if (!spot)
	{
		if (!spot)
		{
			{	// there wasn't a spawnpoint without a target, so use any
				spot = G_Find (spot, FOFS(classname), "info_player_start");
			}
			if (!spot) {
				gi.dprintf ("WARNING: Couldn't find info_player_start, using worldspawn\n");
				spot = world;
			}
		}
	}

	VectorCopy (spot->s.origin, origin);
	origin[2] += 9;
	VectorCopy (spot->s.angles, angles);
}

/*void InitBodyQue (void)
{
	int		i;
	edict_t	*ent;

	level.body_que = 0;
	for (i=0; i < BODY_QUEUE_SIZE ; i++)
	{
		ent = G_Spawn();
		ent->classname = "bodyque";
		ent->s.effects = 0;
		ent->s.frame = 0;
		ent->svflags |= SVF_NOCLIENT;
	}
}*/

void body_explode_die (edict_t *self, edict_t *inflictor __attribute__((unused)), edict_t *attacker, int damage, vec3_t point __attribute__((unused))) { //FV7 attributes
int dmg;
//int i = 0;
int gibtype = GIB_ORGANIC;
edict_t *hum = NULL;

	if (self->health < 0) {
	//gi.dprintf("body_die: health %i\n", self->health);
	gi.sound (self, CHAN_AUTO, SoundIndex (misc_bdeath), 1, ATTN_NORM, 0);

	//r1: kami != mech
	//if (self->enttype == ENT_NOBLOOD_BODY)
	//	gibtype = GIB_METALLIC;

		if (bandwidth_mode->value) { ThrowGib (self, self->gib1, damage, gibtype); }
		if (bandwidth_mode->value == 2) { ThrowGib (self, self->gib2, damage, gibtype); }

	//r1: unnecessary with no queue
	//ThrowClientHead (self, damage);
	self->takedamage = DAMAGE_NO;
	self->s.frame = 0;

	dmg = 200;

		if (attacker->client && attacker->client->resp.team == TEAM_HUMAN) {
			while ((hum = findradius_c (hum, self, 150))) {
				if (hum->health > 0 && hum->client->resp.team == TEAM_ALIEN) {
				self->target_ent = NULL;
				dmg += 100;
				break;
				}
			}
		}

		if (self->target_ent) { T_RadiusDamage(self, self->target_ent, dmg, self, 300, MOD_CORPSE_EXPLOSION, 0); }
		else { T_RadiusDamage(self, self, dmg, self, 222, MOD_CORPSE_EXPLOSION, 0); }

/*Savvy hack for spike trops for upgraded kami*/
    if (self->oldupgrades & UPGRADE_SPIKE_POUCH) {
    spew_spiketrops (self, self->owner, 13, 150, 300);
    }

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_EXPLOSION1);
	gi.WritePosition (self->s.origin);
	gi.multicast (self->s.origin, MULTICAST_PVS);

	/*self->nextthink = 0;
	self->think = NULL;
	self->touch = NULL;
	self->svflags |= SVF_NOCLIENT;
	gi.unlinkentity (self);*/
	G_FreeEdict(self);
	}
}

/*Savvy silly me, this would probably work nicely though ;(
void body_upgraded_die(edict_t *body) {
  if ((body->oldclasstype == CLASS_KAMI) && (body->oldupgrades & UPGRADE_SPIKE_POUCH)) {
  
  }
  else if ((body->oldclasstype == CLASS_HATCHLING) && (body->oldupgrades & UPGRADE_HATCHLING_MARTYRDOM)) {
  //fire spike spore here as well, so double spike spores on death :)
  }
}
*/

void body_die (edict_t *self, edict_t *inflictor __attribute__((unused)), edict_t *attacker __attribute__((unused)), int damage, vec3_t point __attribute__((unused))) { //FV7 attributes
//int i = 0;
int gibtype = GIB_ORGANIC;

	if (self->health < 0) {
	//gi.dprintf("body_die: health %i\n", self->health);
	gi.sound (self, CHAN_AUTO, SoundIndex (misc_bdeath), 1, ATTN_NORM, 0);

		if (self->enttype == ENT_NOBLOOD_BODY) { gibtype = GIB_METALLIC; }

		if (bandwidth_mode->value) { ThrowGib (self, self->gib1, damage, gibtype); }
		if (bandwidth_mode->value == 2) { ThrowGib (self, self->gib2, damage, gibtype); }

	IntelligentThrowGib (self, self->gib1, damage, gibtype, 0, 2);
	IntelligentThrowGib (self, self->gib2, damage, gibtype, 0, 2);
  
  /*Savvy before we free it, and after gibs etc come, let's first check for upgraded corpse effects*/
    if (self->oldclasstype == CLASS_HATCHLING && self->oldupgrades & UPGRADE_HATCHLING_MARTYRDOM) {
    MarytrdomSpikes(self);
    }
  
	//throwclienthead sets unlinking
	/*ThrowClientHead (self, damage);
	self->nextthink = 0;
	self->think = NULL;
	self->touch = NULL;
	self->svflags |= SVF_NOCLIENT;
	gi.unlinkentity (self);*/
	G_FreeEdict(self);
	}
}

void body_decompose (edict_t *self);

void body_cool_down (edict_t *body)
{
	body->s.renderfx &= ~RF_IR_VISIBLE;

	if (body->die == body_explode_die)
		body->nextthink = level.time + 300 + random() * 300;
	else
		body->nextthink = level.time + 30 + random() * 20;

	body->think = body_decompose;
}

void body_decompose (edict_t *self)
{
	vec3_t normal={0,0,-1};

	if (self->enttype != ENT_NOBLOOD_BODY)
		SpawnDamage(TE_BLOOD,self->s.origin, normal ,0);

	//ThrowClientHead (self, 1);
	//self->nextthink = level.time + 10;
	//self->think = G_FreeEdict;
	G_FreeEdict(self);

}


void CopyToBodyQue (edict_t *ent)
{
	edict_t		*body;
	trace_t	tr;
	vec3_t start;
	vec3_t end;

	/*Savvy for armor hax*/
	int index;

	// precaution: we only want player models
	if (ent->s.modelindex != 255)
		return;

	// remember to reset any throwclienthead sets here too

	// grab a body que
	//body = &g_edicts[(int)game.maxclients + level.body_que];
	// cycle to next one
	//level.body_que = (level.body_que + 1) % BODY_QUEUE_SIZE;

	// free next one
	//gi.unlinkentity (&g_edicts[(int)game.maxclients + level.body_que]);

	// using bodyques SUCK!
	body = G_Spawn ();
	body->classname = "dead body";

	ent->s.modelindex = 0; // this prevents copytobodyque loops when dead
	gi.unlinkentity (ent);
	//gi.unlinkentity (body);

	VectorClear (ent->mins);
	VectorClear (ent->maxs);

	body->s.modelindex = gi.modelindex(ent->model);

//	body->s.frame = ent->client->anim_end;
	body->s.frame = ent->s.frame;

	body->svflags &= ~SVF_NOCLIENT;

	// direct indexes
	body->headgib = ent->headgib;
	body->gib1 = ent->gib1;
	body->gib2 = ent->gib2;

	// make sure real ent doesn't take any damage
	ent->takedamage = DAMAGE_NO;

	// for few classes, this is modified in below switch
	body->s.skinnum = 0;
	body->die = body_die;
	body->clipmask = MASK_SHOT;

	body->s.renderfx |= RF_IR_VISIBLE;

/*Savvy let's use my hack to store information in corpses - BUT ONLY IF IT HAS ARMOR TO BEGIN WITH!*/
body->oldclasstype = ent->client->resp.class_type;
body->oldupgrades = ent->client->resp.upgrades;
/*WARNING
crash bug, if statement passes on uninitialized armor variable, therefore ITEM_INDEX fucks up with a large number, and it accesses 255 max item array with number in the thousands
i may have fixed this through a dirty hack below (savvy)
WARNING*/
  if (ent->client->armor) { index = ITEM_INDEX(ent->client->armor); }
  if ((index <= 255) && (index >= 0)) {
    if (ent->client->resp.inventory[index]) { body->remainingarmor = ent->client->resp.inventory[index]; }
  }
  else {
  	gi.cprintf(NULL, PRINT_HIGH, "ERROR (catched, so not fatal): player %s when they died, their class's armor type returned too large a number (%d)", ent->client->pers.netname, index);
  }

	// make sure bbox doesn't enlarge in downwards z dir, otherwise body drops off map
	switch (ent->client->resp.class_type) {
	case CLASS_ENGINEER:
		// no visible death model
		VectorSet (body->mins, -8, -8, -8);
		VectorSet (body->maxs, 0, 0, 0);
		break;
	case CLASS_SHOCK:
		body->s.skinnum = 1;
		VectorSet (body->mins, -32, -32, -24);
		VectorSet (body->maxs, 32, 32, -8);
		break;
	case CLASS_GRUNT:
		VectorSet (body->mins, -32, -32, -24);
		VectorSet (body->maxs, 32, 32, -8);
		break;
	case CLASS_COMMANDO:
		body->s.skinnum = 2;
		VectorSet (body->mins, -32, -32, -24);
		VectorSet (body->maxs, 32, 32, -8);
		break;
	case CLASS_BIO:
		VectorSet (body->mins, -24, -24, -24);
		VectorSet (body->maxs, 24, 24, 16);
		break;
	case CLASS_HEAVY:
		VectorSet (body->mins, -32, -32, -24);
		VectorSet (body->maxs, 32, 32, 16);
		break;
	case CLASS_EXTERM:
		VectorSet (body->mins, -48, -48, -24);
		VectorSet (body->maxs, 48, 48, 0);
		break;
	case CLASS_MECH:
		VectorSet (body->mins, -48, -48, -8);
		VectorSet (body->maxs, 48, 48, 8);
		break;
	case CLASS_BREEDER:
		//now there are 2 breeder models this makes it much more fun
#ifdef TWOBREEDERS
		if (!Q_stricmp (ent->model, "players/breeder2/tris.md2")) {
			VectorSet (body->mins, -32, -32, -30); //-32 extends off (30 it is then)
			VectorSet (body->maxs, 32, 32, -8);
		} else {
			VectorSet (body->mins, -32, -32, -24); //-32 extends off
			VectorSet (body->maxs, 32, 32, -8);
		}
#else
			VectorSet (body->mins, -32, -32, -24); //-32 extends off
			VectorSet (body->maxs, 32, 32, -8);
#endif
		break;
	case CLASS_KAMIKAZE:
		body->s.skinnum = 1;
		body->target_ent = ent;
		body->die = body_explode_die;
		__attribute__((fallthrough)); //FV7
	case CLASS_HATCHLING:
		VectorSet (body->mins, -16, -16, -24);
		VectorSet (body->maxs, 16, 16, -8);
		break;
	case CLASS_DRONE:
		VectorSet (body->mins, -32, -32, -24); //-32 extends off
		VectorSet (body->maxs, 32, 32, -8);
		break;
	case CLASS_WRAITH:
		VectorSet (body->mins, -32, -32, -24);
		VectorSet (body->maxs, 32, 32, -8);
		break;
	case CLASS_STINGER:
		VectorSet (body->mins, -24, -24, -24);
		VectorSet (body->maxs, 24, 24, 24);
		break;
	case CLASS_GUARDIAN:
		VectorSet (body->mins, -32, -32, -24);
		VectorSet (body->maxs, 32, 32, 16);
		break;
	case CLASS_STALKER:
		VectorSet (body->mins, -40, -40, -16);
		VectorSet (body->maxs, 40, 40, 16);
		break;
	default:
		VectorSet (body->mins, -8, -8, -8);
		VectorSet (body->maxs, 0, 0, 0);
		break;
	}

	//R1: fixed dead body through floor code !!!!!!!!!!!!
	//trace haxage, but it WORKS!! in all testing, not ONE body fell through
	//even on angled slopes, etc. i own :D

	VectorCopy(ent->s.origin,start);
	VectorCopy(ent->s.origin,end);

	VectorClear(body->velocity);
	VectorCopy (ent->s.origin, body->s.origin);
	VectorCopy (ent->s.origin, body->s.old_origin);
	VectorCopy (ent->s.angles, body->s.angles);


	start[2] += 32;
	end[2] -= 96;

	//we trace with no horizontal clipping
	//this way we don't end up with floating
	//shit.

	tr = gi.trace (start, body->mins, body->maxs, end, ent, MASK_SOLID);

	if (tr.fraction != 1.0) {
		VectorCopy (tr.endpos, body->s.origin);
	}

	ent->solid = SOLID_NOT;
	body->s.angles[0] = 0;

	body->svflags = SVF_DEADMONSTER;
	body->solid = SOLID_BBOX;
	body->clipmask = MASK_SHOT; // this seems not have any effect, svflags has
	body->owner = NULL;
	body->flags = FL_NO_KNOCKBACK;

	body->s.effects = 0; // for clearing throwclienthead effects

	/* notneeded
	body->s.renderfx = 0;
	body->s.sound = 0;*/

	body->movetype = MOVETYPE_TOSS;

	body->think = body_cool_down;
	body->nextthink = level.time + 20 + random() * 20;

	// transfer gib health to dead body
	body->health = 0 - classlist[ent->client->resp.class_type].gib_health - ent->health;

	body->takedamage = DAMAGE_YES;

	if (ent->client->resp.class_type == CLASS_GRUNT || ent->client->resp.class_type == CLASS_HEAVY || ent->client->resp.class_type == CLASS_COMMANDO || ent->client->resp.class_type == CLASS_SHOCK) {
		body->enttype = ENT_HUMAN_BODY;
	} else if (ent->client->resp.class_type == CLASS_BIO) {
		body->enttype = ENT_FEMALE_BODY;
	} else if (ent->client->resp.class_type == CLASS_MECH || ent->client->resp.class_type == CLASS_EXTERM) {
		body->enttype = ENT_NOBLOOD_BODY;
	} else {
		body->enttype = ENT_CORPSE;
	}

	gi.linkentity (body);
}

/*void hack_bbox_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == self->owner)
		return;

	//gi.dprintf ("%s hit the hack\n", other->classname);
}*/

/*
PutClientInServer

Called when a player connects to a server or respawns in
a deathmatch.
*/

void PutClientInServer (edict_t *ent, unsigned class_type){
	int		i;

	client_persistant_t	pers;
	client_respawn_t	resp;

	if (ent->client->resp.turret) {
		ent->client->resp.turret->owner = NULL;
		ent->client->resp.turret = NULL;
	}

	// clear everything but the persistant data
	pers = ent->client->pers;
	// preserve resp.
	resp = ent->client->resp;

	// CLEAR CLIENT STRUCT
	memset (ent->client, 0, sizeof(*ent->client));

	// restore
	ent->client->resp = resp;
	ent->client->pers = pers;

	// clear inventory
	memset (ent->client->resp.inventory, 0, sizeof(ent->client->resp.inventory));

	ent->health = ent->max_health = classlist[class_type].health;

	// clear entity values
	ent->groundentity = NULL;
	ent->takedamage = DAMAGE_AIM;
	ent->movetype = MOVETYPE_STEP;
	ent->viewheight = classlist[class_type].viewheight;
	ent->wait = 0;
	ent->client->last_menu_frame = 0;
	ent->gravity = sv_gravity->value;
	ent->mass = classlist[class_type].mass;
	ent->solid = SOLID_BBOX;
	ent->deadflag = DEAD_NO;
	ent->air_finished = level.time + 12;
	ent->clipmask = MASK_PLAYERSOLID;
	ent->pain_debounce_time = 0;

	ent->dmg = ent->radius_dmg = ent->dmg_radius = 0;

	if (!deathmatch->value)
	{
		ent->movetype = MOVETYPE_NOCLIP;
		ent->client->pers.connected = true;
		ent->client->pers.ingame = true;
		class_type = CLASS_OBSERVER;
	}

	ent->model = classlist[class_type].model;

#ifdef TWOBREEDERS
	if (class_type == CLASS_BREEDER)
		if (randomMT() & 1)
			ent->model = "players/breeder2/tris.md2";
#endif

	if (classlist[class_type].headgib)
		ent->headgib = gi.modelindex(classlist[class_type].headgib);

	if (classlist[class_type].gib1)
		ent->gib1 = gi.modelindex(classlist[class_type].gib1);

	if (classlist[class_type].gib2)
		ent->gib2 = gi.modelindex(classlist[class_type].gib2);

	ent->damage_absorb = classlist[class_type].absorb;
	ent->pain = player_pain;
	ent->die = player_die;
	ent->watertype = 0;
	ent->waterlevel = 0;
	ent->flags = 0;
	ent->svflags = 0;

	ent->client->invincible_framenum = level.framenum + 25;

	ent->pain_debounce_time = 0;

	ent->client->resp.selected_item = -1;
	ent->client->resp.primed = false;

	// Special inventory assignment for each class
	if(classlist[class_type].startfunc) {
		classlist[class_type].startfunc(ent);
	}

	//r1: new function to handle armor.
	if (classlist[class_type].armor) {
		int index;
		ent->client->armor = FindArmor(classlist[class_type].armor);
		index = ITEM_INDEX (ent->client->armor);
		ent->client->resp.inventory[index] = classlist[class_type].armorcount;
	}

	// all classes are ir visible
	ent->s.renderfx |= RF_IR_VISIBLE;

	if(ent->client->resp.team == TEAM_HUMAN) {
		ent->s.event = EV_PLAYER_TELEPORT;
		ent->flags |= FL_IMMUNE_LASER;
		if (class_type == CLASS_MECH)
			ent->client->jumpattack = true;

	} else if (ent->client->resp.team == TEAM_ALIEN) {
		ent->client->ps.rdflags |= RDF_IRGOGGLES;

		//r1: lerp fix
		ent->s.event = EV_OTHER_TELEPORT;

		if (class_type != CLASS_BREEDER && class_type != CLASS_WRAITH)
			ent->client->jumpattack = true;
	}

	ent->client->ps.pmove.origin[0] = ent->s.origin[0]*8;
	ent->client->ps.pmove.origin[1] = ent->s.origin[1]*8;
	ent->client->ps.pmove.origin[2] = ent->s.origin[2]*8;

	ent->client->ps.fov = classlist[class_type].fov;

	// clear entity state values
	ent->s.effects = 0;
	// integer, with upper 8 bits marking optional vwep weapon (from a static list) and lower 8 bits player skin
	// if set to 255, takes skin from CS_PLAYERSTRING
	// http://www.z-studios.com/resources/tutorials/q2/09-3.html
	ent->s.skinnum = ent - g_edicts - 1;
	// if set to 255, takes model from CS_PLAYERSTRING
	ent->s.modelindex = 255;		// will use the skin specified model
	// if set to 255, takes predefined model from upper 8 bits of skinnum
	ent->s.modelindex2 = 0;

	if (class_type != CLASS_ENGINEER)
		ent->s.frame = 0;

	if (class_type == CLASS_OBSERVER) {

		// reset into spectator mode
		ent->movetype = MOVETYPE_NOCLIP;
		ent->solid = SOLID_NOT;
		ent->svflags |= SVF_NOCLIENT;

		ent->client->invincible_framenum = 0;
	}

	/* no angles or origin modification here, they're set before calling putclientinserver */

	VectorCopy (ent->s.angles, ent->client->ps.viewangles); // client viewangle
	VectorCopy (ent->s.angles, ent->client->v_angle); // v_angle is used for ent->s.angles

	VectorCopy (ent->s.origin, ent->s.old_origin);

	// set the delta angle
	for (i=0 ; i<3 ; i++)
		ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(ent->s.angles[i] - ent->client->resp.cmd_angles[i]);

	// only send configstrings if class_type has changed
	//but breeder asses this check up nicely :(
#ifdef TWOBREEDERS
	if (ent->client->pers.ingame && (class_type != ent->client->resp.class_type || class_type == CLASS_OBSERVER || class_type == CLASS_BREEDER)) {
#else
	if (ent->client->pers.ingame && (class_type != (unsigned int)ent->client->resp.class_type || class_type == CLASS_OBSERVER)) {  //FV7 typecast int to unsigned int
#endif
		set_player_configstrings(ent, class_type);
	}

	SetClientVelocity (ent, DEFAULT_VELOCITY);
	stuffcmd (ent, va("set cl_upspeed %d\n", DEFAULT_VELOCITY));

	// set new class
	ent->client->resp.class_type = class_type;

	// force the current weapon up
	if (ent->client->weapon){
		ent->client->newweapon = ent->client->weapon;
		ChangeWeapon (ent);
	}

	gi.linkentity (ent);
}

/*
ClientBeginDeathmatch

A client has just connected to the server in
deathmatch mode, so clear everything out before starting them.
gets called once in connect and afterwards EVERY level change
*/
void ClientBeginDeathmatch (edict_t *ent)
{
	int i = 0;
	edict_t *other;
	int	ok = 0;
	int	playernum = ent - g_edicts - 1;

	// init game state info
	InitClientResp (ent->client);

	//anti bot reconnect code
	if (*reconnect_address->string) {
		//check expired reconnects

		#pragma GCC diagnostic push
		#pragma GCC diagnostic ignored "-Waggressive-loop-optimizations" //FV7 ignore warnings for the following loop
		for (i = 0; i <= MAX_CLIENTS; i++) {
			if ((float)reconnect[i].time < (float)level.time && reconnect[i].valid == false) {
				reconnect[i].completed = reconnect[i].reconnecting = false;
				reconnect[i].ip[0] = '\0';
			}
		}
		#pragma GCC diagnostic pop //FV7

		//they haven't reconnected yet, send em away
		if (((!reconnect[playernum].reconnecting || reconnect[playernum].token != 1) && !ent->client->pers.old_connected) || Q_stricmp(ent->client->pers.ip,reconnect[playernum].ip)) {
			char var1[6];
			char var2[6];
			char var3[9];

			char message[16];
			char value[16];

			for (i = 0; i < 5; i++) {
				var1[i] = randomMT()%26+65;
				var2[i] = randomMT()%26+65;
				var3[i] = randomMT()%57+65;
			}

			value[15] = message[15] = var1[5] = var2[5] = var3[8] = 0;

			for (i = 5; i < 8; i++)
				var3[i] = randomMT()%57+65;

			for (i = 0; i < (int)sizeof(message)-1; i++) //FV7 typecast long unsigned to int
				message[i] = randomMT()%57+65;

			for (i = 0; i < (int)sizeof(value)-1; i++) //FV7 typecast long unsigned to int
				value[i] = randomMT()%57+65;

			//make sure client is valid for a few frames or so
			ent->client->ps.fov = 1;

			//send them off to reconnect (45 sec timeout before they are fried)
			//NOTE that no way to check same client, we assume that once we send them
			//off to reconnect, they will reconnect with the same playernum. (HAX!!)
			gi.WriteByte(svc_stufftext);
 			gi.WriteString(va("\nalias nobots connect 123.123.123.123:27910\nset %s \"connect\"\nset %s %s\nalias %s $%s $%s\nset %s %s\n%s\n", var1, var2, reconnect_address->string, var3, var1, var2, message, value, var3));
 			gi.unicast(ent, true);

			// if they filtered the reconnect they will be hanging in limbo using
			// a client slot. every time 'uptime' is updated (60 secs), CheckReconnectEnts
			// will be called which detects limbo players and give em the boot.

			strncpy (reconnect[playernum].ip,ent->client->pers.ip,sizeof(reconnect[playernum].ip));
			strncpy (reconnect[playernum].message,  message, sizeof(reconnect[playernum].message));
			strncpy (reconnect[playernum].value,  value, sizeof(reconnect[playernum].value));
			
			reconnect[playernum].token = 2;
			reconnect[playernum].reconnecting = true;
			reconnect[playernum].completed = false;
			reconnect[playernum].time = level.time + 45;
			return;
		}

		//they're ok, mark entry as valid
		reconnect[playernum].reconnecting = false;
		reconnect[playernum].completed = true;

		if (!ent->client->pers.old_connected)
		{
			ent->client->pers.stuff_request |= 8;
			gi.WriteByte(svc_stufftext);
 			gi.WriteString(va("\ncmd %s $%s\n", reconnect[playernum].message, reconnect[playernum].message));
 			gi.unicast(ent, true);
		}
	}

	if (ent->client->pers.old_connected && ent->client->pers.ingame) {
		log_connection (ent);

		//r1: configstrings are wiped every level - resend
		gi.configstring (CS_GENERAL+(ent-g_edicts-1), colortext(ent->client->pers.netname));
		ent->client->pers.connected = true;
		/*if (gloomgamelog->value > 0) {
			gi.WriteByte (svc_stufftext);
			gi.WriteString (va("alias getlog \"download %s/logs/%s\"\n", game.port, lastlogname));
			gi.unicast (ent, true);
		}*/
	}

	// init edict
	ent->inuse = true;
	ent->classname = "player";

	// end of game state info

	// move player to info_player_start
	SelectSpawnPoint (ent, ent->s.origin, ent->s.angles);

	// client is ingame
	ent->client->pers.ingame = true;

	// quick hook player into game
	PutClientInServer(ent, CLASS_OBSERVER);

	// numplayers for team_none needs to be updated here
	team_info.numplayers[TEAM_NONE]++;

	Updateteam_menu();

	// if intermission
	if (level.intermissiontime && (level.intermissiontime < level.framenum))
	{
		MoveClientToIntermission (ent);
	}

	if (!ent->client->pers.ghostcode) {
		while (ok == 0) {
			ent->client->pers.ghostcode = random()*99999;
			ok = 1;
			for (i = 0;i <= 64;i++) {
				if (fragsave[i].inuse == 1)
					if (ent->client->pers.ghostcode == fragsave[i].ghostcode)
						ok = 0;
			}

			for (other = &g_edicts[1]; other <= &g_edicts[game.maxclients]; other++) {
				if (!other->inuse)
					continue;
				if (other == ent)
					continue;
				if (!other->client)
					continue;
				if (!other->client->pers.ingame)
					continue;
				if (!other->client->pers.ghostcode)
					continue;
				if (ent->client->pers.ghostcode == other->client->pers.ghostcode) {
					ok = 0;
					break;
				}
			}
		}
	}

	//already in game but show message again if they took a while to join (autodl for example)
	if (ent->client->pers.check_stage == 5 && level.time > 60 && !level.suddendeath)
	{
		gi.bprintf (PRINT_HIGH, "%s entered the game.\n", ent->client->pers.netname);
	}

	// make sure all view stuff is valid
	ClientEndServerFrame (ent);
}

qboolean CheckLogin (edict_t *ent, char *userinfo) {
	char *loginstring = NULL;
	char password[9] = "";
	char *pdest = NULL;
	size_t result = 0;
	int i = 0;
	char username[9] = {0};

	loginstring = Info_ValueForKey (userinfo,"user");

	//no userinfo for long, tell user they need account
	if (loginstring == NULL || !*loginstring) {
		if (*login_message->string) {
			gi.cprintf (ent, PRINT_HIGH, "\n%s\n",login_message->string);
		}
		return false;
	}


	//search for : user:pass separatpr
	pdest = strchr(loginstring, ':');
	if (pdest != NULL) {
		result = pdest - loginstring;

		//username too short to be valid
		if (result < 2) {
			return false;
		}

		//username too long, truncate
		if (result > 8) {
			result = 8;
		}

		//copy it off, valid
		strncpy (username,loginstring,result);
		loginstring += result + 1;
		result = strlen(loginstring);

		//password too short to be valid
		if (result < 2) {
			return false;
		}

		//password too long, truncate
		if (result > 8) {
			result = 8;
		}

		//valid, copy it off
		strncpy (password,loginstring,result);
	} else {
		//no : in separator, invalid
		return false;
	}

	i = CheckAccount(username, password, &ent->client->pers.adminpermissions);
	if (i == 0) {
		return false;
	} else if (i < 0) {
		// disabled account
		return false;
	}

	strcpy (ent->client->pers.username, username);
	return true;
}

/*
ClientBegin

called when a client has finished connecting, and is ready
to be placed into the game.	This will happen every level load.
*/
void EXPORT ClientBegin (edict_t *ent)
{
	int		i;

	if (deathmatch->value)
	{
		ClientBeginDeathmatch (ent);
		return;
	}

	// if there is already a body waiting for us (a loadgame), just
	// take it, otherwise spawn one from scratch
	if (ent->inuse == true)
	{
		// the client has cleared the client side viewangles upon
		// connecting to the server, which is different than the
		// state when the game is saved, so we need to compensate
		// with deltaangles
		for (i=0 ; i<3 ; i++)
			ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(ent->client->ps.viewangles[i]);
	}
	else
	{
		// a spawn point will completely reinitialize the entity
		// except for the persistant data that was initialized at
		// ClientConnect() time
		ent->inuse = true;
		ent->classname = "player";
		InitClientResp (ent->client);
		SelectSpawnPoint (ent, ent->s.origin, ent->s.angles);
		PutClientInServer (ent, CLASS_OBSERVER);
	}

	if (level.intermissiontime && (level.intermissiontime < level.framenum))
	{
		MoveClientToIntermission (ent);
	}

	// make sure all view stuff is valid
	ClientEndServerFrame (ent);
}

/*void DumpUser(edict_t *ent, char *userinfo) {
	FILE *log;

	log = fopen("userinfo.log", "a");
	fprintf(log, "\nPlayer Loop:\n");
	fprintf(log, "%s@%s is causing a recurring loop\n",ent->client->pers.netname,Info_ValueForKey (ent->client->pers.userinfo,"ip"));
	fprintf(log, "Client Pers Userinfo dump:\n%s\n",ent->client->pers.userinfo);
	fprintf(log, "Changed Userinfo dump:\n%s\n",userinfo);
	fclose(log);
}*/

qboolean scan_validate (char *s)
{
	while (*s) {
		if (isdigit (*s))
			return true;
		s++;
	}

	return false;
}

/*
ClientUserInfoChanged

called whenever the player updates a userinfo variable.

The game can override any of the settings in place
(forcing skins or names, etc) before copying it off.
*/

//r1: moved to data section as opposed to stack
char	drawflatcap[] = "set sw_drawflat 0 u\n";
char	polyblendcap[] = "set gl_polyblend 1 u\n";
char	picmipcap[] = "set gl_picmip 0 u\n";
char	modulatecap[] = "set gl_modulate 1 u\n";
char	lockpvscap[] = "set gl_lockpvs 0 u\n";
char	swlockpvscap[] = "set sw_lockpvs 0 u\n";
char	monolightmapcap[] = "set gl_monolightmap 0 u\n";
char	mipcapcap[] = "set sw_mipcap 0 u\n";
char	mipscalecap[] = "set sw_mipscale 0 u\n";
char	dynamiccap[] = "set gl_dynamic 1 u\n";
char	testblendcap[] = "set cl_testblend 0 u\n";
char	shadowscap[] = "set gl_shadows 0 u\n";
char	fixedtimecap[] = "set fixedtime 0 u\n";
char	timescalecap[] = "set timescale 1 u\n";
char	clblendcap[] = "set cl_blend 1 u\n";
char	glclearcap[] = "set gl_clear 0 u\n";

char	drawworldcap[] = "set r_drawworld 1 u\n";
char	lightmapcap[] = "set gl_lightmap 0 u\n";
char	fullbrightcap[] = "set r_fullbright 0 u\n";

char	gp[10];

qboolean isSpecialEntityName (char *name);
void EXPORT ClientUserinfoChanged (edict_t *ent, char *userinfo)
{
	int	i = 0;
	float	maxmodulate = 1.4, maxintensity = 2, max_gamma = 0.5;
	float	v = 0, current_modulate = 0;
	char	*s = NULL;
	//char	*newname = NULL;

	char	ip[22];
	char	*bp;

	
	char	myname[16] = {0};

	edict_t *victim;

	qboolean	need_fov = false, need_vidrestart = false;

	if (!ent->client)
		return;

	// stops quake2 doing ClientUserinfoChanged call on connect which includes \ip in userinfo
	if (!ent->client->pers.ingame && !ent->client->pers.connected)
		return;

	//gi.dprintf("USERINFO call len %i: %s:\n", strlen(userinfo), userinfo);

	// check for malformed or illegal info strings
	if (!Info_Validate(userinfo))
	{
		strcpy (userinfo, "\\name\\badinfo");
	}

	//note the password is 'blanked' here, to stop people doing a dumpuser
	//and finding it out. Still works though, since it is passed in client
	//userinfo just not copied to the server userinfo after being set.
	/*if (*adminpassword->string && ent->client->pers.god != GOD_ADMIN && Q_strcasecmp(Info_ValueForKey (userinfo, "ap"),adminpassword->string)==0) {
		ent->client->pers.god = GOD_ADMIN;
	} else if (Q_strcasecmp(Info_ValueForKey (userinfo, "gp"),gp)==0 && !ent->client->pers.god) {
		ent->client->pers.god = GOD_NORMAL;
	}*/

	Info_RemoveKey (userinfo, "ap");
	Info_RemoveKey (userinfo, "gp");

	// set and check name
	//newname = Info_ValueForKey (userinfo, "name");

	//make some more room
	if (Info_ValueForKey (userinfo, "skin")[0])
	{
		stuffcmd (ent, "set skin \"\"\n");
	}

	if (Info_ValueForKey (userinfo, "spectator")[0])
	{
		stuffcmd (ent, "set spectator \"\"\n");
	}

	if (Info_ValueForKey (userinfo, "gender")[0])
	{
		stuffcmd (ent, "set gender_auto 0\nset gender \"\"\n");
	}

	// set and check name, 15 chars max
	strncpy (myname, Info_ValueForKey (userinfo, "name"), sizeof(myname)-1);

	// check if name changed and if so
	// changed from Q_stricmp to strcmp (so cae sensitive names can be changed)
	if (strcmp(myname, ent->client->pers.netname) != 0)
	{
		char *p = myname;
		// run set_player_configstrings thru before acting on client as quake2
		// uses configstring name for kicking and such (otherwise you'll get client id)

		// name flood protection, first of all if the forced name changes bomb out thus creating a loop
		if (++ent->client->pers.namechanges > 4)
		{
			gi.bprintf (PRINT_HIGH,"%s changed names too much\n", ent->client->pers.netname);

			//r1: server already took a copy of the new name - kick message will be wrong
			//unless we set it back.
			Info_SetValueForKey (userinfo, "name", ent->client->pers.netname);

			kick (ent);
			return;
		}

		//r1; check for stripping of "fun" names
		if (strip_names->value)
		{
			char *stripped = StripHighBits (myname);
			
			if (strcmp (myname, stripped)) {
				strncpy (myname, stripped, sizeof(myname)-1);

				//trigger another update
				stuffcmd (ent,va("set name \"%s\"\n", myname));
				return;
			}
		}

		// blank name check
		while (*p && isspace(*p))
			p++;

		if (!*p)
		{
			gi.cprintf (ent,PRINT_HIGH,"Can't use a blank name.\n");
			stuffcmd (ent,va("set name \"noname%d\"\n", (int)(random() * 65535)));
			return;
		}

		//% is banned to avoid the say preprocessor 'macro' replacements changing part
		//of the client name.
		if (strchr(myname, '%'))
		{
			gi.cprintf (ent, PRINT_HIGH, "Can't use a name with a %% sign in it.\n");

			if (ent->client->pers.netname[0])
				stuffcmd (ent,va("set name \"%s\"\n",ent->client->pers.netname));
			else
				stuffcmd (ent,va("set name \"noname%d\"\n", (int)(random() * 65535)));

			return;
		}

		//; is banned since some evil hax0r d00d might use the name i;quit or something
		//and joe n00bie runs vote kick i;quit... oops
		if (strchr(myname, ';'))
		{
			gi.cprintf (ent, PRINT_HIGH, "Can't use a name with a semi-colon in it.\n");

			if (ent->client->pers.netname[0])
				stuffcmd (ent,va("set name \"%s\"\n",ent->client->pers.netname));
			else
				stuffcmd (ent,va("set name \"noname%d\"\n", (int)(random() * 65535)));

			return;
		}

		//check they aren't attempting to impersonate an entity (rox)
		// because of the game logging
		if (isSpecialEntityName (myname))
		{
			gi.cprintf (ent, PRINT_HIGH, "Can't use reserved entity names for your name.\n");

			if (ent->client->pers.netname[0])
				stuffcmd (ent,va("set name \"%s\"\n",ent->client->pers.netname));
			else
				stuffcmd (ent,va("set name \"noname%d\"\n", (int)(random() * 65535)));

			return;
		}

		for (victim=&g_edicts[1] ; victim <= &g_edicts[game.maxclients] ; victim++)
		{
			if (victim->inuse && victim != ent && victim->client->pers.connected && !Q_stricmp (myname, victim->client->pers.netname)) {
					gi.cprintf (ent,PRINT_HIGH,"Name '%s' is already taken.\n", myname);

					if (ent->client->pers.netname[0])
						stuffcmd (ent,va("set name \"%s\"\n",ent->client->pers.netname));
					else
						stuffcmd (ent,va("set name \"noname%d\"\n", (int)(random() * 65535)));

					return;
			}
		}

		if (ent->client->pers.netname[0])
		{
			if (ent->client->resp.visible && !level.suddendeath)
				gi.bprintf (PRINT_MEDIUM, "%s changed name to %s\n", ent->client->pers.netname, myname);
			//log people changing names for future reference (eg connect as player and
			//lame as some other name, previously would not be logged)
			log_iplog (myname, ent->client->pers.ip);
			log_namechange (ent->client->pers.netname, myname);
		}

		// accept name
		strncpy (ent->client->pers.netname, myname, sizeof(ent->client->pers.netname)-1);
		set_player_configstrings(ent, ent->client->resp.class_type);
		// set ctfid cstring
		gi.configstring (CS_GENERAL+(ent-g_edicts-1), colortext(ent->client->pers.netname));
	}

	// this should announce player entering server after ClientConnect
	if (!ent->client->pers.connected) {
		int	banid;

		// if we're using accounts, we need to do ban checking here
		if (accounts->string[0]) {

			// check for banned ip (r1: ignore admins)
			if (!(ent->client->pers.adminpermissions & (1 << PERMISSION_BAN)))
				banid = SV_FilterPacket(ent->client->pers.ip);
			else
				banid = -2;
			// check for account
			if (!CheckLogin (ent, userinfo)) {
				if (banid >= 0) {
				char *banreason = va("\"%s\"",ipfilters[banid].reason ? ipfilters[banid].reason : "no reason");
				char *banner = va("%s",ipfilters[banid].banner);
					gi.cprintf (ent, PRINT_HIGH, 
						"\n"
						"       ******************************\n"
						"       You are banned for %s (id=%-2.2X).\n"
						"       ******************************\n"
						"\n"
						"You were banned for %s\n" /*Savvy char[33]*/
            "by %s\n" /*char[16]*/
						"If you believe this to be in error, please contact\n"
						"a server administrator (probably before ban time wears out).\n"
						"\n", TimeToString(ipfilters[banid].expiretime - time(0)), banid, banreason, banner);
					gi.cprintf (NULL, PRINT_HIGH, "%s@%s was kicked (%s)\n", ent->client->pers.netname, ent->client->pers.ip, ipfilters[banid].reason);

					//r1: nuke their name so the server hides the kick message
					Info_RemoveKey (userinfo, "name");
					kick (ent);
					return;
				} else if (login->value){
					gi.cprintf (ent, PRINT_HIGH, "\n********* ERROR *********\nInvalid username/password or account disabled.\n*************************\n");
					gi.cprintf (NULL, PRINT_HIGH, "%s@%s was kicked (%s)\n", ent->client->pers.netname, ent->client->pers.ip, ipfilters[banid].reason);

					//r1: nuke their name so the server hides the kick message
					Info_RemoveKey (userinfo, "name");
					kick (ent);
					return;
				}
			} else {
				// valid account, let player pass
				if (banid >= 0)
					gi.cprintf (NULL, PRINT_HIGH, "%s@%s bypassed IP ban with username %s\n",ent->client->pers.netname, ent->client->pers.ip, ent->client->pers.username);

				//hide their details from prying eyes
				Info_RemoveKey (userinfo, "user");
			}
		}

		//r1: kick people trying to use admin slots who aren't admins
		if (!(ent->client->pers.adminpermissions & (1 << PERMISSION_ADMINSLOT)) && team_info.numplayers[TEAM_ALIEN] + team_info.numplayers[TEAM_HUMAN] + team_info.numplayers[TEAM_NONE] > (maxclients->value - adminslots->value))
		{
			gi.cprintf (ent, PRINT_HIGH, "Invalid password for admin playerslot.\n");
			Info_RemoveKey (userinfo, "name");
			kick (ent);
			return;
		}

		//we just want the IP, screw the port
		strncpy (ip,ent->client->pers.ip,sizeof(ip));
		bp = ip;
		while(*bp && *bp != ':')
			bp++;
		*bp = 0;

		//save IP log
		log_iplog (ent->client->pers.netname, ip);

		//check for ghost/clones/multi's whatever
		if (accounts->string[0]) {
			for (victim=&g_edicts[1] ; victim <= &g_edicts[game.maxclients] ; victim++) {
				if (victim->inuse && victim != ent && victim->client->pers.connected) {
					if (victim->client->pers.username[0] && !Q_stricmp(victim->client->pers.username,ent->client->pers.username)) {
						gi.bprintf (PRINT_HIGH,"Kicking %s because it is a ghost.\n",victim->client->pers.netname);
						//kick (victim);
						gi.AddCommandString (va("kick %d\n", (victim - g_edicts)-1 ));
					}
				}
			}
		}

		log_connection (ent);

		//r1: name change configstring won't run per maps?
		ent->client->pers.connected = true;
	}

	// save off the userinfo in case we want to check something later
	strncpy (ent->client->pers.userinfo, userinfo, sizeof(ent->client->pers.userinfo)-1);

	s = Info_ValueForKey (userinfo, "hand");
	if (s[0])
	{
		ent->client->pers.hand = atoi(s);
	}

	// prevents stuffcmd loops (ratbot with default settings (force name on) cause these)
	if (ent->client->pers.cheat_checked > MAX_CHEAT_CHECKING_ATTEMPTS) {
		//r1: prevent repeat loops
		ent->client->pers.cheat_checked = 0;

		gi.bprintf (PRINT_HIGH,"Kicking %s due to failed cheat checks\n",ent->client->pers.netname);
		kick (ent);
		return;
	}

	if (ent->inuse && ent->client->pers.ingame && gloomcaps->value) {

		if (ent->client->pers.check_stage > 1 )
		{
			/* add any variables which are capped in this block
			   also any variables which need vid_restart should be in this block */

			char driverstr[20] = "";
			//char vidrefstr[10] = "";

			s = Info_ValueForKey(ent->client->pers.userinfo, "gl_driver");
			i = sscanf(s, "%19s", driverstr);
			if (i == -1) {
				// if gl_driver isn't set
				stuffcmd(ent,"set gl_driver $gl_driver u\n");
				need_fov = true;
				ent->client->pers.cheat_checked++;
			} else {

				if(Q_stricmp(driverstr, "3dfxgl") == 0) {
					maxmodulate = 1.4;
					maxintensity = 2.0;
				} else if(Q_strncasecmp (driverstr, "opengl32", 8) == 0) {
					maxmodulate = 2.0;
					maxintensity = 2.0;
				} else if (Q_stricmp (driverstr, "u") == 0) {
					maxmodulate = 1.0;
					maxintensity = 2.0;
				} else {
					maxmodulate = 1.4; // will match AMD and other reference drivers
					maxintensity = 2.0;
				}
			}

			//FIXME: make this work.
			//       at the moment the stuffcmd(ent, "set vid_ref $vid_ref u\n"); causes
			//       a vid_restart... looks like it's the q2.exe doing it, so no solution

			/*s=Info_ValueForKey(ent->client->pers.userinfo, "vid_ref");
			i = sscanf(s, "%s9", &vidrefstr);
			if (i == -1) {
				// gets called if gl_modulate isn't there
				stuffcmd(ent, "set vid_ref $vid_ref u\n");
				need_fov = true;
				ent->client->pers.cheat_checked++;
			} else {
				if (!Q_stricmp (s, "gl"))
					max_gamma = 0.5;
				else
					max_gamma = 0.7;
			}*/

			s=Info_ValueForKey(ent->client->pers.userinfo, "gl_modulate");
			i = sscanf(s, "%f", &current_modulate);
			if (i == 0)
			{
				// gets called if gl_modulate is invalid (not float or empty)
				if (!Q_stricmp (s, "u"))
					ent->client->pers.modulate_was_undef = true;
				stuffcmd(ent, modulatecap);

			} else if (i == -1) {
				// gets called if gl_modulate isn't there
				stuffcmd(ent, "set gl_modulate $gl_modulate u\n");
				need_fov = true;
				need_vidrestart = true;
				ent->client->pers.cheat_checked++;

			} else {
				if (current_modulate > maxmodulate || current_modulate < 0) {
					// when capping, set back at quake2 default 1, which enforces
					// players to use proper modulate
					if (current_modulate > 2 && ent->client->pers.check_stage >= 3)
						SendToAdmins ("warning, %s[%s] tried using modulate %.2f after connecting!\n", ent->client->pers.netname, ent->client->pers.ip, current_modulate);
					ent->client->pers.cheat_checked++;
					need_vidrestart = true;
					stuffcmd(ent, va("set gl_modulate %.1f u\n", maxmodulate));
					//stuffcmd (ent, modulatecap);
					gi.dprintf("capped gl_modulate for %s\n", ent->client->pers.netname);
				}
			}

			s=Info_ValueForKey(ent->client->pers.userinfo, "vid_gamma");

			//r1: illegal string detection (using string such as '.'
			//    would allow essentially vid_gamma 0 = white ents)
			if (*s && !(scan_validate (s))) {
				//catch anyone who tried it
				if (!strcmp (s, ".")) {
					gi.bprintf (PRINT_HIGH,"%s was banned for illegal vid_gamma\n",ent->client->pers.netname);
					ban (ent, -1, "vid_gamma hack", NULL);
					kick (ent);
					return;
				}

				gi.dprintf ("malformed vid_gamma string '%s' from %s\n", s, ent->client->pers.netname);

				stuffcmd(ent, va ("set vid_gamma 0.75 u\n"));
				need_vidrestart = true;
			} else {
				i = sscanf(s, "%f", &v);
				if (current_modulate <= 1.65)
					max_gamma = 0.4;
				else if (current_modulate <= 1.1)
					max_gamma = 0.3;
				else
					max_gamma = 0.5;
				if (i == 0) {
					// gets called if gl_modulate is invalid (not float or empty)
					stuffcmd(ent, va ("set vid_gamma %.1f u\n", max_gamma));
				} else if (i == -1) {
					// gets called if gl_modulate isn't there
					stuffcmd(ent, "set vid_gamma $vid_gamma u\n");
					need_fov = true;
					need_vidrestart = true;
					ent->client->pers.cheat_checked++;
				} else {
					if (v < max_gamma) {
						ent->client->pers.cheat_checked++;
						need_vidrestart = true;
						if (ent->client->pers.check_stage >= 3)
							SendToAdmins ("warning, %s[%s] tried using vid_gamma %.2f after connecting!\n", ent->client->pers.netname, ent->client->pers.ip, v);
						stuffcmd (ent, va("set vid_gamma %.1f u\n", max_gamma));
						gi.dprintf("capped vid_gamma for %s\n", ent->client->pers.netname);
					}
				}
			}

			s=Info_ValueForKey(ent->client->pers.userinfo, "gl_picmip");
			i = sscanf(s, "%f", &v);
			if (i == 0) {
				// invalid or empty
				stuffcmd(ent, picmipcap);
			} else if (i == -1) {
				stuffcmd(ent, "set gl_picmip $gl_picmip u\n");
				ent->client->pers.cheat_checked++;
				need_fov = true;
				need_vidrestart = true;
			} else {
				if (v > 2 || v < 0) {
					ent->client->pers.cheat_checked++;
					need_vidrestart = true;
					stuffcmd(ent, picmipcap);
					gi.dprintf("capped gl_picmip for %s\n", ent->client->pers.netname);
				}
			}

			if (max_cl_maxfps->value) {
				s=Info_ValueForKey(ent->client->pers.userinfo, "cl_maxfps");
				i = sscanf(s, "%f", &v);
				if (i == 0) {
					stuffcmd(ent, va("set cl_maxfps %d u\n", (int)max_cl_maxfps->value));
				} else if (i == -1) {
					stuffcmd(ent, "set cl_maxfps $cl_maxfps u\n");
					need_fov = true;
				} else {
					if (v > max_cl_maxfps->value) {
						stuffcmd(ent, va("set cl_maxfps %d u\n", (int)max_cl_maxfps->value));
						gi.dprintf("highcapped cl_maxfps for %s\n", ent->client->pers.netname);
						ent->client->pers.cheat_checked++;
					} else if (v < 1) {
						gi.bprintf (PRINT_HIGH,"%s tried to ghost\n", ent->client->pers.netname);
						kick (ent);
						return;
					}
				}
			}

			if (min_cl_maxfps->value) {
				s=Info_ValueForKey(ent->client->pers.userinfo, "cl_maxfps");
				i = sscanf(s, "%f", &v);
				if (i == 0) {
					stuffcmd(ent, va("set cl_maxfps %d u\n", (int)max_cl_maxfps->value));
				} else if (i == -1) {
					stuffcmd(ent, "set cl_maxfps $cl_maxfps u\n");
					need_fov = true;
				} else {
					if (v < min_cl_maxfps->value) {
						stuffcmd(ent, va("set cl_maxfps %d u\n", (int)min_cl_maxfps->value));
						gi.dprintf("lowcapped cl_maxfps for %s\n", ent->client->pers.netname);
						ent->client->pers.cheat_checked++;
					}
				}				
			}

			if (max_rate->value) {
				s=Info_ValueForKey(ent->client->pers.userinfo, "rate");
				i = sscanf(s, "%f", &v);
				if (i == 0) {
					stuffcmd(ent, va("set rate %d u\n", (int)max_rate->value));
				} else if (i == -1) {
					stuffcmd(ent, "set rate $rate u\n");
					need_fov = true;
				} else {
					if (v > max_rate->value || v < 0) {
						stuffcmd(ent, va("set rate %d u\n", (int)max_rate->value));
						gi.dprintf("capped rate for %s\n", ent->client->pers.netname);
						ent->client->pers.cheat_checked++;
					} else if (v < 1000) {
						stuffcmd (ent, "set rate 1000 u\n");
						gi.dprintf("capped rate for %s\n", ent->client->pers.netname);
					}
				}
			}

			s=Info_ValueForKey(ent->client->pers.userinfo, "intensity");
			i = sscanf(s, "%f", &v);
			if (i == 0) {
				// invalid or empty
				//stuffcmd(ent, intensitycap);
				stuffcmd(ent, va("set intensity %.1f u\n", maxintensity));

			} else if (i == -1) {

				stuffcmd(ent, "set intensity $intensity u\n");

				need_fov = true;
				ent->client->pers.cheat_checked++;
				need_vidrestart = true;

			} else {
				if (v > maxintensity || v < 0) {
					ent->client->pers.cheat_checked++;
					need_vidrestart = true;
					//stuffcmd(ent, intensitycap);
					if (ent->client->pers.check_stage >= 3)
						SendToAdmins ("warning, %s[%s] tried using intensity %.2f after connecting!\n", ent->client->pers.netname, ent->client->pers.ip, v);
					stuffcmd(ent, va("set intensity %.1f u\n", maxintensity));
					gi.dprintf("capped intensity for %s\n", ent->client->pers.netname);
				}
			}

			s=Info_ValueForKey(ent->client->pers.userinfo, "sw_mipcap");
			i = sscanf(s, "%f", &v);
			if (i == 0) {
				// invalid or empty
				stuffcmd(ent, mipcapcap);

			} else if (i == -1) {
				 // not set
				stuffcmd(ent, "set sw_mipcap $sw_mipcap u\n");
				need_fov = true;
				ent->client->pers.cheat_checked++;

			} else {
				if (v > 2 || v < 0) {
					ent->client->pers.cheat_checked++;
					stuffcmd(ent, mipcapcap);
					gi.dprintf("capped sw_mipcap for %s\n", ent->client->pers.netname);
				}
			}

			s=Info_ValueForKey(ent->client->pers.userinfo, "sw_mipscale");
			i = sscanf(s, "%f", &v);
			if (i == 0) {
				// invalid or empty
				stuffcmd(ent, mipscalecap);

			} else if (i == -1) {
				// not set
				stuffcmd(ent, "set sw_mipscale $sw_mipscale u\n");
				need_fov = true;
				ent->client->pers.cheat_checked++;
			} else {
				if (v > 2 || v < 0) {
					ent->client->pers.cheat_checked++;
					stuffcmd(ent, mipscalecap);
					gi.dprintf("capped sw_mipscale for %s\n", ent->client->pers.netname);
				}
			}

			} // pers.check_stage > 1

			if (ent->client->pers.check_stage > 0) {

			// add any forced variables in this block

			s=Info_ValueForKey(ent->client->pers.userinfo, "gl_clear");
			i = sscanf(s, "%f", &v);
			if (i < 1) {
				if (i == -1)
				{
					stuffcmd (ent, "set gl_clear $gl_clear u\n");
				}
				else
				{
					if (Q_stricmp (s, "u") && ent->client->pers.modulate_was_undef)
						SendToAdmins ("Warning, %s[%s] has missing gl_modulate. check vid_ref.\n", ent->client->pers.netname, ent->client->pers.ip);
					stuffcmd(ent, glclearcap);
				}
				need_fov = true;
				ent->client->pers.cheat_checked++;
			} else {
				if (v != 0) {
					ent->client->pers.cheat_checked++;
					stuffcmd(ent, glclearcap);
					need_vidrestart = true;
					if (ent->client->pers.check_stage >= 3)
						SendToAdmins ("warning, %s[%s] tried using gl_clear after connecting!\n", ent->client->pers.netname, ent->client->pers.ip);
					gi.dprintf("turned off gl_clear for %s\n", ent->client->pers.netname);
				}
			}

			s=Info_ValueForKey(ent->client->pers.userinfo, "gl_polyblend");
			if (sscanf(s, "%f", &v) < 1) {
				stuffcmd(ent, polyblendcap);
				need_fov = true;
				ent->client->pers.cheat_checked++;
			} else {
				if (v != 1) {
					ent->client->pers.cheat_checked++;
					stuffcmd(ent, polyblendcap);
					gi.dprintf("turned on gl_polyblend for %s\n", ent->client->pers.netname);
				}
			}

			s=Info_ValueForKey(ent->client->pers.userinfo, "cl_blend");
			if (sscanf(s, "%f", &v) < 1) {
				stuffcmd(ent, clblendcap);
				need_fov = true;
				ent->client->pers.cheat_checked++;
			} else {
				if (v != 1) {
					ent->client->pers.cheat_checked++;
					stuffcmd(ent, clblendcap);
					gi.dprintf("turned on cl_blend for %s\n", ent->client->pers.netname);
				}
			}

			s=Info_ValueForKey(ent->client->pers.userinfo, "cl_testblend");
			if (sscanf(s, "%f", &v) < 1) {
				stuffcmd(ent, testblendcap);
				need_fov = true;
				ent->client->pers.cheat_checked++;
			} else {
				if (v != 0) {
					ent->client->pers.cheat_checked++;
					stuffcmd(ent, testblendcap);
					gi.dprintf("turned off cl_testblend for %s\n", ent->client->pers.netname);
				}
			}

			s=Info_ValueForKey(ent->client->pers.userinfo, "gl_dynamic");
			if (sscanf(s, "%f", &v) < 1) {
				stuffcmd(ent, dynamiccap);
				need_fov = true;
				ent->client->pers.cheat_checked++;
			} else {
				if (v != 1) {
					ent->client->pers.cheat_checked++;
					stuffcmd(ent, dynamiccap);
					gi.dprintf("turned on gl_dynamic for %s\n", ent->client->pers.netname);
				}
			}

			s=Info_ValueForKey(ent->client->pers.userinfo, "gl_lockpvs");
			if (sscanf(s, "%f", &v) < 1) {
				stuffcmd(ent, lockpvscap);
				need_fov = true;
				ent->client->pers.cheat_checked++;
			} else {
				if (v != 0) {
					ent->client->pers.cheat_checked++;
					stuffcmd(ent, lockpvscap);
					if (ent->client->pers.check_stage >= 3)
						SendToAdmins ("warning, %s[%s] tried using gl_lockpvs after connecting!\n", ent->client->pers.netname, ent->client->pers.ip);
					gi.dprintf("turned off gl_lockpvs for %s\n", ent->client->pers.netname);
				}
			}

			s=Info_ValueForKey(ent->client->pers.userinfo, "gl_monolightmap");
			if (sscanf(s, "%f", &v) < 1) {
				stuffcmd(ent, monolightmapcap);
				need_fov = true;
				ent->client->pers.cheat_checked++;
			} else {
				if (v != 0) {
					ent->client->pers.cheat_checked++;
					stuffcmd(ent, monolightmapcap);
					gi.dprintf("turned off gl_monolightmap for %s\n", ent->client->pers.netname);
				}
			}

			s=Info_ValueForKey(ent->client->pers.userinfo, "gl_shadows");
			if (sscanf(s, "%f", &v) < 1) {
				stuffcmd(ent, shadowscap);
				need_fov = true;
				ent->client->pers.cheat_checked++;
			} else {
				if (v != 0) {
					ent->client->pers.cheat_checked++;
					stuffcmd(ent, shadowscap);
					gi.dprintf("turned off gl_shadows for %s\n", ent->client->pers.netname);
				}
			}

			s=Info_ValueForKey(ent->client->pers.userinfo, "sw_drawflat");
			if (sscanf(s, "%f", &v) < 1) {
				stuffcmd(ent, drawflatcap);
				need_fov = true;
				ent->client->pers.cheat_checked++;
			} else {
				if (v != 0) {
					ent->client->pers.cheat_checked++;
					stuffcmd(ent, drawflatcap);
					if (ent->client->pers.check_stage >= 3)
						SendToAdmins ("warning, %s[%s] tried using sw_drawflat after connecting!\n", ent->client->pers.netname, ent->client->pers.ip);
					gi.dprintf("turned off sw_drawflat for %s\n", ent->client->pers.netname);
				}
			}

			s=Info_ValueForKey(ent->client->pers.userinfo, "sw_lockpvs");
			if (sscanf(s, "%f", &v) < 1) {
				stuffcmd(ent, swlockpvscap);
				need_fov = true;
				ent->client->pers.cheat_checked++;
			} else {
				if (v != 0) {
					ent->client->pers.cheat_checked++;
					stuffcmd(ent, swlockpvscap);
					gi.dprintf("turned off sw_lockpvs for %s\n", ent->client->pers.netname);
				}
			}

			s=Info_ValueForKey(ent->client->pers.userinfo, "fixedtime");
			if (sscanf(s, "%f", &v) < 1) {
				stuffcmd(ent, fixedtimecap);
				need_fov = true;
				ent->client->pers.cheat_checked++;
			} else {
				if (v != 0) {
					//ent->client->pers.cheat_checked++;
					//stuffcmd(ent, timescale);
					//gi.dprintf("turned off timescale for %s\n", ent->client->pers.netname);
					gi.bprintf (PRINT_HIGH,"%s is using a speed-hacked Quake II!\n",ent->client->pers.netname);
					ban (ent, -1, "auto-ban for speedhack q2 [ft]", NULL);
					kick (ent);
				}
			}

			s=Info_ValueForKey(ent->client->pers.userinfo, "gl_lightmap");
			i = sscanf(s, "%f", &v);
			if (i == 0) {
				stuffcmd(ent, lightmapcap);
				need_fov = true;
				ent->client->pers.cheat_checked++;
			} else if (i == -1) {
				stuffcmd (ent, "set gl_lightmap $gl_lightmap u\n");
			} else {
				if (v != 0) {
					//ent->client->pers.cheat_checked++;
					//stuffcmd(ent, timescale);
					//gi.dprintf("turned off timescale for %s\n", ent->client->pers.netname);
					gi.dprintf ("%s is using hacked gl_lightmap!\n", ent->client->pers.netname);
					gi.bprintf (PRINT_HIGH,"%s is using a vid-hacked Quake II!\n",ent->client->pers.netname);
					ban (ent, -1, "auto-ban for vidhack q2 [gl]", NULL);
					kick (ent);
				}
			}

			s=Info_ValueForKey(ent->client->pers.userinfo, "r_drawworld");
			i = sscanf(s, "%f", &v);
			if (i == -1) {
				stuffcmd(ent, "set r_drawworld $r_drawworld u\n");
				need_fov = true;
				ent->client->pers.cheat_checked++;
			} else if (i == 0) {
				stuffcmd(ent, drawworldcap);
			} else {
				if (v != 1) {
					//ent->client->pers.cheat_checked++;
					//stuffcmd(ent, timescale);
					//gi.dprintf("turned off timescale for %s\n", ent->client->pers.netname);
					gi.dprintf ("%s is using hacked r_drawworld!\n", ent->client->pers.netname);
					gi.bprintf (PRINT_HIGH,"%s is using a vid-hacked Quake II!\n",ent->client->pers.netname);
					ban (ent, -1, "auto-ban for vidhack q2 [rd]", NULL);
					kick (ent);
				}
			}

			s=Info_ValueForKey(ent->client->pers.userinfo, "r_fullbright");
			i = sscanf(s, "%f", &v);
			if (i == 0) {
				stuffcmd(ent, fullbrightcap);
				need_fov = true;
				ent->client->pers.cheat_checked++;
			} else if (i == -1) {
				stuffcmd (ent, "set r_fullbright $r_fullbright u\n");
			} else {
				if (v != 0) {
					//ent->client->pers.cheat_checked++;
					//stuffcmd(ent, timescale);
					//gi.dprintf("turned off timescale for %s\n", ent->client->pers.netname);
					gi.dprintf ("%s is using hacked r_fullbright!\n", ent->client->pers.netname);
					gi.bprintf (PRINT_HIGH,"%s is using a vid-hacked Quake II!\n",ent->client->pers.netname);
					ban (ent, -1, "auto-ban for vidhack q2 [rf]", NULL);
					kick (ent);
				}
			}

			s=Info_ValueForKey(ent->client->pers.userinfo, "timescale");
			if (sscanf(s, "%f", &v) < 1) {
				stuffcmd(ent, timescalecap);
				need_fov = true;
				ent->client->pers.cheat_checked++;
			} else {
				if (v != 1) {
					gi.bprintf (PRINT_HIGH,"%s is using a speed-hacked Quake II!\n",ent->client->pers.netname);
					ban (ent, -1, "auto-ban for speedhack q2 [ts]", NULL);
					SendToAdmins ("warning, detected timescale skew from %s: %.3f != 1.00 (please keep these details for R1)\n", ent->client->pers.netname, v);
					kick (ent);
				}
			}
		} // pers.check_stage > 0

		if (need_fov)
			ent->client->pers.stuff_request |= 2;

		if (need_vidrestart)
			ent->client->pers.stuff_request |= 1;

	}
}

void set_player_configstrings (edict_t *ent, unsigned class_type) {

	char	*newskin;

	newskin = classlist[class_type].skin;

#ifdef TWOBREEDERS
	if (class_type == CLASS_BREEDER)
		if (Q_stricmp (ent->model, "players/breeder/tris.md2"))
			newskin = "breeder2/skin";
#endif

	gi.configstring (CS_PLAYERSKINS+ent->s.number-1, va("%s\\%s", ent->client->pers.netname, newskin));

	/* moved to clientuserinfochanged
	if (ent->client->resp.team) {
		gi.configstring (CS_GENERAL+playernum,va("%s (%s)",colortext(ent->client->pers.netname), classlist[ent->client->resp.class_type].classname));
	}*/
}

int AddIP (char *ip, int seconds, char *banner, char *reason);
int ban (edict_t *victim, int seconds, char *reason, char *banner)
{
	char ip[22];
	char *bp;

	if (!victim->client)
		return 8;

	if (victim->client->pers.adminpermissions & (1 << PERMISSION_BAN))
		return 9;

	//how to ban an account holder :O
	if (*victim->client->pers.username && banaccountholders->value) {
		ToggleAccount (FindAccount(victim->client->pers.username), false);
		return IPF_NO_ERROR;
	}

	strncpy (ip, victim->client->pers.ip, sizeof(ip));

	bp = ip;

	while(*bp && *bp != ':')
		bp++;

	*bp = 0;

	if (!banner)
		banner = "[gloom-dll]";

	bp = ip+(strlen(ip)-1);
	while(*bp && *bp != '.')
		bp--;
	*bp = 0;

	strcat (ip, ".*");

	return AddIP (ip, seconds, banner, reason);
}

/*
CheckReconnectEnts

This looks for people who are in 'limbo' (bypassed the bot reconnect check and aren't
really in game) and kicks em.
*/

//FIXME: do we even need this?
/*void CheckReconnectEnts (void)
{
	int j;
	edict_t *ent;

	for (ent = &g_edicts[1], j = ent-g_edicts-1 ; ent <= &g_edicts[game.maxclients] ; ent++, j++) {

		if (!ent->inuse && !ent->client->pers.ingame && !reconnect[j].valid && reconnect[j].completed && reconnect[j].time < level.time && !Q_stricmp(reconnect[j].ip, ent->client->pers.ip)) {
			kick (ent);
			reconnect[j].valid = reconnect[j].completed = reconnect[j].time = 0;
		}
	}
}*/

void ValidateReconnect (edict_t *ent)
{
	int playernum = ent-g_edicts-1;

	if (reconnect[playernum].reconnecting && !Q_stricmp (ent->client->pers.ip, reconnect[playernum].ip))
		reconnect[playernum].token--;
}

/*
ClientConnect

Called when a player begins connecting to the server.
The game can refuse entrance to a client by returning false.
If the client is allowed, the connection process will continue
and eventually get to ClientBegin()
Changing levels will NOT cause this to be called again, but
loadgames will.
*/
#define	BAN_UNABLE_TO_DETERMINE_IP	-1
#define	BAN_NOT_BANNED				-2
qboolean EXPORT ClientConnect (edict_t *ent, char *userinfo){
	char *ipa = NULL, *value = NULL;
	int baninfo = 0;
	/*
	On ClientConnect, the \ip variable is overwritten to the userinfo string around
	128 bytes mark, so the only variable/value you can be sure to get from userinfo on
	this point is \ip. All other variables should be off-limits.
	Also this prevents using userinfo string until it's triggered from client-side
	(ie. using fov stuff).

	\ip variable isn't available after ClientConnect so it needs to be copied to elsewhere
	if access to it is desired (Quake2 calls ClientUserinfoChanged with the \ip after
	ClientConnect).

  Code should prevent accessing variables got from userinfo until it's contents have been
	checked (and client->pers.userinfo set) in ClientUserinfoChanged.
	*/

#ifdef ADMINDLLHACK
	//bypass any "admin" DLLs... ugly as hell :(
	if (game.dllhack == false && admindllbypass->value) {
		FILE *admindll;

		game.dllhack = true;
		admindll = fopen ("release/gamex86.dll","r");
		if (admindll) {
			fclose (admindll);

			if (level.time  < admindlltimer->value) {
				Info_SetValueForKey (userinfo, "rejmsg", va("Server is initializing, please try again in %d secs.", (int)(admindlltimer->value - level.time)));
				return false;
			}

			rename ("release/gamex86.dll", "release/gamex86.gloom-hax");
			gi.dprintf ("ClientConnect: Attempting to bypass Server Admin DLL...\n");

			//FIXME: under r1q2 at least this map command will be denied
			gi.AddCommandString (va("map %s\n", level.mapname));
		}
	}

	if (!game.init) {
		game.init = true;
	}
#endif

	if (!Info_Validate(userinfo))
	{
		userinfo[0] = '\0';
		Info_SetValueForKey(userinfo,"rejmsg","Your userinfo string is malformed, please restart your Quake 2.");
		return false;
	}

	if (!strstr (userinfo, "\\user\\") && team_info.numplayers[TEAM_ALIEN] + team_info.numplayers[TEAM_HUMAN] + team_info.numplayers[TEAM_NONE] >= (maxclients->value - adminslots->value)) {
		userinfo[0] = '\0';
		Info_SetValueForKey(userinfo,"rejmsg","Server is full.");
		return false;
	}

	// find \ip variable
	ipa = strstr(userinfo,"\\ip\\");

	if (ipa == NULL) {
		userinfo[0] = '\0';
		Info_SetValueForKey(userinfo,"rejmsg","Your userinfo string is malformed, please restart your Quake 2.");
		return false;
	}

	// skip "\ip\"
	ipa += 4;

	// if \ip is on banned list, kick the player
	baninfo = SV_FilterPacket(ipa);

	//exception ban (couldn't determine IP)
	if (baninfo == BAN_UNABLE_TO_DETERMINE_IP)
	{
		userinfo[0] = '\0';
		Info_SetValueForKey(userinfo,"rejmsg","Your userinfo string is malformed, please restart your Quake 2.");
		return false;
	}

	if (baninfo != BAN_NOT_BANNED) {
		//r1: empty out the userinfo, exe only looks for rejmsg after this point so the string can safely
		//    be destroyed.
		
		// ban here if accounts are not in use
		if (!accounts->string[0])
		{
			userinfo[0] = '\0';

			if (ipfilters[baninfo].expiretime > 0) {
				Info_SetValueForKey (userinfo, "rejmsg",va("You are banned for %s (id=%-2.2X)", TimeToString(ipfilters[baninfo].expiretime - time(0)), baninfo));
				return false;
			} else {
				Info_SetValueForKey (userinfo, "rejmsg", "You are banned from this server.");
				return false;
			}
		}
	}

	// check for password (subject to userinfo string corruption)
	value = Info_ValueForKey (userinfo, "password");
	if (password->string[0] && Q_stricmp(password->string, "none") &&
		Q_stricmp(password->string, value))
	{
		userinfo[0] = '\0';
		Info_SetValueForKey(userinfo, "rejmsg", "Password required or incorrect.");
		return false;
	}

	// check for server dropping a client without ClientDisconnect
	if (ent->classname)
	{
		gi.dprintf("ClientConnect: Server didn't properly disconnect old client #%ld..\n", ent-g_edicts); //FV7 %d to %ld
		if (ent->client->pers.ingame)
			team_info.numplayers[ent->client->resp.team]--;

		// get rid of flashlight if one
	 	if ( ent->client->resp.flashlight )
		{
			G_FreeEdict(ent->client->resp.flashlight);
			ent->client->resp.flashlight = NULL;
		}

		//this cleans up a previous players mess if they didn't disconnect
		//properly and ClientDisconnect() wasn't called
		CleanUpOwnerStuff (ent);

		ent->classname = NULL;
	}

	// clear persistant info for client
	InitClientPersistant (ent->client);

	// take copy of the \ip.
	strncpy(ent->client->pers.ip,ipa,sizeof(ent->client->pers.ip));

	// check reconnects
	ValidateReconnect (ent);

	gi.dprintf ("%s:connect\n", ipa);

	return true;
}

/*void AddIPtoLog (char ip[22])
{
	int i;

	char *bp = ip;
	while(*bp && *bp != ':')
		bp++;
	*bp = 0;

	for (i = 0; i < MAX_CLIENTS; i++) {
		if (!(*vote.iplog[i])) {
			strncpy (vote.iplog[i], ip, sizeof(vote.iplog[i]));
		}
	}
}*/

/*
ClientDisconnect

Called when a player disconnects from the server.
Sometimes not called when server drops a client.
Will not be called between levels.
*/
void ChaseNext(edict_t *ent);
void RemoveFromSpawnQueue (edict_t *ent);
void EXPORT ClientDisconnect (edict_t *ent)
{
	int bantime;
	int playernum;
	int i;
	edict_t	*other;

	// always unlink
	gi.unlinkentity (ent);

	//this will do quite a nice cleanup job on the player
	//if (ent->health > 0)
	if (ent->client->resp.team)
		T_Damage (ent, ent, ent, vec3_origin, vec3_origin, vec3_origin, 10000, 0, DAMAGE_NO_PROTECTION, MOD_CHANGECLASS);

	playernum = ent-g_edicts-1;

	for (other = g_edicts+1; other->client; other++)
	{
		if (!other->inuse)
			continue;

		if (other->client->pers.ignorelist[playernum])
			other->client->pers.ignorelist[playernum] = false;

		if (other->client->pers.last_private == ent)
			other->client->pers.last_private = NULL;

		if (other->client->chase_target == ent) {
			// boot chasers into observer mode
			other->client->chase_target = NULL;
			other->client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
		}
	}

	ent->client->chase_target = NULL;

	//if they're reconnecting so they don't have to earn their negative
	//score back screw them. 2min for each -1
	bantime = 0;

/*Savvy this is a bit stupid,
(n * absolute_value(-n))
instead of just
(-n * -n)
in other words, convert to positive, then multiply or multiply to always be positive?
*/

	//r1: don't ban unconnected ents
	if (ent->client->pers.ingame)
	{
		if (!sv_cheats->value)
		{
			//frags
			if (ent->client->resp.score < 0)
			{
				bantime += 120 * abs(ent->client->resp.score);
				gi.dprintf ("ClientDisconnect: Adding bantime for %s, negative frags (%d)\n", ent->client->pers.netname, bantime);
			}
			else if (ent->client->resp.score > 0)
			{
				bantime -= ent->client->resp.score * 20;
				gi.dprintf ("ClientDisconnect: Reducing bantime for %s, positive frags (%d)\n", ent->client->pers.netname, bantime);
			}
				
			//points
			if (ent->client->resp.total_score < 0)
			{
				bantime += abs(ent->client->resp.total_score) * 30;
				gi.dprintf ("ClientDisconnect: Adding bantime for %s, negative score (%d)\n", ent->client->pers.netname, bantime);
			}
			else if (ent->client->resp.total_score > 0)
			{
				bantime -= ent->client->resp.total_score * 2;
				gi.dprintf ("ClientDisconnect: Reducing bantime for %s, positive score (%d)\n", ent->client->pers.netname, bantime);
			}

			//tks
			if (ent->client->resp.teamkills > 15)
			{
				bantime += ent->client->resp.teamkills * 4;
				gi.dprintf ("ClientDisconnect: Adding bantime for %s, teamkills (%d)\n", ent->client->pers.netname, bantime);
			}
		}

		if (ent->client->resp.teamchanges > maxswitches->value)
		{
			bantime += 300;
			gi.dprintf ("ClientDisconnect: Adding bantime for %s, team changing (%d)\n", ent->client->pers.netname, bantime);
		}

		// ban 5mins if muted
		if (ent->client->pers.muted)
		{
			bantime += 300;
			gi.dprintf ("ClientDisconnect: Adding bantime for %s, muted (%d)\n", ent->client->pers.netname, bantime);
		}

		// additional time if trying to evade vote spam protection
		if (ent->client->last_vote_time > level.time)
		{
			bantime += (ent->client->last_vote_time - level.time) * 1.25f;
			gi.dprintf ("ClientDisconnect: Adding bantime for %s, vote timer (%d)\n", ent->client->pers.netname, bantime);

			if (ent->client->resp.failed_votes == FAILED_VOTES_LIMIT)
			{
				bantime += 300;
				gi.dprintf ("ClientDisconnect: Adding bantime for %s, vote timer + failed votes (%d)\n", ent->client->pers.netname, bantime);
			}
		}

		if (ent->client->resp.failed_votes == FAILED_VOTES_LIMIT)
		{
			bantime += 300;
			gi.dprintf ("ClientDisconnect: Adding bantime for %s, failed votes (%d)\n", ent->client->pers.netname, bantime);
		}
	}

	if (ent->client->ctf_grapple)
		CTFResetGrapple(ent->client->ctf_grapple);

	RemoveFromSpawnQueue (ent);

	log_disconnection (ent);

	// if ingame is set, they have been added to team
	if (ent->client->pers.ingame)
		team_info.numplayers[ent->client->resp.team]--;

	// update mainmenu
	Updateteam_menu();

	if (level.time > 10 && ent->client->pers.connected) {
		/* do others even care how long one has been playing? usually ppl just jump in and out. -ank
		char timeplayed[64] = "";
		strcpy (timeplayed, TimeToString(time(0) - ent->client->pers.starttime));
		gi.bprintf (PRINT_HIGH, "%s disconnected (%s)\n", ent->client->pers.netname, timeplayed);
		*/
		if (ent->client->resp.visible && !level.suddendeath)
			gi.bprintf (PRINT_HIGH, "%s disconnected\n", ent->client->pers.netname);
		game.seenclients++;
	}

	// send effect
	if (ent->health > 0 && ent->client->resp.team) {
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_LOGOUT);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
	}

	//r1: check vote.type or someone disconnecting with the same edict number as the map
	//    vote would be erranesouly banned if that's a word.
/*Savvy let's leave this up to the coder, R1*/
#ifdef BAN_VOTE_VICTIM_ON_DISCONNECT
	if (active_vote && (vote.type == VOTE_KICK || vote.type == VOTE_TEAMKICK || vote.type == VOTE_MUTE)) {
		//ban vote evaders
		if (&g_edicts[vote.target_index] == ent) {
			gi.bprintf(PRINT_HIGH,"[vote] Vote cancelled, the victim has left. Banned him anyway.\n");
			bantime += banmaps->value;
			active_vote=false;
			vote.timer=0;
		}

		//log reconnect voters
		/*if (ent->client->resp.voted)
			AddIPtoLog (ent->client->pers.ip);*/
	}
#endif

	if (ent->client->resp.score > 0 && ent->client->resp.team != TEAM_NONE && !(level.intermissiontime)) {
		//r1ch - ghost code code :)
		//FIXME: we only need ghostcode, frags, total_score, team and expiretime, don't bother with others
		//FIXME: and set the variables when changes happen, server might not call ClientDisconnect
		for (i = 0;i <= 64;i++) {
			if (fragsave[i].inuse == 0) {
				fragsave[i].starttime = ent->client->pers.starttime;
				fragsave[i].total_score = ent->client->resp.total_score;
				fragsave[i].frags = ent->client->resp.score;
				fragsave[i].team = ent->client->resp.team;
				fragsave[i].oldclass = ent->client->resp.class_type;
				fragsave[i].expiretime = level.time + 600;
				fragsave[i].inuse = 1;
				strcpy(fragsave[i].netname,ent->client->pers.netname);
				fragsave[i].ghostcode = ent->client->pers.ghostcode;
				VectorCopy(ent->s.origin,fragsave[i].oldorigin);
				VectorCopy(ent->s.angles,fragsave[i].oldangles);
				break;
			} else if (fragsave[i].inuse == 1 && level.time > fragsave[i].expiretime) {
				fragsave[i].inuse = 0;
				i--;
			}
		}
	}

	if (bantime > 0)
		ban (ent, bantime, "disconnect penalties", NULL);

	// get rid of flashlight if one
 	if ( ent->client->resp.flashlight )
	{
		G_FreeEdict(ent->client->resp.flashlight);
		ent->client->resp.flashlight = NULL;
	}

	if (ent->client->pers.connected) {
		memset (&reconnect[playernum], 0, sizeof(reconnect[playernum]));
	}

	reconnect[playernum].valid = false;

	CleanUpOwnerStuff (ent);

	ent->inuse = false;
	ent->classname = NULL;
	ent->s.modelindex = ent->s.sound = ent->s.event = ent->s.effects = 0;

	gi.configstring (CS_PLAYERSKINS+playernum, "");
	//gi.configstring (CS_GENERAL+playernum, "");
}

void ListGhosts(void)
{
	int i = 0;
	gi.dprintf ("+------------+-----+---------------+\n");
	gi.dprintf ("|Code        |Frags|Owner          |\n");
	gi.dprintf ("+------------+-----+---------------+\n");
	for (i = 0;i <= 64;i++) {
		if (fragsave[i].inuse == 1) {
			gi.dprintf ("|%-12d|%-5d|%-15.15s|\n",fragsave[i].ghostcode,fragsave[i].frags,fragsave[i].netname);
		}
	}
	gi.dprintf ("+------------+-----+---------------+\n");
}

// Jump Attack
void player_jump_touch (edict_t *self, edict_t *other, cplane_t *plane __attribute__((unused)), csurface_t *surf __attribute__((unused))) //FV7 attributes
{
	vec3_t	point;
	vec3_t	normal;
	qboolean result;
	float	xyspeed = 0;
	int mod=MOD_JUMPATTACK;

	int		damage;

	if (!other->takedamage)
		return;

	//r1: weird bug here!!
	if (!other->classname)
	{
		gi.bprintf (PRINT_HIGH, "WARNING: player_jump_touch with empty classname!! (et=%d, mi=%d)\n", other->enttype, other->s.modelindex);
		gi.cprintf (self, PRINT_HIGH, "Please inform the developer of this error.\n"); /*Savvy used to say go to edgeirc.net #gloom - HAHA, they would die*/
		return;
	}

	//gi.bprintf (PRINT_HIGH, "%s pjt %s: %p %p\n", self->client->pers.netname, other->client->pers.netname, self->touch, other->touch);

	damage = classlist[self->client->resp.class_type].damage;
	if (!damage)
		return;

	if(other->client && self->client->resp.class_type == CLASS_HATCHLING && (other->client->resp.class_type == CLASS_MECH || other->client->resp.class_type == CLASS_EXTERM))
		damage *= 0.6f;

	if (self->client && other->client)
	{
		if (self->client->resp.class_type == CLASS_MECH)
		{
			if (fabs(self->client->oldvelocity[2]) < 100) {
				//if (classlist[other->client->resp.class_type].frags_needed >= 3)
				//	return;
				mod = MOD_MECHSQUISH;
			} else {
				mod = MOD_OW_MY_HEAD_HURT;
			}
		} else if (self->client->resp.class_type == CLASS_HATCHLING && (other->client->resp.class_type == CLASS_GRUNT || other->client->resp.class_type == CLASS_ENGINEER)) {
			damage += 25;
		}
		else if (self->client->resp.class_type == CLASS_STALKER && (other->client->resp.class_type == CLASS_GRUNT || other->client->resp.class_type == CLASS_ENGINEER))
		{
			damage += 75 + random() * 30;
		}
	}

	if (other->client && other->client->resp.team == self->client->resp.team)
		return;

	// don't hurt my own units
	if (self->client->resp.team == TEAM_ALIEN) {
		if (other->svflags & SVF_MONSTER)
			return;

		switch (other->enttype) {
			case ENT_PROXY_SPORE:
			case ENT_TELEPORTER:
			case ENT_PROXYSPIKE:
				return;
			case ENT_FUNC_DOOR:
			case ENT_FUNC_EXPLOSIVE:
				damage /= 6;
				break;
			case ENT_TURRETBASE:
			case ENT_TURRET:
			case ENT_MGTURRET:
				if (self->client->resp.class_type != CLASS_STALKER)
					damage /= 4;
				break;
			default:
				break;
		}

		if(other->client && other->client->resp.class_type == CLASS_MECH)
			damage = damage / 1.8f;
	} else if(self->client->resp.team == TEAM_HUMAN && other->flags & FL_CLIPPING) {
		return;
	}

	// If on ground, don't do damage?

	if (self->health < 1)
	{
		self->touch = NULL;
		return;
	}

	/*if(self->groundentity)
	{
		if(!self->groundentity->takedamage)
		{
			self->touch = NULL;
			return;
		}
	}*/

	if (self->client->resp.team == TEAM_ALIEN && (classlist[self->client->resp.class_type].frags_needed >= 3 || self->client->resp.class_type == CLASS_HATCHLING || self->client->resp.class_type == CLASS_KAMIKAZE))
	{
		int i = 0;
		for (i = 0;i <= 4;i++)
			xyspeed += self->client->lastspeeds[i];

		xyspeed /= 5;

		if (!other->groundentity)
		{
			//gi.bprintf (PRINT_HIGH, "not in air, doubling base dmg %d -> %d\n", damage, (int)(damage * 1.5f));
			damage *= 1.5f;
		}

		if (self->client->resp.class_type == CLASS_HATCHLING || self->client->resp.class_type == CLASS_KAMIKAZE)
		{
			if (xyspeed > 500)
			{
				int add = (xyspeed / 24);
				//gi.bprintf (PRINT_HIGH, "velo dmg: speed (%.5f) added %d\n", xyspeed, add);
				damage += add;
			}
		} else if (classlist[self->client->resp.class_type].frags_needed >= 3 && xyspeed > 400) {
			//gi.bprintf (PRINT_HIGH, "%d damage before\n",damage);
			damage *= (xyspeed / 400);
			//if (xyspeed > 350)
				mod = MOD_RUN_OVER;
			if (damage <=0)
				damage = classlist[self->client->resp.class_type].damage / 2;
		}

		//gi.bprintf (PRINT_HIGH, "%d damage, %f speed\n",damage,xyspeed);
	}

	VectorCopy (self->velocity, normal);
	VectorNormalize(normal);
	VectorMA (self->s.origin, self->maxs[0], normal, point);
	result = T_Damage (other, self, self, normal, point, normal, damage, damage, 0, mod);

	if (result && other->enttype != ENT_TELEPORTER && other->enttype != ENT_COCOON)
		gi.sound(self, CHAN_AUTO, SoundIndex (mutant_mutatck2), 1, ATTN_NORM, 0);

	//self->touch = NULL;

	if (other->client)
	{
		if (classlist[other->client->resp.class_type].frags_needed <= 2)
			self->touch_debounce_time = level.time + 0.15f;
		else if  (classlist[other->client->resp.class_type].frags_needed <= 3 || self->client->resp.class_type == CLASS_MECH)
			self->touch_debounce_time = level.time + 0.33f;
		else
			self->touch_debounce_time = level.time + 0.45f;
	}
	else
	{
		//FIXME
		self->touch_debounce_time = level.time + 0.25f;
		/*if (classlist[self->client->resp.class_type].frags_needed <= 2)
			
		else if  (classlist[self->client->resp.class_type].frags_needed <= 3 || self->client->resp.class_type == CLASS_MECH)
			self->touch_debounce_time = level.time + 0.33f;
		else
			self->touch_debounce_time = level.time + 0.45f;*/
	}
}

void JumpSlash(edict_t *ent, int damage)
{
	vec3_t		angles, forward, start;

	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	VectorCopy (ent->s.origin, start);
	start[2] += ent->viewheight;
	angles[2] += 10;
	AngleVectors (angles, forward, NULL, NULL);

	if (ent->client->invincible_framenum > level.framenum)
		ent->client->invincible_framenum = level.framenum;

	fire_melee(ent, start, forward, damage, damage, 56);
}

/*unsigned CheckBlock (void *b, int c)
{
	int	v,i;
	v = 0;
	for (i=0 ; i<c ; i++)
		v+= ((byte *)b)[i];
	return v;
}

void PrintPmove (pmove_t *pm)
{
	unsigned	c1, c2;

	c1 = CheckBlock (&pm->s, sizeof(pm->s));
	c2 = CheckBlock (&pm->cmd, sizeof(pm->cmd));
	Com_Printf ("sv %3i:%i %i\n", pm->cmd.impulse, c1, c2);
}*/

edict_t	*pm_passent;
// pmove doesn't need to know about passent and contentmask
trace_t	EXPORT PM_trace (vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end)
{
	trace_t tr;
	// use the true client bbox on enhanced client

/*#ifdef R1Q2_BUILD
	VectorCopy (pm_passent->mins,mins);
	VectorCopy (pm_passent->maxs,maxs);
#else*/
	if (basetest->value) {
		VectorCopy (pm_passent->mins,mins);
		VectorCopy (pm_passent->maxs,maxs);
	}
#ifdef R1Q2_BUILD
	/*else {
		VectorSet (mins, -16, -16, -24);
		VectorSet (maxs,  16,  16, 32);
	}*/
#endif
//#endif

	//check if the trace will hit a client with our real bbox
	if (playerclipfix->value)
	{
		tr = gi.trace (start, pm_passent->mins, pm_passent->maxs, end, pm_passent, CONTENTS_MONSTER);
		if (tr.ent && tr.ent->client)
		{
			return tr;
		}
	}

	//nope, lets trace to the world with our grunt bbox
	if (pm_passent->health > 0)
		return gi.trace (start, mins, maxs, end, pm_passent, MASK_PLAYERSOLID);
	else
		return gi.trace (start, mins, maxs, end, pm_passent, MASK_DEADSOLID);
}

//for impulse
void Use_Health (edict_t *ent, const gitem_t *item);
void Cmd_Flashlight_f (edict_t *ent);
void Cmd_Grenade_f (edict_t *ent);

/*
ClientThink

This will be called once for each client frame, which will
usually be a couple times for each server frame.

client frame rate / server frame rate = calls per server frame
ie.	90 fps / 10 fps = 9 calls per server frame
	50 fps / 10 fps = 5 calls
	30 fps / 10 fps = 3 calls
use showpackets command on client to see it in real time
I think one should do complex movement (velocities and origin) on server frame
*/
void EXPORT ClientThink (edict_t *ent, usercmd_t *ucmd)
{
	static gclient_t	*client;
	static edict_t	*other;
	static int		i, j;
	static pmove_t	pm;
	qboolean wasonground;

	client = ent->client;

	if (level.intermissiontime && (level.intermissiontime < level.framenum))
	{
		client->ps.pmove.pm_type = PM_FREEZE;
		// can exit intermission after five seconds
		/*if (level.framenum > level.intermissiontime + 12*10
			&& (ucmd->buttons & (BUTTON_ATTACK|BUTTON_USE)) ) // was _ANY, enter key usually triggers it (chat!)
			level.exitintermission = true;*/
		return;
	}

	//r1: we want to set buttons preferably before we return so the chasecam IR toggle
	//    works.

	client->oldbuttons = client->buttons;
	client->buttons = ucmd->buttons;
	client->latched_buttons |= client->buttons & ~client->oldbuttons;

	// chasecammers can't control themselves
	if (client->chase_target) {
		for (i=0 ; i<3 ; i++) {
			client->resp.cmd_angles[i] = SHORT2ANGLE(ucmd->angles[i]);
		}
		return;
	}

	pm_passent = ent;

	// set up for pmove
	memset (&pm, 0, sizeof(pm));

	client->ps.pmove.pm_type = PM_NORMAL;

	if (ent->movetype == MOVETYPE_NOCLIP)
		client->ps.pmove.pm_type = PM_SPECTATOR;
	else if (ent->health <= 0) {
		// if were either playermodel, set us PM_DEAD, else we're PM_GIB.
		if (ent->movetype != MOVETYPE_BOUNCE)
			client->ps.pmove.pm_type = PM_DEAD;
		else
			client->ps.pmove.pm_type = PM_GIB;
	} else if (client->resp.turret) {
		client->ps.pmove.pm_type = PM_DEAD;
		if (ucmd->sidemove > 0) {
			client->resp.turret->movedir[YAW]++;
		} else if (ucmd->sidemove < 0) {
			client->resp.turret->movedir[YAW]--;
		}
	}

	if ((client->resp.flying && client->resp.class_type==CLASS_WRAITH) || (client->ctf_grapple && client->ctf_grapple->spawnflags == CTF_GRAPPLE_STATE_PULL) || client->resp.turret) {
		client->ps.pmove.gravity = 0;
	} else {
		if (!ent->gravity) {
			client->ps.pmove.gravity = sv_gravity->value;
		} else {
			client->ps.pmove.gravity = ent->gravity;
		}
	}

	// copy last playerinfo pmove_state_t info over
	pm.s = client->ps.pmove;

	for (i=0 ; i<3 ; i++)
	{
		pm.s.origin[i] = ent->s.origin[i]*8;
		pm.s.velocity[i] = ent->velocity[i]*8;
	}

	if (game.paused && !(ent->client->pers.adminpermissions & (1 << PERMISSION_BENEFITS))) {
		//pm.s.pm_time = 255;
		client->frozentime = 1;
		client->frozenmode = PMF_TIME_TELEPORT;
	}

	//if frozen, nuke all their input so they can't move
	if (client->frozentime > 0) {
		if (client->frozenmode == PMF_TIME_TELEPORT) {
			if (!ent->groundentity)
				pm.s.pm_flags = PMF_NO_PREDICTION;
			else
				pm.s.pm_flags = PMF_TIME_TELEPORT;
		} else {
			pm.s.pm_flags = client->frozenmode;
		}
		//gi.bprintf (PRINT_HIGH, "frozen for %f\n", client->frozentime);
		if (client->frozentime > .2)
			pm.s.pm_time = 36;
		else if (client->frozentime > .1)
			pm.s.pm_time = 24;
		else
			pm.s.pm_time = 12;
		ucmd->forwardmove = ucmd->upmove = ucmd->sidemove = 0;
	}

	if (client->slow_timer > 0)
	{
		if (ucmd->forwardmove > (DEFAULT_VELOCITY*2) * client->slow_threshold)
			ucmd->forwardmove = (DEFAULT_VELOCITY*2) * client->slow_threshold;
		else if (ucmd->forwardmove < -((DEFAULT_VELOCITY*2) * client->slow_threshold))
			ucmd->forwardmove = -((DEFAULT_VELOCITY*2) * client->slow_threshold);

		if (ucmd->sidemove > (DEFAULT_VELOCITY*2) * client->slow_threshold)
			ucmd->sidemove = (DEFAULT_VELOCITY*2) * client->slow_threshold;
		else if (ucmd->sidemove < -((DEFAULT_VELOCITY*2) * client->slow_threshold))
			ucmd->sidemove = -((DEFAULT_VELOCITY*2) * client->slow_threshold);

		if (client->slow_last_jump_framenum > level.framenum)
			ucmd->upmove = 0;
	}

	/*if (pm.s.delta_angles[2] > 10 || pm.s.delta_angles[2] < -10)
		pm.s.delta_angles[2] /= 2;
	else
		pm.s.delta_angles[2] = 0;*/

	// if pm.s have been changed without going thru gi.Pmove, reset delta encoding state kept on pm.s
	if (memcmp(&client->old_pmove, &pm.s, sizeof(pm.s))) {
		pm.snapinitial = true;
	}

	wasonground = ent->groundentity ? true : false;

	// player view and movement
	pm.cmd = *ucmd;

	// set the bbox if we can use it for Pmove calc
	if (basetest->value) {
		VectorCopy (ent->mins,pm.mins);
		VectorCopy (ent->maxs,pm.maxs);
	}
#ifdef R1Q2_BUILD
	/*else {
		VectorSet (pm.maxs, 0, 0, 0);
		VectorSet (pm.mins, 0, 0, 0);
	}*/
#endif

	// hax to set proper bbox for ducking (r1ch q2)
//	if (!classlist[client->resp.class_type].fixview && (client->ps.pmove.pm_flags & PMF_DUCKED)) {
//		pm.maxs[2] = 4;
//	}

	/*if (client->ps.pmove.pm_type == PM_GIB) {
		pm.mins[0] = -8; pm.mins[1] = -8; pm.mins[2] = -2;
		pm.maxs[0] = 8; pm.maxs[1] = 8; pm.maxs[2] = 4;
	}*/

	pm.trace = PM_trace;	// adds default parms
	pm.pointcontents = gi.pointcontents;

	// gi.Pmove handles client movement against world, bounding, clipping and such
	// pm.trace and pm.pointcontents is used for ent and content handling
	// gi.Pmove takes in last client actions (ucmd) and gives out valid origin and velocity + few other values

	// perform a pmove
	gi.Pmove (&pm);

	// save results of pmove
	client->ps.pmove = pm.s;
	client->old_pmove = pm.s;

	// FIXME: cap the velocities elsewhere?
	for (i=0 ; i<2 ; i++)
	{
		ent->s.origin[i] = pm.s.origin[i]*0.125f;
		ent->velocity[i] += ((pm.s.velocity[i]*0.125f) - ent->velocity[i]);//classlist[ent->client->resp.class_type].walkspeed*modifier;
	}
	ent->s.origin[2] = pm.s.origin[2]*0.125f;

	if(pm.s.velocity[2]*0.125f - ent->velocity[2] > 0.0)
		ent->velocity[2] += (pm.s.velocity[2]*0.125f - ent->velocity[2]) * classlist[client->resp.class_type].walkspeed;

	else
		ent->velocity[2] = pm.s.velocity[2]*0.125f;

	if ((pm.s.pm_flags & PMF_DUCKED) && !classlist[client->resp.class_type].fixview)
		ent->viewheight = pm.viewheight;
	else {
		if (ent->deadflag && client->resp.class_type != CLASS_OBSERVER)
			ent->viewheight = 8;
		else
			ent->viewheight = classlist[client->resp.class_type].viewheight;
	}

	/*if (client->resp.class_type == CLASS_GRUNT && client->resp.upgrades & UPGRADE_SMARTGUN)
	{
		vec3_t		dir;

		if (ent->target_ent)
		{
			if (!ent->target_ent->inuse || ent->target_ent->health <= 0)
			{
				ent->target_ent = NULL;
			}
			else
			{
				VectorSubtract (ent->target_ent->s.origin, ent->s.origin, dir);

				vectoangles (dir, ent->client->ps.viewangles);
				vectoangles (dir, ent->client->v_angle);

				ent->client->ps.pmove.delta_angles[0] = ANGLE2SHORT(ent->client->v_angle[0] - ent->client->resp.cmd_angles[0]);
				ent->client->ps.pmove.delta_angles[1] = ANGLE2SHORT(ent->client->v_angle[1] - ent->client->resp.cmd_angles[1]);
				ent->client->ps.pmove.delta_angles[2] = ANGLE2SHORT(ent->client->v_angle[2] - ent->client->resp.cmd_angles[2]);
			}
		}
	}*/

	// copy clients' last viewangle
	for (i=0 ; i<3 ; i++)
		client->resp.cmd_angles[i] = SHORT2ANGLE(ucmd->angles[i]);

	// Gloom: Try to control walking speeds.

	// new jump from ground!
	// FIXME: move enhanced jumping out of clientthink
	// FIXME: move jump sound playing out of clientthink (it can't be send anyway until server frame)
	if ((pm.cmd.upmove >= 10) && ent->groundentity && !pm.groundentity)
	{
		if (client->slow_timer > 0)
		{
			client->slow_last_jump_framenum = level.framenum + 25;
			stuffcmd (ent, va("set cl_upspeed 0\n"));
		}
		// Jump attack?
		if(client->jumpattack)
		{
			for (i=0 ; i<2 ; i++)
				ent->velocity[i] = ent->velocity[i]*1.2f;

			if(client->resp.class_type == CLASS_DRONE)
			{
				ent->velocity[2] = ent->velocity[2] * DRONE_JUMP_HEIGHT_MODIFIER; /*Savvy edit, get this into balance.h, also was 1.2f originally*/
			}
			/*else if (client->resp.class_type == CLASS_GUARDIAN)
			{
				if (VectorLength (ent->velocity) > 500) {
					VectorNormalize(ent->velocity);
					VectorScale(ent->velocity, 500, ent->velocity);
				}
			}*/
			else if (client->resp.class_type == CLASS_STALKER)
			{
				if (!(client->resp.upgrades & UPGRADE_ANABOLIC) && VectorLength (ent->velocity) > 600) {
					VectorNormalize(ent->velocity);
					VectorScale(ent->velocity, 600, ent->velocity);
				}
			}
			else
			{
				ent->velocity[2] = ent->velocity[2] * 0.8f;
			}

			if (!pm.waterlevel)
			{
				if ((client->resp.class_type == CLASS_HATCHLING || client->resp.class_type == CLASS_KAMIKAZE))	// Brian: water attack handled below
					JumpSlash(ent, 80);
				// mech electrify
				else if (client->resp.class_type == CLASS_MECH)
				{
					T_Damage (ent, ent, ent, ent->velocity, ent->s.origin, vec3_origin, 5, 1, DAMAGE_IGNORE_RESISTANCES, MOD_MECH);
					ent->touch = player_jump_touch;
					ent->fly_sound_debounce_time = level.framenum + 2;
				}
			}
		}

		// play jump sound except for certain classes
		if (!client->resp.flying && (client->resp.class_type != CLASS_COMMANDO && client->resp.class_type != CLASS_GUARDIAN &&
			!((ent->client->resp.class_type == CLASS_HATCHLING) && (ent->client->resp.upgrades & UPGRADE_TRANS_MEMBRANE)))) { //FV7 missing braces? possible breaking change
		/*Savvy edit - move ATTN values into balance.h so teams could have specific fades etc*/
			if (client->resp.team == TEAM_HUMAN) {
      #ifdef CACHE_CLIENTSOUNDS
		  gi.sound(ent, CHAN_AUTO, SoundIndex(classlist[ent->client->resp.class_type].sounds[SOUND_JUMP]), 1, HUMAN_JUMP_ATTN, 0);
      #else
		  gi.sound(ent, CHAN_AUTO, gi.soundindex("*jump1.wav"), 1, HUMAN_JUMP_ATTN, 0);
      #endif
      }
      else if (client->resp.team == TEAM_ALIEN) {
      #ifdef CACHE_CLIENTSOUNDS
      /*Savvy edit hatchling is reduced sound*/
        if (ent->client->resp.class_type == CLASS_HATCHLING) {
		    gi.sound(ent, CHAN_AUTO, SoundIndex(classlist[ent->client->resp.class_type].sounds[SOUND_JUMP]), 0.5, ALIEN_JUMP_ATTN, 0);
        } else {
		    gi.sound(ent, CHAN_AUTO, SoundIndex(classlist[ent->client->resp.class_type].sounds[SOUND_JUMP]), 1, ALIEN_JUMP_ATTN, 0);
        }
      #else
      /*Savvy edit hatchling is reduced sound*/
        if (ent->client->resp.class_type == CLASS_HATCHLING) {
		    gi.sound(ent, CHAN_AUTO, gi.soundindex("*jump1.wav"), 0.5, ALIEN_JUMP_ATTN, 0);
		    } else {
		    gi.sound(ent, CHAN_AUTO, gi.soundindex("*jump1.wav"), 1, ALIEN_JUMP_ATTN, 0);
		    }
      #endif
      }
		} //FV7 missing braces?
		// exterm and wraith can only activate fly after small delay
		//FIXME: flyer drops like a stone if he ends flying because of the delay :)
		if(client->resp.class_type == CLASS_EXTERM || client->resp.class_type == CLASS_WRAITH)
			ent->delay = level.time + 0.2f;
	}

	if (client->jumpattack)
	{
		if (ent->touch_debounce_time < level.time)
		{
			if(!pm.groundentity || ent->client->resp.class_type == CLASS_HATCHLING
				|| ent->client->resp.class_type == CLASS_KAMIKAZE || ent->client->resp.class_type == CLASS_STALKER
				|| ent->client->lastspeeds[4] > 400)
			{
				// damage on touch
				ent->touch = player_jump_touch;
			}
			else if(pm.groundentity->client && ent->client->resp.class_type != CLASS_MECH && (ent->client->resp.class_type == CLASS_DRONE || (pm.groundentity && !wasonground)))// || ent->client->resp.class_type == CLASS_MECH || ent->client->resp.class_type == CLASS_GUARDIAN))
			{
				// damage on touch
				ent->touch = player_jump_touch;
			}
			else
			{
				if (ent->client->resp.class_type == CLASS_MECH) {
					if (ent->fly_sound_debounce_time < level.framenum)
						ent->touch = NULL;
				} else {
					ent->touch = NULL;
				}
			}
		} else {
			ent->touch = NULL;
		}
	} else {
		ent->touch = NULL;
	}

	// exterm jumpjet works while pressing up
	if(!pm.waterlevel && client->resp.class_type == CLASS_EXTERM)
		if (pm.cmd.upmove >= 10 && ent->delay < level.time && ent->client->last_move_time < level.time && !(ent->flags & FL_POWER_ARMOR))
			client->resp.flying=1;

	// FIXME: wraith fly in clientthink looks too hacky
	if (client->resp.class_type == CLASS_WRAITH) {
		//gi.dprintf ("upmove = %d\n", pm.cmd.upmove);
		if (client->resp.flying){
			vec3_t	angles, forward, side;

			VectorAdd (client->v_angle, client->kick_angles, angles);
			AngleVectors (angles, forward, side, NULL);
			VectorScale(ent->velocity, 0.95f, ent->velocity);
			// these used FRAMETIME, this is same but same type as ucmd->*move => faster
			VectorMA(ent->velocity, pm.cmd.forwardmove / 10, forward, ent->velocity);
			VectorMA(ent->velocity, pm.cmd.sidemove / 10, side, ent->velocity);
			//VectorMA(ent->velocity, pm.cmd.upmove / 10, up, ent->velocity); // new type of movement
			ent->velocity[2] += pm.cmd.upmove / 10;

			if (VectorLength(ent->velocity) > 400) {
				VectorNormalize(ent->velocity);
				VectorScale(ent->velocity, 400, ent->velocity);
			}

			// check for fly stop conditions
			if (pm.cmd.upmove < 0)
				client->resp.flying = 0;
			/*else if (pm.groundentity || pm.waterlevel > 1) // autolanding
				client->resp.flying = 0;*/
		}
		else if (pm.waterlevel < 2 && pm.cmd.upmove >= 10 && ent->health > WRAITH_MIN_FLY_HEALTH && ent->delay < level.time )
			client->resp.flying=1;
	}

	ent->groundentity = pm.groundentity;
	ent->watertype = pm.watertype;
	ent->waterlevel = pm.waterlevel;

	if (pm.groundentity)
		ent->groundentity_linkcount = pm.groundentity->linkcount;

	// copy viewangles
	// r1: must always do this for observers! otherwise admin commands, ctf ident, etc
	// won't trace with the correct view angles!!

	if (!ent->deadflag || ent->client->resp.class_type == CLASS_OBSERVER) {
		VectorCopy (pm.viewangles, client->v_angle); // *this* is going to be the final ent angle

		if (ent->client->resp.turret)
			VectorCopy (ent->client->resp.turret->s.angles, client->ps.viewangles);
		else
			VectorCopy (pm.viewangles, client->ps.viewangles);
	}

	gi.linkentity (ent);

	if (ent->movetype != MOVETYPE_NOCLIP)
		G_TouchTriggers (ent);

	// touch other objects
	for (i=0 ; i<pm.numtouch ; i++)
	{
		other = pm.touchents[i];

		for (j=0 ; j<i ; j++)
			if (pm.touchents[j] == other)
				break;
		if (j != i)
			continue;	// duplicated

		if(ent->touch)
			ent->touch (ent, other, NULL, NULL);

		if (!other->touch)
			continue;

		if (other->client)
			gi.dprintf ("warning, %s touched a %s\n", classlist[ent->client->resp.class_type].classname, classlist[other->client->resp.class_type].classname);

		other->touch (other, ent, NULL, NULL);
	}

	//nuke touch so we don't dupe it for other players
	ent->touch = NULL;

	//r1: old buttons etc was here

	// save light level the player is standing on for monster sighting AI
	//client->lightlevel = ucmd->lightlevel;

	// fire weapon from final position if needed
	if (!client->weapon_thunk && (client->latched_buttons & (BUTTON_ATTACK|BUTTON_USE)) && (ent->health > 0 || ent->client->resp.class_type == CLASS_OBSERVER))
	{
		client->weapon_thunk = true;
		Think_Weapon (ent);
	}

	switch (ucmd->impulse)
	{
		case 69:
			Cmd_Grenade_f (ent);
			break;
		case 70:
			Cmd_Flashlight_f (ent);
			break;
		case 71:
			Use_Health (ent, &itemlist[ITEM_HEALTH]);
			break;
	}
}


/*
ClientBeginServerFrame

This will be called once for each server frame, before running
any other entities in the world.
*/
void biotech_heal (edict_t *self, float dist);
void ClientBeginServerFrame (edict_t *ent) {
gclient_t	*client;
//	static char testmsg[6] = {46,105,110,102,111,0};

client = ent->client;
client->talk_rate *= 0.94;

	if (level.intermissiontime && (level.intermissiontime < level.framenum)) { return; }

	//reduce TK counter
	if (level.framenum % 600 == 0) { client->resp.teamkills *= 0.9f; }

	// check idle timer
	if (!(ent->client->pers.adminpermissions & (1 << PERMISSION_BENEFITS)) && ((client->pers.idletime++ >= 10*idletime->value && (client->resp.team || (team_info.numplayers[0]+team_info.numplayers[1]+team_info.numplayers[2]) > (int)(game.maxclients * 0.92))) || (client->pers.idletime >= 60000))) {
		if (!(!dedicated->value && ent->s.number == 1)) {
			gi.bprintf (PRINT_HIGH, "Kicking %s for prolonged inactivity.\n", client->pers.netname);
			gi.cprintf (ent, PRINT_HIGH, "You have been kicked due to inactivity.\n");
			kick (ent);
			return;
		}
		client->pers.idletime = 0;
	} else if (client->latched_buttons & BUTTON_ANY) // latched prevents players putting heavy objects to keyb
		client->pers.idletime = 0;

	// update chasecammers
	if (ent->client->resp.chased) {
		edict_t *other;
		ent->client->resp.chased = false;
		for (other = g_edicts + 1; other->client; other++) {
			if (other->client->chase_target == ent)
				UpdateChaseCam(other);
		}
	}

	if (client->resp.team) {

		//reset how much damage they've taken from previous frames
		//client->damage_count = 0;

		if (client->resp.team == TEAM_HUMAN && ent->health > ent->max_health && level.framenum % 10 == 0)
			ent->health--;

		if (ent->client->healthinc && ent->health > 0 && (int)level.framenum >= ent->client->healthinc_wait) { //FV7 typecast unsigned int to int on framenum in case healthinc can be negative
			if (ent->health == ent->max_health) {
				ent->client->healthinc = 0;
			} else {
				ent->health += 2;
				ent->client->healthinc -= 2;
				if (ent->health > ent->max_health) { ent->health = ent->max_health; }
			}
		}

		if ((client->resp.class_type==CLASS_BREEDER || client->resp.class_type==CLASS_ENGINEER) && ent->health > 0) {
			if (level.framenum % 150 == 0){
			client->resp.total_score++;
			}

			if (client->resp.score < 3) {
			client->resp.builder_time += 0.1;
				if (client->resp.builder_time >= 300) {
				client->resp.score++;
					if (client->resp.score > MAX_SCORE->value) {
					client->resp.score = MAX_SCORE->value;
				  }
				client->resp.builder_time = 0;
				}
			}
		}

		if (level.framenum % 4200 == 0) {
		stuffcmd(ent,va("alias ghost echo Your ghost code is: %d\nghost\n",ent->client->pers.ghostcode));
		}
		client->spikes *= 0.95;

		if (level.suddendeath && !level.intermissiontime) {
			if (ent->health > 0){
				if ((int)level.framenum + 300 == ent->client->sudden_death_frame) { //FV7 typecast
				gi.cprintf (ent, PRINT_HIGH, "[sd] You will die in %s\n", colortext("30 seconds"));
				}
				else if ((int)level.framenum + 600 == ent->client->sudden_death_frame) { //FV7 typecast
				gi.cprintf (ent, PRINT_HIGH, "[sd] You will die in %s - kill enemy players to increase your timer (top right)\n", colortext("60 seconds"));
				}
				else if ((int)level.framenum + 100 == ent->client->sudden_death_frame) { //FV7 typecast
				gi.cprintf (ent, PRINT_HIGH, "[sd] You will die in %s\n", colortext("10 seconds"));
				}
			}

			if ((int)level.framenum == ent->client->sudden_death_frame) { T_Damage (ent, world, world, vec3_origin, vec3_origin, vec3_origin, 100000, 0, DAMAGE_NO_PROTECTION, MOD_SUDDENDEATH); }
		}

		if (client->slow_timer > 0)
		{
			client->slow_timer -= 0.1;
			if (client->slow_timer <= 0)
			{
				SetClientVelocity (ent, DEFAULT_VELOCITY);
				stuffcmd (ent, va("set cl_upspeed %d\n", DEFAULT_VELOCITY));
				ent->client->slow_threshold = 1;
			}
			else if (client->slow_last_jump_framenum == level.framenum)
				stuffcmd (ent, va("set cl_upspeed %d\n", DEFAULT_VELOCITY));
		}

		if (client->frozentime > 0) {
			client->frozentime -= 0.1;
			if (client->frozentime <= 0) {
				client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
				client->ps.pmove.pm_flags &= ~PMF_TIME_TELEPORT;
				client->frozentime = 0;
				client->ps.pmove.delta_angles[PITCH] = ANGLE2SHORT(0 - client->resp.cmd_angles[PITCH]);
				client->ps.pmove.delta_angles[ROLL] = ANGLE2SHORT(0 - client->resp.cmd_angles[ROLL]);
			} else {
				if (client->frozenmode == PMF_TIME_TELEPORT && !client->resp.turret && ent->groundentity) {
					client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
					client->ps.pmove.pm_time = 15;
				}
			}
		}
	}

	/*if (client->resp.class_type == CLASS_MECH)
	{
		float len;
		len = VectorLength (ent->velocity);
		gi.bprintf (PRINT_HIGH, "velocity = %.1f\n", len);
	}*/

	/* Cheat checking trigger routines
	   keep the framenumbers which generate userinfo stuffs away from 0 to prevent
	   overflows with q2admin/bwadmin stuffs happening the same time

	   not needed any more since q2a/bwa don't work with gloom....
	*/

	if (deathmatch->value) {
		int playerframe = level.framenum - client->resp.enterframe;
		switch (playerframe) {
			case 10:
				// first stage of cheat checks
				if (ent->client->pers.check_stage < 1) {
				ent->client->pers.check_stage = 1;
				ent->client->pers.stuff_request |= 2;
				}
				break;
			case 20:
				if (*reconnect_address->string && ent->client->pers.stuff_request & 8) {
				gi.dprintf ("WARNING: client %s[%s] didn't respond to reconnect cvar. Sending them off again...\n", ent->client->pers.ip, ent->client->pers.netname);
				stuffcmd (ent, va("connect \"%s\"\n", reconnect_address->string));
				return;
				}
				break;
			case 30:
				if (*reconnect_address->string && ent->client->pers.stuff_request & 8) {
				gi.dprintf ("WARNING: client %s[%s] still connected after sending reconnect.\n", ent->client->pers.ip, ent->client->pers.netname);
				gi.AddCommandString (va("kick %d\n", ent-g_edicts-1));
				return;
				}
				// second stage of cheat checks
				if (ent->client->pers.check_stage < 2) {
				ent->client->pers.check_stage = 2;
				ent->client->pers.stuff_request |= 2;
				}
				break;
			case 50:
				// third stage, vid_restart forced on connect
				if (ent->client->pers.check_stage < 3) {
				ent->client->pers.stuff_request |= 1;
				ent->client->pers.check_stage = 3;
				}
				break;
				//to find out when their vid_restart ends so we can continue
				//with some of the less intensive checks
				//they should be pretty much "locked up" during the vid_restart
				//so this command will only get sent once the restart is complete.
				//in theory...
			case 52:
				stuffcmd (ent, "cmd vid_done\n");
				break;
		}

		//we can't use a switch any more because the case expression isn't constant
		//so we have to use if..
		if (ent->client->pers.check_stage < 5 && ent->client->pers.vid_done) {
			if (playerframe == ent->client->pers.vid_frame + 5) {
			// safety feature for possible fov/userinfo filters
			// r1: added xania protection (removed now)
				if (!ent->client->pers.connected) {
				//gi.dprintf ("%s kick: Possibly filtering cheat caps or very lagged!\n", ent->client->pers.netname);
				SendToAdmins ("%s: !connected\n", ent->client->pers.netname);
				kick (ent);
				//r1: considering we just nuked them, do we want the rest of this code below
				//    to execute? I think we need a return;
				return;
				}
        else {
					if (ent->client->resp.visible) {
					gi.bprintf (PRINT_HIGH, "%s entered the game.\n", ent->client->pers.netname);
					}
				}
			}
      else if (playerframe == ent->client->pers.vid_frame + 10) {
			stuffcmd (ent,"cmd ghost $sgcode\n");
			ent->client->pers.check_stage = 5;
			}
		}
		
		//FV7 removed a commented block of code that was getting picked up by gcc

		// do a vid_restart if requested, only after cheat prevention setup
		if (ent->client->pers.check_stage >= 3 && (ent->client->pers.stuff_request & 1)) {
			ent->client->pers.stuff_request &= ~1;
			if (secure_modulate_cap->value)
				stuffcmd (ent, "vid_restart\n");
		}

		// send a fov if requested
		if (ent->client->pers.stuff_request & 2) {
			int i;
			ent->client->pers.stuff_request &= ~2;
			i = random()*180;
			if (i == atoi(Info_ValueForKey(ent->client->pers.userinfo, "fov")))
				i++;

			stuffcmd (ent, va("fov %d\n", i));
		}
	}

	//yuck
	ent->gravity = 0;

	if (client->fogged_alpha >= 5.0f) { client->fogged_alpha -= 5.0f; }
	else { client->fogged_alpha -= 0.1f; }
	
	if (client->fogged_alpha < 0.0f) { client->fogged_alpha = 0.0f; }
	if (client->fogged_alpha > 1.0f && client->fogged_alpha < 5.0f) { client->fogged_alpha = 0.0f; }
	// ENDGLOOM

	if (client->blind_time > 0)
	{
		client->blind_time -= 0.1f;
		client->blinded_alpha = client->blind_time_alpha;
		client->blind_time_alpha -= (float)((client->blind_time_alpha / client->blind_time)/10);
	}

	// menu popup here, menu doesn't belong to think_weapon
	if (ent->health < 1 && ent->client->latched_buttons & BUTTON_ATTACK) {
		if (!ent->client->menu.entries && !ent->client->trace_object && !ent->client->resp.adminmode) {
			OpenMenu (ent);
		} else if (ent->client->menu.entries) {
			PMenu_Close(ent);
		}
	}

	// special +use button handling
	if (ent->health > 0 && ent->client->latched_buttons & BUTTON_USE)
	{
		if (ent->client->resp.class_type == CLASS_BREEDER) {
			if (ent->client->menu.entries)
				PMenu_Close(ent);
			else
				PMenu_Open(ent, breeder_menu, 2);

		} else if (ent->client->resp.class_type == CLASS_ENGINEER) {
			if (ent->client->menu.entries)
				PMenu_Close(ent);
			else if (ent->client->resp.turret)
				ent->client->resp.turret = NULL;
			else
				PMenu_Open(ent, engineer_menu, 2);
		}
	}

	// run weapon animations if it hasn't been done by a ucmd_t

	// FIXME: do we really want to do this? the only time this would happen is if the client is
	// lagged out... no one likes a constant firing ht :)
	if ((ent->health > 0 || client->resp.class_type == CLASS_OBSERVER) && !client->weapon_thunk) { Think_Weapon (ent); }
	else { client->weapon_thunk = false; }

	//running anim
	if (ent->deadflag == DEAD_DYING) {
		// clients dont do normal think handling, but we need it for the wraith death animations
		if (ent->think) {
			if (level.time >= ent->nextthink) {
			ent->think (ent);
			}
		}

		//FIXME: move engi (and breeder) death into copytobodyque
		if (ent->s.modelindex == 255 && (ent->groundentity || level.framenum - ent->client->damage_last_frame > 50) && ent->s.frame == client->anim_end && !ent->think) {
			if (client->resp.class_type != CLASS_ENGINEER)
				CopyToBodyQue(ent);
			// after this client ent is unlinked
			ent->deadflag = DEAD_DEAD;
		}
	}

	// grapple pulling
	if (client->ctf_grapple)
		CTFGrapplePull(client->ctf_grapple);

	if (ent->health > 0 && ent->client->resp.team == TEAM_HUMAN) {

		//exterm fps independant jetjump
		if(client->resp.class_type == CLASS_EXTERM){

			if(client->resp.inventory[client->ammo_index] < ent->client->resp.shot_type && ent->client->weaponstate == WEAPON_READY && ent->client->last_move_time - level.time <= 5) {
				client->resp.inventory[client->ammo_index]++;		// recharge
			}

			if(client->resp.inventory[client->ammo_index] == 1)
				gi.sound(ent, CHAN_AUTO, SoundIndex (weapons_up), 1, ATTN_NORM, 0);
			else if(client->resp.inventory[client->ammo_index] == ent->client->resp.shot_type-1)
				gi.sound(ent, CHAN_AUTO, SoundIndex (weapons_up_end), 1, ATTN_NORM, 0);

			if(client->resp.inventory[client->ammo_index] <= 0)
			{
				if(ent->client->last_move_time < level.time){
					gi.cprintf(ent, PRINT_HIGH, "Power cells overheated!\n");
					gi.sound(ent, CHAN_AUTO, SoundIndex (weapons_pulseout), 1, ATTN_NORM, 0);
					ent->client->last_move_time = level.time + 10;
					if (ent->client->resp.upgrades & UPGRADE_COOLANT)
						ent->client->last_move_time -= 5;
					if (ent->flags & FL_POWER_ARMOR) {
						gi.sound(ent, CHAN_AUTO, SoundIndex (misc_power2), 1, ATTN_IDLE, 0);
						ent->client->last_move_time += 5;
					}
					ent->flags &= ~FL_POWER_ARMOR;
					client->resp.flying = false;
				}
			}

			if (client->resp.flying)
			{
				int jetcost;

				if (!(client->resp.upgrades & UPGRADE_CHARON_BOOSTER))
				{
					if (ent->flags & FL_POWER_ARMOR)
						jetcost = 15;
					else
						jetcost = 10;
				}
				else
				{
					jetcost = 6;
				}

				client->resp.inventory[client->ammo_index] -= jetcost;

				if (client->resp.inventory[client->ammo_index] < 1) {
					client->resp.inventory[client->ammo_index] = 1;
				} else {
					ent->velocity[2] += sv_gravity->value*FRAMETIME*2; // 160;
					if (client->resp.inventory[client->ammo_index] & 1)
						gi.sound(ent, CHAN_BODY, SoundIndex (weapons_flame), 1, ATTN_IDLE, 0);
				}
//				client->resp.flying = false;
			}

		}

		// acid support
		if (ent->client->acid_duration)
		{
			ent->client->acid_duration--;
			if (!ent->client->acid_duration)
			{
				ent->client->acid_damage = 0;
				ent->client->acid_attacker = NULL;
			}
			else
			{
				if (level.framenum % 5 == 0)
				{
					//gi.dprintf("acid dmg %d, lastc %d, delay %d\n", ent->client->acid_damage, ent->client->acid_duration, ent->client->acid_delay);
					T_Damage(ent,ent->client->acid_attacker,ent->client->acid_attacker, vec3_origin, ent->s.origin,
						vec3_origin, ent->client->acid_damage, 0, DAMAGE_NO_ARMOR, MOD_POISON);

					

					//r1: test feature, longer you are poisoned the more it hurts.
					//ent->client->acid_damage += (int)((float)ent->client->acid_delay / 10.0f);

					if (ent->health <= 0)
					{
						ent->client->acid_damage = ent->client->acid_duration = 0;
						ent->client->acid_attacker = NULL;
					}
				}
			}
		}

		//FIXED: acid could kill player via t_damage: recheck health !! (bio can become ghost
		//       if healing after being killed by acid)

		if (ent->health > 0) {
			// reload weapon (except if bio- reserved for heal)
			if (ent->client->weapon && ent->client->latched_buttons & BUTTON_USE && ent->client->resp.class_type != CLASS_BIO)
				Auto_Reload(ent);

			//bio heal self and cell recharge
			if (ent->client->resp.class_type == CLASS_BIO)
			{
				//gitem_t *item;
				//static int cells = ITEM_INDEX(FindItem("Cells"));
				if ((ent->health<ent->max_health)&&(level.time>ent->client->last_reload_time)){
					ent->health+=2;
					ent->client->last_reload_time = level.time+.5f;
					if (ent->health>ent->max_health)
						ent->health=ent->max_health;
				} else if (ent->client->resp.inventory[ITEM_AMMO_CELLS] < BIOTECH_MAX_CELLS) {
					if (!(client->buttons & BUTTON_USE)) {
						if (level.time > ent->client->last_reload_time) {
							ent->client->resp.inventory[ITEM_AMMO_CELLS]+=2;
							if (ent->client->resp.inventory[ITEM_AMMO_CELLS] > BIOTECH_MAX_CELLS)
								ent->client->resp.inventory[ITEM_AMMO_CELLS] = BIOTECH_MAX_CELLS;
							ent->client->last_reload_time = level.time +.4f;
						}
					}
				}
			}

			//bleed
			if (ent->health<(ent->max_health/2) && ent->client->blood_time < level.time)
			{
				vec3_t origin,normal;
				int i;
				VectorCopy(ent->s.origin,origin);
				origin[2]+=(ent->viewheight/2);
				for (i=0;i<3;i++)
					normal[i]=randomMT();
				if ((client->resp.class_type==CLASS_ENGINEER)||(client->resp.class_type==CLASS_MECH))
					SpawnDamage(TE_SPARKS,origin,normal,0);
				else
					SpawnDamage(TE_BLOOD,origin,normal,0);
				ent->client->blood_time=level.time + random()*5;
			}
		}
	} else if (ent->health > 0 && client->resp.team == TEAM_ALIEN) {

		//wraith heal/flying
		if (client->resp.class_type == CLASS_WRAITH) {
			if (ent->health < ent->max_health && (level.framenum % 10) == 0 && ent->pain_debounce_time + 2.5 < level.time) {
				ent->health += 2;
				if (ent->health > ent->max_health)
					ent->health = ent->max_health;
			}

			if (ent->health <= 60)
				ent->client->resp.flying = 0;
		}

		//water attack
		if(ent->waterlevel > 0 && (ent->client->resp.class_type == CLASS_KAMIKAZE || ent->client->resp.class_type == CLASS_HATCHLING))
			JumpSlash(ent, 66);
	} else {
		// spec IR toggle
		if (ent->delay < level.time && ((ent->client->latched_buttons) & BUTTON_USE) ) {
			ent->client->ps.rdflags ^= RDF_IRGOGGLES;
			ent->delay = level.time + .2f;
		}
	}

	if ((ent->client->pers.adminpermissions & (1 << PERMISSION_DEBUG)) && ent->client->trace_object && !((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) && !client->resp.adminmode) {
		trace_t tr;
		vec3_t start, forward;

		VectorCopy (ent->s.origin,start);
		start[2] += ent->viewheight;

		AngleVectors(ent->client->v_angle, forward, NULL, NULL);
		VectorScale(forward, 1024, forward);
		VectorAdd(ent->s.origin, forward, forward);

		tr = gi.trace (start, ent->client->trace_object->mins, ent->client->trace_object->maxs, forward, ent->client->trace_object, MASK_SHOT);
		VectorCopy (tr.endpos, ent->client->trace_object->s.origin);
		gi.linkentity (ent->client->trace_object);
	}

	client->latched_buttons = 0;
}

