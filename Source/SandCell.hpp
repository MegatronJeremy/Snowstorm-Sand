#pragma once

#include <cstdint>

namespace Snowstorm
{
	// One cell is one RGBA8 texel: R = material, G = grain (a per-particle shade that moves with it), B and A
	// unused. MUST match SandCell.hlsli.
	enum class SandMaterial : uint8_t
	{
		Air = 0,
		Stone = 1,
		Sand = 2,
		Water = 3,
	};

	inline constexpr uint8_t kSandGrainLevels = 17; // grain is 0..16; the shade pass maps it to -8..+8
}
