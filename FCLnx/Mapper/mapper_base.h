#ifndef MAPPER_BASE_H
#define MAPPER_BASE_H

#include <stdint.h>

class Mapper
{
public:
    //CPU读写0x6000-0xffff地址的数据
    virtual uint8_t cpu_read_addram(uint16_t addr) = 0;
    virtual void cpu_write_addram(uint16_t addr, uint8_t data) = 0;
    virtual uint8_t cpu_read_prg(uint16_t addr) = 0;
    virtual void cpu_write_prg(uint16_t addr, uint8_t data) = 0;
public:
    //给PPU提供的接口：读写图案表，切换名称表的映射方式
    virtual uint8_t ppu_read_pt(uint16_t addr) = 0;
    virtual void ppu_write_pt(uint16_t addr, uint8_t data) = 0;

    uint8_t nametable_mirror;
    uint8_t addram[0x2000]; //扩展的RAM
};

#endif
