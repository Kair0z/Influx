#pragma once
#include "GraphicsAPI.h"
#include "Type/String.h"

namespace Influx::Graphics
{
	struct RHIRenderPassAttachmentDesc
	{
		RHIRenderPassAttachmentDesc() = default;
		RHIRenderPassAttachmentDesc(ERHIRenderPassLoadOp loadOp, ERHIRenderPassStoreOp storeOp, ERHIFormat format, 
			ERHIResourceState initialState, ERHIResourceState finalState, ERHISampleCount samples, String name, 
			ERHIRenderPassLoadOp stencilLoadOp = ERHIRenderPassLoadOp::DontCare, ERHIRenderPassStoreOp stencilStoreOp = ERHIRenderPassStoreOp::DontCare)
			: LoadOperation{ loadOp }, StoreOperation{ storeOp }, Format{ format }, InitialState{ initialState }, FinalState{ finalState }, Samples{ samples }, 
			StencilLoadOperation{ stencilLoadOp }, StencilStoreOperation{ stencilStoreOp }, Name{ name }{}

		ERHIFormat Format							= ERHIFormat::INVALID;
		ERHISampleCount Samples						= ERHISampleCount::_1;
		ERHIRenderPassLoadOp LoadOperation			= ERHIRenderPassLoadOp::DontCare;
		ERHIRenderPassStoreOp StoreOperation		= ERHIRenderPassStoreOp::DontCare;
		ERHIRenderPassLoadOp StencilLoadOperation	= ERHIRenderPassLoadOp::DontCare;;
		ERHIRenderPassStoreOp StencilStoreOperation = ERHIRenderPassStoreOp::DontCare;
		ERHIResourceState InitialState				= ERHIResourceState::Common;
		ERHIResourceState FinalState				= ERHIResourceState::Common;

		String Name = "";
		int ID = -1;
	};

	struct RHIRenderSubPassDesc
	{
		struct AttachmentRef
		{
			enum class EType
			{
				Color, Input, Resolve, DepthStencil, INVALID
			};

			enum class ELayout
			{
				ColorAttachmentOptimal
			};

			AttachmentRef(EType type, int attachmentIndex, ELayout layout) : Type{ type }, AttachmentIndex { attachmentIndex }, Layout{ layout } {}

			EType Type;
			ELayout Layout;
			int AttachmentIndex; // This is so ugly ~o~
		};

		ERHIPipelineBindPoint PipelineBindPoint;
		std::vector<AttachmentRef> AttachmentReferences;

		int NumColorReferences;
		int NumBlablaReferences;
	};

	struct RHIRenderSubPassDependency
	{

	};

	struct RHIRenderPassBeginInfo final
	{
		RHIRenderPassBeginInfo(const Math::Vector2u& renderAreaExtent, const Math::Vector2u& renderAreaOffset, const std::vector<Math::Vector4f>& clearValues)
			: RenderAreaExtent{ renderAreaExtent }, RenderAreaOffset{ renderAreaOffset }, ClearValues{ clearValues }{}

		Math::Vector2u RenderAreaExtent;		// 
		Math::Vector2u RenderAreaOffset;		// 
		std::vector<Math::Vector4f> ClearValues;
	};

	class RHIRenderPass
	{
	public:
		RHIRenderPass(const std::vector<RHIRenderPassAttachmentDesc>& attachments, 
			const std::vector<RHIRenderSubPassDesc>& subpasses, const std::vector<RHIRenderSubPassDependency>& dependencies);

		virtual ~RHIRenderPass() = default;

		void PushAttachment(const RHIRenderPassAttachmentDesc& newAttachment, GraphicsAPI* api = nullptr);
		void PushSubPass(const RHIRenderSubPassDesc& subPass, GraphicsAPI* api = nullptr);
		void PushSubPassDependency(const RHIRenderSubPassDependency& dependency, GraphicsAPI* api = nullptr);

		const std::vector<RHIRenderPassAttachmentDesc>& GetAttachments() const;
		const std::vector<RHIRenderSubPassDependency>& GetDependencies() const;
		const std::vector<RHIRenderSubPassDesc>& GetSubPasses() const;

	private:
		std::vector<RHIRenderPassAttachmentDesc> Attachments;
		std::vector<RHIRenderSubPassDesc> SubPasses;
		std::vector<RHIRenderSubPassDependency> Dependencies;

		/* Called when internal RHI Renderpass objects need to be recreated. */
		virtual void OnUpdateRHI(GraphicsAPI* api) {};
	};
}


