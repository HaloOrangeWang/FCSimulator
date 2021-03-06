#ifndef MAPPER_1_H
#define MAPPER_1_H

#include "mapper_base.h"

class REG_CTRL
{
public:
    uint8_t data;
    bool get_pattern_bank_mode(){ //第4位表示图案表的寻址方式
        if (data & 0x10) return 1;
        else return 0;
    }
    uint8_t get_nametable_mirror(){ //第0-1位表示命名表的映射关系
        return (data & 0x03);
    }
    uint8_t get_program_bank_mode(){ //第2-3位表示代码段的寻址方式
        return ((data >> 2) & 0x03);
    }
    void set_program_bank_mode(uint8_t prg_mode){
        data &= 0x13;
        data |= ((prg_mode & 0x3) << 2);
    }
};

class Mapper1: public Mapper
{
public:
    Mapper1();
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
    uint8_t num_write = 0; //Mapper1扩展的寄存器：已经写了多少次(0-5)
    uint8_t reg_load = 0; //Mapper1扩展的寄存器：目前的输入值是多少
    REG_CTRL reg_ctrl;

    uint8_t pt_select_4kb_lo;
    uint8_t pt_select_4kb_hi;
    uint8_t pt_select_8kb;

    uint8_t prg_select_16kb_lo;
    uint8_t prg_select_16kb_hi;
    uint8_t prg_select_32kb;
};

#endif
