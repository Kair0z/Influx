#pragma once

#include "core/singleton/singleton.h"
#include "Core/Container/Vector.h"

struct IDXGIFactory1;
struct IDXGIAdapter1;
struct ID3D12Device;
struct ID3D12CommandQueue;
struct ID3D12DescriptorHeap;
struct IDXGISwapChain4;
struct ID3D12CommandAllocator;
struct ID3D12Resource;
struct ID3D12Fence;
struct ID3D12GraphicsCommandList;
struct D3D12_CPU_DESCRIPTOR_HANDLE;
struct ID3D12RootSignature;
struct ID3D12PipelineState;
struct IDXGIFactory4;

namespace influx::renderer
{
	class command_list;

	class renderer_state final
		: public singleton<renderer_state>
	{
	public:
		void initialize(const init_args& args);
		command_list* record();
		void submit(const command_list* list);
		void submit(const vector<command_list*> lists);
		void present_to_window(platform::window_handle window_handle, const present_args& args);
		bool is_initialized() const;
		void cleanup();

		struct context final
		{
			context() = default;
			context(ID3D12Device* device, ID3D12DescriptorHeap* srvheap)
				: mp_device{ device }, mp_srvheap{ srvheap }{}
			ID3D12Device* mp_device = nullptr;
			ID3D12DescriptorHeap* mp_srvheap = nullptr;
		};

		struct per_frame_context final
		{
			uint64 m_frame = 0u;
			ID3D12CommandAllocator* mpdx_commandAllocator = nullptr;
			ID3D12GraphicsCommandList* mpdx_commandList = nullptr;
			platform::event_handle m_complete_event = NULL;
		};

	private:
		IDXGIFactory4* mpdx_factory = nullptr;
		ID3D12Device* mpdx_device = nullptr;
		ID3D12CommandQueue* mpdx_commandQueue = nullptr;
		IDXGISwapChain4* mpdx_swapchain = nullptr;
		ID3D12DescriptorHeap* mpdx_rendertargetHeap = nullptr;
		ID3D12DescriptorHeap* mpdx_srvheap = nullptr;
		ID3D12RootSignature* mpdx_rootsignature = nullptr;
		ID3D12PipelineState* mpdx_pipeline = nullptr;
		ID3D12Resource* mpdx_vertexbuffer = nullptr;

		vector<ID3D12CommandAllocator*> mpdx_commandAllocators{};
		vector<ID3D12GraphicsCommandList*> mpdx_commandLists{};
		ID3D12Fence* mpdx_fence = nullptr;
		vector<ID3D12Resource*> mpdx_backbufferResources{};
		uint32 m_swapchain_buffer_idx = 0u;
		uint32 m_rtvDescriptorSize = 0u;
		uint32 m_srvDescriptorSize = 0u;

		uint64 m_frame = 0u;

		void recreate_swapchain_from_window(const e_buffering& buffering, platform::window_handle handle);

	private:
		bool m_is_initialized = false;
	};

#if 0
	class RootRenderer;

	class RenderContext final
	{
	public:
		const Graphics::RHIDevice* GetDevice() const;

		/* Creating Textures */
		Graphics::RHITexture* GetAndOrCreateTexture(const string& key, const Graphics::RHITextureDesc& desc) const;

		/* Creating Graphics PSO */
		Graphics::RHIGraphicsPipeline* GetAndOrCreateGraphicsPipeline(const Graphics::RHIGraphicsPipelineDescription& key, const Graphics::RHIGraphicsPipelineLayoutDescription& pipelineLayoutKey) const;
		
		/* Creating Graphics PSO Layout */
		Graphics::RHIGraphicsPipelineLayout* GetAndOrCreateGraphicsPipelineLayout(const Graphics::RHIGraphicsPipelineLayoutDescription& key) const;

		/* Copying a texture into our current Swapchain */
		bool CopyTextureIntoSwapchain(Graphics::RHITexture* texture, Graphics::RHICommandList* cmdList) const;

		/* Accessing the window swapchain */
		Graphics::RHISwapchain* GetSwapchain() const;

	private:
		RootRenderer* mp_rootRendererPtr = nullptr;

		RenderContext() = default;
		friend class RootRenderer;
	};

	/* 
	* Root Renderer
	* 
	*/
	class RootRenderer final
	{
	public:
#pragma region TypeAliases
		using Ptr = RootRenderer*;
		using DevicePtr = Graphics::RHIDevice*;
		using SwapchainPtr = Graphics::RHISwapchain*;
		using GraphicsPipelinePtr = Graphics::RHIGraphicsPipeline*;
		using GraphicsPipelineLayoutPtr = Graphics::RHIGraphicsPipelineLayout*;
		using TexturePtr = Graphics::RHITexture*;

		using GfxPipelineKey = Graphics::RHIGraphicsPipelineDescription;
		using GfxPipelineLayoutKey = Graphics::RHIGraphicsPipelineLayoutDescription;

		using TextureCache = Cache<TexturePtr, string>;
		using GfxPipelineCache = Cache<GraphicsPipelinePtr, GfxPipelineKey, GfxPipelineLayoutKey>;
		using GfxPipelineLayoutCache = Cache<GraphicsPipelineLayoutPtr, GfxPipelineLayoutKey>;
	
		using IRendererList = vector<IRenderer*>;

		using OnPostInitializeAPI = function<void(const Graphics::EGraphicsAPI eApi, Graphics::RHIDevice*)>;
		using OnBuildCommandList = function<void(Graphics::RHICommandList*)>;
		using OnWindowResize = function<void(const Math::Vectoru2& newSize)>;
		using OnPreCleanupAPI = function<void(const Graphics::EGraphicsAPI eApi, Graphics::RHIDevice*)>;
#pragma endregion

	public:
		RootRenderer(const Graphics::EGraphicsAPI api, platform::window_handle windowHandle = nullptr);
		static Ptr Create(const Graphics::EGraphicsAPI api, platform::window_handle windowHandle = nullptr);
		static void Destroy(Ptr& renderer);
		virtual ~RootRenderer();

		/* Runs Command Queues */
		void Render();

		/* Runs Command Queues and schedules the passed CommandList to be built */
		void Render(OnBuildCommandList internalRenderClb);

		/* Present Swapchain */
		void Present(bool vsync);

		/* Adding child IRenderers as callbacks */
		template <class _ret, class ...Args>
		_ret* AddRenderer(Args&&... args)
		{
			static_assert(std::is_base_of<IRenderer, _ret>::value, "RootRenderer::AddRenderer() argument must be castable to IRenderer interface!");
			
			_ret* newRenderer = new _ret(args...);
			mp_childRenderers.push_back(newRenderer);

			// Initialize to API
			if (IsGraphicsAPIInitialized())
			{
				mp_childRenderers.back()->OnPostInitializeAPI(GetCurrentGraphicsAPI(), GetDevice());
			}

			return newRenderer;
		}

		/* Attach to Window and create a Swapchain */
		bool AttachToWindow(platform::window_handle windowHandle);
		bool DetachFromCurrentWindow();
		bool IsAttachedToWindow() const;

		/* Signal Window to be resized */
		bool SignalWindowResize(const Math::Vectoru2& newSize);
		bool DoesSwapchainNeedResize() const;

		/* Set Dynamic Graphics API */
		void SetGraphicsAPI(const Graphics::EGraphicsAPI api);
		Graphics::EGraphicsAPI GetCurrentGraphicsAPI() const;
		static bool IsGraphicsAPISupported(const Graphics::EGraphicsAPI api);

		/* Creating Graphics PSO */
		GraphicsPipelinePtr GetAndOrCreateGraphicsPipeline(const GfxPipelineKey& key, const GfxPipelineLayoutKey& pipelineLayoutKey);
		GraphicsPipelineLayoutPtr GetAndOrCreateGraphicsPipelineLayout(const GfxPipelineLayoutKey& key);
		
		/* Creating Textures */
		TexturePtr GetAndOrCreateTexture(const string& key, const Graphics::RHITextureDesc& desc);

		SwapchainPtr GetWindowSwapchain() const;
		const Math::Vectoru2& GetWindowSwapchainDimensions() const;

		uint64 GetFrame() const;

		Graphics::RHIDevice* GetDevice() const;
		const RenderContext& GetRenderContext();

	private:
		/* RHI Graphics Device */
		Graphics::RHIDevice* mp_rhiDevice;
		Graphics::EGraphicsAPI m_currentGraphicsAPI = Graphics::EGraphicsAPI::NotSupported;
		Graphics::EGraphicsAPI m_initializedDeviceAPI = Graphics::EGraphicsAPI::NotSupported;

		Graphics::RHIDescriptorHeap* mp_rtvDescriptorHeap;
		Graphics::RHICommandQueue* mp_gfxCommandQueue;
		
		IRendererList mp_childRenderers;

		/* Cached Graphics Pipelines */
		GfxPipelineCache mp_graphicsPipelineCache;
		GfxPipelineLayoutCache mp_graphicsPipelineLayoutCache;
		
		/* Cached Textures */
		TextureCache mp_textureCache;

		/* Swapchain */
		struct SwapchainTarget final
		{
			Graphics::RHISwapchain* mp_rhiSwapchain;
			Math::Vectoru2 m_previousSize;
			Math::Vectoru2 m_updatedSize;
			bool m_isDirty = true;
		};
		SwapchainTarget* mp_windowSwapchain = nullptr;

		struct PerFrameContext final
		{
			Graphics::RHICommandList* CommandList;

		} PerFrameContext;

		uint64 m_frame;

		/* Is Renderer initialized on this specific EGraphicsAPI? */
		bool IsGraphicsAPIInitialized(const Graphics::EGraphicsAPI api) const;
		bool IsGraphicsAPIInitialized() const;

		/* Initializes GraphicsDevice object */
		void InitializeGraphicsAPI(const Graphics::EGraphicsAPI api);

		/* Cleans up an initialized GraphicsDevice object */
		void CleanupGraphicsAPI(const Graphics::EGraphicsAPI api);
		void Cleanup();

		void UpdateSwapchain();

		RenderContext m_renderContext;
	};
#endif
}
