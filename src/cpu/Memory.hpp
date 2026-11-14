#pragma once

#include <cstdint>
#include <string>
#include <vector>
using namespace std;

// The Game Boy's 64 KB of memory: one array, addresses 0x0000 to 0xFFFF.
// The game file (ROM) is copied into the first 32 KB. Only games with no
// bank switching (32 KB or smaller) work for now.
class Memory {
 public:
  Memory() : bytes_(0x10000, 0) {}

  uint8_t read(int address) { return bytes_[real_address(address)]; }

  void write(int address, int value) {
    address = real_address(address);
    bytes_[address] = value & 0xff;

    // Test programs print text by sending one byte at a time through the
    // serial port: put the byte in 0xFF01, then write 0x81 to 0xFF02.
    if (address == 0xff02 && (value & 0xff) == 0x81) {
      serial_output += static_cast<char>(bytes_[0xff01]);
    }
  }

  // Copy the game file into the start of memory.
  void load_rom(const vector<uint8_t>& rom) {
    for (size_t i = 0; i < rom.size() && i < 0x8000; i++) {
      bytes_[i] = rom[i];
    }
  }

  // Everything the program has printed over the serial port so far.
  string serial_output;

 private:
  // 0xE000-0xFDFF is a mirror of 0xC000-0xDDFF: same bytes, second address.
  int real_address(int address) {
    address = address & 0xffff;
    if (address >= 0xe000 && address <= 0xfdff) {
      return address - 0x2000;
    }
    return address;
  }

  vector<uint8_t> bytes_;
};
