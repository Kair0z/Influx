
#include "Geometry/Vertex.h"
#include "D3D12API.h"

#include <iostream>

LRESULT CALLBACK MainWndProc(
    HWND hwnd,        // handle to window
    UINT uMsg,        // message identifier
    WPARAM wParam,    // first message parameter
    LPARAM lParam)    // second message parameter
{

    switch (uMsg)
    {
    case WM_CREATE:
        // Initialize the window. 
        return 0;

    case WM_PAINT:
        // Paint the window's client area. 
        return 0;

    case WM_SIZE:
        // Set the size and position of the window. 
        return 0;

    case WM_DESTROY:
        // Clean up window-specific data objects. 
        return 0;

        // 
        // Process other messages. 
        // 

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}


HWND CreateAWindow(bool andShowIt)
{
    // Register the window class.
    const wchar_t CLASS_NAME[] = L"Sample Window Class";

    WNDCLASS wc = { };
    HINSTANCE instance = ::GetModuleHandle(NULL);
    wc.lpfnWndProc = (WNDPROC)MainWndProc;
    wc.hInstance = instance;
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,                              // Optional window styles.
        CLASS_NAME,                     // Window class
        L"Learn to Program Windows",    // Window text
        WS_OVERLAPPEDWINDOW,            // Window style

        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

        NULL,       // Parent window    
        NULL,       // Menu
        instance,  // Instance handle
        NULL        // Additional application data
    );

    if (hwnd != NULL && andShowIt)
    {
        ShowWindow(hwnd, 1);
    }

    return hwnd;
}


Influx::Vertex triangleVertices[3] =
{
    {},
    {},
    {}
};

int main()
{
    using namespace Influx;
    using namespace Graphics;

	// Creates a D3D12API object -> Initialize
	GraphicsAPI& api = D3D12API::Get();

    // Setup CmdQueue, Window & SwapChain...
    RHICommandQueue* cmdQueue = api.CreateCommandQueue(ECommandQueueType::Graphics);
    HWND hwnd = CreateAWindow(true);
    RHISwapChain* swapchain = api.CreateSwapChain(hwnd, cmdQueue);

    // Setup Assets:
    RHIVertexBuffer* vertexBuffer = api.CreateVertexBuffer(&triangleVertices[0].Position[0], sizeof(triangleVertices), sizeof(Vertex));

    MSG mssg;
    while (PeekMessageW(&mssg, hwnd, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&mssg);
        DispatchMessage(&mssg);

        /* Render loop*/
        {
            RHICommandList* cmdList = cmdQueue->SetupNewCommandList(&api);

            cmdList->TransitionResource(swapchain->GetCurrentBackBufferResource(), ERHIResourceState::RenderTarget);

            cmdList->ClearRTV(swapchain->GetCurrentRenderTargetView());

            cmdList->TransitionResource(swapchain->GetCurrentBackBufferResource(), ERHIResourceState::Present);

            cmdQueue->ExecuteCommmandList(cmdList);

            swapchain->Present(true);
        }
        
        if (mssg.message == WM_QUIT)
        {
            return 0;
        }
    }

    D3D12API::ReportLiveObjects();
}

