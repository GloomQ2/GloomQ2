/*Savvy Entity Bot
This is made for a bot which walks around, targets, and follows things near it.
It is not effiecent nor good yet.
Everything in this file was made by Savvy with little or no external help.
*/

#include "g_local.h"
/*for most of what we need !*/
#include "m_player.h"
/*for the frames*/

/*
enum {
SPBOT_FRAMES_IDLE,
SPBOT_FRAMES_RUN,
SPBOT_FRAMES_ATTACK,
SPBOT_FRAMES_PAIN1,
SPBOT_FRAMES_PAIN2,
SPBOT_FRAMES_PAIN3,
SPBOT_FRAMES_JUMP,
SPBOT_FRAMES_CROUCH_IDLE,
SPBOT_FRAMES_CROUCH_WALK,
SPBOT_FRAMES_CROUCH_ATTACK,
SPBOT_FRAMES_CROUCH_PAIN,
SPBOT_FRAMES_CROUCH_DEATH,
SPBOT_FRAMES_DEATH1,
SPBOT_FRAMES_DEATH2,
SPBOT_FRAMES_DEATH3,
SPBOT_MAX_FRAMES
};
*/

spbotani_t frameinfo[(SPBOT_MAX_FRAMES+1)] = {
{SPBOT_FRAMES_IDLE,FRAME_stand01,FRAME_stand40},
{SPBOT_FRAMES_RUN,FRAME_run1,FRAME_run6},
{SPBOT_FRAMES_ATTACK,FRAME_attack1,FRAME_attack8},
{SPBOT_FRAMES_PAIN1,FRAME_pain101,FRAME_pain104},
{SPBOT_FRAMES_PAIN2,FRAME_pain201,FRAME_pain204},
{SPBOT_FRAMES_PAIN3,FRAME_pain301,FRAME_pain304},
{SPBOT_FRAMES_JUMP,FRAME_jump1,FRAME_jump6},
{SPBOT_FRAMES_CROUCH_IDLE,FRAME_crstnd01,FRAME_crstnd19},
{SPBOT_FRAMES_CROUCH_WALK,FRAME_crwalk1,FRAME_crwalk6},
{SPBOT_FRAMES_CROUCH_ATTACK,FRAME_crattak1,FRAME_crattak9},
{SPBOT_FRAMES_CROUCH_PAIN,FRAME_crpain1,FRAME_crpain4},
{SPBOT_FRAMES_CROUCH_DEATH,FRAME_crdeath1,FRAME_crdeath5},
{SPBOT_FRAMES_DEATH1,FRAME_death101,FRAME_death106},
{SPBOT_FRAMES_DEATH2,FRAME_death201,FRAME_death206},
{SPBOT_FRAMES_DEATH3,FRAME_death301,FRAME_death308},
{0,0,0}
};

/*--------------- TARGETTING AND FIRING --------------------*/

int SPBotOkToFire(edict_t *b) {
  if (b->fly_sound_debounce_time > level.time) { return 1; }
return 0;
}

int SPBotGetFrameSet(edict_t *b) {
int i,frameset;
frameset = SPBOT_FRAMES_IDLE; /*default*/
  for (i = 0; i < SPBOT_MAX_FRAMES; i++) {
  /*if our current frame resides within the range, then this must be our break (i love puns)*/
    if ((b->s.frame >= frameinfo[i].startframe) && (b->s.frame <= frameinfo[i].endframe)) {
    frameset = i;
    break;
    }
  }
return frameset;
}

void SPBotSetFrame(edict_t *b, int frameset) {
int f,of;
int startf,endf;
of = b->s.frame;
f = of;
/*
  if (frameset == SPBOT_FRAMES_IDLE) {
    if (of >= FRAME_stand01) { f++; }
    else if (of => FRAME_stand40) { f = FRAME_stand01; }
    else { f = FRAME_stand01; }
  b->s.frame = f;
  }
*/
/*find the animation wanted so that we can react correctly*/

/*check for valid frameset, if it isn't valid, go with default*/
  if ((frameset < 0) || (frameset >= SPBOT_MAX_FRAMES)) {
  frameset = SPBOT_FRAMES_IDLE;
  }

/*bad index check*/
  if (frameinfo[frameset].frameset != frameset) { return; }

/*set start and end frames*/
startf = frameinfo[frameset].startframe;
endf = frameinfo[frameset].endframe;

/*find what frame it should be at*/
  if ((of >= startf) && (of < endf)) { f++; } /*just starting or in the center*/
  else if (of == endf) { f = startf; } /*at end frame or beyond it (SHOULD NOT BE BEYOND IT)*/
  else { f = startf; } /*at some other animation*/
/*set frame*/
b->s.frame = f;
}

void SPBotTargetOther(edict_t *b, edict_t *other) {
  if ((other) && (other->client) && (!b->enemy)) {
  /*no teamkilling*/
    if ((Q_stricmp(b->team,"alien") == 0) && (other->client->resp.team == TEAM_ALIEN)) { return; }
    else if ((Q_stricmp(b->team,"human") == 0) && (other->client->resp.team == TEAM_HUMAN)) { return; }
  b->enemy = other;
  }
}

void SPBotFire(edict_t *b) {
/*alien*/
  if (Q_stricmp(b->team,"alien") == 0) {
  //do melee here
  return;
  }
  else if (Q_stricmp(b->team,"human") == 0) {
  /*find the correct weapon for human and fire if at a correct frame*/
    switch (b->style) {
    case SPBOT_ATTACK_NONE:
      b->fly_sound_debounce_time = level.time + 300; /*makes it so this function is rarely called, and even when it is, does nothing*/
      break;
    case SPBOT_ATTACK_AUTOGUN:
      fire_piercing (b, b->s.origin, b->s.angles, TE_GUNSHOT, DEFAULT_AUTOGUN_HSPREAD, DEFAULT_AUTOGUN_VSPREAD, 50, 100, MOD_AUTOGUN);
      b->fly_sound_debounce_time = level.time + 0.25;
      break;
/*add more in
    case SPBOT_ATTACK_:
      b->fly_sound_debounce_time = level.time + 2;
      break;
*/
    case SPBOT_ATTACK_MECHROCKETS:
      fire_drunk_missile (b, b->s.origin, b->s.angles, 0, 1000, 10, 10);
      b->fly_sound_debounce_time = level.time + 2;
      break;
    }
  }
}

void SPBotHome(edict_t *b) {
edict_t *target;
vec3_t tdir;
vec_t vel;
vec3_t anglevelocity,velocity;

target = b->enemy;

/*disengage if you've lost sight of the enemy, so you don't run into walls*/
	if (target->health < 1 || !visible(b, target)) {
	b->enemy = NULL;
	return;
	}

/*avelocity: pitch, yaw, role (do we want it rotating?)*/
/*velocity: x, y, z x being fowards-back, y being left-right, and z being up-down*/
anglevelocity[0] = anglevelocity[1] = anglevelocity[2] = 0;

  if (target) {
  /*if human, charge blindly towards target - NEVER jump*/
    if (Q_stricmp(b->team,"human") == 0) {
    velocity[0] = SPBOT_HUMAN_FORWARDVELOCITY; /* + forwards - back*/
    velocity[1] = 0; /* + right - left*/
    velocity[2] = SPBOT_HUMAN_JUMP_VELOCITY; /*velocity for jump, if jump is scheduled*/
    }
    else if (Q_stricmp(b->team, "alien") == 0) {
    velocity[0] = SPBOT_ALIEN_FORWARDVELOCITY; /* + forwards - back*/
    velocity[1] = 0; /* + right - left*/
    velocity[2] = SPBOT_ALIEN_JUMP_VELOCITY; /*velocity for jump, if jump is scheduled*/
    }
  /*for both teams*/
    if (b->random > 0) {
    /*NOT scheduled for jump*/
    velocity[2] = 0;
    }
  }
  else {
  /*since we don't have a target, stand still*/
  velocity[0] = velocity[1] = velocity[2] = 0;
  }

/*set velocities*/
VectorCopy(anglevelocity,b->avelocity);
VectorCopy(velocity,b->velocity);

/*calculate angles*/
VectorSubtract(target->s.origin, b->s.origin, tdir);

/*set angles*/
VectorAdd(tdir, b->movedir, tdir);
VectorNormalize(tdir);
VectorCopy(tdir, b->movedir);
vectoangles(tdir, b->s.angles);

/*set velocity*/
vel = VectorLength(b->velocity);
VectorScale(tdir, vel, b->velocity);

#ifdef SPBOT_HUMAN_JUMP
  if (Q_stricmp(b->team,"human") == 0) {
    if (b->random <= 0) { b->random = SPBOT_HUMAN_JUMP_DELAY; }
    else { b->random--; }
  }
#endif
#ifdef SPBOT_ALIEN_JUMP
  if (Q_stricmp(b->team,"alien") == 0) {
    if (b->random <= 0) { b->random = SPBOT_ALIEN_JUMP_DELAY; }
    else { b->random--; }
  }
#endif
}

/*--------------- TOUCH AND USE ---------------*/

void SPBotTouch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf) {
vec3_t normal, normalnormal;
  if (plane) {
  VectorCopy(plane->normal,normal);
  VectorCopy(normal,normalnormal);
  VectorInverse(normalnormal);
  }

/*don't continue if other ent is not a client*/
  if ((!other) || (!other->client)) { return; }

/*if the other ent is defined and is a client, then check to see if we can attack
bot must be the opposite team of said enemy
*/
  if ((Q_stricmp(self->team, "human") == 0) && (other->client->resp.team == TEAM_ALIEN)) {
  SPBotTargetOther(self, other);
  }
  else if ((Q_stricmp(self->team, "alien") == 0) && (other->client->resp.team == TEAM_HUMAN)) {
  SPBotTargetOther(self, other);
    if (self->volume >= level.time) {
    T_Damage(other, self, self, normal, other->s.origin, normalnormal, self->dmg, 0, (DAMAGE_IGNORE_STRUCTS|DAMAGE_RADIUS|DAMAGE_PIERCING), MOD_SPBOT);
    self->volume = level.time + SPBOT_ALIEN_TOUCH_ATTACK_INTERVAL;
    }
  self->volume -= 1.00f; /*make sure it is even, so we can call it 'seconds' instead of having a floating point [pound]define*/
  }
}

void SPBotUse(edict_t *self, edict_t *other, edict_t *activator) {
  if (self->think) { self->think = NULL; self->nextthink = 0; }
  else { self->think = SPBotThink; self->nextthink = level.time + SPBOT_RESTART_DELAY; }
}

/*--------------- PAIN AND DEATH ---------------*/

/*target attacker - ONLY IF other->client !!!*/
void SPBotPain(edict_t *self, edict_t *other, float kick, int damage) {
SPBotTargetOther(self, other);
}
/*do death animations*/
void SPBotDie(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point) {
int d; /*what death?*/
/*a very stylish death - i crack myself up*/
d = (int)(random() * 2);
  if (d == 0) { self->style = SPBOT_FRAMES_DEATH1; }
  if (d == 1) { self->style = SPBOT_FRAMES_DEATH2; }
  if (d == 2) { self->style = SPBOT_FRAMES_DEATH3; }

self->think = SPBotDeathThink;
self->nextthink = level.time + 0.1f;
}

/*--------------- THINKING ---------------*/

void SPBotDeathThink (edict_t *b) {
/*check if last frame of death sequence*/
  if (b->s.frame == frameinfo[b->style].endframe) {
  b->think = G_FreeEdict; /*remove ent now that we've died properly*/
  }

/*do a frame*/
SPBotSetFrame(b, b->style);
/*set time for next frame*/
b->nextthink = level.time + 0.1f;
}

void SPBotThink(edict_t *b) {
edict_t *e;

/*bot has no target, so it just turns in circles
  if (!b->enemy) {
    if (b->delay == 1) {
    if (s.angles == 360) { b->s.angles[0] = 0; }
    else { b->s.angles[0]++; }
    }
    else if (b->delay == 2) {
    if (s.angles == 0) { b->s.angles[0] = 360; }
    else { b->s.angles[0]--; }
    }
  }
*/

/*if no enemy, our current enemy is dead, not visable, or too far away, then it is our duty to find players of the opposite team near us and attack them*/
	if (!b->enemy || b->enemy->health < 1 || !visible(b, b->enemy) || Distance (b->enemy->s.origin, b->s.origin) > SPBOT_TARGET_DISTANCE){
	int otherteam;
	b->enemy = e = NULL;
		if (Q_stricmp(b->team,"alien") == 0) { otherteam = TEAM_HUMAN; }
		else if (Q_stricmp(b->team,"human") == 0) { otherteam = TEAM_ALIEN; }
		else { return; } /*SHOULD NEVER GET HERE, THIS IS OBSERVER OR SOME WIERD UNKNOWN TEAM*/

		while ((e = findradius_c(e, b, SPBOT_TARGET_DISTANCE)) != NULL) {
			if ((e->client->resp.team == otherteam) && (visible(b, e))) {
			b->enemy = e;
			break;
			}
		}
	}

  if (b->enemy) {
  SPBotHome(b);/*this moves the bot*/
  /*fire if bot has a target, should be facing or trying to face the player*/
    if (SPBotOkToFire(b)) {
    SPBotFire(b);
    SPBotSetFrame(b, SPBOT_FRAMES_ATTACK);
    }
  /*tell the enemy we are targeting them !*/
  temp_line (TE_BFG_LASER,b->s.origin,b->enemy->s.origin);
  gi.unicast(b->enemy,false);
  }
  else {
  /*if no target, stop dead (just a figure of speech)
  will do it normally
  VectorClear(b->velocity);
  */
  }

  /*animations
  check to see if we are attacking
  check for upwards velocity, if so, do jump animations
  if no upwards velocity, then check of we are moving normally, if so, do run animations
  if no velocity at all, then run idle animations
  */
  if (SPBotGetFrameSet(b) == SPBOT_FRAMES_ATTACK) { }
  else if (b->velocity[2] > 0) { SPBotSetFrame(b, SPBOT_FRAMES_JUMP); }
  else if ((b->velocity[0] > 0) || (b->velocity[1] > 0)) { SPBotSetFrame(b, SPBOT_FRAMES_RUN); }
  else { SPBotSetFrame(b, SPBOT_FRAMES_IDLE); }


b->nextthink = level.time + SPBOT_THINK;
}

/*-------------------- SPAWNING --------------------*/

void SPBotHumanWeapon(edict_t *b, int bot_class) {
	switch (bot_class) {
	case CLASS_GRUNT:
		b->s.modelindex2 = gi.modelindex("players/male/autogun.md2");
		break;
	case CLASS_HEAVY:
		b->s.modelindex2 = gi.modelindex("players/hsold/weapon.md2");
		break;
	case CLASS_COMMANDO:
		b->s.modelindex2 = gi.modelindex("players/male/smg.md2");
		break;
	case CLASS_SHOCK:
		b->s.modelindex2 = gi.modelindex("players/male/shotgun.md2");
		break;
	case CLASS_BIO:
		b->s.modelindex2 = gi.modelindex("players/female/weapon.md2");
		break;
	default:
		break;
	}
}

void SpawnSPBot(edict_t *owner, vec3_t origin, vec3_t angles, int team) {
edict_t *b;
vec3_t mins;
vec3_t maxs;
int bot_class;

/*
grunt:
  mins[0] = -16;
  mins[1] = -16;
  mins[2] = -24;
  maxs[0] = 16;
  maxs[1] = 16;
  maxs[2] = 32;
hatchling:
  mins[0] = -16;
  mins[1] = -16;
  mins[2] = -24;
  maxs[0] = 16;
  maxs[1] = 16;
  maxs[2] = 8;
*/

/*spawn a simple ent*/
b = G_Spawn();

/*first thing we do, is make sure that it IS NOT CONSIDERED A PLAYER
this bot has it's own functions for everything, and is made to chase after an opponent firing
*/

b->client = NULL;

  if (team == TEAM_HUMAN) {
  /*grunt-specific*/
  bot_class = CLASS_GRUNT;
  b->item = FindItem("Autogun");
//  b->s.modelindex2 = gi.modelindex(itemlist[ITEM_INDEX(b->item)].view_model); /*autogun active model*/
  b->team = "human";
  b->style = SPBOT_ATTACK_AUTOGUN;
  /*this sets the human weapon in place*/
  SPBotHumanWeapon(b, bot_class);
  }
  else if (team == TEAM_ALIEN) {
  /*hatchling-specific*/
  bot_class = CLASS_HATCHLING;
  b->team = "alien";
  b->style = SPBOT_ATTACK_MELEE;
  }
  else {
  /*should never get here - BAD TEAM VALUE*/
  return;
  }

b->spawnflags = bot_class;

/*bbox and models*/
VectorCopy(classlist[bot_class].mins,mins);
VectorCopy(classlist[bot_class].maxs,maxs);
b->model = classlist[bot_class].model;
b->s.modelindex = gi.modelindex(b->model);
b->s.modelindex2 = 0;

/*set position and initial angles*/
VectorCopy(angles,b->s.angles);
VectorCopy(origin,b->s.origin);

/*health and gib values*/
b->health = classlist[bot_class].health;
b->max_health = classlist[bot_class].health;
b->gib_health = classlist[bot_class].gib_health;
/*set the gibs if there are any*/
	if (classlist[bot_class].headgib) { b->headgib = gi.modelindex(classlist[bot_class].headgib); }
	if (classlist[bot_class].gib1) { b->gib1 = gi.modelindex(classlist[bot_class].gib1); }
	if (classlist[bot_class].gib2) { b->gib2 = gi.modelindex(classlist[bot_class].gib2); }

/*clip / dead / hurt*/
b->solid = SOLID_BBOX;
b->clipmask = MASK_PLAYERSOLID;
b->hurtflags = 0;
b->takedamage = DAMAGE_AIM;
b->deadflag = DEAD_NO;

/*make sure it is viewed correctly*/
b->viewheight = classlist[bot_class].viewheight;

/*various timers*/
//b->wait = 0;
b->fly_sound_debounce_time = level.time + SPBOT_STARTDELAY; //wait SPBOT_STARTDELAY seconds after spawning before it can attack
b->random = 0; //just right off, so it starts the timer (if there is a jump)

/*gravity and physics*/
b->gravity = sv_gravity->value;
b->movetype = MOVETYPE_STEP;

/*other random values*/
b->dmg = classlist[bot_class].damage;
b->radius_dmg = 0;
b->dmg_radius = 0;

/*make visable in infared*/
b->s.renderfx |= RF_IR_VISIBLE;

/*make the game recognize this as an entity*/
b->inuse = true;
b->classname = "spbot";

b->enttype = ENT_SPBOT;

/*set functions
prethink blocked
think touch use pain die
*/

b->touch = SPBotTouch;
b->use = SPBotUse;
b->pain = SPBotPain;
b->die = SPBotDie;

b->think = SPBotThink;
b->nextthink = level.time + SPBOT_STARTDELAY;

/*start*/
gi.linkentity(b);
}

/*stuff we should work with
s
mins, maxs
solid
clipmask
owner
movetype
health
max_health
gib_health
hurtflags
takedamage
prethink
think
blocked
touch
use
pain
die
*/
/*stuff we can to work with
	entity_state_t	s;
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
THIS SHIT IS FOR MAPS? = FREE CHARACTER SPOTS
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
MORE SHIT FOR MAPS!
	edict_t		*target_ent;
	float		timestamp;
	float		angle;			// set in qe3, -1 = up, -2 = down
	float		speed, accel, decel;
	vec3_t		movedir;
	vec3_t		pos1, pos2;
	vec3_t		velocity;
	vec3_t		avelocity;
BACK TO NORMAL?
	float		air_finished;
	float		gravity;
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
*/