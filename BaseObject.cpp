#include "BaseObject.h"
#include "Math3D.h"

BaseObject::BaseObject(ID3D11Device* localDevice, SimpleVertex vertices[], WORD indices[], HRESULT hr, 
    UINT indexCount, UINT vertexCount) 
{
    Math3D::NormalAvarage(vertices, indices, vertexCount, indexCount);

    //
    // Create and bind the vertex buffer
    //
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(SimpleVertex) * vertexCount;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = vertices;

    // check for any errors with binding the buffer
    hr = localDevice->CreateBuffer(&bd, &InitData, &m_vertexBuffer);

    //
    // Create and bind the index buffer
    //
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(WORD) * indexCount;     
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;

	ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = indices;
    hr = localDevice->CreateBuffer(&bd, &InitData, &m_indexBuffer);
}

BaseObject::~BaseObject()
{
	delete m_indexBuffer;
	delete m_vertexBuffer;
}

void BaseObject::Render(XMMATRIX& worldMatrix, ConstantBuffer& buffer, ID3D11Buffer* constBuffer, ID3D11DeviceContext* deviceContext)
{
    UINT stride = sizeof(SimpleVertex);
    UINT offset = 0;

    worldMatrix = XMLoadFloat4x4(&m_world);
    buffer.mWorld = XMMatrixTranspose(worldMatrix);
    deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
    deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R16_UINT, 0);
    deviceContext->UpdateSubresource(constBuffer, 0, nullptr, &buffer, 0, 0);
    deviceContext->DrawIndexed(36, 0, 0);
}



