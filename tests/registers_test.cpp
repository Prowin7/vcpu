#include <iostream>
#include <string>

#include "../src/cpu/Registers.hpp"

int failures = 0;

void check(string name, int actual, int expected) {
  if (actual == expected) {
    cout << "pass: " << name << endl;
  } else {
    cout << "FAIL: " << name << " (got " << actual << ", expected " << expected
         << ")" << endl;
    failures++;
  }
}

int main() {
  Registers r;

  // A register keeps only its low 8 bits.
  r.set_a(0x1234);
  check("a keeps low byte", r.a(), 0x34);
  r.set_l(-1);
  check("l keeps low byte of -1", r.l(), 0xff);

  // Pairs split into two registers.
  r.set_bc(0x1234);
  check("bc -> b", r.b(), 0x12);
  check("bc -> c", r.c(), 0x34);

  // Two registers join into a pair.
  r.set_h(0xab);
  r.set_l(0xcd);
  check("h and l -> hl", r.hl(), 0xabcd);

  // A pair is 16 bits, so extra bits are dropped.
  r.set_de(0x12345);
  check("de wraps to 16 bits", r.de(), 0x2345);

  // F can't hold anything in its bottom 4 bits.
  r.set_f(0xff);
  check("f bottom nibble is 0", r.f(), 0xf0);
  r.set_f(0x0f);
  check("f of 0x0f is 0", r.f(), 0x00);

  // ...even when written through AF.
  r.set_af(0x00ff);
  check("af -> a", r.a(), 0x00);
  check("af -> f", r.f(), 0xf0);

  // Changing one flag leaves the others alone.
  r.set_f(FLAG_Z | FLAG_C);
  r.set_subtract_flag(true);
  check("subtract flag on", r.f(), FLAG_Z | FLAG_N | FLAG_C);
  r.set_subtract_flag(false);
  check("subtract flag off", r.f(), FLAG_Z | FLAG_C);
  check("zero flag still on", r.zero_flag(), true);
  check("half carry flag still off", r.half_carry_flag(), false);

  // Values after the startup program.
  Registers boot;
  boot.set_post_boot_values();
  check("boot af", boot.af(), 0x01b0);
  check("boot bc", boot.bc(), 0x0013);
  check("boot de", boot.de(), 0x00d8);
  check("boot hl", boot.hl(), 0x014d);
  check("boot sp", boot.sp(), 0xfffe);
  check("boot pc", boot.pc(), 0x0100);

  if (failures > 0) {
    cout << failures << " failed" << endl;
    return 1;
  }
  cout << "all passed" << endl;
  return 0;
}
