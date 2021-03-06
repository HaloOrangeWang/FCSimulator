#ifndef CPU_RAM_H
#define CPU_RAM_H

#include <stdint.h>
#include <string.h>

class CpuBus
{
public:
    uint8_t ram_data[2048];
    CpuBus(){
        memset(ram_data, 0, sizeof(uint8_t) * 2048);
    }
    void save(uint16_t addr, uint8_t data);
    uint8_t load(uint16_t addr);
    uint8_t* get_page_ptr(uint8_t page_id);
};

#endif // CPU_RAM_H
