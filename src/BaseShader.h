#pragma once
#include <d3d11_1.h>
#include <directxmath.h>
#include <windows.h>
#include <d3dcompiler.h>
#include "structs.h"

using namespace DirectX;

class BaseShader
{
public:
private:
	ID3DBlob* m_pVSBlob = nullptr;
	ID3D11VertexShader* m_VertexShader;
	ID3D11PixelShader* m_PixelShader;
	ID3D11InputLayout* m_InputLayout;
public:
	BaseShader() {}
	BaseShader(HRESULT hr, WCHAR* file, ID3D11Device* device, D3D11_INPUT_ELEMENT_DESC layout[], UINT numElements);
	~BaseShader();

	HRESULT CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);

	HRESULT CreateVertexShader(HRESULT hr, WCHAR* file, ID3D11Device* device);
	HRESULT CreatePixelShader(HRESULT hr, WCHAR* file, ID3D11Device* device);

	void inline BindInputLayout(ID3D11DeviceContext* immediateContext) { immediateContext->IASetInputLayout(m_InputLayout); }

	inline ID3D11VertexShader* GetVertexShader() { return m_VertexShader; }
	inline ID3D11PixelShader* GetPixelShader() { return m_PixelShader; }
	inline ID3D11InputLayout* GetInputLayout() { return m_InputLayout; }
};

