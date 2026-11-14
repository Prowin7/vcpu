#include <fstream>
#include <iostream>

#include "cpu/Cpu.hpp"

// Change these two lines to run a different game or run for longer.
const char* ROM_PATH = "game.gb";
const int MAX_INSTRUCTIONS = 10000000;

int main() {
  // Read the game file into one array (32 KB is the most we use).
  static uint8_t rom[0x8000];
  ifstream file(ROM_PATH, ios::binary);
  if (!file) {
    cout << "cannot open " << ROM_PATH << endl;
    return 1;
  }
  file.read(reinterpret_cast<char*>(rom), sizeof(rom));
  int rom_size = file.gcount();

  Memory memory;
  memory.load_rom(rom, rom_size);
  Cpu cpu(memory);

  try {
    for (int i = 0; i < MAX_INSTRUCTIONS; i++) {
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
