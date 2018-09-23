#include <HEAP_syscalls.h>
#include <color.h>
#include <rtc.h>
#include <display.h>
#include <system.h>

#include "bomberman.h"
#include "bombs.h"
#include "world.h"
#include "draw.h"
#include "null.h"
#include "key.h"

static short map[WORLD_GRID_WIDTH][WORLD_GRID_HEIGHT];
static Bomberman *player, *bombermen[4];

static short numPlayers;
static int lastrandom, waitTilEnd, winner;

_Bool World_Init(short numBots) {
	numPlayers = numBots + 1;
	lastrandom = (unsigned int) RTC_GetTicks();
	waitTilEnd = 50;
	winner = -1;
	
	// register wood
	for(int y = 0; y < WORLD_GRID_HEIGHT; y += 2)
		for(int x = 0; x < WORLD_GRID_WIDTH; x++) {
			// free areas to spawn
			if((y == 0 || y == (WORLD_GRID_HEIGHT - 1)) && (x < 2 || x > WORLD_GRID_WIDTH - 3))
				continue;
			
			map[x][y] = 0x01;
		}
	// register wood
	for(int y = 1; y < WORLD_GRID_HEIGHT; y += 2)
		for(int x = 0; x < WORLD_GRID_WIDTH; x += 2) {
			// free areas to spawn
			if((y == 1 || y == WORLD_GRID_HEIGHT - 2) && (x == 0 || x == WORLD_GRID_WIDTH - 1))
				continue;
			
			map[x][y] = 0x01;
		}
	// register stone
	for(int x = 1; x < WORLD_GRID_WIDTH; x += 2)
		for(int y = 1; y < WORLD_GRID_HEIGHT; y += 2)
			map[x][y] = 0x02;
	
	// defaults for player
	player = malloc(sizeof(Bomberman));
	if(player == NULL)
		return 0;

	player->x = 2;
	player->y = 2;
	player->oldX = player->x;
	player->oldY = player->y;
	player->data = 0x3110;
	player->rotation = ROTATION_S;
	player->ticksHit = 0;
	bombermen[0] = player;

	// defaults for bombermen
	for(int i = 1; i < numPlayers; i++) {
		Bomberman *bot = malloc(sizeof(Bomberman));
		if(bot == NULL)
			return 0;

		bot->x = i == 3 ? 2 : WORLD_WIDTH_PX - 21;
		bot->y = i == 2 ? 2 : WORLD_HEIGHT_PX - 21;
		bot->oldX = bot->x;
		bot->oldY = bot->y;
		bot->data = 0x3110;
		bot->AI_data = 0x0500 + (((short) (Random() * 25)) << 8);
		bot->AI_target = 0x0000;
		bot->AI_path = NULL;
		bot->rotation = i == 2 ? ROTATION_S : ROTATION_N;
		bot->ticksHit = 0;
		bombermen[i] = bot;
	}

	return 1;
}

int World_Run(_Bool firstDraw) {

	if(Key_IsDown(KEY_EXIT)) {
		return 3;
	}

	if(firstDraw) {
		World_DrawBackground();
		World_DrawPlayerStats();
		World_DrawStats();
	}
	
	int i;
	for(i = 0; i < numPlayers; i++)
		if(Bomberman_GetLives(bombermen[i]) > 0)
			Draw_ClearBomberman(bombermen[i]);

	Bombs_Tick();

	if(Bomberman_GetLives(player) > 0)
		Bomberman_ControlByInput(player);
	for(i = 1; i < numPlayers; i++)
		if(Bomberman_GetLives(bombermen[i]) > 0)
			Bomberman_ControlByAlgorithm(bombermen[i]);
	
	Bomb_t* bomb = Bombs_First();
	while(bomb != NULL) {
		if(bomb->ticksLived < BOMB_EXPLODE_AT) {
			switch(bomb->state) {
				case 0: case 8: Draw_AlphaBitmap(WORLD_X0 + bomb->x * WORLD_GRID_SIZE_B + 2, bomb->y * WORLD_GRID_SIZE_B + 2, ENTITY_SIZE, ENTITY_SIZE, Bitmap_GetBomb0(), 0x0000); break;
				case 1: case 7:	Draw_AlphaBitmap(WORLD_X0 + bomb->x * WORLD_GRID_SIZE_B + 2, bomb->y * WORLD_GRID_SIZE_B + 2, ENTITY_SIZE, ENTITY_SIZE, Bitmap_GetBomb1(), 0x0000); break;
				case 2: case 6: Draw_AlphaBitmap(WORLD_X0 + bomb->x * WORLD_GRID_SIZE_B + 2, bomb->y * WORLD_GRID_SIZE_B + 2, ENTITY_SIZE, ENTITY_SIZE, Bitmap_GetBomb2(), 0x0000); break;
				case 3: case 5: Draw_AlphaBitmap(WORLD_X0 + bomb->x * WORLD_GRID_SIZE_B + 2, bomb->y * WORLD_GRID_SIZE_B + 2, ENTITY_SIZE, ENTITY_SIZE, Bitmap_GetBomb3(), 0x0000); break;
				case 4: Draw_AlphaBitmap(WORLD_X0 + bomb->x * WORLD_GRID_SIZE_B + 2, bomb->y * WORLD_GRID_SIZE_B + 2, ENTITY_SIZE, ENTITY_SIZE, Bitmap_GetBomb4(), 0x0000); break;
			}
		}

		bomb = bomb->next;
	}

	for(i = 1; i < numPlayers; i++)
		if(Bomberman_GetLives(bombermen[i]) > 0)
			Draw_Bomberman(bombermen[i],
				i == 1 ? Bitmap_GetBombermanRed() :
				i == 2 ? Bitmap_GetBombermanPurple() :
				Bitmap_GetBombermanYellow()
			);
	if(Bomberman_GetLives(player) > 0)
		Draw_Bomberman(player, Bitmap_GetBombermanBlue());
	
	bomb = Bombs_First();
	while(bomb != NULL) {
		if(bomb->ticksLived < BOMB_EXPLODE_AT)
			break;

		_Bool clear = bomb->ticksLived == BOMB_VANISH_AT;
		color_t *range1 = clear ? Bitmap_ClearRange1() : Bitmap_GetRange1(),
				*range2 = clear ? Bitmap_ClearRange2() : Bitmap_GetRange2();

		if(clear) {
			Draw_Rectangle(WORLD_X0 + bomb->x * WORLD_GRID_SIZE_B, bomb->y * WORLD_GRID_SIZE_B, WORLD_GRID_SIZE, WORLD_GRID_SIZE, WORLD_COLOR_BG);
		} else
			Draw_AlphaBitmap(WORLD_X0 + bomb->x * WORLD_GRID_SIZE_B, bomb->y * WORLD_GRID_SIZE_B, WORLD_GRID_SIZE, WORLD_GRID_SIZE, Bitmap_GetRange0(), 0x0000);
		if(World_HitBombermen(bomb, 0, 0)) {
			World_DrawPlayerStats();
			World_DrawStats();
		}

		// Right
		for(int dx = 1; dx <= bomb->range; dx++) {
			short br = World_DetonateAndBreak(bomb, dx, 0);
			if(br == 1)
				break;

			Draw_AlphaBitmap180deg(WORLD_X0 + (bomb->x + dx) * WORLD_GRID_SIZE_B - 1, bomb->y * WORLD_GRID_SIZE_B - 1, 25, 25, dx == bomb->range ? range2 : range1, 0x0000);
			if(clear)
				World_DrawEntity(bomb->x + dx, bomb->y);
			
			if(bomb->x + dx == WORLD_GRID_WIDTH - 1 || World_GetBlockType(bomb->x + dx + 1, bomb->y) != ID_GRASS)
				Draw_Rectangle(WORLD_X0 + (bomb->x + dx + 1) * WORLD_GRID_SIZE_B - 1, bomb->y * WORLD_GRID_SIZE_B, 1, WORLD_GRID_SIZE, 0x0000);

			if(br == 2)
				break;
		}

		// Left
		for(int dx = -1; dx >= -bomb->range; dx--) {
			short br = World_DetonateAndBreak(bomb, dx, 0);
			if(br == 1)
				break;

			Draw_AlphaBitmap(WORLD_X0 + (bomb->x + dx) * WORLD_GRID_SIZE_B - 1, bomb->y * WORLD_GRID_SIZE_B - 1, 25, 25, dx == -bomb->range ? range2 : range1, 0x0000);
			if(clear)
				World_DrawEntity(bomb->x + dx, bomb->y);
			
			if(bomb->x + dx == 0 || World_GetBlockType(bomb->x + dx - 1, bomb->y) != ID_GRASS)
				Draw_Rectangle(WORLD_X0 + (bomb->x + dx) * WORLD_GRID_SIZE_B - 1, bomb->y * WORLD_GRID_SIZE_B, 1, WORLD_GRID_SIZE, 0x0000);
			
			if(br == 2)
				break;
		}

		// Down
		for(int dy = 1; dy <= bomb->range; dy++) {
			short br = World_DetonateAndBreak(bomb, 0, dy);
			if(br == 1)
				break;

			Draw_AlphaBitmap270deg(WORLD_X0 + bomb->x * WORLD_GRID_SIZE_B - 1, (bomb->y + dy) * WORLD_GRID_SIZE_B - 1, 25, 25, dy == bomb->range ? range2 : range1, 0x0000);
			if(clear)
				World_DrawEntity(bomb->x, bomb->y + dy);
			
			if(bomb->y + dy == WORLD_GRID_HEIGHT - 1 || World_GetBlockType(bomb->x, bomb->y + dy + 1) != ID_GRASS)
				Draw_Rectangle(WORLD_X0 + bomb->x * WORLD_GRID_SIZE_B, (bomb->y + dy + 1) * WORLD_GRID_SIZE_B - 1, WORLD_GRID_SIZE, 1, 0x0000);
			
			if(br == 2)
				break;
		}

		// Up
		for(int dy = -1; dy >= -bomb->range; dy--) {
			short br = World_DetonateAndBreak(bomb, 0, dy);
			if(br == 1)
				break;

			Draw_AlphaBitmap90deg(WORLD_X0 + bomb->x * WORLD_GRID_SIZE_B - 1, (bomb->y + dy) * WORLD_GRID_SIZE_B - 1, 25, 25, dy == -bomb->range ? range2 : range1, 0x0000);
			if(clear)
				World_DrawEntity(bomb->x, bomb->y + dy);

			if(bomb->y + dy > 0 && World_GetBlockType(bomb->x, bomb->y + dy - 1) != ID_GRASS)
				Draw_Rectangle(WORLD_X0 + bomb->x * WORLD_GRID_SIZE_B, (bomb->y + dy) * WORLD_GRID_SIZE_B - 1, WORLD_GRID_SIZE, 1, 0x0000);
			
			if(br == 2)
				break;
		}

		if(clear) {
			World_SetEntityType(bomb->x, bomb->y, 0);
			Bobmerman_AddBomb(bomb->bomberman);
			bomb = bomb->next;
			Bombs_Unshift();
		} else
			bomb = bomb->next;
	}
	
	int alive = 0, idx = -1;
	for(i = 0; i < numPlayers; i++)
		if(Bomberman_GetLives(bombermen[i]) > 0) {
			Bomberman_BecomeOld(bombermen[i]);
			alive++;
			idx = i;
		}

	waitTilEnd--;
	if(waitTilEnd == 0) {
		World_CleanUp();
		return 4 + ((winner + 1) << 4);
	} else
	if(waitTilEnd < 49) {
		return 2;
	} else
	if(alive < 2) {
		winner = idx;
	} else
		waitTilEnd = 50;
	
	return 2;
}

void World_CleanUp() {
	while(Bombs_First())
		Bombs_Unshift();

	for(int i = 0; i < numPlayers; i++) {
		BombermanAI_RemoveTarget(bombermen[i]);
		free(bombermen[i]);
	}

	for(int x = 0; x < WORLD_GRID_WIDTH; x ++)
		for(int y = 0; y < WORLD_GRID_HEIGHT; y ++)
			map[x][y] = 0;
}

short World_GetNumPlayers() {
	return numPlayers;
}

Bomberman** World_GetBombermen() {
	return bombermen;
}

void World_DrawStats() {
	Draw_Rectangle(WORLD_X0 + WORLD_WIDTH_PX, 0, WORLD_X0 + 1, 71, 0x0000);

	static unsigned char nums[] = {'0', '1', '2', '3'};
	for(int i = 1; i < numPlayers; i++) {
		Draw_AlphaBitmap(WORLD_X0 + WORLD_WIDTH_PX + 3, 5 + (i - 1) * 22, ENTITY_SIZE, ENTITY_SIZE,
						 i == 1 ? Bitmap_GetBombermanRed() :
						 i == 2 ? Bitmap_GetBombermanPurple() :
						 Bitmap_GetBombermanYellow(),
						 0x0000
		);

		unsigned char text[] = {nums[Bomberman_GetLives(bombermen[i])], '\0'};
		Draw_Text(WORLD_X0 + WORLD_WIDTH_PX + 25, 5 + (i - 1) * 22, text, COLOR_LIGHTGRAY);
	}
}

void World_DrawPlayerStats() {
	Draw_Rectangle(0, 0, WORLD_X0, 88, 0x0000);
	Bomberman* player = bombermen[0];

	static unsigned char nums[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
	Draw_AlphaBitmap(WORLD_X0 - 22, 5 + 0 * 22, ENTITY_SIZE, ENTITY_SIZE, Bitmap_getItemHeart(), 0x0000);
	Draw_AlphaBitmap(WORLD_X0 - 22, 5 + 1 * 22, ENTITY_SIZE, ENTITY_SIZE, Bitmap_getItemBomb(), 0x0000);
	Draw_AlphaBitmap(WORLD_X0 - 22, 5 + 2 * 22, ENTITY_SIZE, ENTITY_SIZE, Bitmap_getItemRange(), 0x0000);
	Draw_AlphaBitmap(WORLD_X0 - 22, 5 + 3 * 22, ENTITY_SIZE, ENTITY_SIZE, Bitmap_getItemShoes(), 0x0000);

	for(int i = 0; i < 4; i++) {
		unsigned char text[] = {nums[(player->data & (0x000f << ((3 - i) * 4))) >> ((3 - i) * 4)], '\0'};
		Draw_Text(0, 5 + i * 22, text, COLOR_LIGHTGRAY);
	}
}

// ----------------------------------

Bomb_t* World_GetBomb(short x, short y) {

	Bomb_t* bomb = Bombs_First();
	while(bomb != NULL) {
		if(bomb->x == x && bomb->y == y)
			return bomb;

		bomb = bomb->next;
	}

	return NULL;
}

_Bool World_PlaceBomb(Bomberman* bomberman, short x, short y) {
	if(Bombs_NewBomb(bomberman, x, y)) {
		World_SetEntityType(x, y, ID_BOMB_IGNITED);
		return 1;
	}
	return 0;
}

void World_DrawBackground() {
	Draw_FillScreen(0x0000);
	Draw_Rectangle(WORLD_X0, 0, WORLD_WIDTH_PX, WORLD_HEIGHT_PX, WORLD_COLOR_BG);
	
	for(int x = 0; x < WORLD_GRID_WIDTH; x++)
		for(int y = 0; y < WORLD_GRID_HEIGHT; y++) {
			switch(World_GetBlockType(x, y)) {
				case ID_STONE: Draw_Stone(x, y); break;
				case ID_WOOD: Draw_Wood(x, y); break;
			}
			switch(World_GetEntityType(x, y)) {
				case ID_BOMB: break;
				case ID_RANGE: break;
				case ID_SHOES: break;
				case ID_HEAL: break;
			}
		}
}

short World_DetonateAndBreak(Bomb_t* bomb, int dx, int dy) {
	if(bomb->y + dy < 0 || bomb->y + dy >= WORLD_GRID_HEIGHT || bomb->x + dx < 0 || bomb->x + dx >= WORLD_GRID_WIDTH)
		return 1;
	
	if(bomb->ticksLived == BOMB_EXPLODE_AT && World_GetEntityType(bomb->x + dx, bomb->y + dy))
		World_RemoveEntity(bomb->x + dx, bomb->y + dy);
	
	if(World_HitBombermen(bomb, dx, dy)) {
		World_DrawPlayerStats();
		World_DrawStats();
	}
	
	short blockType = World_GetBlockType(bomb->x + dx, bomb->y + dy);
	if(blockType == ID_STONE) {
		return 1;
	} else if(blockType == ID_WOOD) {
		
		if(bomb->ticksLived > BOMB_EXPLODE_AT)
			return 1;
		
		World_RemoveBlock(bomb->x + dx, bomb->y + dy);
		float random = Random();
		if(random >= 0.82) {
			if(random < 0.85) {
				World_SetEntityType(bomb->x + dx, bomb->y + dy, ID_HEAL);
			} else
			if(random < 0.9) {
				World_SetEntityType(bomb->x + dx, bomb->y + dy, ID_BOMB);
			} else
			if(random < 0.95) {
				World_SetEntityType(bomb->x + dx, bomb->y + dy, ID_RANGE);
			} else
				World_SetEntityType(bomb->x + dx, bomb->y + dy, ID_SHOES);
		}
		return 2; // return after drawing
	}
	
	return 0; // Returns whether to continue destroying in current direction
}

_Bool World_HitBombermen(Bomb_t* bomb, int dx, int dy) {
	_Bool result = 0;
	for(int i = 0; i < numPlayers; i++)
		if(Bomberman_GetLives(bombermen[i]) > 0) {
			result = result || Bomberman_Hit(bombermen[i], bomb->x + dx, bomb->y + dy);
			if(Bomberman_GetLives(bombermen[i]) == 0)
				Draw_ClearBomberman(bombermen[i]);
		}
	return result;
}

short World_ToGrid(int i) {
	return i / WORLD_GRID_SIZE_B;
}

_Bool World_IsValid(short x, short y) {
	return x > -1 && x < WORLD_GRID_WIDTH && y > -1 && y < WORLD_GRID_HEIGHT;
}

short World_GetBlockType(short x, short y) {
	return map[x][y] & 0x0f;
}

void World_SetBlockType(short x, short y, short type) {
	map[x][y] = (map[x][y] & 0xf0) + type;
}

void World_RemoveBlock(short x, short y) {
	World_SetBlockType(x, y, 0);
	Draw_Rectangle(WORLD_X0 + x * WORLD_GRID_SIZE_B, y * WORLD_GRID_SIZE_B, WORLD_GRID_SIZE, WORLD_GRID_SIZE, WORLD_COLOR_BG);
	
	if(x > 0 && World_GetBlockType(x - 1, y) == ID_GRASS)
		Draw_Rectangle(WORLD_X0 + x * WORLD_GRID_SIZE_B - 1, y * WORLD_GRID_SIZE_B, 1, WORLD_GRID_SIZE, WORLD_COLOR_BG);
	if(x < WORLD_GRID_WIDTH - 1 && World_GetBlockType(x + 1, y) == ID_GRASS)
		Draw_Rectangle(WORLD_X0 + (x + 1) * WORLD_GRID_SIZE_B - 1, y * WORLD_GRID_SIZE_B, 1, WORLD_GRID_SIZE, WORLD_COLOR_BG);

	if(y > 0 && World_GetBlockType(x, y - 1) == ID_GRASS)
		Draw_Rectangle(WORLD_X0 + x * WORLD_GRID_SIZE_B, y * WORLD_GRID_SIZE_B - 1, WORLD_GRID_SIZE, 1, WORLD_COLOR_BG);
	if(y < WORLD_GRID_HEIGHT - 1 && World_GetBlockType(x, y + 1) == ID_GRASS)
		Draw_Rectangle(WORLD_X0 + x * WORLD_GRID_SIZE_B, (y + 1) * WORLD_GRID_SIZE_B - 1, WORLD_GRID_SIZE, 1, WORLD_COLOR_BG);
}

short World_GetEntityType(short x, short y) {
	return map[x][y] & 0xf0;
}

void World_SetEntityType(short x, short y, short type) {
	map[x][y] = (map[x][y] & 0x0f) + type;
}

void World_DrawEntity(short x, short y) {
	short type = World_GetEntityType(x, y);
	if(!type)
		return;
	
	switch(type) {
		case ID_HEAL: Draw_AlphaBitmap(WORLD_X0 + x * WORLD_GRID_SIZE_B + 2, y * WORLD_GRID_SIZE_B + 2, ENTITY_SIZE, ENTITY_SIZE, Bitmap_getItemHeart(), 0x0000); break;
		case ID_BOMB: Draw_AlphaBitmap(WORLD_X0 + x * WORLD_GRID_SIZE_B + 2, y * WORLD_GRID_SIZE_B + 2, ENTITY_SIZE, ENTITY_SIZE, Bitmap_getItemBomb(), 0x0000); break;
		case ID_RANGE: Draw_AlphaBitmap(WORLD_X0 + x * WORLD_GRID_SIZE_B + 2, y * WORLD_GRID_SIZE_B + 2, ENTITY_SIZE, ENTITY_SIZE, Bitmap_getItemRange(), 0x0000); break;
		case ID_SHOES: Draw_AlphaBitmap(WORLD_X0 + x * WORLD_GRID_SIZE_B + 2, y * WORLD_GRID_SIZE_B + 2, ENTITY_SIZE, ENTITY_SIZE, Bitmap_getItemShoes(), 0x0000); break;
	}
}

void World_RemoveEntity(short x, short y) {
	if(World_GetEntityType(x, y) != ID_BOMB_IGNITED)
		World_SetEntityType(x, y, 0);
	Draw_Rectangle(WORLD_X0 + x * WORLD_GRID_SIZE_B + 2, y * WORLD_GRID_SIZE_B + 2, ENTITY_SIZE, ENTITY_SIZE, WORLD_COLOR_BG);
}

float Random() {
	lastrandom = 0x41C64E6D * lastrandom + 0x3039;
	return (float) (lastrandom >> 16 & 0x7fff) / 0x7fff;
}