#pragma once

#include "Snowstorm/ECS/System.hpp"

#include <cstdint>
#include <vector>

namespace Snowstorm
{
	// Gives each SandWorldComponent its cell texture and shows it through the entity's sprite. Runs in edit
	// mode too, so the world is visible while the scene is authored.
	class SandWorldSystem final : public System
	{
	public:
		explicit SandWorldSystem(const WorldRef world)
		    : System(world)
		{
		}

		void Execute(Timestep ts) override;
		void DeclareAccess(SystemAccess& a) const override;

	private:
		// EnqueueTextureUpload reads the bytes when the frame's graph executes, after this system returns.
		std::vector<std::vector<uint8_t>> m_Uploads;
	};
}
