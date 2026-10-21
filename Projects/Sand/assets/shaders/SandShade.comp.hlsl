// Turns cell state into the colour the world sprite shows. The output is linear and the present target
// re-encodes to sRGB, which round-trips every 8-bit palette value exactly: an sRGB image cannot be a storage
// target, and a UNORM one sampled as-is would be encoded twice.

#include "SandCell.hlsli"

[[vk::image_format("rgba8")]] RWTexture2D<float4> Cells : register(u0, space0);
[[vk::image_format("rgba16f")]] RWTexture2D<float4> Colors : register(u1, space0);

struct ShadePush
{
	uint2 Size;
};
[[vk::push_constant]] ShadePush gShade;

float3 SrgbToLinear(float3 c)
{
	return select(c <= 0.04045, c / 12.92, pow((c + 0.055) / 1.055, 2.4));
}

[numthreads(8, 8, 1)] void main(uint3 id : SV_DispatchThreadID)
{
	if (any(id.xy >= gShade.Size))
	{
		return;
	}

	const float4 cell = Cells[id.xy];
	const uint m = Material(cell);
	const int grain = int(Grain(cell)) - 8;

	// sRGB bytes, so the palette reads like the colours it is authored as.
	float3 rgb = float3(12, 12, 20);
	int shade = 0;
	if (m == kStone)
	{
		rgb = float3(70, 70, 78);
		shade = grain;
	}
	else if (m == kSand)
	{
		rgb = float3(204, 170, 102);
		shade = grain;
	}
	else if (m == kWater)
	{
		rgb = float3(40, 90, 180);
		shade = grain / 4;
	}

	Colors[id.xy] = float4(SrgbToLinear(clamp(rgb + shade, 0.0, 255.0) / 255.0), 1.0);
}
