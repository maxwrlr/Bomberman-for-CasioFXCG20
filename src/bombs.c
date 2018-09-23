#include <HEAP_syscalls.h>
#include <stdlib.h>

#include "bomberman.h"
#include "bombs.h"
#include "null.h"

static Bomb_t *first;

void Bombs_Tick() {
	Bomb_t* bomb = first;
	while(bomb != NULL) {
		bomb->ticksLived++;
		bomb->state = bomb->ticksLived % 9;
		bomb = bomb->next;
	}
}

short Bombs_NewBomb(Bomberman* bomberman, short x, short y) {
	Bomb_t* newBomb = malloc(sizeof(Bomb_t));
	if(newBomb == NULL)
		return 0;
	
	newBomb->x = x;
	newBomb->y = y;
	newBomb->state = 0;
	newBomb->range = Bomberman_GetRange(bomberman);
	newBomb->ticksLived = 0;
	newBomb->bomberman = bomberman;
	newBomb->next = NULL;
	
	if(!first) {
		first = newBomb;
		return 1;
	}
	
	Bomb_t* last = first;
	while(last->next != NULL)
		last = last->next;
		
	last->next = newBomb;
	
	return 1;
}

void Bombs_Unshift() {
	Bomb_t* old = first;
	first = first->next;
	free(old);
}

Bomb_t* Bombs_First() {
	return first;
}