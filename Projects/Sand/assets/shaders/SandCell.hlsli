// One cell is one RGBA8 texel: R = material, G = grain (a per-particle shade that moves with it), B and A
// unused. MUST match Source/SandCell.hpp.

static const uint kAir = 0;
static const uint kStone = 1;
static const uint kSand = 2;
static const uint kWater = 3;

uint Material(float4 cell)
{
	return uint(cell.r * 255.0 + 0.5);
}

uint Grain(float4 cell)
{
	return uint(cell.g * 255.0 + 0.5);
}
