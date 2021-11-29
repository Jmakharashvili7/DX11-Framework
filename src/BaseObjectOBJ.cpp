#include "BaseObjectOBJ.h"
#include "Math3D.h"

BaseObjectOBJ::BaseObjectOBJ(MeshData meshData)
{
    m_MeshData = meshData;
}

BaseObjectOBJ::~BaseObjectOBJ()
{

}

void BaseObjectOBJ::Render(XMMATRIX& worldMatrix, ConstantBuffer& buffer, ID3D11Buffer* constBuffer, ID3D11DeviceContext* deviceContext)
{
    UINT stride = sizeof(SimpleVertex);
    UINT offset = 0;

    worldMatrix = XMLoadFloat4x4(&m_World);
    buffer.mWorld = XMMatrixTranspose(worldMatrix);
    deviceContext->IASetVertexBuffers(0, 1, &m_MeshData.VertexBuffer, &m_MeshData.VBStride, &m_MeshData.VBOffset);
    deviceContext->IASetIndexBuffer(m_MeshData.IndexBuffer, DXGI_FORMAT_R16_UINT, 0);
    deviceContext->UpdateSubresource(constBuffer, 0, nullptr, &buffer, 0, 0);
    deviceContext->DrawIndexed(m_MeshData.IndexCount, 0, 0);
}



