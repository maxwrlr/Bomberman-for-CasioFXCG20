#include <HEAP_syscalls.h>
#include "bomberman.h"
#include "bombs.h"
#include "world.h"
#include "key.h"
#include "astar.h"
#include "null.h"

static _Bool wasShiftDown = 0, wasXDown = 0, lastPressUp, lastPressDown, lastPressLeft, lastPressRight;

void Bomberman_ControlByInput(Bomberman* bomberman) {
	Bomberman_Tick(bomberman);
	short speed = Bomberman_GetSpeed(bomberman);

	_Bool	pressUp = Key_IsDown(KEY_UP),
			pressDown = Key_IsDown(KEY_DOWN),
			pressLeft = Key_IsDown(KEY_LEFT),
			pressRight = Key_IsDown(KEY_RIGHT);

	if(pressUp && pressDown) {
		pressUp = lastPressUp;
		pressDown = lastPressDown;
	}

	if(pressLeft && pressRight) {
		pressLeft = lastPressLeft;
		pressRight = lastPressRight;
	}

	if(pressUp || Key_IsDown(KEY_7) || Key_IsDown(KEY_8) || Key_IsDown(KEY_9)) {
		bomberman->y -= speed;
		bomberman->rotation = ROTATION_N;
	}
	if(pressDown || Key_IsDown(KEY_1) || Key_IsDown(KEY_2) || Key_IsDown(KEY_3)) {
		bomberman->y += speed;
		bomberman->rotation = ROTATION_S;
	}
	if(pressLeft || Key_IsDown(KEY_7) || Key_IsDown(KEY_4) || Key_IsDown(KEY_1)) {
		bomberman->x -= speed;
		bomberman->rotation = ROTATION_W;
	}
	if(pressRight || Key_IsDown(KEY_9) || Key_IsDown(KEY_6) || Key_IsDown(KEY_3)) {
		bomberman->x += speed;
		bomberman->rotation = ROTATION_E;
	}

	lastPressUp = pressUp;
	lastPressDown = pressDown;
	lastPressLeft = pressLeft;
	lastPressRight = pressRight;

	Bomberman_CheckCollision(bomberman);
	Bomberman_UpdateRotation(bomberman);
	
	if(Bomberman_GrabItem(bomberman))
		World_DrawPlayerStats();
	
	/*
	 * Place a bomb
	 */
	
	short gridx = Bomberman_GetGridX(bomberman),
			gridy = Bomberman_GetGridY(bomberman);
	
	if((!wasShiftDown && Key_IsDown(KEY_SHIFT)) || (!wasXDown && Key_IsDown(KEY_MULT))) {
		if(!wasShiftDown && Key_IsDown(KEY_SHIFT)) {
			wasShiftDown = 1;
		} else
			wasXDown = 1;
		
		if(Bomberman_GetBombs(bomberman) > 0 && World_GetEntityType(gridx, gridy) != ID_BOMB_IGNITED) {
			if(World_PlaceBomb(bomberman, gridx, gridy))
				bomberman->data -= 0x0100;
			World_DrawPlayerStats();
		}
	}

	if(wasShiftDown && !Key_IsDown(KEY_SHIFT))
		wasShiftDown = 0;

	if(wasXDown && !Key_IsDown(KEY_MULT))
		wasXDown = 0;
}

void Bomberman_ControlByAlgorithm(Bomberman* bomberman) {
	Bomberman_Tick(bomberman);

	short 	gridx = Bomberman_GetGridX(bomberman),
			gridy = Bomberman_GetGridY(bomberman);
	short 	originalDirection = BombermanAI_GetDirection(bomberman);

	_Bool moved = 0, force = 0;
	_Bool isInMiddle = bomberman->x % 24 < 5 && bomberman->y % 24 < 5;

	/*
	 * I. in danger?
	 */

	short range = Bomberman_GetRange(bomberman) + 2;

	_Bool isBomb = World_GetEntityType(gridx, gridy) == ID_BOMB_IGNITED,
			isBombX = isBomb || AIBehavior_isBombN(gridx, gridy, range) || AIBehavior_isBombS(gridx, gridy, range) || AIBehavior_isBombE(gridx, gridy, range) || AIBehavior_isBombW(gridx, gridy, range),
			isBombN = AIBehavior_isBombN(gridx, gridy, -range) || AIBehavior_isBombE(gridx, gridy - 1, range) || AIBehavior_isBombW(gridx, gridy - 1, range),
			isBombS = AIBehavior_isBombS(gridx, gridy, -range) || AIBehavior_isBombE(gridx, gridy + 1, range) || AIBehavior_isBombW(gridx, gridy + 1, range),
			isBombE = AIBehavior_isBombE(gridx, gridy, -range) || AIBehavior_isBombN(gridx + 1, gridy, range) || AIBehavior_isBombS(gridx + 1, gridy, range),
			isBombW = AIBehavior_isBombW(gridx, gridy, -range) || AIBehavior_isBombN(gridx - 1, gridy, range) || AIBehavior_isBombS(gridx - 1, gridy, range);

	_Bool isFreeN = gridy > 0 && World_GetBlockType(gridx, gridy - 1) == ID_GRASS,
			isFreeE = gridx < WORLD_GRID_WIDTH - 1 && World_GetBlockType(gridx + 1, gridy) == ID_GRASS,
			isFreeS = gridy < WORLD_GRID_HEIGHT - 1 && World_GetBlockType(gridx, gridy + 1) == ID_GRASS,
			isFreeW = gridx > 0 && World_GetBlockType(gridx - 1, gridy) == ID_GRASS;

	_Bool canGoN = !isBombN && isFreeN,
			canGoE = !isBombE && isFreeE,
			canGoS = !isBombS && isFreeS,
			canGoW = !isBombW && isFreeW;
	_Bool canGoSomewhere = canGoN || canGoE || canGoS || canGoW;

	if(isBombX) {
		_Bool search = 1;

		if(BombermanAI_HasTarget(bomberman) && (bomberman->AI_target & 0x0f00) == 0x0200) {
			short tx = BombermanAI_GetTargetX(bomberman),
					ty = BombermanAI_GetTargetY(bomberman);
			if(AIBehavior_IsSafeSpot(tx, ty, range)) {
				search = 0;
				force = 1;
			}
		}

		if(search) {
			CoordList* safeTargetPath = AIBehavior_FindSafeSpot(gridx, gridy, range);
			if(safeTargetPath != NULL) {
				CoordList* target = safeTargetPath;
				while(target->next)
					target = target->next;

				BombermanAI_SetTargetData(bomberman, (target->coords & 0xf0ff) + (safeTargetPath->coords & 0x0f00));
				bomberman->AI_path = safeTargetPath;
				force = 1;
			}
		}
	} else
	if(BombermanAI_IsWaiting(bomberman)) {
		bomberman->AI_data -= 0x0100;
		return;
	}

	/*
	 * I-. check walking in middle
	 */

	int nextX = (bomberman->x + (originalDirection == DIRECTION_E ? Bomberman_GetSpeed(bomberman) : (originalDirection == DIRECTION_W ? -Bomberman_GetSpeed(bomberman) : 0))) % 24;
	int nextY = (bomberman->y + (originalDirection == DIRECTION_S ? Bomberman_GetSpeed(bomberman) : (originalDirection == DIRECTION_N ? -Bomberman_GetSpeed(bomberman) : 0))) % 24;
	_Bool justMove = (isInMiddle && nextX < 5 && nextY < 5 && ((bomberman->x % 24 != nextX && (bomberman->x % 24 == 0 || bomberman->x % 24 == 4)) || (bomberman->y % 24 != nextY && (bomberman->y % 24 == 0 || bomberman->y % 24 == 4))));

	if(!justMove) {

	/*
	 * II. collect items?
	 */

	if(isInMiddle) {

		if(!force && BombermanAI_HasTarget(bomberman)) {

			if(BombermanAI_PlaceBombAtTarget(bomberman) && !AIBehavior_IsWoodNearby(BombermanAI_GetTargetX(bomberman), BombermanAI_GetTargetY(bomberman))) {
				BombermanAI_RemoveTarget(bomberman);
			}
		}

		if(!BombermanAI_HasTarget(bomberman)) {
			CoordList* targetItemPath = AIBehavior_FindItem(gridx, gridy);
			CoordList* target = targetItemPath;
			while(target != NULL && target->next)
				target = target->next;

			Bomberman* enemy = Random() > 0.1 ? ((target == NULL || (targetItemPath->coords & 0x0f00) == 0x0100) ? AIBehavior_FindEnemy(bomberman) : NULL) : NULL;

			if(enemy != NULL && (target == NULL || dist(gridx, gridy, ((target->coords & 0x00f0) >> 4), (target->coords & 0x000f)) > dist(gridx, gridy, Bomberman_GetGridX(enemy), Bomberman_GetGridY(enemy)))) {
				short egx = Bomberman_GetGridX(enemy),
						egy = Bomberman_GetGridY(enemy);

				List_Free(targetItemPath);

				if(egx != gridx || egy != gridy) {
					BombermanAI_SetTarget(bomberman, egx, egy);
					bomberman->AI_path = AStar_Next(gridx, gridy, egx, egy);
				}
			} else
			if(targetItemPath != NULL) {
				BombermanAI_SetTargetData(bomberman, (target->coords & 0xf0ff) + (targetItemPath->coords & 0x0f00));
				bomberman->AI_path = targetItemPath;
			}
		}

		if(BombermanAI_HasTarget(bomberman)) {

			if(AIBehavior_IsSafeSpot(gridx, gridy, range) && BombermanAI_GetTargetX(bomberman) == gridx && BombermanAI_GetTargetY(bomberman) == gridy && BombermanAI_PlaceBombAtTarget(bomberman)) {
				moved = 1;
			} else
			if(!isBombX || canGoSomewhere) {

				CoordList* nextBlock = bomberman->AI_path;

				if(nextBlock != NULL && gridx == ((nextBlock->coords & 0xf0) >> 4) &&
						gridy == (nextBlock->coords & 0x0f)) {

					CoordList* next = nextBlock->next;
					free(nextBlock);
					nextBlock = next;
					bomberman->AI_path = next;
				}

				if(nextBlock != NULL) {

					short nextXY = nextBlock->coords;

					short dx = (short) (((nextXY & 0xf0) >> 4) - gridx);
					short dy = (short) ((nextXY & 0x0f) - gridy);

					if(!((abs(dx) == 1 && dy == 0) || (dx == 0 && abs(dy) == 1))) {
						BombermanAI_RemoveTarget(bomberman);
						return;
					}

					if (dx > 0) {
						BombermanAI_SetDirection(bomberman, DIRECTION_E);
						moved = 1;
					} else if (dx < 0) {
						BombermanAI_SetDirection(bomberman, DIRECTION_W);
						moved = 1;
					}

					if (dy > 0) {
						BombermanAI_SetDirection(bomberman, DIRECTION_S);
						moved = 1;
					} else if (dy < 0) {
						BombermanAI_SetDirection(bomberman, DIRECTION_N);
						moved = 1;
					}

					short direction = BombermanAI_GetDirection(bomberman);
					if((AIBehavior_isBombE(gridx, gridy, range) && direction == DIRECTION_W) || (AIBehavior_isBombW(gridx, gridy, range) && direction == DIRECTION_E) ||
							(AIBehavior_isBombE(gridx, gridy, range) && direction == DIRECTION_E) || (AIBehavior_isBombW(gridx, gridy, range) && direction == DIRECTION_W)) {
						if(canGoN) BombermanAI_SetDirection(bomberman, DIRECTION_N);
						else if(canGoS) BombermanAI_SetDirection(bomberman, DIRECTION_S);
						else moved = 0;
					} else
					if((AIBehavior_isBombN(gridx, gridy, range) && direction == DIRECTION_S) || (AIBehavior_isBombS(gridx, gridy, range) && direction == DIRECTION_N) ||
							(AIBehavior_isBombN(gridx, gridy, range) && direction == DIRECTION_N) || (AIBehavior_isBombS(gridx, gridy, range) && direction == DIRECTION_S)) {
						if(canGoE) BombermanAI_SetDirection(bomberman, DIRECTION_E);
						else if(canGoW) BombermanAI_SetDirection(bomberman, DIRECTION_W);
						else moved = 0;
					}

				} else
					BombermanAI_RemoveTarget(bomberman);
			}
		}
	}

	/*
	 * III. damage player?
	 */

	if(!isBombX && !force && canGoSomewhere) {
		short numPlayers = World_GetNumPlayers();
		Bomberman** bombermen = World_GetBombermen();
		for(int i = 0; i < numPlayers; i++) {
			if(bombermen[i] == bomberman || Bomberman_GetLives(bombermen[i]) == 0)
				continue;

			short r = Bomberman_GetRange(bomberman);

			if(Bomberman_GetGridX(bombermen[i]) == gridx) {
				short dist = abs(Bomberman_GetGridY(bombermen[i]) - gridy);
				if(dist <= r) {
					_Bool obstacle = 0;

					for(int y = gridy - r; y < gridy + r; y++) {

						if(y < 0 || y >= WORLD_GRID_HEIGHT)
							break;

						if(World_GetBlockType(gridx, y) != ID_GRASS) {
							obstacle = 1;
							break;
						}
					}

					if(!obstacle)
						AIBehavior_PlaceBomb(bomberman, gridx, gridy);
				}

			} else
			if(Bomberman_GetGridY(bombermen[i]) == gridy) {
				short dist = abs(Bomberman_GetGridX(bombermen[i]) - gridx);
				if(dist <= r) {
					_Bool obstacle = 0;
					for(int x = gridx - r; x < gridx + r; x++) {

						if(x < 0 || x >= WORLD_GRID_WIDTH)
							break;

						if(World_GetBlockType(x, gridy) != ID_GRASS) {
							obstacle = 1;
							break;
						}
					}

					if(!obstacle)
						AIBehavior_PlaceBomb(bomberman, gridx, gridy);
				}
			}
		}
	}

	/*
	 * IV. move randomly
	 */

	if(!moved && isInMiddle && !AIBehavior_DidSearchRandom(bomberman, gridx, gridy)) {
		BombermanAI_RemoveTarget(bomberman);
		AIBehavior_DoSearchRandom(bomberman, gridx, gridy);

		int random = (int) (1 + Random() * 3.99), security = 0;
		while(security < 5) {
			if(!(AIBehavior_isBombS(gridx, gridy, range) && (canGoE || canGoW)) && (canGoN || (isBombX && !canGoSomewhere && gridy > 0 && World_GetBlockType(gridx, gridy - 1) == ID_GRASS)) && !(canGoS && originalDirection == DIRECTION_S)) {
				if(random == 1) {
					BombermanAI_SetDirection(bomberman, DIRECTION_N);
					break;
				}
				random--;
			}
			if(!(AIBehavior_isBombW(gridx, gridy, range) && (canGoN || canGoS)) && (canGoE || (isBombX && !canGoSomewhere && gridx + 1 < WORLD_GRID_WIDTH && World_GetBlockType(gridx + 1, gridy) == ID_GRASS)) && !(canGoW && originalDirection == DIRECTION_W)) {
				if(random == 1) {
					BombermanAI_SetDirection(bomberman, DIRECTION_E);
					break;
				}
				random--;
			}
			if(!(AIBehavior_isBombN(gridx, gridy, range) && (canGoE || canGoW)) && (canGoS || (isBombX && !canGoSomewhere && gridy + 1 < WORLD_GRID_HEIGHT && World_GetBlockType(gridx, gridy + 1) == ID_GRASS)) && !(canGoN && originalDirection == DIRECTION_N)) {
				if(random == 1) {
					BombermanAI_SetDirection(bomberman, DIRECTION_S);
					break;
				}
				random--;
			}
			if(!(AIBehavior_isBombE(gridx, gridy, range) && (canGoN || canGoS)) && (canGoW || (isBombX && !canGoSomewhere && gridx > 0 && World_GetBlockType(gridx - 1, gridy) == ID_GRASS)) && !(canGoE && originalDirection == DIRECTION_E)) {
				if(random == 1) {
					BombermanAI_SetDirection(bomberman, DIRECTION_W);
					break;
				}
				random--;
			}

			security++;
		}

		if(security == 5)
			return;
	}

	}

	/*
	 * V. move
	 */

	short direction = BombermanAI_GetDirection(bomberman);

	short speed = Bomberman_GetSpeed(bomberman);
	switch (direction) {
		case DIRECTION_N:
			if(!justMove && isInMiddle && !force && !canGoN) {
				AIBehavior_DoSearchRandom(bomberman, 9, 13);
				break;
			}
			if(justMove || !isInMiddle || isFreeN)
				bomberman->y -= speed;
			break;
		case DIRECTION_S:
			if(!justMove && isInMiddle && !force && !canGoS) {
				AIBehavior_DoSearchRandom(bomberman, 9, 13);
				break;
			}
			if(justMove || !isInMiddle || isFreeS)
				bomberman->y += speed;
			break;
		case DIRECTION_W:
			if(!justMove && isInMiddle && !force && !canGoW) {
				AIBehavior_DoSearchRandom(bomberman, 9, 13);
				break;
			}
			if(justMove || !isInMiddle || isFreeW)
				bomberman->x -= speed;
			break;
		case DIRECTION_E:
			if(!justMove && isInMiddle && !force && !canGoE) {
				AIBehavior_DoSearchRandom(bomberman, 9, 13);
				break;
			}
			if(justMove || !isInMiddle || isFreeE)
				bomberman->x += speed;
			break;
		default:
			break;
	}

	Bomberman_UpdateRotation(bomberman);
	Bomberman_GrabItem(bomberman);

	if(justMove)
		return;

	if(isInMiddle) {
		if(direction != originalDirection)
			BombermanAI_Wait(bomberman, Random() * 4);

		gridx = Bomberman_GetGridX(bomberman);
		gridy = Bomberman_GetGridY(bomberman);

		if(canGoSomewhere) {

			if(BombermanAI_GetTargetX(bomberman) == gridx && BombermanAI_GetTargetY(bomberman) == gridy) {

				if(BombermanAI_PlaceBombAtTarget(bomberman)) {
					if (!AIBehavior_PlaceBomb(bomberman, gridx, gridy)) {
						return;
					}
				} else
				if(Random() > 0.8 && AIBehavior_IsWoodNearby(gridx, gridy))
					AIBehavior_PlaceBomb(bomberman, gridx, gridy);

				BombermanAI_RemoveTarget(bomberman);

			} else
			if(Random() > 0.9 && AIBehavior_IsWoodNearby(gridx, gridy))
				AIBehavior_PlaceBomb(bomberman, gridx, gridy);
		}
	}
}

void Bomberman_CheckCollision(Bomberman* bomberman) {
	bomberman->x = bomberman->x < 0 ? 0 : bomberman->x;
	bomberman->y = bomberman->y < 0 ? 0 : bomberman->y;
	bomberman->x = bomberman->x + ENTITY_SIZE >= WORLD_WIDTH_PX ? WORLD_WIDTH_PX - ENTITY_SIZE : bomberman->x;
	bomberman->y = bomberman->y + ENTITY_SIZE >= WORLD_HEIGHT_PX ? WORLD_HEIGHT_PX - ENTITY_SIZE : bomberman->y;

	if(!Bomberman_Collides(bomberman->x, bomberman->y))
		return;

	int newX = bomberman->x;
	int newY = bomberman->y;
	
	if(World_ToGrid(bomberman->x) < World_ToGrid(bomberman->oldX)) {
		newX = (World_ToGrid(bomberman->x) + 1) * WORLD_GRID_SIZE_B;
	} else
	if(World_ToGrid(bomberman->oldX + ENTITY_SIZE) < World_ToGrid(bomberman->x + ENTITY_SIZE)) {
		newX = World_ToGrid(bomberman->x + ENTITY_SIZE) * WORLD_GRID_SIZE_B - 1 - ENTITY_SIZE;
	}

	if(World_ToGrid(bomberman->y) < World_ToGrid(bomberman->oldY)) {
		newY = (World_ToGrid(bomberman->y) + 1) * WORLD_GRID_SIZE_B;
	} else
	if(World_ToGrid(bomberman->oldY + ENTITY_SIZE) < World_ToGrid(bomberman->y + ENTITY_SIZE)) {
		newY = World_ToGrid(bomberman->y + ENTITY_SIZE) * WORLD_GRID_SIZE_B - 1 - ENTITY_SIZE;
	}

	if(!Bomberman_Collides(newX, bomberman->y)) {
		bomberman->x = newX;
		bomberman->y = bomberman->y;
	} else
	if(!Bomberman_Collides(bomberman->x, newY)) {
		bomberman->x = bomberman->x;
		bomberman->y = newY;
	} else {
		bomberman->x = newX;
		bomberman->y = newY;
	}
}

_Bool Bomberman_Collides(int x, int y) {
	return World_GetBlockType(World_ToGrid(x), World_ToGrid(y)) != ID_GRASS ||
	   World_GetBlockType(World_ToGrid(x + ENTITY_SIZE), World_ToGrid(y)) != ID_GRASS ||
	   World_GetBlockType(World_ToGrid(x), World_ToGrid(y + ENTITY_SIZE)) != ID_GRASS ||
	   World_GetBlockType(World_ToGrid(x + ENTITY_SIZE), World_ToGrid(y + ENTITY_SIZE)) != ID_GRASS;
}

_Bool Bomberman_GrabItem(Bomberman* bomberman) {
	short gridx = Bomberman_GetGridX(bomberman),
			gridy = Bomberman_GetGridY(bomberman);
	_Bool found = 0;

	switch(World_GetEntityType(gridx, gridy)) {
		case ID_HEAL:
			if(Bomberman_GetLives(bomberman) < 3) {
				bomberman->data += 0x1000;
				World_DrawStats();
				found = 1;
			}
			break;
		case ID_BOMB:
			if(Bomberman_GetBombs(bomberman) < 15) {
				bomberman->data += 0x0100;
				found = 1;
			}
			break;
		case ID_RANGE:
			if(Bomberman_GetRange(bomberman) < 15) {
				bomberman->data += 0x0010;
				found = 1;
			}
			break;
		case ID_SHOES:
			if(Bomberman_GetShoes(bomberman) < 5) {
				bomberman->data += 0x0001;
				found = 1;
			}
			break;
	}

	World_RemoveEntity(gridx, gridy);
	return found;
}

_Bool Bomberman_Hit(Bomberman* bomberman, short x, short y) {
	if(bomberman->ticksHit == 0 && Bomberman_GetGridX(bomberman) == x && Bomberman_GetGridY(bomberman) == y) {
		bomberman->ticksHit = 40;
		bomberman->data -= 0x1000;
		return 1;
	}
	return 0;
}

/*
 * functions for AI behavior
 */

_Bool AIBehavior_IsSafeSpot(short x, short y, short range) {
	return !AIBehavior_isBombN(x, y, range) && !AIBehavior_isBombE(x, y, range) && !AIBehavior_isBombS(x, y, range) && !AIBehavior_isBombW(x, y, range) && World_GetEntityType(x, y) != ID_BOMB_IGNITED;
}

CoordList* AIBehavior_FindSafeSpot(short startX, short startY, short range) {
	int distance = 10000;
	CoordList* path = NULL;

	for(int x = startX - 2; x <= startX + 2; x++)
		for(int y = startY - 2; y <= startY + 2; y++) {
			if(x % 2 && y % 2)	// stone at this position
				continue;
			if(!World_IsValid(x, y))
				continue;
			if(x == startX && y == startY)
				continue;

			if(AIBehavior_IsSafeSpot(x, y, range)) {
				CoordList* nextBlock = AStar_Next(startX, startY, x, y);

				if(nextBlock != NULL) {

					int dist = dist(startX, startY, x, y);
					if (dist < distance || (dist == distance && Random() > 0.5)) {
						if(path) List_Free(path);
						path = nextBlock;
						path->coords += 0x0200;
						distance = dist;
					} else
						List_Free(nextBlock);
				}
			}
		}

	return path;
}

CoordList* AIBehavior_FindItem(short startX, short startY) {
	int distance = 10000;
	CoordList* path = NULL;

	for(int x = startX - 3; x <= startX + 3; x++)
		for(int y = startY - 3; y <= startY + 3; y++) {
			if(x % 2 && y % 2)	// stone at this position
				continue;
			if(!World_IsValid(x, y))
				continue;
			if(x == startX && y == startY)
				continue;

			if(World_GetEntityType(x, y) != ID_BOMB_IGNITED && (World_GetEntityType(x, y) != 0 || (World_GetBlockType(x, y) == ID_GRASS && AIBehavior_IsWoodNearby(x, y)))) {
				CoordList* nextBlock = AStar_Next(startX, startY, x, y);

				if(nextBlock != NULL) {
					int dist = square(x - startX) + square(y - startY);
					if(World_GetEntityType(x, y) == 0) {
						dist += 5 + Random() * 50;
					}

					if(dist < distance) {
						if(path) List_Free(path);
						path = nextBlock;
						distance = dist;

						if(AIBehavior_IsWoodNearby(x, y)) {
							path->coords += 0x0100;
						}
					} else
						List_Free(nextBlock);
				}
			}
		}

	return path;
}

Bomberman* AIBehavior_FindEnemy(Bomberman* bomberman) {
	Bomberman* enemy = NULL;
	short gx = Bomberman_GetGridX(bomberman), gy = Bomberman_GetGridY(bomberman);
	int tIdx = -1, distance = -1;

	short numPlayers = World_GetNumPlayers();
	Bomberman** bombermen = World_GetBombermen();
	for(int i = 0; i < numPlayers; i++) {
		if(bombermen[i] == bomberman || Bomberman_GetLives(bombermen[i]) == 0)
			continue;

		int dist = dist(Bomberman_GetGridX(bombermen[i]), Bomberman_GetGridY(bombermen[i]), gx, gy);
		if(dist < 25 && (tIdx == -1 || dist < distance)) {
			tIdx = i;
			distance = dist;
		}
	}

	if(tIdx != -1) {
		CoordList* nextXY = AStar_Next(gx, gy, Bomberman_GetGridX(bombermen[tIdx]), Bomberman_GetGridY(bombermen[tIdx]));

		if(nextXY != NULL) {
			enemy = bombermen[tIdx];
			List_Free(nextXY);
		}
	}

	return enemy;
}

_Bool AIBehavior_IsWoodNearby(short x, short y) {
	return (x < WORLD_GRID_WIDTH -1 && World_GetBlockType(x + 1, y) == ID_WOOD) || (x > 0 &&World_GetBlockType(x - 1, y) == ID_WOOD) || (y < WORLD_GRID_HEIGHT - 1 && World_GetBlockType(x, y + 1) == ID_WOOD) || (y > 0 && World_GetBlockType(x, y - 1) == ID_WOOD);

}

_Bool AIBehavior_isBombN(short gridx, short gridy, short range) {
	short dr = 0;
	if(range < 0) {
		range = -range;
		dr++;
	}

	for(int y = gridy - 1; y >= gridy - range; y--) {
		if(y < 0)
			return 0;
		
		if(World_GetEntityType(gridx, y) == ID_BOMB_IGNITED) {
			Bomb_t* bomb = World_GetBomb(gridx, y);
			if(bomb->range + dr < gridy - y)
				continue;
			return 1;
		} else
		if(World_GetBlockType(gridx, y) != ID_GRASS)
			return 0;
	}
	return 0;
}

_Bool AIBehavior_isBombE(short gridx, short gridy, short range) {
	short dr = 0;
	if(range < 0) {
		range = -range;
		dr++;
	}

	for(int x = gridx + 1; x <= gridx + range; x++) {
		if(x >= WORLD_GRID_WIDTH)
			return 0;
		
		if(World_GetEntityType(x, gridy) == ID_BOMB_IGNITED) {
			Bomb_t* bomb = World_GetBomb(x, gridy);
			if(bomb->range + dr < x - gridx)
				continue;
			return 1;
		} else
		if(World_GetBlockType(x, gridy) != ID_GRASS)
			return 0;
	}
	return 0;
}

_Bool AIBehavior_isBombS(short gridx, short gridy, short range) {
	short dr = 0;
	if(range < 0) {
		range = -range;
		dr++;
	}

	for(int y = gridy + 1; y <= gridy + range; y++) {
		if(y >= WORLD_GRID_HEIGHT)
			return 0;
		
		if(World_GetEntityType(gridx, y) == ID_BOMB_IGNITED) {
			Bomb_t* bomb = World_GetBomb(gridx, y);
			if(bomb->range + dr < y - gridy)
				continue;
			return 1;
		} else
		if(World_GetBlockType(gridx, y) != ID_GRASS)
			return 0;
	}
	return 0;
}

_Bool AIBehavior_isBombW(short gridx, short gridy, short range) {
	short dr = 0;
	if(range < 0) {
		range = -range;
		dr++;
	}

	for(int x = gridx - 1; x >= gridx - range; x--) {
		if(x < 0)
			return 0;
		
		if(World_GetEntityType(x, gridy) == ID_BOMB_IGNITED) {
			Bomb_t* bomb = World_GetBomb(x, gridy);
			if(bomb->range + dr < gridx - x)
				continue;
			return 1;
		} else
		if(World_GetBlockType(x, gridy) != ID_GRASS)
			return 0;
	}
	return 0;
}

_Bool AIBehavior_PlaceBomb(Bomberman* bomberman, short gridx, short gridy) {
	if(Bomberman_GetBombs(bomberman) > 0 && World_GetEntityType(gridx, gridy) != ID_BOMB_IGNITED) {
		if(World_PlaceBomb(bomberman, gridx, gridy))
			bomberman->data -= 0x0100;
		return 1;
	}
	return 0;
}

/*
 * struct related functions
 */

void Bomberman_Tick(Bomberman* bomberman) {
	if(bomberman->ticksHit)
		bomberman->ticksHit -= 1;
}

void Bomberman_UpdateRotation(Bomberman* bomberman) {
	if(bomberman->oldX < bomberman->x) {
		bomberman->rotation = ROTATION_E;
	} else
	if(bomberman->x < bomberman->oldX) {
		bomberman->rotation = ROTATION_W;
	} else
	if(bomberman->oldY < bomberman->y) {
		bomberman->rotation = ROTATION_S;
	} else
	if(bomberman->y < bomberman->oldY)
		bomberman->rotation = ROTATION_N;
}

void Bomberman_BecomeOld(Bomberman* bomberman) {
	bomberman->oldX = bomberman->x;
	bomberman->oldY = bomberman->y;
}

short Bomberman_GetGridX(Bomberman* bomberman) {
	return World_ToGrid(bomberman->x + 10);
}

short Bomberman_GetGridY(Bomberman* bomberman) {
	return World_ToGrid(bomberman->y + 10);
}

short Bomberman_GetLives(Bomberman* bomberman) {
	return (bomberman->data & 0xf000) >> 12;
}

short Bomberman_GetBombs(Bomberman* bomberman) {
	return (bomberman->data & 0x0f00) >> 8;
}

void Bobmerman_AddBomb(Bomberman* bomberman) {
	if(Bomberman_GetBombs(bomberman) < 15)
		bomberman->data += 0x0100;
	if(bomberman == World_GetBombermen()[0])
		World_DrawPlayerStats();
}

short Bomberman_GetRange(Bomberman* bomberman) {
	return (bomberman->data & 0x00f0) >> 4;
}

short Bomberman_GetShoes(Bomberman* bomberman) {
	return (bomberman->data & 0x000f);
}

short Bomberman_GetSpeed(Bomberman* bomberman) {
	short s = Bomberman_GetShoes(bomberman);
	
	if(s > 4)
		return 4;
	if(s > 1)
		return 3;
	return 2;
}

/*
 * struct related functions for AI
 */

_Bool AIBehavior_DidSearchRandom(Bomberman* bomberman, short x, short y) {
	return (bomberman->AI_mvdata & 0xff) == (x << 4) + y;
}

void AIBehavior_DoSearchRandom(Bomberman* bomberman, short x, short y) {
	bomberman->AI_mvdata &= 0xff00;
	bomberman->AI_mvdata += (x << 4) + y;
}

_Bool BombermanAI_HasTarget(Bomberman* bomberman) {
	return (bomberman->AI_target & 0xf000) > 0 && bomberman->AI_path != NULL;
}

short BombermanAI_GetTargetX(Bomberman* bomberman) {
	return (bomberman->AI_target & 0x00f0) >> 4;
}

short BombermanAI_GetTargetY(Bomberman* bomberman) {
	return bomberman->AI_target & 0x000f;
}

void BombermanAI_SetTarget(Bomberman* bomberman, short x, short y) {
	BombermanAI_RemoveTarget(bomberman);
	bomberman->AI_target = (bomberman->AI_target & 0x0fff) + 0x1000;
	bomberman->AI_target = (bomberman->AI_target & 0xff0f) + (x << 4);
	bomberman->AI_target = (bomberman->AI_target & 0xfff0) + y;
}

void BombermanAI_SetTargetData(Bomberman* bomberman, int target) {
	BombermanAI_RemoveTarget(bomberman);
	bomberman->AI_target = (target & 0x0fff) + 0x1000;
}

void BombermanAI_RemoveTarget(Bomberman* bomberman) {
	bomberman->AI_target = 0x0000;
	if(bomberman->AI_path)
		List_Free(bomberman->AI_path);
	bomberman->AI_path = NULL;
}

_Bool BombermanAI_PlaceBombAtTarget(Bomberman* bomberman) {
	return (bomberman->AI_target & 0x0f00) == 0x0100 || (bomberman->AI_target & 0x0f00) == 0x0300;
}

short BombermanAI_GetDirection(Bomberman* bomberman) {
	return bomberman->AI_data & 0x000f;
}

void BombermanAI_SetDirection(Bomberman* bomberman, short direction) {
	bomberman->AI_data = (bomberman->AI_data & 0xfff0) + direction;
}

_Bool BombermanAI_IsWaiting(Bomberman* bomberman) {
	return (bomberman->AI_data & 0xff00) > 0;
}

void BombermanAI_Wait(Bomberman* bomberman, short ticks) {
	bomberman->AI_data = (bomberman->AI_data & 0x00ff) + (ticks << 8);
}