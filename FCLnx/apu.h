#ifndef APU_H
#define APU_H

#include <stdint.h>
#include "audio_map.h"

class PulseReg
{
    //方波寄存器相关的内容
public:
    uint8_t data[4];
    uint8_t get_duty(){ //获取占空比的相关信息
        return (data[0] >> 6);
    }
    bool get_halt(){ //获取方波是否会一直被播放的标志位
        if (data[0] & (1 << 5)) return 1;
        else return 0;
    }
    bool get_constant_volume_flag(){ // 获取是否使用包络控制音量的标志位
        if (data[0] & (1 << 4)) return 1;
        else return 0;
    }
    uint8_t get_constant_volume(){ //常量状态下的音量
        return (data[0] & 0xf);
    }
    uint8_t get_envelop_period(){ //获取包络的时间长度
        return (data[0] & 0xf);
    }
    bool get_sweep_flag(){ //获取是否启用扫描单元（滑音）的标志位
        return (data[1] >> 7);
    }
    uint8_t get_sweep_period(){ //扫描单元每多少个周期调整一次频率
        return ((data[1] >> 4) & 0x7);
    }
    bool get_negate_flag(){ //0表示分频器数值上升，频率下降；1表示分频器数值下降，频率上升
        if (data[1] & (1 << 3)) return 1;
        else return 0;
    }
    uint8_t get_sweep_shift(){ //确定扫描单元每次触发时，分频器的数值上升和下降的幅度
        return (data[1] & 0x7);
    }
    uint16_t get_period(){ //获取分频数值
        uint16_t period = data[2];
        period |= ((data[3] & 0x7) << 8);
        return period;
    }
    uint8_t get_lencntr_load(){ //获取长度计数器(Length Counter)的初值
        uint8_t load_val = (data[3] >> 3);
        return LengthCounterMap[load_val];
    }
};

class TriangleReg
{
    //三角波的相关寄存器内容
public:
    uint8_t data[4];
    bool get_halt(){ //获取三角波是否会一直被播放的标志位
        if (data[0] & (1 << 7)) return 1;
        else return 0;
    }
    uint8_t get_linecntr_load(){ //获取线性计数器的初始值
        return (data[0] & 0x7f);
    }
    uint16_t get_period(){ //获取分频数值
        uint16_t period = data[2];
        period |= ((data[3] & 0x7) << 8);
        return period;
    }
    uint8_t get_lencntr_load(){ //获取长度计数器(Length Counter)的初值
        uint8_t load_val = (data[3] >> 3);
        return LengthCounterMap[load_val];
    }
};

class NoiseReg
{
    // 噪声寄存器的相关内容
public:
    uint8_t data[4];
    bool get_halt(){ //获取噪声是否会一直被播放的标志位
        if (data[0] & (1 << 5)) return 1;
        else return 0;
    }
    bool get_constant_volume_flag(){ // 获取是否使用包络控制音量的标志位
        if (data[0] & (1 << 4)) return 1;
        else return 0;
    }
    uint8_t get_constant_volume(){ //常量状态下的音量
        return (data[0] & 0xf);
    }
    uint8_t get_envelop_period(){ //获取包络的时间长度
        return (data[0] & 0xf);
    }
    bool get_noise_mode(){ //获取噪声是长模式还是短模式的标志位。1为短模式，0为长模式
        return (data[2] >> 7);
    }
    uint16_t get_noise_period(){ //获取噪声采样的周期
        uint8_t reg_val = (data[2] & 0xf);
        return NoisePeriodMap[reg_val];
    }
    uint8_t get_lencntr_load(){ //获取长度计数器(Length Counter)的初值
        uint8_t load_val = (data[3] >> 3);
        return LengthCounterMap[load_val];
    }
};

class DPCMReg
{
    //DPCM寄存器的相关内容
public:
    uint8_t data[4];
    bool get_irq_flag(){ //PCM播放完成之后是否产生IRQ中断的标志位
        return (data[0] >> 7);
    }
    bool get_loop_flag(){ //PCM是否循环播放
        if (data[0] & (1 << 6)) return 1;
        else return 0;
    }
    uint16_t get_period(){ //每隔多少个周期播放一次PCM数据
        uint8_t period_index = (data[0] & 0xf);
        return DPCMPeriodMap[period_index];
    }
    uint8_t get_output_level(){ //获取DPCM输出的基础音量数值
        return (data[1] & 0x7f);
    }
    uint16_t get_sample_address(){ //获取样本地址
        uint8_t sample_address_val = data[2];
        return (0xC000 | uint16_t(sample_address_val << 6));
    }
    uint16_t get_sample_len(){
        uint8_t sample_len_val = data[3];
        return (1 | uint16_t(sample_len_val << 4));
    }
};

class APUSTATUS
{
public:
    uint8_t data;
    bool get_dmc_flag(){
        if (data & (1 << 4)) return 1;
        else return 0;
    }
    bool get_noise_flag(){
        if (data & (1 << 3)) return 1;
        else return 0;
    }
    bool get_triangle_flag(){
        if (data & (1 << 2)) return 1;
        else return 0;
    }
    bool get_pulse1_flag(){
        if (data & (1 << 1)) return 1;
        else return 0;
    }
    bool get_pulse0_flag(){
        if (data & 1) return 1;
        else return 0;
    }
};

class FrameCounter
{
public:
    uint8_t data;
    bool get_mode(){ //四步模式还是五步模式
        if (data & (1 << 7)) return 1;
        else return 0;
    }
    bool get_frame_irq(){ //四步模式下，最后一步是否触发IRQ中断的标志位
        if (data & (1 << 6)) return 1;
        else return 0;
    }
};

class Pulse
{
    //方波的相关内容
public:
    //寄存器
    PulseReg reg_pulse; //CPU写入的方波寄存器
public:
    int pulse_id; //0号还是1号方波
    bool envelope_restart; //包络是否重新开始
    uint8_t envelope_divider; //包络的分频器
    uint8_t envelope_val; //当前当前包络的数值是多少
    bool sweep_restart; //扫描单元是否重新开始
    uint8_t sweep_divider; //扫描单元的分频器
    uint16_t curr_period; //当前方波的频率
    uint8_t len_counter; //长度计数器
    // 播放方波时需要缓存的内容
    uint8_t pulse_seq[4 * SAMPLE_PER_CLOCK];
    double seq_loc_old; //上次运行到方波的什么位置了
    int cur_seq_id;
public:
    Pulse(int id): pulse_id(id){}
    void init();
    void write_data(uint8_t index, uint8_t data, bool enable); //给CPU提供的接口：写入寄存器数据
    void on_envelope_clock(); //触发包络的时钟
    void on_length_clock(); //触发长度寄存器的时钟
    void on_sweep_clock(); //触发扫描单元的时钟
    void play(uint64_t clock_cnt, bool enable);
};

class Triangle
{
    //三角波的相关内容
public:
    //寄存器
    TriangleReg reg_triangle; //CPU写入的三角波寄存器
public:
    bool linear_restart; //线性计数器是否重新开始
    uint8_t linear_counter; //线性计数器
    uint8_t len_counter; //长度计数器
    uint16_t curr_period; //当前三角波的频率
    // 播放三角波时需要缓存的内容
    uint8_t triangle_seq[4 * SAMPLE_PER_CLOCK];
    int cur_seq_id;
    double seq_loc_old; //上次运行到三角波的什么位置了
public:
    void init();
    void write_data(uint8_t index, uint8_t data, bool enable); //给CPU提供的接口：写入寄存器数据
    void on_length_clock(); //触发长度寄存器的时钟
    void on_linear_clock(); //触发线性计数器的时钟
    void play(uint64_t clock_cnt, bool enable);
};

class Noise
{
    //噪声的相关内容
public:
    //寄存器
    NoiseReg reg_noise; //CPU写入的噪声寄存器
public:
    bool envelope_restart; //包络是否重新开始
    uint8_t envelope_divider; //包络的分频器
    uint8_t envelope_val; //当前包络的数值是多少
    uint8_t len_counter; //长度计数器
    uint16_t reg_lfsr; //LSFR寄存器
    // 播放噪声时需要缓存的内容
    uint8_t noise_seq[4 * SAMPLE_PER_CLOCK];
    int cur_seq_id;
public:
    void init();
    void write_data(uint8_t index, uint8_t data, bool enable); //给CPU提供的接口：写入寄存器数据
    void on_envelope_clock(); //触发包络的时钟
    void on_length_clock(); //触发长度寄存器的时钟
    void play(uint64_t clock_cnt, bool enable);
};

class DPCM
{
    //DPCM的相关内容
public:
    //寄存器
    DPCMReg reg_dpcm;
public:
    uint16_t curr_addr; //指向下一个要处理的DMAC样本字节
    uint8_t curr_byte; //当前的DMAC样本字节
    uint16_t bytes_remain; //还有多少字节没有完成
    uint8_t bits_remain; //这个正在处理的字节，还有多少位没处理完
    uint8_t output_level; //当前缓存的音量
    // 播放DPCM需要缓存的内容
    uint8_t dpcm_seq[4 * SAMPLE_PER_CLOCK];
    int cur_seq_id;
public:
    void init();
    void write_data(uint8_t index, uint8_t data, bool enable); //给CPU提供的接口：写入寄存器数据
    void play(uint64_t clock_cnt, bool enable);
};

class APU
{
public:
    //寄存器
    Pulse pulse0;
    Pulse pulse1;
    Triangle triangle;
    Noise noise;
    DPCM dpcm;
    APUSTATUS reg_sta;
    FrameCounter reg_fcnt;
    bool frame_interrupt;
public:
    APU(): pulse0(0), pulse1(1){}
    void init();
    //供CPU那边调用的接口
    void write_data(uint8_t index, uint8_t data); //给CPU提供的接口：写入寄存器数据
    uint8_t read_4015(); //从0x4015读取数据
    //APU自身运行的函数
    void run_1cycle(); //APU运行一个周期
    void on_envelope_linear_clock(); //触发包络和线性计数器的时钟
    void on_length_sweep_clock(); //触发包络和线性计数器的时钟
    void on_frame_irq(); //四步模式下，可能每四步触发一次IRQ中断
    void gen_wave(); //生成一个周期的波形
    uint8_t seq[4 * SAMPLE_PER_CLOCK];
    int seq_len;
    uint64_t clock_cnt = 0;
};

#endif
