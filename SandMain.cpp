#include <Snowstorm.h>
#include <Snowstorm/Core/EntryPoint.hpp>
#include <Snowstorm/Core/GameLayer.hpp>

#include "SandGame.hpp"

namespace Snowstorm
{
	class SandApplication final : public Application
	{
	public:
		SandApplication()
		    : Application("Snowstorm-Sand")
		{
			PushLayer(new GameLayer([](World& world)
			                        { RegisterSandSystems(world); },
			                        "Projects/Sand/Sand.ssproj"));
		}
	};

	Application* CreateApplication()
	{
		return new SandApplication();
	}
}
