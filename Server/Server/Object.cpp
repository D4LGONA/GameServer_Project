#include "stdafx.h"

bool Object::do_healing()
{
	if (hp == max_hp)
	{
		ishealing = false;
		return false;
	}
	hp = min(hp + max_hp * 0.1f, max_hp);

	return true;
}

bool Object::isNear(int other_id)
{
	if (other_id < 0) // npc
	{
		int k = (other_id * -1) - 1;
		int dist = (x - npcs[k].x) * (x - npcs[k].x) +
			(y - npcs[k].y) * (y - npcs[k].y);
		return dist <= EYESIGHT * EYESIGHT;
	}
	else
	{
		int dist = (x - players[other_id].x) * (x - players[other_id].x) +
			(y - players[other_id].y) * (y - players[other_id].y);
		return dist <= EYESIGHT * EYESIGHT;
	}
}

bool Object::isNear(int other_id, int distance)
{
	if (other_id < 0) // npc
	{
		int k = (other_id * -1) - 1;
		int dist = (x - npcs[k].x) * (x - npcs[k].x) +
			(y - npcs[k].y) * (y - npcs[k].y);
		return dist <= distance * distance;
	}
	else
	{
		int dist = (x - players[other_id].x) * (x - players[other_id].x) +
			(y - players[other_id].y) * (y - players[other_id].y);
		return dist <= distance * distance;
	}
}

double Object::distance(int other_id)
{
	if (other_id < 0) // npc
	{
		int k = (other_id * -1) - 1;
		int dist = (x - npcs[k].x) * (x - npcs[k].x) +
			(y - npcs[k].y) * (y - npcs[k].y);
		return dist;
	}
	else
	{
		int dist = (x - players[other_id].x) * (x - players[other_id].x) +
			(y - players[other_id].y) * (y - players[other_id].y);
		return dist;
	}
}
