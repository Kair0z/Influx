#include "core/string.h"
#include "core/container/vector.h"

namespace influx::assets
{
	using asset_handle = uint64;
	inline static constexpr uint64 k_invalid_asset = (asset_handle)-1;

	class asset_bundle final
	{
		vector<asset_handle> m_assets{};
	public:
		const vector<asset_handle>& get_assets() const
		{ return m_assets; }

		void add(asset_handle handle)
		{
			m_assets.push_back(handle);
		}
	};

	struct asset_meta final
	{

	};

	struct asset_header final
	{
		string			m_type_str;
		asset_version	m_version;
		string			m_id_str;
	};
}