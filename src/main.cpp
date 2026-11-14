#include <cstdlib>
#include <fstream>
#include <iostream>

#include "cpu/Cpu.hpp"

// Usage: vcpu game.gb [max_instructions]
int main(int argc, char** argv) {
  if (argc < 2) {
    cout << "usage: vcpu game.gb [max_instructions]" << endl;
    return 1;
  }

  // Read the whole game file into one array (32 KB is the most we use).
  static uint8_t rom[0x8000];
  ifstream file(argv[1], ios::binary);
  if (!file) {
    cout << "cannot open " << argv[1] << endl;
    return 1;
  }
  file.read(reinterpret_cast<char*>(rom), sizeof(rom));
  int rom_size = file.gcount();

  int max_instructions = 10000000;
  if (argc >= 3) {
    max_instructions = atoi(argv[2]);
  }

  Memory memory;
  memory.load_rom(rom, rom_size);
  Cpu cpu(memory);

  try {
    for (int i = 0; i < max_instructions; i++) {
      cpu.step();
    }
  } catch (const exception& error) {
    cout << "stopped: " << error.what() << " at PC=" << hex << cpu.registers.pc() << endl;
  }

  cout << memory.serial_output << endl;
  cout << hex << "A=" << +cpu.registers.a() << " F=" << +cpu.registers.f()
       << " BC=" << cpu.registers.bc() << " DE=" << cpu.registers.de()
       << " HL=" << cpu.registers.hl() << " SP=" << cpu.registers.sp()
       << " PC=" << cpu.registers.pc() << endl;
  return 0;
}
