#ifndef TOTAL_H
#define TOTAL_H

#include "cpu.h"
#include "cpu_ram.h"
#include "ppu.h"
#include "ppu_bus.h"
#include "controller.h"
#include "cartridge.h"
#include "Mapper/mapper_base.h"
#include "Mapper/mapper_0.h"
#include "Mapper/mapper_1.h"
#include "Mapper/mapper_2.h"

extern CpuBus Cpubus;
extern CPU Cpu;
extern PPU2 Ppu2;
extern PictureBus PpuBus;
extern Controller controller_left;
extern Controller controller_right;
extern Cartridge cartridge;

#endif // TOTAL_H
