#pragma once

namespace influx
{
	template <typename _t>
	class range
	{
	public:
		range() = default;
		range(const _t& start, const _t& size)
			: m_start{ start }
			, m_size{ size }
		{

		}

		void set_size(const _t& size)
		{
			m_size = size;
		}

		void set_start(const _t& new_start)
		{
			m_start = m_size;
		}

		void shrink(const _t& size)
		{
			if (get_size() > size)
			{
				set_size(size);
			}
		}

		void grow(const _t& size)
		{
			if (get_size() < size)
			{
				set_size(size);
			}
		}

		_t get_start() const
		{
			return m_start;
		}

		_t get_end() const
		{
			return m_start + m_size;
		}

		_t get_size() const
		{
			return m_size;
		}

	private:
		_t m_start;
		_t m_size;
	};
}