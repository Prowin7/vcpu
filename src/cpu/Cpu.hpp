#pragma once

#include <stdexcept>

#include "Memory.hpp"
#include "Registers.hpp"
using namespace std;

// Runs one instruction at a time: read the byte at PC (the "opcode"), then do
// what that opcode says. Each opcode has its own case in the big switch.
//
// Not done yet: interrupts, the timer, HALT waiting, and cycle counts.
//
// Some instructions work on "(HL)", which means the memory byte at the
// address stored in HL. The register helpers below treat it like a register.

// Register numbers, used by the 0x40-0xBF opcodes and the 0xCB opcodes.
const int REG_B = 0;
const int REG_C = 1;
const int REG_D = 2;
const int REG_E = 3;
const int REG_H = 4;
const int REG_L = 5;
const int REG_HL_MEMORY = 6;
const int REG_A = 7;

// Math operations. The order matches the order of the 0x80-0xBF opcodes.
const int MATH_ADD = 0;
const int MATH_ADD_WITH_CARRY = 1;
const int MATH_SUBTRACT = 2;
const int MATH_SUBTRACT_WITH_CARRY = 3;
const int MATH_AND = 4;
const int MATH_XOR = 5;
const int MATH_OR = 6;
const int MATH_COMPARE = 7;

class Cpu {
 public:
  // The CPU does not own the memory, it uses the one it is given.
  // ": memory_(memory)" stores that memory in memory_ below.
  Cpu(Memory& memory) : memory_(memory) {
    registers.set_post_boot_values();
  }

  Registers registers;

  void step() {
    int opcode = fetch_byte();

    // 0x40-0x7F: copy one register into another (0x76 is HALT, not done yet).
    // Opcodes go in rows of 8, one per source register:
    //   destination = (opcode - 0x40) / 8,  source = opcode % 8
    if (opcode >= 0x40 && opcode <= 0x7f) {
      if (opcode != 0x76) {
        int destination = (opcode - 0x40) / 8;
        int source = opcode % 8;
        set_register(destination, get_register(source));
      }
      return;
    }

    // 0x80-0xBF: math on A using a register. Rows of 8 again:
    //   operation = (opcode - 0x80) / 8,  source = opcode % 8
    if (opcode >= 0x80 && opcode <= 0xbf) {
      int operation = (opcode - 0x80) / 8;
      int source = opcode % 8;
      do_math_on_a(operation, get_register(source));
      return;
    }

    switch (opcode) {
      case 0x00: break;  // NOP: do nothing
      case 0xcb: run_cb_instruction(); break;

      // ---- load a number into a register ----
      case 0x06: set_register(REG_B, fetch_byte()); break;
      case 0x0e: set_register(REG_C, fetch_byte()); break;
      case 0x16: set_register(REG_D, fetch_byte()); break;
      case 0x1e: set_register(REG_E, fetch_byte()); break;
      case 0x26: set_register(REG_H, fetch_byte()); break;
      case 0x2e: set_register(REG_L, fetch_byte()); break;
      case 0x36: set_register(REG_HL_MEMORY, fetch_byte()); break;
      case 0x3e: set_register(REG_A, fetch_byte()); break;

      // ---- load a 16-bit number into a pair ----
      case 0x01: registers.set_bc(fetch_word()); break;
      case 0x11: registers.set_de(fetch_word()); break;
      case 0x21: registers.set_hl(fetch_word()); break;
      case 0x31: registers.set_sp(fetch_word()); break;

      // ---- load between A and memory ----
      case 0x02: memory_.write(registers.bc(), registers.a()); break;
      case 0x12: memory_.write(registers.de(), registers.a()); break;
      case 0x0a: registers.set_a(memory_.read(registers.bc())); break;
      case 0x1a: registers.set_a(memory_.read(registers.de())); break;
      case 0x22:  // LD (HL+),A: write A, then HL goes up by 1
        memory_.write(registers.hl(), registers.a());
        registers.set_hl(registers.hl() + 1);
        break;
      case 0x32:  // LD (HL-),A: write A, then HL goes down by 1
        memory_.write(registers.hl(), registers.a());
        registers.set_hl(registers.hl() - 1);
        break;
      case 0x2a:  // LD A,(HL+)
        registers.set_a(memory_.read(registers.hl()));
        registers.set_hl(registers.hl() + 1);
        break;
      case 0x3a:  // LD A,(HL-)
        registers.set_a(memory_.read(registers.hl()));
        registers.set_hl(registers.hl() - 1);
        break;
      case 0xea: memory_.write(fetch_word(), registers.a()); break;
      case 0xfa: registers.set_a(memory_.read(fetch_word())); break;
      case 0xe0: memory_.write(0xff00 + fetch_byte(), registers.a()); break;
      case 0xf0: registers.set_a(memory_.read(0xff00 + fetch_byte())); break;
      case 0xe2: memory_.write(0xff00 + registers.c(), registers.a()); break;
      case 0xf2: registers.set_a(memory_.read(0xff00 + registers.c())); break;
      case 0x08: {  // LD (nn),SP
        int address = fetch_word();
        memory_.write(address, registers.sp() & 0xff);
        memory_.write(address + 1, registers.sp() >> 8);
        break;
      }
      case 0xf9: registers.set_sp(registers.hl()); break;
      case 0xf8: registers.set_hl(sp_plus_signed_byte()); break;

      // ---- add or subtract 1 ----
      case 0x04: set_register(REG_B, add_one(get_register(REG_B))); break;
      case 0x0c: set_register(REG_C, add_one(get_register(REG_C))); break;
      case 0x14: set_register(REG_D, add_one(get_register(REG_D))); break;
      case 0x1c: set_register(REG_E, add_one(get_register(REG_E))); break;
      case 0x24: set_register(REG_H, add_one(get_register(REG_H))); break;
      case 0x2c: set_register(REG_L, add_one(get_register(REG_L))); break;
      case 0x34: set_register(REG_HL_MEMORY, add_one(get_register(REG_HL_MEMORY))); break;
      case 0x3c: set_register(REG_A, add_one(get_register(REG_A))); break;

      case 0x05: set_register(REG_B, subtract_one(get_register(REG_B))); break;
      case 0x0d: set_register(REG_C, subtract_one(get_register(REG_C))); break;
      case 0x15: set_register(REG_D, subtract_one(get_register(REG_D))); break;
      case 0x1d: set_register(REG_E, subtract_one(get_register(REG_E))); break;
      case 0x25: set_register(REG_H, subtract_one(get_register(REG_H))); break;
      case 0x2d: set_register(REG_L, subtract_one(get_register(REG_L))); break;
      case 0x35: set_register(REG_HL_MEMORY, subtract_one(get_register(REG_HL_MEMORY))); break;
      case 0x3d: set_register(REG_A, subtract_one(get_register(REG_A))); break;

      case 0x03: registers.set_bc(registers.bc() + 1); break;
      case 0x13: registers.set_de(registers.de() + 1); break;
      case 0x23: registers.set_hl(registers.hl() + 1); break;
      case 0x33: registers.set_sp(registers.sp() + 1); break;
      case 0x0b: registers.set_bc(registers.bc() - 1); break;
      case 0x1b: registers.set_de(registers.de() - 1); break;
      case 0x2b: registers.set_hl(registers.hl() - 1); break;
      case 0x3b: registers.set_sp(registers.sp() - 1); break;

      // ---- 16-bit add into HL ----
      case 0x09: add_to_hl(registers.bc()); break;
      case 0x19: add_to_hl(registers.de()); break;
      case 0x29: add_to_hl(registers.hl()); break;
      case 0x39: add_to_hl(registers.sp()); break;
      case 0xe8: registers.set_sp(sp_plus_signed_byte()); break;

      // ---- math on A using a number that follows the opcode ----
      case 0xc6: do_math_on_a(MATH_ADD, fetch_byte()); break;
      case 0xce: do_math_on_a(MATH_ADD_WITH_CARRY, fetch_byte()); break;
      case 0xd6: do_math_on_a(MATH_SUBTRACT, fetch_byte()); break;
      case 0xde: do_math_on_a(MATH_SUBTRACT_WITH_CARRY, fetch_byte()); break;
      case 0xe6: do_math_on_a(MATH_AND, fetch_byte()); break;
      case 0xee: do_math_on_a(MATH_XOR, fetch_byte()); break;
      case 0xf6: do_math_on_a(MATH_OR, fetch_byte()); break;
      case 0xfe: do_math_on_a(MATH_COMPARE, fetch_byte()); break;

      // ---- rotate A (like the 0xCB versions, but Z is always 0) ----
      case 0x07: rotate_a(rotate_left_with_wrap(registers.a())); break;
      case 0x0f: rotate_a(rotate_right_with_wrap(registers.a())); break;
      case 0x17: rotate_a(rotate_left_through_carry(registers.a())); break;
      case 0x1f: rotate_a(rotate_right_through_carry(registers.a())); break;

      // ---- odd one-off instructions ----
      case 0x27: decimal_adjust(); break;
      case 0x2f:  // CPL: flip every bit of A
        registers.set_a(~registers.a());
        registers.set_subtract_flag(true);
        registers.set_half_carry_flag(true);
        break;
      case 0x37:  // SCF: set carry
        registers.set_subtract_flag(false);
        registers.set_half_carry_flag(false);
        registers.set_carry_flag(true);
        break;
      case 0x3f:  // CCF: flip carry
        registers.set_subtract_flag(false);
        registers.set_half_carry_flag(false);
        registers.set_carry_flag(!registers.carry_flag());
        break;
      case 0x10: fetch_byte(); break;  // STOP: skip its extra byte, do nothing
      case 0xf3: interrupts_enabled_ = false; break;  // DI
      case 0xfb: interrupts_enabled_ = true; break;   // EI

      // ---- jumps ----
      case 0xc3: registers.set_pc(fetch_word()); break;
      case 0xe9: registers.set_pc(registers.hl()); break;
      case 0x18: jump_relative_if(true); break;
      case 0x20: jump_relative_if(!registers.zero_flag()); break;
      case 0x28: jump_relative_if(registers.zero_flag()); break;
      case 0x30: jump_relative_if(!registers.carry_flag()); break;
      case 0x38: jump_relative_if(registers.carry_flag()); break;
      case 0xc2: jump_if(!registers.zero_flag()); break;
      case 0xca: jump_if(registers.zero_flag()); break;
      case 0xd2: jump_if(!registers.carry_flag()); break;
      case 0xda: jump_if(registers.carry_flag()); break;

      // ---- calls and returns ----
      case 0xcd: call_if(true); break;
      case 0xc4: call_if(!registers.zero_flag()); break;
      case 0xcc: call_if(registers.zero_flag()); break;
      case 0xd4: call_if(!registers.carry_flag()); break;
      case 0xdc: call_if(registers.carry_flag()); break;
      case 0xc9: registers.set_pc(pop_word()); break;
      case 0xd9:  // RETI: return and turn interrupts on
        registers.set_pc(pop_word());
        interrupts_enabled_ = true;
        break;
      case 0xc0: return_if(!registers.zero_flag()); break;
      case 0xc8: return_if(registers.zero_flag()); break;
      case 0xd0: return_if(!registers.carry_flag()); break;
      case 0xd8: return_if(registers.carry_flag()); break;

      // RST: call one of 8 fixed addresses
      case 0xc7: restart(0x00); break;
      case 0xcf: restart(0x08); break;
      case 0xd7: restart(0x10); break;
      case 0xdf: restart(0x18); break;
      case 0xe7: restart(0x20); break;
      case 0xef: restart(0x28); break;
      case 0xf7: restart(0x30); break;
      case 0xff: restart(0x38); break;

      // ---- the stack ----
      case 0xc5: push_word(registers.bc()); break;
      case 0xd5: push_word(registers.de()); break;
      case 0xe5: push_word(registers.hl()); break;
      case 0xf5: push_word(registers.af()); break;
      case 0xc1: registers.set_bc(pop_word()); break;
      case 0xd1: registers.set_de(pop_word()); break;
      case 0xe1: registers.set_hl(pop_word()); break;
      case 0xf1: registers.set_af(pop_word()); break;

      default:
        throw runtime_error("unknown opcode");
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
  int fetch_signed_byte() {
    int value = fetch_byte();
    if (value > 127) {
      value = value - 256;
    }
    return value;
  }

  // ---- registers by number ----
  int get_register(int number) {
    switch (number) {
      case REG_B: return registers.b();
      case REG_C: return registers.c();
      case REG_D: return registers.d();
      case REG_E: return registers.e();
      case REG_H: return registers.h();
      case REG_L: return registers.l();
      case REG_HL_MEMORY: return memory_.read(registers.hl());
      default: return registers.a();
    }
  }

  void set_register(int number, int value) {
    switch (number) {
      case REG_B: registers.set_b(value); break;
      case REG_C: registers.set_c(value); break;
      case REG_D: registers.set_d(value); break;
      case REG_E: registers.set_e(value); break;
      case REG_H: registers.set_h(value); break;
      case REG_L: registers.set_l(value); break;
      case REG_HL_MEMORY: memory_.write(registers.hl(), value); break;
      default: registers.set_a(value); break;
    }
  }

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

  // ---- jumps, calls, returns ----
  // The jump address (or offset) is always read, even when the jump is not
  // taken, so PC ends up past the instruction either way.
  void jump_if(bool condition) {
    int address = fetch_word();
    if (condition) {
      registers.set_pc(address);
    }
  }

  void jump_relative_if(bool condition) {
    int offset = fetch_signed_byte();
    if (condition) {
      registers.set_pc(registers.pc() + offset);
    }
  }

  void call_if(bool condition) {
    int address = fetch_word();
    if (condition) {
      push_word(registers.pc());
      registers.set_pc(address);
    }
  }

  void return_if(bool condition) {
    if (condition) {
      registers.set_pc(pop_word());
    }
  }

  void restart(int address) {
    push_word(registers.pc());
    registers.set_pc(address);
  }

  // ---- math on A ----
  void do_math_on_a(int operation, int value) {
    int a = registers.a();
    int carry = registers.carry_flag() ? 1 : 0;

    switch (operation) {
      case MATH_ADD: registers.set_a(add(a, value, 0)); break;
      case MATH_ADD_WITH_CARRY: registers.set_a(add(a, value, carry)); break;
      case MATH_SUBTRACT: registers.set_a(subtract(a, value, 0)); break;
      case MATH_SUBTRACT_WITH_CARRY: registers.set_a(subtract(a, value, carry)); break;
      case MATH_AND: registers.set_a(logic(a & value, true)); break;
      case MATH_XOR: registers.set_a(logic(a ^ value, false)); break;
      case MATH_OR: registers.set_a(logic(a | value, false)); break;
      default: subtract(a, value, 0); break;  // COMPARE: flags only, A unchanged
    }
  }

  // Each function below returns the result and sets all four flags.
  int add(int a, int value, int carry) {
    int result = a + value + carry;
    registers.set_zero_flag((result & 0xff) == 0);
    registers.set_subtract_flag(false);
    registers.set_half_carry_flag(((a & 0xf) + (value & 0xf) + carry) > 0xf);
    registers.set_carry_flag(result > 0xff);
    return result;
  }

  int subtract(int a, int value, int carry) {
    int result = a - value - carry;
    registers.set_zero_flag((result & 0xff) == 0);
    registers.set_subtract_flag(true);
    registers.set_half_carry_flag(((a & 0xf) - (value & 0xf) - carry) < 0);
    registers.set_carry_flag(result < 0);
    return result;
  }

  // AND sets half-carry, OR and XOR clear it. All three clear carry.
  int logic(int result, bool is_and) {
    registers.set_zero_flag((result & 0xff) == 0);
    registers.set_subtract_flag(false);
    registers.set_half_carry_flag(is_and);
    registers.set_carry_flag(false);
    return result;
  }

  // INC and DEC leave the carry flag as it was.
  int add_one(int value) {
    int result = (value + 1) & 0xff;
    registers.set_zero_flag(result == 0);
    registers.set_subtract_flag(false);
    registers.set_half_carry_flag((value & 0xf) == 0xf);
    return result;
  }

  int subtract_one(int value) {
    int result = (value - 1) & 0xff;
    registers.set_zero_flag(result == 0);
    registers.set_subtract_flag(true);
    registers.set_half_carry_flag((value & 0xf) == 0);
    return result;
  }

  // 16-bit add leaves the zero flag as it was.
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
  // Each one moves the bits of a byte and puts the bit that fell off the end
  // into the carry flag. They return the new byte and set all four flags.
  int finish_shift(int result, int carry_out) {
    result = result & 0xff;
    registers.set_zero_flag(result == 0);
    registers.set_subtract_flag(false);
    registers.set_half_carry_flag(false);
    registers.set_carry_flag(carry_out == 1);
    return result;
  }

  int rotate_left_with_wrap(int value) {
    int top_bit = value >> 7;
    return finish_shift((value << 1) | top_bit, top_bit);
  }

  int rotate_right_with_wrap(int value) {
    int bottom_bit = value & 1;
    return finish_shift((value >> 1) | (bottom_bit << 7), bottom_bit);
  }

  int rotate_left_through_carry(int value) {
    int old_carry = registers.carry_flag() ? 1 : 0;
    return finish_shift((value << 1) | old_carry, value >> 7);
  }

  int rotate_right_through_carry(int value) {
    int old_carry = registers.carry_flag() ? 1 : 0;
    return finish_shift((value >> 1) | (old_carry << 7), value & 1);
  }

  int shift_left(int value) { return finish_shift(value << 1, value >> 7); }

  // Shift right but keep the top bit as it was (keeps the sign).
  int shift_right_keep_sign(int value) {
    return finish_shift((value >> 1) | (value & 0x80), value & 1);
  }

  int shift_right(int value) { return finish_shift(value >> 1, value & 1); }

  int swap_halves(int value) { return finish_shift((value << 4) | (value >> 4), 0); }

  // RLCA, RRCA, RLA, RRA: the same as the 0xCB versions, but Z is always 0.
  void rotate_a(int result) {
    registers.set_a(result);
    registers.set_zero_flag(false);
  }

  // ---- the 0xCB opcodes ----
  // 0xCB is a prefix: the byte after it picks the real instruction.
  //   0x00-0x3F: rotate or shift   (8 kinds, each in a row of 8 registers)
  //   0x40-0x7F: BIT  (test a bit)
  //   0x80-0xBF: RES  (clear a bit)
  //   0xC0-0xFF: SET  (turn a bit on)
  // Inside each range: bit number (or kind) = (opcode % 64) / 8, register = opcode % 8
  void run_cb_instruction() {
    int opcode = fetch_byte();
    int register_number = opcode % 8;
    int kind_or_bit = (opcode % 64) / 8;
    int value = get_register(register_number);

    if (opcode < 0x40) {
      set_register(register_number, rotate_or_shift(kind_or_bit, value));
    } else if (opcode < 0x80) {  // BIT
      registers.set_zero_flag(((value >> kind_or_bit) & 1) == 0);
      registers.set_subtract_flag(false);
      registers.set_half_carry_flag(true);
    } else if (opcode < 0xc0) {  // RES
      set_register(register_number, value & ~(1 << kind_or_bit));
    } else {  // SET
      set_register(register_number, value | (1 << kind_or_bit));
    }
  }

  int rotate_or_shift(int kind, int value) {
    switch (kind) {
      case 0: return rotate_left_with_wrap(value);
      case 1: return rotate_right_with_wrap(value);
      case 2: return rotate_left_through_carry(value);
      case 3: return rotate_right_through_carry(value);
      case 4: return shift_left(value);
      case 5: return shift_right_keep_sign(value);
      case 6: return swap_halves(value);
      default: return shift_right(value);
    }
  }
};
