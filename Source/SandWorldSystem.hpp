#pragma once

#include "Snowstorm/ECS/System.hpp"
#include "Snowstorm/Render/Buffer.hpp"
#include "Snowstorm/Render/DescriptorSet.hpp"
#include "Snowstorm/Render/Pipeline.hpp"

#include <cstdint>
#include <vector>

namespace Snowstorm
{
	struct SandWorldComponent;

	// Owns each world's textures and runs it on the GPU: fixed-rate simulation steps while playing, then a
	// shade into the colour texture the world sprite shows. Runs in edit mode too, without stepping, so the
	// world is visible while the scene is authored.
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
		bool EnsurePipelines();
		void EnsureTextures(entt::entity e, SandWorldComponent& world, uint32_t width, uint32_t height);
		[[nodiscard]] uint32_t StepsThisFrame(float seconds);
		void WriteRetiredCaptures();
		void LogStats();

		Ref<Pipeline> m_SimPipeline;
		Ref<Pipeline> m_ShadePipeline;
		// Rebuilt with the textures; one world per scene. The views are held here because a descriptor set
		// does not keep what it binds alive, and GetDefaultView caches only weakly.
		Ref<TextureView> m_CellsView;
		Ref<TextureView> m_ColorsView;
		Ref<DescriptorSet> m_SimSet;
		Ref<DescriptorSet> m_ShadeSet;

		// EnqueueTextureUpload reads the bytes when the frame's graph executes, after this system returns.
		std::vector<std::vector<uint8_t>> m_Uploads;

		float m_Accumulator = 0.0f;
		uint32_t m_Step = 0; // seeds the sim's per-block random choices and picks the block offset

		// sand.capture.steps: steps still to capture, and copies waiting for their frame to retire.
		struct PendingCapture
		{
			Ref<Buffer> Readback;
			uint32_t Step = 0;
			uint32_t Width = 0;
			uint32_t Height = 0;
			uint64_t ReadyFrame = 0;
		};
		std::vector<uint32_t> m_PendingSteps;
		std::vector<PendingCapture> m_Captures;
		bool m_CaptureParsed = false;

		// sand.stats window
		uint32_t m_StatFrames = 0;
		uint32_t m_StatSteps = 0;
		double m_StatStepMs = 0.0;
		double m_StatShadeMs = 0.0;
	};
}
