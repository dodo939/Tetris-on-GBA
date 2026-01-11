#ifndef UTIL_H
#define UTIL_H
#include <stdbool.h>
#define BASE_X 80
#define BASE_Y 0
#define APPEAR_REL_POS_X 3
#define APPEAR_REL_POS_Y -2
#define BLOCK_BASE_SE 640  // Where the block tiles start in bg.bmp
#define NEXT_REL_POS_X 13
#define NEXT_REL_POS_Y 3

extern int next_type;

typedef struct
{
    int x;
    int y;
} Pos;

typedef struct
{
    int type;
    int rot;
    Pos pos;
} Block4;


static inline Pos rel2abs(int x, int y)
{ return (Pos){BASE_X + (x << 3), BASE_Y + (y << 3)}; }

void set_random_blocks(Block4 *curr, int *frame, int field[][10]);

void move_block(Block4 *curr, int dx, int dy, int field[][10]);
void to_the_bottom(Block4 *curr, int field[][10]);

Pos get_ghost_pos(Block4 curr, int field[][10]);

void stick_block(Block4 curr, int field[][10]);

bool can_down(Block4 curr, int field[][10]);
bool can_left(Block4 curr, int field[][10]);
bool can_right(Block4 curr, int field[][10]);
void rot_if_possible(Block4 *curr, int field[][10]);

void clear_lines(Block4 curr, int field[][10], int *score);

bool is_game_over(int field[][10]);

void reset_game(int field[][10], int *score);

#endif
