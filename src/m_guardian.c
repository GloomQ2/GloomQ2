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

void GuardianStart(edict_t *ent)
{
	if (spiketest->value)
		ent->client->resp.inventory[ITEM_SPIKESPORE] = 1;
	else
		ent->client->resp.inventory[ITEM_SPIKESPORE] = 2;

	ent->client->resp.selected_item = ITEM_SPIKESPORE;
	ent->client->last_move_time = level.time + 3;

	if (spiketest->value)
		ent->client->healwait = 120;
}

int GuardianHeal(edict_t *ent, edict_t *healer)
{
	float wait = 0;

	ent->client->last_move_time = level.time + 1;

	if (ent->client->healwait > level.time) {
		if (spiketest->value) {
			if (ent->client->resp.inventory[ITEM_SPIKESPORE])
				return 2;
		} else {
			if (ent->client->resp.inventory[ITEM_SPIKESPORE] == 2)
				return 2;
		}

		return 0;
	}

	if (spiketest->value) {
		if (ent->client->resp.inventory[ITEM_SPIKESPORE])
			return 2;
	} else {
		if (ent->client->resp.inventory[ITEM_SPIKESPORE] == 2)
			return 2;
	}

	if (spiketest->value) {
		wait = 120;
		ent->client->resp.inventory[ITEM_SPIKESPORE] = 1;
	} else {
		wait = (2-(float)ent->client->resp.inventory[ITEM_SPIKESPORE])*30;
		ent->client->resp.inventory[ITEM_SPIKESPORE] = 2;
	}

	ent->client->resp.selected_item = ITEM_SPIKESPORE;
	ent->client->healwait = wait;

	return 1;
}
