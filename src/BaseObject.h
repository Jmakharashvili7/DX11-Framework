#pragma once

#include <d3d11_1.h>
#include <directxmath.h>
#include "structs.h"

using namespace DirectX;

class BaseObject
{
public:
	XMFLOAT4X4 m_world;
private:
	ID3D11Buffer* m_indexBuffer, *m_vertexBuffer;
public:
	BaseObject(ID3D11Device* localDevice, SimpleVertex vertices[], WORD indices[], HRESULT hr, 
    UINT indexCount, UINT vertexCount);
	~BaseObject();

	void Render(XMMATRIX& worldMatrix, ConstantBuffer& buffer, ID3D11Buffer* constBuffer, ID3D11DeviceContext* deviceContext);

	inline ID3D11Buffer* GetIndexBuffer() { return m_indexBuffer; }
	inline ID3D11Buffer* GetVertexBuffer() { return m_vertexBuffer; } 
};

