#include "util.h"
#include "sdata.h"
#include <tonc.h>

int next_type;

void set_random_blocks(Block4 *curr, int *frame, int field[][10])
{
    int type = next_type;
    next_type = qran_range(0, 7);

    *curr = (Block4){type, 0, (Pos){APPEAR_REL_POS_X, APPEAR_REL_POS_Y}};
    Pos ghost_pos = get_ghost_pos(*curr, field);

    for (int i = 0; i < 4; i++)
    {
        // Set current block
        Pos pos = rel2abs(APPEAR_REL_POS_X + SHAPES[type][0][i][0], APPEAR_REL_POS_Y + SHAPES[type][0][i][1]);
        obj_set_attr(&oam_mem[i],
            ATTR0_8BPP | ATTR0_SQUARE,
            ATTR1_SIZE_8,
            ATTR2_ID(type<<1)
        );
        obj_set_pos(&oam_mem[i], pos.x, pos.y);

        // Set ghost block
        Pos ghost_pos_real = rel2abs(ghost_pos.x + SHAPES[type][0][i][0], ghost_pos.y + SHAPES[type][0][i][1]);
        obj_set_attr(&oam_mem[i+4],
            ATTR0_8BPP | ATTR0_SQUARE,
            ATTR1_SIZE_8,
            ATTR2_ID(7<<1) | ATTR2_PRIO(1)
        );
        obj_set_pos(&oam_mem[i+4], ghost_pos_real.x, ghost_pos_real.y);

        // Set next block
        Pos next_pos = rel2abs(NEXT_REL_POS_X + SHAPES[next_type][0][i][0], NEXT_REL_POS_Y + SHAPES[next_type][0][i][1]);
        obj_set_attr(&oam_mem[i+8],
            ATTR0_8BPP | ATTR0_SQUARE,
            ATTR1_SIZE_8,
            ATTR2_ID(next_type<<1)
        );
        obj_set_pos(&oam_mem[i+8], next_pos.x, next_pos.y);
    }

    *frame = 30;  // Immediately trigger fall detection
}

void move_block(Block4 *curr, int dx, int dy, int field[][10])
{
    curr->pos.x += dx;
    curr->pos.y += dy;
    Pos ghost_pos = get_ghost_pos(*curr, field);

    for (int i = 0; i < 4; i++)
    {
        obj_set_pos(&oam_mem[i],
            (oam_mem[i].attr1 & ATTR1_X_MASK) + (dx << 3),
            (oam_mem[i].attr0 & ATTR0_Y_MASK) + (dy << 3)
        );

        Pos ghost_pos_real = rel2abs(ghost_pos.x + SHAPES[curr->type][curr->rot][i][0], ghost_pos.y + SHAPES[curr->type][curr->rot][i][1]);
        obj_set_pos(&oam_mem[i+4], ghost_pos_real.x, ghost_pos_real.y);
    }
}

void to_the_bottom(Block4 *curr, int field[][10])
{
    Block4 curr_copy = *curr;
    while (can_down(curr_copy, field))
    {
        curr_copy.pos.y++;
    }
    move_block(curr, 0, curr_copy.pos.y - curr->pos.y, field);
}

Pos get_ghost_pos(Block4 curr, int field[][10])
{
    while (can_down(curr, field))
    {
        curr.pos.y++;
    }
    return curr.pos;
}

void stick_block(Block4 curr, int field[][10])
{
    for (int i = 0; i < 4; i++)
    {
        Pos rel_pos = {curr.pos.x + SHAPES[curr.type][curr.rot][i][0], curr.pos.y + SHAPES[curr.type][curr.rot][i][1]};
        field[rel_pos.y][rel_pos.x] = 1;
        if (rel_pos.y >= 0)
        {
            memcpy16(&se_mem[31][(rel_pos.y<<5)+10+rel_pos.x], &se_mem[31][BLOCK_BASE_SE+curr.type], 1);
        }
    }
}

bool can_down(Block4 curr, int field[][10])
{
    for (int i = 0; i < 4; i++)
    {  // Every block in total 4 blocks
        Pos rel_pos = {curr.pos.x + SHAPES[curr.type][curr.rot][i][0], curr.pos.y + SHAPES[curr.type][curr.rot][i][1]};
        if (rel_pos.y >= 19 || field[rel_pos.y + 1][rel_pos.x] != 0)
            return false;
    }
    return true;
}

bool can_left(Block4 curr, int field[][10])
{
    for (int i = 0; i < 4; i++)
    {
        Pos rel_pos = {curr.pos.x + SHAPES[curr.type][curr.rot][i][0], curr.pos.y + SHAPES[curr.type][curr.rot][i][1]};
        if (rel_pos.x <= 0 || field[rel_pos.y][rel_pos.x - 1] != 0)
            return false;
    }
    return true;
}

bool can_right(Block4 curr, int field[][10])
{
    for (int i = 0; i < 4; i++)
    {
        Pos rel_pos = {curr.pos.x + SHAPES[curr.type][curr.rot][i][0], curr.pos.y + SHAPES[curr.type][curr.rot][i][1]};
        if (rel_pos.x >= 9 || field[rel_pos.y][rel_pos.x + 1] != 0)
            return false;
    }
    return true;
}

void rot_if_possible(Block4 *curr, int field[][10])
{
    int rot = (curr->rot + 1) % 4;
    Pos pos[4];
    for (int i = 0; i < 4; i++)
    {
        Pos rel_pos = {curr->pos.x + SHAPES[curr->type][rot][i][0], curr->pos.y + SHAPES[curr->type][rot][i][1]};
        pos[i] = rel_pos;
        if (field[rel_pos.y][rel_pos.x] != 0 || rel_pos.y > 19 || rel_pos.x < 0 || rel_pos.x > 9)
            return;
    }

    curr->rot = rot;
    Pos ghost_pos = get_ghost_pos(*curr, field);

    for (int i = 0; i < 4; i++)
    {
        Pos real_pos = rel2abs(pos[i].x, pos[i].y);
        obj_set_pos(&oam_mem[i], real_pos.x, real_pos.y);

        Pos ghost_pos_real = rel2abs(ghost_pos.x + SHAPES[curr->type][curr->rot][i][0], ghost_pos.y + SHAPES[curr->type][curr->rot][i][1]);
        obj_set_pos(&oam_mem[i+4], ghost_pos_real.x, ghost_pos_real.y);
    }
}

void clear_lines(Block4 curr, int field[][10], int *score)
{
    bool emptyLines[20] = {false};
    bool fullLines[20] = {false};
    bool anyFullLine = false;

    // Check empty lines and full lines
    for (int i = 0; i < 4; i++)
    {
        int y = curr.pos.y + SHAPES[curr.type][curr.rot][i][1];
        if (fullLines[y] || emptyLines[y]) continue;

        int count = 0;
        for (int j = 0; j < 10; j++)
        {
            if (field[y][j] != 0)
                count++;
        }

        if (count == 10)
        {
            fullLines[y] = true;
            anyFullLine = true;
        }
        else if (count == 0)
        {
            emptyLines[y] = true;
        }
    }

    if (!anyFullLine) return;

    // zip lines using double pointer
    int write = 19;
    int scoreAdd = 0;
    for (int read = 19; read >= -4; read--)
    {
        if (fullLines[read])
        {
            scoreAdd += 10;
            *score += scoreAdd;
            continue;
        }
        if (emptyLines[read]) break;
        if (read != write)
        {
            for (int x = 0; x < 10; x++) field[write][x] = field[read][x];
            memcpy16(&se_mem[31][(write<<5)+10], &se_mem[31][(read<<5)+10], 10);
        }
        write--;
    }

    tte_printf("#{P:8,24}%d", *score);

    // Empty the 4 lines above (bc most 4 lines can be cleared one time)
    for (int i = 0; i < 4 && write >= -4; i++, write--)
    {
        // se_mem[31][31<<5] is the empty line (the last line)
        memcpy16(&se_mem[31][(write<<5)+10], &se_mem[31][31<<5], 10);
    }
}

bool is_game_over(int field[][10])
{
    for (int i = 0; i < 10; i++)
    {
        if (field[-1][i] != 0)
            return true;
    }
    return false;
}

void reset_game(int field[][10], int *score)
{
    *score = 0;
    for (int i = -4; i < 20; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            field[i][j] = 0;
        }
        if (i >= 0)
        {
            memcpy16(&se_mem[31][(i<<5)+10], &se_mem[31][31<<5], 10);
        }
    }
}
