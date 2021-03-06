#include "total.h"
#include <QDebug>

Mapper2::Mapper2()
{
    if (cartridge.vrom_num == 0)
        character_ram_ptr = new uint8_t[8192];
    else
        character_ram_ptr = nullptr;

    prg_select_16kb_lo = 0;
    prg_select_16kb_hi = cartridge.rom_num - 1;
}

uint8_t Mapper2::cpu_read_prg(uint16_t addr)
{
    //CPU读取两个16KB的代码段
    if (addr <= 0xbfff){
        uint32_t program_addr = 0x4000 * prg_select_16kb_lo + (addr & 0x3fff);
        return cartridge.program_data[program_addr];
    }else{
        uint32_t program_addr = 0x4000 * prg_select_16kb_hi + (addr & 0x3fff);
        return cartridge.program_data[program_addr];
    }
}

void Mapper2::cpu_write_prg(uint16_t addr, uint8_t data)
{
    prg_select_16kb_lo = data & 0x0f;
}

uint8_t Mapper2::cpu_read_addram(uint16_t addr){
    qDebug() << "Mapper2在" << addr << "处没有额外的RAM" << endl;
    return 0;
}

void Mapper2::cpu_write_addram(uint16_t addr, uint8_t data){
    qDebug() << "Mapper2在" << addr << "处没有额外的RAM" << endl;
}

uint8_t Mapper2::ppu_read_pt(uint16_t addr){
    if (cartridge.vrom_num == 0)
        return character_ram_ptr[addr];
    else
        return cartridge.vrom_data[addr];
}

void Mapper2::ppu_write_pt(uint16_t addr, uint8_t data){
    if (cartridge.vrom_num == 0)
        character_ram_ptr[addr] = data;
    else
        qDebug() << "卡带的VROM是只读的" << endl;
}
