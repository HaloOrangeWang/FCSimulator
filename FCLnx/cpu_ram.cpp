#include "cpu_ram.h"
#include "total.h"
#include <QDebug>


void CpuBus::save(uint16_t addr, uint8_t data)
{
    if (addr < 0x2000){
        ram_data[addr & 0x7ff] = data;
    }else if (addr < 0x4000){
        switch (addr & 0x2007){
        case 0x2000: //PPUCTRL
            Ppu2.write_ctrl(data);
            break;
        case 0x2001: //PPUMASK
            Ppu2.write_mask(data);
            break;
        case 0x2002:
            qDebug("cannot write to PPUSTATUS\n");
            break;
        case 0x2003:
            Ppu2.write_oamaddr(data);
            break;
        case 0x2004:
            Ppu2.write_oamdata(data);
            break;
        case 0x2005:
            Ppu2.write_scroll(data);
            break;
        case 0x2006:
            Ppu2.write_addr(data);
            break;
        case 0x2007:
            Ppu2.write_data(data);
            break;
        }
    }else if (addr == 0x4014){
        //OAM DMA
        Cpu.dma_sleep(); //执行DMA时，CPU会被阻塞513或514个周期
        uint8_t* page_ptr = get_page_ptr(data);
        Ppu2.oam_dma(page_ptr);
    }else if (addr == 0x4016){
        controller_left.write_strobe(data);
        controller_right.write_strobe(data);
    }else if (addr >= 0x4000 && addr <= 0x4017){
        Apu.write_data(uint8_t(addr - 0x4000), data);
    }else if (addr >= 0x4000 && addr < 0x6000){
        //qDebug() << "0x4000 - 0x6000, Cannot write to " << addr << endl;
    }else if (addr >= 0x6000 && addr < 0x8000){
        if (cartridge.has_added_ram){
            cartridge.mapper_ptr->cpu_write_addram(addr, data);
        }
    }else{
        cartridge.mapper_ptr->cpu_write_prg(addr, data);
    }
}

uint8_t CpuBus::load(uint16_t addr)
{
    if (addr < 0x2000){
        return ram_data[addr & 0x7ff];
    }else if (addr < 0x4000){
        switch (addr & 0x2007){
        case 0x2000: //PPUCTRL
            qDebug("cannot read PPUCTRL\n");
            break;
        case 0x2001: //PPUMASK
            qDebug("cannot read PPUMASK\n");
            break;
        case 0x2002:
            return Ppu2.get_status();
        case 0x2003:
            qDebug("cannot read OAMADDR\n");
            break;
        case 0x2004:
            return Ppu2.get_oamdata();
        case 0x2005:
            qDebug("cannot read PPUSCROLL\n");
            break;
        case 0x2006:
            qDebug("cannot read PPUADDR\n");
            break;
        case 0x2007:
            return Ppu2.read_data();
        }
    }else if (addr == 0x4016){
        //手柄处理
        return controller_left.output_key_states();
    }else if (addr == 0x4017){
        return controller_right.output_key_states();
    }else if (addr == 0x4015){
        return Apu.read_4015();
    }else if (addr >= 0x4000 && addr < 0x6000){
        //qDebug() << "0x4000 - 0x6000, Cannot read " << addr << endl;
    }else if (addr >= 0x6000 && addr < 0x8000){
        if (cartridge.has_added_ram){
            return cartridge.mapper_ptr->cpu_read_addram(addr);
        }
    }else{
        //从卡带中读取程序数据
        return cartridge.mapper_ptr->cpu_read_prg(addr);
    }
    return 0;
}

uint8_t* CpuBus::get_page_ptr(uint8_t page_id){
    uint16_t addr = uint16_t(page_id << 8);
    if (addr < 0x2000){
        return &ram_data[addr & 0x7ff];
    }else if (addr >= 0x6000 && addr < 0x8000){
        if (cartridge.has_added_ram){
            return &cartridge.mapper_ptr->addram[addr - 0x6000];
        }else{
            qDebug() << "get_page_ptr, Cannot get page ptr of " << addr << endl;
            abort();
        }
    }else{
        qDebug() << "get_page_ptr, Cannot get page ptr of " << addr << endl;
        abort();
    }
}
