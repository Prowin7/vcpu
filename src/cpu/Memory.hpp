#pragma once

#include <cstdint>
#include <string>
using namespace std;

// The Game Boy's 64 KB of memory: one array, addresses 0x0000 to 0xFFFF.
// The game file (ROM) is written into the first 32 KB, one byte at a time.
// Only games of 32 KB or smaller work for now.
class Memory {
 public:
  Memory() {
    for (int address = 0; address < 0x10000; address++) {
      bytes_[address] = 0;
    }
  }

  uint8_t read(int address) {
    address = fix_address(address);
    return bytes_[address];
  }

  void write(int address, int value) {
    address = fix_address(address);
    bytes_[address] = value & 0xff;

    // Test programs print text one byte at a time: put the byte in 0xFF01,
    // then write 0x81 to 0xFF02.
    if (address == 0xff02 && bytes_[address] == 0x81) {
      char letter = bytes_[0xff01];
      serial_output += letter;
    }
  }

  // Everything the program has printed so far.
  string serial_output;

 private:
  // Keep the address between 0 and 65535. Also, 0xE000-0xFDFF is a mirror of
  // 0xC000-0xDDFF (same bytes, second address), so move it down by 0x2000.
  int fix_address(int address) {
    address = address & 0xffff;
    if (address >= 0xe000 && address <= 0xfdff) {
      address = address - 0x2000;
    }
    return address;
  }

  uint8_t bytes_[0x10000];
};
