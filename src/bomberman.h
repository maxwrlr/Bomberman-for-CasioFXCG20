#ifndef _BOMBERMAN
#define _BOMBERMAN

#include "astar.h"

#define ROTATION_N 0
#define ROTATION_E 90
#define ROTATION_S 180
#define ROTATION_W 270

#define DIRECTION_N 1
#define DIRECTION_E 2
#define DIRECTION_S 3
#define DIRECTION_W 4

#define abs(x) ((x) < 0 ? -(x) : (x))

typedef struct {
	short x;
	short y;
	short oldX;
	short oldY;
	short data;
	short AI_data;
	short AI_target;
	short AI_mvdata;
	struct Coords* AI_path;
	short rotation;
	short ticksHit;
} Bomberman;

void Bomberman_ControlByInput(Bomberman*);
void Bomberman_ControlByAlgorithm(Bomberman*);
void Bomberman_CheckCollision(Bomberman*);
_Bool Bomberman_Collides(int, int);
_Bool Bomberman_GrabItem(Bomberman*);
_Bool Bomberman_Hit(Bomberman*, short, short);

void Bomberman_Tick(Bomberman*);
void Bomberman_UpdateRotation(Bomberman*);
void Bomberman_BecomeOld(Bomberman*);
short Bomberman_GetGridX(Bomberman*);
short Bomberman_GetGridY(Bomberman*);
short Bomberman_GetLives(Bomberman*);
short Bomberman_GetBombs(Bomberman*);
void Bobmerman_AddBomb(Bomberman*);
short Bomberman_GetRange(Bomberman*);
short Bomberman_GetShoes(Bomberman*);
short Bomberman_GetSpeed(Bomberman*);

_Bool AIBehavior_DidSearchRandom(Bomberman*, short, short);
void AIBehavior_DoSearchRandom(Bomberman*, short, short);

_Bool AIBehavior_IsSafeSpot(short, short, short);
CoordList* AIBehavior_FindSafeSpot(short, short, short);
CoordList* AIBehavior_FindItem(short, short);
Bomberman* AIBehavior_FindEnemy(Bomberman*);
_Bool AIBehavior_IsWoodNearby(short, short);
short AIBehavior_WalkDistN(short, short);
short AIBehavior_WalkDistE(short, short);
short AIBehavior_WalkDistS(short, short);
short AIBehavior_WalkDistW(short, short);
_Bool AIBehavior_isBombN(short, short, short);
_Bool AIBehavior_isBombE(short, short, short);
_Bool AIBehavior_isBombS(short, short, short);
_Bool AIBehavior_isBombW(short, short, short);
_Bool AIBehavior_PlaceBomb(Bomberman*, short, short);

_Bool BombermanAI_HasTarget(Bomberman*);
short BombermanAI_GetTargetX(Bomberman*);
short BombermanAI_GetTargetY(Bomberman*);
void BombermanAI_SetTarget(Bomberman*, short, short);
void BombermanAI_SetTargetData(Bomberman*, int);
void BombermanAI_RemoveTarget(Bomberman* bomberman);
_Bool BombermanAI_PlaceBombAtTarget(Bomberman*);
short BombermanAI_GetDirection(Bomberman*);
void BombermanAI_SetDirection(Bomberman*, short);
_Bool BombermanAI_IsWaiting(Bomberman*);
void BombermanAI_Wait(Bomberman*, short);

#endif //_BOMBERMAN
