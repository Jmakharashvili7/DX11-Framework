#pragma once
#include "DirectXMath.h"

using namespace DirectX;

struct SimpleVertex
{
	XMFLOAT3 pos;
    XMFLOAT3 normal;
	XMFLOAT2 texCord;
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