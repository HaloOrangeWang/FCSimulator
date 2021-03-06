#include "total.h"
#include <QDebug>

Mapper1::Mapper1()
{
    if (cartridge.vrom_num == 0)
        character_ram_ptr = new uint8_t[8192];
    else
        character_ram_ptr = nullptr;
    memset(addram, 0, sizeof(uint8_t) * 0x2000);

    reg_ctrl.data = 0x1c;
    pt_select_4kb_lo = 0;
    pt_select_4kb_hi = 0;
    pt_select_8kb = 0;

    prg_select_16kb_lo = 0;
    prg_select_16kb_hi = cartridge.rom_num - 1;
    prg_select_32kb = 0;

}

uint8_t Mapper1::cpu_read_addram(uint16_t addr){
    return addram[addr - 0x6000];
}

void Mapper1::cpu_write_addram(uint16_t addr, uint8_t data){
    addram[addr - 0x6000] = data;
}

uint8_t Mapper1::cpu_read_prg(uint16_t addr){
    if (reg_ctrl.get_program_bank_mode() >= 2){
        //CPU读取两个16KB的代码段
        if (addr <= 0xbfff){
            uint32_t program_addr = 0x4000 * prg_select_16kb_lo + (addr & 0x3fff);
            return cartridge.program_data[program_addr];
        }else{
            uint32_t program_addr = 0x4000 * prg_select_16kb_hi + (addr & 0x3fff);
            return cartridge.program_data[program_addr];
        }
    }else{
        //CPU读取一个32KB的代码段
        uint32_t program_addr = 0x8000 * prg_select_32kb + (addr & 0x7fff);
        return cartridge.program_data[program_addr];
    }
}

void Mapper1::cpu_write_prg(uint16_t addr, uint8_t data){
    if (data & 0x80){
        //根据Mapper1的定义，当data的最高位为1时，清掉缓存
        reg_load = 0;
        num_write = 0;
        reg_ctrl.set_program_bank_mode(3);
    }else{
        bool input_flag = (data & 0x01);
        reg_load >>= 1;
        reg_load |= (input_flag << 4);
        num_write++;
        if (num_write == 5){
            //连续输入五次后，做出对应的控制调整
            if (addr >= 0x8000 && addr <= 0x9fff){
                //向0x8000-0x9fff写入数据，调整命名表的映射关系
                reg_ctrl.data = reg_load & 0x1f;
                switch(reg_ctrl.get_nametable_mirror()){
                case 0:
                    nametable_mirror = NAMETABLE_ONESCREEN_LOWER;
                    break;
                case 1:
                    nametable_mirror = NAMETABLE_ONESCREEN_HIGHER;
                    break;
                case 2:
                    nametable_mirror = NAMETABLE_VIRTICAL;
                    break;
                case 3:
                    nametable_mirror = NAMETABLE_HORIZONTAL;
                    break;
                }
            }else if (addr >= 0xa000 && addr <= 0xbfff){
                //向0xa000-0xbfff写入数据，调整图案表的寻址结果
                //模式为1时，PPU分成两个4KB的图案表来读取，调整图案表（0X0000-0X1000）的寻址结果
                //模式为0时，PPU读取一个8KB的图案表，调整图案表（0X0000-0X2000）的寻址结果
                if (reg_ctrl.get_pattern_bank_mode()){
                    pt_select_4kb_lo = reg_load & 0x1f;
                }else{
                    pt_select_8kb = reg_load & 0x1f;
                }
            }else if (addr >= 0xc000 && addr <= 0xdfff){
                //向0xc000-0xdfff写入数据，调整图案表的寻址结果
                //模式为1时，PPU分成两个4KB的图案表来读取，调整图案表（0X1000-0X2000）的寻址结果
                //模式为0时，PPU读取一个8KB的图案表，在此处输出无效
                if (reg_ctrl.get_pattern_bank_mode()){
                    pt_select_4kb_hi = reg_load & 0x1f;
                }
            }else if (addr >= 0xe000 && addr <= 0xffff){
                uint8_t program_bank_mode = reg_ctrl.get_program_bank_mode();
                if (program_bank_mode == 0 || program_bank_mode == 1){
                    //向0xe000-0xffff写入数据，调整程序代码的寻址结果
                    //模式为0或1时，CPU读取一个32KB的代码段。调整程序代码（0X8000-0Xffff）的寻址结果
                    //模式为2或3时，CPU读取两个16KB的代码段，按照相应规则调整程序代码的寻址结果
                    prg_select_32kb = ((reg_load & 0x0e) >> 1);
                }else if (program_bank_mode == 2){
                    prg_select_16kb_hi = (reg_load & 0x0f);
                    prg_select_16kb_lo = 0;
                }else if (program_bank_mode == 3){
                    prg_select_16kb_hi = cartridge.rom_num - 1;
                    prg_select_16kb_lo = (reg_load & 0x0f);
                }
            }
            reg_load = 0;
            num_write = 0;
        }
    }
}

uint8_t Mapper1::ppu_read_pt(uint16_t addr){
    if (cartridge.vrom_num == 0)
        return character_ram_ptr[addr];
    else{
        if (reg_ctrl.get_pattern_bank_mode()){
            //模式为1时，PPU分成两个4KB的图案表来读取
            if (addr <= 0x0fff){
                uint32_t vrom_addr = 0x1000 * pt_select_4kb_lo + (addr & 0x0fff);
                return cartridge.vrom_data[vrom_addr];
            }else{
                uint32_t vrom_addr = 0x1000 * pt_select_4kb_hi + (addr & 0x0fff);
                return cartridge.vrom_data[vrom_addr];
            }
        }else{
            //模式为0时，PPU读取一个8KB的图案表
            uint32_t vrom_addr = 0x2000 * pt_select_8kb + (addr & 0x1fff);
            return cartridge.vrom_data[vrom_addr];
        }
    }
}

void Mapper1::ppu_write_pt(uint16_t addr, uint8_t data){
    if (cartridge.vrom_num == 0)
        character_ram_ptr[addr] = data;
    else
        qDebug() << "卡带的VROM是只读的" << endl;
}
