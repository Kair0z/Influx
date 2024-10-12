#pragma once

namespace influx::renderer
{
	class rgname
	{
#if _DEBUG
	public:
		const char* get_name() const
		{
			return m_name.c_str();
		}

		void set_name(const string& name)
		{
			m_name = name;
		}

	private:
		string m_name;
#endif
	};

	using rghandle = uint64;
	using rgbuffer_id = rghandle;
	using rgtexture_id = rghandle;
	using rgpass_id = rghandle;

	enum class e_rg_load : uint8
	{
		clear,
		discard,
		preserve,
		no_access,
		count
	};

	enum class e_rg_store : uint8
	{
		resolve,
		discard,
		preserve,
		no_access,
		count
	};

	struct rgaccess final
	{
		e_rg_load m_load;
		e_rg_store m_store;
	};
}