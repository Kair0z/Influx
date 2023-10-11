#pragma once

#include "InfluxRenderer/IRenderer.h"

#include "Core/Math/Vector.h"
#include "Core/Container/List.h" //  List<WidgetPtr>;
#include "Core/Platform/Platform.h"
#include "Core/Pointer.h"

#pragma region Predeclarations
namespace influx::Graphics
{
	class RHIGraphicsPipelineLayout;
	class RHIGraphicsPipeline;
	class RHIDescriptorHeap;
	class RHITexture;
	class RHIResource;
}

namespace influx::GUI
{
	class IWidget;
}

namespace influx::Renderer
{
	class RenderContext;
}
#pragma endregion

namespace influx::GUI
{
	class GUIRenderer final : public Renderer::IRenderer
	{
	public:
		using WidgetPtr = Ptr<IWidget>;

	public:
		/* Child Widgets */
		void AddWidget(WidgetPtr widget, bool allowDuplicate = false);
		void RemoveWidget(WidgetPtr widget);
		bool HasWidget(WidgetPtr widget) const;

	private:
		/* After initializing the RHI API Device */
		/* Here you initialize / create your RHI objects */
		virtual void OnPostInitializeAPI(const Graphics::EGraphicsAPI api, Graphics::RHIDevice* device) override;

		/* */
		virtual void OnBuildRenderCommandList(
			const Renderer::RenderContext& context, 
			Graphics::RHICommandList* cmdList) override;

		virtual void OnAttachToWindow(const Renderer::RenderContext& context) override;
		virtual void OnDetachFromWindow(const Renderer::RenderContext& context) override;

		/* */
		virtual void OnWindowResize(
			const Renderer::RenderContext& context, 
			const Math::Vectoru2& oldSize, const Math::Vectoru2& newSize) override;

		/* Before cleaning up the RHI API Device */
		/* Here you Release your RHI objects */
		virtual void OnPreCleanupAPI(const Graphics::EGraphicsAPI api, Graphics::RHIDevice* device) override;

	private:
		GUIRenderer();

		using WidgetList = List<WidgetPtr>;
		WidgetList m_widgetList{};

		/* RHI Objects for ImGui */
		Graphics::RHIGraphicsPipelineLayout* mp_pipelineLayout;
		Graphics::RHIGraphicsPipeline* mp_pipeline;
		Graphics::RHITexture* mp_fontTexture;
		Graphics::RHIResource* mp_vertexBufferResource;
		Graphics::RHIResource* mp_indexBufferResource;

		/* Imgui keeps this state */
		static void AttachToWin32Backend(platform::window_handle windowHandle);
		static void DetachFromWin32Backend();
		static bool IsAttachedToWin32Backend();
	};
}


