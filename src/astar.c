#include <HEAP_syscalls.h>
#include <stdlib.h>

#include "astar.h"
#include "null.h"

#include "bomberman.h"
#include "bombs.h"
#include "world.h"

_Bool List_Contains(CoordList *list, short coords) {
	CoordList *item = list;
	while(item != NULL) {
		if(item->coords == coords)
			return 1;
		item = item->next;
	}
	return 0;
}

_Bool List_Push(CoordList *list, short coords) {

	CoordList *newItem = malloc(sizeof(CoordList));
	if(newItem == NULL)
		return 0;

	newItem->coords = coords;
	newItem->next = NULL;

	CoordList *item = list;
	while(item->next != NULL)
		item = item->next;

	item->next = newItem;

	return 1;
}

void List_Pop(CoordList *list) {
	CoordList *item = list;
	while(item->next != NULL && item->next->next != NULL)
		item = item->next;

	free(item->next);
	item->next = NULL;
}

void List_Free(CoordList* list) {
	CoordList* item = list;
	while(item) {
		CoordList* temp = item->next;
		free(item);
		item = temp;
	}
}

CoordList* AStar_Deeper(short x, short y, short target, int maxdist, CoordList *list) {
	short pos = y + (x << 4);
	if(pos == target) {
		if(List_Push(list, pos))
			return list;
	} else
	if(!List_Contains(list, pos)) {
		if(List_Push(list, pos)) {
			if(AStar_GetNext(x, y, target, maxdist, list) != NULL)
				return list;

			List_Pop(list);
		}
	}

	return NULL;
}

CoordList* AStar_Next(short fx, short fy, short tx, short ty) {
	CoordList* list = malloc(sizeof(CoordList));
	if(list == NULL)
		return NULL;

	list->coords = fy + (fx << 4);
	list->next = NULL;

	CoordList* result = AStar_GetNext(fx, fy, ty + (tx << 4), 2 * dist(fx, fy, tx, ty), list);
	if(result == NULL || result->next == NULL) {
		free(result);
		return NULL;
	}

	CoordList* ret = result->next;
	free(result);

	return ret;
}

CoordList* AStar_GetNext(short x, short y, short target, int maxdist, CoordList *list) {
	short	tx = (short) ((target & 0x00f0) >> 4),
			ty = (short) (target & 0x000f);

	if(dist(x, y, tx, ty) > maxdist)
		return NULL;

	int dists[4] = {square(tx - (x + 1)) + square(ty - y),		// EAST
					square(tx - (x - 1)) + square(ty - y),		// WEST
					square(tx - x) + square(ty - (y + 1)),		// SOUTH
					square(tx - x) + square(ty - (y - 1))};		// NORTH

	int sorted[4] = {dists[0], dists[1], dists[2], dists[3]};	// copy of dists to sort

	/*
	 * BEGIN SORT
	 */
	for(int i = 0; i < 3; i++) {
		int idx = i;
		for(int j = i + 1; j < 4; j++) {
			if(sorted[idx] > sorted[j])
				idx = j;
		}
		if(idx != i) {
			int dummy = sorted[idx];
			sorted[idx] = sorted[i];
			sorted[i] = dummy;
		}
	}
	/*
	 * END SORT
	 */

	for(int i = 0; i < 4; i++) {

		// go EAST
		if(sorted[i] == dists[0] && x + 1 < WORLD_GRID_WIDTH && World_GetBlockType((short) (x + 1), y) == ID_GRASS) {
			dists[0] = -1;

			if(AStar_Deeper((short) (x + 1), y, target, maxdist, list))
				return list;
			continue;
		}

		// GO WEST
		if(sorted[i] == dists[1] && x - 1 >= 0 && World_GetBlockType((short) (x - 1), y) == ID_GRASS) {
			dists[1] = -1;

			if(AStar_Deeper((short) (x - 1), y, target, maxdist, list))
				return list;
			continue;
		}

		// go SOUTH
		if(sorted[i] == dists[2] && y + 1 < WORLD_GRID_HEIGHT && World_GetBlockType(x, (short) (y + 1)) == ID_GRASS) {
			dists[2] = -1;

			if(AStar_Deeper(x, (short) (y + 1), target, maxdist, list))
				return list;
			continue;
		}

		// go NORTH
		if(sorted[i] == dists[3] && y - 1 >= 0 && World_GetBlockType(x, (short) (y - 1)) == ID_GRASS) {
			dists[3] = -1;

			if(AStar_Deeper(x, (short) (y - 1), target, maxdist, list))
				return list;
		}
	}

	return NULL;
}