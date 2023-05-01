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

void ChaseNext(edict_t *ent)
{
	edict_t *e;

	if (!ent->client->chase_target)
		return;

	e = ent->client->chase_target;
	do {
		e++;
		if (!e->client)
			e = g_edicts+1;
		if (!e->inuse)
			continue;
		if (e->solid != SOLID_NOT)
			break;
	} while (e != ent->client->chase_target);

	ent->client->chase_target = e;
	e->client->resp.chased = true;
}

//FIXME: when in chase overhead mode, if target is looking down and jumps, view
//       clips out of the map if their head touches the ceiling.

//FIXME: when chasing someone who disconnects chasecam should find another suitable
//       target and only drop to observer if one isn't found.

void UpdateChaseCam(edict_t *ent)
{
	vec3_t	targorigin, angles, goal, goaldir, temp;
	vec3_t	forward, right;
	edict_t	*targ;
	trace_t	trace;
	int	i;

	targ = ent->client->chase_target;
	if (targ != ent->client->last_chase_target) {
		ent->client->last_chase_target = targ;

		// vary looking position depending on some classes
		switch (targ->client->resp.class_type) {
			case CLASS_BREEDER:
				ent->client->chase_height = 60;
				ent->client->chase_dist = 100;
				break;
			case CLASS_STALKER:
				ent->client->chase_height = 25;
				ent->client->chase_dist = 80;
				break;
			case CLASS_ENGINEER:
				ent->client->chase_height = 30;
				ent->client->chase_dist = 80;
				break;
			case CLASS_MECH:
				ent->client->chase_height = 50;
				ent->client->chase_dist = 105;
				break;
			default:
				ent->client->chase_height = 20;
				ent->client->chase_dist = 75;
				break;
		}

	}

	targ->client->resp.chased = true;

	VectorCopy(targ->s.origin, targorigin);

	targorigin[2] += targ->viewheight;

	// copy target view angles to temp variable
	VectorCopy(targ->client->v_angle, angles);

	ent->client->ps.fov = 90;

	switch (ent->client->chase_view){
	case 1:   //    chase
		// do a smooth movement feedback on pitch
		angles[PITCH] /= 2;

		// convert angles to vectors
		AngleVectors (angles, forward, right, NULL);

		// calc view pos
		VectorMA(targorigin, -ent->client->chase_dist, forward, goal);

		// up it
		goal[2] += ent->client->chase_height;

		// sub o from targorigin, creating a dir vector
		VectorSubtract(targorigin, goal, goaldir);

		break;
	case 2:  //     right side
		AngleVectors (angles, forward, right, NULL);
		VectorMA(targorigin, ent->client->chase_dist, right, goal);
		goal[2] += ent->client->chase_height;
		VectorSubtract(targorigin,goal,goaldir);
		break;
	case 3:  //     front side
		angles[PITCH] = 0;
		AngleVectors (angles, forward, right, NULL);
		VectorMA(targorigin, ent->client->chase_dist, forward, goal);
		goal[2] += ent->client->chase_height;
		VectorSubtract(targorigin,goal,goaldir);
		break;
	case 4:  //     left side
		AngleVectors (angles, forward, right, NULL);
		VectorMA(targorigin, -ent->client->chase_dist, right, goal);
		goal[2] += ent->client->chase_height;
		VectorSubtract(targorigin,goal,goaldir);
		break;
	default:
		// eyes
		AngleVectors (angles, forward, right, NULL);
		VectorMA(targorigin, 30, forward, goal);
		VectorSubtract (goal, targorigin, goaldir); // swapped

		ent->client->ps.fov = targ->client->ps.fov;
		break;
	}

	// trace from targorigin to final chase origin goal
	trace = gi.trace(targorigin, vec3_origin, vec3_origin, goal, targ, MASK_SOLID);

	// test for hit
	if (trace.fraction < 1) {
		// we hit something, need to do a bit of avoidance

		// take real end point
		VectorCopy (trace.endpos, goal);
		// real dir vector
		VectorSubtract(goal, targorigin, temp);
		// scale it back bit more
		VectorMA(targorigin, 0.9, temp, goal);
	}

	// someone set us up the target origin :)
	VectorCopy(goal, ent->s.origin);

	// move view
	vectoangles(goaldir, ent->client->ps.viewangles);
	vectoangles(goaldir, ent->client->v_angle);

	// deltas
	for (i=0 ; i<3 ; i++)
		ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(targ->client->v_angle[i] - ent->client->resp.cmd_angles[i]);
}

void ChasePrev(edict_t *ent)
{
	edict_t *e;

	if (!ent->client->chase_target)
		return;

	e = ent->client->chase_target;
	do {
		e--;
		if (!e->client)
			e = g_edicts+game.maxclients;
		if (!e->inuse)
			continue;
		if (e->solid != SOLID_NOT)
			break;
	} while (e != ent->client->chase_target);

	ent->client->chase_target = e;
	e->client->resp.chased = true;
}
