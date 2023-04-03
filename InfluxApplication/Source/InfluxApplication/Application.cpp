#include "app_pch.h"

#include "InfluxApplication/Application.h"

#include "InfluxEngine/Engine.h"

#include "InfluxGraphics/RHI.h"

#include "InfluxAssets/InfluxAssets.h"

#include "InfluxRenderer/RootRenderer.h"
#include "InfluxRenderer/Renderers/SceneRenderer.h"

namespace Influx::Application
{
	Application::Application(const Settings& desc)
        : m_hasStarted{false}
        , m_recievedQuit{false}
        , m_creationSettings{desc}
        , m_currentSettings{desc}
        , m_time{}
        , m_frame{}
        , m_isInitialized{false}
        , m_hasCleanedUp{false}
        , mp_engine{nullptr}
        , mp_appRenderer{}
        , m_windowHandle{nullptr}
        , m_processHandle{nullptr}
        , m_appInstanceHandle{nullptr}
        , m_hasCreatedWindow{false}
	{
        
	}

    Application::~Application()
    {
        Cleanup();
    }

    void Application::Run(int argc, char** argv)
    {
        if (IsRunning())
        {
            // Cannot re-run. Call SignalQuit() instead!
            return;
        }

        m_isRunning = true;

        Initialize();

        Start();

        while (GetHasRecievedQuit() == false)
        {
            PollWindowEvents();

            Update();
            
            Render();

            ++m_frame;
        }

        m_isRunning = false;
    }

    void Application::Initialize()
    {
        if (m_isInitialized) return;

        m_processHandle     = Platform::GetCurrentProcess();
        m_appInstanceHandle = Platform::GetCurrentInstance();

        CreateEngine();

        if (GetShouldHaveWindow())
        {
            CreateWindow();

            CreateRenderer();
        }

        m_isInitialized = true;
    }

    void Application::Cleanup()
    {
        if (GetHasCreatedWindow())
        {
            Platform::DestroyWindow(m_windowHandle);
        }

        if (GetHasCreatedRenderer())
        {
            Renderer::RootRenderer::Destroy(mp_appRenderer);
        }

        if (GetHasCreatedEngine())
        {
            Engine::Destroy(mp_engine);
        }

        m_hasCleanedUp = true;
    }

    void Application::Start()
    {
        if (m_hasStarted)
        {
            // Don't restart!
            return;
        }

        const bool shouldRenderScene    = GetShouldRenderScene();
        const bool hasCreatedWindow     = GetHasCreatedWindow();
        const bool hasCreatedRenderer   = GetHasCreatedRenderer();
        const bool shouldRenderImGui    = GetShouldHaveImgui();

        if (!shouldRenderScene || !hasCreatedWindow || !hasCreatedRenderer)
        {
            return;
        }

        Graphics::RHIGraphicsPipelineDescription pipelineDesc{};
        {
            pipelineDesc.InputElements.push_back(Graphics::RHIGraphicsPipelineDescription::InputElement{ "POSITION", 0u, Graphics::ERHIFormat::RGB_32_Float, 0u, 0u, true, 0u });
            pipelineDesc.InputElements.push_back(Graphics::RHIGraphicsPipelineDescription::InputElement{ "COLOR", 0u, Graphics::ERHIFormat::RGBA_32_Float, 0u, 12u, true, 0u });

            //pipelineDesc.VS = compiledVertexShader;
            //pipelineDesc.PS = compiledPixelShader;

            pipelineDesc.PrimitiveTopologyType = Graphics::ERHIPrimitiveTopologyType::Triangle;

            pipelineDesc.BlendState = Graphics::RHIBlendState::GetDefault();
            pipelineDesc.RasterizerState = Graphics::RHIRasterizerState::GetDefault();
            pipelineDesc.DepthStencilState = Graphics::RHIDepthStencilState::GetDefault();

            pipelineDesc.RenderTargets[0].Format = Graphics::ERHIFormat::RGBA_8_Unorm;
        }

        Graphics::RHIGraphicsPipelineLayoutDescription layoutDesc{};

        m_hasStarted = true;
    }

    void Application::PollWindowEvents()
    {
        const bool shouldHaveWindow = GetShouldHaveWindow();
        const bool hasCreatedWindow = GetHasCreatedWindow();

        if (!shouldHaveWindow || !hasCreatedWindow)
        {
            return;
        }

        const bool hasRecievedQuit = GetHasRecievedQuit();

        if (!hasRecievedQuit)
        {
            bool hasQuit = !Platform::PollWindowEvents(m_windowHandle);
            m_recievedQuit = hasQuit;
        }
    }

    void Application::Update()
    {
        const bool hasCreatedEngine = GetHasCreatedEngine();

        if (!hasCreatedEngine)
        {
            return;
        }

        mp_engine->Tick();
    }

    void Application::Render()
    {
        const bool shouldRenderScene    = GetShouldRenderScene();
        const bool hasCreatedWindow     = GetHasCreatedWindow();
        const bool hasCreatedRenderer   = GetHasCreatedRenderer();

        if (!shouldRenderScene || !hasCreatedWindow || !hasCreatedRenderer)
        {
            return;
        }

        mp_appRenderer->Render();

        mp_appRenderer->Present(GetSettings().VSync);
    }

    void Application::CreateWindow()
    {
        const bool shouldHaveWindow = GetSettings().HasWindow;
        
        if (!shouldHaveWindow)
        {
            return;
        }

        Platform::WindowSettings windowSettings{};
        windowSettings.Width    = GetSettings().WindowDimensions.x;
        windowSettings.Heigth   = GetSettings().WindowDimensions.y;
        windowSettings.Name     = GetSettings().Name;

        const bool shouldOpen = true;
        m_windowHandle = Platform::CreateWindow(windowSettings, shouldOpen);

        m_hasCreatedWindow = true;
    }

    void Application::CreateEngine()
    {
        if (GetHasCreatedEngine())
        {
            return;
        }

        Engine::ConstructArgs constrArgs{};
        
        mp_engine = Engine::Create(constrArgs);
    }

    void Application::CreateRenderer()
    {
        if (!GetShouldHaveWindow() || !GetHasCreatedWindow())
        {
            return;
        }

        // Create a D3D12 renderer interface
        mp_appRenderer = Renderer::RootRenderer::Create(Graphics::EGraphicsAPI::D3D12, m_windowHandle);

        // Create Scene-renderer:
        Renderer::SceneRenderer* sceneRenderer = mp_appRenderer->AddRenderer<Renderer::SceneRenderer>();

        sceneRenderer->AddLight(Renderer::SceneRenderer::LightData{ {}, {}, {1,1,1}, 1.0f });
        sceneRenderer->SetCamera(Renderer::SceneRenderer::CameraData{ { 0, 0, 50.0f }, {0, 0, -1}, 90.0f });

        // Parse a scene-file and upload resulting mesh-data into sceneRenderer
        Assets::Scene out_scene{};
        Assets::LoadSceneFile("E:/Git/Influx/Resources/Meshes/box.fbx", out_scene, nullptr, Assets::SceneLoadDesc{});
        
        for (uint64 i = 0u; i < out_scene.Meshes.size(); ++i)
        {
            const Scene::Mesh& mesh = out_scene.Meshes[i];
            Renderer::SceneRenderer::MeshData meshData = mesh;

            sceneRenderer->AddMesh(meshData);
        }
    }

    void Application::SignalQuit()
    {
        m_recievedQuit = true;
    }

    bool Application::GetHasStarted() const
    {
        return m_hasStarted;
    }

    bool Application::IsRunning() const
    {
        return m_isRunning;
    }

    bool Application::GetHasRecievedQuit() const
    {
        return m_recievedQuit;
    }

    bool Application::GetShouldHaveWindow() const
    {
        return GetSettings().HasWindow;
    }

    bool Application::GetShouldHaveImgui() const
    {
        return GetSettings().HasImGUI && GetShouldHaveWindow();
    }

    bool Application::GetShouldRenderScene() const
    {
        return GetSettings().HasSceneRender && GetShouldHaveWindow();
    }

    bool Application::GetHasCleanedUp() const
    {
        return m_hasCleanedUp;
    }

    bool Application::GetHasCreatedWindow() const
    {
        return m_hasCreatedWindow && m_windowHandle != nullptr;
    }

    bool Application::GetHasCreatedRenderer() const
    {
        return mp_appRenderer != nullptr;
    }

    bool Application::GetHasCreatedEngine() const
    {
        return mp_engine != nullptr;
    }

    const Application::Time& Application::GetTime() const
    {
        return m_time;
    }

    float Application::GetTimeSinceCreation() const
    {
        return m_time.TimeSinceCreation;
    }

    float Application::GetTimeSinceRun() const
    {
        return m_time.TimeSinceRun;
    }

    const Application::Settings& Application::GetSettings() const
    {
        return m_currentSettings;
    }

    const Application::Settings& Application::GetCreationSettings() const
    {
        return m_creationSettings;
    }
}

