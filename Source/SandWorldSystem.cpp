#include "SandWorldSystem.hpp"

#include "SandComponents.hpp"
#include "SandWorldGen.hpp"

#include "Snowstorm/Components/SpriteComponent.hpp"
#include "Snowstorm/Core/Application.hpp"
#include "Snowstorm/Core/Log.hpp"
#include "Snowstorm/Render/CommandContext.hpp"
#include "Snowstorm/Render/RenderGraph.hpp"
#include "Snowstorm/Render/RendererService.hpp"
#include "Snowstorm/Render/Shader.hpp"
#include "Snowstorm/Render/Texture.hpp"
#include "Snowstorm/Service/ServiceManager.hpp"
#include "Snowstorm/Utility/CVar.hpp"
#include "Snowstorm/World/SimulationStateSingleton.hpp"

#include <algorithm>
#include <cmath>

namespace Snowstorm
{
	namespace
	{
		CVar<int> SimSteps{"sand.sim.steps", 0,
		                   "Simulation steps per frame. 0 = real time at sand.sim.hz. A fixed count makes sand.stats exact per step."};
		CVar<int> SimHz{"sand.sim.hz", 60, "Simulation steps per second in real-time mode."};
		CVar<int> WidthOverride{"sand.width", 0, "World width in cells, overriding the scene. 0 = the scene's."};
		CVar<int> HeightOverride{"sand.height", 0, "World height in cells, overriding the scene. 0 = the scene's."};
		CVar<bool> Stats{"sand.stats", false, "Log the sim's GPU ms per step and the shade's ms per frame every 120 frames."};

		constexpr uint32_t kMaxStepsPerFrame = 8; // a long frame drops time rather than spiralling
		constexpr uint32_t kGroupSize = 8;        // numthreads in SandSim/SandShade.comp.hlsl
		constexpr uint32_t kStatWindow = 120;

		// Mirror the push constants in the shaders.
		struct SimPush
		{
			uint32_t Width, Height, Offset, Step;
		};
		struct ShadePush
		{
			uint32_t Width, Height;
		};

		Ref<Pipeline> MakeCompute(const char* path, const char* name, const uint32_t pushSize)
		{
			Ref<Shader> cs = Application::Get().GetServiceManager().GetService<ShaderLibrary>().Load(path);
			if (!cs || !cs->IsReady())
			{
				return nullptr; // compiling asynchronously; retried next frame
			}
			PipelineDesc p{};
			p.Type = PipelineType::Compute;
			p.Shader = cs;
			p.PushConstants = {{.Offset = 0, .Size = pushSize, .Stages = ShaderStage::Compute}};
			p.DebugName = name;
			return Pipeline::Create(p);
		}

		uint32_t Groups(const uint32_t n)
		{
			return (n + kGroupSize - 1) / kGroupSize;
		}
	}

	bool SandWorldSystem::EnsurePipelines()
	{
		if (!m_SimPipeline)
			m_SimPipeline = MakeCompute("/Game/shaders/SandSim.comp.hlsl", "SandSimPipeline", sizeof(SimPush));
		if (!m_ShadePipeline)
			m_ShadePipeline = MakeCompute("/Game/shaders/SandShade.comp.hlsl", "SandShadePipeline", sizeof(ShadePush));
		return m_SimPipeline && m_ShadePipeline;
	}

	void SandWorldSystem::EnsureTextures(const entt::entity e, SandWorldComponent& world, const uint32_t width,
	                                     const uint32_t height)
	{
		TextureDesc cells{};
		cells.Format = PixelFormat::RGBA8_UNorm;
		cells.Usage = TextureUsage::Sampled | TextureUsage::Storage | TextureUsage::TransferDst;
		cells.Width = width;
		cells.Height = height;
		cells.MipLevels = 1;
		cells.DebugName = "SandCells";
		world.Cells = Texture::Create(cells);

		TextureDesc colors = cells;
		colors.Format = PixelFormat::RGBA16_SFloat;
		colors.Usage = TextureUsage::Sampled | TextureUsage::Storage;
		colors.DebugName = "SandColors";
		world.Colors = Texture::Create(colors);

		std::vector<uint8_t>& pixels = m_Uploads.emplace_back(GenerateStartingWorld(width, height));
		ServiceView<RendererService>().EnqueueTextureUpload(pixels.data(), pixels.size(), world.Cells);

		m_CellsView = world.Cells->GetDefaultView();
		m_ColorsView = world.Colors->GetDefaultView();

		DescriptorSetDesc sim{};
		sim.DebugName = "SandSimSet";
		m_SimSet = DescriptorSet::Create(m_SimPipeline->GetSetLayouts()[0], sim);
		m_SimSet->SetTexture(0, m_CellsView);
		m_SimSet->Commit();

		DescriptorSetDesc shade{};
		shade.DebugName = "SandShadeSet";
		m_ShadeSet = DescriptorSet::Create(m_ShadePipeline->GetSetLayouts()[0], shade);
		m_ShadeSet->SetTexture(0, m_CellsView);
		m_ShadeSet->SetTexture(1, m_ColorsView);
		m_ShadeSet->Commit();

		m_World->GetRegistry().Write<SpriteComponent>(e).View = m_ColorsView;
		m_Step = 0;
		m_Accumulator = 0.0f;
	}

	uint32_t SandWorldSystem::StepsThisFrame(const float seconds)
	{
		if (m_World->HasSingleton<SimulationStateSingleton>() && !SingletonView<SimulationStateSingleton>().IsPlaying())
		{
			m_Accumulator = 0.0f;
			return 0; // editing: show the world, do not run it
		}
		if (SimSteps.Get() > 0)
		{
			return static_cast<uint32_t>(SimSteps.Get());
		}
		const float stepSeconds = 1.0f / static_cast<float>(std::max(SimHz.Get(), 1));
		m_Accumulator += seconds;
		const auto steps = std::min(static_cast<uint32_t>(m_Accumulator / stepSeconds), kMaxStepsPerFrame);
		m_Accumulator = std::min(m_Accumulator - static_cast<float>(steps) * stepSeconds, stepSeconds);
		return steps;
	}

	void SandWorldSystem::Execute(const Timestep ts)
	{
		m_Uploads.clear();
		if (Stats.Get())
		{
			LogStats();
		}
		if (!EnsurePipelines())
		{
			return;
		}

		auto& reg = m_World->GetRegistry();
		for (const entt::entity e : View<SandWorldComponent, SpriteComponent>())
		{
			const auto& read = reg.Read<SandWorldComponent>(e);
			const auto width = static_cast<uint32_t>(std::max(WidthOverride.Get() > 0 ? WidthOverride.Get() : read.Width, 1));
			const auto height = static_cast<uint32_t>(std::max(HeightOverride.Get() > 0 ? HeightOverride.Get() : read.Height, 1));
			if (!read.Cells || read.Cells->GetWidth() != width || read.Cells->GetHeight() != height ||
			    reg.Read<SpriteComponent>(e).View != m_ColorsView)
			{
				EnsureTextures(e, reg.Write<SandWorldComponent>(e), width, height);
			}

			const SandWorldComponent& world = reg.Read<SandWorldComponent>(e);
			const uint32_t steps = StepsThisFrame(ts.GetSeconds());
			const uint32_t firstStep = m_Step;
			m_Step += steps;
			m_StatSteps += steps;

			auto& renderer = ServiceView<RendererService>();
			renderer.EnqueueGraphPass(
			    {.Name = "SandWorld",
			     .IsCompute = true,
			     .Writes = {{world.Cells, RenderGraph::AccessState::Storage}, {world.Colors, RenderGraph::AccessState::Storage}},
			     .Execute = [this, width, height, steps, firstStep](CommandContext& c)
			     {
				     c.BindPipeline(m_SimPipeline);
				     c.BindDescriptorSet(m_SimSet, 0);
				     c.BeginGpuScope("SandSteps");
				     for (uint32_t i = 0; i < steps; ++i)
				     {
					     const uint32_t step = firstStep + i;
					     const SimPush push{width, height, step & 1u, step};
					     c.PushConstants(&push, sizeof(push));
					     // Blocks per axis, counting the partial ones the (-1,-1) offset adds at the edges.
					     c.Dispatch(Groups((width + 2) / 2), Groups((height + 2) / 2), 1);
					     c.BarrierComputeStorage();
				     }
				     c.EndGpuScope();

				     c.BindPipeline(m_ShadePipeline);
				     c.BindDescriptorSet(m_ShadeSet, 0);
				     c.BeginGpuScope("SandShade");
				     const ShadePush push{width, height};
				     c.PushConstants(&push, sizeof(push));
				     c.Dispatch(Groups(width), Groups(height), 1);
				     c.EndGpuScope();
			     }});
			// The sprite samples Colors through bindless, a read the graph cannot see, so leave it sampled here.
			renderer.EnqueueGraphPass({.Name = "SandPublish",
			                           .IsCompute = true,
			                           .Reads = {{world.Colors, RenderGraph::AccessState::Sampled}},
			                           .Execute = [](CommandContext&) {}});
		}
	}

	void SandWorldSystem::LogStats()
	{
		for (const GpuScope& s : ServiceView<RendererService>().GetGpuPassTimes())
		{
			if (s.Name == "SandSteps")
				m_StatStepMs += s.Milliseconds;
			else if (s.Name == "SandShade")
				m_StatShadeMs += s.Milliseconds;
		}
		if (++m_StatFrames < kStatWindow)
		{
			return;
		}
		const double perStep = m_StatSteps > 0 ? m_StatStepMs / m_StatSteps : 0.0;
		SS_INFO("sand.stats: {} frames, {} steps, {:.4f} ms/step, {:.4f} ms shade/frame (GPU)", m_StatFrames, m_StatSteps, perStep,
		        m_StatShadeMs / m_StatFrames);
		m_StatFrames = 0;
		m_StatSteps = 0;
		m_StatStepMs = 0.0;
		m_StatShadeMs = 0.0;
	}

	void SandWorldSystem::DeclareAccess(SystemAccess& a) const
	{
		a.WriteComponents<SandWorldComponent, SpriteComponent>().WriteResources<RendererService>().ReadResources<SimulationStateSingleton>();
	}
}
