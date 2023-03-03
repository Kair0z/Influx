#include "app_pch.h"

#include "InfluxApplication/Application.h"
#include "InfluxEngine/Engine.h"
#include "InfluxRenderer/RootRenderer.h"

#include "InfluxGraphics/RHI.h"

namespace Influx::Application
{
	Application::Application(const Settings& desc)
        : m_hasStarted{false}
        , m_recievedQuit{false}
        , m_creationSettings{desc}
        , m_currentSettings{desc}
        , m_deltaTime{}
        , m_frame{}
        , m_isInitialized{false}
        , m_time{}
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
        Initialize();

        Start();

        while (!GetHasRecievedQuit())
        {
            PollWindowEvents();
            Update();
            Render();
            ImguiRender();

            ++m_frame;
        }

        Cleanup();
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

            if (GetShouldHaveImgui())
            {
                // Todo...
            }
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
            return;
        }

        const bool shouldRenderScene = GetShouldRenderScene();
        const bool hasCreatedWindow = GetHasCreatedWindow();
        const bool hasCreatedRenderer = GetHasCreatedRenderer();
        const bool shouldRenderImGui = GetShouldHaveImgui();
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
        const bool shouldRenderScene = GetShouldRenderScene();
        const bool hasCreatedWindow = GetHasCreatedWindow();
        const bool hasCreatedRenderer = GetHasCreatedRenderer();
        const bool shouldRenderImGui = GetShouldHaveImgui();

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

            pipelineDesc.BlendState         = Graphics::RHIBlendState::GetDefault();
            pipelineDesc.RasterizerState    = Graphics::RHIRasterizerState::GetDefault();
            pipelineDesc.DepthStencilState  = Graphics::RHIDepthStencilState::GetDefault();

            pipelineDesc.RenderTargets[0].Format = Graphics::ERHIFormat::RGBA_8_Unorm;
        }
        Graphics::RHIGraphicsPipelineLayoutDescription layoutDesc{};

        mp_appRenderer->Render([this, pipelineDesc, layoutDesc](Graphics::RHICommandList* cmdList)
            {
                using namespace Influx::Graphics;
                RHIResource* currentSwapchainResource = mp_appRenderer->GetWindowSwapchain()->GetCurrentBackBufferResource();
                
                RHITextureDesc sceneColourDesc{};
                sceneColourDesc.Dimensions = mp_appRenderer->GetWindowSwapchainDimensions();
                sceneColourDesc.Format = Graphics::ERHIFormat::RGBA_8_Unorm;
                sceneColourDesc.NumMips = 1;

                RHITexture* sceneColour = mp_appRenderer->GetAndOrCreateTexture("SceneColour", sceneColourDesc);
                RHIRenderTargetView* sceneColourRTV = sceneColour->GetAndOrCreateRenderTargetView(mp_appRenderer->GetDevice());

                RHIViewport viewport{};
                RHIScissorRect scissorRect{};

		        // Clear Scene colour:
		        cmdList->ClearRTV(sceneColourRTV, {1.0f, 0.0f, 0.0f, 1.0f});
                
		        // Draw Triangle:
		        {
                    // cmdList->BindPipelineLayout(mp_appRenderer->GetAndOrCreateGraphicsPipelineLayout(layoutDesc));
                    // cmdList->BindPipelineState(mp_appRenderer->GetAndOrCreateGraphicsPipeline(pipelineDesc, layoutDesc));
                    // 
		        	// cmdList->BindRenderTarget(sceneColourRTV);
		        	// cmdList->BindViewports(viewport);
		        	// cmdList->BindScissorRect(scissorRect);
                    // 
		        	// cmdList->SetPrimitiveTopology(Graphics::ERHIPrimitiveTopology::TriangleList);

		        	//cmdList->BindIndexBuffer(indexBuffer, indexBufferSize);
		        	//cmdList->BindVertexBuffer(vertexBuffer, vertexBufferSize, vertexSize);
		        	//cmdList->DrawIndexedInstanced(numIndices, 1u, 0u, 0u, 0u);
		        }
		        
		        // Copy Scene Colour -> swapchainBackbuffer
		        cmdList->CopyResource(sceneColour->GetResource(), currentSwapchainResource);
            });

        mp_appRenderer->Present(true);
    }

    void Application::ImguiRender()
    {
       

        
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

        mp_appRenderer = Renderer::RootRenderer::Create(Graphics::EGraphicsAPI::D3D12, m_windowHandle);
    }

    void Application::SignalQuit()
    {
        m_recievedQuit = true;
    }


    bool Application::GetHasStarted() const
    {
        return m_hasStarted;
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

    bool Application::GetShouldHaveUpdate() const
    {
        return GetSettings().HasUpdate;
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

    const Application::Settings& Application::GetSettings() const
    {
        return m_currentSettings;
    }

    const Application::Settings& Application::GetCreationSettings() const
    {
        return m_creationSettings;
    }
}

