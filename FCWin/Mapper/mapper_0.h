#ifndef MAPPER_0_H
#define MAPPER_0_H

#include "mapper_base.h"

class Mapper0: public Mapper
{
public:
    Mapper0();
    //
    uint8_t cpu_read_addram(uint16_t addr);
    void cpu_write_addram(uint16_t addr, uint8_t data);
    uint8_t cpu_read_prg(uint16_t addr);
    void cpu_write_prg(uint16_t addr, uint8_t data);
    //
    uint8_t ppu_read_pt(uint16_t addr);
    void ppu_write_pt(uint16_t addr, uint8_t data);
    uint8_t* character_ram_ptr; //
};

#endif
