#include "SandWorldGen.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace Snowstorm
{
	namespace
	{
		struct Rgb
		{
			uint8_t R, G, B;
		};

		constexpr Rgb kAir{12, 12, 20};
		constexpr Rgb kStone{70, 70, 78};
		constexpr Rgb kSand{204, 170, 102};
		constexpr Rgb kWater{40, 90, 180};

		// Per-cell grain, so a material reads as particles rather than a flat fill. Deterministic in (x, y).
		int Grain(const uint32_t x, const uint32_t y)
		{
			uint32_t h = x * 374761393u + y * 668265263u;
			h = (h ^ (h >> 13)) * 1274126177u;
			return static_cast<int>((h ^ (h >> 16)) % 17u) - 8;
		}

		Rgb Shade(const Rgb c, const int delta)
		{
			const auto ch = [delta](const uint8_t v)
			{ return static_cast<uint8_t>(std::clamp(v + delta, 0, 255)); };
			return {ch(c.R), ch(c.G), ch(c.B)};
		}
	}

	std::vector<uint8_t> GenerateStartingWorld(const uint32_t width, const uint32_t height)
	{
		std::vector<uint8_t> rgba(static_cast<size_t>(width) * height * 4);
		const float stoneTop = 0.15f;
		const float waterLevel = 0.27f;

		for (uint32_t x = 0; x < width; ++x)
		{
			const float u = (static_cast<float>(x) + 0.5f) / static_cast<float>(width);
			const float duneTop = stoneTop + 0.04f + 0.18f * (0.5f + 0.5f * std::sin(u * 3.0f * std::numbers::pi_v<float>));

			for (uint32_t row = 0; row < height; ++row)
			{
				const float v = 1.0f - (static_cast<float>(row) + 0.5f) / static_cast<float>(height); // 0 at the bottom
				Rgb c = kAir;
				if (v < stoneTop)
					c = Shade(kStone, Grain(x, row));
				else if (v < duneTop)
					c = Shade(kSand, Grain(x, row));
				else if (v < waterLevel)
					c = Shade(kWater, Grain(x, row) / 4);

				uint8_t* px = &rgba[(static_cast<size_t>(row) * width + x) * 4];
				px[0] = c.R;
				px[1] = c.G;
				px[2] = c.B;
				px[3] = 255;
			}
		}
		return rgba;
	}
}
