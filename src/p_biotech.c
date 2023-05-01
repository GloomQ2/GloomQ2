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

/*Savvy edited so that biotechs can have "battletech pack"s which will boost armor, and allow more nade room*/

void BiotechStart(edict_t *ent) {
const gitem_t *item;
int i;

ent->client->resp.inventory[ITEM_AMMO_CELLS]=BIOTECH_MAX_CELLS;

item = FindItem("Pistol");
i = ITEM_INDEX(item);
ent->client->resp.inventory[i] = 1;

item = FindItem("9mm Clip");
i = ITEM_INDEX(item);
ent->client->resp.inventory[i] = BIOTECH_AMMO_9MM_CLIPS;
ent->client->resp.inventory[ITEM_INDEX(FindAmmo(item))] = item->quantity;

item = FindItem("Flash Grenade");
i = ITEM_INDEX(item);
ent->client->resp.inventory[i] = BIOTECH_AMMO_FLASH_GRENADES;

item = FindItem("Flare");
i = ITEM_INDEX(item);
ent->client->resp.inventory[i] = BIOTECH_AMMO_FLARES;
ent->client->resp.selected_item = i;

item = FindItem("Health");
i = ITEM_INDEX(item);
ent->client->resp.inventory[i] = BIOTECH_AMMO_HEALTH;

item = FindItem("Scattergun");
i = ITEM_INDEX(item);
ent->client->resp.inventory[i] = 1;
ent->client->weapon = item;

item = FindItem("Scattershot Clips");
i = ITEM_INDEX(item);
ent->client->resp.inventory[i] = 0; //keep 0
ent->client->resp.inventory[ITEM_INDEX(FindAmmo(item))] = item->quantity;
ent->client->ammo_index = i;

ent->random = 0;

// give bio ir ability
ent->client->ps.rdflags |= RDF_IRGOGGLES;
}

/*Savvy edited biotech for special battletech upgrade, which edits inventory, so depot etc functions must change :)*/
int BiotechHeal (edict_t *ent, edict_t *healer) {
const gitem_t *item;
int i,armor,max_scattershot_clips,max_flash_grenades,max_flares,max_9mm_clips,max_health;
float wait;
char *msg;

wait = 0;
armor = 0;
msg = NULL;

/*Savvy start inventory elements*/
max_scattershot_clips = BIOTECH_AMMO_SCATTERHOT_CLIPS;
max_flash_grenades = BIOTECH_AMMO_FLASH_GRENADES;
max_flares = BIOTECH_AMMO_FLARES;
max_9mm_clips = BIOTECH_AMMO_9MM_CLIPS;
max_health = BIOTECH_AMMO_HEALTH;

/*add what else there is*/
  if (ent->client->resp.upgrades & UPGRADE_BATTLETECH) {
  max_scattershot_clips += BIOTECH_AMMO_BATTLETECH_SCATTERHOT_CLIPS_ADD;
  max_flash_grenades += BIOTECH_AMMO_BATTLETECH_FLASH_GRENADES_ADD;
  max_flares += BIOTECH_AMMO_BATTLETECH_FLARES_ADD;
  max_9mm_clips += BIOTECH_AMMO_BATTLETECH_9MM_CLIPS_ADD;
  max_health += BIOTECH_AMMO_BATTLETECH_HEALTH_ADD;
  max_flares += BIOTECH_AMMO_BATTLETECH_FLARES_ADD;
  }
  if (ent->client->resp.upgrades & UPGRADE_FLARE_PACK) { max_flares += BIOTECH_FLARE_PACK_ADD; }

//gitem_armor_t	*newinfo;
/*Savvy had to move to top of function, makes no sense to declare here
float wait = 0;
int armor = 0;
char *msg = NULL;
*/

	if(healer && healer->client) {
		if (healer->client->resp.class_type == CLASS_ENGINEER) {
			armor = RepairArmor (ent, NULL, 25, 0);
			if (armor != -1)
				msg = va("Report: Armor at %d", armor);
			else
				msg = va("Report: Armor full");
		}else if (healer->client->resp.class_type==CLASS_BIO){
			HealbyBiotech(ent,healer);
			return 2;
		}	
	}

	if ((ent->client->resp.inventory[ITEM_INDEX(FindItem("Scattershot"))] != 0 ||
		ent->client->resp.inventory[ITEM_INDEX(FindItem("Scattershot Clips"))] == max_scattershot_clips) &&
		ent->client->resp.inventory[ITEM_INDEX(FindItem("Flash Grenade"))] == max_flash_grenades &&
		ent->client->resp.inventory[ITEM_AMMO_SCATTERSHOT] == CLIP_SCATTERSHOT_VALUE &&
		ent->client->resp.inventory[ITEM_INDEX(FindItem("Flare"))] == max_flares &&
		( (ent->client->resp.inventory[ITEM_INDEX(FindItem("9mm Clip"))] == max_9mm_clips &&
		  ent->client->resp.inventory[ITEM_INDEX(FindItem("Bullets"))] == 0) ||
      (ent->client->resp.inventory[ITEM_INDEX(FindItem("9mm Clip"))] == 1 &&
		  ent->client->resp.inventory[ITEM_INDEX(FindItem("Bullets"))] != 0)
    ) &&
		ent->client->resp.inventory[ITEM_AMMO_CELLS] == BIOTECH_MAX_CELLS &&
		ent->client->resp.inventory[ITEM_INDEX(FindItem("Health"))] == max_health &&
		( !(healer && healer->client)))
		return 2;

	if (ent->client->healwait > level.time && !(healer && healer->client))
	return 0;

	if ((healer && healer->client) && 
		(ent->client->resp.inventory[ITEM_INDEX(FindItem("Scattershot"))] != 0 ||
		ent->client->resp.inventory[ITEM_INDEX(FindItem("Scattershot Clips"))] == 1) &&
		//ent->client->resp.inventory[ITEM_INDEX(FindItem("Flash Grenade"))] == 1 &&
		//ent->client->resp.inventory[ITEM_INDEX(FindItem("Flare"))] == 4 &&
		((ent->client->resp.inventory[ITEM_INDEX(FindItem("9mm Clip"))] == 2 &&
		ent->client->resp.inventory[ITEM_INDEX(FindItem("Bullets"))] == 0) ||(
		ent->client->resp.inventory[ITEM_INDEX(FindItem("9mm Clip"))] == 1 &&
		ent->client->resp.inventory[ITEM_INDEX(FindItem("Bullets"))] != 0)) &&
		ent->client->resp.inventory[ITEM_AMMO_CELLS] == BIOTECH_MAX_CELLS) {
		if (armor == -1) {
		strcat(msg,", repairs complete.");
		gi.sound (ent, CHAN_AUTO, SoundIndex (misc_keytry), 1, ATTN_IDLE, 0);
		}
	gi.cprintf(healer, PRINT_HIGH, "%s\n",msg);	
	return 2;
	}

	if (!healer || !healer->client) {
	item = FindItem("Health");
	i = ITEM_INDEX(item);
	wait=(max_health - ent->client->resp.inventory[i]) * 10;
	ent->client->resp.inventory[i] = max_health;
	}

	if (ent->client->resp.inventory[ITEM_INDEX(FindItem("Scattershot"))] == 0) {
	item = FindItem("Scattershot Clips");
	i = ITEM_INDEX(item);
	wait+=(max_scattershot_clips - ent->client->resp.inventory[i]) * 20;
	ent->client->resp.inventory[i] = max_scattershot_clips;
	}

	if (!healer || !healer->client) {
	item = FindItem("Flash Grenade");
	i = ITEM_INDEX(item);
	wait += (max_flash_grenades - ent->client->resp.inventory[i]) * 10;
	ent->client->resp.inventory[i] = max_flash_grenades;

	item = FindItem("Flare");
	i = ITEM_INDEX(item);
	wait += (max_flares - ent->client->resp.inventory[i]) * 2; //*4
	ent->client->resp.inventory[i] = max_flares;
	}

item = FindItem("9mm Clip");
i = ITEM_INDEX(item);

	if (ent->client->resp.inventory[ITEM_INDEX(FindItem("Bullets"))] != 0) {
	wait+=(1-ent->client->resp.inventory[i])*5;
	ent->client->resp.inventory[i] = 1;
	} else {
	wait+=(max_9mm_clips-ent->client->resp.inventory[i])*5;
	ent->client->resp.inventory[i] = max_9mm_clips;
	}

	if (!ent->client->resp.inventory[ITEM_AMMO_SCATTERCLIP]) {
	wait += (CLIP_SCATTERSHOT_VALUE - ent->client->resp.inventory[ITEM_AMMO_SCATTERSHOT]);
	ent->client->resp.inventory[ITEM_AMMO_SCATTERSHOT] = CLIP_SCATTERSHOT_VALUE;
	}

wait += (BIOTECH_MAX_CELLS - ent->client->resp.inventory[ITEM_AMMO_CELLS]) / 15;
ent->client->resp.inventory[ITEM_AMMO_CELLS] = BIOTECH_MAX_CELLS;

	if (!healer || !healer->client)
	ent->client->healwait=wait+level.time;

	if (healer && healer->client)
	gi.cprintf(healer, PRINT_HIGH, "%s\n",msg);

return 1;
}

// FIXME: implement this in GenericVoice
void BiotechVoice(edict_t *self, int n)
{
	switch(n)
	{
	case VOICE_SPAWN:
#ifdef CACHE_CLIENTSOUNDS
		teamcastSound(TEAM_HUMAN, SoundIndex(biotech_spawn), 0.7);
#else
		teamcastSound(TEAM_HUMAN, gi.soundindex("voice/biotech/spawn1.wav"), 0.7);
#endif
		break;
	case VOICE_HELP:
#ifdef CACHE_CLIENTSOUNDS
		teamcastSound(TEAM_HUMAN, SoundIndex(biotech_voice1+(randomMT()%3)), 1.0);
#else
		teamcastSound(TEAM_HUMAN, gi.soundindex(va("voice/biotech/help%i.wav", (randomMT()%3)+1)), 1.0);
#endif
		break;
	case VOICE_ORDER:
#ifdef CACHE_CLIENTSOUNDS
		gi.sound (self, CHAN_AUTO, SoundIndex(biotech_voice4), 1, ATTN_NORM, 0);
#else
		gi.sound (self, CHAN_AUTO, gi.soundindex("voice/biotech/order1.wav"), 1, ATTN_NORM, 0);
#endif
		break;
	case VOICE_AFFIRM:
#ifdef CACHE_CLIENTSOUNDS
		gi.sound (self, CHAN_AUTO, SoundIndex(biotech_voice5), 1, ATTN_NORM, 0);
#else
		gi.sound (self, CHAN_AUTO, gi.soundindex("voice/biotech/affirm1.wav"), 1, ATTN_NORM, 0);
#endif
		break;
	case VOICE_DEFEND:
#ifdef CACHE_CLIENTSOUNDS
		gi.sound (self, CHAN_AUTO, SoundIndex(biotech_voice6), 1, ATTN_NORM, 0);
#else
		gi.sound (self, CHAN_AUTO, gi.soundindex("voice/biotech/defend1.wav"), 1, ATTN_NORM, 0);
#endif
		break;
	case VOICE_NOAMMO:
#ifdef CACHE_CLIENTSOUNDS
		gi.sound (self, CHAN_AUTO, SoundIndex(biotech_voice7), 1, ATTN_NORM, 0);
#else
		gi.sound (self, CHAN_AUTO, gi.soundindex("voice/biotech/noammo1.wav"), 1, ATTN_NORM, 0);
#endif
		break;
	case VOICE_TAUNT:
#ifdef CACHE_CLIENTSOUNDS
		gi.sound (self, CHAN_AUTO, SoundIndex(biotech_voice8+(randomMT()&1)), 1, ATTN_NORM, 0);
#else
		gi.sound (self, CHAN_AUTO, gi.soundindex(va("voice/biotech/taunt%i.wav", (randomMT()&1)+1)), 1, ATTN_NORM, 0);
#endif
		break;
	}
}
