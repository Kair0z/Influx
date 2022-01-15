#pragma once

namespace Influx
{
	namespace Internal
	{
		
	}

	template <class Type, class NumElements>
	class RHIBuffer
	{
	public:
		inline static constexpr uint32_t GetStride() noexcept 
		{
			return sizeof(Type);
		}

		inline static constexpr uint32_t GetNumElements() noexcept 
		{
			return NumElements;
		}

		inline static constexpr uint32_t GetSizeInBytes() noexcept 
		{
			return NumElements * sizeof(Type);
		}
	};
}


