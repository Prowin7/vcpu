#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <vector>

#include "cpu/Cpu.hpp"

// Usage: vcpu game.gb [max_instructions]
int main(int argc, char** argv) {
  if (argc < 2) {
    cout << "usage: vcpu game.gb [max_instructions]" << endl;
    return 1;
  }

  ifstream file(argv[1], ios::binary);
  if (!file) {
    cout << "cannot open " << argv[1] << endl;
    return 1;
  }
  vector<uint8_t> rom((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());

  long max_instructions = 10000000;
  if (argc >= 3) {
    max_instructions = atol(argv[2]);
  }

  Memory memory;
  memory.load_rom(rom);
  Cpu cpu(memory);

  try {
    for (long i = 0; i < max_instructions; i++) {
      cpu.step();
    }
  } catch (const exception& error) {
    cout << "stopped: " << error.what() << " at PC=" << hex << cpu.registers.pc() << dec << endl;
  }

  cout << memory.serial_output << endl;
  printf("A=%02X F=%02X BC=%04X DE=%04X HL=%04X SP=%04X PC=%04X\n", cpu.registers.a(),
         cpu.registers.f(), cpu.registers.bc(), cpu.registers.de(), cpu.registers.hl(),
         cpu.registers.sp(), cpu.registers.pc());
  return 0;
}
