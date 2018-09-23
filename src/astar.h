#ifndef _ASTAR
#define _ASTAR

#define square(x) ((x) * (x))
#define dist(x1, y1, x2, y2) (square((x2) - (x1)) + square((y2) - (y1)))

typedef struct Coords {
	short coords;
	struct Coords* next;
} CoordList;

_Bool List_Contains(CoordList*, short);
_Bool List_Push(CoordList*, short);
void List_Pop(CoordList*);
void List_Free(CoordList*);

CoordList* AStar_Deeper(short, short, short, int, CoordList*);
CoordList* AStar_Next(short, short, short, short);
CoordList* AStar_GetNext(short, short, short, int, CoordList*);

#endif //_ASTAR
