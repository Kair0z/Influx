#pragma once

#ifndef __GR_RHI_ROOTSIGNATURE_H_
#define __GR_RHI_ROOTSIGNATURE_H_

#include "Types.h"

namespace Influx::Graphics
{
	/* RHIRootSignature */
	class RHIRootSignature
	{
	protected:
		RHIRootSignature() = default;

	public:
		RHIRootSignature(const RHIRootSignature&) = delete;
		RHIRootSignature(RHIRootSignature&&) = delete;
		RHIRootSignature& operator=(const RHIRootSignature&) = delete;
		RHIRootSignature& operator=(RHIRootSignature&&) = delete;
		virtual ~RHIRootSignature() = default;
	};
}

#endif