#include "SandWorldGen.hpp"

#include "SandCell.hpp"

#include <cmath>
#include <numbers>

namespace Snowstorm
{
	namespace
	{
		// Deterministic in (x, y), so a fresh world looks the same every run.
		uint8_t GrainAt(const uint32_t x, const uint32_t y)
		{
			uint32_t h = x * 374761393u + y * 668265263u;
			h = (h ^ (h >> 13)) * 1274126177u;
			return static_cast<uint8_t>((h ^ (h >> 16)) % kSandGrainLevels);
		}

		// A square of 0.08 x 0.12 of the world (roughly square cells at 16:9) with its bottom-left at (u0, v0).
		bool InBlock(const float u, const float v, const float u0, const float v0)
		{
			return u >= u0 && u < u0 + 0.08f && v >= v0 && v < v0 + 0.12f;
		}
	}

	std::vector<uint8_t> GenerateStartingWorld(const uint32_t width, const uint32_t height)
	{
		std::vector<uint8_t> cells(static_cast<size_t>(width) * height * 4);
		const float stoneTop = 0.15f;
		const float waterLevel = 0.27f;

		for (uint32_t x = 0; x < width; ++x)
		{
			const float u = (static_cast<float>(x) + 0.5f) / static_cast<float>(width);
			const float duneTop = stoneTop + 0.04f + 0.18f * (0.5f + 0.5f * std::sin(u * 3.0f * std::numbers::pi_v<float>));

			for (uint32_t row = 0; row < height; ++row)
			{
				const float v = 1.0f - (static_cast<float>(row) + 0.5f) / static_cast<float>(height); // 0 at the bottom
				SandMaterial m = SandMaterial::Air;
				if (v < stoneTop)
					m = SandMaterial::Stone;
				else if (v < duneTop)
					m = SandMaterial::Sand;
				else if (v < waterLevel)
					m = SandMaterial::Water;
				else if (InBlock(u, v, 0.12f, 0.70f))
					m = SandMaterial::Sand; // falls onto the left dune and piles
				else if (InBlock(u, v, 0.78f, 0.70f))
					m = SandMaterial::Water; // falls onto the right dune and runs into the pool
				else if (InBlock(u, v, 0.46f, 0.55f))
					m = SandMaterial::Sand; // falls into the pool and sinks through it

				uint8_t* cell = &cells[(static_cast<size_t>(row) * width + x) * 4];
				cell[0] = static_cast<uint8_t>(m);
				cell[1] = GrainAt(x, row);
				cell[2] = 0;
				cell[3] = 255;
			}
		}
		return cells;
	}
}
