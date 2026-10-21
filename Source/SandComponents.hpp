#pragma once

#include "Snowstorm/Core/Base.hpp"
#include "Snowstorm/Render/Texture.hpp"

namespace Snowstorm
{
	// The simulated world: a grid of cells, shown through the same entity's SpriteComponent.
	struct SandWorldComponent
	{
		int Width = 320; // cells
		int Height = 180;

		// Runtime only (not reflected, so never serialized): one RGBA8 texel per cell.
		Ref<Texture> Cells;
	};
}
