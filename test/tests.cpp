#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>

#include "chip8.h"

template<bool test>
bool static_test()
{
	static_assert(test);
	return test;
}

#define TEST(x) static_test<x>()

template<typename... T>
constexpr auto run(T... program)
{
	std::array<uint8_t, sizeof...(T)> data{ static_cast<uint8_t>(program)... };

	chip8 emu{ data };
	emu.run();

	return emu;
}

TEST_CASE("00E0 resets all pixels", "[opcode]")
{
	constexpr auto emu = run(0xD0, 0x15, 0x00, 0xE0);

	REQUIRE(TEST(emu.is_pixel_set(0, 0) == false));
	REQUIRE(TEST(emu.is_pixel_set(63, 0) == false));
	REQUIRE(TEST(emu.is_pixel_set(0, 31) == false));
	REQUIRE(TEST(emu.is_pixel_set(63, 31) == false));
}

TEST_CASE("00EE returns from a subroutine", "[opcode]")
{
	constexpr auto emu = run(0x22, 0x04, 0x00, 0xEE, 0x00, 0xEE);

	REQUIRE(TEST(emu.program_counter == 514));
	REQUIRE(TEST(emu.stack_pointer == 0));
}

TEST_CASE("1NNN jumps to address NNN", "[opcode]")
{
	constexpr auto emu = run(0x12, 0x04, 0x00, 0xEE, 0x00, 0xEE);

	REQUIRE(TEST(emu.program_counter == 516));
}

TEST_CASE("2NNN calls a subroutine at NNN", "[opcode]")
{
	constexpr auto emu = run(0x22, 0x04, 0x00, 0xEE);

	REQUIRE(TEST(emu.program_counter == 516));
	REQUIRE(TEST(emu.stack_pointer == 1));
}

TEST_CASE("3XNN skips the next instruction if Vx equals NN", "[opcode]")
{
	constexpr auto emu = run(0x60, 0x12, 0x30, 0x12, 0x00, 0xEE, 0x00, 0xEE);

	REQUIRE(TEST(emu.program_counter == 518));
}

TEST_CASE("4XNN skips the next instruction if Vx does not equal NN", "[opcode]")
{
	constexpr auto emu = run(0x60, 0x12, 0x40, 0x12, 0x00, 0xEE, 0x00, 0xEE);

	REQUIRE(TEST(emu.program_counter == 516));
}

TEST_CASE("5XY0 skips the next instruction if Vx equals Vy", "[opcode]")
{
	constexpr auto emu = run(0x60, 0x12, 0x61, 0x12, 0x50, 0x10, 0x00, 0xEE, 0x00, 0xEE);

	REQUIRE(TEST(emu.program_counter == 520));
}

TEST_CASE("6XNN sets Vx to NN", "[opcode]")
{
	constexpr auto emu = run(0x60, 0x12, 0x6F, 0x13);

	REQUIRE(TEST(emu.registers.data[0x0] == 18));
	REQUIRE(TEST(emu.registers.data[0xF] == 19));
}

TEST_CASE("7XNN adds NN to Vx", "[opcode]")
{
	constexpr auto emu = run(0x60, 0x12, 0x70, 0x15);

	REQUIRE(TEST(emu.registers.data[0x0] == 39));
}

TEST_CASE("8XY0 sets Vx to Vy", "[opcode]")
{
	constexpr auto emu = run(0x60, 0x12, 0x81, 0x00);

	REQUIRE(TEST(emu.registers.data[0x1] == 18));
}

TEST_CASE("8XY1 sets Vx to Vx OR Vy", "[opcode]")
{
	constexpr auto emu = run(0x60, 0x14, 0x61, 0x0C, 0x80, 0x11);

	REQUIRE(TEST(emu.registers.data[0x0] == 28));
}

TEST_CASE("8XY2 sets Vx to Vx AND Vy", "[opcode]")
{
	constexpr auto emu = run(0x60, 0x14, 0x61, 0x0C, 0x80, 0x12);

	REQUIRE(TEST(emu.registers.data[0x0] == 4));
}

TEST_CASE("8XY3 sets Vx to Vx XOR Vy", "[opcode]")
{
	constexpr auto emu = run(0x60, 0x14, 0x61, 0x0C, 0x80, 0x13);

	REQUIRE(TEST(emu.registers.data[0x0] == 24));
}

TEST_CASE("8XY4 adds Vy to Vx, settings Vf to 1 when there is a carry", "[opcode]")
{
	constexpr auto emu = run(0x60, 0xC8, 0x61, 0x64, 0x80, 0x14);

	REQUIRE(TEST(emu.registers.data[0x0] == 44));
	REQUIRE(TEST(emu.registers.data[0xF] == 1));
}

TEST_CASE("8XY4 adds Vy to Vx, settings Vf to 0 when there is not a carry", "[opcode]")
{
	constexpr auto emu = run(0x60, 0xC8, 0x61, 0x0A, 0x80, 0x14);

	REQUIRE(TEST(emu.registers.data[0x0] == 210));
	REQUIRE(TEST(emu.registers.data[0xF] == 0));
}

TEST_CASE("8XY5 subtracts Vy from Vx, settings Vf to 0 when there is a borrow", "[opcode]")
{
	constexpr auto emu = run(0x60, 0x0A, 0x61, 0x0F, 0x80, 0x15);

	REQUIRE(TEST(emu.registers.data[0x0] == 251));
	REQUIRE(TEST(emu.registers.data[0xF] == 0));
}

TEST_CASE("8XY5 subtracts Vy from Vx, settings Vf to 1 when there is not a borrow", "[opcode]")
{
	constexpr auto emu = run(0x60, 0x0A, 0x61, 0x09, 0x80, 0x15);

	REQUIRE(TEST(emu.registers.data[0x0] == 1));
	REQUIRE(TEST(emu.registers.data[0xF] == 1));
}

TEST_CASE("8XY6 stores the least significant bit of Vx in Vf, and shifts Vx to the right by 1", "[opcode]")
{
	constexpr auto emu = run(0x60, 0x79, 0x80, 0x16);

	REQUIRE(TEST(emu.registers.data[0x0] == 60));
	REQUIRE(TEST(emu.registers.data[0xF] == 1));
}

TEST_CASE("8XY7 sets Vx to Vy minus Vx, settings Vf to 0 when there is a borrow", "[opcode]")
{
	constexpr auto emu = run(0x60, 0x0F, 0x61, 0x0A, 0x80, 0x17);

	REQUIRE(TEST(emu.registers.data[0x0] == 251));
	REQUIRE(TEST(emu.registers.data[0xF] == 0));
}

TEST_CASE("8XY7 sets Vx to Vy minus Vx, settings Vf to 1 when there is not a borrow", "[opcode]")
{
	constexpr auto emu = run(0x60, 0x09, 0x61, 0x0A, 0x80, 0x17);

	REQUIRE(TEST(emu.registers.data[0x0] == 1));
	REQUIRE(TEST(emu.registers.data[0xF] == 1));
}

TEST_CASE("8XYE stores the most significant bit of Vx in Vf, and shifts Vx to the left by 1", "[opcode]")
{
	constexpr auto emu = run(0x60, 0xF3, 0x80, 0x1E);

	REQUIRE(TEST(emu.registers.data[0x0] == 230));
	REQUIRE(TEST(emu.registers.data[0xF] == 1));
}

TEST_CASE("9XY0 skips the next instruction if Vx does not equal Vy", "[opcode]")
{
	constexpr auto emu = run(0x60, 0x12, 0x61, 0x13, 0x50, 0x10, 0x00, 0xEE, 0x00, 0xEE);

	REQUIRE(TEST(emu.program_counter == 518));
}

TEST_CASE("ANNN sets the address register to NNN", "[opcode]")
{
	constexpr auto emu = run(0xA0, 0x12);

	REQUIRE(TEST(emu.registers.address == 18));
}

TEST_CASE("BNNN jumps to address NNN plus V0", "[opcode]")
{
	constexpr auto emu = run(0x60, 0x06, 0xB2, 0x00, 0x00, 0xEE, 0x00, 0xEE);

	REQUIRE(TEST(emu.program_counter == 518));
}

// CXNN cannot be tested easily at compile-time

TEST_CASE("DXYN draws a sprite located at the address register at (Vx,Vy), with a height of N", "[opcode]")
{
	constexpr auto emu = run(0xD0, 0x05);

	REQUIRE(TEST(emu.is_pixel_set(0, 0) == true));
	REQUIRE(TEST(emu.is_pixel_set(3, 0) == true));
	REQUIRE(TEST(emu.is_pixel_set(0, 4) == true));
	REQUIRE(TEST(emu.is_pixel_set(3, 4) == true));
}

// EX9E and EXA1 cannot be tested as key input is not yet implemented

TEST_CASE("FX07 sets Vx to the delay timer", "[opcode]")
{
	constexpr auto emu = run(0xF0, 0x07);

	REQUIRE(TEST(emu.registers.data[0x0] == 60));
}

// FX0A cannot be tested as key input is not yet implemented

TEST_CASE("FX15 sets the delay timer to Vx", "[opcode]")
{
	constexpr auto emu = run(0x60, 0x12, 0xF0, 0x15);

	REQUIRE(TEST(emu.delay_timer == 17));
}

TEST_CASE("FX18 sets the sound timer to Vx", "[opcode]")
{
	constexpr auto emu = run(0x60, 0x12, 0xF0, 0x18);

	REQUIRE(TEST(emu.sound_timer == 17));
}

TEST_CASE("FX1E adds Vx to the address register, setting Vf to 1 when there is a range overflow", "[opcode]")
{
	constexpr auto emu = run(0xAF, 0xFF, 0x60, 0xFF, 0xF0, 0x1E);

	REQUIRE(TEST(emu.registers.address == 4350));
	REQUIRE(TEST(emu.registers.data[0xF] == 1));
}

TEST_CASE("FX1E adds Vx to the address register, setting Vf to 0 when there is not a range overflow", "[opcode]")
{
	constexpr auto emu = run(0xAF, 0xA0, 0x60, 0x0A, 0xF0, 0x1E);

	REQUIRE(TEST(emu.registers.address == 4010));
	REQUIRE(TEST(emu.registers.data[0xF] == 0));
}

TEST_CASE("FX29 sets the address register to the location of the sprite for the character in Vx", "[opcode]")
{
	constexpr auto emu = run(0x60, 0x0F, 0xF0, 0x29);

	REQUIRE(TEST(emu.registers.address == 75));
}

TEST_CASE("FX33 stores the binary-coded decimal representation of Vx in the address register and next two locations", "[opcode]")
{
	constexpr auto emu = run(0xA2, 0x08, 0x60, 0xEB, 0xF0, 0x33);

	REQUIRE(TEST(emu.memory[520] == 2));
	REQUIRE(TEST(emu.memory[521] == 3));
	REQUIRE(TEST(emu.memory[522] == 5));
}

TEST_CASE("FX55 stores V0 to Vx in memory starting at the address register", "[opcode]")
{
	constexpr auto emu = run(0xA2, 0xFF, 0x60, 0xFF, 0x6A, 0xF0, 0xFA, 0x55);

	REQUIRE(TEST(emu.memory[767] == 255));
	REQUIRE(TEST(emu.memory[777] == 240));
}

TEST_CASE("FX65 fills V0 to Vx with memory starting at the address register", "[opcode]")
{
	constexpr auto emu = run(0xA2, 0x06, 0xF5, 0x65, 0x00, 0xEE, 0x01, 0x02, 0x05, 0x10, 0x20, 0xFF);

	REQUIRE(TEST(emu.registers.data[0x0 == 1]));
	REQUIRE(TEST(emu.registers.data[0x1 == 2]));
	REQUIRE(TEST(emu.registers.data[0x2 == 5]));
	REQUIRE(TEST(emu.registers.data[0x3 == 16]));
	REQUIRE(TEST(emu.registers.data[0x4 == 32]));
	REQUIRE(TEST(emu.registers.data[0x5 == 255]));
}
