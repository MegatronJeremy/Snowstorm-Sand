#include "SandWorldSystem.hpp"

#include "SandComponents.hpp"
#include "SandWorldGen.hpp"

#include "Snowstorm/Components/SpriteComponent.hpp"
#include "Snowstorm/Render/RendererService.hpp"
#include "Snowstorm/Render/Texture.hpp"

#include <algorithm>

namespace Snowstorm
{
	void SandWorldSystem::Execute(Timestep)
	{
		m_Uploads.clear();
		auto& reg = m_World->GetRegistry();
		auto& renderer = ServiceView<RendererService>();

		for (const entt::entity e : View<SandWorldComponent, SpriteComponent>())
		{
			const auto& world = reg.Read<SandWorldComponent>(e);
			const auto width = static_cast<uint32_t>(std::max(world.Width, 1));
			const auto height = static_cast<uint32_t>(std::max(world.Height, 1));
			const bool sized = world.Cells && world.Cells->GetWidth() == width && world.Cells->GetHeight() == height;
			if (sized && reg.Read<SpriteComponent>(e).View)
			{
				continue;
			}

			if (!sized)
			{
				TextureDesc desc{};
				desc.Format = PixelFormat::RGBA8_sRGB; // cell colours are authored sRGB; the sprite pass re-encodes exactly
				desc.Usage = TextureUsage::Sampled | TextureUsage::TransferDst;
				desc.Width = width;
				desc.Height = height;
				desc.MipLevels = 1;
				desc.DebugName = "SandCells";
				Ref<Texture> cells = Texture::Create(desc);

				std::vector<uint8_t>& pixels = m_Uploads.emplace_back(GenerateStartingWorld(width, height));
				renderer.EnqueueTextureUpload(pixels.data(), pixels.size(), cells);
				reg.Write<SandWorldComponent>(e).Cells = std::move(cells);
			}
			reg.Write<SpriteComponent>(e).View = reg.Read<SandWorldComponent>(e).Cells->GetDefaultView();
		}
	}

	void SandWorldSystem::DeclareAccess(SystemAccess& a) const
	{
		a.WriteComponents<SandWorldComponent, SpriteComponent>().WriteResources<RendererService>();
	}
}
