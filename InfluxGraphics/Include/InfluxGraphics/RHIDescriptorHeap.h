#pragma once

#ifndef __GR_RHI_DESCRIPTORHEAP_H_
#define __GR_RHI_DESCRIPTORHEAP_H_

#include "Types.h"
#include "RHITypes.h"

namespace influx::Graphics
{
	/* DescriptorHeap */
	class RHIDescriptorHeap
	{
	private:
		ERHIResourceViewType m_type;
		bool m_isShaderVisible;
		uint64 m_numDescriptors;

	protected:
		RHIDescriptorHeap(const ERHIResourceViewType type, uint64 numDescriptors, bool isShaderVisible)
			: m_type{ type }
			, m_isShaderVisible{ isShaderVisible }
			, m_numDescriptors{ numDescriptors }
		{}

		virtual ~RHIDescriptorHeap() = default;

	public:
		const ERHIResourceViewType GetType() const;

		const uint64 GetNumDescriptors() const;

		bool IsShaderVisible() const;
	};

}

#endif