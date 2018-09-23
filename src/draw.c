#include <display_syscalls.h>
#include <color.h>

#include "bomberman.h"
#include "bombs.h"
#include "draw.h"
#include "world.h"
#include "bitmaps.h"

void Draw_FillScreen(unsigned short color) {
	Draw_Rectangle(0, 0, LCD_WIDTH_PX, LCD_HEIGHT_PX, color);
}

void Draw_Rectangle(short x, short y, short w, short h, unsigned short color) {
	for(short i = x; i < x + w; i++)
		for(short j = y; j < y + h; j++)
			*(VRAM_Address + j * LCD_WIDTH_PX + i) = color;
}

void Draw_Bitmap(short x, short y, short w, short h, const color_t *bmp) {
	short bx = 0, by = 0;
	
	if(y < 0)
		by -= y;
	
	for(short i = bx; i < w; i++)
		for(short j = by; j < h; j++)
			*(VRAM_Address + (y + j) * LCD_WIDTH_PX + (x + i)) = bmp[j * w + i];
}

void Draw_AlphaBitmap(short x, short y, short w, short h, const color_t *bmp, unsigned short alpha) {
	short bx = 0, by = 0;
	
	if(y < 0)
		by -= y;
	
	for(short i = bx; i < w; i++)
		for(short j = by; j < h; j++) {
			color_t curr = bmp[j * w + i];
			if(curr == alpha)
				continue;
			
			*(VRAM_Address + (y + j) * LCD_WIDTH_PX + (x + i)) = curr;
		}
}

void Draw_AlphaBitmap90deg(short x, short y, short w, short h, const color_t *bmp, unsigned short alpha) {
	short bx = 0, by = 0, base = (short) ((h - 1) * w);
	
	if(y < 0)
		by -= y;
	
	for(short i = bx; i < h; i++)
		for(short j = by; j < w; j++) {
			color_t curr = bmp[base - i * w + j];
			if(curr == alpha)
				continue;
			
			*(VRAM_Address + (y + j) * LCD_WIDTH_PX + (x + i)) = curr;
		}
}

void Draw_AlphaBitmap180deg(short x, short y, short w, short h, const color_t *bmp, unsigned short alpha) {
	short bx = 0, by = 0;
	
	if(y < 0)
		by -= y;
	
	for(short i = bx; i < w; i++)
		for(short j = by; j < h; j++) {
			color_t curr = bmp[(h - 1 - j) * w + (w - 1 - i)];
			if(curr == alpha)
				continue;
			
			*(VRAM_Address + (y + j) * LCD_WIDTH_PX + (x + i)) = curr;
		}
}

void Draw_AlphaBitmap270deg(short x, short y, short w, short h, const color_t *bmp, unsigned short alpha) {
	short bx = 0, by = 0;
	
	if(y < 0)
		by -= y;
	
	for(short i = bx; i < w; i++)
		for(short j = by; j < h; j++) {
			color_t curr = bmp[(i + 1) * w - (j + 1)];
			if(curr == alpha)
				continue;
			
			*(VRAM_Address + (y + j) * LCD_WIDTH_PX + (x + i)) = curr;
		}
}

/*
 *
 */

void Draw_BombermanTitle() {
	Draw_Bitmap((LCD_WIDTH_PX - 248) / 2, (LCD_HEIGHT_PX - 44) / 2, 248, 44, bomberman_title);
}

void Draw_BombermanTitleTop() {
	Draw_Bitmap((LCD_WIDTH_PX - 248) / 2, 15, 248, 44, bomberman_title);
}

void Draw_Text(int x, int y, unsigned char* text, unsigned short color) {
	PrintMini(&x, &y, text, 0x40, 0xffffffff, 0, 0, color, COLOR_BLACK, 1, 0);
}

void Draw_Button(int y, int w, int px, unsigned char* text, _Bool hovered) {
	int x = (LCD_WIDTH_PX - w) / 2, h = 28;

	if(hovered) {
		Draw_Rectangle(x - 2, y - 2, w + 4, 1, COLOR_BLACK);
		Draw_Rectangle(x - 2, y - 2, 1, h + 4, COLOR_BLACK);
		Draw_Rectangle(x + w + 1, y - 2, 1, h + 4, COLOR_BLACK);
		Draw_Rectangle(x - 2, y + h + 1, w + 4, 1, COLOR_BLACK);
	}
	Draw_Rectangle(x, y, w, h, hovered ? COLOR_BLACK : 0x2965);

	x += 20 + px;
	y += 6;
	PrintMini(&x, &y, text, 0x40, 0xffffffff, 0, 0, COLOR_WHITE, hovered ? COLOR_BLACK : 0x2965, 1, 0);
}

void Draw_Copyright() {
	unsigned char copyright[] = {'(', 'C', ')', ' ', 'M', 'a', 'x', ' ', 'W', 'e', 'r', 'l', 'e', 'r', '\0'};

	int x = (LCD_WIDTH_PX - 142) / 2, y = LCD_HEIGHT_PX - 18;
	PrintMini(&x, &y, copyright, 0x40, 0xffffffff, 0, 0, 0x08f2, 0xffff, 1, 0);
}

/*
 *
 */

void Draw_Stone(short x, short y) {
	Draw_Bitmap(WORLD_X0 + 24 * x - 1, 24 * y - 1, 25, 25, stone);
}

void Draw_Wood(short x, short y) {
	if((x + y) % 2 == 0) {
		Draw_Bitmap(WORLD_X0 + 24 * x - 1, 24 * y - 1, 25, 25, wood);
	} else
		Draw_AlphaBitmap90deg(WORLD_X0 + 24 * x - 1, 24 * y - 1, 25, 25, wood, 0xffff);
}

void Draw_Bomberman(void *ptr, const color_t *bmp) {
	Bomberman* bomberman = ptr;
	if((bomberman->ticksHit / 5) % 2 == 0) {
		switch(bomberman->rotation) {
			case ROTATION_N:
				Draw_AlphaBitmap(WORLD_X0 + bomberman->x, bomberman->y, ENTITY_SIZE, ENTITY_SIZE, bmp, 0x0000);
				break;
			case ROTATION_E:
				Draw_AlphaBitmap90deg(WORLD_X0 + bomberman->x, bomberman->y, ENTITY_SIZE, ENTITY_SIZE, bmp, 0x0000);
				break;
			case ROTATION_S:
				Draw_AlphaBitmap180deg(WORLD_X0 + bomberman->x, bomberman->y, ENTITY_SIZE, ENTITY_SIZE, bmp, 0x0000);
				break;
			case ROTATION_W:
				Draw_AlphaBitmap270deg(WORLD_X0 + bomberman->x, bomberman->y, ENTITY_SIZE, ENTITY_SIZE, bmp, 0x0000);
				break;
		}
	}
}

void Draw_ClearBomberman(void *ptr) {
	Bomberman* bomberman = ptr;
	Draw_Rectangle(WORLD_X0 + bomberman->x, bomberman->y, ENTITY_SIZE, ENTITY_SIZE, WORLD_COLOR_BG);
}

/*
 *
 */

color_t* Bitmap_GetBombermanRed() {
	return bomberman_red;
}

color_t* Bitmap_GetBombermanYellow() {
	return bomberman_yellow;
}

color_t* Bitmap_GetBombermanBlue() {
	return bomberman_blue;
}

color_t* Bitmap_GetBombermanPurple() {
	return bomberman_purple;
}

color_t* Bitmap_GetBomb0() {
	return bomb_0;
}

color_t* Bitmap_GetBomb1() {
	return bomb_1;
}

color_t* Bitmap_GetBomb2() {
	return bomb_2;
}

color_t* Bitmap_GetBomb3() {
	return bomb_3;
}

color_t* Bitmap_GetBomb4() {
	return bomb_4;
}

color_t* Bitmap_GetRange0() {
	return range_0;
}

color_t* Bitmap_GetRange1() {
	return range_1;
}

color_t* Bitmap_GetRange2() {
	return range_2;
}

color_t* Bitmap_ClearRange1() {
	return crange_1;
}

color_t* Bitmap_ClearRange2() {
	return crange_2;
}

color_t* Bitmap_getItemHeart() {
	return item_heart;
}

color_t* Bitmap_getItemBomb() {
	return item_bomb;
}

color_t* Bitmap_getItemRange() {
	return item_range;
}

color_t* Bitmap_getItemShoes() {
	return item_shoes;
}