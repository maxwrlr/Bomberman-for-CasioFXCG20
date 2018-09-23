#ifndef _BOMBS
#define _BOMBS

#define BOMB_EXPLODE_AT 40
#define BOMB_VANISH_AT 60

typedef struct Bomb {
	short x;
	short y;
	short state;
	short range;
	short ticksLived;
	Bomberman* bomberman;
	struct Bomb* next;
} Bomb_t;

void Bombs_Tick();
short Bombs_NewBomb(Bomberman* bomberman, short, short);
void Bombs_Unshift();

Bomb_t* Bombs_First();

#endif //_BOMBS