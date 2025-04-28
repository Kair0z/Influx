#pragma once
#include "influx_graphics/pipeline.h"

struct ID3D12PipelineState;

namespace influx::graphics
{
	// Shader record = {{Shader ID}, {RootArguments}}
	struct dx12_raytracing_shader_record final
	{
		struct pointer_and_size { void* m_ptr = nullptr; uint64 m_size = 0u; };

		pointer_and_size m_shader_id{};
		pointer_and_size m_root_args{};

		inline uint64 get_record_size() const
		{
			return m_shader_id.m_size + m_root_args.m_size;
		}
	};

	// Shader table = {{ ShaderRecord 1}, {ShaderRecord 2}, ...}
	class dx12_raytracing_shadertable final
	{
	public:
		inline void initialize(device& device, const vector<dx12_raytracing_shader_record>& records)
		{
			if (records.empty() == false)
			{
				m_shader_record_size = records[0].get_record_size();
				const auto stride = D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT;
				m_bytesize = records.size() * stride;

				// create a cpu-writable gpu buffer for the table
				buffer_desc buffer_desc = {};
				buffer_desc.m_bytesize = m_bytesize;
				buffer_desc.m_bytestride = stride;
				buffer_desc.m_init_state = e_resource_state::gen_read;
				m_resource = device.create_resource(buffer_desc, heap_desc::shared_heap());
				mpdx_resource = m_resource->get_native<ID3D12Resource>();

				// map records
				m_resource->map([this, &records](void* target)
				{
					byte* byte_target = static_cast<byte*>(target);
					for (const auto& record : records)
					{
						// copy the shader id
						memcpy(byte_target, record.m_shader_id.m_ptr, record.m_shader_id.m_size);

						// copy the optional root arguments
						if (record.m_root_args.m_ptr)
						{
							memcpy(byte_target + record.m_shader_id.m_size, record.m_root_args.m_ptr, record.m_root_args.m_size);
						}
					}

					byte_target += m_shader_record_size;
				});
			}
			m_records = records;
		}

		inline void cleanup(device& device)
		{
			device.release(m_resource);
		}

		resource* m_resource = nullptr;
		ID3D12Resource* mpdx_resource = nullptr;
		uint64 m_shader_record_size = 0u;
		uint64 m_bytesize;
		vector<dx12_raytracing_shader_record> m_records{};
		
		friend class dx12_device;
	};

	struct dx12_stateobject_properties final
	{
		string m_program_name = "";

		using program_id = uint64[4]; // wow
		program_id m_program_id{};				// GetProgramIdentifier()
	};
	struct dx12_workgraph_properties final
	{
		uint32 m_graph_index = 0u;				// GetWorkGraphIndex()
		uint64 m_backing_memory_bytesize = 0u;	// GetWorkGraphMemoryRequirements()
		uint32 m_num_entrypoints = 0u;			// GetNumEntrypoints()
	};
	struct dx12_workgraph_resources final
	{
		resource* m_backing_memory = nullptr;
		inline void initialize(graphics::device& device, const dx12_workgraph_properties& props)
		{
			buffer_desc desc{};
			desc.m_bindflags = e_bind_flags::uav;
			desc.m_bytesize = props.m_backing_memory_bytesize;
			m_backing_memory = device.create_resource(desc);
		}
		inline void cleanup(graphics::device& device)
		{
			m_backing_memory->release(&device);
		}
	};

	template <e_pipeline_type _t>
	class dx12_pipeline final : public pipeline<_t>
	{
		ID3D12PipelineState* mpdx_pipeline;
		ID3D12StateObject* mpdx_stateobject; // I hate Dx12 >:(

	public:
		dx12_raytracing_shadertable m_raygen_shadertable;
		dx12_raytracing_shadertable m_miss_shadertable;
		dx12_raytracing_shadertable m_hitgroup_shadertable;
		dx12_stateobject_properties m_stateobject_props;
		dx12_workgraph_properties m_workgraph_props;
		dx12_workgraph_resources m_workgraph_resources;

	private:
		dx12_pipeline(ID3D12StateObject* rtdxpipeline, const pipeline_desc<_t>& desc)
			: pipeline<_t>(desc)
		{
			static_assert(_t == e_pipeline_type::raytracing || _t == e_pipeline_type::workgraph);
			base::mp_native = mpdx_stateobject = rtdxpipeline;
			mpdx_pipeline = nullptr;
		}

		dx12_pipeline(ID3D12PipelineState* dxpipeline, const pipeline_desc<_t>& desc)
			: pipeline<_t>(desc)
		{
			base::mp_native = mpdx_pipeline = dxpipeline;
			mpdx_stateobject = nullptr;
		}

		virtual void release_impl(device* device) override
		{
			if (mpdx_pipeline)
				mpdx_pipeline->Release();
			if (mpdx_stateobject)
				mpdx_stateobject->Release();

			switch (_t)
			{
			case e_pipeline_type::raytracing:
				m_raygen_shadertable.cleanup(*device);
				m_miss_shadertable.cleanup(*device);
				m_hitgroup_shadertable.cleanup(*device);
				break;

			case e_pipeline_type::workgraph:
				m_workgraph_resources.cleanup(*device);
				break;
			}
		}

		friend class dx12_device;
	};
}