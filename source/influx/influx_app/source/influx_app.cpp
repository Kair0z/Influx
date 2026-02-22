#include "influx_app.h"

// STL
#include <iostream>
#include <string>

// influx::core
#include "core/threading/thread.h"
#include "core/container/ringBuffer.h"
#include "core/enum.h"
#include "core/plugin.h"

// influx::platform
#include "influx_platform/window.h"
#include "influx_platform/library.h"

// influx::app
#include "plugins/plugins.h"

// imgui
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"

// dx12
#include <d3d12.h>
#include <dxgi.h>
#pragma comment (lib, "d3d12.lib")
#pragma comment (lib, "dxgi.lib")

namespace influx::app
{
	bool g_imgui_initialized = false;
	bool g_imgui_win_initialized = false;
	bool g_imgui_dx12_initialized = false;
	ImDrawData* g_draw_data = nullptr;

	namespace constants
	{
		static constexpr uint32 k_max_cmd_title_length = 32u;
		static constexpr uint32 k_max_cmd_args = 8u;
	}

	class app_context final
	{
	public:
		file_manager& m_fileman;
		command_manager& m_cmdman;
		window_manager* m_windowman;
		editor_manager& m_editorman;

		app_context(command_manager& cmdman, file_manager& fileman, window_manager* windowman, editor_manager& editorman) 
			: m_cmdman{ cmdman }, m_fileman{ fileman }, m_windowman{ windowman }, m_editorman { editorman }  { }
	};

	enum class inhouse_command
	{
		load_plugin,
		num
	};
	constexpr auto k_inhouse_command_meta = enum_metadata{
		inhouse_command::load_plugin, "load_plugin"
	};

	class command final
	{
		using title = static_string<constants::k_max_cmd_title_length>;
		string m_title;
		vector<string> m_args;

	public:
		static result<command> parse(const string& line)
		{
			using result_type = result<command>;

			command result{};
			vector<string> parts = line.split(" ");
			string title = parts.size() >= 1u ? parts[0] : "";
			if (title.empty())
				return result_type::make_error("empty command line!");

			result.m_title = title;
			for (uint32 i = 1u; i < parts.size(); ++i)
			{
				result.m_args.push_back(parts[i]);
			}
			return result;
		}

		const string& get_title() const { return m_title; }

		const vector<string>& get_args() const { return m_args; }

		enum class flags
		{

		};
		enum class type
		{

		};
		struct create_args final
		{

		};
	};

	class file_manager final
	{
	public:
		file_manager() {}

		string get_absolute_path(const string& path)
		{

		}
	};

	class command_manager final
	{
		uint64 m_frame = 0u;

		using queue = ringbuffer<command, 64u>;
		static constexpr uint32 k_num_queues = 2u;
		queue m_queues[k_num_queues]{};

		queue& get_write_queue() { return m_queues[(m_frame)		% k_num_queues]; }
		queue& get_read_queue() { return m_queues[ (m_frame - 1u)	% k_num_queues]; }

	public:
		void tick(app_context& ctx)
		{
			m_frame++;
			get_write_queue().clear_lockless();
		}

		result<> push(const command& cmd)
		{
			using result_type = result<>;
			bool result = get_write_queue().push(cmd);
			if (result == false) return result_type::make_error("failed push!");
			return {};
		}

		template <typename _func>
		void process_command(const string& title, _func&& func)
		{
			if (m_frame == 0u) return;

			queue& read_queue = get_read_queue();
			read_queue.for_each_lockless([&](const command& cmd)
			{
				if (cmd.get_title() == title) 
					func(cmd);
			});
		}
	};

	class plugin_manager final
	{
		struct plugin_entry final
		{
			platform::library* m_library;
		};
		vector<plugin_entry> m_plugins;

	public:
		void load_plugin(const char* filepath)
		{
			platform::library* plugin_library = platform::library::load(filepath);
			for (const auto& func : plugin_library->get_functions())
			{
				// std::cout << "function: " << func.c_str() << "\n";
			}

			// call the load function
			plugin_load_args args{};
			typedef void (*load_func)(const plugin_load_args&);
			void* func = plugin_library->get_func_address("load");
			if (func)
			{
				((load_func)func)(args);
			}
		}

		void tick(app_context& ctx)
		{
			// process events
			ctx.m_cmdman.process_command("load_plugin", [&](const command & cmd)
			{
				const auto& args = cmd.get_args();
				load_plugin("D:/Git/Influx/bin/debug-windows-x86_64/influx_rendergraph/influx_rendergraph.dll");
				// load_plugin("D:/Git/Influx/bin/debug-windows-x86_64/graphtool/graphtool.dll");
			});
		}
		void tick_imgui()
		{
			for (const plugin_entry& plugin : m_plugins)
			{
				plugin.m_library->call("tick_imgui");
			}
		}
	};

	class window_manager final
	{
		using window_id = uint64;
		using window = platform::window;
		using window_desc = platform::window_desc;
		vector<window*> m_windows;

	public:
		uint32 get_num_active_window() const
		{
			return m_windows.size();
		}

		result<> new_window(const window_desc& desc)
		{
			using result_type = result<>;
			window* new_window = platform::window::create(desc);
			if (!new_window)
				return result_type::make_error("failed creating window from desc");

			ImGui_ImplWin32_Init(new_window->get_platform_handle());
			g_imgui_win_initialized = true;

			m_windows.push_back(new_window);
			// return m_windows.size() - 1u;
			return {};
		}

		window const* get_main_window() const
		{
			return m_windows[0];
		}

		void tick(app_context& ctx)
		{
			// process commands
			ctx.m_cmdman.process_command("new_window", [&](const command& cmd)
			{
				window_desc desc{};
				for (const auto& arg : cmd.get_args())
				{
					
				}
				desc.m_dimensions = { 640u, 480u };
				desc.m_name = "app";
				desc.m_position;
				desc.m_style.set_exit_button(false);
				new_window(desc).get();
			});

			// tick windows
			for (window const* window : m_windows)
			{
				bool is_quit = false;
				window->poll_events(is_quit);

				const math::uint2& window_dim = window->get_dimensions();
				ImGui::GetIO().DisplaySize = { (float)window_dim.x, (float)window_dim.y };
			}
		}
	};

	class console_manager final
	{
		thread m_input_thread;
		bool m_quit_requested = false;
		ringbuffer<string, 32u> m_queued_commands;

		void input_loop()
		{
			if (m_quit_requested)
				return;
			
			while (!m_quit_requested)
			{
				std::wstring str;
				std::getline(std::wcin, str);
				m_queued_commands.push(str.c_str());
			}
		}

	public:
		console_manager()
		{
			m_input_thread = thread([this]() { input_loop(); });
			print("Hello World!");
		}

		~console_manager()
		{
			m_quit_requested = true;
			m_input_thread.join();
		}

		void print(const char* mssg)
		{
			std::cout << mssg << "\n";
		}

		void tick(app_context& ctx)
		{
			// queue events
			string line;
			if (m_queued_commands.try_pop(line))
			{
				auto result = command::parse(line);
				if (result.is_success())
					ctx.m_cmdman.push(result.get());
			}
		}
	};

	class editor_manager final
	{
	public:
		editor_manager()
		{
			ImGui::SetCurrentContext(ImGui::CreateContext());
			
			g_imgui_initialized = true;
		}

		void begin_imgui()
		{
			if (!g_imgui_dx12_initialized || !g_imgui_win_initialized)
				return;

			ImGui_ImplDX12_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();
		}
		void end_imgui()
		{
			if (!g_imgui_dx12_initialized || !g_imgui_win_initialized)
				return;

			ImGui::EndFrame();
			ImGui::Render();
		}
	};

	class render_manager final
	{
		IDXGIFactory* m_factory;
		IDXGIAdapter* m_adapter;
		ID3D12CommandQueue* m_queue;
		ID3D12CommandAllocator* m_allocator;
		ID3D12GraphicsCommandList* m_cmdlist;
		ID3D12Device* m_device;
		ID3D12DescriptorHeap* m_resource_gpu_heap;
		ID3D12DescriptorHeap* m_resource_heap;
		ID3D12Fence* m_fence;
		uint64 m_frame = 0u;
		uint64 m_resource_heap_stack = 0u;

		umap<void*, IDXGISwapChain*> m_swapchains;

		static constexpr uint32_t k_num_frames = 2u;

		static void imgui_descriptor_allocate(ImGui_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_desc_handle, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_desc_handle)
		{
			render_manager& manager = *(render_manager*)info->UserData;
			ID3D12DescriptorHeap& gpu_heap = *manager.m_resource_gpu_heap;
			ID3D12DescriptorHeap& heap = *manager.m_resource_heap;

			const uint64 new_slot_index = manager.m_resource_heap_stack;
			*out_cpu_desc_handle = (D3D12_CPU_DESCRIPTOR_HANDLE)(gpu_heap.GetCPUDescriptorHandleForHeapStart().ptr + new_slot_index);
			*out_gpu_desc_handle = (D3D12_GPU_DESCRIPTOR_HANDLE)(gpu_heap.GetGPUDescriptorHandleForHeapStart().ptr + new_slot_index);
			manager.m_resource_heap_stack++;
		}
		static void imgui_descriptor_free(ImGui_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE cpu_desc_handle, D3D12_GPU_DESCRIPTOR_HANDLE gpu_desc_handle)
		{
			// don't do anything, we're freeing every frame :)
		}

	public:
		render_manager()
		{
			CreateDXGIFactory(IID_PPV_ARGS(&m_factory));
			m_factory->EnumAdapters(0u, &m_adapter);
			D3D12CreateDevice(m_adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device));

			HRESULT hres;

			const D3D12_COMMAND_LIST_TYPE cmdlist_type = D3D12_COMMAND_LIST_TYPE_DIRECT;
			D3D12_COMMAND_QUEUE_DESC desc{};
			desc.Flags;
			desc.NodeMask;
			desc.Priority = 0u;
			desc.Type = cmdlist_type;
			hres = m_device->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_queue));
			hres = m_device->CreateCommandAllocator(cmdlist_type, IID_PPV_ARGS(&m_allocator));
			hres = m_device->CreateCommandList(0u, cmdlist_type, m_allocator, nullptr, IID_PPV_ARGS(&m_cmdlist));
			hres = m_device->CreateFence(0u, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
			D3D12_DESCRIPTOR_HEAP_DESC heap_desc{};
			heap_desc.Flags;
			heap_desc.NodeMask;
			heap_desc.NumDescriptors = 3u;
			heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			m_device->CreateDescriptorHeap(&heap_desc, IID_PPV_ARGS(&m_resource_heap));
			heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			m_device->CreateDescriptorHeap(&heap_desc, IID_PPV_ARGS(&m_resource_gpu_heap));
			m_frame = 1u;
		}

		void tick(app_context& ctx)
		{
			if (g_imgui_initialized && !g_imgui_dx12_initialized)
			{
				ImGui_ImplDX12_InitInfo imgui_init{};
				imgui_init.CommandQueue = m_queue;
				imgui_init.Device = m_device;
				imgui_init.DSVFormat = DXGI_FORMAT_D32_FLOAT;
				imgui_init.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
				imgui_init.NumFramesInFlight = k_num_frames;
				imgui_init.SrvDescriptorAllocFn = imgui_descriptor_allocate;
				imgui_init.SrvDescriptorFreeFn = imgui_descriptor_free;
				imgui_init.SrvDescriptorHeap = m_resource_gpu_heap;
				imgui_init.UserData = this;

				ImGui_ImplDX12_Init(&imgui_init);
				g_imgui_dx12_initialized = true;
			}

#if 0
			void* window_handle = (ctx.m_windowman != nullptr) ? ctx.m_windowman->get_main_window() : nullptr;
			if (window_handle != nullptr && )
			{
				
			}
#endif

			while (m_frame != 0u && m_fence->GetCompletedValue() < m_frame - 1u)
			{
				// just stall
			}

			m_allocator->Reset();
			m_cmdlist->Reset(m_allocator, nullptr);
			
			if (g_imgui_dx12_initialized) 
			{
				ImDrawData* drawdata = ImGui::GetDrawData();
				if (drawdata) ImGui_ImplDX12_RenderDrawData(drawdata, m_cmdlist);
			}
				
			m_cmdlist->Close();
			m_queue->ExecuteCommandLists(1u, (ID3D12CommandList* const*)(&m_cmdlist));
			m_queue->Signal(m_fence, m_frame);
			m_frame++;
		}
	};

	app::app(component_flags flags)
		: m_active_components{flags}
	{
		m_command_man = new command_manager();
		m_file_man = new file_manager();
	}

	app::~app()
	{
		if (m_is_running && !m_is_quit_requested)
		{
			quit();
		}

		if (m_thread.joinable())
			m_thread.join();

		m_active_components = {};
		create_and_destroy_components();

		delete m_file_man;
		delete m_command_man;
	}

	app::result<> app::run_impl()
	{
		app_context ctx{ *m_command_man, *m_file_man, m_window_man, *m_editor_man };
		while (!m_is_quit_requested)
		{
			create_and_destroy_components();

			if (m_command_man) m_command_man->tick(ctx);
			if (m_window_man) m_window_man->tick(ctx);
			if (m_console_man) m_console_man->tick(ctx);
			if (m_plugin_man) m_plugin_man->tick(ctx);

			if (m_window_man && m_window_man->get_num_active_window() > 0)
			{
				if (m_editor_man) m_editor_man->begin_imgui();
				if (m_plugin_man) m_plugin_man->tick_imgui();
				if (m_editor_man) m_editor_man->end_imgui();
				if (m_render_man) m_render_man->tick(ctx);
			}
			
			m_is_running = true;
		}

		m_is_running = false;
		return {};
	}

	void app::create_and_destroy_components()
	{
		static constexpr uint32 k_num_components = static_cast<uint32>(component::num);
		for (uint32 i = 0u; i < k_num_components; ++i)
		{
			component comp = (component)i;
			const bool is_comp_enabled = is_enabled(comp);
			const bool is_comp_initialized = is_initialized(comp);
			if (is_comp_enabled != is_comp_initialized)
			{
				create_or_destroy(comp, is_comp_enabled);
			}
		}
	}

	void app::create_or_destroy(component comp, bool create)
	{
		switch (comp)
		{
		case component::window:
			if (create && !m_window_man)
			{
				m_window_man = new window_manager();
				m_render_man = new render_manager();
			}
			else if (!create && m_window_man)
			{
				delete m_window_man; m_window_man = nullptr;
				delete m_render_man; m_render_man = nullptr;
			}
			break;
		case component::console:
			if (create && !m_console_man) m_console_man = new console_manager();
			else if (!create && m_console_man) delete m_console_man;
			break;
		case component::plugins:
			if (create && !m_plugin_man) m_plugin_man = new plugin_manager();
			else if (!create && m_plugin_man) delete m_plugin_man;
			break;
		case component::editor:
			if (create && !m_editor_man) m_editor_man = new editor_manager();
			else if (!create && m_editor_man) delete m_editor_man;
			break;
		}
	}

	bool app::is_enabled(component comp) const
	{
		return has_flag(m_active_components, make_flag(comp));
	}

	bool app::is_initialized(component comp) const
	{
		switch (comp)
		{
		case component::window: return m_window_man != nullptr;
		case component::plugins: return m_plugin_man != nullptr;
		case component::console: return m_console_man != nullptr;
		}
		return false;
	}

	bool app::is_running() const
	{
		return m_is_running;
	}

	void app::set_enabled(component_flags components)
	{
		m_active_components |= components;
	}

	void app::quit()
	{
		m_is_quit_requested = true;
	}

	app::result<> app::run(const run_args& args)
	{
		if (!args.m_run_async)
		{
			return run_impl();
		}
		else
		{
			m_thread = thread([this]() {
				this->run_impl();
			});
			return{};
		}
	}
}

