#include "total.h"
#include <QDebug>

Mapper0::Mapper0()
{
    if (cartridge.vrom_num == 0)
        character_ram_ptr = new uint8_t[8192];
    else
        character_ram_ptr = nullptr;
}

uint8_t Mapper0::cpu_read_prg(uint16_t addr)
{
    uint16_t prg_addr;
    if (cartridge.rom_num == 1)
        prg_addr = addr & 0x3fff;
    else
        prg_addr = addr & 0x7fff;
    return cartridge.program_data[prg_addr];
}

void Mapper0::cpu_write_prg(uint16_t addr, uint8_t data)
{
    qDebug() << "0x8000 - 0xFFFF, cannot write to " << addr << endl;
}

uint8_t Mapper0::cpu_read_addram(uint16_t addr){
    qDebug() << "Mapper0 zai" << addr << "chu meiyou ewai de RAM" << endl;
    return 0;
}

void Mapper0::cpu_write_addram(uint16_t addr, uint8_t data){
    qDebug() << "Mapper0 zai" << addr << "chu meiyou ewai de RAM" << endl;
}

uint8_t Mapper0::ppu_read_pt(uint16_t addr){
    if (cartridge.vrom_num == 0)
        return character_ram_ptr[addr];
    else
        return cartridge.vrom_data[addr];
}

void Mapper0::ppu_write_pt(uint16_t addr, uint8_t data){
    if (cartridge.vrom_num == 0)
        character_ram_ptr[addr] = data;
    else
        qDebug() << "kadai de VROM shi zhidu de" << endl;
}
