#ifndef PPU_BUS_H
#define PPU_BUS_H

#include <stdint.h>

#define NAMETABLE_HORIZONTAL 0
#define NAMETABLE_VIRTICAL 1
#define NAMETABLE_ONESCREEN_LOWER 9
#define NAMETABLE_ONESCREEN_HIGHER 10

//
struct OAM_1Sprite
{
    uint8_t loc_x; //
    uint8_t loc_y; //
    uint16_t patterntable_addr; //
    uint8_t palette_dx; //
    bool behind_background; //
    bool flip_x; //
    bool flip_y; //
};

//
class OAM_RAM
{
public:
    uint8_t data[256];
    OAM_1Sprite load_1sprite(uint8_t spr_id); //
    OAM_1Sprite load_1sprite_long(uint8_t spr_id); //
};

class PictureBus
{
public:
    uint8_t ram_data[0x400];
    uint8_t ram_data_2[0x400];
    uint8_t palette_data[0x20];
    void save(uint16_t addr, uint8_t data);
    uint8_t load(uint16_t addr);
    PictureBus();
private:
    void get_nametable_addr(uint16_t addr, uint8_t& nametable_id, uint16_t& nametable_addr);
};

#endif
