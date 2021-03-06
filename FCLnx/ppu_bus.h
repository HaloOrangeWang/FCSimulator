#ifndef PPU_BUS_H
#define PPU_BUS_H

#include <stdint.h>

#define NAMETABLE_HORIZONTAL 0
#define NAMETABLE_VIRTICAL 1
#define NAMETABLE_ONESCREEN_LOWER 9
#define NAMETABLE_ONESCREEN_HIGHER 10

//1个精灵的数据
struct OAM_1Sprite
{
    uint8_t loc_x; //精灵的横坐标
    uint8_t loc_y; //精灵的纵坐标
    uint16_t patterntable_addr; //这个精灵使用的Pattern Table的起始地址
    uint8_t palette_dx; //这个精灵使用哪个调色板
    bool behind_background; //这个精灵是否在背景之后显示
    bool flip_x; //在x轴上是否翻转
    bool flip_y; //在y轴上是否翻转
};

//全部64个精灵的OAM数据
class OAM_RAM
{
public:
    uint8_t data[256];
    OAM_1Sprite load_1sprite(uint8_t spr_id); //使用8*8的Sprite时，sprite的属性解析方法
    OAM_1Sprite load_1sprite_long(uint8_t spr_id); //使用8*16的Sprite时，sprite的属性解析方法
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
