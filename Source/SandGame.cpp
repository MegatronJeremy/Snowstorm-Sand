#include "SandGame.hpp"

#include "SandWorldSystem.hpp"

#include "Snowstorm/ECS/SystemManager.hpp"
#include "Snowstorm/ECS/SystemPhase.hpp"
#include "Snowstorm/World/World.hpp"

namespace Snowstorm
{
	void RegisterSandSystems(World& world)
	{
		world.GetSystemManager().RegisterSystem<SandWorldSystem>(SystemPhase::Resolve);
	}
}
