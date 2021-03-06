#include "total.h"
#include <QDebug>

PictureBus::PictureBus(){
    memset(ram_data, 0, sizeof(uint8_t) * 0x400);
    memset(ram_data_2, 0, sizeof(uint8_t) * 0x400);
    memset(palette_data, 0, sizeof(uint8_t) * 0x20);
}

void PictureBus::save(uint16_t addr, uint8_t data){
    if (addr <= 0x1fff){
        cartridge.mapper_ptr->ppu_write_pt(addr, data);
    }else if (addr >= 0x2000 && addr <= 0x3eff){
        uint8_t nametable_id;
        uint16_t nametable_addr;
        get_nametable_addr(addr, nametable_id, nametable_addr);
        if (nametable_id == 1)
            ram_data_2[nametable_addr] = data;
        else
            ram_data[nametable_addr] = data;
    }else if (addr >= 0x3f00 && addr <= 0x3fff){
        uint8_t palette_addr = addr & 0x1f;
        if (palette_addr == 0x10)
            palette_addr = 0x0;
        else if (palette_addr == 0x14)
            palette_addr = 0x4;
        else if (palette_addr == 0x18)
            palette_addr = 0x8;
        else if (palette_addr == 0x1c)
            palette_addr = 0xc;
        palette_data[palette_addr] = data;
    }
}


uint8_t PictureBus::load(uint16_t addr){
    if (addr < 0x2000){
        return cartridge.mapper_ptr->ppu_read_pt(addr);
    }else if (addr >= 0x2000 && addr <= 0x3eff){
        uint8_t nametable_id;
        uint16_t nametable_addr;
        get_nametable_addr(addr, nametable_id, nametable_addr);
        if (nametable_id == 1)
            return ram_data_2[nametable_addr];
        else
            return ram_data[nametable_addr];
    }else if (addr >= 0x3f00 && addr <= 0x3fff){
        uint8_t palette_addr = addr & 0x1f;
        return palette_data[palette_addr];
    }else{
        qDebug() << "PictureBus::load, 取值的地址应当在0-0x4000之间" << endl;
        return 0x00;
    }
}

void PictureBus::get_nametable_addr(uint16_t addr, uint8_t& nametable_id, uint16_t& nametable_addr){
    switch(cartridge.mapper_ptr->nametable_mirror){
    case NAMETABLE_HORIZONTAL:
        if (addr & 0x800)
            nametable_id = 1;
        else
            nametable_id = 0;
        nametable_addr = addr & 0x3ff;
        break;
    case NAMETABLE_VIRTICAL:
        if (addr & 0x400)
            nametable_id = 1;
        else
            nametable_id = 0;
        nametable_addr = addr & 0x3ff;
        break;
    case NAMETABLE_ONESCREEN_LOWER:
        nametable_id = 0;
        nametable_addr = addr & 0x3ff;
        break;
    case NAMETABLE_ONESCREEN_HIGHER:
        nametable_id = 1;
        nametable_addr = addr & 0x3ff;
        break;
    default:
        qDebug() << "不支持Nametable映射关系为" << cartridge.mapper_ptr->nametable_mirror << "的情况" << endl;
        nametable_id = 0;
        nametable_addr = addr & 0x3ff;
        break;
    }
}

OAM_1Sprite OAM_RAM::load_1sprite(uint8_t spr_id){
    OAM_1Sprite sprite_data;
    sprite_data.loc_y = data[spr_id * 4]; //Tips:这里先不加一，到PPU使用时，再将y减一
    sprite_data.patterntable_addr = data[spr_id * 4 + 1] * 16;
    sprite_data.palette_dx = data[spr_id * 4 + 2] & 0x3;
    sprite_data.behind_background = bool(data[spr_id * 4 + 2] & (1 << 5));
    //x轴翻转：当第6位等于0时翻转，等于1时不翻转
    if (data[spr_id * 4 + 2] & (1 << 6)) sprite_data.flip_x = false;
    else sprite_data.flip_x = true;
    //y轴翻转：当第7位等于1时翻转，等于0时不翻转
    if (data[spr_id * 4 + 2] & (1 << 7)) sprite_data.flip_y = true;
    else sprite_data.flip_y = false;
    sprite_data.loc_x = data[spr_id * 4 + 3];
    return sprite_data;
}

OAM_1Sprite OAM_RAM::load_1sprite_long(uint8_t spr_id){
    OAM_1Sprite sprite_data;
    sprite_data.loc_y = data[spr_id * 4]; //Tips:这里先不加一，到PPU使用时，再将y减一
    bool bptable_dx = data[spr_id * 4 + 1] & 1; //对于long sprite，最后一位的含义是使用哪个图案表
    uint8_t tile_dx = data[spr_id * 4 + 1] >> 1;
    sprite_data.patterntable_addr = bptable_dx * 0x1000 + tile_dx * 32;
    sprite_data.palette_dx = data[spr_id * 4 + 2] & 0x3;
    sprite_data.behind_background = bool(data[spr_id * 4 + 2] & (1 << 5));
    //x轴翻转：当第6位等于0时翻转，等于1时不翻转
    if (data[spr_id * 4 + 2] & (1 << 6)) sprite_data.flip_x = false;
    else sprite_data.flip_x = true;
    //y轴翻转：当第7位等于1时翻转，等于0时不翻转
    if (data[spr_id * 4 + 2] & (1 << 7)) sprite_data.flip_y = true;
    else sprite_data.flip_y = false;
    sprite_data.loc_x = data[spr_id * 4 + 3];
    return sprite_data;
}
