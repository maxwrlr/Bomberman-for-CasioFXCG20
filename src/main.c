#include "display.h"

#include "key.h"
#include "bomberman.h"
#include "bombs.h"
#include "world.h"
#include "menu.h"

int main(void) {
	int state = 0, winner = -1;
	_Bool running = 1, firstDraw = 1;

	while(running) {
		int previous = state;

		switch (state) {
			case 0:
				state = Menu_Intro(firstDraw);
				break;
			case 1:
				state = Menu_Main(firstDraw);
				if(state > 2) {
					if(!World_Init((state & 0x00f0) >> 4)) {
						state = 0;
					} else
						state = 2;
				}
				break;
			case 2:
				state = World_Run(firstDraw);
				if(state >= 4) {
					winner = (state & 0x00f0) >> 4;
					state &= 0x000f;
				}
				break;
			case 3:
				state = Menu_Pause(firstDraw);
				break;
			case 4:
				state = Menu_Finish(firstDraw, winner);
				break;
			case 9:
				World_CleanUp();
				state = 1;
				break;
			default: // actually redundant
				running = 0;
		}

		if(state != previous) {
			firstDraw = 1;
		} else
			firstDraw = 0;

		if(Key_IsDown(KEY_MENU)) {
			Menu_Quit();
			firstDraw = 1;
			if(state != 2 && state != 3)
				state = 0;
		}

		Bdisp_PutDisp_DD();
	}
}
