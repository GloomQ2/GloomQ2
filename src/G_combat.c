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

void Toss_C4 (edict_t *ent, const gitem_t *item);

const double pislash180 = (M_PI / 180.0);

/*
CanDamage

Returns true if the inflictor can directly damage the target.  Used for
explosions and melee attacks.
*/
qboolean CanDamage (edict_t *targ, edict_t *inflictor)
{
	vec3_t	dest;
	trace_t	trace;

// bmodels need special checking because their origin is 0,0,0
	if (targ->movetype == MOVETYPE_PUSH)
	{
		VectorAdd (targ->absmin, targ->absmax, dest);
		VectorScale (dest, 0.5f, dest);
		trace = gi.trace (inflictor->s.origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID);
		if (trace.fraction == 1.0f)
			return true;
		if (trace.ent == targ)
			return true;
		return false;
	}

	trace = gi.trace (inflictor->s.origin, vec3_origin, vec3_origin, targ->s.origin, inflictor, MASK_SOLID);
	if (trace.fraction == 1.0)
		return true;

	VectorCopy (targ->s.origin, dest);
	dest[0] += 15.0f;
	dest[1] += 15.0f;
	trace = gi.trace (inflictor->s.origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID);
	if (trace.fraction == 1.0f)
		return true;

	VectorCopy (targ->s.origin, dest);
	dest[0] += 15.0f;
	dest[1] -= 15.0f;
	trace = gi.trace (inflictor->s.origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID);
	if (trace.fraction == 1.0)
		return true;

	VectorCopy (targ->s.origin, dest);
	dest[0] -= 15.0f;
	dest[1] += 15.0f;
	trace = gi.trace (inflictor->s.origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID);
	if (trace.fraction == 1.0f)
		return true;

	VectorCopy (targ->s.origin, dest);
	dest[0] -= 15.0f;
	dest[1] -= 15.0f;
	trace = gi.trace (inflictor->s.origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID);
	if (trace.fraction == 1.0f)
		return true;


	return false;
}


/*
Killed
*/
static void Killed (edict_t *targ, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	if (targ->health < -999)
		targ->health = -999;

	targ->enemy = attacker;

	/*if (targ->movetype == MOVETYPE_PUSH || targ->movetype == MOVETYPE_STOP || targ->movetype == MOVETYPE_NONE)
	{	// doors, triggers, etc
		targ->die (targ, inflictor, attacker, damage, point);
		return;
	}*/

	//we just "died" into something else (this stops eggs, spikers, etc from keeping their
	//enttype after dying and probably some other stuff i don't care about)
	targ->die (targ, inflictor, attacker, damage, point);
}


/*
SpawnDamage
*/
void SpawnDamage (int type, vec3_t origin, vec3_t normal, int damage)
{
	if (damage > 255)
		damage = 255;
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (type);
//	gi.WriteByte (damage);
	gi.WritePosition (origin);
	gi.WriteDir (normal);
	gi.multicast (origin, MULTICAST_PVS);
}

/*
SpawnDamage
*/
void SpawnDamage2 (int type, vec3_t origin, vec3_t normal, int count, int color)
{
	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(type);
	gi.WriteByte(count);
	gi.WritePosition(origin);
	gi.WriteDir(normal);
	gi.WriteByte(color);
	gi.multicast(origin, MULTICAST_PVS);
}

/*
T_Damage

targ		entity that is being damaged
inflictor	entity that is causing the damage
attacker	entity that caused the inflictor to damage targ
	example: targ=monster, inflictor=rocket, attacker=player

dir			direction of the attack
point		point at which the damage is being inflicted
normal		normal vector from that point
damage		amount of damage being inflicted
knockback	force to be applied against targ as a result of the damage

dflags		these flags are used to control how T_Damage works
	DAMAGE_RADIUS			damage was indirect (from a nearby explosion)
	DAMAGE_NO_ARMOR			armor does not protect from this damage
	DAMAGE_ENERGY			damage is from an energy based weapon
	DAMAGE_NO_KNOCKBACK		do not affect velocity, just view angles
	DAMAGE_BULLET			damage is from a bullet (used for ricochets)
	DAMAGE_NO_PROTECTION	kills godmode, armor, everything

static int CheckPowerArmor (edict_t *ent, vec3_t point, vec3_t normal, int damage, int dflags)
{
	gclient_t	*client;
	int			save;
	int			power_armor_type;
	int			index;
	int			damagePerCell;
	int			pa_te_type;
	int			power;
	int			power_used;

	if (!damage)
		return 0;

	if (dflags & DAMAGE_NO_ARMOR)
		return 0;

	client = ent->client;

	power_armor_type = PowerArmorType (ent);

	if (power_armor_type == POWER_ARMOR_NONE)
		return 0;

	index = ITEM_INDEX(FindItem("Cells"));
	power = client->resp.inventory[index];

	if (!power)
		return 0;

	if (power_armor_type == POWER_ARMOR_SCREEN)
	{
		vec3_t		vec;
		float		dot;
		vec3_t		forward;

		// only works if damage point is in front
		AngleVectors (ent->s.angles, forward, NULL, NULL);
		VectorSubtract (point, ent->s.origin, vec);
		VectorNormalize (vec);
		dot = DotProduct (vec, forward);
		if (dot <= 0.3)
			return 0;

		damagePerCell = 1;
		pa_te_type = TE_SCREEN_SPARKS;
		damage = damage / 3;
	}
	else
	{
		damagePerCell = 2;
		pa_te_type = TE_SCREEN_SPARKS;//TE_SHIELD_SPARKS;
		damage = (2 * damage) / 3;
	}

	save = power * damagePerCell;
	if (!save)
		return 0;
	if (save > damage)
		save = damage;

	//SpawnDamage (pa_te_type, point, normal, save);
	SpawnDamage (pa_te_type, ent->s.origin, normal, save);

	power_used = save / damagePerCell;

	ent->client->powerarmor_time = level.time + 0.2f;
	client->resp.inventory[index] -= power_used;

	return save;
}*/

int GetArmor (edict_t *ent)
{
	if (!ent->client->armor)
		return 0;

	return ent->client->resp.inventory[ITEM_INDEX(ent->client->armor)];
}

static int CheckArmor (edict_t *ent, edict_t *attacker, vec3_t point, vec3_t normal, int damage, int te_sparks, int dflags, int mod, vec3_t dir)
{
	gclient_t	*client;
	int			save;
	int			remsave = 0;
	int			index;
	const gitem_t		*armor;

	if (!damage)
		return 0;

	if (dflags & DAMAGE_NO_ARMOR)
		return 0;

	client = ent->client;

	if (!client->armor)
		return 0;

	index = ITEM_INDEX(client->armor);

	/*if (dflags & DAMAGE_FRIENDLY_FIRE) {
		damage -= ent->damage_absorb;
		damage *= 0.25;
		if (damage <= 0)
			return 0;
	}*/

	save = damage;

	// limit armor taken damage to armor health
	if (save >= client->resp.inventory[index])
		save = client->resp.inventory[index];

	// if no armor, just deal the damage
	if (!save)
		return 0;

	remsave = save;

	// mech armor absorbs more
	if (client->armor->tag == ARMOR_FULL) {
		if (!attacker->client)
			remsave *= 0.35f;
		else
			remsave *= 0.6f;
	}

	// combat armor halves damage done
	//if (index == combat_armor_index && remsave > 2)
	if (client->armor->tag == ARMOR_COMBAT && remsave > 2)
		remsave /= 2;

	if (remsave < 1)
		remsave = 1;

	if (dflags & DAMAGE_FRIENDLY_FIRE && attacker->client && classlist[client->resp.class_type].frags_needed > 0) {
		if (dflags & DAMAGE_RADIUS && teamreflectradiusdamage->value) {
			if (teamreflectradiusdamage->value == 2) {
				//full reflect
				T_Damage (attacker, attacker, attacker, dir, point, normal, remsave, 0, DAMAGE_NO_KNOCKBACK, mod);
			} else if (teamreflectradiusdamage->value == 1) {
				//half reflect
				T_Damage (attacker, attacker, attacker, dir, point, normal, (int)(remsave * 0.5), 0, DAMAGE_NO_KNOCKBACK, mod);
			}
		}

		if (teamreflectarmordamage->value) {
			if (teamreflectarmordamage->value == 1)
				T_Damage (attacker, attacker, attacker, point, point, normal, (int)(remsave * 0.5), 0, DAMAGE_NO_KNOCKBACK, mod);
			else if (teamreflectarmordamage->value == 2)
				T_Damage (attacker, attacker, attacker, point, point, normal, remsave, 0, DAMAGE_NO_KNOCKBACK, mod);
		}
	}

	if (!(dflags & DAMAGE_FRIENDLY_FIRE) || dflags & DAMAGE_RADIUS || (teamarmordamage->value)) {
		client->resp.inventory[index] -= remsave;
		SpawnDamage (te_sparks, point, normal, save);
	}

	if(dflags & DAMAGE_PIERCING)
	{
		armor = &itemlist[index];

		return ceil(((gitem_armor_t *)armor->info)->normal_protection * save);		// piercing causes damage to health
	}
	else
		return save;		// all damage taken by armor
}


qboolean CheckTeamDamage (edict_t *targ, edict_t *attacker)
{
		//FIXME make the next line real and uncomment this block
		// if ((ability to damage a teammate == OFF) && (targ's team == attacker's team))
	if((!targ || !attacker) || (!targ->client && !attacker->client))
		return false;

	if (teamdamage->value == 2)
		return false;

	if (teamdamage->value == 1 && classlist[attacker->client->resp.class_type].frags_needed > 0)
		return false;

	if(targ->client->resp.team == attacker->client->resp.team)
		return true;

	return false;
}

#define HURTCLASS_SPARKS \
	if ((!(targ->flags & FL_NO_HURTSPARKS))) { \
		if (targ->hurtflags & WC_NO_SPARKS) \
			SpawnDamage (te_sparks, point, normal, take); \
		else \
			SpawnDamage2 (TE_LASER_SPARKS, point, normal, 5, 2); \
	} \
	return false;


qboolean T_Damage (edict_t *targ, edict_t *inflictor, edict_t *attacker, vec3_t dir, vec3_t point, vec3_t normal, int damage, int knockback, int dflags, int mod) {
	gclient_t	*client;
	int			take;
	int			save = 0;
	int			asave = 0;
	int			psave = 0;
	int			te_sparks = TE_SPARKS;

	//we shouldn't really be here ?
	if (!targ->takedamage)
		return false;

	//can't kill stuff after the game is over.
	if (level.intermissiontime && (level.intermissiontime < level.framenum))
		return false;

	//shortcut
	client = targ->client;

	// copy original damage
	take = damage;

	// normalize dir, used below anyway
	VectorNormalize (dir);

	// if target was a player
	if (client)
	{
		//damage specified not to hurt clients
		if (dflags & DAMAGE_IGNORE_CLIENTS)
			return false;

		if (dflags & DAMAGE_GAS)
		{
			//immune to gas.
			if (client->resp.team == TEAM_ALIEN)
				return false;

			//add greenish tint
			if (inflictor->enttype == ENT_GAS_SPORE)
				client->damage_parmor += (damage / 2);
		}

		//r1: prevent wraithspit/fire from tking
		if (mod == MOD_ACID || mod == MOD_BURNED)
			dflags &= ~DAMAGE_RADIUS;

		if (attacker == world && client->invincible_framenum > level.framenum && mod != MOD_FALLING)
			client->invincible_framenum = level.framenum;

		if (targ->deadflag == DEAD_DYING)
			take *= 0.5f;

		//gi.dprintf ("before resistance code: %d\n", take);

 		/*if (level.framenum == client->damage_last_frame && (mod != MOD_SHOTGUN && mod != MOD_SSHOTGUN) && attacker->client && attacker->client->resp.team == TEAM_ALIEN) {
			gi.bprintf (PRINT_HIGH, "multiframe damage (%u:%u): new damage %d (from %d)\n", level.framenum, client->damage_last_frame, (int)(take * 1.5), take);
			take *= 1.5;
		}*/

		if (!(dflags & DAMAGE_RADIUS) && attacker->client)
		{
			client->damage_last_frame = level.framenum;
			client->resp.can_respawn = false;
		}

		// if ignore_res, we want to inflict pure damage
		if (!(dflags & DAMAGE_IGNORE_RESISTANCES))

		#pragma GCC diagnostic push //FV7
		#pragma GCC diagnostic ignored "-Wimplicit-fallthrough" //FV7
		
		switch (client->resp.class_type) {
			case CLASS_GRUNT:
				if (mod == MOD_BURNED)
					take *= 1.25f; //last 1.0
				break;

			case CLASS_HATCHLING:
          /*Savvy hatchling takes 1/2 normal damage when enhanced*/
          if ((client->resp.upgrades) && (client->resp.upgrades & UPGRADE_HATCHLING_ENHANCEMENT)) { take *= 0.5f; }
				//if (mod == MOD_AUTOGUN || mod == MOD_SHOTGUN || mod == MOD_MACHINEGUN)
				//	take -= 2;
				//if (mod == MOD_PULSELASER)
				//	take += 2;
				break;

			case CLASS_ENGINEER:
				if (mod == MOD_GAS_SPORE)
					return false;
				else if (mod == MOD_BURNED)
					take *= 1.15f;
				break;

			case CLASS_WRAITH:
				if (mod == MOD_AUTOGUN)
					take *= 0.5f;
				break;

			case CLASS_GUARDIAN:
				if (client->glow_time >= level.time)
					take *= 1.5f;

				if (mod == MOD_GRENADE || mod == MOD_ROCKET || mod == MOD_R_SPLASH || mod == MOD_KAMIKAZE || mod == MOD_C4)
					take *= 0.25f;
				else if (mod == MOD_EXSHELL)
					take *= 0.2f;
				//else if (mod == MOD_TARGET_LASER)
				//	take *= 2;
				else if (mod == MOD_HYPERBLASTER)
					take *= 0.7f;
				else if (mod == MOD_MELEE)
					take *= 0.3f;
				else if (mod == MOD_RAIL)
					take *= 0.9f;
				else if (mod == MOD_PULSELASER || mod == MOD_MAGNUM)
					take *= 0.8f;
				case MOD_EX_MAGSHELL:
          take *= 0.75f;
          break;

				if (GetArmor(targ) == 0) {
					switch (mod) {
						case MOD_AUTOGUN:
						case MOD_PISTOL:
						case MOD_SHOTGUN:
						case MOD_SSHOTGUN:
							take *= 0.4f;
							break;
						case MOD_HYPERBLASTER:
							take *= 0.7f;
							break;
						case MOD_MACHINEGUN:
							take *= 0.25f;
							break;
					}
				} else {
				  switch (mod) {
					case MOD_SSHOTGUN:
						take *= 0.16f;
						break;
					case MOD_AUTOGUN:
						take *= 0.18f;
						break;
					case MOD_PISTOL:
					case MOD_SHOTGUN:
						take *= 0.34f;
						break;
					case MOD_MACHINEGUN:
						take *= 0.28f;
						break;
					case MOD_EXSHELL:
					case MOD_HYPERBLASTER:
						take *= 0.32f;
						break;
					case MOD_PULSELASER:
						take *= 0.30f;
						break;
					case MOD_DISCHARGE:
						take *= 0.25f;
						break;
					case MOD_EX_MAGSHELL:
						take *= 0.30f;
						break;

					//if (dflags & DAMAGE_BULLET)
					//	take *= 0.8;
				  }
        }
				break;
			case CLASS_STALKER:
				if (client->glow_time >= level.time)
					take *= 1.1f;

				if (mod == MOD_TARGET_LASER)
					take *= 0.45;
				else if (mod == MOD_MELEE)
					take *= 0.3f;
				else if (mod == MOD_EXSHELL)
					take *= 0.77f;

				//else if (dflags & DAMAGE_ENERGY)
				//	take += 11 + random();

			if (GetArmor(targ) == 0) {
				switch (mod) {
					case MOD_PISTOL:
					case MOD_MACHINEGUN:
					case MOD_AUTOGUN:
					case MOD_MGTURRET:
						take *= 0.31f;
						break;

					case MOD_SSHOTGUN:
					case MOD_SHOTGUN:
						take *= 0.5f;
						break;

					case MOD_HYPERBLASTER:
						take *= 0.45f;
						break;
				}

				//if (dflags & DAMAGE_BULLET)
				//	take *= 1.7;
			} else {
				switch (mod) {
				case MOD_PISTOL:
				case MOD_MACHINEGUN:
					take *= 0.19f;
					break;
				case MOD_AUTOGUN:
					take *= 0.12f;
					break;
				case MOD_SHOTGUN:
				case MOD_SSHOTGUN:
					take *= 0.3f;
					break;
				case MOD_MAGNUM:
				case MOD_EXSHELL:
					take *= 0.7f;
					break;
				case MOD_HYPERBLASTER:
					take *= 0.40f;
					break;
				case MOD_PULSELASER:
					take *= 0.4f;
					break;
				case MOD_DISCHARGE:
					take *= 0.8f;
					break;
					case MOD_EX_MAGSHELL:
            take *= 0.50f;
            break;
				}
			}

				break;

			case CLASS_DRONE:
		    if (GetArmor(targ) != 0) {
				  switch (mod) {
				  case MOD_EXSHELL:
				    take += 10;
				    break;
				  }
				/*
					case MOD_PISTOL:
					case MOD_MACHINEGUN:
					case MOD_AUTOGUN:
					case MOD_MGTURRET:
						//take += random() * 3 + random() + 3;
						take *= 0.4;
						break;

					case MOD_SSHOTGUN:
					case MOD_SHOTGUN:
						//take += (random() * 4) + 2;
						take *= 0.6;
						break;

					case MOD_PULSELASER:
						//take = random()*3+2;
						break;

					case MOD_DISCHARGE:
						take /= 2.01;
						break;
				}*/

				//if (dflags & DAMAGE_BULLET)
				//	take *= 1.7;
			//} else {
				  switch (mod) {
					case MOD_PISTOL:
					case MOD_MACHINEGUN:
					case MOD_AUTOGUN:
					case MOD_MAGNUM:
						take *= 0.5f;
						break;
					case MOD_SHOTGUN:
					case MOD_SSHOTGUN:
						take *= 0.9f;
						break;
					case MOD_EXSHELL:
						take *= 0.9f;
						break;
					/*case MOD_PULSELASER:
						take *= 0.6;
						break;*/
					case MOD_EX_MAGSHELL:
            take *= 1.0f;
            break;
				  }
				}
				else {
					switch (mod) {
						case MOD_PISTOL:
						case MOD_AUTOGUN:
							take *= 0.55f;
							break;
						case MOD_HYPERBLASTER:
							take *= 1.20f;
							break;
					}
				}
				break;

			case CLASS_KAMIKAZE:
				if (mod == MOD_GRENADE || mod == MOD_ROCKET || mod == MOD_R_SPLASH || mod == MOD_KAMIKAZE || mod == MOD_C4 || mod == MOD_EXSHELL || mod == MOD_EX_MAGSHELL)
					take *= 0.1f;
				break;

			case CLASS_SHOCK:
				if (mod == MOD_BURNED)
					take *= 0.8f; /*Savvy sts shouldn't resist fire nearly so much, was 0.5f*/
				else if (mod == MOD_ACID)
					take /= 2.5f;
				else if (dflags & DAMAGE_GAS)
					return false;

				break;

			case CLASS_COMMANDO:
				if (mod == MOD_SPIKE && !attacker->client)
					take *= 0.9f;
				else if (attacker->client && attacker->client->resp.class_type == CLASS_HATCHLING)
					take *= 0.80f;
				break;

			case CLASS_MECH:
				if (mod == MOD_BURNED || dflags & DAMAGE_GAS)
					return false;
				else if (mod == MOD_ACID)
					if (GetArmor (targ) > 0)
						take *= 0.4f;
					else
						take *= 0.8f;
				else if (mod == MOD_SPIKE)
					take *= 0.88f;
				//else if (mod == MOD_MELEE && attacker->client && attacker->client->resp.class_type == CLASS_STINGER)
				//	take *= 1.15;
				else if (mod == MOD_SLIME && GetArmor (targ) > 0)
					return false;
				break;

			case CLASS_STINGER:
				switch (mod) {
				case MOD_R_SPLASH:
					take *= 0.80f;
					break;
				case MOD_PULSELASER:
					take -= 25;
					break;
				case MOD_MGTURRET:
					take *= 0.5f;
					break;
				case MOD_PISTOL:
				case MOD_AUTOGUN:
					take *= 0.3f;
					break;
				case MOD_SSHOTGUN:
					take *= 0.25f;
					break;
				case MOD_MACHINEGUN:
					take *= 0.3f; //0.4 last
					break;
				case MOD_HYPERBLASTER:
					take *= 0.45f;
					break;
				case MOD_SHOTGUN:
					take *= 0.38f;
					break;
				case MOD_TARGET_LASER:
					take *= 0.50f;
					break;
				case MOD_MAGNUM:
					take *= 0.40f; /*Savvy edit - was 0.70*/
					break;
				case MOD_EXSHELL:
					take *= 0.80f; /*Savvy edit - was 0.90*/
					break;
				case MOD_EX_MAGSHELL:
					take *= 0.80f;
					break;
				}
				break;

			case CLASS_EXTERM:

				if (mod == MOD_MELEE && attacker->client) {
					if (attacker->client->resp.class_type == CLASS_DRONE)
						take /= 1.5f;
					else if (attacker->client->resp.class_type == CLASS_HATCHLING)
						take *= 0.38f;
					else if (attacker->client->resp.class_type == CLASS_STINGER)
						take *= 0.9f; /*Savvy edit - was 0.8f*/
				}
				else if (mod == MOD_SPIKE && !attacker->client)
					take *= 0.82f;
				else if (mod == MOD_ACID)
					take *= 0.8f;
				else if (dflags & DAMAGE_GAS)
					return false;

				break;

			case CLASS_BREEDER:
				if (mod == MOD_EXSHELL)
					take += 50;
				break;

			case CLASS_BIO:
				if (dflags & DAMAGE_GAS || mod == MOD_ACID)	//bio is resistant
					take *= 0.8f;
				break;
			default:
				break;
		}

		#pragma GCC diagnostic pop //FV7

		//gi.dprintf ("after resistance code: %d\n", take);

		// area damage code
		//R1: update: falling on people as 'big' classes damage bonuses !
		if (attacker->client && (mod == MOD_RUN_OVER || mod == MOD_JUMPATTACK || mod == MOD_OW_MY_HEAD_HURT || mod == MOD_MECHSQUISH))
		{
			//int height = point[2] - targ->s.origin[2] + targ->viewheight;
			int height;// = attacker->absmax[2] -  targ->absmax[2];
			
			height = attacker->s.old_origin[2] - targ->s.origin[2];

			if (mod == MOD_MECHSQUISH && height > 10 && (targ->client->resp.class_type == CLASS_HATCHLING || targ->client->resp.class_type == CLASS_KAMIKAZE))
				mod = MOD_MECHSQUISH_SQUISH;

			if ((attacker->client->resp.class_type == CLASS_STINGER || attacker->client->resp.class_type == CLASS_DRONE) && height > 75) { //drone based fall on head dmg
				int fallspeed = -attacker->client->oldvelocity[2];

				if (fallspeed > 6)
					take += fallspeed / 6;
				//gi.bprintf (PRINT_HIGH,"drone fallspeed additional damage: %d",(int)(fallspeed / 6));

			} else if ((attacker->client->resp.class_type == CLASS_GUARDIAN || attacker->client->resp.class_type == CLASS_STALKER || attacker->client->resp.class_type == CLASS_MECH) && height > 50) {
				int fallspeed = -attacker->client->oldvelocity[2];

				if (fallspeed > 0)
					take += (fallspeed * 2);

				//gi.bprintf (PRINT_HIGH,"fallspeed %d (additional damage: %d)\n",fallspeed, (int)(fallspeed * 2));

				if (fallspeed > 100) //likely to kill, update MoD
					mod = MOD_OW_MY_HEAD_HURT;

			} else if (attacker->client->resp.class_type == CLASS_HATCHLING || attacker->client->resp.class_type == CLASS_KAMIKAZE) {
				//different height calc.
				height = point[2] - targ->s.origin[2] + targ->viewheight;
				//if they're crouched it's always the head (+hack)
				if (client->ps.pmove.pm_flags & PMF_DUCKED) {
					height = 75;
				}

				/*Savvy added in switch statements for the "good old days" hatchie pwnage with headshots*/
				if (height > 66) { /*head hit*/
				  switch (attacker->client->resp.class_type) {
				    case CLASS_HATCHLING:
				      take *= 1.5f;
				      break;
				    case CLASS_KAMIKAZE:
				      take *= 2.0f;
				      break;
				    default: /*Savvy original is below, in default statement*/
              take *= 1.275f;
				  }
				//if (classlist[targ->client->resp.class_type].frags_needed < 5 && targ->client->resp.class_type != CLASS_GRUNT)
				//	take *= 2;
				//gi.bprintf (PRINT_HIGH, "height %d = head\n", height);
				} else if (height > 50) { /*neck hit*/
				  switch (attacker->client->resp.class_type) {
				    case CLASS_HATCHLING:
				      take *= 1.25f;
				      break;
				    case CLASS_KAMIKAZE:
				      take *= 1.75f;
				      break;
				    default: /*Savvy original is below, in default statement*/
            take *= 1.08f;
					}
				//gi.bprintf (PRINT_HIGH, "height %d = neck\n", height);
				}else if (height > 22) { /*body*/
				  switch (attacker->client->resp.class_type) {
				    case CLASS_HATCHLING:
				      take *= 1.0f;
				      break;
				    case CLASS_KAMIKAZE:
				      take *= 1.25f;
				      break;
				    default: /*Savvy original is below, in default statement*/
            take *= 1.02f;
					}
				//gi.bprintf (PRINT_HIGH, "height %d = body\n", height);
				} else if (height > 7) { /*legs*/
				  switch (attacker->client->resp.class_type) {
				    case CLASS_HATCHLING:
				      take *= 0.75f;
				      break;
				    case CLASS_KAMIKAZE:
				      take *= 0.90f;
				      break;
				    default: /*Savvy original is below, in default statement*/
            take *= 0.92f;
					}
				//gi.bprintf (PRINT_HIGH, "height %d = legs\n", height);
				} else { /*feet*/
				  switch (attacker->client->resp.class_type) {
				    case CLASS_HATCHLING:
				      take *= 1.00f;
				      break;
				    case CLASS_KAMIKAZE:
				      take *= 1.25f;
				      break;
				    default: /*Savvy original is below, in default statement*/
            take *= 0.85f;
					}
				//gi.bprintf (PRINT_HIGH, "height %d = feet\n", height);
				}
			//gi.bprintf (PRINT_HIGH, "final damage = %d\n", take);
      
      
			/*Savvy add slightly more dmg if hatchie and upgraded*/
        if ((attacker->client->resp.class_type == CLASS_HATCHLING) && (attacker->client->resp.upgrades & UPGRADE_HATCHLING_ENHANCEMENT)) { take *= 1.25f; }
			}
		}

		// if teammate attacked, do tk checking
		if (attacker->client && client->resp.team == attacker->client->resp.team &&
			!(dflags & (DAMAGE_NO_PROTECTION|DAMAGE_FRIENDLY_FIRE)) && targ != attacker) {
			//stop tking continuing
			if (attacker->client->resp.teamkills == -1)
				return false;

			// FIXME: change the reflect to reflect any damage done, not just splash
			// allow team splash damage
			dflags |= DAMAGE_FRIENDLY_FIRE;
			// splash damage?
			if (mod == MOD_GRENADE || mod == MOD_ROCKET || mod == MOD_R_SPLASH || mod == MOD_C4 || mod == MOD_KAMIKAZE || mod == MOD_EXSHELL || mod == MOD_EX_MAGSHELL) {
				dflags |= DAMAGE_FRIENDLY_SPLASH;
				// allow splash damage?
				if (teamsplashdamage->value) {

					//if we hurt someone, check for reflect damage
					if (teamreflectradiusdamage->value == 2) {
						//full reflect
						T_Damage (attacker, attacker, attacker, dir, point, normal, take, 0, DAMAGE_IGNORE_RESISTANCES|DAMAGE_FRIENDLY_FIRE|DAMAGE_NO_KNOCKBACK, mod);
					} else if (teamreflectradiusdamage->value == 1) {
						//half reflect
						T_Damage (attacker, attacker, attacker, dir, point, normal, (int)(take * 0.5), 0, DAMAGE_IGNORE_RESISTANCES|DAMAGE_FRIENDLY_FIRE|DAMAGE_NO_KNOCKBACK, mod);
					}
				} else {
					take = 0;
				}
			}
		}

		// save last info?
		if (attacker->client &&
		attacker != targ &&
		(mod == MOD_SHOTGUN ||
		mod == MOD_SSHOTGUN ||
		mod == MOD_MELEE ||
		mod == MOD_GRENADE ||
		mod == MOD_R_SPLASH || 
		mod == MOD_KAMIKAZE ||
		mod == MOD_DISCHARGE ||
		mod == MOD_SPIKESPORE ||
		mod == MOD_C4 ||
		mod == MOD_EAT_C4 ||
		mod == MOD_POISON ||
		mod == MOD_SPIKE ||
		mod == MOD_JUMPATTACK ||
		/*Savvy my own additions*/
		mod == MOD_EX_MAGSHELL
		|| mod == MOD_ACID)) {
			if (!(attacker->client->resp.team == client->resp.team && (mod == MOD_JUMPATTACK || mod == MOD_ACID || mod == MOD_SPIKE || mod == MOD_MELEE))) {
			client->last_attacker = attacker;
			client->last_damage = level.time;
			client->last_mod = mod;
			}
		}

		//select type of temp_ent
		if (dflags & DAMAGE_BULLET)
			te_sparks = TE_BULLET_SPARKS;

		//R1: NOTICE! this code is only run when hurting clients!
		//if hurting ent, te_sparks = UNINIT! sending uninit in tempent = crashed clients
		//moving the te_sparks = TE_SPARKS; to the declaration.

		// check any savings, if dmg_no_prot isn't on
		if (!(dflags & DAMAGE_NO_PROTECTION) && take)
		{
			// check for godmode
			if ( (targ->flags & FL_GODMODE))
			{
				take = 0;
				save = damage;
				//SpawnDamage (te_sparks, point, normal, save);
			}

			// check for invincibility
			if (client->invincible_framenum > level.framenum  && (attacker!=targ))
			{
				//noise
				/*if (targ->pain_debounce_time < level.time) {
					gi.sound(targ, CHAN_AUTO, SoundIndex (items_protect4), 1, ATTN_NORM, 0);
					targ->pain_debounce_time = level.time + 2;
				}*/
				take = 0;
				save = damage;
			}

			// check if this goes thru armor
			if (!(dflags & DAMAGE_NO_ARMOR) && take) 
			{
				//power armor
				// unnecessary check for those without power armor
				//psave = CheckPowerArmor (targ, point, normal, take, dflags);
				if (targ->flags & FL_POWER_ARMOR && (client->resp.class_type == CLASS_ENGINEER || !attacker->client) && attacker->enttype != ENT_OBSTACLE)
				{
					int oldtake = take;
					//int index = ITEM_INDEX(FindItem("Cells"));
					
					//gi.bprintf (PRINT_HIGH, "old dmg: %d\n", take);
					take *= 1.0f - ((float)client->resp.inventory[ITEM_AMMO_CELLS]) / 100.0f;
					//gi.bprintf (PRINT_HIGH, "new dmg: %d for %d cells\n", take, (int)((oldtake - take)*0.5f));
					client->resp.inventory[ITEM_AMMO_CELLS] -= (oldtake - take)*0.5f;
					if (client->resp.class_type == CLASS_EXTERM && client->resp.inventory[ITEM_AMMO_CELLS] < 0)
					{
						client->resp.inventory[ITEM_AMMO_CELLS] = 0;
						gi.cprintf(targ, PRINT_HIGH, "Power cells overheated!\n");
						gi.sound(targ, CHAN_AUTO, SoundIndex (weapons_pulseout), 1, ATTN_NORM, 0);
						targ->client->last_move_time = level.time + EXTERM_TAKEDAMAGE_PS_RESTART_DELAY;
						if (targ->client->resp.upgrades & UPGRADE_COOLANT)
							targ->client->last_move_time -= (EXTERM_TAKEDAMAGE_PS_RESTART_DELAY * EXTERM_TAKEDAMAGE_PS_RESTART_MODIFIER);
					}
					client->damage_parmor += (oldtake - take)*0.33f;
					SpawnDamage (TE_SCREEN_SPARKS, point, normal, (int)(take * 0.33f));

					if (client->resp.class_type == CLASS_ENGINEER)
					{
						client->resp.inventory[ITEM_AMMO_CELLS] -= (take*0.5f);
						if (client->resp.inventory[ITEM_AMMO_CELLS] < 0)
							client->resp.inventory[ITEM_AMMO_CELLS] = 0;
					}
					else if (attacker->client && (attacker->client->resp.class_type == CLASS_HATCHLING || attacker->client->resp.class_type == CLASS_KAMIKAZE))
					{
						vec3_t forward;
						VectorCopy(dir, forward);
						VectorScale(forward, -300, forward);
						VectorCopy(forward, attacker->velocity);					
					}
				}

				// normal armor
				asave = CheckArmor (targ, attacker, point, normal, take, te_sparks, dflags, mod, dir);
				take -= asave;
			}

			// allow any teamdamage?
			if (teamdamage->value == 0 && (dflags & DAMAGE_FRIENDLY_FIRE) && !(dflags & DAMAGE_FRIENDLY_SPLASH))
				take = 0;
				//return false;
			else if (teamdamage->value == 1 && attacker->client && classlist[attacker->client->resp.class_type].frags_needed == 0
				&& (dflags & DAMAGE_FRIENDLY_FIRE) && !(dflags & DAMAGE_FRIENDLY_SPLASH))
				// allow teamdamage from 1+ frag classes?
				take = 0;
				//return false;
			// teamdamage > 1, allow all teamdamage
		}
	}
	else {
		// hurting non-client
		// STRUCTURE STUFF

		//damage flags specify can't hurt non-clients
		if (dflags & DAMAGE_IGNORE_STRUCTS)
			return false;

		//gas doesn't hurt spider structures
		if (targ->svflags & SVF_MONSTER)
		{
			if (dflags & DAMAGE_GAS && mod != MOD_INFEST)
				return false;
		}

		if (mod == MOD_ACID)
			take *= 0.4f;

		if (targ->enttype == ENT_TRIPWIRE_BOMB)
			take *= 0.90f;

		//gas doesn't damage structures
		//if (targ->flags & FL_CLIPPING && dflags & DAMAGE_GAS)
		//	return false;

		/*if (targ->svflags & SVF_MONSTER) {
			if (targ->enttype == ENT_COCOON) {
				if (!(dflags & DAMAGE_RADIUS) && mod != MOD_ROCKET && mod != MOD_EXSHELL && mod != MOD_HYPERBLASTER) {
					take *= 0.33;
				}
			} else if (mod == MOD_GRENADE || mod == MOD_KAMIKAZE) {
				take *= 0.5;
			}
		}*/

		//tripwires reduce dmg to other tripwires, can't be set off by fire.

		if (targ->enttype == ENT_TRIPWIRE_BOMB)
		{
			if (mod == MOD_MINE)
				take /= 8;
			else if (mod == MOD_BURNED)
			{
				if (randomMT() & 1)
					take += 8;
			}
		}

		//spikes on proxy spores won't kill tele v. easily
		/*Savvy edit - cross that out you peice of shit, spike spores are hard enough to get close to a tele*/
		if (targ->enttype == ENT_TELEPORTER)
		{
			if (mod == MOD_ACID)
				return false;
			if (dflags & DAMAGE_RADIUS && mod == MOD_SPIKE)
				take = 50; /*Savvy edit - was 35*/
			if (mod == MOD_BURNED)
				take += 22;
		}

		// direct attack from client
		if (attacker && attacker->client)
		{
			//stop tking continuing
			if (attacker->client->resp.teamkills == -1)
				return false;

			if (mod == MOD_AUTOGUN && targ->enttype == ENT_SPIKER)
				take *= 0.4f; /*Savvy edit - was 0.8f*/

			if (targ->enttype == ENT_TURRET || targ->enttype == ENT_TURRETBASE)
			{
				if (mod == MOD_ACID)
					take *= 1.25f;
				else if (mod == MOD_BURNED)
					take *= 1.0f;
			}

			if (targ->enttype == ENT_PROXYSPIKE && attacker->client->resp.team == TEAM_ALIEN)
			{
				if (dflags & DAMAGE_RADIUS)
					return false;
				take *= 0.66f;
			}

			if (mod == MOD_MELEE)
			switch (targ->enttype) {

			case ENT_TURRET:
			case ENT_MGTURRET:
			case ENT_TURRETBASE:
				switch (attacker->client->resp.class_type)
				{

				case CLASS_GUARDIAN:
				case CLASS_WRAITH:
				  /*Savvy stupid
					//guard shouldn't be hitting turrets!
					take /= 3.5f;
					*/
				    if (randomMT() & 1) {
				    take /= 2; /*randomly half damage*/
				    }
					break;
				case CLASS_HATCHLING:
				case CLASS_KAMIKAZE:
					take = 40;
					break;
				case CLASS_STINGER:
					take *= 0.4f;
					break;
				default:
					break;
				}
				break;
			case ENT_TELEPORTER:
				switch (attacker->client->resp.class_type)
				{

				case CLASS_WRAITH:
				case CLASS_HATCHLING:
				case CLASS_KAMIKAZE:
					take = 0;
					break;
				case CLASS_STINGER:
					take *= 0.7f;
					break;
				case CLASS_DRONE:
					// punch up drone tele dmg
					take += 17;
					break;
				case CLASS_GUARDIAN:
					take += 3;
					break;
				default:
					break;
				}
				break;
			default:
				break;
			}

			// toggle for teamspawndamage
			if (teamspawndamage->value == 0) {
				if (targ->enttype == ENT_TELEPORTER && attacker->client->resp.team == TEAM_HUMAN)
					take = 0;
				else if (targ->enttype == ENT_COCOON && attacker->client->resp.team == TEAM_ALIEN)
					take = 0;
			}
		}

		// xmin checks
		if (!(dflags & DAMAGE_NO_PROTECTION)) {
			if (targ->enttype == ENT_TELEPORTER && (targ->flags & FL_MAP_SPAWNED) && level.time < (60*xmins->value))
				take = 0;
			else if (targ->enttype == ENT_COCOON && (targ->flags & FL_MAP_SPAWNED) && level.time < (60*xmins->value))
				take = 0;
		}
	}

	//entity class damage
	if (targ->hurtflags) {
		if (!attacker->client)
			return false;

		switch (mod) {
			case MOD_JUMPATTACK:
			case MOD_MECHSQUISH:
			case MOD_MECHSQUISH_SQUISH:
			case MOD_RUN_OVER:
				if (!(targ->hurtflags & WC_JUMPATTACK)) {
					return false;
				}
				break;
			case MOD_MELEE:
				if (!(targ->hurtflags & WC_MELEE)) {
					HURTCLASS_SPARKS
				}
				break;
			case MOD_AUTOGUN:
				if (!(targ->hurtflags & WC_AUTOGUN)) {
					HURTCLASS_SPARKS
				}
				break;
			case MOD_POKED:
				if (!(targ->hurtflags & WC_POKE)) {
					HURTCLASS_SPARKS
				}
				break;
			case MOD_PISTOL:
				if (!(targ->hurtflags & WC_PISTOL)) {
					HURTCLASS_SPARKS
				}
				break;
			case MOD_SHOTGUN:
				if (!(targ->hurtflags & WC_SHOTGUN)) {
					HURTCLASS_SPARKS
				}
				break;
			case MOD_SSHOTGUN:
				if (!(targ->hurtflags & WC_SCATTER)) {
					HURTCLASS_SPARKS
				}
				break;
			case MOD_MACHINEGUN:
				if (!(targ->hurtflags & WC_SMG)) {
					HURTCLASS_SPARKS
				}
				break;
			case MOD_MAGNUM:
				if (!(targ->hurtflags & WC_MAGNUM)) {
					HURTCLASS_SPARKS
				}
				break;
			case MOD_EXSHELL:
				if (!(targ->hurtflags & WC_EXSHELL)) {
					HURTCLASS_SPARKS
				}
				break;
			case MOD_R_SPLASH:
			case MOD_ROCKET:
				if (!(targ->hurtflags & WC_ROCKET)) {
					HURTCLASS_SPARKS
				}
				break;
			case MOD_GRENADE:
				if (!(targ->hurtflags & WC_GRENADE)) {
					HURTCLASS_SPARKS
				}
				break;
			case MOD_C4:
				if (!(targ->hurtflags & WC_C4)) {
					HURTCLASS_SPARKS
				}
				break;
			case MOD_PULSELASER:
				if (!(targ->hurtflags & WC_PULSE)) {
					HURTCLASS_SPARKS
				}
				break;
			case MOD_DISCHARGE:
				if (!(targ->hurtflags & WC_DISCHARGE)) {
					HURTCLASS_SPARKS
				}
				break;
			case MOD_HYPERBLASTER:
				if (!(targ->hurtflags & WC_AUTOCANNON)) {
					HURTCLASS_SPARKS
				}
				break;
			case MOD_FLASH:
				if (!(targ->hurtflags & WC_FLASHGRENADE)) {
					HURTCLASS_SPARKS
				}
				break;
			case MOD_SPIKESPORE:
				if (!(targ->hurtflags & WC_SPIKESPORE)) {
					HURTCLASS_SPARKS
				}
				break;
			case MOD_SPIKE:
				if (!(targ->hurtflags & WC_SPIKES)) {
					HURTCLASS_SPARKS
				}
				break;
			case MOD_GAS_SPORE:
				if (!(targ->hurtflags & WC_GAS)) {
					HURTCLASS_SPARKS
				}
				break;
			case MOD_BURNED:
				if (!(targ->hurtflags & WC_FIRE)) {
					HURTCLASS_SPARKS
				}
				break;
			case MOD_ACID:
				if (!(targ->hurtflags & WC_WRAITHSPIT)) {
					HURTCLASS_SPARKS
				}
				break;
			case MOD_KAMIKAZE:
				if (!(targ->hurtflags & WC_KAMI)) {
					HURTCLASS_SPARKS
				}
				break;
			default:
				HURTCLASS_SPARKS
		}
	}

	//classes field on things like func_explosive
	if (targ->style) {
		//can only be killed by players
		if (!attacker->client)
			return false;

		//draw "youcan'thurtme" sparks
		if ((!(targ->flags & FL_NO_HURTSPARKS)) && ((targ->style) & classtypebit[attacker->client->resp.class_type])) {
			SpawnDamage2 (TE_LASER_SPARKS, point, normal, 5, 3);
			return false;
		}
	}

	if (targ->flags & FL_NO_KNOCKBACK) {
		//they aren't supposed to be knockedback
		knockback = 0;
	}
  else {
		// figure momentum add
		if (!(dflags & DAMAGE_NO_KNOCKBACK)) {
			if ((knockback) && (targ->movetype != MOVETYPE_NONE) && (targ->movetype != MOVETYPE_BOUNCE) && (targ->movetype != MOVETYPE_PUSH) && (targ->movetype != MOVETYPE_STOP)) {
				vec3_t	kvel;
				float	mass;

				if (targ->mass < 50)
					mass = 50;
				else
					mass = targ->mass;

				// ???
				//if (client)
				//	mass = 250;]

				//r1: fix for overly powerful greande knock
				if (mod == MOD_GRENADE  || mod == MOD_R_SPLASH)
					knockback *= 0.25f;

				if (mod == MOD_SSHOTGUN && targ->client)
					knockback /= (1 + (classlist[targ->client->resp.class_type].frags_needed)*0.2f);

				if (mod == MOD_JUMPATTACK)
					knockback *= 0.5f;

				//gi.bprintf (PRINT_HIGH, "knockback: %d (dir %s)\n", knockback, vtos(dir));
				VectorScale (dir, 500 * (float)knockback / mass, kvel);
				VectorAdd (targ->velocity, kvel, targ->velocity);
			}
		}
	}

	//they lose invuln if they are attacking
	if (attacker->client) {
		if (attacker->client->invincible_framenum > level.framenum) { attacker->client->invincible_framenum = level.framenum; }
	attacker->client->resp.can_respawn = false;
	}

	//ignore resistances flag (water/lava/slime/some others)
	// if ignore_res is on, all damage should be applied without any resistance calcs, eg. pure damage
	if (targ->damage_absorb && !(dflags & DAMAGE_IGNORE_RESISTANCES))
		take -= targ->damage_absorb;

	/*if (targ->client && targ->client->resp.class_type == CLASS_EXTERM) {
		client->resp.inventory[client->ammo_index] -= damage * 0.2f;
		if (client->resp.inventory[client->ammo_index] < 0)
			client->resp.inventory[client->ammo_index] = 0;
	}*/

	//don't want to be giving them health...
	if(take < 0)
		take = 0;

	//gi.bprintf (PRINT_HIGH, "final dmg = %d\n", take);

	//treat cheat/powerup savings the same as armor
	asave += save;

	if (take) {
		if (client) {

			switch (client->resp.class_type) {
			case CLASS_EXTERM:
			case CLASS_MECH:
			case CLASS_ENGINEER:
				//spawn "oil"
				gi.WriteByte(svc_temp_entity);
				gi.WriteByte(TE_LASER_SPARKS);
				gi.WriteByte(20);
				gi.WritePosition(targ->s.origin);
				gi.WriteDir(normal);
				gi.WriteByte(0);
				gi.multicast(targ->s.origin, MULTICAST_PVS);

				break;

			case CLASS_GUARDIAN:

				if (targ->health > 0) {
					//make guardian visible
					targ->s.modelindex = 255;
					targ->fly_sound_debounce_time = level.framenum + 5;
				}
				SpawnDamage (TE_BLOOD, point, normal, take);
				break;

			case CLASS_COMMANDO:

				if (client->resp.primed) {
					//make command invuln. die
					if (client->invincible_framenum > level.framenum)
						client->invincible_framenum = level.framenum;

/*Savvy set up preprocessing as a toggle for these*/
#ifdef CMDO_DROP_C4_FROM_FALL
					//ouch! damnit i dropped the c4!
					if (mod == MOD_FALLING && take > 10) {
						targ->dmg = CMDO_DROP_C4_FALL_EXPLODE_DAMAGE;
						targ->dmg_radius = CMDO_DROP_C4_FALL_EXPLODE_RADIUS;
						client->resp.primed = 0;
						kamikaze_explode (targ, MOD_R_SPLASH, 0);
					}
#endif
#ifdef CMDO_DROP_C4_FROM_DAMAGE
/*Savvy make sure to validate that the above did NOT run*/
          if ((take > 0) && (targ->dmg_radius != CMDO_DROP_C4_FALL_EXPLODE_RADIUS)) {
					const gitem_t *item;
					item = FindItem("C4 Explosive");
					Toss_C4 (targ, item);
					}
#endif
				}
				SpawnDamage (TE_BLOOD, point, normal, take);
				break;

			default:
				//bleed
				SpawnDamage (TE_BLOOD, point, normal, take);
				break;
			}
		}
    else {
			if (targ->svflags & SVF_MONSTER || targ->enttype == ENT_CORPSE || targ->enttype == ENT_HUMAN_BODY || targ->enttype == ENT_FEMALE_BODY) {
			//bleed
			SpawnDamage (TE_BLOOD, point, normal, take);
			} else {
			//armor sparks
			SpawnDamage (te_sparks, point, normal, take);
			}
		}

#ifndef ONECALL
		//gi.dprintf("damage inflicted: %d\n", take);
#endif
		// inflict damage
		targ->health -= take;

		meansOfDeath = mod;

		//did we kill them?
		if (targ->health <= 0) {
		Killed (targ, inflictor, attacker, take, point);
		return true;
		}
    else if (targ->pain && !(targ->flags & FL_GODMODE)) {
		targ->pain (targ, attacker, knockback, take); // run pain function
		}
	}

	// add to the damage inflicted on a player this frame
	// the total will be turned into screen blends and view angle kicks
	// at the end of the frame
	if (client && (!(!take && dflags & (DAMAGE_FRIENDLY_FIRE)) || (dflags & DAMAGE_FRIENDLY_SPLASH))) {
		//if (client->resp.class_type == CLASS_BREEDER && targ->client->build_timer - level.time < 1)
		//	targ->client->build_timer = level.time + 1;

 		client->damage_parmor += psave;
		client->damage_armor += asave;
		client->damage_blood += take;
		client->damage_knockback += knockback;
		VectorCopy (point, client->damage_from);
		client->healthinc_wait = level.framenum + 20;
	}

	//we did the damage
	return true;
}

/*
T_RadiusDamage
*/
void T_RadiusDamage (edict_t *inflictor, edict_t *attacker, float damage, edict_t *ignore, float radius, int mod, int dflags) {
float	points, prod;
edict_t	*ent = NULL;
vec3_t	v;
float knockback;
vec3_t	dir;

dflags |= DAMAGE_RADIUS;

	//r1: small optimz.
	if (dflags & DAMAGE_IGNORE_CLIENTS) { ent = g_edicts + game.maxclients; }

	// FIXME: slow, maybe should use gi.pvs?
	while ((ent = findradius(ent, inflictor->s.origin, radius)) != NULL) {
		if (ent == ignore) { continue; }
		if (!ent->takedamage) { continue; }
		if (dflags & DAMAGE_IGNORE_STRUCTS && !ent->client) { continue; }

		if (!(dflags & DAMAGE_NO_FALLOFF)) {
		VectorAdd (ent->mins, ent->maxs, v);
		VectorMA (ent->s.origin, 0.5f, v, v);
		VectorSubtract (inflictor->s.origin, v, v);

		points = damage - VectorLength (v);
		}
		else { points = damage; }

		// if distance to ent too great, skip it (fixes alpha blending on no damage)
		if (points <= 0) { continue; }

	knockback = points * 1.5f;

	VectorSubtract (ent->s.origin, inflictor->s.origin, dir);
		if (CanDamage (ent, inflictor)) {
			if (ent->client) {
				if(mod == MOD_R_SPLASH) {
					prod = -DotProduct(dir, ent->client->v_angle);
					if(prod > 0) {
						ent->client->blinded_alpha += (prod * points / damage)*10;
						if(ent->client->blinded_alpha > 1.0) { ent->client->blinded_alpha = 1.0; }
						if(ent->client->blinded_alpha < 0.3) { ent->client->blinded_alpha = 0.3; }
					}
				}
				else if (inflictor->enttype == ENT_FLASH_GRENADE) {
				ent->client->blind_time = (5 - (classlist[ent->client->resp.class_type].frags_needed / 2)) / 2;
				ent->client->blind_time_alpha = 1;
					if (ent->client->resp.team == TEAM_HUMAN) {
					ent->client->blind_time /= 2;
					ent->client->blind_time_alpha = 0.75;
					}
					else {
					int		i;
					float	dot;
					vec3_t	forward, vec, viewpos;

						if (inflictor->owner && inflictor->owner->client && (inflictor->owner->client->resp.upgrades & UPGRADE_GLOW_GRENADE)) {
						ent->client->glow_time = level.time + SHOCK_GRENADE_ALIEN_GLOW_TIME;
						}
					
/*Savvy add in immunities to flash nades via the switch statement i added here*/     

						#pragma GCC diagnostic push //FV7
						#pragma GCC diagnostic ignored "-Wimplicit-fallthrough" //FV7

            switch (ent->client->resp.class_type) {
            case CLASS_HATCHLING:
/*Savvy if upgraded hatchling, then break and end switch statement, otherwise continue into the default area, even if class is hatchling*/
              if (ent->client->resp.upgrades & UPGRADE_HATCHLING_ENHANCEMENT) { break; }
            default:
						  //ent->client->frozenmode =  PMF_NO_PREDICTION;
						  //ent->client->frozentime += 1.25;
						  VectorCopy (ent->s.origin, viewpos);
						  viewpos[2] += ent->viewheight;
						  AngleVectors (ent->s.angles, forward, NULL, NULL);
						  VectorSubtract (inflictor->s.origin, viewpos, vec);
						  VectorNormalize (vec);
						  dot = DotProduct (vec, forward);
						    if (dot > 0.5f) {
                  for (i=0 ; i<2 ; i++) { ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(crandom()*(180/(classlist[ent->client->resp.class_type].frags_needed+1)) - ent->client->resp.cmd_angles[i]); }
						    }
						}

						#pragma GCC diagnostic pop //FV7

					}

				knockback = 0;
				points = 10;
				}
			}
			else {
			//r1: stop flash grenades being micro nukes to structures
				if (inflictor->enttype == ENT_FLASH_GRENADE) { points = 20; }
			}

			// the friendly fire flag tells radiusdamage to not hurt (used by engi destroying mines)
			if (dflags & DAMAGE_FRIENDLY_FIRE) { T_Damage (ent, inflictor, attacker, dir, ent->s.origin, vec3_origin, 0, (int)knockback, dflags, mod); }
			else { T_Damage (ent, inflictor, attacker, dir, ent->s.origin, vec3_origin, (int)points, (int)knockback, dflags, mod); }
		}
	}
}

void T_GasDamage (edict_t *inflictor, edict_t *attacker, float damage, edict_t *ignore, float radius, int mod, int dflags)
{
	edict_t	*ent = NULL;
	vec3_t	dir, v;
	int points;

	if(damage == 0)
		return;

	while ((ent = findradius(ent, inflictor->s.origin, radius)) != NULL)
	{
		if (ent == ignore)
			continue;
		if (!ent->takedamage)
			continue;
		if (!ent->client && !attacker->client)
			continue;
/*Savvy the following lines were here:
		//if (ent->enttype == ENT_NOBLOOD_BODY || ent->enttype == ENT_FEMALE_BODY || ent->enttype == ENT_HUMAN_BODY || ent->enttype == ENT_CORPSE)
		//	continue;
*/
		if (ent->enttype == ENT_NOBLOOD_BODY || ent->enttype == ENT_FEMALE_BODY || ent->enttype == ENT_HUMAN_BODY || ent->enttype == ENT_CORPSE) { continue; }

		points = damage;

		VectorAdd (ent->mins, ent->maxs, v);
		VectorMA (ent->s.origin, 0.5f, v, v);
		VectorSubtract (inflictor->s.origin, v, v);

		points *= 1 - (VectorLength (v) / radius);
		//gi.bprintf (PRINT_HIGH, "inflicting %s, radius = %f, dmg = %d\n", ent->classname, VectorLength (v), points);
		// if distance to ent too great, skip it (fixes alpha blending on no damage)
		if (points <= 0)
			continue;



		if (CanDamage (ent, inflictor))
		{
			if (ent->enttype == ENT_TRIPWIRE_BOMB)
				points *= 0.65f;
			else if (ent->enttype == ENT_TURRET || ent->enttype == ENT_TURRETBASE)
				points *= 0.8f; /*Savvy edit - was 0.7f*/
			else if (ent->enttype == ENT_AMMO_DEPOT)
				points *= 1.0f; /*Savvy edit - was 0.75f*/
			else if (ent->enttype == ENT_MGTURRET)
				points *= 0.18f;

			if (ent->enttype == ENT_SPIKE)
				continue;
			
			if (ent->enttype == ENT_TRIPWIRE_BOMB)
				points += 11;

			//r1: set dir! uninit memory otherwise... eek
			VectorSubtract (ent->s.origin, inflictor->s.origin, dir);

			T_Damage (ent, inflictor, attacker, dir, ent->s.origin, vec3_origin, points, damage, DAMAGE_RADIUS | DAMAGE_GAS | DAMAGE_NO_KNOCKBACK | DAMAGE_NO_ARMOR | dflags, mod);
		}
	}
}

void T_InfestDamage (edict_t *inflictor, edict_t *attacker, float damage, edict_t *ignore, float radius, int mod, int dflags) {
edict_t	*ent = NULL;
vec3_t	dir, v;
float points;

	/*if(damage == 0.0)
		return;*/

	while ((ent = findradius_c(ent, inflictor, radius*2)) != NULL) {
		if (ent == ignore) { continue; }

		if (!ent->takedamage) { continue; }

		if (ent->client->resp.team != TEAM_HUMAN) { continue; }

		//points = damage;

	VectorAdd (ent->mins, ent->maxs, v);
	VectorMA (ent->s.origin, 0.5f, v, v);
	VectorSubtract (inflictor->s.origin, v, v);

	points = ((radius * 2) - VectorLength (v)) / (radius *2);
	damage *= points;

	VectorSubtract (ent->s.origin, inflictor->s.origin, dir);

		if (CanDamage (ent, inflictor)) { T_Damage (ent, inflictor, attacker, dir, ent->s.origin, vec3_origin, damage, damage, (DAMAGE_PIERCING | DAMAGE_NO_KNOCKBACK | dflags), mod); }
	}
}

/*Savvy hatchling damage function
eg. T_HatchlingDamage (ent, ent->owner, HATCHLING_RADIUS_DAMAGE, ent, HATCHLING_DAMAGE_RADIUS, MOD_HATCHLING_RADIUS_DMG, 0);
*/
void T_HatchlingDamage (edict_t *inflictor, edict_t *attacker, float damage, edict_t *ignore, float radius, int mod __attribute__((unused)), int dflags) { //FV7 explicitly unuse parameter
edict_t	*ent = NULL;
vec3_t	dir, v;
float points;

	/*if(damage == 0.0)
		return;*/

	while ((ent = findradius_c(ent, inflictor, radius*2)) != NULL) {
		if (ent == ignore) { continue; }
		if (!ent->takedamage) { continue; }
		if (ent->client->resp.team != TEAM_HUMAN) { continue; }
		//points = damage;
	VectorAdd (ent->mins, ent->maxs, v);
	VectorMA (ent->s.origin, 0.5f, v, v);
	VectorSubtract (inflictor->s.origin, v, v);

	points = ((radius * 2) - VectorLength (v)) / (radius * 2);
	damage *= points;

	VectorSubtract (ent->s.origin, inflictor->s.origin, dir);

		if (CanDamage (ent, inflictor)) { T_Damage (ent, inflictor, attacker, dir, ent->s.origin, vec3_origin, damage, damage, DAMAGE_RADIUS | DAMAGE_GAS | DAMAGE_NO_KNOCKBACK | dflags, MOD_HATCHLING_RADIUS_DMG); }
	}
}
