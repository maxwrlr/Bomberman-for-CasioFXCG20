#ifndef _DRAW
#define _DRAW

#define LCD_WIDTH_PX 384
#define LCD_HEIGHT_PX 216

void Draw_FillScreen(unsigned short);
void Draw_Rectangle(short, short, short, short, unsigned short);
void Draw_Bitmap(short, short, short, short, const color_t*);
void Draw_AlphaBitmap(short, short, short, short, const color_t*, unsigned short);
void Draw_AlphaBitmap90deg(short, short, short, short, const color_t*, unsigned short);
void Draw_AlphaBitmap180deg(short, short, short, short, const color_t*, unsigned short);
void Draw_AlphaBitmap270deg(short, short, short, short, const color_t*, unsigned short);

void Draw_BombermanTitle();
void Draw_BombermanTitleTop();
void Draw_Text(int, int, unsigned char*, unsigned short);
void Draw_Button(int, int, int, unsigned char*, _Bool);
void Draw_Copyright();

void Draw_Stone(short, short);
void Draw_Wood(short, short);
void Draw_Bomberman(void*, const color_t*);
void Draw_ClearBomberman(void*);

color_t* Bitmap_GetBombermanRed();
color_t* Bitmap_GetBombermanYellow();
color_t* Bitmap_GetBombermanBlue();
color_t* Bitmap_GetBombermanPurple();

color_t* Bitmap_GetBomb0();
color_t* Bitmap_GetBomb1();
color_t* Bitmap_GetBomb2();
color_t* Bitmap_GetBomb3();
color_t* Bitmap_GetBomb4();

color_t* Bitmap_GetRange0();
color_t* Bitmap_GetRange1();
color_t* Bitmap_GetRange2();
color_t* Bitmap_ClearRange1();
color_t* Bitmap_ClearRange2();

color_t* Bitmap_getItemHeart();
color_t* Bitmap_getItemBomb();
color_t* Bitmap_getItemRange();
color_t* Bitmap_getItemShoes();

#endif //_DRAW
