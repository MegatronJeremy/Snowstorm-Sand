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

		// Runtime only (not reflected, so never serialized).
		Ref<Texture> Cells;  // RGBA8 cell state (SandCell.hpp), written in place by the sim
		Ref<Texture> Colors; // RGBA16F linear colour per cell, what the sprite samples
	};
}
