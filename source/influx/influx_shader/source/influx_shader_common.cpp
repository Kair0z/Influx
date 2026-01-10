#include "influx_shader.h"
#include "core/file.h"
#include "core/regex.h"

namespace influx::shader
{
	// parse out potential shader types
	static const char* k_type_to_signature[shader::k_num_shadertypes] =
	{
		R"(\[shader\(\"vertex\"\)\])",
		R"(\[shader\(\"pixel\"\)\])",
		R"(\[shader\(\"domain\"\)\])",
		R"(\[shader\(\"geometry\"\)\])",
		R"(\[shader\(\"hull\"\)\])",

		R"(\[shader\(\"compute\"\)\])",

		R"(\[shader\(\"raygeneration\"\)\])",
		R"(\[shader\(\"miss\"\)\])",
		R"(\[shader\(\"closesthit\"\)\])",
		R"(\[shader\(\"anyhit\"\)\])",
		R"(\[shader\(\"intersection\"\)\])",

		R"(\[shader\(\"amp\"\)\])",
		R"(\[shader\(\"mesh\"\)\])",

		R"(\[shader\(\"callable\"\)\])",
		R"(\[shader\(\"library\"\)\])"
	};
	// see core::shader
	static_assert(_countof(k_type_to_signature) == shader::k_num_shadertypes);

	static const char* k_type_to_signature_ps_alt[] =
	{
		R"(\[shader\(\"pixel\"\)\])",
		R"(\[shader\(\"fragment\"\)\])"
	};

	result<parse_output> parse_shaders_in_file(const string& filepath)
	{
		using result_type = result<parse_output>;

		if (!path::exists(filepath))
			return result_type::make_error("error: filepath doesnt exist!");

		string file_content = path::content_to_string(filepath).get();
		if (file_content.empty())
			return result_type::make_error("error: file is empty!");

		auto result = parse_shaders_in_source(file_content);
		if (result.is_fail())
			return result_type::make_error("failed parsing shaders in source!");

		// we already parsed the shaders, but their result signatures contain no filename
		// update each parsed shader's filename
		for (auto& pair : result.get().m_shadermap)
		{
			for (auto& shader : pair.second)
			{
				path as_file = path(filepath); const bool without_extension = true;
				const string& filename = to_string(as_file.get_filename(without_extension));
				shader.m_signature.set_filename(filename);
			}
		}

		return result;
	}

	result<parse_output> parse_shaders_in_folder(const string& folderpath, const bool recursive, const char* file_extension)
	{
		using result_type = result<parse_output>;

		if (!path::exists(folderpath))
			return result_type::make_error("error: folderpath doesnt exist!");

		auto found_files_res = path::get_files_in_directory(folderpath, recursive, file_extension);
		if (!found_files_res.is_success())
			return result_type::make_error("error: failed getting files in folder!");

		result_type result{};
		for (const auto& filepath : found_files_res.get())
		{
			auto parsed_file_res = parse_shaders_in_file(to_string(filepath.get_full_path()));
			if (!parsed_file_res.is_success())
				continue;
			result.get().merge(parsed_file_res.get());
		}
		return result;
	}

	result<parse_output> parse_shaders_in_source(const string& shader_source)
	{
		using result_type = result<parse_output>;
		if (shader_source.empty())
		{
			return result_type::make_error("error: source string is empty!");
		}

		parse_output result_parse{};
		vector<string> source_lines = shader_source.split("\n");

		static auto parse_match = [&source_lines, &result_parse](
			shader::e_shader_type shader_type, uint32 line_index, const char* pattern) 
		{
			const string& line = source_lines[line_index];

			// search each line for the current type's signature ([shader("vertex")])
			influx::regex::for_each_match(line, pattern,
				[line_index, &source_lines, shader_type, &result_parse](const string& str)
				{
					// now figure out the function entrypoint name:
					// todo: make this a bit more error-proof
					uint32 next_idx = line_index + 1;
					string next_line = source_lines[next_idx];
					static uint32 max_it = line_index + 100;
					while ((next_line.empty() || next_line[0] == '[') && next_idx < max_it) next_line = source_lines[next_idx++];

					// found the entrypoint line, parse the entrypoint
					vector<string> entrypoints = regex::get_all_matches(next_line, R"(\b\w+\s+(\w+)\()");
					if (entrypoints.size() > 0 && entrypoints[0].empty() == false)
					{
						const string& entrypoint = entrypoints[0];

						parse_output::per_shader shader_parse{};
						shader_parse.m_signature.set_type(shader_type);
						shader_parse.m_signature.set_entrypoint(entrypoint);

						// flag this type as found
						result_parse.m_found_types |= get_shader_flag(shader_type);
						result_parse.m_shadermap[shader_type].push_back(shader_parse);
					}
				});
		};

		for (uint32 l = 0u; l < source_lines.size(); ++l)
		{
			for (uint32 i = 0u; i < shader::k_num_shadertypes; ++i)
			{
				if (i == (uint32)e_shader_type::ps)
				{
					for (uint32 a = 0u; a < _countof(k_type_to_signature_ps_alt); ++a)
						parse_match((e_shader_type)i, l, k_type_to_signature_ps_alt[a]);
				}
				else
				{
					parse_match((e_shader_type)i, l, k_type_to_signature[i]);
				}
			}
		}

		return result_parse;
	}

	template <typename _t>
	static void serialize_type(const _t& type, std::ostream& out) {
		out.write(reinterpret_cast<const char*>(&type), sizeof(type));
	}
	template <typename _t>
	static void deserialize_type(_t& type, std::istream& in) {
		in.read(reinterpret_cast<char*>(&type), sizeof(type));
	}
	template <typename _t>
	static void serialize_vector_type(const vector<_t>& vec, std::ostream& out) {
		const uint64 size = vec.size();
		out.write(reinterpret_cast<const char*>(&size), sizeof(size));
		out.write(reinterpret_cast<const char*>(vec.data()), size * sizeof(_t));
	}
	template <typename _t>
	static void deserialize_vector_type(vector<_t>& vec, std::istream& is) {
		size_t size;
		is.read(reinterpret_cast<char*>(&size), sizeof(size));
		vec.resize(size);
		is.read(reinterpret_cast<char*>(vec.data()), size * sizeof(_t));
	}

	void reflection::serialize(const reflection& refl, std::ostream& out)
	{
		serialize_type(refl.m_shader_type, out);
		serialize_vector_type(refl.m_bound_resources, out);
		serialize_vector_type(refl.m_parmblocks, out);
		serialize_vector_type(refl.m_resources, out);
		serialize_vector_type(refl.m_ioparams, out);
	}

	void reflection::deserialize(reflection& refl, std::istream& in)
	{
		deserialize_type(refl.m_shader_type, in);
		deserialize_vector_type(refl.m_bound_resources, in);
		deserialize_vector_type(refl.m_parmblocks, in);
		deserialize_vector_type(refl.m_resources, in);
		deserialize_vector_type(refl.m_ioparams, in);
	}

	void reflection::deserialize(reflection& refl, const byte* bytes, const uint64 size)
	{
		class memory_istreambuf : public std::streambuf {
		public:
			memory_istreambuf(const char* data, size_t size) {
				char* p = const_cast<char*>(data);
				setg(p, p, p + size);
			}
		};

		memory_istreambuf buf(
			reinterpret_cast<const char*>(bytes),
			size
		);

		std::istream in(&buf);
		return deserialize(refl, in);
	}
}