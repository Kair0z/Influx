#pragma once
#include "influx_graphics/pipeline.h"

struct ID3D12PipelineState;

namespace influx::graphics
{
	// Shader record = {{Shader ID}, {RootArguments}}
	struct dx12_raytracing_shader_record final
	{
		struct pointer_and_size { void* m_ptr; uint64 m_size; };

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
		void initialize(device& device, const vector<dx12_raytracing_shader_record>& records)
		{
			if (records.empty() == false)
			{
				m_shader_record_size = records[0].get_record_size();
				m_bytesize = records.size() * m_shader_record_size;

				// create a cpu-writable gpu buffer for the table
				heap_desc heap_desc = heap_desc::shared_heap();
				buffer_desc buffer_desc = {};
				buffer_desc.m_bytesize = m_bytesize;
				buffer_desc.m_bytestride = m_shader_record_size;
				buffer_desc.m_init_state = e_resource_state::gen_read;
				m_resource = device.create_resource(buffer_desc, heap_desc);
				mpdx_resource = m_resource->get_native<ID3D12Resource>();

				// map records
				m_resource->map([this, &records](void* target)
				{
					byte* byte_target = static_cast<byte*>(target);
					for (const auto& record : records)
					{
						// copy the shader id, after it copy the root args
						memcpy(byte_target, record.m_shader_id.m_ptr, record.m_shader_id.m_size);
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

		void cleanup(device& device)
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

	template <e_pipeline_type _t>
	class dx12_pipeline final : public pipeline<_t>
	{
		ID3D12PipelineState* mpdx_pipeline;
		ID3D12StateObject* mpdx_raytracing_state_object; // I hate Dx12 >:(

	public:
		dx12_raytracing_shadertable m_raygen_shadertable;
		dx12_raytracing_shadertable m_miss_shadertable;
		dx12_raytracing_shadertable m_hitgroup_shadertable;

	private:
		dx12_pipeline(ID3D12StateObject* rtdxpipeline, const pipeline_desc<_t>& desc)
			: pipeline<_t>(desc)
		{
			static_assert(_t == e_pipeline_type::raytracing);
			base::mp_native = mpdx_raytracing_state_object = rtdxpipeline;
			mpdx_pipeline = nullptr;
		}

		dx12_pipeline(ID3D12PipelineState* dxpipeline, const pipeline_desc<_t>& desc)
			: pipeline<_t>(desc)
		{
			base::mp_native = mpdx_pipeline = dxpipeline;
			mpdx_raytracing_state_object = nullptr;
		}

		virtual void release_impl(device* device) override
		{
			if (mpdx_pipeline)
				mpdx_pipeline->Release();
			if (mpdx_raytracing_state_object)
				mpdx_raytracing_state_object->Release();

			m_raygen_shadertable.cleanup(*device);
			m_miss_shadertable.cleanup(*device);
			m_hitgroup_shadertable.cleanup(*device);
		}

		friend class dx12_device;
	};
}