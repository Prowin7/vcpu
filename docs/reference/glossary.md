# Glossary

Plain meanings for every technical term this project uses, so no page is
unreadable on its own.

| Term | Meaning |
| :--- | :--- |
| **Address** | A number that names one byte of memory, like a house number. |
| **ADR** | Architecture Decision Record: a short file recording one decision, its context and its consequences. |
| **Byte** | One number, 0–255. |
| **Emulator** | Software that pretends to be a machine. |
| **Flag** | A single bit recording what the last calculation did. |
| **Frame** | One complete picture. The screen redraws about 60 times a second. |
| **Half-carry** | A carry out of the 4th bit. Exists only so decimal arithmetic works. |
| **Hex (`0xFF00`)** | How memory addresses are written. `0xFF00` is 65280. |
| **Instruction** | One thing the CPU can do, like "add these two numbers". |
| **Interrupt** | The hardware tapping the CPU on the shoulder: "deal with me now". |
| **Mask** | A number used to keep or clear specific bits, e.g. `0xff` keeps the bottom 8 bits. |
| **MMU / memory bus** | The part that decides what a given address means. |
| **Register** | A tiny slot inside the CPU holding one number, 0–255. |
| **ROM / game file** | The game itself, a single `.gb` file. "ROM" means read-only memory, the chip games came on. |
| **Snapshot / save state** | A copy of the entire machine at one instant, so you can return to it. |
| **Sprite** | A small picture that can move anywhere on screen: a character, a bullet. |
| **Tile** | A small 8×8 picture. The screen is built from a grid of them. |
| **T-state** | One tick of the CPU clock. The Game Boy does 4,194,304 per second. |
| **16-bit / word** | Two bytes read as one number, 0–65535. |
| **V-Blank** | The short gap after each frame, when the screen is idle and safe to redraw. |
