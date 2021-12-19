#pragma once
#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include <iostream>
#include <string>

#include "DDSTextureLoader.h"
#include "resource.h"	
#include "structs.h"
#include "BaseObject.h"
#include "BaseObjectOBJ.h"
#include "OBJLoader.h"
#include "Camera.h"
#include "BaseShader.h"
#include "KeyboardClass.h"
#include "MouseClass.h"
#include "FPCamera.h"

using namespace DirectX;

class Application
{
private:
	HINSTANCE                 m_hInst;
	HWND                      m_hWnd;
	D3D_DRIVER_TYPE           m_driverType;
	D3D_FEATURE_LEVEL         m_featureLevel;
	ID3D11Device			 *m_pd3dDevice;
	ID3D11DeviceContext      *m_pImmediateContext;
	IDXGISwapChain           *m_pSwapChain;
	ID3D11RenderTargetView   *m_pRenderTargetView;
	ID3D11RasterizerState    *m_pWireFrame;
	ID3D11InputLayout        *m_pVertexLayout;
	ID3D11Buffer			 *m_pConstantBuffer;
	ID3D11DepthStencilView   *m_pDepthStencilView;
	ID3D11Texture2D          *m_pDepthStencilBuffer;
	// Keyboard input manager
	// Shaders
	BaseShader               *m_pTemplateShader;
	// Texture Resource View
	ID3D11ShaderResourceView *m_pTextureSunRV,*m_pTextureMoonRV, *m_pTextureEarthRV, *m_pTextureMarsRV, *m_pTextureNrms;
	ID3D11SamplerState       *m_pSamplerLinear = nullptr;
	// View Matrices
	XMFLOAT4X4                m_world;
	// Lighting variables	  
	XMFLOAT3				  m_LightDirection;
	XMFLOAT4				  m_DiffuseMaterial, m_DiffuseLight;
	// Consant buffer		  
	ConstantBuffer			  m_cb;
	// Cameras
	Camera                    *m_MainCamera;
	FP_Camera                 *m_FPCamera;
	// Game objects			  
	BaseObject                *m_LegacySun, *m_LegacyMars, *m_LegacyEarth, *m_LegacyMoonEarth, *m_LegacyMoonMars, *m_LegacyPyramid;
	// OBJ Game Objects
	BaseObjectOBJ             *m_Sun, *m_Mars, *m_Moon, *m_Earth;
	bool typing = false;
private:
	HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);
	HRESULT InitDevice();
	void Cleanup();
	void InitLights();
	HRESULT InitTextures();
	HRESULT InitShadersAndInputLayout();
	HRESULT LegacyInitObjects();
	void InitCameras();
	HRESULT InitObjects();

	UINT _WindowHeight;
	UINT _WindowWidth;

public:
	Application();
	~Application();

	HRESULT Initialise(HINSTANCE hInstance, int nCmdShow);

	void Update();
	void Draw();
	void LegacyDraw();
	void HandleInput();
};

