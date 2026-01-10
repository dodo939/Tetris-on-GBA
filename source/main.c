#include <tonc.h>
#include "bg.h"
#include "blocks.h"
#include "util.h"

int _field[24][10] = {0};
int (*field)[10] = _field + 4;

int main()
{
    irq_init(NULL);
    irq_add(II_VBLANK, NULL);
    int frame = 0;
    Block4 curr;

    // Load bg tile, palette and map
    memcpy16(pal_bg_mem, bgPal, bgPalLen / sizeof(u16));
    memcpy32(&tile_mem[0][0], bgTiles, bgTilesLen / sizeof(u32));
    memcpy32(&se_mem[31][0], bgMap, bgMapLen / sizeof(u32));

    // Load block tiles
    memcpy16(pal_obj_mem, blocksPal, blocksPalLen / sizeof(u16));
    memcpy32(&tile_mem[4][0], blocksTiles, blocksTilesLen / sizeof(u32));

    oam_init(obj_mem, 128);
    REG_DISPCNT = DCNT_MODE0 | DCNT_BG0 | DCNT_OBJ | DCNT_OBJ_1D;
    REG_BG0CNT= BG_CBB(0) | BG_SBB(31) | BG_8BPP | BG_REG_32x32;

    set_random_blocks(&curr, &frame, field);

    while (1)
    {
        frame++;
        key_poll();
        VBlankIntrWait();
        if (key_hit(KEY_LEFT) && can_left(curr, field))
        {
            move_block(&curr, -1, 0, field);
        }
        if (key_hit(KEY_RIGHT) && can_right(curr, field))
        {
            move_block(&curr, 1, 0, field);
        }
        if (key_hit(KEY_A | KEY_UP))
        {
            rot_if_possible(&curr, field);
        }
        if (key_hit(KEY_B))
        {
            to_the_bottom(&curr, field);
            frame = 30;
        }
        if (key_is_down(KEY_DOWN))
        {
            frame += 5;
        }
        if (frame >= 30)
        {
            frame = 0;
            if (can_down(curr, field))
            {
                move_block(&curr, 0, 1, field);
            }
            else
            {
                stick_block(curr, field);
                clear_lines(curr, field);
                if (is_game_over(field))
                {
                    while (1) VBlankIntrWait();
                }
                set_random_blocks(&curr, &frame, field);
            }
        }
    }
    return 0;
}
