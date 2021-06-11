#include "total.h"
#include <QDebug>

void Pulse::init()
{
    for (int t = 0; t <= 3; t++)
        reg_pulse.data[t] = 0;
    envelope_restart = 0;
    envelope_divider = 0;
    envelope_val = 0;
    sweep_restart = 0;
    sweep_divider = 0;
    curr_period = 0;
    len_counter = 0;
}

void Pulse::write_data(uint8_t index, uint8_t data, bool enable)
{
    switch(index){
    case 0:
        reg_pulse.data[0] = data;
        break;
    case 1:
        reg_pulse.data[1] = data;
        sweep_restart = true; //写入$4001会重置扫描单元
        break;
    case 2:
        reg_pulse.data[2] = data;
        curr_period = reg_pulse.get_period();
        break;
    case 3:
        reg_pulse.data[3] = data;
        curr_period = reg_pulse.get_period();
        envelope_restart = true; //写入$4003会重置包络
        if (enable) //在方波启用的情况下，写入$4003会重置长度计数器
            len_counter = reg_pulse.get_lencntr_load();
        //这里先做一个简单的近似处理。在每个clock起始的位置重置方波和改变频率。实际情况应该是写入之后立即重置方波和改变频率，但先允许有1/240秒的误差
        seq_loc_old = 0;
        break;
    default:
        break;
    }
}

void Pulse::on_sweep_clock()
{
    if (sweep_divider == 0 && reg_pulse.get_sweep_flag() && reg_pulse.get_sweep_shift() > 0){
        if (curr_period >= 8 && curr_period <= 0x7ff){
            if (reg_pulse.get_negate_flag()){
                if (pulse_id == 1)
                    curr_period = curr_period - (curr_period >> reg_pulse.get_sweep_shift()) - 1;
                else
                   curr_period = curr_period - (curr_period >> reg_pulse.get_sweep_shift());
            }else{
                curr_period = curr_period + (curr_period >> reg_pulse.get_sweep_shift());
            }
        }
    }
    if (sweep_restart || (sweep_divider == 0)){
        //如果扫描单元重新开始，则重置扫描单元的分频器，并将重新开始的标志位置为False
        sweep_restart = false;
        sweep_divider = reg_pulse.get_sweep_period();
    }else{
        sweep_divider--;
    }
}

void Pulse::on_length_clock(){
    if (!reg_pulse.get_halt()){
        if (len_counter >= 1)
            len_counter -= 1;
    }
}

void Pulse::on_envelope_clock(){
    if (envelope_restart){
        //包络重新开始
        envelope_restart = false;
        envelope_divider = reg_pulse.get_envelop_period();
        envelope_val = 15;
    }else{
        if (envelope_divider == 0){
            envelope_divider = reg_pulse.get_envelop_period();
            if (envelope_val == 0){
                if (reg_pulse.get_halt()){
                    envelope_val = 15;
                }
            }else{
                envelope_val--;
            }
        }else{
            envelope_divider--;
        }
    }
}

void Pulse::play(uint64_t clock_cnt, bool enable){
    uint64_t cpu_loc_old = clock_cnt * CPU_CYCLE_PER_SEC / 240;
    for (uint64_t sample_loc = clock_cnt * SAMPLE_PER_SEC / 240 + 1; sample_loc <= (clock_cnt + 1) * SAMPLE_PER_SEC / 240; sample_loc++){
        //计算这个采样点是否要静音
        bool mute = false;
        if ((!enable) || (len_counter == 0)){
            mute = true;
            pulse_seq[cur_seq_id] = 0;
            seq_loc_old = 0;
            cur_seq_id++;
            continue;
        }
        else if (curr_period <= 7 || curr_period >= 0x800)
            mute = true;
        //计算这个采样点属于方波的什么位置
        uint64_t cpu_loc = uint64_t(sample_loc) * CPU_CYCLE_PER_SEC / SAMPLE_PER_SEC;
        uint64_t cpu_loc_diff = cpu_loc - cpu_loc_old; //这个采样点与时钟触发时的CPU周期间隔数
        double seq_diff = cpu_loc_diff * 1.0 / (16 * (curr_period + 1)); //这个采样点与时钟触发时相差了多少个方波周期
        double seq_loc = (seq_diff + seq_loc_old) - int(seq_diff + seq_loc_old); //这个采样点在方波的位置
        bool seq_val = PulseWave[reg_pulse.get_duty()][int(seq_loc * 8)];
        //计算这个采样点的音量
        uint8_t volume;
        if (mute){
            volume = 0;
        }else if (reg_pulse.get_constant_volume_flag()){ //使用固定音量
            volume = reg_pulse.get_constant_volume();
        }else{ //使用包络音量
            volume = envelope_val;
        }
        pulse_seq[cur_seq_id] = seq_val * volume;
        //收尾操作
        cur_seq_id++;
        if (sample_loc == uint64_t((clock_cnt + 1) * SAMPLE_PER_SEC / 240)){
            seq_loc_old = seq_loc;
        }
    }
}

void Triangle::init(){
    for (int t = 0; t <= 3; t++)
        reg_triangle.data[t] = 0;
    linear_counter = 0;
    len_counter = 0;
    linear_restart = 0;
    curr_period = 0;
}

void Triangle::write_data(uint8_t index, uint8_t data, bool enable)
{
    switch(index){
    case 0:
        reg_triangle.data[0] = data;
        break;
    case 2:
        reg_triangle.data[2] = data;
        curr_period = reg_triangle.get_period();
        break;
    case 3:
        reg_triangle.data[3] = data;
        curr_period = reg_triangle.get_period();
        linear_restart = true; //写入$400B会重置线性计数器
        if (enable) //在三角波启用的情况下，写入$4003会重置长度计数器
            len_counter = reg_triangle.get_lencntr_load();
        //和方波不一致的地方是，三角波的波形不会因为寄存器的写入而重置
        break;
    default:
        break;
    }
}

void Triangle::on_length_clock(){
    if (!reg_triangle.get_halt()){
        if (len_counter >= 1)
            len_counter -= 1;
    }
}

void Triangle::on_linear_clock(){
    if (linear_restart){
        linear_counter = reg_triangle.get_linecntr_load();
    }else if (linear_counter >= 1){
        linear_counter--;
    }
    // 非halt的情况下，才会清掉restart标志位
    if (!reg_triangle.get_halt())
        linear_restart = false;
}

void Triangle::play(uint64_t clock_cnt, bool enable){
    uint64_t cpu_loc_old = clock_cnt * CPU_CYCLE_PER_SEC / 240;
    for (uint64_t sample_loc = clock_cnt * SAMPLE_PER_SEC / 240 + 1; sample_loc <= (clock_cnt + 1) * SAMPLE_PER_SEC / 240; sample_loc++){
        //计算这个采样点是否要静音. 长度计数器和线性计数器中，只要有一个为零，就静音
        if ((!enable) || (len_counter == 0) || (linear_counter == 0)){
            triangle_seq[cur_seq_id] = 0;
            seq_loc_old = 0;
            cur_seq_id++;
            continue;
        }
        //计算这个采样点属于三角波的什么位置
        uint64_t cpu_loc = uint64_t(sample_loc) * CPU_CYCLE_PER_SEC / SAMPLE_PER_SEC;
        uint64_t cpu_loc_diff = cpu_loc - cpu_loc_old; //这个采样点与时钟触发时的CPU周期间隔数
        double seq_diff = cpu_loc_diff * 1.0 / (32 * (curr_period + 1)); //这个采样点与时钟触发时相差了多少个方波周期
        double seq_loc = (seq_diff + seq_loc_old) - int(seq_diff + seq_loc_old); //这个采样点在方波的位置
        uint8_t volume = TriangleWave[int(seq_loc * 32)];
        triangle_seq[cur_seq_id] = volume;
        //收尾操作
        cur_seq_id++;
        if (sample_loc == uint64_t((clock_cnt + 1) * SAMPLE_PER_SEC / 240)){
            seq_loc_old = seq_loc;
        }
    }
}

void Noise::init()
{
    for (int t = 0; t <= 3; t++)
        reg_noise.data[t] = 0;
    envelope_restart = 0;
    envelope_divider = 0;
    envelope_val = 0;
    len_counter = 0;
    reg_lfsr = 1; //LSFR寄存器初始化为1

}

void Noise::write_data(uint8_t index, uint8_t data, bool enable)
{
    switch(index){
    case 0:
        reg_noise.data[0] = data;
        break;
    case 2:
        reg_noise.data[2] = data;
        break;
    case 3:
        reg_noise.data[3] = data;
        envelope_restart = true; //写入$400F会重置包络
        if (enable) //在噪声启用的情况下，写入$400F会重置长度计数器
            len_counter = reg_noise.get_lencntr_load();
        break;
    default:
        break;
    }
}

void Noise::on_envelope_clock(){
    if (envelope_restart){
        //包络重新开始
        envelope_restart = false;
        envelope_divider = reg_noise.get_envelop_period();
        envelope_val = 15;
    }else{
        if (envelope_divider == 0){
            envelope_divider = reg_noise.get_envelop_period();
            if (envelope_val == 0){
                if (reg_noise.get_halt()){
                    envelope_val = 15;
                }
            }else{
                envelope_val--;
            }
        }else{
            envelope_divider--;
        }
    }
}

void Noise::on_length_clock(){
    if (!reg_noise.get_halt()){
        if (len_counter >= 1)
            len_counter -= 1;
    }
}

void Noise::play(uint64_t clock_cnt, bool enable){
    uint64_t cpu_loc_old = clock_cnt * CPU_CYCLE_PER_SEC / 240;
    for (uint64_t sample_loc = clock_cnt * SAMPLE_PER_SEC / 240 + 1; sample_loc <= (clock_cnt + 1) * SAMPLE_PER_SEC / 240; sample_loc++){
        //计算这个采样点是否要静音
        if ((!enable) || (len_counter == 0)){
            noise_seq[cur_seq_id] = 0;
            cur_seq_id++;
            continue;
        }
        //计算这两次采样之间，要进行几次LFSR计算
        uint64_t cpu_loc = uint64_t(sample_loc) * CPU_CYCLE_PER_SEC / SAMPLE_PER_SEC;
        uint64_t lfsr_count = (cpu_loc / reg_noise.get_noise_period()) - (cpu_loc_old / reg_noise.get_noise_period());
        cpu_loc_old = cpu_loc;
        //进行LFSR计算
        bool d0 = reg_lfsr & 1;
        for (uint64_t t = 0; t < lfsr_count; t++){
            if (reg_noise.get_noise_mode()){
                //短模式
                d0 = reg_lfsr & 1;
                bool d6 = (reg_lfsr & (1 << 6)) ? 1 : 0;
                bool yihuo = d0 ^ d6;
                reg_lfsr = (reg_lfsr >> 1) | (uint16_t(yihuo << 14));
            }else{
                //长模式
                d0 = reg_lfsr & 1;
                bool d1 = (reg_lfsr & (1 << 1)) ? 1 : 0;
                bool yihuo = d0 ^ d1;
                reg_lfsr = (reg_lfsr >> 1) | (uint16_t(yihuo << 14));
            }
        }
        //计算这个采样点的音量
        uint8_t volume;
        if (reg_noise.get_constant_volume_flag()){ //使用固定音量
            volume = reg_noise.get_constant_volume();
        }else{ //使用包络音量
            volume = envelope_val;
        }
        noise_seq[cur_seq_id] = d0 * volume;
        //收尾操作
        cur_seq_id++;
    }
}

void DPCM::init(){
    for(int t = 0; t <= 3; t++)
        reg_dpcm.data[t] = 0;
    curr_addr = 0;
    curr_byte = 0;
    bytes_remain = 0; //还有多少字节没有完成
    bits_remain = 0; //这个正在处理的字节，还有多少位没处理完
}

void DPCM::write_data(uint8_t index, uint8_t data, bool enable){
    switch(index){
    case 0:
        reg_dpcm.data[0] = data;
        break;
    case 1:
        reg_dpcm.data[1] = data;
        output_level = reg_dpcm.get_output_level();
        break;
    case 2:
        reg_dpcm.data[2] = data;
        break;
    case 3:
        reg_dpcm.data[3] = data;
        curr_addr = reg_dpcm.get_sample_address();
        bytes_remain = reg_dpcm.get_sample_len();
        break;
    default:
        break;
    }
}

void DPCM::play(uint64_t clock_cnt, bool enable){
    //if (enable && bytes_remain){
    //    qDebug() << "clock_cnt = " << clock_cnt << ", play dpcm" << endl;
    //}
    uint64_t cpu_loc_old = clock_cnt * CPU_CYCLE_PER_SEC / 240;
    for (uint64_t sample_loc = clock_cnt * SAMPLE_PER_SEC / 240 + 1; sample_loc <= (clock_cnt + 1) * SAMPLE_PER_SEC / 240; sample_loc++){
        //先计算这个采样点是否要静音
        if (!enable){
            dpcm_seq[cur_seq_id] = 0;
            cur_seq_id++;
            continue;
        }
        //计算这个采样周期内，要做几次DPCM运算
        uint64_t cpu_loc = uint64_t(sample_loc) * CPU_CYCLE_PER_SEC / SAMPLE_PER_SEC;
        uint64_t dpcm_count = (cpu_loc / reg_dpcm.get_period()) - (cpu_loc_old / reg_dpcm.get_period());
        cpu_loc_old = cpu_loc;
        //DPCM计算
        for (uint64_t t = 0; t < dpcm_count; t++){
            if (bytes_remain && (!bits_remain)){
                //当前样本字节已经读完了，load下一个样本字节
                curr_byte = Cpubus.load(curr_addr);
                if (curr_addr == 0xFFFF)
                    curr_addr = 0x8000;
                else
                    curr_addr++;
                bytes_remain--;
                bits_remain = 8;
            }
            if (bits_remain){
                //把当前样本字节的音频播放完成
                if (curr_byte & 1){
                    if (output_level <= 125)
                        output_level += 2;
                }else{
                    if (output_level >= 2)
                        output_level -= 2;
                }
                bits_remain--;
                curr_byte >>= 1;
                //如果播放完成了，而且设置了IRQ中断，则给出一个IRQ中断
                if (bits_remain == 0 && bytes_remain == 0){
                    if (reg_dpcm.get_irq_flag())
                        Cpu.irq();
                    if (reg_dpcm.get_loop_flag()){
                        curr_addr = reg_dpcm.get_sample_address();
                        bytes_remain = reg_dpcm.get_sample_len();
                    }
                }
            }
        }
        //收尾操作
        dpcm_seq[cur_seq_id] = output_level;
        cur_seq_id++;
    }
}

void APU::init(){
    //
    //寄存器初始化
    pulse0.init();
    pulse1.init();
    triangle.init();
    noise.init();
    dpcm.init();
    reg_sta.data = 0;
    reg_fcnt.data = 0;
    frame_interrupt = false;
    clock_cnt = 0;
}

void APU::write_data(uint8_t index, uint8_t data){
    if (index >= 0 && index <= 3){
        //写入方波0的寄存器
        pulse0.write_data(index, data, reg_sta.get_pulse0_flag());
    }else if (index >= 4 && index <= 7){
        //写入方波1的寄存器
        pulse1.write_data(index - 4, data, reg_sta.get_pulse1_flag());
    }else if (index >= 8 && index <= 11){
        //写入三角波的寄存器
        triangle.write_data(index - 8, data, reg_sta.get_triangle_flag());
    }else if (index >= 12 && index <= 15){
        //写入噪声的寄存器
        noise.write_data(index - 12, data, reg_sta.get_noise_flag());
    }else if (index >= 0x10 && index <= 0x13){
        //写入DPCM的寄存器
        dpcm.write_data(index - 0x10, data, reg_sta.get_dmc_flag());
    }else if (index == 0x15){
        //写入APU Status
        reg_sta.data = data;
        if (reg_sta.get_pulse0_flag() == false)
            pulse0.len_counter = 0;
        if (reg_sta.get_pulse1_flag() == false)
            pulse1.len_counter = 0;
        if (reg_sta.get_triangle_flag() == false)
            triangle.len_counter = 0;
        if (reg_sta.get_noise_flag() == false)
            noise.len_counter = 0;
    }else if (index == 0x17){
        //写入frame counter
        reg_fcnt.data = data;
        if (reg_fcnt.get_mode() == 1 || (!reg_fcnt.get_frame_irq()))
            frame_interrupt = false;
        if (reg_fcnt.get_mode() == 1){
            //启用五步模式，会立即产生一个时钟信号
            on_envelope_linear_clock();
            on_length_sweep_clock();
        }
    }
}

uint8_t APU::read_4015(){
    uint8_t res = 0;
    if (pulse0.len_counter) res |= 1;
    if (pulse1.len_counter) res |= (1 << 1);
    if (triangle.len_counter) res |= (1 << 2);
    if (noise.len_counter) res |= (1 << 3);
    if (frame_interrupt) res |= (1 << 6);
    if (dpcm.bytes_remain) res |= (1 << 4);
    frame_interrupt = false; //读取$4015数据时，会立即清掉frame_interrupt
    return res;
}

void APU::on_length_sweep_clock(){
    pulse0.on_length_clock();
    pulse0.on_sweep_clock();
    pulse1.on_length_clock();
    pulse1.on_sweep_clock();
    triangle.on_length_clock();
    noise.on_length_clock();
}

void APU::on_envelope_linear_clock(){
    pulse0.on_envelope_clock();
    pulse1.on_envelope_clock();
    triangle.on_linear_clock();
    noise.on_envelope_clock();
}

void APU::on_frame_irq(){
    if (reg_fcnt.get_frame_irq()){
        frame_interrupt = true;
        Cpu.irq();
    }
}

void APU::run_1cycle()
{
    //调整寄存器的数值
    if (reg_fcnt.get_mode()){
        switch (clock_cnt % 5){
        case 0:
            on_envelope_linear_clock();
            break;
        case 1:
            on_length_sweep_clock();
            on_envelope_linear_clock();
            break;
        case 2:
            on_envelope_linear_clock();
            break;
        case 4:
            on_length_sweep_clock();
            on_envelope_linear_clock();
            break;
        default:
            break;
        }
    }else{
        switch (clock_cnt % 4){
        case 0:
            on_envelope_linear_clock();
            break;
        case 1:
            on_length_sweep_clock();
            on_envelope_linear_clock();
            break;
        case 2:
            on_envelope_linear_clock();
            break;
        case 3:
            on_length_sweep_clock();
            on_envelope_linear_clock();
            on_frame_irq();
            break;
        default:
            break;
        }
    }
    //播放音频
    gen_wave();
    clock_cnt++;
}

void APU::gen_wave()
{
    //生成方波、三角波、噪音、DPCM
    pulse0.play(clock_cnt, reg_sta.get_pulse0_flag());
    pulse1.play(clock_cnt, reg_sta.get_pulse1_flag());
    triangle.play(clock_cnt, reg_sta.get_triangle_flag());
    noise.play(clock_cnt, reg_sta.get_noise_flag());
    dpcm.play(clock_cnt, reg_sta.get_noise_flag());

    if (clock_cnt % 4 == 3){
        //混音
        seq_len = pulse0.cur_seq_id;
        for (int t = 0; t <= seq_len - 1; t++){
            double volume_total = 0;
            //volume_total += 95.88 / ((8128.0 / (pulse0.pulse_seq[t] + pulse1.pulse_seq[t])) + 100);
            //volume_total += 159.79 / ((1 / (triangle.triangle_seq[t] / 8227.0 + noise.noise_seq[t] / 12241.0 + dpcm.dpcm_seq[t] / 22638.0)) + 100);
            volume_total += 0.00752 * (pulse0.pulse_seq[t] + pulse1.pulse_seq[t]);
            volume_total += 0.00851 * triangle.triangle_seq[t] + 0.00494 * noise.noise_seq[t] + 0.00335 * dpcm.dpcm_seq[t];
            seq[t] = uint8_t(volume_total * 256);
        }
        //if (seq[seq_len - 1] == 0)
        //    seq[seq_len - 1] = 1;
        //将各个波形中的一些缓存数据清零
        memset(pulse0.pulse_seq, 0, sizeof(uint8_t) * 4 * SAMPLE_PER_CLOCK);
        memset(pulse1.pulse_seq, 0, sizeof(uint8_t) * 4 * SAMPLE_PER_CLOCK);
        memset(triangle.triangle_seq, 0, sizeof(uint8_t) * 4 * SAMPLE_PER_CLOCK);
        memset(noise.noise_seq, 0, sizeof(uint8_t) * 4 * SAMPLE_PER_CLOCK);
        memset(dpcm.dpcm_seq, 0, sizeof(uint8_t) * 4 * SAMPLE_PER_CLOCK);
        pulse0.cur_seq_id = 0;
        pulse1.cur_seq_id = 0;
        triangle.cur_seq_id = 0;
        noise.cur_seq_id = 0;
        dpcm.cur_seq_id = 0;
    }
}
