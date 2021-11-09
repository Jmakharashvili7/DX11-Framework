#include "Application.h"
#include "Math3D.h"

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;

    switch (message)
    {
        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

Application::Application()
{
	_hInst = nullptr;
	_hWnd = nullptr;
	_driverType = D3D_DRIVER_TYPE_NULL;
	_featureLevel = D3D_FEATURE_LEVEL_11_0;
	_pd3dDevice = nullptr;
	_pImmediateContext = nullptr;
	_pSwapChain = nullptr;
	_pRenderTargetView = nullptr;
	_pVertexShader = nullptr;
	_pPixelShader = nullptr;
	_pVertexLayout = nullptr;
    _pVertexBufferSun = nullptr;
	_pIndexBufferCube = nullptr;
    _pIndexBufferPyramid = nullptr;
	_pConstantBuffer = nullptr;
}

Application::~Application()
{
	Cleanup();
}

HRESULT Application::Initialise(HINSTANCE hInstance, int nCmdShow)
{
    if (FAILED(InitWindow(hInstance, nCmdShow)))
	{
        return E_FAIL;
	}

    RECT rc;
    GetClientRect(_hWnd, &rc);
    _WindowWidth = rc.right - rc.left;
    _WindowHeight = rc.bottom - rc.top;

    if (FAILED(InitDevice()))
    {
        Cleanup();

        return E_FAIL;
    }

	// Initialize the world matrix
	XMStoreFloat4x4(&_world, XMMatrixIdentity());

    // Initialize the view matrix
	XMVECTOR Eye = XMVectorSet(0.0f, 0.0f, -3.0f, 0.0f);
	XMVECTOR At = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMStoreFloat4x4(&_view, XMMatrixLookAtLH(Eye, At, Up));

    // Initialize the projection matrix
	XMStoreFloat4x4(&_projection, XMMatrixPerspectiveFovLH(XM_PIDIV2, _WindowWidth / (FLOAT) _WindowHeight, 0.01f, 100.0f));

	return S_OK;
}

HRESULT Application::InitShadersAndInputLayout()
{
	HRESULT hr;

    // Compile the vertex shader
    ID3DBlob* pVSBlob = nullptr;
    hr = CompileShaderFromFile(L"DX11 Framework.fx", "VS", "vs_4_0", &pVSBlob);

    if (FAILED(hr))
    {
        MessageBox(nullptr,
                   L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

	// Create the vertex shader
	hr = _pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &_pVertexShader);

	if (FAILED(hr))
	{	
		pVSBlob->Release();
        return hr;
	}

	// Compile the pixel shader
	ID3DBlob* pPSBlob = nullptr;
    hr = CompileShaderFromFile(L"DX11 Framework.fx", "PS", "ps_4_0", &pPSBlob);

    if (FAILED(hr))
    {
        MessageBox(nullptr,
                   L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

	// Create the pixel shader
	hr = _pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &_pPixelShader);
	pPSBlob->Release();

    if (FAILED(hr))
        return hr;

    // Define the input layout
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	UINT numElements = ARRAYSIZE(layout);

    // Create the input layout
	hr = _pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
                                        pVSBlob->GetBufferSize(), &_pVertexLayout);
	pVSBlob->Release();

	if (FAILED(hr))
        return hr;

    // Set the input layout
    _pImmediateContext->IASetInputLayout(_pVertexLayout);

	return hr;
}

HRESULT Application::InitVertexBuffer()
{
    return S_OK;
}

HRESULT Application::InitObjects()
{
	HRESULT hr; // stands for hex result

    //
    // Create vertex buffer for sun
    //

    SimpleVertex vertices[] =
    {
        { XMFLOAT3( -0.25f, 0.5f, -0.25f ) }, // 0
        { XMFLOAT3( -0.25f, 0.5f,  0.25f ) }, // 1
        { XMFLOAT3(  0.25f, 0.5f,  0.25f ) }, // 2
        { XMFLOAT3(  0.25f, 0.5f, -0.25f ) }, // 3
		{ XMFLOAT3( -0.25f, 0.0f, -0.25f ) }, // 4
		{ XMFLOAT3( -0.25f, 0.0f,  0.25f ) }, // 5
        { XMFLOAT3(  0.25f, 0.0f,  0.25f ) }, // 6
        { XMFLOAT3(  0.25f, 0.0f, -0.25f ) }, // 7
    };

    
    // Create index buffer
    UINT indicesCube[] =
    {
        0, 1, 2, 0, 2, 3, // Top
		0, 4, 5, 0, 5, 1, // Bottom
		1, 5, 6, 1, 6, 2, // Left
		2, 6, 7, 2, 7, 3, // Right
		3, 7, 4, 3, 4, 0, // Front
		4, 7, 6, 4, 6, 5  // Back
    };

    vertices[0].normal = { };
    vertices[1].normal = { };
    vertices[2].normal = { };
    vertices[3].normal = { };
    vertices[4].normal = { };
    vertices[5].normal = { };
    vertices[6].normal = { };
    vertices[7].normal = { };

    D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(SimpleVertex) * 8;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = vertices;

    // Create the buffer and check for any errors
    hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pVertexBufferSun);
    if (FAILED(hr))
        return hr;

    //
    // Vertex buffer for Mars
    //

	SimpleVertex verticesMars[] =
    {
        { XMFLOAT3( -0.25f, 0.5f, -0.25f ) }, // 0
        { XMFLOAT3( -0.25f, 0.5f,  0.25f ) }, // 1
        { XMFLOAT3(  0.25f, 0.5f,  0.25f ) }, // 2
        { XMFLOAT3(  0.25f, 0.5f, -0.25f ) }, // 3
		{ XMFLOAT3( -0.25f, 0.0f, -0.25f ) }, // 4
		{ XMFLOAT3( -0.25f, 0.0f,  0.25f ) }, // 5
        { XMFLOAT3(  0.25f, 0.0f,  0.25f ) }, // 6
        { XMFLOAT3(  0.25f, 0.0f, -0.25f ) }, // 7
    };

	Math3D::NormalAvarage(verticesMars, indicesCube, 12);

	ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(SimpleVertex) * 8;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = verticesMars;

    // Create the buffer and check for any errors
    hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pVertexBufferMars);
    if (FAILED(hr))
        return hr;

	//
    // Vertex buffer for Earth
    //
    SimpleVertex verticesEarth[] =
    {
        { XMFLOAT3( -0.25f, 0.5f, -0.25f ) }, // 0
        { XMFLOAT3( -0.25f, 0.5f,  0.25f ) }, // 1
        { XMFLOAT3(  0.25f, 0.5f,  0.25f ) }, // 2
        { XMFLOAT3(  0.25f, 0.5f, -0.25f ) }, // 3
		{ XMFLOAT3( -0.25f, 0.0f, -0.25f ) }, // 4
		{ XMFLOAT3( -0.25f, 0.0f,  0.25f ) }, // 5
        { XMFLOAT3(  0.25f, 0.0f,  0.25f ) }, // 6
        { XMFLOAT3(  0.25f, 0.0f, -0.25f ) }, // 7
    };

	Math3D::NormalAvarage(verticesEarth, indicesCube, 12);

	ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(SimpleVertex) * 8;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = verticesEarth;

    // Create the buffer and check for any errors
    hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pVertexBufferEarth);
    if (FAILED(hr))
        return hr;

	//
    // Vertex buffer for moons
    //
    SimpleVertex verticesMoon[] =
    {
        { XMFLOAT3( -0.25f, 0.5f, -0.25f ) }, // 0
        { XMFLOAT3( -0.25f, 0.5f,  0.25f ) }, // 1
        { XMFLOAT3(  0.25f, 0.5f,  0.25f ) }, // 2
        { XMFLOAT3(  0.25f, 0.5f, -0.25f ) }, // 3
		{ XMFLOAT3( -0.25f, 0.0f, -0.25f ) }, // 4
		{ XMFLOAT3( -0.25f, 0.0f,  0.25f ) }, // 5
        { XMFLOAT3(  0.25f, 0.0f,  0.25f ) }, // 6
        { XMFLOAT3(  0.25f, 0.0f, -0.25f ) }, // 7
    };

	Math3D::NormalAvarage(verticesMoon, indicesCube, 12);

	ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(SimpleVertex) * 8;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = verticesMoon;

    // Create the buffer and check for any errors
    hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pVertexBufferMoon);
    if (FAILED(hr))
        return hr;

    //
    // Vertex buffer for the pyramid
    //
    SimpleVertex verticesPyramid[] =
    {
        {XMFLOAT3( 0.5f, 0.0f, -0.5f) }, // 0
        {XMFLOAT3( 0.5f, 0.0f,  0.5f) }, // 1
        {XMFLOAT3(-0.5f, 0.0f, -0.5f) }, // 2
        {XMFLOAT3(-0.5f, 0.0f,  0.5f) }, // 3
        {XMFLOAT3( 0.0f, 0.5f,  0.0f) }, // 4 This is the tip
    };

	UINT indicesPyramid[] =
    {
        3, 0, 1,
        3, 2, 0,
        0, 4, 1,
        0, 2, 4,
        4, 3, 1,
        3, 4, 2
    };

    Math3D::NormalAvarage(verticesMoon, indicesPyramid, 6);

    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(SimpleVertex) * 5;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;

    ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = verticesPyramid;

    // Create the buffer and check for any errors
    hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pVertexBufferPyramid);
    if (FAILED(hr))
        return hr;

    ///
    /// Initialize the Index buffers
    ///

	ZeroMemory(&bd, sizeof(bd));

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(UINT) * 36;     
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;

	ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = indicesCube;
    hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pIndexBufferCube);

    if (FAILED(hr))
        return hr;

    ZeroMemory(&bd, sizeof(bd));

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(UINT) * 18;
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = 0;

    ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = indicesPyramid;
    hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pIndexBufferPyramid);

    if (FAILED(hr))
        return hr;

    // Light direction from surface (XYZ)
	_lightDirection = XMFLOAT3(0.25f, 0.5f, -1.0f);
    _cb.LightVecW = _lightDirection;

    // Diffuse material properties (RGBA)
    _diffuseMaterial = XMFLOAT4(0.8f, 0.5f, 0.5f, 1.0f);
    _cb.DiffuseMtrl = _diffuseMaterial;

    // Diffuse light color (RGBA)
    _diffuseLight = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    _cb.DiffuseLight = _diffuseLight;

	return S_OK;
}

HRESULT Application::InitIndexBuffer()
{
        return S_OK;
}

HRESULT Application::InitWindow(HINSTANCE hInstance, int nCmdShow)
{
    // Register class
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_TUTORIAL1);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW );
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = L"TutorialWindowClass";
    wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_TUTORIAL1);
    if (!RegisterClassEx(&wcex))
        return E_FAIL;

    // Create window
    _hInst = hInstance;
    RECT rc = {0, 0, 640, 480};
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    _hWnd = CreateWindow(L"TutorialWindowClass", L"DX11 Framework", WS_OVERLAPPEDWINDOW,
                         CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
                         nullptr);
    if (!_hWnd)
		return E_FAIL;

    ShowWindow(_hWnd, nCmdShow);

    return S_OK;
}

HRESULT Application::CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
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

HRESULT Application::InitDevice()
{
    HRESULT hr = S_OK;

    UINT createDeviceFlags = 0;

#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };

    UINT numDriverTypes = ARRAYSIZE(driverTypes);

    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };

	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

    // setup the buffer
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 1;
    sd.BufferDesc.Width = _WindowWidth;
    sd.BufferDesc.Height = _WindowHeight;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = _hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;

    for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
    {
        _driverType = driverTypes[driverTypeIndex];
        hr = D3D11CreateDeviceAndSwapChain(nullptr, _driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
                                           D3D11_SDK_VERSION, &sd, &_pSwapChain, &_pd3dDevice, &_featureLevel, &_pImmediateContext);
        if (SUCCEEDED(hr))
            break;
    }

    if (FAILED(hr))
        return hr;

    // Create a render target view
    ID3D11Texture2D* pBackBuffer = nullptr;
    hr = _pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

    if (FAILED(hr))
        return hr;

    hr = _pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &_pRenderTargetView);
    pBackBuffer->Release();

    if (FAILED(hr))
    return hr;

    //
    // Setup the depth buffer/stencil 
    //
	D3D11_TEXTURE2D_DESC depthStencilDesc;

	depthStencilDesc.Width = _WindowWidth;
	depthStencilDesc.Height = _WindowHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0; 

    hr = _pd3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, &_depthStencilBuffer);
    if (FAILED(hr))
        return hr;

	hr = _pd3dDevice->CreateDepthStencilView(_depthStencilBuffer, nullptr, &_depthStencilView);
    if (FAILED(hr))
        return hr;

    _pImmediateContext->OMSetRenderTargets(1, &_pRenderTargetView, _depthStencilView);

    // Setup the viewport
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)_WindowWidth;
    vp.Height = (FLOAT)_WindowHeight;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    _pImmediateContext->RSSetViewports(1, &vp);

	InitShadersAndInputLayout();

	InitObjects();

    // Set primitive topology
    _pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Create the constant buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
    hr = _pd3dDevice->CreateBuffer(&bd, nullptr, &_pConstantBuffer);

    if (FAILED(hr))
        return hr;

    // Setup rasterizer for wire frame
    D3D11_RASTERIZER_DESC wfdesc;
	ZeroMemory(&wfdesc, sizeof(D3D11_RASTERIZER_DESC));
	wfdesc.FillMode = D3D11_FILL_WIREFRAME;
	wfdesc.CullMode = D3D11_CULL_NONE;
	hr = _pd3dDevice->CreateRasterizerState(&wfdesc, &_wireFrame);

    return S_OK;
}

void Application::Cleanup()
{
    if (_pImmediateContext) _pImmediateContext->ClearState();

    if (_pConstantBuffer) _pConstantBuffer->Release();
    if (_pVertexBufferSun) _pVertexBufferSun->Release();
    if (_pVertexBufferMars) _pVertexBufferMars->Release();
    if (_pVertexBufferEarth) _pVertexBufferEarth->Release();
    if (_pVertexBufferMoon) _pVertexBufferMoon->Release();
    if (_pIndexBufferCube) _pIndexBufferCube->Release();
    if (_pIndexBufferPyramid) _pIndexBufferPyramid->Release();

    if (_pVertexLayout) _pVertexLayout->Release();
    if (_pVertexShader) _pVertexShader->Release();
    if (_pPixelShader) _pPixelShader->Release();

	if (_pRenderTargetView) _pRenderTargetView->Release();
    if (_pSwapChain) _pSwapChain->Release();
    if (_pImmediateContext) _pImmediateContext->Release();
	if (_pd3dDevice) _pd3dDevice->Release();
    if (_wireFrame) _wireFrame->Release();
	if (_depthStencilView) _depthStencilView->Release();
	if (_depthStencilBuffer) _depthStencilBuffer->Release();
}

void Application::Update()
{
    // Update our time
    static float t = 0.0f;
    static float t2 = 0.0f;

    if (_driverType == D3D_DRIVER_TYPE_REFERENCE)
    {
        t += (float) XM_PI * 0.0125f;
        t2 += (float) XM_PI * 0.0125f;
    }
    else
    {
        static DWORD dwTimeStart = 0;
        DWORD dwTimeCur = GetTickCount();

        if (dwTimeStart == 0)
            dwTimeStart = dwTimeCur;

        t =  (dwTimeCur - dwTimeStart) / 3000.0f;
        t2 = (dwTimeCur - dwTimeStart) / 2000.0f;

        
        _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &_cb, 0, 0);
    }
	
    HandleInput();

    //
    // Animate the sun
    //
	XMStoreFloat4x4(&_world, XMMatrixScaling(2.0f, 2.0f, 2.0f) * XMMatrixRotationY(t2) *
        XMMatrixTranslation(0.0f, -0.25f, 0.0f));

    //
    // Animate mars
    //
    XMStoreFloat4x4(&_worldMars, XMMatrixScaling(0.75f, 0.75f, 0.75f) * XMMatrixRotationY(t) * 
        XMMatrixTranslation(1.3f, 0.0f, 0.0f) * XMMatrixRotationY(t));
    //
    // Animate earth
    //
	XMStoreFloat4x4(&_worldEarth, XMMatrixScaling(0.5f, 0.5f, 0.5f) * XMMatrixRotationY(t) * 
        XMMatrixTranslation(2.25f, 0.05f, 0.0f) * XMMatrixRotationY(t));

    //
    // Animate the moons
    //

    // Moon for Earth
    XMStoreFloat4x4(&_worldMoonEarth, XMMatrixScaling(0.1, 0.1, 0.1) * XMMatrixRotationY(t) * XMMatrixTranslation(2.25f, 0.05f, 0.0f) * 
        XMMatrixRotationY(t) * XMMatrixTranslation(0.2f, 0.1f, 0.0f));

    // Moon for Mars
    XMStoreFloat4x4(&_worldMoonMars, XMMatrixScaling(0.1, 0.1, 0.1) * XMMatrixRotationY(t) * XMMatrixTranslation(1.3f, 0.0f, 0.0f) * 
        XMMatrixRotationY(t) * XMMatrixTranslation(0.25f, 0.2f, 0.0f));

    // Animate the pyramid
    XMStoreFloat4x4(&_worldPyramid, XMMatrixScaling(1.0f, 1.0f, 1.0f) * XMMatrixTranslation(0.0f, -1.0f, 0.0f));
}

void Application::Draw()
{
    //
    // Clear the back buffer
    //
    float ClearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f}; // red,green,blue,alpha   
    _pImmediateContext->ClearRenderTargetView(_pRenderTargetView, ClearColor);

    //
    // Clear the depth/stencil view
    //
    _pImmediateContext->ClearDepthStencilView(_depthStencilView, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);

    //
    // Setup the transformation matrices
    //
	XMMATRIX world = XMLoadFloat4x4(&_world);
	XMMATRIX view = XMLoadFloat4x4(&_view);
	XMMATRIX projection = XMLoadFloat4x4(&_projection);

    //
    //   Update variables
    //
	_cb.mWorld = XMMatrixTranspose(world);
	_cb.mView = XMMatrixTranspose(view);
	_cb.mProjection = XMMatrixTranspose(projection);
	_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &_cb, 0, 0);

    //
    // Renders the first object
    //
    UINT stride = sizeof(SimpleVertex);
    UINT offset = 0;
    _pImmediateContext->IASetVertexBuffers(0, 1, &_pVertexBufferSun, &stride, &offset);
    _pImmediateContext->IASetIndexBuffer(_pIndexBufferCube, DXGI_FORMAT_R16_UINT, 0);
	_pImmediateContext->VSSetShader(_pVertexShader, nullptr, 0);
	_pImmediateContext->VSSetConstantBuffers(0, 1, &_pConstantBuffer);
    _pImmediateContext->PSSetConstantBuffers(0, 1, &_pConstantBuffer);
	_pImmediateContext->PSSetShader(_pPixelShader, nullptr, 0);
	_pImmediateContext->DrawIndexed(36, 0, 0);        

    //
    // Draw the second Object
    //
    world = XMLoadFloat4x4(&_worldMars);
    _cb.mWorld = XMMatrixTranspose(world);
    _pImmediateContext->IASetVertexBuffers(0, 1, &_pVertexBufferMars, &stride, &offset);
	_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &_cb, 0, 0);
    _pImmediateContext->DrawIndexed(36, 0, 0);

    //
    // Draw the third Object
    //
    world = XMLoadFloat4x4(&_worldEarth);
    _cb.mWorld = XMMatrixTranspose(world);
    _pImmediateContext->IASetVertexBuffers(0, 1, &_pVertexBufferEarth, &stride, &offset);
    _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &_cb, 0, 0);
    _pImmediateContext->DrawIndexed(36, 0, 0);

    //
    // Draw the moons
    //

    // Moon for Earth
    world = XMLoadFloat4x4(&_worldMoonEarth);
    _cb.mWorld = XMMatrixTranspose(world);

    _pImmediateContext->IASetVertexBuffers(0, 1, &_pVertexBufferMoon, &stride, &offset);
    _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &_cb, 0, 0);
    _pImmediateContext->DrawIndexed(36, 0, 0);

    // Moon for Mars
	world = XMLoadFloat4x4(&_worldMoonMars);
    _cb.mWorld = XMMatrixTranspose(world);

    _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &_cb, 0, 0);
    _pImmediateContext->DrawIndexed(36, 0, 0);

    //
    // Draw the pyramid
    //
    world = XMLoadFloat4x4(&_worldPyramid);
    _cb.mWorld = XMMatrixTranspose(world);

    _pImmediateContext->IASetVertexBuffers(0, 1, &_pVertexBufferPyramid, &stride, &offset);
    _pImmediateContext->IASetIndexBuffer(_pIndexBufferPyramid, DXGI_FORMAT_R16_UINT, 0);
    _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &_cb, 0, 0);
    _pImmediateContext->DrawIndexed(18, 0, 0);

    //
    // Present our back buffer to our front buffer
    //
    _pSwapChain->Present(0, 0);
}

void Application::HandleInput()
{
	if(GetAsyncKeyState(VK_F1))
    {
	    _pImmediateContext->RSSetState(_wireFrame); 
    }
    if(GetAsyncKeyState(VK_F2))
    {
	    _pImmediateContext->RSSetState(nullptr); 
    }
}