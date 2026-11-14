#pragma once

#include <cstdint>
using namespace std;

// The CPU's registers: A B C D E H L (8-bit), F (flags), PC and SP (16-bit).
// BC, DE, HL and AF are just two 8-bit registers read as one 16-bit number.
//
// Every setter cuts the value down to the register's size, so a register can
// never hold a number that is too big.

// Each flag is one bit inside the F register.
const int FLAG_Z = 0x80;  // zero: the result was 0
const int FLAG_N = 0x40;  // subtract: the last operation was a subtraction
const int FLAG_H = 0x20;  // half-carry: carry out of bit 3
const int FLAG_C = 0x10;  // carry: went past 255 or below 0

class Registers {
 public:
  // ---- 8-bit registers ----
  // A register holds 0 to 255. "value & 0xff" keeps only the lowest 8 bits
  // and throws the rest away, so set_a(300) stores 44 and set_a(-1) stores 255.
  // The other 6 registers below do exactly the same thing.
  uint8_t a() { return a_; }
  void set_a(int value) { a_ = value & 0xff; }

  uint8_t b() { return b_; }
  void set_b(int value) { b_ = value & 0xff; }

  uint8_t c() { return c_; }
  void set_c(int value) { c_ = value & 0xff; }

  uint8_t d() { return d_; }
  void set_d(int value) { d_ = value & 0xff; }

  uint8_t e() { return e_; }
  void set_e(int value) { e_ = value & 0xff; }

  uint8_t h() { return h_; }
  void set_h(int value) { h_ = value & 0xff; }

  uint8_t l() { return l_; }
  void set_l(int value) { l_ = value & 0xff; }

  // F only has 4 real bits (the top 4), so the bottom 4 are always 0.
  // 0xf0 is 1111 0000 in binary: "& 0xf0" keeps the top 4 bits, clears the rest.
  uint8_t f() { return f_; }
  void set_f(int value) { f_ = value & 0xf0; }

  // ---- 16-bit registers ----
  // PC = the address of the next instruction to run.
  // SP = the address of the top of the stack.
  // Addresses go from 0 to 65535, so we keep the lowest 16 bits (0xffff).
  uint16_t pc() { return pc_; }
  void set_pc(int value) { pc_ = value & 0xffff; }

  uint16_t sp() { return sp_; }
  void set_sp(int value) { sp_ = value & 0xffff; }

  // ---- 16-bit pairs: high register first, low register second ----
  // A pair is NOT extra storage. AF is just A and F next to each other.
  // Example: A = 0x12 and F = 0x30 make AF = 0x1230.
  //
  // Reading (joining two bytes into one number):
  //   a_ << 8   slides A up 8 places:      0x12 -> 0x1200
  //   | f_      puts F in the empty space: 0x1200 | 0x30 -> 0x1230
  uint16_t af() { return (a_ << 8) | f_; }

  // Writing (splitting one number into two bytes), e.g. value = 0x1230:
  //   value >> 8   slides value down 8 places, leaving the left half: 0x12 -> A
  //   value        the right half (0x30) goes to F. The setter drops the left
  //                half by itself because of its "& 0xff" / "& 0xf0".
  // BC, DE and HL below work the same way.
  void set_af(int value) {
    set_a(value >> 8);
    set_f(value);
  }

  uint16_t bc() { return (b_ << 8) | c_; }
  void set_bc(int value) {
    set_b(value >> 8);
    set_c(value);
  }

  uint16_t de() { return (d_ << 8) | e_; }
  void set_de(int value) {
    set_d(value >> 8);
    set_e(value);
  }

  uint16_t hl() { return (h_ << 8) | l_; }
  void set_hl(int value) {
    set_h(value >> 8);
    set_l(value);
  }

  // ---- flags ----
  // Named in full because `h` and `c` are already register names.
  //
  // Each flag is one bit inside F. Example: F = 1010 0000 means Z is on, N is
  // off, H is on, C is off.
  //
  // Reading: "f_ & FLAG_Z" keeps only the Z bit. If anything is left (not 0),
  // the flag is on.
  bool zero_flag() { return (f_ & FLAG_Z) != 0; }

  // Writing:
  //   turn on:  f_ | FLAG_Z    "|" switches that bit on, other bits unchanged
  //   turn off: f_ & ~FLAG_Z   "~" flips the mask (0111 1111), so "&" clears
  //                            only that bit, other bits unchanged
  // The other 3 flags below are the same, just with their own bit.
  void set_zero_flag(bool on) {
    if (on) {
      set_f(f_ | FLAG_Z);
    } else {
      set_f(f_ & ~FLAG_Z);
    }
  }

  bool subtract_flag() { return (f_ & FLAG_N) != 0; }
  void set_subtract_flag(bool on) {
    if (on) {
      set_f(f_ | FLAG_N);
    } else {
      set_f(f_ & ~FLAG_N);
    }
  }

  bool half_carry_flag() { return (f_ & FLAG_H) != 0; }
  void set_half_carry_flag(bool on) {
    if (on) {
      set_f(f_ | FLAG_H);
    } else {
      set_f(f_ & ~FLAG_H);
    }
  }

  bool carry_flag() { return (f_ & FLAG_C) != 0; }
  void set_carry_flag(bool on) {
    if (on) {
      set_f(f_ | FLAG_C);
    } else {
      set_f(f_ & ~FLAG_C);
    }
  }

  // The values a real Game Boy's startup program leaves behind.
  // We skip that program and start from here. PC = 0x0100 is where games begin.
  void set_post_boot_values() {
    set_af(0x01b0);
    set_bc(0x0013);
    set_de(0x00d8);
    set_hl(0x014d);
    set_sp(0xfffe);
    set_pc(0x0100);
  }

 private:
  uint8_t a_ = 0;
  uint8_t b_ = 0;
  uint8_t c_ = 0;
  uint8_t d_ = 0;
  uint8_t e_ = 0;
  uint8_t h_ = 0;
  uint8_t l_ = 0;
  uint8_t f_ = 0;
  uint16_t pc_ = 0;
  uint16_t sp_ = 0;
};
