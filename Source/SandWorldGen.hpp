#pragma once

#include <cstdint>
#include <vector>

namespace Snowstorm
{
	// The world a new grid starts as, one RGBA8 texel per cell, row 0 at the top (texture order): air above,
	// a sand dune, a water pool in its trough, a stone floor.
	[[nodiscard]] std::vector<uint8_t> GenerateStartingWorld(uint32_t width, uint32_t height);
}
