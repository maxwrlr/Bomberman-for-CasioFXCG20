#include "key.h"

_Bool Key_IsDown(int keycode)
{
	const unsigned short* keyregister = (unsigned short*)0xA44B0000;
	int row, col, word, bit;
	row = keycode % 10;
	col = keycode / 10 - 1;
	word = row >> 1;
	bit = col + ((row&1) << 3);
	return (keyregister[word] & 1 << bit) != 0;
}
