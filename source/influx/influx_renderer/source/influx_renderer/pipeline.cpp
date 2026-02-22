#include "renderer_pch.h"
#include "influx_renderer/pipeline.h"

// sol2
#include "sol/sol.hpp"

namespace influx::renderer
{
	static result<texture_desc> parse_texture_desc(const sol::table& table)
	{
		using result_type = result<texture_desc>;

		texture_desc desc{};
		desc.m_width = table["width"];
		desc.m_heigth = table["height"];
		desc.m_common.m_allow_uav;
		desc.m_common.m_array_size;
		return desc;
	}

	static result<buffer_desc> parse_buffer_desc(const sol::table& table)
	{
		using result_type = result<buffer_desc>;

		buffer_desc desc{};

		return desc;
	}

	static uint32 make_id(const char* name)
	{
		std::hash<string> hasher;
		return (uint32)hasher(name);
	}

	pipeline::pipeline()
	{
		texture_desc final_desc{};
		register_texture(make_texture_id("final"), final_desc).get();
	}

	pipeline::pass_id pipeline::make_pass_id(const char* name)		{ return make_id(name); }
	pipeline::texture_id pipeline::make_texture_id(const char* name)	{ return make_id(name); }
	pipeline::buffer_id pipeline::make_buffer_id(const char* name)	{ return make_id(name); }

	result<> pipeline::parse(const string& filepath)
	{
		using result_type = result<>;

		sol::state lua;
		lua.open_libraries(sol::lib::base, sol::lib::table);

		sol::load_result script = lua.load_file("pipeline.lua");
		if (!script.valid()) 
		{
			sol::error err = script;
			return result_type::make_error(err.what());
		}

		sol::protected_function_result result = script();
		if (!result.valid()) 
		{
			sol::error err = result;
			return result_type::make_error(err.what());
		}

		sol::table pipeline = result;
		sol::table resources = pipeline["resources"];
		for (auto& [key, value] : resources) 
		{
			sol::table resource = value.as<sol::table>();
			std::string type = resource["type"];
			std::string name = resource["name"];

			if (type == "texture")
			{
				auto res = parse_texture_desc(resource);
				if (res.is_success())
				{
					const texture_id id = make_texture_id(name.c_str());
					register_texture(id, res.get()).get();
				}
			}
			else if (type == "buffer") 
			{
				auto res = parse_buffer_desc(resource);
				if (res.is_success())
				{
					const buffer_id id = make_buffer_id(name.c_str());
					register_buffer(id, res.get()).get();
				}
			}
		}

		sol::table passes = pipeline["passes"];
		for (auto& [key, value] : passes)
		{
			sol::table pass = value.as<sol::table>();
			std::string name = pass["name"];

			pass_desc pass_desc{};

			sol::table reads = pass["reads"];
			sol::table writes = pass["writes"];
			for (auto& [key, value] : reads)
			{
				const std::string& read = value.as<std::string>();
				const texture_id tex_id = make_texture_id(read.c_str());
				if (has_texture(tex_id))
				{
					pass_desc.m_texture_reads.push_back(tex_id);
				}
				else
				{
					const buffer_id buff_id = make_buffer_id(read.c_str());
					if (has_buffer(buff_id))
					{
						pass_desc.m_buffer_reads.push_back(buff_id);
					}
				}
			}
			for (auto& [key, value] : writes)
			{
				const std::string& write = value.as<std::string>();
				const texture_id tex_id = make_texture_id(write.c_str());
				if (has_texture(tex_id))
				{
					pass_desc.m_texture_writes.push_back(tex_id);
				}
				else
				{
					const buffer_id buff_id = make_buffer_id(write.c_str());
					if (has_buffer(buff_id))
					{
						pass_desc.m_buffer_writes.push_back(buff_id);
					}
				}
			}

			register_pass(make_pass_id(name.c_str()), pass_desc).get();
		}

		return result_type::make_success();
	}

	result<pipeline::texture_id> pipeline::register_texture(const texture_id& id, const texture_desc& desc)
	{
		using result_type = result<texture_id>;

		if (m_textures.contains(id))
			return result_type::make_warning(id, "warning: texture_id already registered! Nothing happened...");

		m_textures.push_back(id, desc);
		return id;
	}

	result<pipeline::buffer_id> pipeline::register_buffer(const buffer_id& id, const buffer_desc& desc)
	{
		using result_type = result<buffer_id>;

		if (m_buffers.contains(id))
			return result_type::make_warning(id, "warning: buffer_id already registered! Nothing happened...");

		m_buffers.push_back(id, desc);
		return id;
	}

	result<pipeline::pass_id> pipeline::register_pass(const pass_id& id, const pass_desc& desc)
	{
		using result_type = result<pass_id>;

		if (m_passes.contains(id))
			return result_type::make_warning(id, "warning: pass_id already registered! Nothing happened...");

		m_passes.push_back(id, desc);
		return id;
	}
	
	result<cptr<texture_desc>> pipeline::get_texture(const texture_id& id) const
	{
		using result_type = result<cptr<texture_desc>>;
		cptr<texture_desc> found = m_textures.find(id);
		if (found == nullptr)
		{
			return result_type::make_error("error: texture at id not found!");
		}
		return found;
	}

	result<cptr<buffer_desc>> pipeline::get_buffer(const buffer_id& id) const
	{
		using result_type = result<cptr<buffer_desc>>;

		cptr<buffer_desc> found = m_buffers.find(id);
		if (found == nullptr)
		{
			return result_type::make_error("error: buffer at id not found!");
		}
		return found;
	}

	result<cptr<pipeline::pass_desc>> pipeline::get_pass(const pass_id& id) const
	{
		using result_type = result<cptr<pass_desc>>;

		cptr<pass_desc> found = m_passes.find(id);
		if (found == nullptr)
		{
			return result_type::make_error("error: pass at id not found!");
		}
		return found;
	}
	
	bool pipeline::has_texture(const texture_id& id) const
	{
		return m_textures.contains(id);
	}
	bool pipeline::has_buffer(const buffer_id& id) const
	{
		return m_buffers.contains(id);
	}
	bool pipeline::has_pass(const pass_id& id) const
	{
		return m_passes.contains(id);
	}

	const vector<pipeline::pass_desc>& pipeline::get_passes() const
	{
		return m_passes.get_vector();
	}
}