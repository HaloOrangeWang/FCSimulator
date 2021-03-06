#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#include <stdint.h>
#include <string>
#include "Mapper/mapper_base.h"

using namespace std;

class Cartridge
{
public:
    uint8_t rom_num;
    uint8_t vrom_num;
    uint8_t* program_data; //这个卡带的程序数据
    uint8_t* vrom_data; //这个卡带的字模数据
    uint8_t mapper_id; //这个卡带使用哪个mapper
    bool has_added_ram; //卡带上是否有多余的RAM内存
    Mapper* mapper_ptr; //这个卡带使用哪一种mapper
    void read_from_file(string input_file, int fsize);
};

#endif
