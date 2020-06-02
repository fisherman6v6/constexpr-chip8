#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>

template<uint16_t memory_size = 4096>
class chip8
{
public:
	// Emulator specifications
	static constexpr uint8_t display_width = 64;
	static constexpr uint8_t display_height = 32;
	static constexpr uint8_t max_stacks = 12;

	// Sizes used for later calculations
	static constexpr uint16_t display_memory_size = (display_width * display_height) / 8; // Each pixel is only a single bit
	static constexpr uint16_t stack_size = max_stacks * 4; // 4 byte stack pointers
	static constexpr uint16_t reserved_memory_size = 96; // Reserved for the call stack and address register
	
	static constexpr uint16_t display_memory_start = memory_size - display_memory_size;
	static constexpr uint16_t call_stack_start = display_memory_start - reserved_memory_size;
	static constexpr uint16_t address_register = call_stack_start + stack_size;

	static constexpr uint16_t program_memory_start = 512; // The first 512 bytes are for internal use only
	static constexpr uint16_t program_memory_end = call_stack_start;

	static_assert(program_memory_end - program_memory_start > 0, "No memory for programs");

	template<std::size_t size>
	constexpr chip8(const std::array<uint8_t, size>& program) noexcept
	{
		assert(size <= program_memory_end - program_memory_start);

		for (auto i = 0; i < size; ++i)
			m_memory[static_cast<decltype(m_memory)::size_type>(program_memory_start) + i] = program[i];
	}

private:
	std::array<uint8_t, memory_size> m_memory{};
	std::array<uint8_t, 16> m_registers{};

	uint16_t m_program_counter = program_memory_start;
	uint16_t m_stack_pointer = call_stack_start;

	uint8_t m_delay_timer = 60;
	uint8_t m_sound_timer = 60;
};
