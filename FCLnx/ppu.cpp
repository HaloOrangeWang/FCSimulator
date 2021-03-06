#include "ppu.h"
#include "total.h"
#include "colors_map.h"
#include <QDebug>

PPU2::PPU2(){
    this->p_bus = &PpuBus;
}

void PPU2::reset(){
    reg_ctrl.data = 0;
    reg_mask.data = 0;
    reg_sta.data = 0;

    address_latch = true;
    xfine = 0;
    tmp_addr.data = 0;
    data_addr.data = 0;
    data_buffer = 0;

    scanline = -1;
    cycle = 0;
    scanline_spr_cnt = 0;
    memset(oamram.data, 0, sizeof(uint8_t) * 256);
    even_frame = true;

    memset(frame_data, 0, sizeof(uint8_t) * 256 * 240 * 3);
}

void PPU2::write_ctrl(uint8_t ctrl){
    reg_ctrl.data = ctrl;
    tmp_addr.set_nametable(ctrl & 0x3); //设置nametable的时候要修改temp addr的内容
}

void PPU2::write_mask(uint8_t mask){
    reg_mask.data = mask;
}

uint8_t PPU2::get_status(){
    uint8_t data_ret = reg_sta.data;
    reg_sta.set_vblank(false);
    address_latch = true;
    return data_ret;
}

void PPU2::write_oamaddr(uint8_t addr){
    reg_oamaddr = addr;
}

void PPU2::write_oamdata(uint8_t data){
    oamram.data[reg_oamaddr] = data;
    reg_oamaddr++;
}

uint8_t PPU2::get_oamdata(){
    return oamram.data[reg_oamaddr];
}

void PPU2::write_scroll(uint8_t scroll){
    if (address_latch){
        //输入值的高五位为x轴滚动，低三位为x轴的精细坐标
        tmp_addr.set_xscroll((scroll >> 3) & 0x1f);
        xfine = scroll & 0x7;
        address_latch = false;
    }else{
        tmp_addr.set_yscroll((scroll >> 3) & 0x1f);
        tmp_addr.set_yfine(scroll & 0x7);
        address_latch = true;
    }
}

void PPU2::write_addr(uint8_t addr){
    if (address_latch){
        tmp_addr.set_hi6(addr & 0x3f);
        address_latch = false;
    }else{
        tmp_addr.set_low8(addr);
        data_addr.data = tmp_addr.data;
        address_latch = true;
    }
}

void PPU2::write_data(uint8_t data){
    p_bus->save(data_addr.data, data);
    data_addr.data += reg_ctrl.get_addrincrement();
}

uint8_t PPU2::read_data(){
    uint8_t data_ret = p_bus->load(data_addr.data);
    if (data_addr.data < 0x3f00){
        uint8_t tmp = data_buffer;
        data_buffer = data_ret;
        data_ret = tmp;
    }else{
        //data_buffer = p_bus->ram_data[data_addr.data & 0x3ff];
        data_buffer = p_bus->load(data_addr.data);
    }
    data_addr.data += reg_ctrl.get_addrincrement();
    return data_ret;
}

void PPU2::oam_dma(uint8_t *p_data){
    //先拷贝到$2003之后的地址，再拷贝到0-$2003的地址
    uint32_t num_data_cpy_first = 256 - reg_oamaddr;
    memcpy(oamram.data + reg_oamaddr, p_data, num_data_cpy_first);
    if (reg_oamaddr){
        memcpy(oamram.data, p_data + num_data_cpy_first, reg_oamaddr);
    }
}

void PPU2::run_1cycle(){
    if (scanline == -1){
        // PreRender扫描线
        if (cycle == 1){
            reg_sta.set_vblank(false);
            reg_sta.set_spr0hit(false);
            reg_sta.set_sproverflow(false);
        }
        if (cycle == 258 && reg_mask.get_showbkg() && reg_mask.get_showspr()){
            data_addr.set_xscroll(tmp_addr.get_xscroll());
            data_addr.set_nametable_x(tmp_addr.get_nametable_x());
        }
        if (cycle >= 280 && cycle <= 304 && reg_mask.get_showbkg() && reg_mask.get_showspr()){
            data_addr.set_yscroll(tmp_addr.get_yscroll());
            data_addr.set_nametable_y(tmp_addr.get_nametable_y());
            data_addr.set_yfine(tmp_addr.get_yfine());
        }
    }
    if (scanline >= 0 && scanline < 240){
        if (cycle > 0 && cycle <= 256){
            int x = cycle - 1; //实际图像中的x和y
            int y = scanline;

            //这三个变量会用来确定颜色的优先级，背景的调色板id，精灵的调色板id，精灵是否在先
            uint8_t bkgcolor_in_palette = 0;
            uint8_t sprcolor_in_palette = 0;
            bool spr_behind_background = true;
            uint16_t bkg_palette_addr = 0;
            uint16_t spr_palette_addr = 0;

            if (reg_mask.get_showbkg()){
                uint8_t x_in_tile = (x + xfine) % 8; //根据偏移量，当前的x是tile中的第几个位置
                uint8_t y_in_tile = data_addr.get_yfine();
                if (x >= 8 || reg_mask.get_showedgebkg()){
                    //如果隐藏最左边八个像素的背景的话，那最左边八个像素的背景渲染就可以略掉
                    //找出这个位置的命名表内容，找到对应的图案表的地址
                    uint16_t nametable_addr = 0x2000 | (data_addr.data & 0x0fff);
                    uint8_t tile_dx =  p_bus->load(nametable_addr);
                    // 读取图案表，获取这个像素点的颜色代码
                    uint16_t patterntable_addr = tile_dx * 16 + y_in_tile; //每一个tile占据16个字节。然后根据y的fine滚动数值，来确定这个像素来对应tile的那个位置（tile中每一行就是一个字节）
                    if (reg_ctrl.get_bptable_dx()){
                        patterntable_addr += 0x1000;
                    }
                    bool low_bit = ((p_bus->load(patterntable_addr) >> (7 - x_in_tile)) & 1);
                    bool hi_bit = ((p_bus->load(patterntable_addr + 8) >> (7 - x_in_tile)) & 1);
                    bkgcolor_in_palette = uint8_t(hi_bit << 1) + low_bit;
                    // 读取属性表，得知这个像素点所在的tile对应的调色板id是什么
                    uint16_t attrib_dx = ((data_addr.get_yscroll() >> 2) << 3) + ((data_addr.get_xscroll() >> 2) & 7);
                    uint16_t attrib_addr = 0x23c0 + (data_addr.get_nametable_y() * 2 + data_addr.get_nametable_x()) * 0x400 + attrib_dx;
                    uint8_t attr_dx = p_bus->load(attrib_addr);
                    // y=16~32，x=16~32时，取attribute table的最高两位作为调色板索引。此时右移6位
                    // y=16~32，x=0~15时，取attribute table的次高两位作为调色板索引。此时右移4位
                    // y=0~15，x=16~32时，取attribute table的次低两位作为调色板索引。此时右移2位
                    // y=0~15，x=0~15时，取attribute table的最低两位作为调色板索引。此时右移0位
                    uint8_t attr_shift = ((data_addr.get_yscroll() & 2) << 1) + (data_addr.get_xscroll() & 2);
                    uint8_t palette_dx = (attr_dx >> attr_shift) & 3;

                    bkg_palette_addr = 0x3f00 + 4 * palette_dx + bkgcolor_in_palette;
                }
                // 如果已经到达了一个tile的最后一个像素，则真实像素的下一格就应该是下一个tile的第一个像素了
                if (x_in_tile == 7){
                    if (data_addr.get_xscroll() == 31){
                        data_addr.set_xscroll(0);
                        data_addr.set_nametable_x(!data_addr.get_nametable_x());  //当已经到达这个命名表横轴上的最后一个位置时，则切换到下一个Horizental命名表
                    }else{
                        data_addr.set_xscroll(data_addr.get_xscroll() + 1);
                    }
                }
            }
            if (reg_mask.get_showspr()){
                if (x >= 8 || reg_mask.get_showedgespr()){
                    for (int spr_it = 0; spr_it <= scanline_spr_cnt - 1; spr_it++){
                        uint8_t spr_dx = scanline_spr_dx[spr_it];
                        OAM_1Sprite oam_1spr;
                        //找出这个像素的的图案表的颜色代码
                        if (reg_ctrl.get_spritesize()){
                            //渲染8*16的精灵
                            oam_1spr = oamram.load_1sprite_long(spr_dx);
                            if (x - oam_1spr.loc_x < 0 || x - oam_1spr.loc_x >= 8)
                                continue;
                            uint8_t x_in_tile = uint8_t(x - oam_1spr.loc_x);
                            uint8_t y_in_tile = uint8_t(y - 1 - oam_1spr.loc_y); //Tips: 在第二个scanline，渲染的其实是第一个scanline中应该渲染的精灵，所以y轴的数值要减一
                            if(oam_1spr.flip_x) //x轴翻转
                                x_in_tile = 7 - x_in_tile;
                            if (oam_1spr.flip_y) //y轴翻转
                                y_in_tile = 15 - y_in_tile;
                            //对于8*16的sprite，tile占据32个字节。然后根据y轴的数值，来确定这个像素来对应tile的那个位置（tile中每一行就是一个字节）
                            uint16_t patterntable_addr;
                            if (y_in_tile >= 8)
                                patterntable_addr = oam_1spr.patterntable_addr + 16 + (y_in_tile & 0x7);
                            else
                                patterntable_addr = oam_1spr.patterntable_addr + (y_in_tile & 0x7);
                            bool low_bit = ((p_bus->load(patterntable_addr) >> (x_in_tile)) & 1);
                            bool hi_bit = ((p_bus->load(patterntable_addr + 8) >> (x_in_tile)) & 1);
                            sprcolor_in_palette = uint8_t(hi_bit << 1) + low_bit;
                        }else{
                            //渲染8*8的精灵
                            oam_1spr = oamram.load_1sprite(spr_dx);
                            if (x - oam_1spr.loc_x < 0 || x - oam_1spr.loc_x >= 8)
                                continue;
                            //找出这个像素的的图案表的颜色代码
                            uint8_t x_in_tile = uint8_t(x - oam_1spr.loc_x);
                            uint8_t y_in_tile = uint8_t(y - 1 - oam_1spr.loc_y); //Tips: 在第二个scanline，渲染的其实是第一个scanline中应该渲染的精灵，所以y轴的数值要减一
                            if(oam_1spr.flip_x) //x轴翻转
                                x_in_tile = 7 - x_in_tile;
                            if (oam_1spr.flip_y) //y轴翻转
                                y_in_tile = 7 - y_in_tile;
                            //对于8*8的sprite，tile占据16个字节。然后根据y轴的数值，来确定这个像素来对应tile的那个位置（tile中每一行就是一个字节）
                            uint16_t patterntable_addr = oam_1spr.patterntable_addr + (y_in_tile & 0x7);
                            if (reg_ctrl.get_sptable_dx()){
                                patterntable_addr += 0x1000;
                            }
                            bool low_bit = ((p_bus->load(patterntable_addr) >> (x_in_tile)) & 1);
                            bool hi_bit = ((p_bus->load(patterntable_addr + 8) >> (x_in_tile)) & 1);
                            sprcolor_in_palette = uint8_t(hi_bit << 1) + low_bit;
                        }
                        //根据颜色代码来获取颜色。如果颜色代码为0的话，则这个像素不使用这个精灵的颜色，否则使用这个精灵的颜色
                        if (sprcolor_in_palette == 0)
                            continue;
                        spr_palette_addr = 0x3f10 + 4 * oam_1spr.palette_dx + sprcolor_in_palette;
                        spr_behind_background = oam_1spr.behind_background;
                        //sprite 0 hit的触发条件是，当数值不为零的0号sprite与数值不为零的background在同一像素出现
                        if (!reg_sta.get_spr0hit() && reg_mask.get_showbkg() && spr_dx == 0 && bkgcolor_in_palette != 0){
                            reg_sta.set_spr0hit(true);
                        }
                        break;
                    }
                }
            }
            //根据背景色和精灵色，综合确定这个像素的颜色是什么
            uint16_t palette_addr;
            if (bkgcolor_in_palette == 0 && sprcolor_in_palette == 0)
                palette_addr = 0x3f00;
            else if (bkgcolor_in_palette != 0 && sprcolor_in_palette == 0)
                palette_addr = bkg_palette_addr;
            else if (bkgcolor_in_palette == 0 && sprcolor_in_palette != 0)
                palette_addr = spr_palette_addr;
            else{
                if (spr_behind_background)
                    palette_addr = bkg_palette_addr;
                else
                    palette_addr = spr_palette_addr;
            }
            frame_data[x][y][0] = RGBColorMap[p_bus->load(palette_addr) & 0x3f][0];
            frame_data[x][y][1] = RGBColorMap[p_bus->load(palette_addr) & 0x3f][1];
            frame_data[x][y][2] = RGBColorMap[p_bus->load(palette_addr) & 0x3f][2];
        }
        if (cycle == 257 && reg_mask.get_showbkg()){
            uint8_t y_in_tile = data_addr.get_yfine();
            if (y_in_tile == 7){
                data_addr.set_yfine(0);
                if (data_addr.get_yscroll() == 29){
                    data_addr.set_yscroll(0);
                    data_addr.set_nametable_y(!data_addr.get_nametable_y());
                }else if (data_addr.get_yscroll() == 31){
                    //如果y超出了边界（30），例如把属性表中的数据当做tile读取的情况，则y到底后直接置为0，不切换命名表
                    data_addr.set_yscroll(0);
                }else{
                    data_addr.set_yscroll(data_addr.get_yscroll() + 1);
                }
            }else{
                data_addr.set_yfine(y_in_tile + 1);
            }
        }
        if (cycle == 258 && reg_mask.get_showbkg() && reg_mask.get_showspr()){
            data_addr.set_xscroll(tmp_addr.get_xscroll());
            data_addr.set_nametable_x(tmp_addr.get_nametable_x());
        }
        if (cycle == 340){
            //获取下一条扫描线上需要渲染哪些精灵
            //先初始化下一条扫描线上需要渲染的精灵列表
            scanline_spr_cnt = 0;
            bool spr_overflow = false;
            //再获取这一条扫描线上需要获取的精灵列表，按照优先级排列前八个精灵。如果超过八个，则置sprite overflow为true
            uint8_t spr_length = reg_ctrl.get_spritesize() ? 16 : 8;
            for (uint8_t spr_it = 0; spr_it <= 63; spr_it++){
                if (oamram.data[spr_it * 4] > scanline - spr_length && oamram.data[spr_it * 4] <= scanline){
                    if (scanline_spr_cnt == 8){
                        //qDebug() << "Sprite overflow, frame_dx = " << frame_dx << ", scanline = " << scanline << ", spr_it = " << spr_it << endl;
                        spr_overflow = true;
                        break;
                    }else{
                        scanline_spr_dx[scanline_spr_cnt] = spr_it;
                        scanline_spr_cnt++;
                    }
                }
            }
            reg_sta.set_sproverflow(spr_overflow);
        }
    }
    if (scanline == 240 && cycle == 0){
        frame_finished++;
    }
    //垂直消隐阶段
    if (scanline >= 241){
        if (scanline == 241 && cycle == 1)
        {
            //进入垂直消隐阶段时，调用CPU的NMI中断
            reg_sta.set_vblank(true);
            if (reg_ctrl.get_nmi())
                Cpu.nmi();
        }
    }
    //scanline和cycle递增
    if (scanline == -1 && cycle >= 340 - (!even_frame && reg_mask.get_showbkg() && reg_mask.get_showspr())){
        // 渲染奇数像素时，会把第-1条扫描线的第340个周期直接过掉
        cycle = 0;
        scanline = 0;
    }else{
        cycle++;
        if (cycle == 341){
            cycle = 0;
            scanline++;
            if (scanline >= 261){
                even_frame = !even_frame;
                scanline = -1;
                frame_dx++;
                //qDebug() << "frame_dx = " << frame_dx << endl;
            }
        }
    }
}
