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
#include "m_mech.h"

/*Savvy custom, easier to balance
MAX_MECH_ROUNDS used to be 125 with the define
MECH_REPAIR_DELAY used to be 5 without the define
MECH_REPAIR_ROUNDS used to be 20 without the define
MECH_ROCKETS used to be 1 without the define
*/

void MechStart(edict_t *ent) {
ent->client->resp.inventory[ITEM_AMMO_MECHCANNON] = MAX_MECH_ROUNDS;
ent->client->ammo_index = ITEM_AMMO_MECHCANNON;
//r1: ugly variable hijacking for mech armor test
ent->client->last_reload_time = (float)classlist[CLASS_MECH].armorcount;
ent->client->resp.selected_item = 0;
/*Savvy old mechrocket system
ent->client->resp.inventory[ITEM_AMMO_MINIMISSILES] = MECH_MISSILE_COUNT;
*/
ent->client->resp.inventory[ITEM_AMMO_MINIMISSILES] = 0;

ent->client->ps.gunindex = gi.modelindex("models/weapons/v_mech/tris.md2");
ent->client->ps.gunframe = 5;
}

int MechHeal(edict_t *ent, edict_t *healer) {
int i;
const gitem_t *item;
//gitem_armor_t	*newinfo;
float wait;
int armor = 0;
char *msg = NULL;

	if (healer && healer->client) {
		if (healer->client->resp.class_type == CLASS_ENGINEER) {
		ent->wait = level.time + 3;
		armor = RepairArmor (ent, NULL, 25, 0);
			if (armor != -1) { msg = va("Report: Armor at %d", armor); }
      else { msg = va("Report: Armor full"); }
		}
    else if (healer->client->resp.class_type == CLASS_BIO){
		HealbyBiotech(ent,healer);
		return 2;
		}
	}

	if (ent->client->resp.inventory[ITEM_INDEX(FindItem("Cannon Rounds"))] == MAX_MECH_ROUNDS &&
		(!(healer && healer->client)) &&
		(((ent->client->resp.upgrades & UPGRADE_DEPOT_INTERFACE) && GetArmor (ent) == classlist[ent->client->resp.class_type].armorcount) ||
		!(ent->client->resp.upgrades & UPGRADE_DEPOT_INTERFACE))
		) { return 2; }

	if (ent->client->healwait > level.time && !(healer && healer->client)) { return 0; }

	if ((healer && healer->client) &&
	ent->client->resp.inventory[ITEM_INDEX(FindItem("Cannon Rounds"))] == MAX_MECH_ROUNDS) {
		if (armor == -1) {
		strcat(msg,", repairs complete.");
		gi.sound (ent, CHAN_AUTO, SoundIndex (misc_keytry), 1, ATTN_IDLE, 0);
		}
	gi.cprintf(healer, PRINT_HIGH, "%s\n",msg);
	return 2;
	}

item = FindItem("Cannon Rounds");	
i = ITEM_INDEX(item);
wait = (MAX_MECH_ROUNDS - ent->client->resp.inventory[i]) * 0.2f;
	if (wait > 4.0) { wait = 4.0; }

ent->client->resp.inventory[i]+= MECH_REPAIR_ROUNDS;

	if ((!healer || !healer->client) && (ent->client->resp.upgrades & UPGRADE_DEPOT_INTERFACE)) {
		if (RepairArmor (ent, NULL, 50, 0) != -1) { wait += MECH_REPAIR_DELAY; }
	}

	if (ent->client->resp.inventory[i] > MAX_MECH_ROUNDS) { ent->client->resp.inventory[i] = MAX_MECH_ROUNDS; }
	
	if (!healer || !healer->client) {
	ent->client->healwait=wait+level.time;
	ent->wait = level.time + MECH_REPAIR_DELAY;
	}

  if (healer && healer->client) { gi.cprintf(healer, PRINT_HIGH, "%s\n",msg); }

return 1;
}


void MechDie(edict_t *self) {
	vec3_t start;
								
	self->client->ps.gunindex = 0;

	VectorCopy(self->s.origin, start);
	start[2] += 25;

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_EXPLOSION1);
	gi.WritePosition (start);
	gi.multicast (start, MULTICAST_PVS);

	gi.sound (self, CHAN_AUTO, gi.soundindex ("*fall1.wav"), 1, ATTN_NORM, 0);
}
