#include <iostream>
#include <string>
#include <vector>

#include "../src/cpu/Cpu.hpp"

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

// Put a tiny program at 0x0100 (where games start) and run `steps` instructions.
void run_program(Memory& memory, Cpu& cpu, vector<int> program, int steps) {
  for (size_t i = 0; i < program.size(); i++) {
    memory.write(0x0100 + i, program[i]);
  }
  for (int i = 0; i < steps; i++) {
    cpu.step();
  }
}

int main() {
  // ---- Memory ----
  {
    Memory memory;
    memory.write(0xc000, 0x42);
    check("echo region mirrors work RAM", memory.read(0xe000), 0x42);
    memory.write(0xe001, 0x99);
    check("write to echo lands in work RAM", memory.read(0xc001), 0x99);
    memory.write(0xc002, 0x1ff);
    check("memory keeps low byte", memory.read(0xc002), 0xff);

    memory.write(0xff01, 'H');
    memory.write(0xff02, 0x81);
    check("serial captures a byte", memory.serial_output == "H", 1);
  }

  // ---- Loads ----
  {
    Memory memory;
    Cpu cpu(memory);
    run_program(memory, cpu, {0x3e, 0x12, 0x06, 0x34}, 2);  // LD A,0x12; LD B,0x34
    check("LD A,n", cpu.registers.a(), 0x12);
    check("LD B,n", cpu.registers.b(), 0x34);
    check("PC moves past both", cpu.registers.pc(), 0x0104);
  }

  // ---- Math and flags ----
  {
    Memory memory;
    Cpu cpu(memory);
    run_program(memory, cpu, {0x3e, 0xff, 0xc6, 0x01}, 2);  // LD A,0xFF; ADD A,1
    check("ADD wraps to 0", cpu.registers.a(), 0);
    check("ADD sets zero flag", cpu.registers.zero_flag(), 1);
    check("ADD sets carry flag", cpu.registers.carry_flag(), 1);
    check("ADD sets half-carry flag", cpu.registers.half_carry_flag(), 1);
  }
  {
    Memory memory;
    Cpu cpu(memory);
    run_program(memory, cpu, {0x3e, 0x03, 0xfe, 0x05}, 2);  // LD A,3; CP 5
    check("CP leaves A alone", cpu.registers.a(), 3);
    check("CP sets carry when A < n", cpu.registers.carry_flag(), 1);
    check("CP sets subtract flag", cpu.registers.subtract_flag(), 1);
  }
  {
    Memory memory;
    Cpu cpu(memory);
    run_program(memory, cpu, {0x3e, 0x45, 0xc6, 0x38, 0x27}, 3);  // 45 + 38, then DAA
    check("DAA gives decimal 83", cpu.registers.a(), 0x83);
  }

  // ---- Jumps and the stack ----
  {
    Memory memory;
    Cpu cpu(memory);
    // CALL 0x0200; at 0x0200: LD A,7; RET
    memory.write(0x0200, 0x3e);
    memory.write(0x0201, 0x07);
    memory.write(0x0202, 0xc9);
    run_program(memory, cpu, {0xcd, 0x00, 0x02}, 3);
    check("CALL then RET runs the function", cpu.registers.a(), 7);
    check("RET comes back after the CALL", cpu.registers.pc(), 0x0103);
    check("SP is back where it started", cpu.registers.sp(), 0xfffe);
  }
  {
    Memory memory;
    Cpu cpu(memory);
    run_program(memory, cpu, {0xaf, 0x20, 0x05}, 2);  // XOR A (Z=1); JR NZ,+5
    check("JR NZ not taken when Z is set", cpu.registers.pc(), 0x0103);
  }

  // ---- CB prefix ----
  {
    Memory memory;
    Cpu cpu(memory);
    run_program(memory, cpu, {0x3e, 0x80, 0xcb, 0x37}, 2);  // LD A,0x80; SWAP A
    check("SWAP swaps nibbles", cpu.registers.a(), 0x08);
    memory.write(0x0104, 0xcb);  // BIT 0,A goes right after SWAP
    memory.write(0x0105, 0x47);
    cpu.step();
    check("BIT 0 on 0x08 sets zero flag", cpu.registers.zero_flag(), 1);
  }

  if (failures > 0) {
    cout << failures << " test(s) failed" << endl;
    return 1;
  }
  cout << "all tests passed" << endl;
  return 0;
}
