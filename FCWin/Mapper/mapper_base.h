#ifndef MAPPER_BASE_H
#define MAPPER_BASE_H

#include <stdint.h>

class Mapper
{
public:
    //
    virtual uint8_t cpu_read_addram(uint16_t addr) = 0;
    virtual void cpu_write_addram(uint16_t addr, uint8_t data) = 0;
    virtual uint8_t cpu_read_prg(uint16_t addr) = 0;
    virtual void cpu_write_prg(uint16_t addr, uint8_t data) = 0;
public:
    //
    virtual uint8_t ppu_read_pt(uint16_t addr) = 0;
    virtual void ppu_write_pt(uint16_t addr, uint8_t data) = 0;

    uint8_t nametable_mirror;
    uint8_t addram[0x2000]; //
};

#endif
