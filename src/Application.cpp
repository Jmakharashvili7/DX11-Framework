#include "Application.h"
#include "Math3D.h"

KeyboardClass m_KeyboardClass;
MouseClass m_MouseClass;

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
        
    // Keyboard messages
    case WM_KEYDOWN:
    {
        unsigned char key = static_cast<unsigned char>(wParam);
        if (m_KeyboardClass.IsKeyAutoRepeat())
        {
            m_KeyboardClass.OnKeyPressed(key);
        }
        else
        {
            const bool wasPressed = lParam &(1<<30); // check the 30th bit to get if the key was pressed 
            if (!wasPressed)
                m_KeyboardClass.OnKeyPressed(key);
        }
        break;
    }
    case WM_KEYUP:
    {
        unsigned char key = static_cast<unsigned char>(wParam);
        m_KeyboardClass.OnKeyReleased(key);
        break;
    }
    case WM_CHAR: 
    {
        unsigned char ch = static_cast<unsigned char>(wParam);
        if (m_KeyboardClass.IsCharsAutoRepeat())
        {
            m_KeyboardClass.OnCharInput(ch);
        }
        else
        {
            const bool wasPressed = lParam &(1<<30); // check the 30th bit to get if the key was pressed 
            if (!wasPressed)
            {
                m_KeyboardClass.OnCharInput(ch);
            }
        }
        break;
    }
    // Mouse messages
    case WM_MOUSEMOVE:
    {
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);
        m_MouseClass.OnMouseMove(x, y);
        return 0;
    }
    case WM_LBUTTONDOWN:
    {
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);
        m_MouseClass.OnLeftClick(x, y);
        return 0;
    }
    case WM_LBUTTONUP:
    {
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);
        m_MouseClass.OnLeftRelease(x, y);
        return 0;
    }
    case WM_RBUTTONDOWN:
    {
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);
        m_MouseClass.OnRightClick(x, y);
        return 0;
    }
    case WM_RBUTTONUP:
    {
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);
        m_MouseClass.OnRightRelease(x, y);
        return 0;
    }
    case WM_MBUTTONDOWN:
    {
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);
        m_MouseClass.OnScrollClick(x, y);
        return 0;
    }
    case WM_MBUTTONUP:
    {
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);
        m_MouseClass.OnScrollRelease(x, y);
        return 0;
    }
    case WM_MOUSEWHEEL:
    {
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);
        if (GET_WHEEL_DELTA_WPARAM(wParam) > 0)
        {
            m_MouseClass.OnScrollUp(x, y);
        }
        else if (GET_WHEEL_DELTA_WPARAM(wParam) < 0)
        {
            m_MouseClass.OnScrollDown(x, y);
        }
        break;
    }
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

Application::Application()
{
    // devices or rendering
    m_hInst = nullptr;
    m_hWnd = nullptr;
    m_driverType = D3D_DRIVER_TYPE_NULL;
    m_featureLevel = D3D_FEATURE_LEVEL_11_0;
    m_pd3dDevice = nullptr;
    m_pImmediateContext = nullptr;
    m_pSwapChain = nullptr;
    m_pRenderTargetView = nullptr;
    m_pVertexLayout = nullptr;
    // legacy planets
    m_LegacySun = nullptr;
    m_LegacyMars = nullptr;
    m_LegacyEarth = nullptr;
    m_LegacyMoonMars = nullptr;
    m_LegacyMoonEarth = nullptr;
    m_LegacyPyramid = nullptr;
    // constant buffers
    m_pConstantBuffer = nullptr;
    // textures
    m_pTextureNrms = nullptr;
    m_pTextureSunRV = nullptr;
    m_pTextureMoonRV = nullptr;
    // objs
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

    InitCameras();

    // Create the sample state
    D3D11_SAMPLER_DESC sampDesc;

    ZeroMemory(&sampDesc, sizeof(sampDesc));

    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

    m_pd3dDevice->CreateSamplerState(&sampDesc, &m_pSamplerLinear);

    return S_OK;
}

HRESULT Application::InitShadersAndInputLayout()
{
    HRESULT hr = S_OK;

    // Define the input layout
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    
    UINT numElements = ARRAYSIZE(layout);

    m_pTemplateShader = new BaseShader(hr, L"Shaders/DX11 Framework.fx", m_pd3dDevice, layout, numElements);
    m_pTemplateShader->BindInputLayout(m_pImmediateContext);

    return hr;
}

//HRESULT Application::LegacyInitObjects()
//{
//    HRESULT hr = S_OK; // stands for hex result
//
//    //
//    // Create vertex buffer for sun
//    //
//
//    SimpleVertex vertices[] =
//    {
//        { XMFLOAT3(-0.25f, 0.5f, -0.25f), XMFLOAT3(0,0,0), XMFLOAT2(0, 0) }, // 0
//        { XMFLOAT3(-0.25f, 0.5f,  0.25f), XMFLOAT3(0,0,0), XMFLOAT2(0, 0) }, // 1
//        { XMFLOAT3(0.25f, 0.5f,  0.25f), XMFLOAT3(0,0,0), XMFLOAT2(0, 0) }, // 2
//        { XMFLOAT3(0.25f, 0.5f, -0.25f), XMFLOAT3(0,0,0), XMFLOAT2(0, 0) }, // 3
//        { XMFLOAT3(-0.25f, 0.0f, -0.25f), XMFLOAT3(0,0,0), XMFLOAT2(1, 1) }, // 4
//        { XMFLOAT3(-0.25f, 0.0f,  0.25f), XMFLOAT3(0,0,0), XMFLOAT2(1, 1) }, // 5
//        { XMFLOAT3(0.25f, 0.0f,  0.25f), XMFLOAT3(0,0,0), XMFLOAT2(1, 1) }, // 6
//        { XMFLOAT3(0.25f, 0.0f, -0.25f), XMFLOAT3(0,0,0), XMFLOAT2(1, 1) }, // 7
//    };
//    WORD indicesCube[] =
//    {
//        0, 1, 2, 0, 2, 3, // Top
//        0, 4, 5, 0, 5, 1, // Bottom
//        1, 5, 6, 1, 6, 2, // Left
//        2, 6, 7, 2, 7, 3, // Right
//        3, 7, 4, 3, 4, 0, // Front
//        4, 7, 6, 4, 6, 5  // Back 
//    };
//
//    m_LegacySun = new BaseObject(m_pd3dDevice, vertices, indicesCube, hr, 36, 8);
//
//    if (FAILED(hr))
//        return hr;
//
//    //
//    // Vertex buffer for moons
//    //
//    SimpleVertex verticesMoon[] =
//    {
//        { XMFLOAT3(-0.25f, 0.5f, -0.25f), XMFLOAT3(0,0,0), XMFLOAT2(0, 0) }, // 0
//        { XMFLOAT3(-0.25f, 0.5f,  0.25f), XMFLOAT3(0,0,0), XMFLOAT2(0, 0) }, // 1
//        { XMFLOAT3( 0.25f, 0.5f,  0.25f), XMFLOAT3(0,0,0), XMFLOAT2(0, 0) }, // 2
//        { XMFLOAT3( 0.25f, 0.5f, -0.25f), XMFLOAT3(0,0,0), XMFLOAT2(0, 0) }, // 3
//        { XMFLOAT3(-0.25f, 0.0f, -0.25f), XMFLOAT3(0,0,0), XMFLOAT2(1, 1) }, // 4
//        { XMFLOAT3(-0.25f, 0.0f,  0.25f), XMFLOAT3(0,0,0), XMFLOAT2(1, 1) }, // 5
//        { XMFLOAT3( 0.25f, 0.0f,  0.25f), XMFLOAT3(0,0,0), XMFLOAT2(1, 1) }, // 6
//        { XMFLOAT3( 0.25f, 0.0f, -0.25f), XMFLOAT3(0,0,0), XMFLOAT2(1, 1) }, // 7
//    };
//
//    m_LegacyMoonEarth = new BaseObject(m_pd3dDevice, verticesMoon, indicesCube, hr, 36, 8);
//    if (FAILED(hr))
//        return hr;
//
//    m_LegacyMoonMars = new BaseObject(m_pd3dDevice, verticesMoon, indicesCube, hr, 36, 8);
//    if (FAILED(hr))
//        return hr;
//
//    //
//    // Vertex buffer for Mars
//    //
//
//    SimpleVertex verticesMars[] =
//    {
//        { XMFLOAT3(-0.25f, 0.5f, -0.25f), XMFLOAT3(0,0,0), XMFLOAT2(0, 0) }, // 0
//        { XMFLOAT3(-0.25f, 0.5f,  0.25f), XMFLOAT3(0,0,0), XMFLOAT2(0, 0) }, // 1
//        { XMFLOAT3(0.25f, 0.5f,  0.25f), XMFLOAT3(0,0,0), XMFLOAT2(0, 0) }, // 2
//        { XMFLOAT3(0.25f, 0.5f, -0.25f), XMFLOAT3(0,0,0), XMFLOAT2(0, 0) }, // 3
//        { XMFLOAT3(-0.25f, 0.0f, -0.25f), XMFLOAT3(0,0,0), XMFLOAT2(1, 1) }, // 4
//        { XMFLOAT3(-0.25f, 0.0f,  0.25f), XMFLOAT3(0,0,0), XMFLOAT2(1, 1) }, // 5
//        { XMFLOAT3(0.25f, 0.0f,  0.25f), XMFLOAT3(0,0,0), XMFLOAT2(1, 1) }, // 6
//        { XMFLOAT3(0.25f, 0.0f, -0.25f), XMFLOAT3(0,0,0), XMFLOAT2(1, 1) }, // 7
//    };
//
//    m_LegacyMars = new BaseObject(m_pd3dDevice, verticesMars, indicesCube, hr, 36, 8);
//    if (FAILED(hr))
//        return hr;
//
//    //
//    // Vertex buffer for Earth
//    //
//    SimpleVertex verticesEarth[] =
//    {
//        { XMFLOAT3(-0.25f, 0.5f, -0.25f), XMFLOAT3(0,0,0), XMFLOAT2(0, 0) }, // 0
//        { XMFLOAT3(-0.25f, 0.5f,  0.25f), XMFLOAT3(0,0,0), XMFLOAT2(0, 0) }, // 1
//        { XMFLOAT3(0.25f, 0.5f,  0.25f), XMFLOAT3(0,0,0), XMFLOAT2(0, 0) }, // 2
//        { XMFLOAT3(0.25f, 0.5f, -0.25f), XMFLOAT3(0,0,0), XMFLOAT2(0, 0) }, // 3
//        { XMFLOAT3(-0.25f, 0.0f, -0.25f), XMFLOAT3(0,0,0), XMFLOAT2(1, 1) }, // 4
//        { XMFLOAT3(-0.25f, 0.0f,  0.25f), XMFLOAT3(0,0,0), XMFLOAT2(1, 1) }, // 5
//        { XMFLOAT3(0.25f, 0.0f,  0.25f), XMFLOAT3(0,0,0), XMFLOAT2(1, 1) }, // 6
//        { XMFLOAT3(0.25f, 0.0f, -0.25f), XMFLOAT3(0,0,0), XMFLOAT2(1, 1) }, // 7
//    };
//
//    m_LegacyEarth = new BaseObject(m_pd3dDevice, verticesEarth, indicesCube, hr, 36, 8);
//    if (FAILED(hr))
//        return hr;
//    //
//    // Vertex buffer for the pyramid
//    //
//    SimpleVertex verticesPyramid[] =
//    {
//        {XMFLOAT3(0.5f, 0.0f, -0.5f), XMFLOAT3(0, 0, 0), XMFLOAT2(1, 1) }, // 0
//        {XMFLOAT3(0.5f, 0.0f,  0.5f), XMFLOAT3(0, 0, 0), XMFLOAT2(1, 0) }, // 1
//        {XMFLOAT3(-0.5f, 0.0f, -0.5f), XMFLOAT3(0, 0, 0), XMFLOAT2(0, 1) }, // 2
//        {XMFLOAT3(-0.5f, 0.0f,  0.5f), XMFLOAT3(0, 0, 0), XMFLOAT2(1, 1) }, // 3
//        {XMFLOAT3(0.0f, 0.5f,  0.0f), XMFLOAT3(0, 0, 0), XMFLOAT2(0.5, 0.5) }, // 4 This is the tip
//    };
//
//    WORD indicesPyramid[] =
//    {
//        3, 0, 1,
//        3, 2, 0,
//        0, 4, 1,
//        0, 2, 4,
//        4, 3, 1,
//        3, 4, 2
//    };
//
//    m_LegacyPyramid = new BaseObject(m_pd3dDevice, verticesPyramid, indicesPyramid, hr, 18, 5);
//    if (FAILED(hr))
//        return hr;
//
//    return S_OK;
//}

void Application::InitCameras()
{
    // Initialize the camera variables
    XMFLOAT3 Position = { 0.0f, 0.0f,-3.0f };
    XMFLOAT3 Look = { 0.0f, 0.0f, 1.0f };
    XMFLOAT3 Up = { 0.0f, 1.0f, 0.0f };
    XMFLOAT3 Right = { 1.0f, 0.0f, 0.0f };

    //// setup the camera
    //m_MainCamera = new Camera(Position, Look, Up, Right, _WindowWidth, _WindowHeight, 0.1f, 100.0f);
    //XMStoreFloat3(&m_cb.EyePosW, m_MainCamera->GetPosition());
    //m_MainCamera->Update();

    // setup first person camera
    m_FPCamera = new FP_Camera(Position, Look, Up, Right, _WindowWidth, _WindowHeight, 0.1f, 100.0f);
    XMStoreFloat3(&m_cb.EyePosW, m_FPCamera->GetPosition());
    m_FPCamera->Update();
}

HRESULT Application::InitObjects()
{
    HRESULT hr = S_OK; // stands for hex result

    m_Sun = new BaseObjectOBJ(OBJLoader::Load("3D_Models/Blender/MoonTest.obj", m_pd3dDevice));

    if (FAILED(hr))
        return hr;

    m_Moon = new BaseObjectOBJ(OBJLoader::Load("3D_Models/Blender/MoonTest.obj", m_pd3dDevice));

    if (FAILED(hr))
    return hr;

    m_Mars = new BaseObjectOBJ(OBJLoader::Load("3D_Models/Blender/MoonTest.obj", m_pd3dDevice));

    if (FAILED(hr))
    return hr;

    m_Earth = new BaseObjectOBJ(OBJLoader::Load("3D_Models/Blender/MoonTest.obj", m_pd3dDevice));

    return hr;
}

void Application::InitLights()
{
    // Light direction from surface (XYZ)
    m_LightDirection = XMFLOAT3(1, 1.0f, -2.0f);
    m_cb.LightVecW = m_LightDirection;

    // Diffuse material properties (RGBA)
    m_DiffuseMaterial = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
    m_cb.DiffuseMtrl = m_DiffuseMaterial;

    // Diffuse light color (RGBA)
    m_DiffuseLight = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    m_cb.DiffuseLight = m_DiffuseLight;

    // Specular light info
    m_cb.SpecularMaterial = { 0.3f, 0.3f, 0.3f, 1.0f };
    m_cb.SpecularLight = { 0.3f, 0.3f, 0.3f, 1.0f };
    m_cb.SpecularPower = 3.0f;
}

HRESULT Application::InitTextures()
{
    HRESULT hr = CreateDDSTextureFromFile(m_pd3dDevice, L"Textures/SunTex.dds", nullptr, &m_pTextureSunRV);

    if (FAILED(hr))
        return hr;

    hr = CreateDDSTextureFromFile(m_pd3dDevice, L"Textures/MoonTex.dds", nullptr, &m_pTextureMoonRV);

    if (FAILED(hr))
        return hr;

    hr = CreateDDSTextureFromFile(m_pd3dDevice, L"Textures/EarthTex.dds", nullptr, &m_pTextureEarthRV);

    if (FAILED(hr))
        return hr;

    hr = CreateDDSTextureFromFile(m_pd3dDevice, L"Textures/MarsTex.dds", nullptr, &m_pTextureMarsRV);

    if (FAILED(hr))
        return hr;

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
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = L"TutorialWindowClass";
    wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_TUTORIAL1);
    if (!RegisterClassEx(&wcex))
        return E_FAIL;

    // Create window
    m_hInst = hInstance;
    RECT rc = { 0, 0, 1920, 1080 };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    m_hWnd = CreateWindow(L"TutorialWindowClass", L"DX11 Framework", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
        nullptr);
    if (!m_hWnd)
        return E_FAIL;

    ShowWindow(m_hWnd, nCmdShow);

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
            D3D11_SDK_VERSION, &sd, &m_pSwapChain, &m_pd3dDevice, &m_featureLevel, &m_pImmediateContext);
        if (SUCCEEDED(hr))
            break;
    }

    if (FAILED(hr))
        return hr;

    // Create a render target view
    ID3D11Texture2D* pBackBuffer = nullptr;
    hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

    if (FAILED(hr))
        return hr;

    hr = m_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &m_pRenderTargetView);
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

    hr = m_pd3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, &m_pDepthStencilBuffer);
    if (FAILED(hr))
        return hr;

    hr = m_pd3dDevice->CreateDepthStencilView(m_pDepthStencilBuffer, nullptr, &m_pDepthStencilView);
    if (FAILED(hr))
        return hr;

    m_pImmediateContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

    // Setup the viewport
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)_WindowWidth;
    vp.Height = (FLOAT)_WindowHeight;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    m_pImmediateContext->RSSetViewports(1, &vp);

    InitShadersAndInputLayout();

    InitObjects();

    InitLights();

    InitTextures();

    // Set primitive topology
    m_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Create the constant buffer
    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(ConstantBuffer);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    hr = m_pd3dDevice->CreateBuffer(&bd, nullptr, &m_pConstantBuffer);

    if (FAILED(hr))
        return hr;

    // Setup rasterizer for wire frame
    D3D11_RASTERIZER_DESC wfdesc;
    ZeroMemory(&wfdesc, sizeof(D3D11_RASTERIZER_DESC));
    wfdesc.FillMode = D3D11_FILL_WIREFRAME;
    wfdesc.CullMode = D3D11_CULL_NONE;
    hr = m_pd3dDevice->CreateRasterizerState(&wfdesc, &m_pWireFrame);

    return S_OK;
}

void Application::Cleanup()
{
    if (m_pImmediateContext) m_pImmediateContext->ClearState();

    if (m_pConstantBuffer) m_pConstantBuffer->Release();

    // Clean up the planets
    //if (m_Sun) delete m_Sun;
    //if (m_Mars) delete m_Mars;
    //if (m_Earth) delete m_Earth;
    //if (m_MoonEarth) delete m_MoonEarth;
    //if (m_MoonMars) delete m_MoonMars;
    //if (m_Pyramid) delete m_Pyramid;

    if (m_pRenderTargetView) m_pRenderTargetView->Release();
    if (m_pSwapChain) m_pSwapChain->Release();
    if (m_pImmediateContext) m_pImmediateContext->Release();
    if (m_pd3dDevice) m_pd3dDevice->Release();
    if (m_pWireFrame) m_pWireFrame->Release();
    if (m_pDepthStencilView) m_pDepthStencilView->Release();
    if (m_pDepthStencilBuffer) m_pDepthStencilBuffer->Release();
}

void Application::Update()
{
    HandleInput();

    // Update our time
    static float t = 0.0f;
    static float t2 = 0.0f;

    // Update the camera
    m_FPCamera->Update();

    if (m_driverType == D3D_DRIVER_TYPE_REFERENCE)
    {
        t += (float)XM_PI * 0.0125f;
        t2 += (float)XM_PI * 0.0125f;
    }
    else
    {
        static DWORD dwTimeStart = 0;
        DWORD dwTimeCur = GetTickCount();

        if (dwTimeStart == 0)
            dwTimeStart = dwTimeCur;

        t = (dwTimeCur - dwTimeStart) / 3000.0f;
        t2 = (dwTimeCur - dwTimeStart) / 2000.0f;


        m_pImmediateContext->UpdateSubresource(m_pConstantBuffer, 0, nullptr, &m_cb, 0, 0);
    }


    //
    // Animate Sun
    //
    XMStoreFloat4x4(&m_Sun->m_World, XMMatrixTranslation(0.0f, -0.25f, 0.0f) * XMMatrixRotationY(t));

    //
    // Animate Mars
    //
    XMStoreFloat4x4(&m_Mars->m_World, XMMatrixScaling(0.3f, 0.3f, 0.3f) * XMMatrixRotationY(t) *
        XMMatrixTranslation(1.7f, 0.0f, 0.0f) * XMMatrixRotationY(t));
    //
    // Animate Earth
    //
    XMStoreFloat4x4(&m_Earth->m_World, XMMatrixScaling(0.3f, 0.3f, 0.3f) * XMMatrixRotationY(t) *
        XMMatrixTranslation(3.0f, 0.05f, 0.0f) * XMMatrixRotationY(t));

    // Moon for Earth
    XMStoreFloat4x4(&m_Moon->m_World, XMMatrixScaling(0.05f, 0.05f, 0.05f) * XMMatrixRotationY(t) * XMMatrixTranslation(3.0f, 0.05f, 0.0f) *
        XMMatrixRotationY(t) * XMMatrixTranslation(0.5f, 0.1f, 0.0f));
}

void Application::Draw()
{
    //
    // Clear the back buffer
    //
    float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f }; // red,green,blue,alpha   
    m_pImmediateContext->ClearRenderTargetView(m_pRenderTargetView, ClearColor);

    //
    // Clear the depth/stencil view
    //
    m_pImmediateContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    //
    // Setup the transformation matrices
    //
    XMMATRIX world = XMLoadFloat4x4(&m_world);
    XMMATRIX view = m_FPCamera->GetViewMatrix();
    XMMATRIX projection = m_FPCamera->GetProjMatrix();

    //
    //   Update variables
    //
    m_cb.mWorld = XMMatrixTranspose(world);
    m_cb.mView = XMMatrixTranspose(view);
    m_cb.mProjection = XMMatrixTranspose(projection);
    m_pImmediateContext->UpdateSubresource(m_pConstantBuffer, 0, nullptr, &m_cb, 0, 0);

    //
    // Setup constant buffer and shaders
    //
    m_pImmediateContext->VSSetShader(m_pTemplateShader->GetVertexShader(), nullptr, 0);
    m_pImmediateContext->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
    m_pImmediateContext->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);
    m_pImmediateContext->PSSetShader(m_pTemplateShader->GetPixelShader(), nullptr, 0);
    m_pImmediateContext->PSSetSamplers(0, 1, &m_pSamplerLinear);
    m_pImmediateContext->PSSetShaderResources(0, 1, &m_pTextureSunRV);
    m_pImmediateContext->CSSetShaderResources(1, 1, &m_pTextureNrms);

    //
    // Render Object
    //
    m_Sun->Render(world, m_cb, m_pConstantBuffer, m_pImmediateContext);

    m_pImmediateContext->PSSetShaderResources(0, 1, &m_pTextureEarthRV);

    m_Earth->Render(world, m_cb, m_pConstantBuffer, m_pImmediateContext);

    m_pImmediateContext->PSSetShaderResources(0, 1, &m_pTextureMarsRV);

    m_Mars->Render(world, m_cb, m_pConstantBuffer, m_pImmediateContext);

    m_pImmediateContext->PSSetShaderResources(0, 1, &m_pTextureMoonRV);

    m_Moon->Render(world, m_cb, m_pConstantBuffer, m_pImmediateContext);

    //m_Sun->Render(world, m_cb, m_ConstantBuffer, m_ImmediateContext);
    //m_MoonEarth->Render(world, m_cb, m_ConstantBuffer, m_ImmediateContext);
    //m_Earth->Render(world, m_cb, m_ConstantBuffer, m_ImmediateContext);
    //m_Mars->Render(world, m_cb, m_ConstantBuffer, m_ImmediateContext);
    //m_Pyramid->Render(world, m_cb, m_ConstantBuffer, m_ImmediateContext);
    //m_MoonMars->Render(world, m_cb, m_ConstantBuffer, m_ImmediateContext);

    //
    // Present our back buffer to our front buffer
    //
    m_pSwapChain->Present(0, 0);
}

//void Application::LegacyDraw()
//{
//    //
//    // Clear the back buffer
//    //
//    float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f }; // red,green,blue,alpha   
//    m_pImmediateContext->ClearRenderTargetView(m_pRenderTargetView, ClearColor);
//
//    //
//    // Clear the depth/stencil view
//    //
//    m_pImmediateContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
//
//    //
//    // Setup the transformation matrices
//    //
//    XMMATRIX world = XMLoadFloat4x4(&m_world);
//    XMMATRIX view = m_MainCamera->GetViewMatrix();
//    XMMATRIX projection = m_MainCamera->GetProjMatrix();
//        
//    //
//    //   Update variables
//    //
//    m_cb.mWorld = XMMatrixTranspose(world);
//    m_cb.mView = XMMatrixTranspose(view);
//    m_cb.mProjection = XMMatrixTranspose(projection);
//    m_pImmediateContext->UpdateSubresource(m_pConstantBuffer, 0, nullptr, &m_cb, 0, 0);
//
//    //
//    // Setup constant buffer and shaders
//    //
//    m_pImmediateContext->VSSetShader(m_pTemplateShader->GetVertexShader(), nullptr, 0);
//    m_pImmediateContext->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
//    m_pImmediateContext->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);
//    m_pImmediateContext->PSSetShader(m_pTemplateShader->GetPixelShader(), nullptr, 0);
//    m_pImmediateContext->PSSetSamplers(0, 1, &m_pSamplerLinear);
//    m_pImmediateContext->PSSetShaderResources(0, 1, &m_pTextureRV);
//    m_pImmediateContext->CSSetShaderResources(1, 1, &m_pTextureNrms);
//
//    //
//    //   Update variables
//    //
//    m_cb.mWorld = XMMatrixTranspose(world);
//    m_cb.mView = XMMatrixTranspose(view);
//    m_cb.mProjection = XMMatrixTranspose(projection);
//    m_pImmediateContext->UpdateSubresource(m_pConstantBuffer, 0, nullptr, &m_cb, 0, 0);
//    m_LegacySun->Render(world, m_cb, m_ConstantBuffer, m_ImmediateContext);
//    m_LegacyMoonEarth->Render(world, m_cb, m_ConstantBuffer, m_ImmediateContext);
//    m_LegacyEarth->Render(world, m_cb, m_ConstantBuffer, m_ImmediateContext);
//    m_LegacyMars->Render(world, m_cb, m_ConstantBuffer, m_ImmediateContext);
//    m_LegacyPyramid->Render(world, m_cb, m_ConstantBuffer, m_ImmediateContext);
//    m_LegacyMoonMars->Render(world, m_cb, m_ConstantBuffer, m_ImmediateContext);
//
//    //
//    // Present our back buffer to our front buffer
//    //
//    m_pSwapChain->Present(0, 0);
//}

void Application::HandleInput()
{
    if (GetAsyncKeyState('W'))
    {
        m_FPCamera->Walk(0.001f);
    }
    if (GetAsyncKeyState('S'))
    {
        m_FPCamera->Walk(-0.001f);
    }
    if (GetAsyncKeyState('A'))
    {
        m_FPCamera->Strafe(-0.001f);
    }
    if (GetAsyncKeyState('D'))
    {
        m_FPCamera->Strafe(0.001f);
    }

    // check the key queue for any char inputs
    if (!m_KeyboardClass.CharBufferIsEmpty() && typing)
    {
        unsigned char ch = m_KeyboardClass.ReadChar(); 
        unsigned int key = m_KeyboardClass.ReadKey().GetKeyCode();

        //if (key == VK_RETURN)
        //    typing = false;

        std::string outmsg = "Char: ";
        outmsg += ch;
        outmsg += "\n";
        OutputDebugStringA(outmsg.c_str());
    }

    // check the key queue for any key inputs
    if (!m_KeyboardClass.KeyBufferIsEmpty() && !typing)
    {
        unsigned int key = m_KeyboardClass.ReadKey().GetKeyCode();
        
        // Main logic for responding to key input
        if (key == VK_F1)
        {
            m_pImmediateContext->RSSetState(m_pWireFrame);
        }
        if (key == VK_F2)
        {
            m_pImmediateContext->RSSetState(nullptr);
        }
        if (key == VK_RETURN) // enter
        {
            typing = true;
        }
    }

    if (!m_MouseClass.IsEventBufferEmpty())
    {
        MouseEvent e = m_MouseClass.ReadEvent();

        if (e.GetType() == MouseEvent::EventType::MOVE)
        {
            if (e.GetDistance.x != 0)
            {
                e.GetMoveAngle();
            }
            m_FPCamera->RotateP(e.GetMoveAngle());
            m_FPCamera->UpdateViewMatrix();
        }
    }
}