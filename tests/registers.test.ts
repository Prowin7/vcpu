import { describe, expect, it } from "vitest";
import { FLAG_C, FLAG_H, FLAG_N, FLAG_Z, Registers } from "../src/cpu/Registers.js";

describe("8-bit registers", () => {
  it("keeps only the low byte of what is written", () => {
    const r = new Registers();
    r.a = 0x1234;
    expect(r.a).toBe(0x34);
    r.l = -1;
    expect(r.l).toBe(0xff);
  });
});

describe("16-bit pairs", () => {
  it("splits BC into B and C", () => {
    const r = new Registers();
    r.bc = 0x1234;
    expect(r.b).toBe(0x12);
    expect(r.c).toBe(0x34);
  });

  it("joins HL back into one number", () => {
    const r = new Registers();
    r.h = 0xab;
    r.l = 0xcd;
    expect(r.hl).toBe(0xabcd);
  });

  it("wraps DE to 16 bits", () => {
    const r = new Registers();
    r.de = 0x12345;
    expect(r.de).toBe(0x2345);
  });
});

describe("flags register", () => {
  it("cannot hold anything in the low nibble", () => {
    const r = new Registers();
    r.f = 0xff;
    expect(r.f).toBe(0xf0);
    r.f = 0x0f;
    expect(r.f).toBe(0x00);
  });

  it("masks F when written through AF", () => {
    const r = new Registers();
    r.af = 0x00ff;
    expect(r.a).toBe(0x00);
    expect(r.f).toBe(0xf0);
  });

  it("flips one flag without disturbing the others", () => {
    const r = new Registers();
    r.f = FLAG_Z | FLAG_C;
    r.subtractFlag = true;
    expect(r.f).toBe(FLAG_Z | FLAG_N | FLAG_C);
    r.subtractFlag = false;
    expect(r.f).toBe(FLAG_Z | FLAG_C);
    expect(r.zeroFlag).toBe(true);
    expect(r.halfCarryFlag).toBe(false);
  });
});

describe("post-boot state", () => {
  it("matches what the boot ROM would leave", () => {
    const r = Registers.postBoot();
    expect(r.af).toBe(0x01b0);
    expect(r.bc).toBe(0x0013);
    expect(r.de).toBe(0x00d8);
    expect(r.hl).toBe(0x014d);
    expect(r.sp).toBe(0xfffe);
    expect(r.pc).toBe(0x0100);
  });
});
