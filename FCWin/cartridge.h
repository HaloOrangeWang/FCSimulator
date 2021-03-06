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
    uint8_t* program_data; //
    uint8_t* vrom_data; //
    uint8_t mapper_id; //
    bool has_added_ram; //
    Mapper* mapper_ptr; //
    void read_from_file(string input_file, int fsize);
};

#endif
