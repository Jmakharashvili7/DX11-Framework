#pragma once
#include <d3d11_1.h>
#include <directxmath.h>

struct SimpleVertex;

namespace Math3D {
	void NormalAvarage(SimpleVertex vertices[], const WORD indices[], UINT vertexCount, UINT indexCount);
}