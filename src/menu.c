#include <display.h>
#include <display_syscalls.h>
#include <keyboard_syscalls.h>
#include <color.h>
#include <string.h>

#include "menu.h"
#include "draw.h"
#include "key.h"

int Menu_Intro(_Bool firstDraw) {
	static int timeLeft = 30;


	if(firstDraw) {
		timeLeft = 30;

		Draw_FillScreen(COLOR_WHITE);
		DrawFrame(COLOR_WHITE);
		Bdisp_EnableColor(1);

		Draw_BombermanTitle();
		Draw_Copyright();

	} else
	if(timeLeft == 0) {
		return 1;
	} else
		timeLeft--;


	return 0;
}

int Menu_Main(_Bool firstDraw) {
	static int selected = 1;
	static _Bool wasExeDown = 1, wasUpDown = 0, wasDownDown = 0;
	_Bool nothing = !firstDraw;

	if(!wasUpDown && Key_IsDown(KEY_UP)) {
		wasUpDown = 1;
		selected--;
		if(selected < 1)
			selected = 3;
		nothing = 0;
	} else
	if(wasUpDown && !Key_IsDown(KEY_UP))
		wasUpDown = 0;

	if(!wasDownDown && Key_IsDown(KEY_DOWN)) {
		wasDownDown = 1;
		selected++;
		if(selected > 3)
			selected = 1;
		nothing = 0;
	} else
	if(wasDownDown && !Key_IsDown(KEY_DOWN))
		wasDownDown = 0;

	if(!wasExeDown && Key_IsDown(KEY_EXE)) {
		wasExeDown = 1;
		DrawFrame(0x0000);
		Bdisp_PutDisp_DD();

		if(selected == 4) {
			return -1;
		} else
			return 2 + (selected << 4);
	} else
	if(wasExeDown && !Key_IsDown(KEY_EXE))
		wasExeDown = 0;

	if(nothing)
		return 1;

	Draw_FillScreen(COLOR_WHITE);
	DrawFrame(COLOR_WHITE);
	Bdisp_EnableColor(1);

	Draw_BombermanTitleTop();

	int y = (LCD_HEIGHT_PX - 140 - 18) / 2 + 48;
	for(int i = 1; i < 4; i++) {
		unsigned char text[] = {(unsigned char) (48 + i), ' ', 'G', 'e', 'g', 'n', 'e', 'r', '\0'};
		Draw_Button(y, 128, 0, text, selected == i);
		y += 36;
	}

	Draw_Copyright();

	return 1;
}

int Menu_Pause(_Bool firstDraw) {
	static int selected = 1;
	static _Bool wasExeDown = 1, wasUpDown = 0, wasDownDown = 0;

	if(!wasUpDown && Key_IsDown(KEY_UP)) {
		wasUpDown = 1;
		selected--;
		if(selected < 1)
			selected = 2;
		firstDraw = 1;
	} else
	if(wasUpDown && !Key_IsDown(KEY_UP))
		wasUpDown = 0;

	if(!wasDownDown && Key_IsDown(KEY_DOWN)) {
		wasDownDown = 1;
		selected++;
		if(selected > 2)
			selected = 1;
		firstDraw = 1;
	} else
	if(wasDownDown && !Key_IsDown(KEY_DOWN))
		wasDownDown = 0;

	if(!wasExeDown && Key_IsDown(KEY_EXE)) {
		wasExeDown = 1;

		if(selected == 1) {
			DrawFrame(0x0000);
			Bdisp_PutDisp_DD();
			return 2;
		} else {
			return 9;
		}
	} else
	if(wasExeDown && !Key_IsDown(KEY_EXE))
		wasExeDown = 0;

	if(!firstDraw)
		return 3;

	Draw_FillScreen(COLOR_WHITE);
	DrawFrame(COLOR_WHITE);
	Bdisp_EnableColor(1);

	PrintCXY((LCD_WIDTH_PX - 85) / 2, 30, "Pause", TEXT_MODE_NORMAL, -1, COLOR_BLACK, COLOR_WHITE, 1, 0);

	int y = (LCD_HEIGHT_PX - 140 - 18) / 2 + 48 + 36;
	unsigned char text[] = {'W', 'e', 'i', 't', 'e', 'r', '\0'};
	Draw_Button(y, 128, 11, text, selected == 1);
	y += 36;

	unsigned char menu[] = {'H', 'a', 'u', 'p', 't', 'm', 'e', 'n', 'u', '\0'};
	Draw_Button(y, 128, -5, menu, selected == 2);

	Draw_Copyright();

	return 3;
}

int Menu_Finish(_Bool firstDraw, int winner) {

	if(!firstDraw) {

		if(Key_IsDown(KEY_EXE))
			return 1;

		return 4;
	}

	Draw_FillScreen(COLOR_WHITE);
	DrawFrame(COLOR_WHITE);
	Bdisp_EnableColor(1);


	int y = 80, type = 0;
	switch(winner) {
		case 1: PrintCXY((LCD_WIDTH_PX - 40) / 2, 30, "Du", TEXT_MODE_NORMAL, -1, 0x00f3, COLOR_WHITE, 1, 0); type = 1; break;
		case 2: PrintCXY((LCD_WIDTH_PX - 51) / 2, 30, "Rot", TEXT_MODE_NORMAL, -1, 0xc0e0, COLOR_WHITE, 1, 0); break;
		case 3: PrintCXY((LCD_WIDTH_PX - 68) / 2, 30, "Lila", TEXT_MODE_NORMAL, -1, 0xb0f6, COLOR_WHITE, 1, 0); break;
		case 4: PrintCXY((LCD_WIDTH_PX - 68) / 2, 30, "Gelb", TEXT_MODE_NORMAL, -1, 0xd480, COLOR_WHITE, 1, 0); break;
		default: PrintCXY((LCD_WIDTH_PX - 102) / 2, 30, "Keiner", TEXT_MODE_NORMAL, -1, COLOR_BLACK, COLOR_WHITE, 1, 0); break;
	}

	int x = (LCD_WIDTH_PX - (type == 1 ? 151 : 140)) / 2;
	PrintMini(&x, &y, type == 1 ? "hast gewonnen!" : "hat gewonnen.", 0x40, 0xffffffff, 0, 0, COLOR_BLACK, COLOR_WHITE, 1, 0);

	unsigned char text[] = {'W', 'e', 'i', 't', 'e', 'r', '\0'};
	Draw_Button(139, 128, 11, text, 1);

	Draw_Copyright();

	return 4;
}

void Menu_Quit() {
	int key = KEY_MENU;
	GetKey(&key);
	DrawFrame(0x0000);
	Bdisp_EnableColor(1);
}