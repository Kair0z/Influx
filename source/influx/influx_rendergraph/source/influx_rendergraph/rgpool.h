#pragma once

// influx::core
#include "core/basetypes.h"
#include "core/container/vector.h"
#include "core/container/list.h"

// influx::rendergraph
#include "rgcommon.h"

namespace influx::rendergraph
{
	/*
		pool keeps track of all resources & descriptors
	*/
	class rgpool final
	{
		enum class e_descheap_slot
		{
			rtv,
			dsv,
			rsc,
			samp,
			rsc_gpu,
			samp_gpu,
			
			rtv_ext,
			dsv_ext,
			samp_ext,
			rsc_ext,

			num
		};
		static constexpr uint32 k_num_descheaps = static_cast<uint32>(e_descheap_slot::num);
		static constexpr uint32 k_num_internal_descheaps = k_num_descheaps - 4u;

		friend class rendergraph;

		struct pooled_buffer final
		{
			buffer_desc m_desc;
			rhi_resource* m_resource;
			uint64 m_last_used_frame;
			bool m_is_active;
		};
		struct pooled_texture final
		{
			texture_desc m_desc;
			rhi_resource* m_resource;
			uint64 m_last_used_frame;
			bool m_is_active;
		};

		uint64					m_frame = 0u;
		vector<pooled_texture>	m_texture_pool;
		vector<pooled_buffer>	m_buffer_pool;
		global_config			m_config;

		rhi_descheap*	m_int_descheaps[k_num_internal_descheaps];
		rhi_descheap*	m_ext_descheaps[k_num_ext_descheap_slots]{};

		// all allocated descriptors
		using descriptor_list = list<rhi_descriptor>;
		descriptor_list m_allocated_descriptors[k_num_descheaps];
		descriptor_list& get_allocated_descriptors(e_descheap_slot slot);

		rgpool(rhi_device& device, const global_config& config);
		~rgpool();

		void init_descriptor_heaps(rhi_device& device, const global_config&);
		void free_all_descriptors();
		void free_all_gpu_descriptors();
		void cleanup(rhi_device& device);

		// descriptors
		rhi_descheap*& get_descheap(e_descheap_slot slot, bool ignore_ext = false);
		result<rhi_descriptor> alloc_cpu_descriptor(rgdescriptor_type type);
		result<rhi_descriptor> alloc_gpu_srv();
		result<rhi_descriptor> alloc_gpu_sampler();

		result<> free_descriptor(e_descheap_slot type, rhi_descriptor handle);
		
		result<> bind_ext_descheap(e_ext_descheap_slot slot, rhi_descheap& heap, bool allow_override);
		result<> unbind_ext_descheap(e_ext_descheap_slot type);
		bool is_ext_descheap_bound(e_ext_descheap_slot slot) const;

		/* (de)allocating new resources */
		result<rhi_resource*> allocate_texture_resource(rhi_device&, const texture_desc& args);
		result<rhi_resource*> allocate_buffer_resource(rhi_device&, const buffer_desc& args);
		result<> release_texture(rhi_device&, rhi_resource& resource);
		result<> release_buffer(rhi_device&, rhi_resource& resource);
	};
}