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
#include "m_mech.h"
#include "m_stalker.h"
#include "m_engineer.h"
#include "m_wraith.h"

void PMenu_Update(edict_t *ent);
void EngineerDie(edict_t *ent);

#define current_player level.current_entity
//static	edict_t		*current_player;
static	gclient_t	*current_client;

static	vec3_t	forward, right, up;
float	xyspeed;

float	bobmove;
int		bobcycle;		// odd cycles are right foot going forward
float	bobfracsin;		// sin(bobfrac*M_PI)

/*
SV_CalcRoll

*/
float SV_CalcRoll (vec3_t angles, vec3_t velocity)
{
	float	sign;
	float	side;
	float	value;

	side = DotProduct (velocity, right);
	sign = side < 0 ? -1 : 1;
	side = (float)fabs(side);

	value = sv_rollangle->value;

	if (side < sv_rollspeed->value)
		side = side * value / sv_rollspeed->value;
	else
		side = value;

	return side*sign;

}


/*
P_DamageFeedback

Handles color blends and view kicks
*/
void P_DamageFeedback (edict_t *player)
{
	gclient_t	*client;
	float	side;
	int		realcount, count, kick;
	vec3_t	v;
	int		r, l;
	static	vec3_t	power_color = {0.0, 1.0, 0.0};
	static	vec3_t	acolor = {1.0, 1.0, 1.0};
	static	vec3_t	bcolor = {1.0, 0.0, 0.0};

	client = player->client;

	// flash the backgrounds behind the status numbers
	client->ps.stats[STAT_FLASHES] = 0;

	// total points of damage shot at the player this frame
	count = (client->damage_blood + client->damage_armor + client->damage_parmor);
	if (count == 0)
		return;		// didn't take any damage

	if (client->damage_blood)
		client->ps.stats[STAT_FLASHES] |= 1;
	if (client->damage_armor && !(player->flags & FL_GODMODE) && (client->invincible_framenum <= level.framenum))
		client->ps.stats[STAT_FLASHES] |= 2;

	realcount = count;
	if (count < 10)
		count = 10;	// always make a visible effect

	// start a pain animation if still in the player model
	if (client->damage_blood || client->damage_armor) {
	if (client->anim_priority < ANIM_PAIN && player->s.modelindex == 255)
	{
		int i, start, end;

		client->anim_priority = ANIM_PAIN;

		i = randomMT() & 2;

		if ((client->ps.pmove.pm_flags & PMF_DUCKED) && !classlist[client->resp.class_type].fixview)
		{
			i = 4;
		}

		if (client->resp.flying)
			i = 5;

		switch (i)
		{
		case 5:
			// wraith fly pain
			start = FRAME_PAIN_FLYING_S;
			end = FRAME_PAIN_FLYING_E;
			break;
		case 4:
			// duck pain
			start = FRAME_DUCKPAIN_S;
			end = FRAME_DUCKPAIN_E;
			break;
		case 1:
			start = FRAME_PAIN2_S;
			end = FRAME_PAIN2_E;
			break;
		case 2:
			start = FRAME_PAIN3_S;
			end = FRAME_PAIN3_E;

			if (client->resp.class_type != CLASS_WRAITH)
				break;
			// wraith drops to case 0/3
		default:
		case 3:
		case 0:
			start = FRAME_PAIN1_S;
			end = FRAME_PAIN1_E;
			break;
		}

		player->s.frame = FrameReference (player, start)-1;
		client->anim_end = FrameReference (player, end);
	}

	//r1: always reset ht beep even if not playing pain sound
	if (player->client->resp.class_type == CLASS_HEAVY)
		player->client->missile_target = 0;

	// play an apropriate pain sound
	if (player->health > 0 && (level.time > player->pain_debounce_time) && !(player->flags & FL_GODMODE) && (client->invincible_framenum <= level.framenum))
	{
		float max = player->max_health;
		float cur = player->health;

		if(player->client->resp.team == TEAM_ALIEN)
			player->pain_debounce_time = level.time + 0.2f;
		else {
			player->pain_debounce_time = level.time + 0.5f;

			if (player->client->resp.class_type == CLASS_MECH) {
				cur = player->client->resp.inventory[ITEM_INDEX(player->client->armor)] + player->health;
				max = classlist[player->client->resp.class_type].armorcount;
			}
			//r1: ht reset code was here
			//} else 
		}
#ifdef CACHE_CLIENTSOUNDS
		if (cur/max < 0.25f)
			l = 6;
		else if (cur/max < 0.50f)
			l = 4;
		else if (cur/max < 0.75f)
			l = 2;
		else
			l = 0;

		r = randomMT()&1;

		gi.sound (player, CHAN_AUTO, SoundIndex(classlist[player->client->resp.class_type].sounds[SOUND_PAIN+l+r]), 1, ATTN_IDLE, 0);
#else
		if (cur/max < 0.25)
			l = 25;
		else if (cur/max < 0.50)
			l = 50;
		else if (cur/max < 0.75)
			l = 75;
		else
			l = 100;

		r = 1 + (randomMT()&1);

		gi.sound (player, CHAN_AUTO, gi.soundindex(va("*pain%i_%i.wav", l, r)), 1, ATTN_IDLE, 0);
#endif
	}
	}

	// the total alpha of the blend is always proportional to count
	if (client->damage_alpha < 0)
		client->damage_alpha = 0;
	client->damage_alpha += count*0.01f;
	if (client->damage_alpha < 0.2f)
		client->damage_alpha = 0.2;
	if (client->damage_alpha > 0.5f)
		client->damage_alpha = 0.5;		// don't go too saturated

	// the color of the blend will vary based on how much was absorbed
	// by different armors
	VectorClear (v);
	if (client->damage_parmor)
		VectorMA (v, (float)client->damage_parmor/realcount, power_color, v);
	if (client->damage_armor)
		VectorMA (v, (float)client->damage_armor/realcount, acolor, v);
	if (client->damage_blood)
		VectorMA (v, (float)client->damage_blood/realcount, bcolor, v);
	VectorCopy (v, client->damage_blend);


	//
	// calculate view angle kicks
	//
	kick = abs(client->damage_knockback);
	if (kick && player->health > 0)	// kick of 0 means no view adjust at all
	{
		kick = (float)(kick * 100) / player->health;

		if (kick < count*0.5)
			kick = count*0.5f;
		if (kick > 50)
			kick = 50;

		VectorSubtract (client->damage_from, player->s.origin, v);
		VectorNormalize (v);

		side = DotProduct (v, right);
		client->v_dmg_roll = kick*side*0.3f;

		side = -DotProduct (v, forward);
		client->v_dmg_pitch = kick*side*0.3f;

		client->v_dmg_time = level.time + DAMAGE_TIME;
	}

	//
	// clear totals
	//
	client->damage_blood =
	client->damage_armor =
	client->damage_parmor =
	client->damage_knockback = 0;
}

/*
SV_CalcViewOffset

Auto pitching on slopes?

	fall from 128: 400 = 160000
	fall from 256: 580 = 336400
	fall from 384: 720 = 518400
	fall from 512: 800 = 640000
	fall from 640: 960 =

	damage = deltavelocity*deltavelocity	* 0.0001

*/
void SV_CalcViewOffset (edict_t *ent)
{
	float		*angles;
	float		bob;
	float		ratio;
	float		delta;
	vec3_t		v;
	int bigbob;

	// base angles
	angles = ent->client->ps.kick_angles;

	// if dead, don't add any kick
	if (ent->health > 0)
	{
		// add angles based on weapon kick

		VectorCopy (ent->client->kick_angles, angles);

		// add angles based on damage kick

		ratio = (ent->client->v_dmg_time - level.time) / DAMAGE_TIME;
		if (ratio < 0)
		{
			ratio = 0;
			ent->client->v_dmg_pitch = 0;
			ent->client->v_dmg_roll = 0;
		}
		angles[PITCH] += ratio * ent->client->v_dmg_pitch;
		angles[ROLL] += ratio * ent->client->v_dmg_roll;

		// add pitch based on fall kick

		ratio = (ent->client->fall_time - level.time) / FALL_TIME;
		if (ratio < 0)
			ratio = 0;
		angles[PITCH] += ratio * ent->client->fall_value;

		// add angles based on velocity

		delta = DotProduct (ent->velocity, forward);
		angles[PITCH] += delta*run_pitch->value;

		delta = DotProduct (ent->velocity, right);
		angles[ROLL] += delta*run_roll->value;

		// add angles based on bob
		if(classlist[ent->client->resp.class_type].bob)
			bigbob = 1;
		else
			bigbob = 0;

		delta = bobfracsin * bob_pitch->value * xyspeed;
		if (ent->client->ps.pmove.pm_flags & PMF_DUCKED || bigbob)
			delta *= 6;		// crouching
		angles[PITCH] += delta;
		delta = bobfracsin * bob_roll->value * xyspeed;
		if (ent->client->ps.pmove.pm_flags & PMF_DUCKED || bigbob)
			delta *= 6;		// crouching
		if (bobcycle & 1)
			delta = -delta;
		angles[ROLL] += delta;
	}

	// base origin

	VectorClear (v);

	// add view height

	v[2] += ent->viewheight;

	// add fall height

	ratio = (ent->client->fall_time - level.time) / FALL_TIME;
	if (ratio < 0)
		ratio = 0;
	v[2] -= ratio * ent->client->fall_value * 0.4f;

	// add bob height

	bob = bobfracsin * xyspeed * bob_up->value;
	if (bob > 6)
		bob = 6;
	//if(bob < -6)
		//bob = -6;
	//gi.DebugGraph (bob *2, 255);

	v[2] += bob;

	// add kick offset

	VectorAdd (v, ent->client->kick_origin, v);

	// absolutely bound offsets
	// so the view can never be outside the player box

	if (v[0] < -14)
		v[0] = -14;
	else if (v[0] > 14)
		v[0] = 14;
	if (v[1] < -14)
		v[1] = -14;
	else if (v[1] > 14)
		v[1] = 14;

	if (v[2] < -31)
		v[2] = -31;

	else if (v[2] > 31)
		v[2] = 31;

	VectorCopy (v, ent->client->ps.viewoffset);

	if (ent->client->resp.turret)
		ent->client->ps.viewoffset[2] += 16;

	//TODO: does this look good?
	if (ent->client->resp.class_type == CLASS_HATCHLING && ent->health > 0)
		ent->client->ps.viewoffset[2] -= 8;
}

/*
SV_CalcGunOffset
*/
void SV_CalcGunOffset (edict_t *ent)
{
	int		i;
	float	delta;

	// gun angles from bobbing
	ent->client->ps.gunangles[ROLL] = xyspeed * bobfracsin * 0.002f;
	ent->client->ps.gunangles[YAW] = xyspeed * bobfracsin * 0.01f;
	if (bobcycle & 1)
	{
		ent->client->ps.gunangles[ROLL] = -ent->client->ps.gunangles[ROLL];
		ent->client->ps.gunangles[YAW] = -ent->client->ps.gunangles[YAW];
	}

	ent->client->ps.gunangles[PITCH] = xyspeed * bobfracsin * 0.005f;

	// gun angles from delta movement
	for (i=0 ; i<3 ; i++)
	{
		delta = ent->client->oldviewangles[i] - ent->client->ps.viewangles[i];
		if (delta > 180)
			delta -= 360;
		if (delta < -180)
			delta += 360;
		if (delta > 45)
			delta = 45;
		if (delta < -45)
			delta = -45;
		if (i == YAW)
			ent->client->ps.gunangles[ROLL] += 0.1f*delta;
		ent->client->ps.gunangles[i] += 0.2f * delta;
	}

	// gun height
	VectorClear (ent->client->ps.gunoffset);
//	ent->ps->gunorigin[2] += bob;

	// gun_x / gun_y / gun_z are development tools
	for (i=0 ; i<3 ; i++)
	{
		ent->client->ps.gunoffset[i] += forward[i]*(gun_y->value);
		ent->client->ps.gunoffset[i] += right[i]*gun_x->value;
		ent->client->ps.gunoffset[i] += up[i]* (-gun_z->value);
	}
}


/*
SV_AddBlend
*/
void SV_AddBlend (float r, float g, float b, float a, float *v_blend)
{
	float	a2, a3;

	if (a <= 0)
		return;
	a2 = v_blend[3] + (1-v_blend[3])*a;	// new total alpha
	a3 = v_blend[3]/a2;		// fraction of color from old

	v_blend[0] = v_blend[0]*a3 + r*(1-a3);
	v_blend[1] = v_blend[1]*a3 + g*(1-a3);
	v_blend[2] = v_blend[2]*a3 + b*(1-a3);
	v_blend[3] = a2;
}

/*
SV_CalcBlend
*/
void SV_CalcBlend (edict_t *ent)
{
	int		contents;
	vec3_t	vieworg;
	int		remaining;
	float	fog;

	if (ent->client->chase_target) {
		memcpy (ent->client->ps.blend, ent->client->chase_target->client->ps.blend, sizeof(ent->client->ps.blend));
		return;
	}

	ent->client->ps.blend[0] = ent->client->ps.blend[1] =
		ent->client->ps.blend[2] = ent->client->ps.blend[3] = 0;

	// add for contents
	VectorAdd (ent->s.origin, ent->client->ps.viewoffset, vieworg);
	contents = gi.pointcontents (vieworg);
	if (contents & (CONTENTS_LAVA|CONTENTS_SLIME|CONTENTS_WATER) )
		ent->client->ps.rdflags |= RDF_UNDERWATER;
	else
		ent->client->ps.rdflags &= ~RDF_UNDERWATER;

	if (contents & (CONTENTS_SOLID|CONTENTS_LAVA))
		SV_AddBlend (1.0, 0.3, 0.0, 0.6, ent->client->ps.blend);
	else if (contents & CONTENTS_SLIME)
		SV_AddBlend (0.0, 0.1, 0.05, 0.6, ent->client->ps.blend);
	else if (contents & CONTENTS_WATER)
		SV_AddBlend (0.5, 0.3, 0.2, 0.4, ent->client->ps.blend);

	if (ent->deadflag && ent->client->resp.class_type != CLASS_OBSERVER)
		SV_AddBlend (0.5, 0.0, 0.0, 0.4, ent->client->ps.blend);

	if (!ent->deadflag && ent->client->resp.class_type == CLASS_GUARDIAN && ent->s.modelindex == 0)
		SV_AddBlend (0.0, 0.0, 0.0, 0.25, ent->client->ps.blend);

	if (!ent->deadflag && ent->client->resp.class_type == CLASS_COMMANDO && !(ent->s.renderfx & RF_IR_VISIBLE))
		SV_AddBlend (0.0, 0.0, 0.0, 0.2, ent->client->ps.blend);

	// add for powerups
	if (ent->client->invincible_framenum > level.framenum)
	{
		remaining = ent->client->invincible_framenum - level.framenum;
		if (remaining > 15 || (remaining & 2) )
			SV_AddBlend (1, 1, 0, 0.08, ent->client->ps.blend);
	}

	// Gloom pseudo fog
	if((fog=ent->client->fogged_alpha) > 0.0)
		SV_AddBlend (ent->client->fog_blend[0], ent->client->fog_blend[1], ent->client->fog_blend[2], (fog>=5.0?fog-5:fog), ent->client->ps.blend);

	if(ent->client->blinded_alpha > 0.0)
		SV_AddBlend (0.8, 0.8, 0.8, ent->client->blinded_alpha, ent->client->ps.blend);


	// add for damage
	if (ent->client->damage_alpha > 0)
		SV_AddBlend (ent->client->damage_blend[0],ent->client->damage_blend[1]
		,ent->client->damage_blend[2], ent->client->damage_alpha, ent->client->ps.blend);

	if (ent->client->bonus_alpha > 0)
		SV_AddBlend (0.85, 0.7, 0.3, ent->client->bonus_alpha, ent->client->ps.blend);

	// drop the damage value
	ent->client->damage_alpha -= 0.06;
	if (ent->client->damage_alpha < 0)
		ent->client->damage_alpha = 0;

	// drop the bonus value
	ent->client->bonus_alpha -= 0.1;
	if (ent->client->bonus_alpha < 0)
		ent->client->bonus_alpha = 0;

	ent->client->blinded_alpha -= 0.03;
	if (ent->client->blinded_alpha < 0)
		ent->client->blinded_alpha = 0;

	// GLOOM - Smoke Grenade
	if (ent->client->smokeintensity)
	{
		float x = (float) ent->client->smokeintensity/100;
		SV_AddBlend(0.7, 0.7, 0.7, x, ent->client->ps.blend);
		if (ent->client->smokeintensity > 0) {
			if (ent->client->smokeintensity > 50)
				ent->client->smokeintensity -= 2;
			else
				ent->client->smokeintensity -= 3;

			if (ent->client->smokeintensity < 0)
				ent->client->smokeintensity = 0;
		}
	}
}

/*
P_FallingDamage
*/
void P_FallingDamage (edict_t *ent)
{
	float	delta;
	int		damage;
	vec3_t	dir;

	//if (ent->s.modelindex != 255 && ent->client->resp.class_type != CLASS_GUARDIAN)
		//return;		// not in the player model

	if (ent->deadflag == DEAD_DEAD)
		return;

	if (ent->movetype == MOVETYPE_NOCLIP)
		return;

	//brian: reversed it, used to say which ones didn't get hurt, I made it say which ones DO get hurt
	if ((ent->client->resp.team == TEAM_ALIEN) && (ent->client->resp.class_type != CLASS_BREEDER) && (ent->client->resp.class_type != CLASS_GUARDIAN) && (ent->client->resp.class_type != CLASS_STALKER)) {
		return;
	}

	if ((ent->client->oldvelocity[2] < 0) && (ent->velocity[2] > ent->client->oldvelocity[2]) && (!ent->groundentity))
	{
		delta = ent->client->oldvelocity[2];
	}
	else
	{
		if (!ent->groundentity)
			return;
		delta = ent->velocity[2] - ent->client->oldvelocity[2];
	}
	delta = delta*delta * 0.0001f;

	// no falling damage when grappling
	if ((ent->client->ctf_grapple && ent->client->ctf_grapple->spawnflags > CTF_GRAPPLE_STATE_FLY))
		return;

	// never take falling damage if completely underwater
	if (ent->waterlevel == 3)
		return;
	if (ent->waterlevel == 2)
		delta *= 0.5f;
	if (ent->waterlevel == 1)
		delta *= 0.75f;

	if (delta < 1)
		return;

	if (delta < 15)
	{
		if (ent->client->resp.team == TEAM_HUMAN)
		{
			if(ent->client->resp.class_type != CLASS_MECH && ent->client->resp.class_type != CLASS_ENGINEER)
			{
				if(ent->client->resp.class_type == CLASS_EXTERM) {
#ifdef CACHE_CLIENTSOUNDS
					gi.sound (ent, CHAN_AUTO, SoundIndex (exterm_step), 1, ATTN_NORM, 0);
#else
					gi.sound (ent, CHAN_AUTO, gi.soundindex ("*step.wav"), 1, ATTN_NORM, 0);
#endif
				} else {
					if (classlist[ent->client->resp.class_type].footsteps) {
						ent->s.event = EV_FOOTSTEP;
					}
				}
			}
			return;
		}
	}

	ent->client->fall_value = delta*0.5f;

	if (ent->client->fall_value > 40)
		ent->client->fall_value = 40;

	ent->client->fall_time = level.time + FALL_TIME;

	if (delta > 30)
	{
		if (ent->health > 0)
		{
			if (delta >= 55) {
				if (ent->client->resp.class_type == CLASS_ENGINEER){
#ifdef CACHE_CLIENTSOUNDS
					gi.sound (ent, CHAN_AUTO, SoundIndex(classlist[CLASS_ENGINEER].sounds[SOUND_PAIN+5]), 1, ATTN_NORM, 0);
#else
					gi.sound (ent, CHAN_AUTO, gi.soundindex ("*pain25_1.wav"), 1, ATTN_IDLE, 0);
#endif
				} else {
					//r1: s.event is much more net efficient than explicit gi.sound...
//#ifdef CACHE_CLIENTSOUNDS
//					gi.sound (ent, CHAN_AUTO, SoundIndex(classlist[ent->client->resp.class_type].sounds[SOUND_FALL]), 1, ATTN_NORM, 0);
//#else
					ent->s.event = EV_FALLFAR;
//#endif
				}
			} else {
				if (ent->client->resp.class_type == CLASS_ENGINEER) {
#ifdef CACHE_CLIENTSOUNDS
					gi.sound (ent, CHAN_AUTO, SoundIndex(classlist[CLASS_ENGINEER].sounds[SOUND_PAIN + 4]), 1, ATTN_NORM, 0);
#else
					gi.sound (ent, CHAN_AUTO, gi.soundindex ("*pain50_1.wav"), 1, ATTN_IDLE, 0);
#endif
				} else {
//#ifdef CACHE_CLIENTSOUNDS
//					gi.sound (ent, CHAN_AUTO, SoundIndex(classlist[ent->client->resp.class_type].sounds[SOUND_FALL+1]), 1, ATTN_NORM, 0);
//#else
					ent->s.event = EV_FALL;
				}
			}
//#endif
		}
		ent->pain_debounce_time = level.time;	// no normal pain sound


		damage = (delta * ((delta / 140)*2))*0.75f;

		VectorSet (dir, 0, 0, 1);

		if (damage < 2)
			damage = 2;
		//damage *= 1.1;

		//poor guardian legs get smashed...
		if (ent->client->resp.class_type == CLASS_GUARDIAN)
			damage *= 1.5f;

		T_Damage (ent, world, world, dir, ent->s.origin, vec3_origin, damage, 0, DAMAGE_IGNORE_RESISTANCES | DAMAGE_NO_ARMOR, MOD_FALLING);
	}
	else
	{
		if (ent->client->resp.team == TEAM_HUMAN) {
			if (classlist[ent->client->resp.class_type].footsteps) {
#ifdef CACHE_CLIENTSOUNDS
				gi.sound (ent, CHAN_AUTO, SoundIndex(player_land1), 1, ATTN_NORM, 0);
#else
				ent->s.event = EV_FALLSHORT;
#endif
			}
		}
		return;
	}
}

/*
P_WorldEffects
*/
void P_WorldEffects (void)
{
	//qboolean	breather;
	//qboolean	envirosuit;
	int			waterlevel, old_waterlevel;

	if (current_player->movetype == MOVETYPE_NOCLIP)
	{
		current_player->air_finished = level.time + 12;	// don't need air
		return;
	}

	waterlevel = current_player->waterlevel;
	old_waterlevel = current_client->old_waterlevel;
	current_client->old_waterlevel = waterlevel;


//	breather = current_client->breather_framenum > level.framenum;
//	envirosuit = current_client->enviro_framenum > level.framenum;

	//
	// if just entered a water volume, play a sound
	//
	if (!old_waterlevel && waterlevel)
	{
//		PlayerNoise(current_player, current_player->s.origin, PNOISE_SELF);
		if (current_player->watertype & CONTENTS_LAVA && current_player->client->resp.team == TEAM_HUMAN && (current_player->client->resp.class_type == CLASS_GRUNT || current_player->client->resp.class_type == CLASS_BIO ||current_player->client->resp.class_type == CLASS_SHOCK ||current_player->client->resp.class_type == CLASS_HEAVY ||current_player->client->resp.class_type == CLASS_COMMANDO))
			gi.sound (current_player, CHAN_AUTO, SoundIndex (player_lava_in), 1, ATTN_NORM, 0);
		else if (current_player->watertype & CONTENTS_SLIME){
			if (current_client->resp.team==TEAM_ALIEN)
				gi.sound (current_player, CHAN_AUTO, SoundIndex (alien_watr_in), 1, ATTN_NORM, 0);
			else
				gi.sound (current_player, CHAN_AUTO, SoundIndex (player_watr_in), 1, ATTN_NORM, 0);
		}else if (current_player->watertype & CONTENTS_WATER){
			if (current_client->resp.team==TEAM_ALIEN)
				gi.sound (current_player, CHAN_AUTO, SoundIndex (alien_watr_in), 1, ATTN_NORM, 0);
			else
				gi.sound (current_player, CHAN_AUTO, SoundIndex (player_watr_in), 1, ATTN_NORM, 0);
		}
		current_player->flags |= FL_INWATER;

		// clear damage_debounce, so the pain sound will play immediately
		current_player->damage_debounce_time = level.time - 1;
	}

	//
	// if just completely exited a water volume, play a sound
	//
	if (old_waterlevel && ! waterlevel)
	{
//		PlayerNoise(current_player, current_player->s.origin, PNOISE_SELF);
		if (current_client->resp.team==TEAM_ALIEN)
			gi.sound (current_player, CHAN_AUTO, SoundIndex (alien_watr_out), 1, ATTN_NORM, 0);
		else
			gi.sound (current_player, CHAN_AUTO, SoundIndex (player_watr_out), 1, ATTN_NORM, 0);
		current_player->flags &= ~FL_INWATER;
	}

	//
	// check for head just going under water
	//
	if (old_waterlevel != 3 && waterlevel == 3)
	{
		if (current_client->resp.team==TEAM_ALIEN)
			gi.sound (current_player, CHAN_AUTO, SoundIndex (alien_watr_un), 1, ATTN_NORM, 0);
		else
			gi.sound (current_player, CHAN_AUTO, SoundIndex (player_watr_un), 1, ATTN_NORM, 0);
	}

	//
	// check for head just coming out of water
	//
	if (old_waterlevel == 3 && waterlevel != 3)
	{
		if (current_player->air_finished < level.time)
		{	// gasp for air
			if (current_player->client->resp.team==TEAM_ALIEN){
				gi.sound (current_player, CHAN_AUTO, SoundIndex (alien_gasp1), 1, ATTN_NORM, 0);
			}else{
				gi.sound (current_player, CHAN_AUTO, SoundIndex (player_gasp1), 1, ATTN_NORM, 0);
			}
//			PlayerNoise(current_player, current_player->s.origin, PNOISE_SELF);
		}
		else	if (current_player->air_finished < level.time + 11)
		{	// just break surface
			if (current_player->client->resp.team==TEAM_ALIEN){
				gi.sound (current_player, CHAN_AUTO, SoundIndex (alien_gasp2), 1, ATTN_NORM, 0);
			}else{
				gi.sound (current_player, CHAN_AUTO, SoundIndex (player_gasp2), 1, ATTN_NORM, 0);
			}
		}
	}

	//
	// check for drowning
	//
	if (waterlevel == 3)
	{
		switch (current_player->client->resp.class_type){
		case CLASS_EXTERM:
			T_Damage (current_player, world, world, vec3_origin, current_player->s.origin, vec3_origin, 3 /* 9 */, 0, DAMAGE_NO_ARMOR, MOD_SHORT);
			return;
		case CLASS_MECH:
			T_Damage (current_player, world, world, vec3_origin, current_player->s.origin, vec3_origin, 18 /* 60 */, 0, DAMAGE_NO_ARMOR, MOD_SHORT);
			return;
		case CLASS_ENGINEER:
			T_Damage (current_player, world, world, vec3_origin, current_player->s.origin, vec3_origin, 9 /* 3 */, 0, DAMAGE_NO_ARMOR, MOD_SHORT);
			return;
		}

		// if out of air, start drowning
		if (current_player->air_finished < level.time)
		{	// drown!
			if (current_player->client->next_drown_time < level.time
				&& current_player->health > 0)
			{
				int dmg = 4 + random() * 9;
				current_player->client->next_drown_time = level.time + 1;

				// play a gurp sound instead of a normal pain sound
				if (current_player->health <= dmg){
					if (current_client->resp.team==TEAM_ALIEN)
						gi.sound (current_player, CHAN_AUTO, SoundIndex (alien_drown1), 1, ATTN_NORM, 0);
					else
						gi.sound (current_player, CHAN_AUTO, SoundIndex (player_drown1), 1, ATTN_NORM, 0);
				}else if (randomMT()&1){
					if (current_client->resp.team==TEAM_ALIEN)
						gi.sound (current_player, CHAN_AUTO, SoundIndex (alien_gurp1), 1, ATTN_NORM, 0);
					else
						gi.sound (current_player, CHAN_AUTO, gi.soundindex("*gurp1.wav"), 1, ATTN_NORM, 0);
				}else{
					if (current_client->resp.team==TEAM_ALIEN)
						gi.sound (current_player, CHAN_AUTO, SoundIndex (alien_gurp2), 1, ATTN_NORM, 0);
					else
						gi.sound (current_player, CHAN_AUTO, gi.soundindex("*gurp2.wav"), 1, ATTN_NORM, 0);
				}

				current_player->pain_debounce_time = level.time;

				T_Damage (current_player, world, world, vec3_origin, current_player->s.origin, vec3_origin, dmg, 0, DAMAGE_NO_ARMOR | DAMAGE_IGNORE_RESISTANCES, MOD_WATER);
			}
		}
	}
	else if (waterlevel == 2)
	{
		current_player->air_finished = level.time + 12;
		switch (current_player->client->resp.class_type){
			case CLASS_EXTERM:
				T_Damage (current_player, world, world, vec3_origin, current_player->s.origin, vec3_origin, 3 /* 9 */, 0, DAMAGE_NO_ARMOR, MOD_SHORT);
				return;
			case CLASS_MECH:
				T_Damage (current_player, world, world, vec3_origin, current_player->s.origin, vec3_origin, 18 /* 60 */, 0, DAMAGE_NO_ARMOR, MOD_SHORT);
				return;
			case CLASS_ENGINEER:
				T_Damage (current_player, world, world, vec3_origin, current_player->s.origin, vec3_origin, 9 /* 3 */, 0, DAMAGE_NO_ARMOR, MOD_SHORT);
				return;
		}
	}
	else
	{
		current_player->air_finished = level.time + 12;
	}

	//
	// check for sizzle damage
	//
	if (waterlevel && (current_player->watertype&(CONTENTS_LAVA|CONTENTS_SLIME)) )
	{
		if (current_player->watertype & CONTENTS_LAVA)
		{
			/*if (current_player->health > 0
				&& current_player->pain_debounce_time <= level.time
				&& current_player->client->invincible_framenum < level.framenum && current_player->client->resp.team == TEAM_HUMAN)
			{
				if (randomMT()&1)
					gi.sound (current_player, CHAN_AUTO, SoundIndex (player_burn1), 1, ATTN_NORM, 0);
				else
					gi.sound (current_player, CHAN_AUTO, SoundIndex (player_burn2), 1, ATTN_NORM, 0);
				current_player->pain_debounce_time = level.time + 1;
			}*/

			//if (envirosuit)	// take 1/3 damage with envirosuit
			//	T_Damage (current_player, world, world, vec3_origin, current_player->s.origin, vec3_origin, 1*waterlevel, 0, 0, MOD_LAVA);
			//else
			T_Damage (current_player, world, world, vec3_origin, current_player->s.origin, vec3_origin, 9*waterlevel, 0, DAMAGE_IGNORE_RESISTANCES, MOD_LAVA);
		}

		if (current_player->watertype & CONTENTS_SLIME)
		{// Gloom
			if (current_player->client->resp.team != TEAM_ALIEN)
			{	// no damage from slime with envirosuit
				T_Damage (current_player, world, world, vec3_origin, current_player->s.origin, vec3_origin, 3*waterlevel, 0, 0, MOD_SLIME);
			}
		}
	}
}

/*
G_SetClientEffects
*/
void G_SetClientEffects (edict_t *ent)
{
	int		pa_type;
	int		remaining;

	ent->s.effects = 0;
	ent->s.renderfx = 0;

	if (!ent->client)
		return;

	if (level.intermissiontime && (level.intermissiontime < level.framenum))
		return;

	if (!ent->deadflag && ent->health > 0) {
		ent->s.renderfx |= RF_IR_VISIBLE;
	} else {
		//r1: still in IR when playing death anims
		if (ent->s.frame != ent->client->anim_end) {
			ent->s.renderfx |= RF_IR_VISIBLE;
		}
		return;
	}

	if (ent->client->resp.flying && ent->client->resp.class_type== CLASS_EXTERM)
	{
		if (ent->client->resp.upgrades & UPGRADE_CHARON_BOOSTER)
			ent->s.effects |= EF_GRENADE;
		ent->s.effects |= EF_ROCKET;
	}

	if (ent->client->powerarmor_time > level.time)
	{
		pa_type = PowerArmorType (ent);
		if (pa_type == POWER_ARMOR_SCREEN)
		{
			ent->s.effects |= EF_POWERSCREEN;
		}
		else if (pa_type == POWER_ARMOR_SHIELD)
		{
			ent->s.effects |= EF_COLOR_SHELL;
			ent->s.renderfx |= RF_SHELL_GREEN;
		}
	}

	if (ent->client->resp.class_type == CLASS_HATCHLING && (ent->client->resp.upgrades & UPGRADE_TRANS_MEMBRANE))
		ent->s.effects |= EF_SPHERETRANS;

/*	if (ent->client->quad_framenum > level.framenum)
	{
		remaining = ent->client->quad_framenum - level.framenum;
		if (remaining > 30 || (remaining & 4) )
			ent->s.effects |= EF_QUAD;
	}*/

	if (ent->client->invincible_framenum > level.framenum && ent->client->resp.team==TEAM_HUMAN){
		remaining = ent->client->invincible_framenum - level.framenum;
		if (remaining > 30 || (remaining & 4) )
			ent->s.effects |= EF_HALF_DAMAGE;
	}

	// show cheaters!!!
	if (ent->flags & FL_GODMODE)
	{
		ent->s.effects |= EF_COLOR_SHELL;
		ent->s.renderfx |= (RF_SHELL_RED|RF_SHELL_GREEN|RF_SHELL_BLUE);
	}

	if (ent->client->resp.team==TEAM_ALIEN){
		/*edict_t *e=NULL;
		// FIXME: findrad = slow = moved to detector think, increased detector think speed... small comrpromise
		while ((e=findradius(e,ent->s.origin,350))){
			if (e->enttype == ENT_DETECTOR)
				ent->client->glow_time = level.time + 1;
		}*/

		if (ent->client->grenade_blow_up > 0)
		{
			if (ent->client->grenade_blow_up == 1)
				ent->s.effects |= EF_FLAG1;
			else
				ent->s.effects |= EF_FLAG2;
		}

		if (ent->client->glow_time >= level.time)
		{
			if (ent->client->resp.class_type == CLASS_KAMIKAZE)
			{
				ent->s.effects |= EF_COLOR_SHELL;
				ent->s.renderfx |= RF_SHELL_RED;
			}
			else if (ent->client->resp.class_type == CLASS_HATCHLING && (ent->client->resp.upgrades & UPGRADE_TRANS_MEMBRANE))
			{
				//no effect!
			}
			else
			{
				ent->s.effects |= EF_DOUBLE;
			}
		}
	}

	// guardian cloaks instantly and commando goes stealth after a while when still
	if (xyspeed > 1.0f || !ent->groundentity)
	{
		if (ent->client->resp.class_type == CLASS_GUARDIAN)
		{
			if (!((ent->client->resp.upgrades & UPGRADE_CELL_WALL) || xyspeed < 150) || ent->client->grenade_blow_up > 0 || ent->client->glow_time >= level.time) 
				ent->s.modelindex = 255;
		}
		else if (ent->client->resp.class_type == CLASS_COMMANDO)
		{
			if (!(ent->client->ps.pmove.pm_flags & PMF_DUCKED))
				ent->fly_sound_debounce_time = level.framenum + 5;
			else
				xyspeed = 0;
				//ent->s.renderfx &= ~RF_IR_VISIBLE;
		}
	}

	if (!(xyspeed > 1.0f || !ent->groundentity)) {
		if (ent->client->resp.class_type == CLASS_GUARDIAN) {
			if (ent->client->grenade_blow_up > 0 || ent->client->glow_time >= level.time)
				ent->s.modelindex = 255;
			else {
				if (ent->s.modelindex && level.framenum > ent->fly_sound_debounce_time) {
					ent->client->last_reload_time = level.time + 5;
					ent->s.modelindex = 0;
				} else if (ent->s.modelindex == 0) {
					//r1: set effects so the server still sends position info to the client
					//(avoids new ent spikes on uncloaking, allows guardian "glow" to be 100% accurate)
					ent->s.effects |= EF_SPHERETRANS;
				}
			}
		} else if (ent->client->resp.class_type == CLASS_COMMANDO && (level.framenum > ent->fly_sound_debounce_time) && !ent->client->resp.primed) {
			ent->s.renderfx &= ~RF_IR_VISIBLE;
			//ent->s.renderfx |= RF_TRANSLUCENT;
		}
	}
	else
	{
		if (ent->client->resp.class_type == CLASS_GUARDIAN && ((ent->client->resp.upgrades & UPGRADE_CELL_WALL) || xyspeed < 150))
		{
			if (ent->s.modelindex == 0)
			{
				ent->client->last_reload_time = level.time + 5;
				ent->s.effects |= EF_SPHERETRANS;
			}
		}
	}
}

/*
G_SetClientEvent
*/
void G_SetClientEvent (edict_t *ent)
{
	if (ent->s.event)
		return;

	if (classlist[ent->client->resp.class_type].footsteps)
	if ( ent->groundentity && xyspeed > 225)
	{
		if ( (int)(ent->client->bobtime+bobmove) != bobcycle )
		{
			if(ent->client->resp.class_type == CLASS_EXTERM)
				gi.sound (ent, CHAN_AUTO, SoundIndex (exterm_step), 1, ATTN_NORM, 0);
			else
				ent->s.event = EV_FOOTSTEP;
		}
	}
}

/*
G_SetClientSound
*/
void G_SetClientSound (edict_t *ent)
{
	if (ent->waterlevel && (ent->watertype&(CONTENTS_LAVA|CONTENTS_SLIME)) && (ent->client->resp.team != TEAM_ALIEN))
		ent->s.sound = SoundIndex (player_fry);
	else if(ent->client->grenade_blow_up > 0)
		ent->s.sound = SoundIndex (weapons_tick2);
	else if (ent->client->resp.flying && ent->client->resp.class_type==CLASS_WRAITH)
		ent->s.sound = SoundIndex (wraith_fly);
	else if (ent->client->weapon_sound)
		ent->s.sound = ent->client->weapon_sound;
	else
		ent->s.sound = 0;
}

/*
G_SetClientFrame
*/
void G_SetClientFrame (edict_t *ent)
{
	gclient_t	*client;
	qboolean	duck, run, fly;

	if (ent->s.modelindex != 255 && (ent->client->resp.class_type != CLASS_GUARDIAN && ent->health > 0))
		return;		// not in the player model

	client = ent->client;

	if (client->ps.pmove.pm_flags & PMF_DUCKED)	{
		if (!classlist[client->resp.class_type].fixview)
			duck = true;
		else
			duck = false;
	}else
		duck = false;

	if (xyspeed > 1.0){
		run = true;
	}else{
		run = false;
	}
	if (client->resp.flying && client->resp.class_type==CLASS_WRAITH)
		fly=true;
	else
		fly=false;

	// check for stand/duck and stop/go transitions
	if (fly != client->anim_fly && client->anim_priority <= ANIM_JUMP)
		goto newanim;
	if (duck != client->anim_duck && client->anim_priority < ANIM_DEATH)
		goto newanim;
	if (run != client->anim_run && client->anim_priority == ANIM_BASIC)
		goto newanim;
	if (!ent->groundentity && client->anim_priority <= ANIM_WAVE)
		goto newanim;

	if (ent->s.frame < client->anim_end) {	// continue an animation
		ent->s.frame++;

		if (client->anim_priority == ANIM_DEATH){
			// Evil hard coding:
			if(ent->client->resp.class_type == CLASS_ENGINEER)
				EngineerDie(ent);
			// FIXME: breederdie was broken, fixed, and removed :)
			if(ent->client->resp.class_type == CLASS_BREEDER)
				BreederDie(ent);
		}

		// Play mech walk sounds	-- Should be in G_SetClientEvent
		if(ent->client->resp.class_type == CLASS_MECH){
			if(ent->s.frame == MECH_WALK_S + 2 || ent->s.frame == MECH_WALK_S + 8 ||
			   ent->s.frame == MECH_WALKSHOOT_S+2 || ent->s.frame == MECH_WALKSHOOT_S+8)
			{
#ifdef CACHE_CLIENTSOUNDS
				gi.sound(ent, CHAN_AUTO, SoundIndex(mech_step), 1, ATTN_NORM, 0);
#else
				gi.sound(ent, CHAN_AUTO, gi.soundindex("*mechwalk.wav"), 1, ATTN_NORM, 0);
#endif
			}
		}
		return;
	}

	// don't let death to new anim part
	if (client->anim_priority == ANIM_DEATH){
		return;
	}

	if (client->anim_priority == ANIM_JUMP && !fly){
		if (!ent->groundentity)
			return;		// stay there
		ent->client->anim_priority = ANIM_WAVE;

		if(ent->client->resp.class_type == CLASS_STALKER)
		{
			ent->s.frame = STALK_LAND_S;
			ent->client->anim_end = STALK_LAND_E;
//		}
//		else if(ent->client->resp.class_type == CLASS_DRONE){
//			ent->s.frame = FrameReference(ent, FRAME_JUMP_S)+4;
//			ent->client->anim_end = FrameReference(ent, FRAME_JUMP_E);
		} else
		{
			//ent->s.frame = FrameReference(ent, FRAME_JUMP_S)+2;
			ent->s.frame++;
			ent->client->anim_end = FrameReference(ent, FRAME_JUMP_E);
		}
		return;
	}

newanim:

	// return to either a running or standing frame
 	client->anim_priority = ANIM_BASIC;
	client->anim_duck = duck;
	client->anim_run = run;

	if (fly){

		// new fly
		ent->s.frame = FrameReference(ent, FRAME_FLY_S);
		client->anim_end = FrameReference(ent, FRAME_FLY_E);
		client->anim_priority = ANIM_JUMP;

		client->anim_fly = fly; // fly state on

	} else if (!fly && client->anim_fly) {

		// end fly
		ent->s.frame = FrameReference(ent, FRAME_FLY_FALL_S);
		client->anim_end = FrameReference(ent, FRAME_FLY_FALL_E);
		client->anim_priority = ANIM_JUMP;

		client->anim_fly = fly; // fly state off

	} else if(!ent->groundentity){
		if (ent->client->resp.turret) {
			ent->s.frame = FrameReference(ent, FRAME_STAND_S);
			client->anim_end = FrameReference(ent, FRAME_STAND_E);
			return;
		} else {
			client->anim_priority = ANIM_JUMP;
		}

		// drone anim fixed in frame defines/m_drone.h
		//if(ent->client->resp.class_type == CLASS_DRONE){

		//	ent->s.frame = FrameReference(ent, FRAME_JUMP_S)+2; // +3
		//	ent->client->anim_end = FrameReference(ent, FRAME_JUMP_S)+3;


		if(ent->client->resp.class_type == CLASS_STALKER) {

			ent->s.frame = FrameReference(ent, FRAME_JUMP_S)+4;
			ent->client->anim_end = FrameReference(ent, FRAME_JUMP_S)+4;

		}else{
			if (ent->s.frame != FrameReference(ent, FRAME_JUMP_S)+1)
				ent->s.frame = FrameReference(ent, FRAME_JUMP_S);

			client->anim_end = FrameReference(ent, FRAME_JUMP_S)+1;
		}

	} else if (run){
		if (duck){
			ent->s.frame = FrameReference(ent, FRAME_DUCKRUN_S);
			client->anim_end = FrameReference(ent, FRAME_DUCKRUN_E);
		}else{
			ent->s.frame = FrameReference(ent, FRAME_RUN_S);
			client->anim_end = FrameReference(ent, FRAME_RUN_E);
		}
	}else if (duck){
		ent->s.frame = FrameReference(ent, FRAME_DUCK_S);
		client->anim_end = FrameReference(ent, FRAME_DUCK_E);
	}else{ // standing
		ent->s.frame = FrameReference(ent, FRAME_STAND_S);
		client->anim_end = FrameReference(ent, FRAME_STAND_E);
	}

}

/*
 * ClientEndServerFrames
 */
void ClientEndServerFrames (void)
{
	edict_t	*ent;
	int i;

	//r1: replenishing build points hack (wrong place i know but needed to be called before stats is populated)
	for (i = 0; i < MAXTEAMS; i++)
	{
		if (!team_info.buildpool[i])
			continue;

		if (replenishbp->value && i == TEAM_ALIEN)
		{
			if (team_info.buildtime[i] < level.framenum)
			{
				team_info.points[i] -= replenishbp_amt->value;
				team_info.buildpool[i] -= replenishbp_amt->value;
				team_info.buildtime[i] = level.framenum + replenishbp_tick->value;
			}
		}
		else
		{
			team_info.points[i] -= team_info.buildpool[i];
			team_info.buildpool[i] = 0;
		}
	}

	// calc the player views now that all pushing
	// and damage has been added
	ent = g_edicts+1;
	do
	{
		if (ent->inuse)
			ClientEndServerFrame (ent);
		ent++;
	}
	while (ent->client);
}

/*
 * ClientEndServerFrame
 *
 * Called for each player at the end of the server frame
 * and right after spawning
 */
void DeathmatchScoreboard (edict_t *ent);
void ClientEndServerFrame (edict_t *ent)
{
	float	bobtime;
	int		i;

	current_player = ent;
	current_client = ent->client;

	/*
	 * If the origin or velocity have changed since ClientThink(),
	 * update the pmove values.	This will happen when the client
	 * is pushed by a bmodel or kicked by an explosion.
	 *
	 * If it wasn't updated here, the view position would lag a frame
	 * behind the body position when pushed -- "sinking into plats"
	 */
	for (i=0 ; i<3 ; i++)
	{
		current_client->ps.pmove.origin[i] = ent->s.origin[i]*8;
		current_client->ps.pmove.velocity[i] = ent->velocity[i]*8;
	}

	/*
	 * If the end of unit layout is displayed, don't give
	 * the player any normal movement attributes
	 */
	if (level.intermissiontime && (level.intermissiontime < level.framenum))
	{
		// FIXME: add view drifting here?
		/*current_client->ps.blend[3] = 0;
		current_client->ps.fov = 90;
		VectorCopy (level.intermission_origin, ent->s.origin);
		VectorCopy (level.intermission_angle, current_client->ps.viewangles);*/
		return;
	}

	AngleVectors (ent->client->v_angle, forward, right, up);

	// burn from lava, etc
	P_WorldEffects ();

	/*
	 * set model angles from view angles so other things in
	 * the world can tell which direction you are looking
	 */
	if (ent->client->v_angle[PITCH] > 180)
		ent->s.angles[PITCH] = (-360 + ent->client->v_angle[PITCH])/3;
	else
		ent->s.angles[PITCH] = ent->client->v_angle[PITCH]/3;
	ent->s.angles[YAW] = ent->client->v_angle[YAW];
	ent->s.angles[ROLL] = 0;
	ent->s.angles[ROLL] = SV_CalcRoll (ent->s.angles, ent->velocity)*4;

	// calculate speed and cycle to be used for all cyclic walking effects
	xyspeed = sqrt(ent->velocity[0]*ent->velocity[0] + ent->velocity[1]*ent->velocity[1]);

	for (i = 0; i < 4;i++) {
		ent->client->lastspeeds[i] = ent->client->lastspeeds[i+1];
	}
	ent->client->lastspeeds[4] = xyspeed;

	if (xyspeed < 5)
	{
		bobmove = 0;
		current_client->bobtime = 0;	// start at beginning of cycle again
	}
	else if (ent->groundentity)
	{	// so bobbing only cycles when on ground
		if (xyspeed > 210)
			bobmove = 0.25;
		else if (xyspeed > 100)
			bobmove = 0.125;
		else
			bobmove = 0.0625;
	}

	bobtime = (current_client->bobtime += bobmove);

	if (current_client->ps.pmove.pm_flags & PMF_DUCKED)
		bobtime *= 4;

	bobcycle = (int)bobtime;
	bobfracsin = fabs(sin(bobtime*M_PI));

	// detect hitting the floor
	P_FallingDamage (ent);

	// apply all the damage taken this frame
	P_DamageFeedback (ent);

	// determine the view offsets
	if (!ent->client->chase_target)
		SV_CalcViewOffset (ent);

	// determine the gun offsets
	SV_CalcGunOffset (ent);

	/* determine the full screen color blend
	 * must be after viewoffset, so eye contents can be accurately determined
	 * FIXME: with client prediction, the contents should be determined by the client
	 */
	SV_CalcBlend (ent);

	G_SetStats (ent);

	G_SetClientEvent (ent);

	G_SetClientEffects (ent);

	G_SetClientSound (ent);

	G_SetClientFrame (ent);

#ifdef R1Q2_BUILD
	if (basetest->value) {
		VectorCopy (ent->mins, ent->client->ps.mins);
		VectorCopy (ent->maxs, ent->client->ps.maxs);
	}
#endif

	VectorCopy (ent->velocity, ent->client->oldvelocity);
	VectorCopy (ent->client->ps.viewangles, ent->client->oldviewangles);

	// clear weapon kicks
	VectorClear (ent->client->kick_origin);
	VectorClear (ent->client->kick_angles);

	//ent->client->kick_angles[0] = 100;

	if (ent->client->resp.class_type == CLASS_EXTERM)
		ent->client->resp.flying = false;

	// if the scoreboard is up, update it
	if (level.framenum % 50 == 0 && ent->client->showscores)
	{
		// if it was opened on "update" frame, don't reupdate (and cause overflow)
		if (!ent->client->menu.entries && ent->client->showscores < level.framenum)
			DeathmatchScoreboard (ent);
		//} else if (ent->client->menu.entries == team_menu) {
		//	PMenu_Update(ent);
		//}

	}
}
