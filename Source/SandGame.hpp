#pragma once

namespace Snowstorm
{
	class World;

	// The seam between this game and its host. Call it after RegisterCoreSystems on the same world, so
	// SandWorldSystem runs behind SpriteResolveSystem in the Resolve phase.
	void RegisterSandSystems(World& world);
}
