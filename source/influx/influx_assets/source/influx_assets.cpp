#include "influx_assets.h"
#include "hash.h"
#include "influx_import.h"

namespace influx::assets
{
	class asset_manager;

	void archiver::write_bytes(const void* data, uint64 bytesize)
	{
		const uint64 offset = m_write_buffer.size();
		m_write_buffer.resize(offset + bytesize);
		memcpy(m_write_buffer.data() + offset, data, bytesize);
	}
	void archiver::read_bytes(void* out_data, uint64 bytesize)
	{
		memcpy(out_data, m_read_buffer + m_offset, bytesize);
		m_offset += bytesize;
	}

	class base_asset_data
	{
		asset_handle m_handle = k_invalid_asset;
		asset_header m_header;
		asset_meta m_meta;

		virtual void* get_data_int() = 0;
		virtual result<> serialize(archiver&) = 0;

	protected:
		friend class asset_manager;
	public:
		template <typename _t>
		_t* get_data() {
			return static_cast<_t*>(get_data_int());
		}
		asset_handle get_handle() const {
			return m_handle;
		}
	};

	template <typename _t>
	class asset_data final : public base_asset_data
	{
		_t m_data;
		virtual void* get_data_int() override { return &m_data; }
		virtual result<> serialize(archiver& arch) override
		{
			return _t::serialize(arch, m_data);
		}
	};

	using scene_asset = asset_data<scene_data>;
	using mesh_asset = asset_data<mesh_data>;
	using actor_asset = asset_data<actor_data>;

	class asset_manager final
	{
		umap<asset_handle, base_asset_data*> m_assets;

		template <typename _t>
		_t& new_asset(const string& filepath, void* data, uint64 bytesize)
		{
			// make a handle
			hash_inputs hsh_inputs{};
			hsh_inputs.m_data = data;
			hsh_inputs.m_data_size = bytesize;
			hsh_inputs.m_filepath = filepath;
			hsh_inputs.m_version = "1.0"; // todo
			asset_handle new_handle = create_hash(hsh_inputs);

			// allocate a data
			_t* new_ass = new _t();
			new_ass->m_handle = new_handle;
			m_assets[new_handle] = new_ass;
			return *new_ass;
		}

	public:
		result<> initialize(const init_args& args)
		{
			using result_type = result<>;
			return result_type::make_error("noimpl");
		}
		result<> cleanup()
		{
			using result_type = result<>;
			return result_type::make_error("noimpl");
		}
		result<asset_handle> load_flx(const string& filepath)
		{
			using result_type = result<asset_handle>;

			if (!path::exists(filepath))
				return result_type::make_error("filepath file does not exist!");

			// load file

#if 0
			using result_type = result<scene_asset*>;
			toml::parse_result table = toml::parse_file(filepath.c_str());

			// get the header info
			static constexpr uint64 k_invalid_id = (uint64)-1;
			static constexpr const char* k_invalid_type = "unknown";
			static constexpr const char* k_invalid_version = "0.0";
			const uint64 id = table["header"]["id"].value_or(k_invalid_id);
			const string type = table["header"]["type"].value_or(k_invalid_type);
			const string version = table["header"]["version"].value_or(k_invalid_version);

			vector<toml::key> header_stack{};

			static auto process_header = [&header_stack](const uint32 lvl, const toml::key& header)
				{
					for (uint32 i = 0u; i < lvl; ++i) std::cout << "  ";
					std::cout << header << "\n";

					if (lvl > header_stack.size())
						header_stack.push_back(header);
				};
			static auto process_array = [](const uint32 lvl, const toml::key& key, const toml::array& array)
				{
					for (uint32 i = 0u; i < lvl; ++i) std::cout << "  ";
					std::cout << key << " = [";

					for (uint32 i = 0u; i < array.size(); ++i)
					{
						std::cout << array[i].as_string();
						const bool is_last = i >= array.size() - 1u;
						if (!is_last) std::cout << ",";
					}

					std::cout << "]\n";
				};
			static auto process_pair = [](const uint32 lvl, const toml::key& key, const toml::node& node)
				{
					for (uint32 i = 0u; i < lvl; ++i) std::cout << "  ";
					std::cout << key << " = " << node.as_string() << "\n";
				};

			function<void(toml::table*, uint32)> traverse_func;
			traverse_func = [&traverse_func](toml::table* tbl, uint32 lvl)
				{
					for (const auto& [key, value] : *tbl)
					{
						const bool is_array_of_tables = value.is_array_of_tables();
						const bool is_table = value.is_table();
						if (is_table || is_array_of_tables)
						{
							process_header(lvl, key);
						}
						if (is_table)
						{
							traverse_func(value.as_table(), lvl + 1u);
						}
						else
						{
							if (is_array_of_tables)
							{
								toml::array* value_as_array = value.as_array();
								for (uint32 i = 0u; i < value_as_array->size(); ++i)
									traverse_func((*value_as_array)[i].as_table(), lvl + 1u);
							}
							else if (value.is_array())
							{
								toml::array* value_as_array = value.as_array();
								process_array(lvl, key, (*value_as_array));
							}
							else process_pair(lvl, key, value);
						}
					}
				};
			traverse_func(&table, 0u);

			return {};
#endif
		}
		result<asset_bundle> load_fbx(const string& filepath)
		{
			using result_type = result<asset_bundle>;

			if (!path::exists(filepath))
				return result_type::make_error("filepath file does not exist!");

			// use import library to import the file
			imp::scene_load_args args{};
			auto load_result = imp::load_scene_file(filepath, args);
			if (load_result.is_fail())
				return result_type::make_error("load_scene_file at path failed!");

			// create the scene asset
			asset_bundle bundle{};
			imp::scene_data& scene = load_result.get();
			scene_asset& scene_ass = new_asset<scene_asset>(filepath, &scene, sizeof(scene));
			bundle.add(scene_ass.get_handle());

			// create the sub-assets (meshes, ...)
			for (const imp::mesh_data& mesh : scene.get_meshes())
			{
				mesh_asset& mesh_ass = new_asset<mesh_asset>(filepath, (void*)&mesh, sizeof(mesh));
				bundle.add(mesh_ass.get_handle());

				mesh_data& data = *(mesh_ass.get_data<mesh_data>());
				data.m_imported_transform = { scene.get_transform(mesh) };
				data.m_indices = mesh.m_indices;
				data.m_positions = mesh.m_positions;
			}

			return bundle;
		}
		result<asset_bundle> load_obj(const string& filepath)
		{
			using result_type = result<asset_bundle>;
			return result_type::make_error("noimpl");
		}
		result<asset_bundle> load_png(const string& filepath)
		{
			using result_type = result<asset_bundle>;
			return result_type::make_error("noimpl");
		}
		result<asset_bundle> load_jpg(const string& filepath)
		{
			using result_type = result<asset_bundle>;
			return result_type::make_error("noimpl");
		}
		result<> write_flx(const string& filepath, const asset_handle& handle, const write_file_args& args)
		{
			using result_type = result<>;

			const bool file_at_path_exists = path::exists(filepath);
			if (file_at_path_exists && !args.m_allow_overwrite)
				return result_type::make_error("failed to write file: m_allow_overwrite is false & the file already exists!");

			const path as_path = path(filepath);
			if (as_path.get_extension() != k_flx_extension)
				return result_type::make_error("filepath is not an .flx file!");

			// serialize the handle data
			archiver arch{};
			if (!m_assets.contains(handle))
				return result_type::make_error("asset handle not found!");
			{
				base_asset_data& data = *m_assets[handle];
				influx_assert(data.get_handle() == handle);

				data.serialize(arch);
			}

			// create the files & write the data
			const string fp_no_extension = as_path.get_full_path_without_extension();
			const string flx_filepath = fp_no_extension + k_flx_extension;
			const string meta_filepath = fp_no_extension + k_flx_meta_extension;
			{
				const bool binary = true;
				auto create_file_res = path::create_file(flx_filepath, binary);
				if (create_file_res.is_fail())
					return result_type::make_error("failed to write binary file!");

				const vector<byte>& bytebuffer = arch.get_writebuffer();
				std::ofstream file(flx_filepath, std::ios::binary);
				file.write(reinterpret_cast<const char*>(bytebuffer.data()), bytebuffer.size());

				auto create_meta_res = path::create_file(meta_filepath, false);
				if (create_meta_res.is_fail())
					return result_type::make_error("failed to write meta file!");
			}

			return {};
		}
		
		result<asset_header> get_header(const asset_handle& handle)
		{
			using result_type = result<asset_header>;
			if (!m_assets.contains(handle))
				return result_type::make_error("asset handle not found!");
			
			base_asset_data& data = *m_assets[handle];
			influx_assert(data.get_handle() == handle);
			return data.m_header;
		}
		result<void const*> get_data(const asset_handle& handle)
		{
			using result_type = result<void const*>;
			if (!m_assets.contains(handle))
				return result_type::make_error("asset handle not found!");

			base_asset_data& data = *m_assets[handle];
			influx_assert(data.get_handle() == handle);
			return data.get_data_int();
		}
		result<asset_meta> get_meta(const asset_handle& handle)
		{
			using result_type = result<asset_meta>;
			if (!m_assets.contains(handle))
				return result_type::make_error("asset handle not found!");

			base_asset_data& data = *m_assets[handle];
			influx_assert(data.get_handle() == handle);
			return data.m_meta;
		}
		static asset_manager& get() { static asset_manager assetman{}; return assetman; }
	};

	result<> initialize(const init_args& args)
	{
		return asset_manager::get().initialize(args);
	}
	result<> cleanup()
	{
		return asset_manager::get().cleanup();
	}
	result<asset_handle> load_flx(const string& filepath)
	{
		return asset_manager::get().load_flx(filepath);
	}
	result<> write_flx(const string& filepath, const asset_handle& asset, const write_file_args& args)
	{
		return asset_manager::get().write_flx(filepath, asset, args);
	}
	result<asset_bundle> load_fbx(const string& filepath)
	{
		return asset_manager::get().load_fbx(filepath);
	}
	result<asset_header> get_header(const asset_handle& handle)
	{
		return asset_manager::get().get_header(handle);
	}
	result<asset_meta> get_meta(const asset_handle& handle)
	{
		return asset_manager::get().get_meta(handle);
	}
	result<void const*> get_data(const asset_handle& handle)
	{
		return asset_manager::get().get_data(handle);
	}
}


