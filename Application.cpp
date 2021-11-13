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
	m_hInst = nullptr;
	m_hWnd = nullptr;
	m_driverType = D3D_DRIVER_TYPE_NULL;
	m_featureLevel = D3D_FEATURE_LEVEL_11_0;
	m_pd3dDevice = nullptr;
	m_ImmediateContext = nullptr;
	m_SwapChain = nullptr;
	m_RenderTargetView = nullptr;
	m_VertexShader = nullptr;
	m_PixelShader = nullptr;
	m_VertexLayout = nullptr;
    m_Sun = nullptr;
	m_Mars = nullptr;
    m_Earth = nullptr;
    m_MoonMars = nullptr;
    m_MoonEarth = nullptr;
    m_Pyramid = nullptr;
	m_ConstantBuffer = nullptr;
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
    GetClientRect(m_hWnd, &rc);
    _WindowWidth = rc.right - rc.left;
    _WindowHeight = rc.bottom - rc.top;

    if (FAILED(InitDevice()))
    {
        Cleanup();

        return E_FAIL;
    }

	// Initialize the world matrix
	XMStoreFloat4x4(&m_world, XMMatrixIdentity());

    // Initialize the view matrix
	XMVECTOR Eye = XMVectorSet(0.0f, 0.0f, -3.0f, 0.0f);
	XMVECTOR At = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMStoreFloat4x4(&m_view, XMMatrixLookAtLH(Eye, At, Up));

    // Initialize the projection matrix
	XMStoreFloat4x4(&m_projection, XMMatrixPerspectiveFovLH(XM_PIDIV2, _WindowWidth / (FLOAT) _WindowHeight, 0.01f, 100.0f));

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
	hr = m_pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &m_VertexShader);

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
	hr = m_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_PixelShader);
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
	hr = m_pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
                                        pVSBlob->GetBufferSize(), &m_VertexLayout);
	pVSBlob->Release();

	if (FAILED(hr))
        return hr;

    // Set the input layout
    m_ImmediateContext->IASetInputLayout(m_VertexLayout);

	return hr;
}

HRESULT Application::InitObjects()
{
	HRESULT hr = S_OK; // stands for hex result

    //
    // Create vertex buffer for sun
    //

    SimpleVertex vertices[] =
    {
        { XMFLOAT3( -0.25f, 0.5f, -0.25f ),XMFLOAT3(0,0,0) }, // 0
        { XMFLOAT3( -0.25f, 0.5f,  0.25f ),XMFLOAT3(0,0,0) }, // 1
        { XMFLOAT3(  0.25f, 0.5f,  0.25f ),XMFLOAT3(0,0,0) }, // 2
        { XMFLOAT3(  0.25f, 0.5f, -0.25f ),XMFLOAT3(0,0,0) }, // 3
		{ XMFLOAT3( -0.25f, 0.0f, -0.25f ),XMFLOAT3(0,0,0) }, // 4
		{ XMFLOAT3( -0.25f, 0.0f,  0.25f ),XMFLOAT3(0,0,0) }, // 5
        { XMFLOAT3(  0.25f, 0.0f,  0.25f ),XMFLOAT3(0,0,0) }, // 6
        { XMFLOAT3(  0.25f, 0.0f, -0.25f ),XMFLOAT3(0,0,0) }, // 7
    };

    
    // Create index buffer
    WORD indicesCube[] =
    {
        0, 1, 2, 0, 2, 3, // Top
		0, 4, 5, 0, 5, 1, // Bottom
		1, 5, 6, 1, 6, 2, // Left
		2, 6, 7, 2, 7, 3, // Right
		3, 7, 4, 3, 4, 0, // Front
		4, 7, 6, 4, 6, 5  // Back
    };

    m_Sun = new BaseObject(m_pd3dDevice, vertices, indicesCube, hr, 36, 8);

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
    
    m_MoonEarth = new BaseObject(m_pd3dDevice, verticesMoon, indicesCube, hr, 36, 8);
    m_MoonMars = new BaseObject(m_pd3dDevice, verticesMoon, indicesCube, hr, 36, 8);

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

    m_Mars = new BaseObject(m_pd3dDevice, verticesMars, indicesCube, hr, 36, 8);

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

	m_Earth = new BaseObject(m_pd3dDevice, verticesEarth, indicesCube, hr, 36, 8);

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

	WORD indicesPyramid[] =
    {
        3, 0, 1,
        3, 2, 0,
        0, 4, 1,
        0, 2, 4,
        4, 3, 1,
        3, 4, 2
    };

    m_Pyramid = new BaseObject(m_pd3dDevice, verticesPyramid, indicesPyramid, hr, 18, 5);

    if (FAILED(hr))
        return hr;

	return S_OK;
}

void Application::InitLights()
{
    // Light direction from surface (XYZ)
	m_LightDirection = XMFLOAT3(1, 1.0f, -2.0f);
    m_cb.LightVecW = m_LightDirection;

    // Diffuse material properties (RGBA)
    m_DiffuseMaterial = XMFLOAT4(0.8f, 0.5f, 0.5f, 1.0f);
    m_cb.DiffuseMtrl = m_DiffuseMaterial;

    // Diffuse light color (RGBA)
    m_DiffuseLight = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    m_cb.DiffuseLight = m_DiffuseLight;
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
    m_hInst = hInstance;
    RECT rc = {0, 0, 640, 480};
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    m_hWnd = CreateWindow(L"TutorialWindowClass", L"DX11 Framework", WS_OVERLAPPEDWINDOW,
                         CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
                         nullptr);
    if (!m_hWnd)
		return E_FAIL;

    ShowWindow(m_hWnd, nCmdShow);

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
    sd.OutputWindow = m_hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;

    for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
    {
        m_driverType = driverTypes[driverTypeIndex];
        hr = D3D11CreateDeviceAndSwapChain(nullptr, m_driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
                                           D3D11_SDK_VERSION, &sd, &m_SwapChain, &m_pd3dDevice, &m_featureLevel, &m_ImmediateContext);
        if (SUCCEEDED(hr))
            break;
    }

    if (FAILED(hr))
        return hr;

    // Create a render target view
    ID3D11Texture2D* pBackBuffer = nullptr;
    hr = m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

    if (FAILED(hr))
        return hr;

    hr = m_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &m_RenderTargetView);
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

    hr = m_pd3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, &m_DepthStencilBuffer);
    if (FAILED(hr))
        return hr;

	hr = m_pd3dDevice->CreateDepthStencilView(m_DepthStencilBuffer, nullptr, &m_DepthStencilView);
    if (FAILED(hr))
        return hr;

    m_ImmediateContext->OMSetRenderTargets(1, &m_RenderTargetView, m_DepthStencilView);

    // Setup the viewport
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)_WindowWidth;
    vp.Height = (FLOAT)_WindowHeight;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    m_ImmediateContext->RSSetViewports(1, &vp);

	InitShadersAndInputLayout();

	InitObjects();
    InitLights();

    // Set primitive topology
    m_ImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Create the constant buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
    hr = m_pd3dDevice->CreateBuffer(&bd, nullptr, &m_ConstantBuffer);

    if (FAILED(hr))
        return hr;

    // Setup rasterizer for wire frame
    D3D11_RASTERIZER_DESC wfdesc;
	ZeroMemory(&wfdesc, sizeof(D3D11_RASTERIZER_DESC));
	wfdesc.FillMode = D3D11_FILL_WIREFRAME;
	wfdesc.CullMode = D3D11_CULL_NONE;
	hr = m_pd3dDevice->CreateRasterizerState(&wfdesc, &m_WireFrame);

    return S_OK;
}

void Application::Cleanup()
{
    if (m_ImmediateContext) m_ImmediateContext->ClearState();

    if (m_ConstantBuffer) m_ConstantBuffer->Release();

    // Clean up the planets
    /*if (m_Sun) delete m_Sun;
    if (m_Mars) delete m_Mars;
    if (m_Earth) delete m_Earth;
    if (m_MoonEarth) delete m_MoonEarth;
    if (m_MoonMars) delete m_MoonMars;
    if (m_Pyramid) delete m_Pyramid;*/

    if (m_VertexLayout) m_VertexLayout->Release();
    if (m_VertexShader) m_VertexShader->Release();
    if (m_PixelShader) m_PixelShader->Release();

	if (m_RenderTargetView) m_RenderTargetView->Release();
    if (m_SwapChain) m_SwapChain->Release();
    if (m_ImmediateContext) m_ImmediateContext->Release();
	if (m_pd3dDevice) m_pd3dDevice->Release();
    if (m_WireFrame) m_WireFrame->Release();
	if (m_DepthStencilView) m_DepthStencilView->Release();
	if (m_DepthStencilBuffer) m_DepthStencilBuffer->Release();
}

void Application::Update()
{
    // Update our time
    static float t = 0.0f;
    static float t2 = 0.0f;

    if (m_driverType == D3D_DRIVER_TYPE_REFERENCE)
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

        
        m_ImmediateContext->UpdateSubresource(m_ConstantBuffer, 0, nullptr, &m_cb, 0, 0);
    }
	
    HandleInput();

    //
    // Animate Sun
    //
    XMStoreFloat4x4(&m_Sun->m_world, XMMatrixScaling(2.0f, 2.0f, 2.0f) * XMMatrixRotationY(t2) *
        XMMatrixTranslation(0.0f, -0.25f, 0.0f));

    //
    // Animate Mars
    //
    XMStoreFloat4x4(&m_Mars->m_world, XMMatrixScaling(0.75f, 0.75f, 0.75f) * XMMatrixRotationY(t) * 
        XMMatrixTranslation(1.3f, 0.0f, 0.0f) * XMMatrixRotationY(t));
    //
    // Animate Earth
    //
	XMStoreFloat4x4(&m_Earth->m_world, XMMatrixScaling(0.5f, 0.5f, 0.5f) * XMMatrixRotationY(t) * 
        XMMatrixTranslation(2.25f, 0.05f, 0.0f) * XMMatrixRotationY(t));

    // Moon for Earth
    XMStoreFloat4x4(&m_MoonEarth->m_world, XMMatrixScaling(0.1, 0.1, 0.1) * XMMatrixRotationY(t) * XMMatrixTranslation(2.25f, 0.05f, 0.0f) * 
        XMMatrixRotationY(t) * XMMatrixTranslation(0.2f, 0.1f, 0.0f));

    // Moon for Mars
    XMStoreFloat4x4(&m_MoonMars->m_world, XMMatrixScaling(0.1, 0.1, 0.1) * XMMatrixRotationY(t) * XMMatrixTranslation(1.3f, 0.0f, 0.0f) * 
        XMMatrixRotationY(t) * XMMatrixTranslation(0.25f, 0.2f, 0.0f));

    // Animate the pyramid
    XMStoreFloat4x4(&m_Pyramid->m_world, XMMatrixScaling(1.0f, 1.0f, 1.0f) * XMMatrixTranslation(0.0f, -1.0f, 0.0f));
}

void Application::Draw()
{
    //
    // Clear the back buffer
    //
    float ClearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f}; // red,green,blue,alpha   
    m_ImmediateContext->ClearRenderTargetView(m_RenderTargetView, ClearColor);

    //
    // Clear the depth/stencil view
    //
    m_ImmediateContext->ClearDepthStencilView(m_DepthStencilView, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);

    //
    // Setup the transformation matrices
    //
	XMMATRIX world = XMLoadFloat4x4(&m_world);
	XMMATRIX view = XMLoadFloat4x4(&m_view);
	XMMATRIX projection = XMLoadFloat4x4(&m_projection);

    //
    //   Update variables
    //
	m_cb.mWorld = XMMatrixTranspose(world);
	m_cb.mView = XMMatrixTranspose(view);
	m_cb.mProjection = XMMatrixTranspose(projection);
	m_ImmediateContext->UpdateSubresource(m_ConstantBuffer, 0, nullptr, &m_cb, 0, 0);
    
    //
    // Setup constant buffer and shaders
    //
	m_ImmediateContext->VSSetShader(m_VertexShader, nullptr, 0);
	m_ImmediateContext->VSSetConstantBuffers(0, 1, &m_ConstantBuffer);
    m_ImmediateContext->PSSetConstantBuffers(0, 1, &m_ConstantBuffer);
	m_ImmediateContext->PSSetShader(m_PixelShader, nullptr, 0);   

    //
    // Render Object
    //
    m_Sun->Render(world, m_cb, m_ConstantBuffer, m_ImmediateContext);
   
    m_MoonEarth->Render(world, m_cb, m_ConstantBuffer, m_ImmediateContext);

    m_Earth->Render(world, m_cb, m_ConstantBuffer, m_ImmediateContext);

    m_Mars->Render(world, m_cb, m_ConstantBuffer, m_ImmediateContext);

    m_Pyramid->Render(world, m_cb, m_ConstantBuffer, m_ImmediateContext);

    m_MoonMars->Render(world, m_cb, m_ConstantBuffer, m_ImmediateContext);

    //
    // Present our back buffer to our front buffer
    //
    m_SwapChain->Present(0, 0);
}

void Application::HandleInput()
{
	if(GetAsyncKeyState(VK_F1))
    {
	    m_ImmediateContext->RSSetState(m_WireFrame); 
    }
    if(GetAsyncKeyState(VK_F2))
    {
	    m_ImmediateContext->RSSetState(nullptr); 
    }
}