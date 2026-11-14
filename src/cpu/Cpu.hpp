#pragma once

#include <stdexcept>

#include "Memory.hpp"
#include "Registers.hpp"
using namespace std;

// Runs one instruction at a time: read the byte at PC, do what it says.
//
// Not done yet: interrupts, the timer, HALT waiting, and cycle counts.
//
// Many instructions follow a pattern in the opcode's bits, so the code reads
// the pattern instead of listing every opcode. For example 0x40-0x7F is
// "LD to, from" where the low 3 bits pick `from` and the next 3 pick `to`:
//   register numbers: 0=B 1=C 2=D 3=E 4=H 5=L 6=(HL) 7=A
// (HL) means "the memory byte at the address in HL".
class Cpu {
 public:
  Cpu(Memory& memory) : memory_(memory) { registers.set_post_boot_values(); }

  Registers registers;

  void step() {
    int opcode = fetch_byte();

    if (opcode == 0xcb) {
      run_cb_instruction();
    } else if (opcode >= 0x40 && opcode <= 0x7f) {
      if (opcode == 0x76) {
        return;  // HALT: not implemented, treated as NOP
      }
      set_register((opcode >> 3) & 7, get_register(opcode & 7));
    } else if (opcode >= 0x80 && opcode <= 0xbf) {
      alu((opcode >> 3) & 7, get_register(opcode & 7));
    } else if (opcode < 0x40) {
      run_block_0(opcode);
    } else {
      run_block_3(opcode);
    }
  }

 private:
  Memory& memory_;
  bool interrupts_enabled_ = false;

  // ---- reading the program ----
  int fetch_byte() {
    int value = memory_.read(registers.pc());
    registers.set_pc(registers.pc() + 1);
    return value;
  }

  int fetch_word() {
    int low = fetch_byte();
    int high = fetch_byte();
    return (high << 8) | low;
  }

  // A signed byte: 0x00-0x7F is 0 to 127, 0x80-0xFF is -128 to -1.
  int fetch_signed_byte() { return static_cast<int8_t>(fetch_byte()); }

  // ---- the stack (grows downward) ----
  void push_word(int value) {
    registers.set_sp(registers.sp() - 1);
    memory_.write(registers.sp(), value >> 8);
    registers.set_sp(registers.sp() - 1);
    memory_.write(registers.sp(), value & 0xff);
  }

  int pop_word() {
    int low = memory_.read(registers.sp());
    registers.set_sp(registers.sp() + 1);
    int high = memory_.read(registers.sp());
    registers.set_sp(registers.sp() + 1);
    return (high << 8) | low;
  }

  // ---- picking registers by number ----
  int get_register(int number) {
    switch (number) {
      case 0: return registers.b();
      case 1: return registers.c();
      case 2: return registers.d();
      case 3: return registers.e();
      case 4: return registers.h();
      case 5: return registers.l();
      case 6: return memory_.read(registers.hl());
      default: return registers.a();
    }
  }

  void set_register(int number, int value) {
    switch (number) {
      case 0: registers.set_b(value); break;
      case 1: registers.set_c(value); break;
      case 2: registers.set_d(value); break;
      case 3: registers.set_e(value); break;
      case 4: registers.set_h(value); break;
      case 5: registers.set_l(value); break;
      case 6: memory_.write(registers.hl(), value); break;
      default: registers.set_a(value); break;
    }
  }

  // 16-bit pairs by number: 0=BC 1=DE 2=HL 3=SP
  int get_pair(int number) {
    switch (number) {
      case 0: return registers.bc();
      case 1: return registers.de();
      case 2: return registers.hl();
      default: return registers.sp();
    }
  }

  void set_pair(int number, int value) {
    switch (number) {
      case 0: registers.set_bc(value); break;
      case 1: registers.set_de(value); break;
      case 2: registers.set_hl(value); break;
      default: registers.set_sp(value); break;
    }
  }

  // Conditions by number: 0=NZ 1=Z 2=NC 3=C
  bool condition_is_true(int number) {
    switch (number) {
      case 0: return !registers.zero_flag();
      case 1: return registers.zero_flag();
      case 2: return !registers.carry_flag();
      default: return registers.carry_flag();
    }
  }

  // ---- math on A ----
  // kind: 0=ADD 1=ADC 2=SUB 3=SBC 4=AND 5=XOR 6=OR 7=CP
  void alu(int kind, int value) {
    int a = registers.a();
    int carry_in = registers.carry_flag() ? 1 : 0;
    int result = 0;

    if (kind == 0 || kind == 1) {
      if (kind == 0) {
        carry_in = 0;
      }
      result = a + value + carry_in;
      registers.set_half_carry_flag(((a & 0xf) + (value & 0xf) + carry_in) > 0xf);
      registers.set_carry_flag(result > 0xff);
      registers.set_subtract_flag(false);
    } else if (kind == 2 || kind == 3 || kind == 7) {
      if (kind == 2 || kind == 7) {
        carry_in = 0;
      }
      result = a - value - carry_in;
      registers.set_half_carry_flag(((a & 0xf) - (value & 0xf) - carry_in) < 0);
      registers.set_carry_flag(result < 0);
      registers.set_subtract_flag(true);
    } else {
      if (kind == 4) {
        result = a & value;
      } else if (kind == 5) {
        result = a ^ value;
      } else {
        result = a | value;
      }
      registers.set_half_carry_flag(kind == 4);
      registers.set_carry_flag(false);
      registers.set_subtract_flag(false);
    }

    registers.set_zero_flag((result & 0xff) == 0);
    if (kind != 7) {  // CP only sets flags, it throws the result away
      registers.set_a(result);
    }
  }

  // INC and DEC keep the carry flag as it was.
  int increment(int value) {
    int result = (value + 1) & 0xff;
    registers.set_zero_flag(result == 0);
    registers.set_subtract_flag(false);
    registers.set_half_carry_flag((value & 0xf) == 0xf);
    return result;
  }

  int decrement(int value) {
    int result = (value - 1) & 0xff;
    registers.set_zero_flag(result == 0);
    registers.set_subtract_flag(true);
    registers.set_half_carry_flag((value & 0xf) == 0);
    return result;
  }

  void add_to_hl(int value) {
    int hl = registers.hl();
    registers.set_subtract_flag(false);
    registers.set_half_carry_flag(((hl & 0xfff) + (value & 0xfff)) > 0xfff);
    registers.set_carry_flag(hl + value > 0xffff);
    registers.set_hl(hl + value);
  }

  // SP plus a signed byte. Used by ADD SP,e and LD HL,SP+e.
  int sp_plus_signed_byte() {
    int sp = registers.sp();
    int offset = fetch_signed_byte();
    registers.set_zero_flag(false);
    registers.set_subtract_flag(false);
    registers.set_half_carry_flag(((sp & 0xf) + (offset & 0xf)) > 0xf);
    registers.set_carry_flag(((sp & 0xff) + (offset & 0xff)) > 0xff);
    return sp + offset;
  }

  // Fix A after adding or subtracting two decimal-coded numbers (like 0x19).
  void decimal_adjust() {
    int a = registers.a();
    if (!registers.subtract_flag()) {
      if (registers.carry_flag() || a > 0x99) {
        a += 0x60;
        registers.set_carry_flag(true);
      }
      if (registers.half_carry_flag() || (a & 0xf) > 9) {
        a += 0x06;
      }
    } else {
      if (registers.carry_flag()) {
        a -= 0x60;
      }
      if (registers.half_carry_flag()) {
        a -= 0x06;
      }
    }
    registers.set_zero_flag((a & 0xff) == 0);
    registers.set_half_carry_flag(false);
    registers.set_a(a);
  }

  // ---- rotates and shifts ----
  // kind: 0=RLC 1=RRC 2=RL 3=RR 4=SLA 5=SRA 6=SWAP 7=SRL
  int rotate_or_shift(int kind, int value) {
    int carry_in = registers.carry_flag() ? 1 : 0;
    int carry_out = 0;
    int result = 0;

    switch (kind) {
      case 0:
        carry_out = value >> 7;
        result = (value << 1) | carry_out;
        break;
      case 1:
        carry_out = value & 1;
        result = (value >> 1) | (carry_out << 7);
        break;
      case 2:
        carry_out = value >> 7;
        result = (value << 1) | carry_in;
        break;
      case 3:
        carry_out = value & 1;
        result = (value >> 1) | (carry_in << 7);
        break;
      case 4:
        carry_out = value >> 7;
        result = value << 1;
        break;
      case 5:
        carry_out = value & 1;
        result = (value >> 1) | (value & 0x80);
        break;
      case 6:
        carry_out = 0;
        result = (value << 4) | (value >> 4);
        break;
      default:
        carry_out = value & 1;
        result = value >> 1;
        break;
    }

    result = result & 0xff;
    registers.set_zero_flag(result == 0);
    registers.set_subtract_flag(false);
    registers.set_half_carry_flag(false);
    registers.set_carry_flag(carry_out == 1);
    return result;
  }

  // 0xCB is a prefix: the next byte picks the real instruction.
  //   bits 7-6: 0 = rotate/shift, 1 = BIT, 2 = RES, 3 = SET
  //   bits 5-3: which shift, or which bit number
  //   bits 2-0: which register
  void run_cb_instruction() {
    int opcode = fetch_byte();
    int group = opcode >> 6;
    int middle = (opcode >> 3) & 7;
    int register_number = opcode & 7;
    int value = get_register(register_number);

    if (group == 0) {
      set_register(register_number, rotate_or_shift(middle, value));
    } else if (group == 1) {
      registers.set_zero_flag(((value >> middle) & 1) == 0);
      registers.set_subtract_flag(false);
      registers.set_half_carry_flag(true);
    } else if (group == 2) {
      set_register(register_number, value & ~(1 << middle));
    } else {
      set_register(register_number, value | (1 << middle));
    }
  }

  // ---- opcodes 0x00-0x3F ----
  void run_block_0(int opcode) {
    int pair = (opcode >> 4) & 3;

    switch (opcode & 0x0f) {
      case 0x01:  // LD pair,nn
        set_pair(pair, fetch_word());
        return;
      case 0x03:  // INC pair
        set_pair(pair, get_pair(pair) + 1);
        return;
      case 0x0b:  // DEC pair
        set_pair(pair, get_pair(pair) - 1);
        return;
      case 0x09:  // ADD HL,pair
        add_to_hl(get_pair(pair));
        return;
    }

    switch (opcode & 7) {
      case 4:  // INC register
        set_register((opcode >> 3) & 7, increment(get_register((opcode >> 3) & 7)));
        return;
      case 5:  // DEC register
        set_register((opcode >> 3) & 7, decrement(get_register((opcode >> 3) & 7)));
        return;
      case 6:  // LD register,n
        set_register((opcode >> 3) & 7, fetch_byte());
        return;
    }

    switch (opcode) {
      case 0x00:  // NOP
        return;
      case 0x02: memory_.write(registers.bc(), registers.a()); return;
      case 0x12: memory_.write(registers.de(), registers.a()); return;
      case 0x22:  // LD (HL+),A
        memory_.write(registers.hl(), registers.a());
        registers.set_hl(registers.hl() + 1);
        return;
      case 0x32:  // LD (HL-),A
        memory_.write(registers.hl(), registers.a());
        registers.set_hl(registers.hl() - 1);
        return;
      case 0x0a: registers.set_a(memory_.read(registers.bc())); return;
      case 0x1a: registers.set_a(memory_.read(registers.de())); return;
      case 0x2a:  // LD A,(HL+)
        registers.set_a(memory_.read(registers.hl()));
        registers.set_hl(registers.hl() + 1);
        return;
      case 0x3a:  // LD A,(HL-)
        registers.set_a(memory_.read(registers.hl()));
        registers.set_hl(registers.hl() - 1);
        return;
      case 0x07: case 0x0f: case 0x17: case 0x1f: {
        // RLCA RRCA RLA RRA: same as the CB versions, but Z is always 0
        int kind = (opcode >> 3) & 3;
        registers.set_a(rotate_or_shift(kind, registers.a()));
        registers.set_zero_flag(false);
        return;
      }
      case 0x08: {  // LD (nn),SP
        int address = fetch_word();
        memory_.write(address, registers.sp() & 0xff);
        memory_.write(address + 1, registers.sp() >> 8);
        return;
      }
      case 0x10:  // STOP: skip its extra byte, do nothing
        fetch_byte();
        return;
      case 0x18: {  // JR e
        int offset = fetch_signed_byte();
        registers.set_pc(registers.pc() + offset);
        return;
      }
      case 0x20: case 0x28: case 0x30: case 0x38: {  // JR cc,e
        int offset = fetch_signed_byte();
        if (condition_is_true((opcode >> 3) & 3)) {
          registers.set_pc(registers.pc() + offset);
        }
        return;
      }
      case 0x27:
        decimal_adjust();
        return;
      case 0x2f:  // CPL
        registers.set_a(~registers.a());
        registers.set_subtract_flag(true);
        registers.set_half_carry_flag(true);
        return;
      case 0x37:  // SCF
        registers.set_subtract_flag(false);
        registers.set_half_carry_flag(false);
        registers.set_carry_flag(true);
        return;
      case 0x3f:  // CCF
        registers.set_subtract_flag(false);
        registers.set_half_carry_flag(false);
        registers.set_carry_flag(!registers.carry_flag());
        return;
    }
    throw runtime_error("unknown opcode");
  }

  // ---- opcodes 0xC0-0xFF ----
  void run_block_3(int opcode) {
    switch (opcode & 0xcf) {
      case 0xc1: {  // POP pair
        int value = pop_word();
        int pair = (opcode >> 4) & 3;
        if (pair == 3) {
          registers.set_af(value);
        } else {
          set_pair(pair, value);
        }
        return;
      }
      case 0xc5: {  // PUSH pair
        int pair = (opcode >> 4) & 3;
        push_word(pair == 3 ? registers.af() : get_pair(pair));
        return;
      }
    }

    switch (opcode & 0xc7) {
      case 0xc6:  // ADD/ADC/SUB/SBC/AND/XOR/OR/CP with a number
        alu((opcode >> 3) & 7, fetch_byte());
        return;
      case 0xc7:  // RST: call one of 8 fixed addresses
        push_word(registers.pc());
        registers.set_pc(opcode & 0x38);
        return;
    }

    switch (opcode & 0xe7) {
      case 0xc0:  // RET cc
        if (condition_is_true((opcode >> 3) & 3)) {
          registers.set_pc(pop_word());
        }
        return;
      case 0xc2: {  // JP cc,nn
        int address = fetch_word();
        if (condition_is_true((opcode >> 3) & 3)) {
          registers.set_pc(address);
        }
        return;
      }
      case 0xc4: {  // CALL cc,nn
        int address = fetch_word();
        if (condition_is_true((opcode >> 3) & 3)) {
          push_word(registers.pc());
          registers.set_pc(address);
        }
        return;
      }
    }

    switch (opcode) {
      case 0xc3: registers.set_pc(fetch_word()); return;  // JP nn
      case 0xc9: registers.set_pc(pop_word()); return;    // RET
      case 0xd9:                                          // RETI
        registers.set_pc(pop_word());
        interrupts_enabled_ = true;
        return;
      case 0xcd: {  // CALL nn
        int address = fetch_word();
        push_word(registers.pc());
        registers.set_pc(address);
        return;
      }
      case 0xe0: memory_.write(0xff00 + fetch_byte(), registers.a()); return;
      case 0xf0: registers.set_a(memory_.read(0xff00 + fetch_byte())); return;
      case 0xe2: memory_.write(0xff00 + registers.c(), registers.a()); return;
      case 0xf2: registers.set_a(memory_.read(0xff00 + registers.c())); return;
      case 0xea: memory_.write(fetch_word(), registers.a()); return;
      case 0xfa: registers.set_a(memory_.read(fetch_word())); return;
      case 0xe8: registers.set_sp(sp_plus_signed_byte()); return;
      case 0xf8: registers.set_hl(sp_plus_signed_byte()); return;
      case 0xe9: registers.set_pc(registers.hl()); return;  // JP HL
      case 0xf9: registers.set_sp(registers.hl()); return;  // LD SP,HL
      case 0xf3: interrupts_enabled_ = false; return;       // DI
      case 0xfb: interrupts_enabled_ = true; return;        // EI
    }
    throw runtime_error("unknown opcode");
  }
};
