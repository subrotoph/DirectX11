
#pragma comment( lib, "user32" )          
#pragma comment( lib, "d3d11.lib" )       
#pragma comment( lib, "dxgi.lib" )        
#pragma comment( lib, "d3dcompiler.lib" ) 
//#pragma comment(lib, "d3dx11.lib")
//#pragma comment(lib, "d3dx10.lib")

#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>        
#include <d3dcompiler.h>
#include <DirectXMath.h>
//#include <d3dx11.h>
//#include <d3dx10.h>
//#include <xnamath.h>

IDXGISwapChain* SwapChain;
ID3D11Device* d3d11Device;
ID3D11DeviceContext* d3d11DevCon;
ID3D11RenderTargetView* renderTargetView;

ID3D11Buffer* vertBuffer;
ID3D11Buffer* indexBuffer;
ID3D11Buffer* uniformBuffer;
ID3D11Texture2D* depthStencilBuffer;
ID3D11DepthStencilView* depthStencilView;

ID3D11VertexShader* VS;
ID3D11PixelShader* PS;
ID3D10Blob* VS_Buffer;
ID3D10Blob* PS_Buffer;
ID3D11InputLayout* vertLayout;

LPCTSTR WndClassName = L"Triangle";
HWND hwnd = NULL;
HRESULT hr;

const float Width = 900.f;
const float Height = 600.f;

float angle;

DirectX::XMMATRIX camView;
DirectX::XMMATRIX camProjection;

DirectX::XMVECTOR camPosition;
DirectX::XMVECTOR camTarget;
DirectX::XMVECTOR camUp;

bool InitializeDirect3d11App(HINSTANCE hInstance);
bool InitScene();
void CleanUp();

void CreateVertexBuffer();
void CreateIndexBuffer();
void CreateUniformBuffer();
void CreateViewport();
void InitCamera();

int mainloop();
void UpdateScene();
void DrawScene();

bool InitializeWindow(HINSTANCE hInstance,
    int ShowWnd,
    int width, int height,
    bool windowed);

LRESULT CALLBACK WndProc(HWND hWnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam);

struct Vertex {
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT4 color;
    Vertex(
        float x, float y, float z,
        float r, float g, float b, float a
    ) : position(x, y, z), color(r, g, b, a) {}
};

struct UniformObj {
    DirectX::XMMATRIX model;
    DirectX::XMMATRIX view;
    DirectX::XMMATRIX projection;
};

UniformObj uniformObj;

D3D11_INPUT_ELEMENT_DESC layout[] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};
UINT numElements = ARRAYSIZE(layout);


bool InitScene() {
    D3DCompileFromFile(L"shader.vs", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0", 0, 0, &VS_Buffer, NULL);
    D3DCompileFromFile(L"shader.ps", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", 0, 0, &PS_Buffer, NULL);

    d3d11Device->CreateVertexShader(VS_Buffer->GetBufferPointer(), VS_Buffer->GetBufferSize(), NULL, &VS);
    d3d11Device->CreatePixelShader(PS_Buffer->GetBufferPointer(), PS_Buffer->GetBufferSize(), NULL, &PS);

    d3d11DevCon->VSSetShader(VS, 0, 0);
    d3d11DevCon->PSSetShader(PS, 0, 0);

    CreateVertexBuffer();
    CreateIndexBuffer();
    CreateUniformBuffer();
    InitCamera();
    CreateViewport();

    return true;
}

void UpdateScene() {
    angle += 0.01;


}

void DrawScene() {
    float bgColor[4] = { (0.0f, 0.0f, 0.0f, 0.0f) };
    d3d11DevCon->ClearRenderTargetView(renderTargetView, bgColor);
    d3d11DevCon->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    DirectX::XMMATRIX model = DirectX::XMMatrixIdentity();
    DirectX::XMVECTOR rotAxis = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationAxis(rotAxis, DirectX::XMConvertToRadians(angle));
    DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(0.0f, 0.0f, 2.0f);

    uniformObj.model = model * rotation * translation;
    uniformObj.view = camView;
    uniformObj.projection = camProjection;

    d3d11DevCon->UpdateSubresource(uniformBuffer, 0, NULL, &uniformObj, 0, 0);
    d3d11DevCon->VSSetConstantBuffers(0, 1, &uniformBuffer);
    d3d11DevCon->DrawIndexed(36, 0, 0);

    SwapChain->Present(0, 0);
}

void InitCamera() {
    camPosition     = DirectX::XMVectorSet(0.0f, 5.0f, -8.0f, 0.0f);
    camTarget       = DirectX::XMVectorSet(0.0f, 2.0f, 0.0f, 0.0f);
    camUp           = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    camView         = DirectX::XMMatrixLookAtLH(camPosition, camTarget, camUp);
    camProjection   = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(60.f), Width / Height, 1.0f, 1000.0f);
}

void CreateViewport() {

    D3D11_VIEWPORT viewport;
    ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = Width;
    viewport.Height = Height;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    d3d11DevCon->RSSetViewports(1, &viewport);

}

void CreateVertexBuffer() {
    Vertex vertex[] = {
        Vertex(-1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f),
        Vertex(-1.0f, +1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f),
        Vertex(+1.0f, +1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f),
        Vertex(+1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f),
        Vertex(-1.0f, -1.0f, +1.0f, 0.0f, 1.0f, 1.0f, 1.0f),
        Vertex(-1.0f, +1.0f, +1.0f, 1.0f, 1.0f, 1.0f, 1.0f),
        Vertex(+1.0f, +1.0f, +1.0f, 1.0f, 0.0f, 1.0f, 1.0f),
        Vertex(+1.0f, -1.0f, +1.0f, 1.0f, 0.0f, 0.0f, 1.0f),
    };

    D3D11_BUFFER_DESC vertexBufferDesc;
    ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.ByteWidth = sizeof(Vertex) * 8;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;
    vertexBufferDesc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA vertexBufferData;

    ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
    vertexBufferData.pSysMem = vertex;

    d3d11Device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &vertBuffer);

    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    d3d11DevCon->IASetVertexBuffers(0, 1, &vertBuffer, &stride, &offset);

    d3d11Device->CreateInputLayout(layout, numElements, VS_Buffer->GetBufferPointer(),
        VS_Buffer->GetBufferSize(), &vertLayout);

    d3d11DevCon->IASetInputLayout(vertLayout);

    d3d11DevCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void CreateIndexBuffer() {
    DWORD indices[] = {
        0, 1, 2,   0, 2, 3,
        4, 6, 5,   4, 7, 6,
        4, 5, 1,   4, 1, 0,
        3, 2, 6,   3, 6, 7,
        1, 5, 6,   1, 6, 2,
        4, 0, 3,   4, 3, 7
    };

    D3D11_BUFFER_DESC indexBufferDesc;
    ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));

    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(DWORD) * 12 * 3;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA indexBufferData;

    indexBufferData.pSysMem = indices;
    d3d11Device->CreateBuffer(&indexBufferDesc, &indexBufferData, &indexBuffer);

    d3d11DevCon->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

}

void CreateUniformBuffer() {
    D3D11_BUFFER_DESC uniformBufferDesc;
    ZeroMemory(&uniformBufferDesc, sizeof(uniformBufferDesc));

    uniformBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    uniformBufferDesc.ByteWidth = sizeof(UniformObj);
    uniformBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    uniformBufferDesc.CPUAccessFlags = 0;
    uniformBufferDesc.MiscFlags = 0;

    d3d11Device->CreateBuffer(&uniformBufferDesc, NULL, &uniformBuffer);
}

void CleanUp() {
    SwapChain->Release();
    d3d11Device->Release();
    d3d11DevCon->Release();
    renderTargetView->Release();

    vertBuffer->Release();
    indexBuffer->Release();
    VS->Release();
    PS->Release();
    VS_Buffer->Release();
    PS_Buffer->Release();
    vertLayout->Release();
    depthStencilView->Release();
    depthStencilBuffer->Release();
    uniformBuffer->Release();
}

int mainloop() {
    MSG msg;
    ZeroMemory(&msg, sizeof(MSG));
    while (true) {
        BOOL PeekMessageL(
            LPMSG lpMsg,
            HWND hWnd,
            UINT wMsgFilterMin,
            UINT wMsgFilterMax,
            UINT wRemoveMsg
        );

        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT)
                break;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            UpdateScene();
            DrawScene();
        }
    }
    return msg.wParam;
}

bool InitializeDirect3d11App(HINSTANCE hInstance) {

    DXGI_MODE_DESC bufferDesc;

    ZeroMemory(&bufferDesc, sizeof(DXGI_MODE_DESC));

    bufferDesc.Width = Width;
    bufferDesc.Height = Height;
    bufferDesc.RefreshRate.Numerator = 60;
    bufferDesc.RefreshRate.Denominator = 1;
    bufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    bufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    bufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

    DXGI_SWAP_CHAIN_DESC swapChainDesc;

    ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

    swapChainDesc.BufferDesc = bufferDesc;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = 1;
    swapChainDesc.OutputWindow = hwnd;
    swapChainDesc.Windowed = TRUE;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;


    hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL, NULL, NULL,
        D3D11_SDK_VERSION, &swapChainDesc, &SwapChain, &d3d11Device, NULL, &d3d11DevCon);

    ID3D11Texture2D* BackBuffer;
    hr = SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&BackBuffer);

    if (BackBuffer) {
        hr = d3d11Device->CreateRenderTargetView(BackBuffer, NULL, &renderTargetView);
        BackBuffer->Release();
    }

    D3D11_TEXTURE2D_DESC depthStencilDesc;
    depthStencilDesc.Width = Width;
    depthStencilDesc.Height = Height;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.ArraySize = 1;
    depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilDesc.SampleDesc.Count = 1;
    depthStencilDesc.SampleDesc.Quality = 0;
    depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
    depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthStencilDesc.CPUAccessFlags = 0;
    depthStencilDesc.MiscFlags = 0;

    d3d11Device->CreateTexture2D(&depthStencilDesc, NULL, &depthStencilBuffer);
    if (depthStencilBuffer) {
        d3d11Device->CreateDepthStencilView(depthStencilBuffer, NULL, &depthStencilView);
    }

    d3d11DevCon->OMSetRenderTargets(1, &renderTargetView, NULL);

    return true;
}

bool InitializeWindow(HINSTANCE hInstance, int ShowWnd, int width, int height, bool windowed) {
    typedef struct _WNDCLASS {
        UINT cbSize;
        UINT style;
        WNDPROC lpfnWndProc;
        int cbClsExtra;
        int cbWndExtra;
        HANDLE hInstance;
        HICON hIcon;
        HCURSOR hCursor;
        HBRUSH hbrBackground;
        LPCTSTR lpszMenuName;
        LPCTSTR lpszClassName;
    } WNDCLASS;

    WNDCLASSEX wc;

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = NULL;
    wc.cbWndExtra = NULL;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 2);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = WndClassName;
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassEx(&wc)) {
        MessageBox(NULL, L"Error registering class", L"Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    hwnd = CreateWindowEx(
        NULL,
        WndClassName,
        L"Window Title",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        width, height,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (!hwnd) {
        MessageBox(NULL, L"Error creating window", L"Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    ShowWindow(hwnd, ShowWnd);
    UpdateWindow(hwnd);

    return true;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {

    if (!InitializeWindow(hInstance, nShowCmd, Width, Height, true)) {
        MessageBox(0, L"Window Initialization - Failed", L"Error", MB_OK);
        return 0;
    }

    if (!InitializeDirect3d11App(hInstance)) {
        MessageBox(0, L"Direct3D Initialization - Failed", L"Error", MB_OK);
        return 0;
    }

    if (!InitScene()) {
        MessageBox(0, L"Scene Initialization - Failed", L"Error", MB_OK);
        return 0;
    }

    mainloop();

    CleanUp();

    return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE) {
            DestroyWindow(hwnd);
        }
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}