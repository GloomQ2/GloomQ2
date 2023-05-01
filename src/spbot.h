/*Savvy Entity Bot
This is made for a bot which walks around, targets, and follows things near it.
It is not effiecent nor good yet.
Everything in this file was made by Savvy with no external help.
Note: since GPL forbids copyright, just note this is my own work (savvy)
*/

#define SPBOT_THINK 0.1
#define SPBOT_STARTDELAY 5
#define SPBOT_RESTART_DELAY 3
#define SPBOT_TARGET_DISTANCE 500

/*human specific*/
#define SPBOT_HUMAN_FORWARDVELOCITY 300
#define SPBOT_HUMAN_JUMP
  #define SPBOT_HUMAN_JUMP_DELAY 60
  #define SPBOT_HUMAN_JUMP_VELOCITY 250

/*alien specific*/
#define SPBOT_ALIEN_TOUCH_ATTACK_INTERVAL 10
#define SPBOT_ALIEN_FORWARDVELOCITY 500
#define SPBOT_ALIEN_JUMP
  #define SPBOT_ALIEN_JUMP_DELAY 30
  #define SPBOT_ALIEN_JUMP_VELOCITY 400

enum {
SPBOT_ATTACK_NONE, //just annoying following, lol
SPBOT_ATTACK_MELEE,
SPBOT_ATTACK_ACID,//wraith spit
SPBOT_ATTACK_SPIT,//drone spit
SPBOT_ATTACK_FIRE,
SPBOT_ATTACK_SPIKES,
SPBOT_ATTACK_FRAG_GRENADE,
SPBOT_ATTACK_SMOKE_GRENADE,
SPBOT_ATTACK_FLASH_GRENADE,
SPBOT_ATTACK_SPIKE_SPORE,
SPBOT_ATTACK_GAS_SPORE,
SPBOT_ATTACK_AUTOGUN,
SPBOT_ATTACK_SUBMACHINEGUN,
SPBOT_ATTACK_ROCKET_LAUNCHER,
SPBOT_ATTACK_MAGNUM,
SPBOT_ATTACK_PISTOL,
SPBOT_ATTACK_POKE,
SPBOT_ATTACK_SCATTERGUN,
SPBOT_ATTACK_AUTOCANNON,
SPBOT_ATTACK_SHOTGUN,
SPBOT_ATTACK_EXPLOSIVE_SHELLS,
SPBOT_ATTACK_EXPLOSIVE_MAGNUM_ROUNDS,
SPBOT_ATTACK_MECHROCKETS
};

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

/*animation information*/
typedef struct spbotani_s {
int frameset;
int startframe;
int endframe;
} spbotani_t;


int SPBotOkToFire(edict_t *b);

int SPBotGetFrameSet(edict_t *b);
void SPBotSetFrame(edict_t *b, int frameset);

void SPBotTargetOther(edict_t *b, edict_t *other);
void SPBotFire(edict_t *b);

void SPBotHome(edict_t *b);

void SPBotTouch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);
void SPBotUse(edict_t *self, edict_t *other, edict_t *activator);

void SPBotPain(edict_t *self, edict_t *other, float kick, int damage);
void SPBotDie(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);

void SPBotDeathThink (edict_t *b);
void SPBotThink(edict_t *b);

void SPBotHumanWeapon(edict_t *b, int bot_class);
void SpawnSPBot(edict_t *owner, vec3_t origin, vec3_t angles, int team);