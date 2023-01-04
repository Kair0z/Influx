#pragma once

#ifndef __GR_RHI_DESCRIPTORHEAP_H_
#define __GR_RHI_DESCRIPTORHEAP_H_

#include "Types.h"
#include "RHITypes.h"

namespace Influx::Graphics
{
	/* DescriptorHeap */
	class RHIDescriptorHeap
	{
	private:
		ERHIDescriptorType m_type;
		bool m_isShaderVisible;
		uint64 m_numDescriptors;

	protected:
		RHIDescriptorHeap(const ERHIDescriptorType type, uint64 numDescriptors, bool isShaderVisible)
			: m_type{ type }
			, m_isShaderVisible{ isShaderVisible }
			, m_numDescriptors{ numDescriptors }
		{}

		virtual ~RHIDescriptorHeap() = default;

	public:
		const ERHIDescriptorType GetType() const
		{
			return m_type;
		}

		const uint64 GetNumDescriptors() const
		{
			return m_numDescriptors;
		}

		bool IsShaderVisible() const
		{
			return m_isShaderVisible;
		}
	};

}

#endif