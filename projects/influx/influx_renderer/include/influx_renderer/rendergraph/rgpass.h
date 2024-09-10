#pragma once

namespace influx::renderer
{
	enum class e_rgpass_type : uint8
	{
		graphics,
		compute,
		async_compute,
		count
	};

	enum class e_rgpass_flags : uint32
	{
		none = 0x00,
		force_no_cull = 0x01,
		allow_uav_write = 0x02
	};

	class rgpass_base
	{
	protected:
		rgpass_base(e_rgpass_type type, e_rgpass_flags flags);

		bool is_culled() const;
		bool allow_uav_writes() const;

	private:
#if _DEBUG
		string m_name{};
#endif

	public:
#if _DEBUG
		void set_name(const string& name);
		const string& get_name() const;
#endif
	};

	template <typename _passdata>
	class trgpass final : public rgpass_base
	{
	public:
		

	protected:
		trgpass() = default;

		e_rgpass_type m_type;
		e_rgpass_flags m_flags;
		bool m_is_culled;
	};

	template<>
	class trgpass<void> final : public rgpass_base
	{
	public:
		using setup_func = function<void()>;
		using execute_func = function<void()>;

	private:
		trgpass(setup_func&& setup, execute_func&& exe,
			e_rgpass_type type = e_rgpass_type::graphics,
			e_rgpass_flags flags = e_rgpass_flags::none)
			: rgpass_base(type, flags)
			, m_setup{ std::move(setup) }
			, m_execute{ std::move(exe) }
		{

		}

		void setup()
		{
			m_setup();
		}

		void execute()
		{
			m_execute();
		}

	private:
		setup_func m_setup;
		execute_func m_execute;
	};
	
	template <typename _passdata>
	using rgpass = trgpass<_passdata>;
}