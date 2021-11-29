#pragma once

#include <d3d11_1.h>
#include <directxmath.h>
#include "structs.h"

using namespace DirectX;

class BaseObjectOBJ
{
public:
	XMFLOAT4X4 m_World;
protected:
	MeshData m_MeshData;
public:
	BaseObjectOBJ(MeshData meshData);
	~BaseObjectOBJ();

	// Main function for rendering the object
	void Render(XMMATRIX& worldMatrix, ConstantBuffer& buffer, ID3D11Buffer* constBuffer, ID3D11DeviceContext* deviceContext);

	// Load and store functions for world matrix
	inline void SetWorldMatrix(XMMATRIX matrix) { return XMStoreFloat4x4(&m_World, matrix); }
	inline XMMATRIX GetWorldMatrix() { return XMLoadFloat4x4(&m_World); }
	inline XMFLOAT4X4 GetWorldFloat4x4() { return m_World; }
};
