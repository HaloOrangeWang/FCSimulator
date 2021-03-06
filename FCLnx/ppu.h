#ifndef PPU_H
#define PPU_H

#include <stdint.h>
#include <string.h>
#include "ppu_bus.h"

// 状态标志位
class PPUCTRL
{
public:
    uint8_t data;
    PPUCTRL(){
        data = 0; //初始标志位为0x00
    }
    int get_basentable(){ //使用哪个nametable
        return data & 3;
    }
    int get_addrincrement(){ //
        if (data & (1 << 2)) return 32;
        else return 1;
    }
    bool get_sptable_dx(){ //Sprite使用哪个图案表的标志位。当标志位为1时，使用0x1000-0x1fff的图案表，否则用0x0000-0x0fff的
        if (data & (1 << 3)) return 1;
        else return 0;
    }
    bool get_bptable_dx(){ //Background使用哪个图案表的标志位。当标志位为1时，使用0x1000-0x1fff的图案表，否则用0x0000-0x0fff的
        if (data & (1 << 4)) return 1;
        else return 0;
    }
    bool get_spritesize(){ //获取sprite大小标志位。当标志位为1时，sprite大小更大
        if (data & (1 << 5)) return 1;
        else return 0;
    }
    bool get_nmi(){ //获取是否在绘制结束后给CPU提供一个nmi中断的标志位
        if (data & (1 << 7)) return 1;
        else return 0;
    }
};

class PPUMASK
{
public:
    uint8_t data;
    PPUMASK(){
        data = 0;
    }
    bool get_greymode(){ //获取是否为黑白图像的标志位
        if (data & 1) return 1;
        else return 0;
    }
    bool get_showedgebkg(){ //获取是否展示最左边八个像素的Background的标志位
        if (data & (1 << 1)) return 1;
        else return 0;
    }
    bool get_showedgespr(){ //获取是否展示最左边八个像素的Sprites的标志位
        if (data & (1 << 2)) return 1;
        else return 0;
    }
    bool get_showbkg(){ //获取是否展示Background的标志位
        if (data & (1 << 3)) return 1;
        else return 0;
    }
    bool get_showspr(){ //获取是否展示Sprites的标志位
        if (data & (1 << 4)) return 1;
        else return 0;
    }
};

class PPUSTATUS
{
public:
    uint8_t data;
    PPUSTATUS(){
        data = 0;
    }
    bool get_sproverflow(){
        if (data & (1 << 5)) return 1;
        else return 0;
    }
    bool get_spr0hit(){
        if (data & (1 << 6)) return 1;
        else return 0;
    }
    bool get_vblank(){ //当前是否处在垂直消隐的阶段
        if (data & (1 << 7)) return 1;
        else return 0;
    }
    void set_sproverflow(bool n){
        if (n) data |= 1 << 5;
        else data &= 0xdf;
    }
    void set_spr0hit(bool n){
        if (n) data |= 1 << 6;
        else data &= 0xbf;
    }
    void set_vblank(bool n){ //设置垂直消隐的标志位
        if (n) data |= 1 << 7;
        else data &= 0x7f;
    }
};

class REG_V
{
public:
    uint16_t data;
    REG_V(){
        data = 0;
    }
    void set_low8(uint8_t data_in){ //设置寄存器的低八位值
        data &= 0xff00;
        data |= data_in;
    }
    void set_hi6(uint8_t data_in){ //设置寄存器的9-14位值. 更高的数值会被映射下去
        data &= 0x00ff;
        data |= ((data_in & 0x3f) << 8);
    }
    void set_nametable(uint8_t nametable_dx){ //设置NameTable的数值
        data &= 0xf3ff;
        data |= ((nametable_dx & 0x3) << 10);
    }
    void set_nametable_x(bool nametable_x){ //设置NameTable_X的数值
        if (nametable_x) data |= 1 << 10;
        else data &= 0xfbff;
    }
    void set_nametable_y(bool nametable_y){ //设置NameTable_Y的数值
        if (nametable_y) data |= 1 << 11;
        else data &= 0xf7ff;
    }
    void set_xscroll(uint8_t xscroll){ //设置x_scroll的数值
        data &= 0xffe0;
        data |= (xscroll & 0x1f);
    }
    void set_yscroll(uint8_t yscroll){ //设置y_scroll的数值
        data &= 0xfc1f;
        data |= ((yscroll & 0x1f) << 5);
    }
    void set_yfine(uint8_t yfine){ //设置y_fine的数值
        data &= 0x8fff;
        data |= ((yfine & 0x7) << 12);
    }
    uint8_t get_xscroll(){ //获取x_scroll的数值
        return data & 0x1f;
    }
    bool get_nametable_x(){
        if (data & (1 << 10)) return 1;
        else return 0;
    }
    bool get_nametable_y(){
        if (data & (1 << 11)) return 1;
        else return 0;
    }
    uint8_t get_yscroll(){ //获取y_scroll的数值
        return (data & 0x3e0) >> 5;
    }
    uint8_t get_yfine(){ //获取y_fine的数值
        return (data & 0x7000) >> 12;
    }
};

class PPU2
{
public:
    //寄存器
    PPUCTRL reg_ctrl;
    PPUMASK reg_mask;
    PPUSTATUS reg_sta;
    uint8_t reg_oamaddr; //当前的OAM地址

    bool address_latch; //地址锁存器。通过锁存器来判断当前要写入高八位还是低八位了
    uint8_t xfine; //x的精准滚动偏移量
    REG_V tmp_addr; //临时的data_addr
    REG_V data_addr; //x/y轴偏移量，使用的命名表id，y轴精准滚动值
    uint8_t data_buffer;
public:
    //供CPU那边调用的接口
    void write_ctrl(uint8_t ctrl);
    void write_mask(uint8_t mask);
    uint8_t get_status();
    void write_oamaddr(uint8_t addr);
    void write_oamdata(uint8_t data);
    uint8_t get_oamdata();
    void write_scroll(uint8_t scroll);
    void write_addr(uint8_t addr);
    void write_data(uint8_t data);
    uint8_t read_data();
    void oam_dma(uint8_t* p_data);
public:
    // PPU自身运行的函数
    void reset(); //PPU重置
    void run_1cycle(); //PPU运行一个周期
    // PPU临时变量
    int scanline; //第几条扫描线
    int cycle; //这条扫描线的第几个周期
    uint8_t scanline_spr_dx[8];
    uint8_t scanline_spr_cnt; //下一条扫描线上需要渲染的精灵个数
    PictureBus *p_bus;
    OAM_RAM oamram; //
    bool even_frame; //是不是偶数像素
    int frame_dx = 0;
    int frame_finished = -1;
    uint8_t frame_data[256][240][3];

    PPU2();
};

#endif
