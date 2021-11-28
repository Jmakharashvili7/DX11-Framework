#pragma once
#include "DirectXMath.h"

using namespace DirectX;


struct MeshData
{
	ID3D11Buffer *VertexBuffer;
	ID3D11Buffer *IndexBuffer;
	UINT VBStride;
	UINT VBOffset;
	UINT IndexCount;
};

struct SimpleVertex
{
	XMFLOAT3 pos;
	XMFLOAT3 normal;
	XMFLOAT2 texCord;

	bool operator<(const SimpleVertex other) const
	{
		return memcmp((void*)this, (void*)&other, sizeof(SimpleVertex)) > 0;
	};
};

struct ConstantBuffer
{
	XMMATRIX mWorld;
	XMMATRIX mView;
	XMMATRIX mProjection;
	XMFLOAT4 DiffuseMtrl;
	XMFLOAT4 DiffuseLight;
	XMFLOAT3 LightVecW;
	float	 SpecularPower;
	XMFLOAT4 SpecularMaterial;
	XMFLOAT4 SpecularLight;
	XMFLOAT3 EyePosW; // camera position in world space
};