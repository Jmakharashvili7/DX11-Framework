#include "BaseShader.h"

BaseShader::BaseShader(HRESULT hr, WCHAR* file, ID3D11Device* device, D3D11_INPUT_ELEMENT_DESC layout[], UINT numElements)
{
    // Compile the vertex shader
    hr = CompileShaderFromFile(file, "VS", "vs_4_0", &m_pVSBlob);

    if (FAILED(hr))
    {
        MessageBox(nullptr,
            L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
    }

    // Create the vertex shader
    hr = device->CreateVertexShader(m_pVSBlob->GetBufferPointer(), m_pVSBlob->GetBufferSize(), nullptr, &m_VertexShader);

    if (FAILED(hr))
    {
        m_pVSBlob->Release();
    }

    // Compile the pixel shader
    ID3DBlob* pPSBlob = nullptr;
    hr = CompileShaderFromFile(file, "PS", "ps_4_0", &pPSBlob);

    if (FAILED(hr))
    {
        MessageBox(nullptr,
            L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
    }

    // Create the pixel shader
    hr = device->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_PixelShader);
    pPSBlob->Release();

    // Create the input layout 
    hr = device->CreateInputLayout(layout, numElements, m_pVSBlob->GetBufferPointer(),
        m_pVSBlob->GetBufferSize(), &m_InputLayout);
}

BaseShader::~BaseShader()
{
    if (m_VertexShader) m_VertexShader->Release();
    if (m_PixelShader) m_PixelShader->Release();
    if (m_pVSBlob) m_pVSBlob->Release();
}

HRESULT BaseShader::CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
    HRESULT hr = S_OK;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

    ID3DBlob* pErrorBlob;
    hr = D3DCompileFromFile(szFileName, nullptr, nullptr, szEntryPoint, szShaderModel,
        dwShaderFlags, 0, ppBlobOut, &pErrorBlob);

    if (FAILED(hr))
    {
        if (pErrorBlob != nullptr)
            OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());

        if (pErrorBlob) pErrorBlob->Release();

        return hr;
    }

    if (pErrorBlob) pErrorBlob->Release();

    return S_OK;
}

HRESULT BaseShader::CreateVertexShader(HRESULT hr, WCHAR* file, ID3D11Device* device)
{
    // Compile the vertex shader
    hr = CompileShaderFromFile(file, "VS", "vs_4_0", &m_pVSBlob);

    if (FAILED(hr))
    {
        MessageBox(nullptr,
            L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

    // Create the vertex shader
    hr = device->CreateVertexShader(m_pVSBlob->GetBufferPointer(), m_pVSBlob->GetBufferSize(), nullptr, &m_VertexShader);

    if (FAILED(hr))
    {
        m_pVSBlob->Release();
        return hr;
    }
}

HRESULT BaseShader::CreatePixelShader(HRESULT hr, WCHAR* file, ID3D11Device* device)
{
    // Compile the pixel shader
    ID3DBlob* pPSBlob = nullptr;
    hr = CompileShaderFromFile(file, "PS", "ps_4_0", &pPSBlob);

    if (FAILED(hr))
    {
        MessageBox(nullptr,
            L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

    // Create the pixel shader
    hr = device->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_PixelShader);
    pPSBlob->Release();

    if (FAILED(hr))
        return hr;
}
