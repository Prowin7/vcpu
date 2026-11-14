#include <fstream>
#include <iostream>

#include "cpu/Cpu.hpp"

// Change these two lines to run a different game or run for longer.
const char* ROM_PATH = "game.gb";
const int MAX_INSTRUCTIONS = 10000000;

int main() {
  ifstream file(ROM_PATH, ios::binary);
  if (!file) {
    cout << "cannot open " << ROM_PATH << endl;
    return 1;
  }

  // Put the game file into memory, one byte at a time (32 KB at most).
  Memory memory;
  char byte;
  int address = 0;
  while (address < 0x8000 && file.get(byte)) {
    memory.write(address, byte);
    address++;
  }

  Cpu cpu(memory);

  try {
    for (int i = 0; i < MAX_INSTRUCTIONS; i++) {
      cpu.step();
    }
  } catch (const exception& error) {
    cout << "stopped: " << error.what() << " at PC=" << hex << cpu.registers.pc() << endl;
  }

  cout << memory.serial_output << endl;

  // Copy into plain ints so cout prints numbers, not characters.
  int a = cpu.registers.a();
  int f = cpu.registers.f();
  cout << hex << "A=" << a << " F=" << f << " BC=" << cpu.registers.bc()
       << " DE=" << cpu.registers.de() << " HL=" << cpu.registers.hl()
       << " SP=" << cpu.registers.sp() << " PC=" << cpu.registers.pc() << endl;
  return 0;
}
