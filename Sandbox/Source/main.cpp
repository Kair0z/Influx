#include "Geometry/Vertex.h"
#include "D3D12API.h"
#include "RHIResource.h"
#include "RHIPipeline.h"

constexpr Influx::Vector2u windowSize = {500, 500};

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
        L"Dx12 Test",    // Window text
        WS_OVERLAPPEDWINDOW,            // Window style

        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, (int)windowSize.x, (int)windowSize.y,

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

struct Camera
{
    Influx::Vector3f Position;
    Influx::Vector3f Forward;
    float Fov;

} gCamera{};


const static float gAspectRatio = (float)windowSize.x / (float)windowSize.y;

Influx::Vertex gTriangleVertices[3] =
{
    Influx::Vertex{{-0.25f, -0.15f * gAspectRatio, 0.0f}, {1.0f, 0.0f, 0.0f}, {}},
    Influx::Vertex{{0.0f, 0.25f * gAspectRatio, 0.0f}, {0.0f, 1.0f, 0.0f}, {}},
    Influx::Vertex{{0.25f, -0.15f * gAspectRatio, 0.0f}, {0.0f, 0.0f, 1.0f}, {}}
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
    RHIVertexBuffer* vertexBuffer = api.CreateVertexBuffer(&gTriangleVertices[0].Position[0], sizeof(gTriangleVertices), sizeof(Vertex));

    float windowWidth = (float)swapchain->GetWidth();
    float windowHeight = (float)swapchain->GetHeight();

    RHITextureDescription textureDesc{};
    textureDesc.Height = windowHeight;
    textureDesc.Width = windowWidth;
    textureDesc.OptimizedClearValue = { 0.5f, 0.0f, 0.0f, 1.0f };
    RHITexture* gameRenderTexture = api.CreateTexture(textureDesc);

    /* Create Graphics Pipeline Layout */
    RHIGraphicsPipelineLayoutDescription description{};
    description.LayoutBindings.AddBinding<PipelineLayout::ConstantsBinding<16, 0>>();
    //description.LayoutBindings.AddBinding<PipelineLayout::CBVBinding<1, 0>>();
    //description.LayoutBindings.AddBinding<PipelineLayout::SRVBinding<1, 1>>();
    RHIGraphicsPipelineLayout* renderPipelineLayout = api.CreateGraphicsPipelineLayout(description);

    /* Create Graphics Pipeline */
    RHIGraphicsPipelineDescription pipelineDesc{};
    pipelineDesc.PixelShader = api.CreateRHIShader(L"Source/Shaders/DefaultLitShader.hlsl", "PixelMain", ERHIShaderType::PixelShader);
    pipelineDesc.VertexShader = api.CreateRHIShader(L"Source/Shaders/DefaultLitShader.hlsl", "VertexMain", ERHIShaderType::VertexShader);
    pipelineDesc.RTVFormats = { ERHIFormat::RGBA_8_Unorm };
    pipelineDesc.PrimitiveTopologyType = ERHIPrimitiveTopologyType::Triangle;
    pipelineDesc.bDepthEnabled = false;
    RHIGraphicsPipeline* renderPipeline = api.CreateGraphicsPipeline(pipelineDesc, renderPipelineLayout);

    MSG mssg;
    bool isQuit = false;
    while (!isQuit && PeekMessageW(&mssg, hwnd, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&mssg);
        DispatchMessage(&mssg);

        {
            /* Setup Command List*/
            RHICommandList* cmdList = cmdQueue->SetupNewCommandList(&api);

            cmdList->TransitionResource(swapchain->GetCurrentBackBufferResource(), ERHIResourceState::RenderTarget);
            cmdList->TransitionResource(gameRenderTexture->GetRHIResource(), ERHIResourceState::RenderTarget);
            cmdList->ClearTextureAsRTV(gameRenderTexture, false);

            cmdList->BindPipelineState(renderPipeline);
            cmdList->BindPipelineLayout(renderPipelineLayout);
            cmdList->BindViewports(RHIViewport(windowWidth, windowHeight));
            cmdList->BindScissorRect(RHIScissorRect(windowWidth, windowHeight));
            cmdList->BindVertexBuffer(vertexBuffer);
            cmdList->SetPrimitiveTopology(ERHIPrimitiveTopology::TriangleList);
            cmdList->BindRenderTarget(gameRenderTexture->GetRenderTargetView());
            cmdList->DrawInstanced(3, 1);

            // Copy game RT -> Window RT
            cmdList->CopyResource(gameRenderTexture->GetRHIResource(), swapchain->GetCurrentBackBufferResource(), true);

            // Window RT - Ready to Present
            cmdList->TransitionResource(swapchain->GetCurrentBackBufferResource(), ERHIResourceState::Present);

            /* Submit Command List */
            cmdQueue->ExecuteCommmandList(cmdList);
            swapchain->Present(true);
        }
    }

    // Cleanup:
    delete cmdQueue;
    delete swapchain;
    delete vertexBuffer;
    delete gameRenderTexture;
    delete renderPipelineLayout;
    delete renderPipeline;
}

