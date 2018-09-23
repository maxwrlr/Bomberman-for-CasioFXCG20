#ifndef _WORLD
#define _WORLD

#define WORLD_WIDTH_PX 311
#define WORLD_HEIGHT_PX 215
#define WORLD_X0 36 // = (LCD_WIDTH_PX - WORLD_WIDTH_PX) / 2

#define WORLD_GRID_WIDTH 13
#define WORLD_GRID_HEIGHT 9
#define WORLD_GRID_SIZE 23
#define WORLD_GRID_SIZE_B 24

#define WORLD_COLOR_BG 0x24a4

#define ID_GRASS 0x00
#define ID_WOOD 0x01
#define ID_STONE 0x02

#define ID_BOMB 0x10
#define ID_RANGE 0x20
#define ID_SHOES 0x30
#define ID_HEAL 0x40
#define ID_BOMB_IGNITED 0xf0

#define ENTITY_SIZE 19

_Bool World_Init(short);
int World_Run(_Bool);
void World_DrawBackground();
void World_CleanUp();

short World_GetNumPlayers();
Bomberman** World_GetBombermen();
void World_DrawStats();
void World_DrawPlayerStats();

Bomb_t* World_GetBomb(short, short);
_Bool World_PlaceBomb(Bomberman*, short, short);
short World_DetonateAndBreak(Bomb_t*, int, int);
_Bool World_HitBombermen(Bomb_t*, int, int);
short World_ToGrid(int);

_Bool World_IsValid(short, short);
short World_GetBlockType(short, short);
void World_SetBlockType(short, short, short);
void World_RemoveBlock(short, short);
short World_GetEntityType(short, short);
void World_SetEntityType(short, short, short);
void World_RemoveEntity(short, short);
void World_DrawEntity(short, short);

unsigned int SeedRandom(int);
float Random();

#endif //_WORLD
