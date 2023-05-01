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

// Laser Tripwire code for Gloom
// 10/21/98 Cyberdog - Initial code

#include "g_local.h"

//#define CONTENTS_NOSPAWNS	(CONTENTS_CURRENT_UP|CONTENTS_CURRENT_DOWN)
//#define CONTENTS_NOBUILD	(CONTENTS_NOSPAWNS|CONTENTS_MIST)
void tripwire_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int m=MOD_MINE;
	edict_t *sent = NULL;

	if(self->deadflag == DEAD_DEAD)
		return;

	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_NO;

	if (attacker && attacker->client) {
		log_killstructure (attacker, self);
		if (attacker->client->resp.team == TEAM_HUMAN) {
			attacker->client->resp.teamkills ++;
			CheckTKs (attacker);
		}
	}

	while ((sent = findradius(sent, self->s.origin, 40)) != NULL)
	{
		if (sent != self && sent->enttype == ENT_TRIPWIRE_BOMB) {
			if (attacker==self) {
				T_Damage (sent,sent,sent,vec3_origin,sent->s.origin,vec3_origin,100,10,0,MOD_UNKNOWN);
			} else {
				T_Damage (sent,self,self,vec3_origin,sent->s.origin,vec3_origin,100,10,0,MOD_UNKNOWN);
			}
		}
	}

	// the friendly fire flag tells radiusdamage to just push, not hurt
	T_RadiusDamage(self, self, attacker==self ? self->radius_dmg / 2 : self->radius_dmg, self, self->dmg_radius, m, attacker==self ? DAMAGE_FRIENDLY_FIRE : 0);

	if (!(self->spawnflags & 16384))
	{
		if (attacker && attacker->client && attacker->client->resp.team == TEAM_ALIEN)
			team_info.buildpool[TEAM_HUMAN] += COST_MINE;
		else
			team_info.points[TEAM_HUMAN] -= COST_MINE;
	}

	gi.WriteByte (svc_temp_entity);
	if (self->waterlevel)
		gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	else
		gi.WriteByte (TE_ROCKET_EXPLOSION);
	gi.WritePosition (self->s.origin);
	gi.multicast (self->s.origin, MULTICAST_PVS);

	if (self->owner)
		G_FreeEdict(self->owner);

	G_FreeEdict(self);
}

void tripwire_think (edict_t *self) {
vec3_t	start;
vec3_t	end;
trace_t	tr;
//edict_t	*hit;
edict_t	*ignore;
int immune; /*Savvy needed for advanced mine stuff i added*/
int num;
num = immune = 0;

ignore = self;
VectorCopy (self->s.origin, start);
VectorMA (start, 2048, self->movedir, end);

tr = gi.trace (start, NULL, NULL, end, ignore, MASK_SOLID);

	if (tr.startsolid) {
	VectorCopy (start, self->s.old_origin);
	self->nextthink = level.time + FRAMETIME;
	return;
	}

VectorCopy(tr.endpos,end);

	for (;;) {
	num++;
	tr = gi.trace (start, NULL, NULL, end, ignore, CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_DEADMONSTER);

		if (!tr.ent) { break; }

		if (tr.ent->client) {
		/*Savvy edited to have a switch statement instead, so multiple aliens could be immune*/
		  switch (tr.ent->client->resp.class_type) {
		    case CLASS_GUARDIAN:
		    case CLASS_HATCHLING:
		        if (tr.ent->client->resp.class_type == CLASS_HATCHLING) {
		          if ((tr.ent->client->resp.upgrades & UPGRADE_HATCHLING_ENHANCEMENT) || (tr.ent->client->resp.upgrades & UPGRADE_TRANS_MEMBRANE)) {
              immune = 1;
              break; //stop from setting to 0
              }
            immune = 0;
            break; //stop from setting to immune 1
            }
          immune = 1;
  		    break;
//  		  default:
//  		    immune = 0;
		  }
/*Savvy added in a different statement, for advanced tripwire triggering
			if(tr.ent->client->resp.team != TEAM_HUMAN && tr.ent->client->resp.class_type != CLASS_GUARDIAN)
*/
			if (tr.ent->client->resp.team != TEAM_HUMAN && !immune) {
			// hurt it if we can
				if (tr.ent->takedamage && !(tr.ent->flags & FL_IMMUNE_LASER)) {
					//wtf? why did this NULL pointer on self->owner->die due to self->owner being freed...
					if (self->owner->inuse) { self->owner->die(self->owner, NULL, NULL, 0, vec3_origin); }
					else { G_FreeEdict (self); }
				return;
				}
			}
		}

		// if we hit something that's not a monster or player or is immune to lasers, we're done
		if (!(tr.ent->svflags & SVF_MONSTER) && (!tr.ent->client)) { break; }

	ignore = tr.ent;
	VectorCopy (tr.endpos, start);
		if (num > 20) { break; }
		//tr = gi.trace (start, NULL, NULL, end, ignore, CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_DEADMONSTER);
	/*Savvy reset immune state*/
	immune = 0;
	}

	//VectorCopy (self->mins,start);
	/*gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_DEBUGTRAIL);
	gi.WritePosition (start);
	gi.WritePosition (end);
	gi.multicast (self->s.origin, MULTICAST_PVS);

	num = gi.BoxEdicts (start, end, touch, MAX_EDICTS, AREA_SOLID);

	// be careful, it is possible to have an entity in this
	// list removed before we get to it (killtriggered)
	for (i=0 ; i<num ; i++)
	{
		hit = touch[i];
		if (!hit->inuse)
			continue;
		if (!hit->client)
			continue;
		if (hit->deadflag != DEAD_DEAD && hit->health > 0) {
			self->die(self, NULL, NULL, 0, vec3_origin);
			break;
		}
	}*/
VectorCopy (tr.endpos, self->s.old_origin);
self->nextthink = level.time + FRAMETIME;
}

void target_laser_think (edict_t *self);
void tripwire_activate (edict_t *mine)
{
	edict_t *tripwire;
	int lasercolor;

	tripwire = G_Spawn();

	VectorCopy(mine->s.origin, tripwire->s.origin);
	VectorCopy(mine->s.angles, tripwire->s.angles);	

	mine->movetype = MOVETYPE_NONE;
	//mine->solid = SOLID_BBOX;//SOLID_TRIGGER;

	tripwire->s.renderfx |= RF_BEAM|RF_TRANSLUCENT;
	tripwire->s.modelindex = 1;			// must be non-zero

	/*mine = G_Spawn();	// Keep bomb portion
	mine->classname = "bomb";
	mine->s.modelindex = gi.modelindex("models/objects/tripwire/tris.md2");	
	VectorCopy(tripwire->s.origin, mine->s.origin);
	VectorCopy(tripwire->s.angles, mine->s.angles);			

	gi.linkentity (mine);*/

	mine->nextthink = 0;

	tripwire->owner = mine;
	mine->owner = tripwire;

	// set the beam diameter
	tripwire->s.frame = 4;

	if (!mine->count)
		lasercolor = laser_color->value;
	else
		lasercolor = mine->count;

	// set the color
	switch (lasercolor){
		case 0:
			tripwire->s.skinnum = 0xf2f2f0f0;	// Red
			break;
		case 1:
			tripwire->s.skinnum = 0xd0d1d2d3;	// Green
			break;
		case 2:
			tripwire->s.skinnum = 0xf3f3f1f1;	// Blue
			break;
		default:
			tripwire->s.skinnum = 0xdcdddedf;	// Yellow/Brown
	//	tripwire->s.skinnum = 0xe0e1e2e3;	// Yellow/Brown
	//	tripwire->s.skinnum = 0;			// Black-Gray
	//	tripwire->s.skinnum = 0xffffffff;	// Black-Gray
	}

	G_SetMovedir (tripwire->s.angles, tripwire->movedir);

	tripwire->dmg = 1;

	VectorSet (tripwire->mins, -8, -8, -8);
	VectorSet (tripwire->maxs, 8, 8, 8);

	tripwire->activator = tripwire;
	tripwire->svflags &= ~SVF_NOCLIENT;
	tripwire->classname = "laser_detonator";
	gi.linkentity (tripwire);

	tripwire->nextthink = level.time + 1;
	tripwire->think = tripwire_think;
	tripwire_think(tripwire);
}

void tripwire_expire (edict_t *ent)
{
	if (ent->waterlevel)
	{
		temp_point (TE_ROCKET_EXPLOSION_WATER, ent->s.origin);
	}
	else
	{
		temp_point (TE_ROCKET_EXPLOSION, ent->s.origin);
	}

	team_info.points[TEAM_HUMAN] -= COST_MINE;

	gi.multicast (ent->s.origin, MULTICAST_PVS);

	if (ent->owner)
		G_FreeEdict(ent->owner);

	G_FreeEdict(ent);
}

void tripwire_blowup (edict_t *ent)
{
	ent->health=0;

	ent->die(ent, ent, ent, 0, ent->s.origin);
}

void BuildLog (edict_t *self, char *object, qboolean created);
void CmdSetTripWire (edict_t *self)
{
	edict_t	*ignore;
	vec3_t	start;
	vec3_t	end;
	int		hax;
	trace_t	tr;
	vec3_t	angles, forward;
	
	hax = 0;

	ignore = self;
	
	VectorAdd (self->client->v_angle, self->client->kick_angles, angles);
	VectorCopy (self->s.origin, start);
	start[2] += self->viewheight;
	AngleVectors (angles, forward, NULL, NULL);

	VectorMA (start, 74, forward, end);

	for (;;)
	{
		if (++hax > 30)
			break;

		tr = gi.trace (start, NULL, NULL, end, ignore, MASK_SOLID);
		
		if (!tr.ent)
			break;

		if (tr.fraction >= 1) {
			gi.cprintf (self,PRINT_HIGH,"Too far from wall to set a tripwire!\n");
			break;
		}

		if (gi.pointcontents(tr.endpos)&CONTENTS_NOBUILD)
		{
			gi.cprintf(self, PRINT_HIGH, "Can\'t build a laser tripwire here!\n");
			break;
		}

		// if we hit something that's not a monster or player or is immune to lasers, we're done
		if (!(tr.ent->svflags & SVF_MONSTER) && (!tr.ent->client))
		{
			edict_t* bomb;
			trace_t tr2;

			//trace backwards again
			tr2 = gi.trace (tr.endpos, NULL, NULL, self->s.origin, self, MASK_SHOT);
			if (tr2.fraction != 1.0) {
				gi.cprintf(self, PRINT_HIGH, "Can't build a laser tripwire here!\n");
				return;
			}

			if (tr.ent->takedamage || (tr.ent->solid == SOLID_BSP && tr.ent != world))
			{
				if (!(tr.ent->enttype == ENT_FUNC_WALL && (tr.ent->spawnflags & 7) == 0))
				{
					gi.cprintf (self, PRINT_HIGH, "Can't attach a tripwire to this object.\n");
					break;
				}
			}

			BuildLog (self, "tripwire", true);
			
			bomb = G_Spawn();
			bomb->classname = "tripwire";
			VectorCopy(tr.endpos, bomb->s.origin);
			vectoangles(tr.plane.normal, bomb->s.angles);

			bomb->mass = 0;

			if (tripwire_repair_count->value)
			{
				bomb->think = tripwire_expire;
				bomb->nextthink = level.time + 60;
			}
			else
			{
				bomb->think = tripwire_activate;
				bomb->nextthink = level.time + 1;
			}
			bomb->s.modelindex = gi.modelindex("models/objects/tripwire/tris.md2");
			bomb->radius_dmg = 200;
			bomb->dmg_radius = 120;
			bomb->damage_absorb = 15;
			bomb->enttype = ENT_TRIPWIRE_BOMB;
			bomb->takedamage = DAMAGE_YES;
			bomb->health = bomb->max_health = 50;
			bomb->die = tripwire_die;
			bomb->target_ent = self;
			//tripwire->owner = NULL;
#ifndef DEBUG
			self->client->build_timer = level.time + 4;
			team_info.points[2] += COST_MINE;
#endif
			gi.linkentity (bomb);
			bomb->solid = SOLID_BBOX;
			break;
		}

		ignore = tr.ent;
		VectorCopy (tr.endpos, start);
	}
}

void SP_tripwire (edict_t *self)
{
	if (st.spawnteam && team_info.spawnteam && !(team_info.spawnteam & st.spawnteam)){
		G_FreeEdict(self);
		return;
	}

	if (!self->enttype && !level.framenum)
		team_info.maxpoints[TEAM_HUMAN] -= COST_MINE;

	//G_SetMovedir (self->s.angles, self->movedir);
	//self->think = tripwire_activate;
	self->classname = "tripwire";
	//self->nextthink = level.time + 1.5;
	self->s.modelindex = gi.modelindex("models/objects/tripwire/tris.md2");
	self->radius_dmg = 200;
	self->dmg_radius = 120;
	self->enttype = ENT_TRIPWIRE_BOMB;
	self->takedamage = DAMAGE_YES;
	self->health = self->max_health = 50;
	self->die = tripwire_die;
	self->count = self->spawnflags; 
	self->damage_absorb = 15;

	gi.linkentity (self);
	self->solid = SOLID_BBOX;
	tripwire_activate (self);
}

