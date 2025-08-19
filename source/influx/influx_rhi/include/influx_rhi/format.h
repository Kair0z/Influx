#pragma once

namespace influx::rhi
{
	namespace format
	{
		enum e_format : uint8
		{
			typeless,
			uint,
			sint,
			unorm,
			unorm_srgb,
			snorm,
			sfloat,
			ufloat,
			shared_exp,
			xr_bias,
			
			num
		};
		enum e_bitsize : uint8
		{
			_32,
			_24,
			_16,
			_11,
			_10,
			_9,
			_8,
			_6,
			_5,
			_4,
			_2,
			_1,
			num
		};
		enum e_semantic : uint8
		{
			_r,
			_g,
			_b,
			_a,	// associated with alpha
			_d,	// associated with depth
			_s,	// associated with stencil
			_x, // additional (unused) (typeless)
			_e, // associated with exponent
			num
		};
		enum class e_spec_format : uint8
		{
			none,

			// block compression formats
			bc1,
			bc2,
			bc3,
			bc4,
			bc5,
			bc6,
			bc7,

			// misc
			AYUV,
			Y410,
			Y416,
			NV12,
			P010,
			P016,
			OP420, // DXGI_FORMAT_420_OPAQUE
			YUY2,
			Y210,
			Y216,
			NV11,
			AI44,
			IA44,
			P8,	  // palletized 
			A8P8,
			P208,
			V208,
			V408,
			sampler_feedback_minmip_opaque,
			sampler_feedback_mip_region_used_opaque,

			num
		};
		static constexpr uint32 k_num_special_formats = static_cast<uint32>(e_spec_format::num) - 1u;

		static constexpr uint32 k_num_semantic_types = static_cast<uint32>(e_semantic::num);
		static constexpr uint32 k_num_bitsize_types = static_cast<uint32>(e_bitsize::num);
		static constexpr uint32 k_num_format_types = static_cast<uint32>(e_format::num);

		static constexpr uint32 get_num_bits(format::e_bitsize _enum)
		{
			switch (_enum)
			{
			default: return 0u;
			case e_bitsize::_2: return 2u;
			case e_bitsize::_8: return 8u;
			case e_bitsize::_10: return 10u;
			case e_bitsize::_16: return 16u;
			case e_bitsize::_24: return 24u;
			case e_bitsize::_32: return 32u;
			}
		}
	}

	class pixelformat final
	{
	public:
		struct element final
		{
			element() = default;
			element(format::e_semantic sem, format::e_bitsize size)
			{
				m_enabled = true;
				m_bitsize = size;
				m_semantic = sem;
				m_overrides_format = false;
			}
			element(format::e_semantic sem, format::e_bitsize size, format::e_format override)
			{
				m_enabled = true;
				m_bitsize = size;
				m_semantic = sem;
				m_format_override = override;
				m_overrides_format = true;
			}
			
			bool operator==(const element& el) const
			{
				if (m_enabled != el.m_enabled)
					return false;
				if (!m_enabled) // if both not enabled, we don't really care
					return true;
				if (m_overrides_format != el.m_overrides_format)
					return false;
				if (m_overrides_format && m_format_override != el.m_format_override)
					return false;
				if (m_bitsize != el.m_bitsize)
					return false;
				if (m_semantic != el.m_semantic)
					return false;
			}
			bool operator!=(const element& el) const
			{
				return !(*this == el);
			}
			format::e_bitsize m_bitsize;
			format::e_semantic m_semantic;
			format::e_format m_format_override; // override on the group format
			bool m_overrides_format = false;
			bool m_enabled = false;
		};
		static constexpr uint32 k_max_num_elements = 4u;

		format::e_format m_format;
		element m_elements[k_max_num_elements];
		uint32 m_num_elements = 0u;
		format::e_spec_format m_special = format::e_spec_format::none;

		result<format::e_format> get_element_format(uint32 index) const
		{
			using result_type = result<format::e_format>;
			if (index >= m_num_elements)
				return result_type::make_error("index past m_num_elements");

			const element& element = m_elements[index];
			const bool overrides_format = element.m_format_override && element.m_enabled;

			return overrides_format ? element.m_format_override : m_format;
		}
		
		bool operator==(const pixelformat& other) const
		{
			if (m_format != other.m_format) return false;
			if (m_num_elements != other.m_num_elements) return false;
			if (m_special != other.m_special) return false;
			for (uint32 i = 0u; i < m_num_elements; ++i)
			{
				if (m_elements[i] != other.m_elements[i])
					return false;
			}
			return true;
		}
		bool operator!=(const pixelformat& format) const
		{
			return !(*this == format);
		}
		pixelformat() = default;
		pixelformat(format::e_spec_format spec)
		{
			m_special = spec;
		}
		pixelformat(format::e_format format, format::e_spec_format spec)
		{
			m_format = format;
			m_special = spec;
		}
		pixelformat(
			format::e_format format,
			const element& e0,
			const element& e1 = {},
			const element& e2 = {},
			const element& e3 = {})
		{
			m_format = format;
			m_elements[0u] = e0;
			m_elements[1u] = e1;
			m_elements[2u] = e2;
			m_elements[3u] = e3;
		}
	};
}