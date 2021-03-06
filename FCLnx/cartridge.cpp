#include "total.h"
#include <string>
#include <fstream>
#include <QDebug>
#include <string.h>

using namespace std;

void Cartridge::read_from_file(string input_file, int fsize)
{
    //1.读NES文件
    uint8_t* nes_data = new uint8_t[fsize];
    ifstream stream(input_file, ios::in | ios::binary);
    if (!stream){
        qDebug() << "Read NES File Error!" << endl;
        abort();
    }
    stream.read((char*)nes_data, fsize);
    stream.close();
    if (nes_data[0] != 'N' || nes_data[1] != 'E' || nes_data[2] != 'S' || nes_data[3] != '\x1A'){
        qDebug() << "First 4 bytes in file must be NES\\x1A!" << endl;
        abort();
    }
    //2.解析NES文件头部，包括ROM和vram的数量，nametable的映射方式，以及这个卡带使用了哪一种mapper
    rom_num = nes_data[4];
    vrom_num = nes_data[5];
    uint8_t nametable_mirror = nes_data[6] & 0xb;//这个卡带规定的nametable的镜像方式
    uint8_t mapper_type = ((nes_data[6] >> 4) & 0xf) | (nes_data[7] & 0xf0);
    has_added_ram = bool(nes_data[6] & 0x2);
    //3.新建Mapper类
    if (mapper_type == 0){
        mapper_ptr = new Mapper0();
        mapper_ptr->nametable_mirror = nametable_mirror;
    }else if (mapper_type == 1){
        mapper_ptr = new Mapper1();
        mapper_ptr->nametable_mirror = nametable_mirror;
    }else if (mapper_type == 2){
        mapper_ptr = new Mapper2();
        mapper_ptr->nametable_mirror = nametable_mirror;
    }else{
        qDebug() << "不支持Mapper = " << mapper_type << endl;
        abort();
    }
    //4.从NES文件中获取程序指令, 图案表（字体库）的数据
    uint32_t rom_start_dx = 16;
    uint32_t vrom_start_dx = rom_num * 16384 + 16;
    program_data = new uint8_t[16384 * rom_num];
    memcpy(program_data, &nes_data[rom_start_dx], 16384 * rom_num);
    vrom_data = new uint8_t[8192 * vrom_num];
    memcpy(vrom_data, &nes_data[vrom_start_dx], 8192 * vrom_num);
}
