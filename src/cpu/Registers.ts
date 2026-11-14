/**
 * The LR35902 register file.
 *
 * The CPU has seven 8-bit registers (A B C D E H L), one flags register (F),
 * and two 16-bit registers (PC, SP). The 16-bit pairs AF, BC, DE and HL are
 * not extra storage: they are two 8-bit registers read as one number.
 *
 * Every write is masked here, at the edge, so no code inside the CPU has to
 * re-check that it stored a byte-sized value. That is the whole job of this
 * file: it is the only place where "a register is 8 bits" is enforced.
 */

/** Zero flag: result was 0. Drives every conditional jump. */
export const FLAG_Z = 0x80;
/** Subtract flag: the last arithmetic operation was a subtraction. */
export const FLAG_N = 0x40;
/** Half-carry: carry out of bit 3 (or borrow out of bit 4). Only DAA reads it. */
export const FLAG_H = 0x20;
/** Carry: carry out of bit 7, borrow below 0, or the bit a shift pushed out. */
export const FLAG_C = 0x10;

const BYTE_MASK = 0xff;
const WORD_MASK = 0xffff;
const FLAG_MASK = 0xf0;

export class Registers {
  private _a = 0;
  private _b = 0;
  private _c = 0;
  private _d = 0;
  private _e = 0;
  private _h = 0;
  private _l = 0;
  private _f = 0;
  private _pc = 0;
  private _sp = 0;

  get a(): number {
    return this._a;
  }
  set a(value: number) {
    this._a = value & BYTE_MASK;
  }

  get b(): number {
    return this._b;
  }
  set b(value: number) {
    this._b = value & BYTE_MASK;
  }

  get c(): number {
    return this._c;
  }
  set c(value: number) {
    this._c = value & BYTE_MASK;
  }

  get d(): number {
    return this._d;
  }
  set d(value: number) {
    this._d = value & BYTE_MASK;
  }

  get e(): number {
    return this._e;
  }
  set e(value: number) {
    this._e = value & BYTE_MASK;
  }

  get h(): number {
    return this._h;
  }
  set h(value: number) {
    this._h = value & BYTE_MASK;
  }

  get l(): number {
    return this._l;
  }
  set l(value: number) {
    this._l = value & BYTE_MASK;
  }

  /**
   * F is special: hardware only has four flag bits, so bits 3..0 do not exist.
   * Reads therefore always return a value with the low nibble clear.
   */
  get f(): number {
    return this._f;
  }
  set f(value: number) {
    this._f = value & FLAG_MASK;
  }

  get pc(): number {
    return this._pc;
  }
  set pc(value: number) {
    this._pc = value & WORD_MASK;
  }

  get sp(): number {
    return this._sp;
  }
  set sp(value: number) {
    this._sp = value & WORD_MASK;
  }

  /** AF packs A as the high byte and F as the low byte. */
  get af(): number {
    return (this._a << 8) | this._f;
  }
  set af(value: number) {
    this.a = value >> 8;
    this.f = value;
  }

  get bc(): number {
    return (this._b << 8) | this._c;
  }
  set bc(value: number) {
    this.b = value >> 8;
    this.c = value;
  }

  get de(): number {
    return (this._d << 8) | this._e;
  }
  set de(value: number) {
    this.d = value >> 8;
    this.e = value;
  }

  get hl(): number {
    return (this._h << 8) | this._l;
  }
  set hl(value: number) {
    this.h = value >> 8;
    this.l = value;
  }

  // Flag accessors are named in full on purpose: `h` and `c` are already the
  // H and C registers, and hardware genuinely uses both names for both things.
  get zeroFlag(): boolean {
    return (this._f & FLAG_Z) !== 0;
  }
  set zeroFlag(on: boolean) {
    this.f = on ? this._f | FLAG_Z : this._f & ~FLAG_Z;
  }

  get subtractFlag(): boolean {
    return (this._f & FLAG_N) !== 0;
  }
  set subtractFlag(on: boolean) {
    this.f = on ? this._f | FLAG_N : this._f & ~FLAG_N;
  }

  get halfCarryFlag(): boolean {
    return (this._f & FLAG_H) !== 0;
  }
  set halfCarryFlag(on: boolean) {
    this.f = on ? this._f | FLAG_H : this._f & ~FLAG_H;
  }

  get carryFlag(): boolean {
    return (this._f & FLAG_C) !== 0;
  }
  set carryFlag(on: boolean) {
    this.f = on ? this._f | FLAG_C : this._f & ~FLAG_C;
  }

  /**
   * The state the boot ROM would leave behind, so we can skip it.
   * PC lands at 0x0100, the cartridge entry point.
   */
  static postBoot(): Registers {
    const r = new Registers();
    r.af = 0x01b0;
    r.bc = 0x0013;
    r.de = 0x00d8;
    r.hl = 0x014d;
    r.sp = 0xfffe;
    r.pc = 0x0100;
    return r;
  }
}
