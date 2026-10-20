#include "Include/Engine.hlsli"

// Presents the cell grid bound to the material's albedo slot, one texel per cell. Unlit: cells carry their
// own colour, and scene lighting would dim it.

float4 main(PSInput i) : SV_Target0
{
	// Mesh.vert always writes InstanceID, and dxc drops an unread member from the fragment input signature,
	// leaving a vertex output nothing consumes (a validation warning per pipeline). The branch never runs.
	if (i.InstanceID == 0xFFFFFFFFu)
	{
		return float4(0.0, 0.0, 0.0, 1.0);
	}

	// Slot 0 is the engine's default white, i.e. no grid bound.
	if (AlbedoTextureIndex == 0)
	{
		return float4(BaseColor.rgb, 1.0);
	}

	// Load, not Sample: a cell is a texel, and no engine sampler is point-filtered.
	Texture2D grid = Textures[NonUniformResourceIndex(AlbedoTextureIndex)];
	uint width;
	uint height;
	grid.GetDimensions(width, height);
	const uint2 texel = min(uint2(i.TexCoord * float2(width, height)), uint2(width - 1, height - 1));
	return float4(grid.Load(int3(texel, 0)).rgb, 1.0);
}
