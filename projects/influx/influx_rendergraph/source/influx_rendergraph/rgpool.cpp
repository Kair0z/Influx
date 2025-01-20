#include "rendergraph_pch.h"
#include "rgpool.h"

#include "influx_graphics/device.h"

namespace influx::rendergraph
{
#pragma region translation
	graphics::buffer_desc translate(const buffer_desc& desc)
	{
		graphics::buffer_desc new_desc{};
		new_desc.m_bytesize;
		return new_desc;
	}
	graphics::tex2D_desc translate(const texture_desc& desc)
	{
		graphics::tex2D_desc new_desc{};
		return new_desc;
	}
#pragma endregion

	rgpool::rgpool(graphics::device* device)
		: m_device{device}
	{

	}

	void rgpool::tick()
	{
		for (uint64 i = 0; i < m_texture_pool.size();)
		{
			pooled_resource& resource = m_texture_pool[i];
			if (!resource.m_is_active && resource.m_last_used_frame + 4 < m_frame)
			{
				std::swap(m_texture_pool[i], m_texture_pool.back());
				m_texture_pool.pop_back();
			}
			else ++i;
		}

		++m_frame;
	}

	graphics::resource* rgpool::allocate_texture_resource(const texture_desc& args)
	{
		for (pooled_resource& item : m_texture_pool)
		{
			if (!item.m_is_active)
			{
				item.m_last_used_frame = m_frame;
				item.m_is_active = true;
				return item.m_resource;
			}
		}

		// create new
		pooled_resource new_item{};
		new_item.m_is_active = true;
		new_item.m_last_used_frame = m_frame;
		new_item.m_resource = m_device->create_resource(translate(args));
		m_texture_pool.push_back(new_item);
		return new_item.m_resource;
	}

	graphics::resource* rgpool::allocate_buffer_resource(const buffer_desc& args)
	{
		for (pooled_resource& item : m_buffer_pool)
		{
			if (!item.m_is_active)
			{
				item.m_last_used_frame = m_frame;
				item.m_is_active = true;
				return item.m_resource;
			}
		}

		// create new
		pooled_resource new_item{};
		new_item.m_is_active = true;
		new_item.m_last_used_frame = m_frame;
		new_item.m_resource = m_device->create_resource(translate(args));
		m_buffer_pool.push_back(new_item);
		return new_item.m_resource;
	}

	bool rgpool::release_texture(graphics::resource* resource)
	{
		for (pooled_resource& item : m_texture_pool)
		{
			if (item.m_is_active && item.m_resource == resource)
			{
				item.m_is_active = false;
				return true;
			}
		}
		return false;
	}

	bool rgpool::release_buffer(graphics::resource* resource)
	{
		for (pooled_resource& item : m_buffer_pool)
		{
			if (item.m_is_active && item.m_resource == resource)
			{
				item.m_is_active = false;
				return true;
			}
		}
		return false;
	}
}