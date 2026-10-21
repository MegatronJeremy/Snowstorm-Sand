// One simulation step as a Margolus block cellular automaton: the grid is tiled into 2x2 blocks whose
// origin alternates between (0,0) and (-1,-1) every step, and one thread rewrites one block. A thread touches
// only its own four cells, so the in-place update has no write races and needs no atomics; the alternating
// offset is what lets matter cross block boundaries.

#include "SandCell.hlsli"

[[vk::image_format("rgba8")]] RWTexture2D<float4> Cells : register(u0, space0);

struct SimPush
{
	uint2 Size;
	uint Offset; // 0 or 1
	uint Step;   // seeds the per-block random choices
};
[[vk::push_constant]] SimPush gSim;

uint Hash(uint2 p, uint step)
{
	uint h = p.x * 374761393u + p.y * 668265263u + step * 2246822519u;
	h = (h ^ (h >> 13)) * 1274126177u;
	return h ^ (h >> 16);
}

bool Inside(int2 p)
{
	return all(p >= 0) && all(p < int2(gSim.Size));
}

// The grid edge is a wall: outside cells read as stone and are never written.
float4 Load(int2 p)
{
	return Inside(p) ? Cells[p] : float4(kStone / 255.0, 0.0, 0.0, 1.0);
}

void Store(int2 p, float4 c)
{
	if (Inside(p))
	{
		Cells[p] = c;
	}
}

bool Movable(uint m)
{
	return m == kSand || m == kWater;
}

uint Density(uint m)
{
	return m == kSand ? 2u : (m == kWater ? 1u : 0u);
}

// Whether `mover` may swap into the place of `target`: only into something lighter, never into stone.
bool Displaces(float4 mover, float4 target)
{
	const uint m = Material(mover);
	const uint t = Material(target);
	return Movable(m) && t != kStone && Density(t) < Density(m);
}

void Swap(inout float4 x, inout float4 y)
{
	const float4 t = x;
	x = y;
	y = t;
}

[numthreads(8, 8, 1)] void main(uint3 id : SV_DispatchThreadID)
{
	const int2 origin = int2(id.xy) * 2 - int(gSim.Offset);
	if (any(origin >= int2(gSim.Size)))
	{
		return;
	}

	// Row 0 is the top of the texture, so c and d lie below a and b.
	//   a b
	//   c d
	float4 a = Load(origin);
	float4 b = Load(origin + int2(1, 0));
	float4 c = Load(origin + int2(0, 1));
	float4 d = Load(origin + int2(1, 1));
	const uint r = Hash(uint2(origin + 1), gSim.Step);

	// Fall straight down.
	if (Displaces(a, c))
		Swap(a, c);
	if (Displaces(b, d))
		Swap(b, d);

	// Topple: a cell that could not fall slides diagonally. When both diagonals are open, the random bit picks
	// which one goes, so piles grow symmetrically.
	const bool aSlides = Displaces(a, d) && !Displaces(a, c);
	const bool bSlides = Displaces(b, c) && !Displaces(b, d);
	if (aSlides && (!bSlides || (r & 1u) != 0))
		Swap(a, d);
	else if (bSlides)
		Swap(b, c);

	// Flow: a liquid steps sideways into lighter matter half the time, which is what levels a pool.
	if ((r & 2u) != 0)
	{
		if ((Material(a) == kWater && Displaces(a, b)) || (Material(b) == kWater && Displaces(b, a)))
			Swap(a, b);
		if ((Material(c) == kWater && Displaces(c, d)) || (Material(d) == kWater && Displaces(d, c)))
			Swap(c, d);
	}

	Store(origin, a);
	Store(origin + int2(1, 0), b);
	Store(origin + int2(0, 1), c);
	Store(origin + int2(1, 1), d);
}
