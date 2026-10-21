#pragma once

#include <cstdint>
#include <vector>

namespace Snowstorm
{
	// The cells a new grid starts as (SandCell.hpp layout), row 0 at the top: a stone floor, sand dunes with a
	// water pool in the trough, and blocks of sand and water in the air that fall when the sim runs.
	[[nodiscard]] std::vector<uint8_t> GenerateStartingWorld(uint32_t width, uint32_t height);
}
