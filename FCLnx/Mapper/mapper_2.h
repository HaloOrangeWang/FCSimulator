#ifndef MAPPER_2_H
#define MAPPER_2_H

#include "mapper_base.h"

class Mapper2: public Mapper
{
public:
    Mapper2();
    //CPU读写0x6000-0xffff地址的数据
    uint8_t cpu_read_addram(uint16_t addr);
    void cpu_write_addram(uint16_t addr, uint8_t data);
    uint8_t cpu_read_prg(uint16_t addr);
    void cpu_write_prg(uint16_t addr, uint8_t data);
    //给PPU提供的接口：读写图案表，切换名称表的映射方式
    uint8_t ppu_read_pt(uint16_t addr);
    void ppu_write_pt(uint16_t addr, uint8_t data);
    uint8_t* character_ram_ptr; //如果卡带上没有图案表的话，则新建一块8KB的ram作为图案表
private:
    uint8_t prg_select_16kb_lo;
    uint8_t prg_select_16kb_hi;
};

#endif
