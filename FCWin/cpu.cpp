#include "total.h"
#include <QDebug>

CPU::CPU(): addr_res(0), addr_rel(0), cycles_wait(0), opcode(0), clock_count(0){
    //
    this->p_ram = &Cpubus;
}

void CPU::push_stack(uint8_t value){
    if (reg_sp == 0){
        qDebug() << "cuowu: zhan yijing xieman le" << endl;
        abort();
    }
    p_ram->save(reg_sp + 0x100, value);
    reg_sp--;
}

uint8_t CPU::pull_stack(){
    reg_sp++;
    uint8_t res = p_ram->load(reg_sp + 0x100);
    return res;
}

void CPU::reset(){
    //
    reg_a = 0;
    reg_x = 0;
    reg_y = 0;
    reg_sp = 0xfd;
    reg_sf.set_i(true); //
    reg_sf.set_u(true);
    uint8_t lo8 = p_ram->load(0xFFFC); //
    uint8_t hi8 = p_ram->load(0xFFFD);
    reg_pc = uint16_t(hi8 << 8) + lo8;
}

void CPU::irq(){
    if (reg_sf.get_i() == 0){ //
        //1.
        push_stack(reg_pc >> 8);
        push_stack(reg_pc & 0xFF);
        //reg_sf.set_b(false);
        push_stack(reg_sf.data);
        reg_sf.set_i(true);
        //2.
        uint8_t lo8 = p_ram->load(0xFFFE); //
        uint8_t hi8 = p_ram->load(0xFFFF);
        reg_pc = uint16_t(hi8 << 8) + lo8;
        //3.
        cycles_wait = 7;
    }
}

void CPU::nmi(){
    //1.
    push_stack(reg_pc >> 8);
    push_stack(reg_pc & 0xFF);
    //reg_sf.set_b(false);
    reg_sf.set_b(false);
    reg_sf.set_u(true);
    reg_sf.set_i(true);
    push_stack(reg_sf.data);
    //2.
    uint8_t lo8 = p_ram->load(0xFFFA); //
    uint8_t hi8 = p_ram->load(0xFFFB);
    reg_pc = uint16_t(hi8 << 8) + lo8;
    //3.
    //qDebug() << "NMI, reg_pc = " << reg_pc << endl;
    cycles_wait = 7;
}

void CPU::dma_sleep(){
    if (clock_count & 1){
        //
        cycles_wait += 514;
    }else{
        //
        cycles_wait += 513;
    }
}

int CPU::IMP(){
    return 0;
}

int CPU::IMM(){
    addr_res = reg_pc;
    reg_pc++;
    oprand_for_log = p_ram->load(addr_res);
    return 0;
}

int CPU::ZP0(){
    addr_res = p_ram->load(reg_pc);
    reg_pc++;
    addr_res &= 0x00FF;
    oprand_for_log = uint16_t(addr_res);
    return 0;
}

int CPU::ZPX(){
    oprand_for_log = p_ram->load(reg_pc);
    addr_res = p_ram->load(reg_pc) + reg_x;
    reg_pc++;
    addr_res &= 0x00FF;
    return 0;
}

int CPU::ZPY(){
    oprand_for_log = p_ram->load(reg_pc);
    addr_res = p_ram->load(reg_pc) + reg_y;
    reg_pc++;
    addr_res &= 0x00FF;
    return 0;
}

int CPU::REL(){
    addr_rel = p_ram->load(reg_pc);
    oprand_for_log = uint16_t(addr_rel);
    reg_pc++;
    if (addr_rel & 0x80)
        addr_rel |= 0xFF00;
    return 0;
}

int CPU::ABS(){
    uint8_t lo8 = p_ram->load(reg_pc);
    uint8_t hi8 = p_ram->load(reg_pc + 1);
    reg_pc += 2;
    addr_res = uint16_t(hi8 << 8) + lo8;
    oprand_for_log = uint16_t(addr_res);
    return 0;
}

int CPU::ABX(){
    uint8_t lo8 = p_ram->load(reg_pc);
    uint8_t hi8 = p_ram->load(reg_pc + 1);
    reg_pc += 2;
    addr_res = uint16_t(hi8 << 8) + lo8 + reg_x;
    oprand_for_log = uint16_t((hi8 << 8) + lo8);
    //
    if ((hi8 << 8) != (addr_res & 0xFF00))
        return 1;
    else
        return 0;
}

int CPU::ABY(){
    uint8_t lo8 = p_ram->load(reg_pc);
    uint8_t hi8 = p_ram->load(reg_pc + 1);
    reg_pc += 2;
    addr_res = uint16_t(hi8 << 8) + lo8 + reg_y;
    oprand_for_log = uint16_t((hi8 << 8) + lo8);
    //
    if ((hi8 << 8) != (addr_res & 0xFF00))
        return 1;
    else
        return 0;
}

int CPU::IND(){
    uint8_t p_lo8 = p_ram->load(reg_pc);
    uint8_t p_hi8 = p_ram->load(reg_pc + 1);
    reg_pc += 2;
    uint16_t ptr = uint16_t(p_hi8 << 8) + p_lo8;
    oprand_for_log = ptr;
    if (p_lo8 == 0xFF)
        addr_res = (p_ram->load(ptr & 0xFF00) << 8) + (p_ram->load(ptr + 0));
    else
        addr_res = (p_ram->load(ptr + 1) << 8) + (p_ram->load(ptr));
    return 0;
}

int CPU::IZX(){
    uint8_t ptr = p_ram->load(reg_pc);
    oprand_for_log = ptr;
    reg_pc++;
    uint8_t lo8 = p_ram->load((ptr + reg_x) & 0x00FF);
    uint8_t hi8 = p_ram->load((ptr + reg_x + 1) & 0x00FF);
    addr_res = (hi8 << 8) + lo8;
    return 0;
}

int CPU::IZY(){
    uint8_t ptr = p_ram->load(reg_pc);
    oprand_for_log = ptr;
    reg_pc++;
    uint8_t lo8 = p_ram->load(ptr & 0x00FF);
    uint8_t hi8 = p_ram->load((ptr + 1) & 0x00FF);
    addr_res = (hi8 << 8) + lo8 + reg_y;
    //
    if ((hi8 << 8) != (addr_res & 0xFF00))
        return 1;
    else
        return 0;
}

int CPU::ADC()
{
    //1.
    uint8_t operand = p_ram->load(addr_res);
    //2.
    uint16_t sum = reg_a + operand + reg_sf.get_c();
    reg_sf.set_c(sum >= 256);
    reg_sf.set_v((reg_a ^ sum) & (operand ^ sum) & 0x80);
    //reg_sf.set_v((~((uint16_t)reg_a ^ (uint16_t)operand) & ((uint16_t)reg_a ^ (uint16_t)sum)) & 0x0080);
    reg_sf.set_z((sum & 0xFF) == 0);
    reg_sf.set_n(sum & 0x80);
    reg_a = sum & 0xFF;
    return 1;
}

int CPU::AND(){
    //1.
    uint8_t operand = p_ram->load(addr_res);
    //2.
    reg_a = reg_a & operand;
    reg_sf.set_z(reg_a == 0);
    reg_sf.set_n(reg_a & 0x80);
    return 1;
}

int CPU::ASL(){
    if (inst_table[opcode].addrmode == &CPU::IMP){
        //IMP(Accumulator)
        uint16_t temp = uint16_t(reg_a << 1);
        reg_sf.set_c(temp >= 0x100);
        reg_sf.set_z((temp & 0x00FF) == 0);
        reg_sf.set_n(temp & 0x80);
        reg_a = temp & 0x00FF;
    }else{
        //
        uint8_t operand = p_ram->load(addr_res);
        uint16_t temp = uint16_t(operand << 1);
        reg_sf.set_c(temp >= 0x100);
        reg_sf.set_z((temp & 0x00FF) == 0);
        reg_sf.set_n(temp & 0x80);
        p_ram->save(addr_res, temp & 0x00FF);
    }
    return 0;
}

int CPU::BCC(){
    //C=0
    uint8_t cycles_add = 0;
    if (reg_sf.get_c() == 0){
        addr_res = reg_pc + addr_rel;
        //
        if ((addr_res & 0xFF00) != (reg_pc & 0xFF00))
            cycles_add = 2;
        else
            cycles_add = 1;
        reg_pc = uint16_t(addr_res);
    }
    return -cycles_add;
}

int CPU::BCS(){
    //C=1
    uint8_t cycles_add = 0;
    if (reg_sf.get_c() == 1){
        addr_res = reg_pc + addr_rel;
        //
        if ((addr_res & 0xFF00) != (reg_pc & 0xFF00))
            cycles_add = 2;
        else
            cycles_add = 1;
        reg_pc = uint16_t(addr_res);
    }
    return -cycles_add;
}

int CPU::BEQ(){
    //Z=1
    uint8_t cycles_add = 0;
    if (reg_sf.get_z() == 1){
        addr_res = reg_pc + addr_rel;
        //
        if ((addr_res & 0xFF00) != (reg_pc & 0xFF00))
            cycles_add = 2;
        else
            cycles_add = 1;
        reg_pc = uint16_t(addr_res);
    }
    return -cycles_add;
}

int CPU::BIT(){
    //1.
    uint8_t operand = p_ram->load(addr_res);
    // 2.
    reg_sf.set_z((reg_a & operand) == 0);
    reg_sf.set_v(operand & (1 << 6));
    reg_sf.set_n(operand & (1 << 7));
    return 0;
}

int CPU::BMI(){
    //N=1
    uint8_t cycles_add = 0;
    if (reg_sf.get_n() == 1){
        addr_res = reg_pc + addr_rel;
        //
        if ((addr_res & 0xFF00) != (reg_pc & 0xFF00))
            cycles_add = 2;
        else
            cycles_add = 1;
        reg_pc = uint16_t(addr_res);
    }
    return -cycles_add;
}

int CPU::BNE(){
    //Z=0
    uint8_t cycles_add = 0;
    if (reg_sf.get_z() == 0){
        addr_res = reg_pc + addr_rel;
        //
        if ((addr_res & 0xFF00) != (reg_pc & 0xFF00))
            cycles_add = 2;
        else
            cycles_add = 1;
        reg_pc = uint16_t(addr_res);
    }
    return -cycles_add;
}

int CPU::BPL(){
    //N=0
    uint8_t cycles_add = 0;
    if (reg_sf.get_n() == 0){
        addr_res = reg_pc + addr_rel;
        //
        if ((addr_res & 0xFF00) != (reg_pc & 0xFF00))
            cycles_add = 2;
        else
            cycles_add = 1;
        reg_pc = uint16_t(addr_res);
    }
    return -cycles_add;
}

int CPU::BRK(){ //
    reg_pc++;
    //1.
    push_stack(reg_pc >> 8);
    push_stack(reg_pc & 0xFF);
    reg_sf.set_b(true);
    reg_sf.set_i(true);
    push_stack(reg_sf.data);
    reg_sf.set_b(false);
    //2.
    uint8_t lo8 = p_ram->load(0xFFFE); //
    uint8_t hi8 = p_ram->load(0xFFFF);
    reg_pc = uint16_t(hi8 << 8) + lo8;
    return 0;
}

int CPU::BVC(){
    //V=0
    uint8_t cycles_add = 0;
    if (reg_sf.get_v() == 0){
        addr_res = reg_pc + addr_rel;
        //
        if ((addr_res & 0xFF00) != (reg_pc & 0xFF00))
            cycles_add = 2;
        else
            cycles_add = 1;
        reg_pc = uint16_t(addr_res);
    }
    return -cycles_add;
}

int CPU::BVS(){
    //V=1
    uint8_t cycles_add = 0;
    if (reg_sf.get_v() == 1){
        addr_res = reg_pc + addr_rel;
        //
        if ((addr_res & 0xFF00) != (reg_pc & 0xFF00))
            cycles_add = 2;
        else
            cycles_add = 1;
        reg_pc = uint16_t(addr_res);
    }
    return -cycles_add;
}
int CPU::CLC(){
    //
    reg_sf.set_c(false);
    return 0;
}

int CPU::CLD(){
    //
    reg_sf.set_d(false);
    return 0;
}

int CPU::CLI(){
    //
    reg_sf.set_i(false);
    return 0;
}

int CPU::CLV(){
    //
    reg_sf.set_v(false);
    return 0;
}

int CPU::CMP(){
    //1.
    uint8_t operand = p_ram->load(addr_res);
    //2.
    uint16_t temp = reg_a - operand;
    reg_sf.set_c(reg_a >= operand);
    reg_sf.set_z((temp & 0x00FF) == 0);
    reg_sf.set_n(bool(temp & 0x0080)); //
    return 1;
}

int CPU::CPX(){
    //1.
    uint8_t operand = p_ram->load(addr_res);
    //2.
    uint16_t temp = reg_x - operand;
    reg_sf.set_c(reg_x >= operand);
    reg_sf.set_z((temp & 0x00FF) == 0);
    reg_sf.set_n(bool(temp & 0x0080)); //
    return 0;
}

int CPU::CPY(){
    //1.
    uint8_t operand = p_ram->load(addr_res);
    //2.
    uint16_t temp = reg_y - operand;
    reg_sf.set_c(reg_y >= operand);
    reg_sf.set_z((temp & 0x00FF) == 0);
    reg_sf.set_n(bool(temp & 0x0080)); //
    return 0;
}

int CPU::DEC(){
    //1.
    uint8_t operand = p_ram->load(addr_res);
    //2.
    uint16_t res = operand - 1;
    p_ram->save(addr_res, res & 0x00FF);
    reg_sf.set_z((res & 0x00FF) == 0);
    reg_sf.set_n(bool(res & 0x0080));
    return 0;
}

int CPU::DEX(){
    //X
    reg_x--;
    reg_sf.set_z(reg_x == 0);
    reg_sf.set_n(bool(reg_x & 0x0080));
    return 0;
}

int CPU::DEY(){
    //Y
    reg_y--;
    reg_sf.set_z(reg_y == 0);
    reg_sf.set_n(bool(reg_y & 0x0080));
    return 0;
}

int CPU::EOR(){
    //1.
    uint8_t operand = p_ram->load(addr_res);
    //2.
    reg_a = reg_a ^ operand;
    reg_sf.set_z(reg_a == 0);
    reg_sf.set_n(bool(reg_a & 0x0080));
    return 1;
}

int CPU::INC(){
    //1
    uint8_t operand = p_ram->load(addr_res);
    //2.
    uint16_t res = operand + 1;
    p_ram->save(addr_res, res & 0x00FF);
    reg_sf.set_z((res & 0x00FF) == 0);
    reg_sf.set_n(bool(res & 0x0080));
    return 0;
}

int CPU::INX(){
    //
    reg_x++;
    reg_sf.set_z(reg_x == 0);
    reg_sf.set_n(bool(reg_x & 0x0080));
    return 0;
}

int CPU::INY(){
    //
    reg_y++;
    reg_sf.set_z(reg_y == 0);
    reg_sf.set_n(bool(reg_y & 0x0080));
    return 0;
}

int CPU::JMP(){
    //
    reg_pc = uint16_t(addr_res);
    return 0;
}

int CPU::JSR(){
    //
    //1.
    push_stack((reg_pc - 1) >> 8);
    push_stack((reg_pc - 1) & 0xFF);
    //2.
    reg_pc = uint16_t(addr_res);
    return 0;
}

int CPU::LDA(){
    //1.
    uint8_t operand = p_ram->load(addr_res);
    //2.
    reg_a = operand;
    reg_sf.set_z(reg_a == 0);
    reg_sf.set_n(bool(reg_a & 0x0080));
    return 1;
}

int CPU::LDX(){
    //1.
    uint8_t operand = p_ram->load(addr_res);
    //2.
    reg_x = operand;
    reg_sf.set_z(reg_x == 0);
    reg_sf.set_n(bool(reg_x & 0x0080));
    return 1;
}

int CPU::LDY(){
    //1.
    uint8_t operand = p_ram->load(addr_res);
    //2.
    reg_y = operand;
    reg_sf.set_z(reg_y == 0);
    reg_sf.set_n(bool(reg_y & 0x0080));
    return 1;
}

int CPU::LSR(){
    if (inst_table[opcode].addrmode == &CPU::IMP){
        //IMP(Accumulator)
        uint16_t temp = uint16_t(reg_a >> 1);
        reg_sf.set_c(reg_a & 0x0001);
        reg_sf.set_z((temp & 0x00FF) == 0);
        reg_sf.set_n(temp & 0x80);
        reg_a = temp & 0x00FF;
    }else{
        //
        uint8_t operand = p_ram->load(addr_res);
        uint16_t temp = uint16_t(operand >> 1);
        reg_sf.set_c(operand & 0x0001);
        reg_sf.set_z((temp & 0x00FF) == 0);
        reg_sf.set_n(temp & 0x80);
        p_ram->save(addr_res, temp & 0x00FF);
    }
    return 0;
}

int CPU::NOP(){
    //
    switch (opcode) {
    case 0x1C:
    case 0x3C:
    case 0x5C:
    case 0x7C:
    case 0xDC:
    case 0xFC:
        return 1;
        break;
    }
    return 0;
}

int CPU::ORA(){
    //1.
    uint8_t operand = p_ram->load(addr_res);
    //2.
    reg_a = reg_a | operand;
    reg_sf.set_z(reg_a == 0);
    reg_sf.set_n(bool(reg_a & 0x0080));
    return 1;
}

int CPU::PHA(){
    //
    push_stack(reg_a);
    return 0;
}

int CPU::PHP(){
    //
    push_stack(reg_sf.data | (1 << 4) | (1 << 5));
    reg_sf.set_b(false);
    reg_sf.set_u(false);
    return 0;
}

int CPU::PLA(){
    //
    reg_a = pull_stack();
    reg_sf.set_z(reg_a == 0);
    reg_sf.set_n(bool(reg_a & 0x0080));
    return 0;
}

int CPU::PLP(){
    //
    reg_sf.data = pull_stack();
    reg_sf.set_u(true);
    return 0;
}

int CPU::ROL(){
    //
    if (inst_table[opcode].addrmode == &CPU::IMP){
        //
        uint16_t temp = uint16_t(reg_a << 1) | reg_sf.get_c(); //
        reg_sf.set_c(temp >= 0x100);
        reg_sf.set_z((temp & 0x00FF) == 0);
        reg_sf.set_n(temp & 0x80);
        reg_a = temp & 0x00FF;
    }else{
        //
        uint8_t operand = p_ram->load(addr_res);
        uint16_t temp = uint16_t(operand << 1) | reg_sf.get_c();
        reg_sf.set_c(temp >= 0x100);
        reg_sf.set_z((temp & 0x00FF) == 0);
        reg_sf.set_n(temp & 0x80);
        p_ram->save(addr_res, temp & 0x00FF);
    }
    return 0;
}

int CPU::ROR(){
    //
    if (inst_table[opcode].addrmode == &CPU::IMP){
        //IMP(Accumulator)
        uint16_t temp = uint16_t(reg_a >> 1) | uint16_t(reg_sf.get_c() << 7); //
        reg_sf.set_c(reg_a & 0x0001);
        reg_sf.set_z((temp & 0x00FF) == 0);
        reg_sf.set_n(temp & 0x80);
        reg_a = temp & 0x00FF;
    }else{
        //
        uint8_t operand = p_ram->load(addr_res);
        uint16_t temp = uint16_t(operand >> 1) | uint16_t(reg_sf.get_c() << 7);
        reg_sf.set_c(operand & 0x0001);
        reg_sf.set_z((temp & 0x00FF) == 0);
        reg_sf.set_n(temp & 0x80);
        p_ram->save(addr_res, temp & 0x00FF);
    }
    return 0;
}

int CPU::RTI(){
    //
    reg_sf.data = pull_stack();
    reg_sf.set_b(false);
    reg_sf.set_u(false);
    uint8_t pc_lo8 = pull_stack();
    uint8_t pc_hi8 = pull_stack();
    reg_pc = uint16_t(pc_hi8 << 8) + pc_lo8;
    return 0;
}

int CPU::RTS(){
    //
    uint8_t pc_lo8 = pull_stack();
    uint8_t pc_hi8 = pull_stack();
    reg_pc = uint16_t(pc_hi8 << 8) + pc_lo8;
    reg_pc++;
    return 0;
}

int CPU::SBC()
{
    //1.
    uint8_t operand = p_ram->load(addr_res);
    //2.
    uint16_t sub = reg_a - operand - (!reg_sf.get_c());
    //reg_sf.set_c(sub >= 256);
    reg_sf.set_c(!(sub & 0x100));
    reg_sf.set_v((reg_a ^ sub) & ((~operand) ^ sub) & 0x80);
    reg_sf.set_z((sub & 0xFF) == 0);
    reg_sf.set_n(sub & 0x80);
    reg_a = sub & 0x00FF;
    return 1;
}

int CPU::SEC(){
    //
    reg_sf.set_c(true);
    return 0;
}

int CPU::SED(){
    //
    reg_sf.set_d(true);
    return 0;
}

int CPU::SEI(){
    //
    reg_sf.set_i(true);
    return 0;
}

int CPU::STA(){
    //
    p_ram->save(addr_res, reg_a);
    return 0;
}

int CPU::STX(){
    //
    p_ram->save(addr_res, reg_x);
    return 0;
}

int CPU::STY(){
    //
    p_ram->save(addr_res, reg_y);
    return 0;
}

int CPU::TAX(){
    //
    reg_x = reg_a;
    reg_sf.set_z(reg_x == 0);
    reg_sf.set_n(reg_x & 0x0080);
    return 0;
}

int CPU::TAY(){
    //
    reg_y = reg_a;
    reg_sf.set_z(reg_y == 0);
    reg_sf.set_n(reg_y & 0x0080);
    return 0;
}

int CPU::TSX(){
    //
    reg_x = reg_sp;
    reg_sf.set_z(reg_x == 0);
    reg_sf.set_n(reg_x & 0x0080);
    return 0;
}

int CPU::TXA(){
    //
    reg_a = reg_x;
    reg_sf.set_z(reg_a == 0);
    reg_sf.set_n(reg_a & 0x0080);
    return 0;
}

int CPU::TXS(){
    //
    reg_sp = reg_x;
    return 0;
}

int CPU::TYA(){
    //
    reg_a = reg_y;
    reg_sf.set_z(reg_a == 0);
    reg_sf.set_n(reg_a & 0x0080);
    return 0;
}

int CPU::XXX(){
    qDebug() << "zhixing daole shangwei dingyi de mingling: " << opcode << endl;
    abort();
    return 0;
}

void CPU::run_1cycle()
{
    if (cycles_wait == 0){
        //
        //1.
        opcode = p_ram->load(reg_pc);
        reg_pc++;
        reg_sf.set_u(true);
        //2.
        int cycles_add_by_addrmode = (this->*inst_table[opcode].addrmode)();
        if (Ppu2.frame_dx == 3)
            print_log();
        int cycles_add_by_operate = (this->*inst_table[opcode].operate)();
        //3.
        cycles_wait = this->inst_table[opcode].cycle_cnt;
        if (cycles_add_by_operate < 0)
            cycles_wait += (-cycles_add_by_operate);
        else
            cycles_wait += (cycles_add_by_operate & cycles_add_by_addrmode);
        reg_sf.set_u(true);
    }

    cycles_wait--;
    clock_count++;
}

void CPU::print_log() const
{
    char addr_str[20] = {0};
    if (inst_table[opcode].addrmode == &CPU::IMP){
        // do nothing.
    }else if (inst_table[opcode].addrmode == &CPU::IMM){
        sprintf(addr_str, " #%02xH", oprand_for_log);
    }else if (inst_table[opcode].addrmode == &CPU::ZP0){
        sprintf(addr_str, " %02xH", oprand_for_log);
    }else if (inst_table[opcode].addrmode == &CPU::ZPX){
        sprintf(addr_str, " %02xH, X", oprand_for_log);
    }else if (inst_table[opcode].addrmode == &CPU::ZPY){
        sprintf(addr_str, " %02xH, Y", oprand_for_log);
    }else if (inst_table[opcode].addrmode == &CPU::REL){
        sprintf(addr_str, " %04xH", oprand_for_log);
    }else if (inst_table[opcode].addrmode == &CPU::ABS){
        sprintf(addr_str, " %04xH", oprand_for_log);
    }else if (inst_table[opcode].addrmode == &CPU::ABX){
        sprintf(addr_str, " %04xH, X", oprand_for_log);
    }else if (inst_table[opcode].addrmode == &CPU::ABY){
        sprintf(addr_str, " %04xH, Y", oprand_for_log);
    }else if (inst_table[opcode].addrmode == &CPU::IND){
        sprintf(addr_str, " (%04xH)", oprand_for_log);
    }else if (inst_table[opcode].addrmode == &CPU::IZX){
        sprintf(addr_str, " (%04xH, X)", oprand_for_log);
    }else if (inst_table[opcode].addrmode == &CPU::IZY){
        sprintf(addr_str, " (%04xH), Y", oprand_for_log);
    }
    //qDebug() << "Program Counter = " << reg_pc << ", opcode = " << opcode << ", code = " << this->inst_table[opcode].name.c_str() << addr_str << ", A = " << this->reg_a << ", X = " << this->reg_x << ", Y = " << this->reg_y << ", stack pointer = " << reg_sp << ", NVDIZC = " << int(reg_sf.get_n()) << int(reg_sf.get_v()) << int(reg_sf.get_d()) << int(reg_sf.get_i()) << int(reg_sf.get_z()) << int(reg_sf.get_c()) << endl;
    //qDebug() << "Program Counter = " << reg_pc << ", opcode = " << opcode << ", code = " << this->inst_table[opcode].name.c_str() << addr_str << endl;
}
