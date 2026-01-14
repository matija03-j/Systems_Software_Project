#pragma once
#include <stdint.h>

constexpr auto PC_start = 0x40000000;
constexpr auto SP_start = 0xFFFFFF00;

constexpr uint32_t PC_REG = 15;
constexpr uint32_t SP_REG = 14;
constexpr auto STATUS = 0;
constexpr auto HANDLER = 1;
constexpr auto CAUSE = 2;

constexpr auto HALT_OC = 0b0000;
constexpr auto INT_OC = 0b0001;
constexpr auto CALL_OC = 0b0010;
constexpr auto JUMP_OC = 0b0011;
constexpr auto XCHG_OC = 0b0100;
constexpr auto ARIT_OC = 0b0101;
constexpr auto LOGIC_OC = 0b0110;
constexpr auto SHIFT_OC = 0b0111;
constexpr auto STORE_OC = 0b1000;
constexpr auto LOAD_OC = 0b1001;

constexpr auto CALL_MOD0 = 0b0000;
constexpr auto CALL_MOD1 = 0b0001;

constexpr auto JMP_MOD0 = 0b0000;
constexpr auto JMP_MOD1 = 0b0001;
constexpr auto JMP_MOD2 = 0b0010;
constexpr auto JMP_MOD3 = 0b0011;
constexpr auto JMP_MOD4 = 0b1000;
constexpr auto JMP_MOD5 = 0b1001;
constexpr auto JMP_MOD6 = 0b1010;
constexpr auto JMP_MOD7 = 0b1011;

constexpr auto ADD_MOD = 0b0000;
constexpr auto SUB_MOD = 0b0001;
constexpr auto MUL_MOD = 0b0010;
constexpr auto DIV_MOD = 0b0011;

constexpr auto NOT_MOD = 0b0000;
constexpr auto AND_MOD = 0b0001;
constexpr auto OR_MOD = 0b0010;
constexpr auto XOR_MOD = 0b0011;

constexpr auto SHL_MOD = 0b0000;
constexpr auto SHR_MOD = 0b0001;

constexpr auto STORE_MOD0 = 0b0000;
constexpr auto STORE_MOD1 = 0b0001;
constexpr auto STORE_MOD2 = 0b0010;

constexpr auto LOAD_MOD0 = 0b0000;
constexpr auto LOAD_MOD1 = 0b0001;
constexpr auto LOAD_MOD2 = 0b0010;
constexpr auto LOAD_MOD3 = 0b0011;
constexpr auto LOAD_MOD4 = 0b0100;
constexpr auto LOAD_MOD5 = 0b0101;
constexpr auto LOAD_MOD6 = 0b0110;
constexpr auto LOAD_MOD7 = 0b0111;