#pragma once

namespace influx::rhi
{
	namespace format
	{
		enum e_format : uint8
		{
			unknown,
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
			__num
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
			_num
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
			bc6h,
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
		static constexpr uint32 k_num_semantic_types = static_cast<uint32>(e_semantic::_num);
		static constexpr uint32 k_num_bitsize_types = static_cast<uint32>(e_bitsize::__num);
		static constexpr uint32 k_num_format_types = static_cast<uint32>(e_format::num); // counting 'unknown' too!
		static constexpr uint32 k_num_special_formats = static_cast<uint32>(e_spec_format::num); // counting 'none' too!
		static constexpr uint32 get_num_bits(format::e_bitsize _enum)
		{
			switch (_enum)
			{
			default: return 0u;
			case e_bitsize::_1: return 1u;
			case e_bitsize::_2: return 2u;
			case e_bitsize::_4: return 4u;
			case e_bitsize::_5: return 5u;
			case e_bitsize::_6: return 6u;
			case e_bitsize::_8: return 8u;
			case e_bitsize::_9: return 9u;
			case e_bitsize::_10: return 10u;
			case e_bitsize::_11: return 11u;
			case e_bitsize::_16: return 16u;
			case e_bitsize::_24: return 24u;
			case e_bitsize::_32: return 32u;
			}
			return (uint32)-1;
		}
		static constexpr uint32 get_num_bits(format::e_spec_format special, const uint32 value_if_unknown)
		{
			switch (special)
			{
				// case e_spec_format::bc1										: return 32u;
				// case e_spec_format::bc2										: return 32u;
				// case e_spec_format::bc3										: return 32u;
				// case e_spec_format::bc4										: return 32u;
				// case e_spec_format::bc5										: return 32u;
				// case e_spec_format::bc6h										: return 32u;
				// case e_spec_format::bc7										: return 32u;
				case e_spec_format::AYUV										: return 32u;
				case e_spec_format::Y410										: return 32u;
				case e_spec_format::Y416										: return 32u;
				case e_spec_format::NV12										: return 32u;
				case e_spec_format::P010										: return 32u;
				case e_spec_format::P016										: return 32u;
				case e_spec_format::OP420										: return 32u;
				case e_spec_format::YUY2										: return 32u;
				case e_spec_format::Y210										: return 32u;
				case e_spec_format::Y216										: return 32u;
				case e_spec_format::NV11										: return 32u;
				case e_spec_format::AI44										: return 32u;
				case e_spec_format::IA44										: return 32u;
				case e_spec_format::P8											: return 32u;
				case e_spec_format::A8P8										: return 32u;
				case e_spec_format::P208										: return 32u;
				case e_spec_format::V208										: return 32u;
				case e_spec_format::V408										: return 32u;
				case e_spec_format::sampler_feedback_minmip_opaque				: return 32u;
				case e_spec_format::sampler_feedback_mip_region_used_opaque		: return 32u;
			}

			return value_if_unknown;
		}
	}

	class pixelformat final
	{
	private:
		struct element final
		{
			element() = default;
			element(format::e_semantic sem, format::e_bitsize size)
			{
				m_is_valid = true;
				m_bitsize = size;
				m_semantic = sem;
				m_overrides_format = false;
			}
			element(format::e_semantic sem, format::e_bitsize size, format::e_format override)
			{
				m_is_valid = true;
				m_bitsize = size;
				m_semantic = sem;
				m_format_override = override;
				m_overrides_format = true;
			}
			
			bool operator==(const element& el) const
			{
				if (m_is_valid != el.m_is_valid)
					return false;
				if (!m_is_valid) // if both not enabled, we don't really care
					return true;
				if (m_overrides_format != el.m_overrides_format)
					return false;
				if (m_overrides_format && m_format_override != el.m_format_override)
					return false;
				if (m_bitsize != el.m_bitsize)
					return false;
				if (m_semantic != el.m_semantic)
					return false;

				return true;
			}
			bool operator!=(const element& el) const
			{
				return !(*this == el);
			}
			format::e_bitsize m_bitsize;
			format::e_semantic m_semantic;
			format::e_format m_format_override; // override on the group format
			bool m_overrides_format = false;
			bool m_is_valid = false;
		};
		static constexpr uint32 k_max_num_elements = 4u;

		// the format shared for most elements (elements can override this)
		format::e_format m_format;

		// the special format that may or may not be combined with e_format
		format::e_spec_format m_special = format::e_spec_format::none;

		// descriptions of each element of the pixel
		element m_elements[k_max_num_elements];

		uint32 m_num_elements = 0u;
		
	public:
		uint32 get_num_elements() const
		{
			return m_num_elements;
		}

		result<format::e_format> get_element_format(uint32 index) const
		{
			using result_type = result<format::e_format>;
			if (index >= get_num_elements())
				return result_type::make_error("index past m_num_elements");

			const element& element = m_elements[index];
			const bool overrides_format = element.m_format_override && element.m_is_valid;

			return overrides_format ? element.m_format_override : m_format;
		}
		
		uint32 get_bytes_per_pixel() const
		{
			uint32 num_bits = 0u;

			// special format overrides
			if (is_special_format())
			{
				uint32 assumed_num_bits = 32u;
				num_bits = format::get_num_bits(m_special, assumed_num_bits);
			}

			// sum the elements
			for (uint32 i = 0u; i < m_num_elements; ++i)
			{
				num_bits += format::get_num_bits(m_elements[i].m_bitsize);
			}
			return num_bits / 8u;
		}

		pixelformat& set_element(uint32 element_index, const element& element)
		{
			if (element_index < k_max_num_elements)
			{
				m_elements[element_index] = element;
				
				if (element_index >= m_num_elements)
					m_num_elements = element_index + 1u;
			}
			return *this;
		}

		bool is_special_format() const
		{
			return m_special != format::e_spec_format::none && m_special != format::e_spec_format::num;
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

			element const* elements[4u]{ &e0, &e1, &e2, &e3 };
			for (uint32 i = 0u; i < 4u; ++i)
			{
				if (elements[i]->m_is_valid)
					m_num_elements++;

				m_elements[i] = *elements[i];
			}
		}

		// common formats
		static pixelformat rgba_8_unorm()
		{
			using namespace format;
			return { e_format::unorm,{_r,_8},{_g,_8},{_b,_8},{_a,_8}};
		}
	};

	INFLUX_RHI_API uint32 get_num_supported_pixel_formats();

	// returns a DXGI_FORMAT that matches the user format
	INFLUX_RHI_API uint32 get_translated_pixelformat(const pixelformat& format);

	// returns a DXGI_FORMAT as a string
	INFLUX_RHI_API const char* get_pixelformat_string(const pixelformat& format);
}