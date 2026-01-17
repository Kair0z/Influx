#pragma once
#include "influx_assets/common.h"
#include "influx_assets/asset.h"
#include "influx_assets/asset_types.h"

namespace influx::assets
{
	struct init_args final
	{

	};

	INFLUX_ASSET_API result<> initialize(const init_args& args);
	INFLUX_ASSET_API result<> cleanup();

	INFLUX_ASSET_API result<asset_handle> load_flx(const string& filepath);
	INFLUX_ASSET_API result<asset_bundle> load_fbx(const string& filepath);
	INFLUX_ASSET_API result<asset_bundle> load_obj(const string& filepath);
	INFLUX_ASSET_API result<asset_bundle> load_png(const string& filepath);
	INFLUX_ASSET_API result<asset_bundle> load_jpg(const string& filepath);

	INFLUX_ASSET_API result<void const*> get_data(const asset_handle& handle);
	INFLUX_ASSET_API result<asset_header> get_header(const asset_handle& handle);
	INFLUX_ASSET_API result<asset_meta> get_meta(const asset_handle& handle);

	template <typename _t>
	inline result<_t const*> get_data(const asset_handle& handle)
	{
		using result_type = result<_t const*>;
		const auto get_data_res = get_data(handle);
		if (get_data_res.is_fail())
			return result_type::make_error("failed getting data at handle!");

		void const* data_ptr = get_data_res.get();
		return result_type::make_success(static_cast<_t const*>(data_ptr));
	}

	struct write_file_args final
	{
		bool m_allow_overwrite = false;

		static const write_file_args& allow_overwrite() {
			static write_file_args args{ true }; return args;
		}
		static const write_file_args& get_default() {
			static write_file_args args{}; return args;
		}
	};
	INFLUX_ASSET_API result<> write_flx(const string& filepath, const asset_handle& asset, const write_file_args& args = write_file_args::get_default());
}