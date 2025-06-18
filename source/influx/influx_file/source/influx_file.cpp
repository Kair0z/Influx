#include "influx_file.h"

// std
#include <type_traits>

// cereal
#include "influx_cereal_layer.h"

namespace influx::files
{
	static constexpr bool k_use_binary_archive = false;

	template <bool _load, bool _binary = k_use_binary_archive>
	using arch_type = typename std::conditional<_binary,
		typename std::conditional<_load, cereal::BinaryInputArchive, cereal::BinaryOutputArchive>::type,
		typename std::conditional<_load, cereal::JSONInputArchive, cereal::JSONOutputArchive>::type>::type;

	template <typename _t, typename _arch_type>
	inline void named(_arch_type& arch, const char* name, _t& var)
	{
		arch.setNextName(name);
		arch(var);
	}

	namespace detail
	{
		std::ofstream start_save(const path& file)
		{
			// make sure the file exists
			auto res = path::create_file(file.get_full_path());
			if (res.is_unex())
			{
				// error here...
				return {};
			}

			std::ofstream ofs{};
			ofs.close();
			ofs.open(file.get_full_path());
			influx_assert(ofs.is_open());

			return ofs;
		}

		std::ifstream start_load(const path& file)
		{
			std::ifstream ifs{};
			ifs.close();
			ifs.open(file.get_full_path());
			influx_assert(ifs.is_open());

			return ifs;
		}
	}

	// generic sized vectors!!
	template <typename _t, typename _arch_type>
	inline bool serialize(vector<_t>& vec, _arch_type& parent)
	{
		size_t size = vec.size();
		named(parent, "num", size);

		vec.resize(size);
		for (size_t i = 0u; i < size; ++i)
		{
			serialize(vec[i], parent);
		}

		return true;
	}

	// matrix
	template <typename _mat_t, typename _arch_type>
	inline bool serialize(_mat_t& matrix, _arch_type& parent)
	{
		const size_t num_collumns = _mat_t::get_num_collumns();
		const size_t num_rows = _mat_t::get_num_rows();

		named(parent, "x", num_collumns);
		named(parent, "y", num_rows);

		for (size_t y = 0u; y < num_rows; ++y)
			for (size_t x = 0u; x < num_collumns; ++x)
			{
				serialize(matrix[y][x], parent);
			}

		return true;
	}

	// componentfile impl
	template <typename _arch_type>
	bool serialize(componentfile& component, _arch_type& parent)
	{
		named(parent, "Name", component.m_name);
		return true;
	}

	// projectfile impl
	template <typename _arch_type>
	bool serialize(projectfile& project, _arch_type& parent)
	{
		named(parent, "Name", project.m_name);
		parent.setNextName("entities");
		parent.startNode();
		serialize(project.m_entities, parent);
		parent.finishNode();

		return true;
	}

	// entityfile impl
	template <typename _arch_type>
	bool serialize(entityfile& entity, _arch_type& parent)
	{
		named(parent, "Name", entity.m_name);

		parent.setNextName("components");
		parent.startNode();
		serialize(entity.m_components, parent);
		parent.finishNode();

		return true;
	}

	// editorfile impl
	template <typename _arch_type>
	bool serialize(editorfile& editor, _arch_type& parent)
	{
		named(parent, "camera_transform", editor.m_camera_transform);
		return true;
	}

	template <typename _t>
	bool load_impl(const path& file, _t& result)
	{
		auto fstream = detail::start_load(file);
		auto archive = arch_type<true>(fstream);
		return serialize(result, archive);
	}

	template <typename _t>
	bool save_impl(const path& file, _t& result)
	{
		auto fstream = detail::start_save(file);
		auto archive = arch_type<false>(fstream);
		return serialize(result, archive);
	}

	void projectfile::save(const path& file)
	{
		save_impl(file, *this);
	}

	void projectfile::load(const path& file)
	{
		load_impl(file, *this);
	}

	void projectfile::clear()
	{
		m_name.clear();
		m_entities.clear();
	}

	void componentfile::save(const path& file)
	{
		save_impl(file, *this);
	}

	void componentfile::load(const path& file)
	{
		load_impl(file, *this);
	}

	void entityfile::save(const path& file)
	{
		save_impl(file, *this);
	}

	void entityfile::load(const path& file)
	{
		load_impl(file, *this);
	}

	void entityfile::clear()
	{
		m_name.clear();
		m_components.clear();
	}

	void editorfile::save(const path& file)
	{
		save_impl(file, *this);
	}
	void editorfile::load(const path& file)
	{
		load_impl(file, *this);
	}
	void editorfile::clear()
	{
		m_camera_transform = math::matrix4x4f::identity();
	}
}