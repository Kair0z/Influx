#pragma once

#ifndef _EDITOR_H_
#define _EDITOR_H_

#include "Core/Singleton/Locator.h"

#include "ThirdParty/ImGui/imgui_impl_win32.h"
#include "ThirdParty/ImGui/imgui_impl_dx12.h"

// [CRINGE]
struct ID3D12DescriptorHeap;
struct ID3D12Resource;

namespace Influx
{
	class RenderAPI;
	class RHIRenderTarget;
	class RHIGraphicsCommandList;

	class EditorRenderer final
	{
	public:
		using OnEditorRenderCallback = Function<void()>;

	public:
		void LoadResources_RenderThread(const Ptr<RenderAPI> RenderAPI, const Ptr<RHIRenderTarget> gameRenderTarget);
		void Render_RenderThread(Ptr<RHIGraphicsCommandList> CmdList, Ptr<RHIRenderTarget> viewportRT) const;

		void RenderViewportWindow() const;

		void ShutDown() const;

		Ptr<RHIRenderTarget> GetRenderTarget() const;

		void RegisterOnEditorRenderCallback(const EditorRenderer::OnEditorRenderCallback& callback);

		EditorRenderer() = default;
		EditorRenderer(const EditorRenderer&) = delete;
		EditorRenderer(EditorRenderer&&) = delete;
		EditorRenderer& operator=(const EditorRenderer&) = delete;
		EditorRenderer& operator=(EditorRenderer&&) = delete;

	private:
		Ptr<RHIRenderTarget> mpEditorRenderTarget;
		ID3D12DescriptorHeap* mpResourceDescriptorHeap;
		ID3D12Resource* mpViewportImageResource;

		Vector<OnEditorRenderCallback> OnEditorRenderCallbacks{};

		static size_t SRVDescriptorHandleIncrementSize;

		constexpr static Vector4f StatEditorClearColour = { 0.0f, 0.0f, 0.0f, 1.0f };

	private:
		void Dx12CreateViewportSRVFromGameRenderTarget(const Ptr<RenderAPI> renderAPI, const Ptr<RHIRenderTarget> viewportRT);
		void CopyGameRenderTargetToViewportSRV(Ptr<RHIGraphicsCommandList> cmdList, Ptr<RHIRenderTarget> gameRenderTarget) const;
		void Dx12RenderToTarget(Ptr<RHIGraphicsCommandList> CmdList) const;
		void RenderWidgets() const;
	};

	class Editor final
	{
	public:
		const EditorRenderer& GetRenderer() const;
		EditorRenderer& GetRenderer();

		void RegisterOnEditorRenderCallback(const EditorRenderer::OnEditorRenderCallback& callback);

		Editor() = default;
		~Editor() = default;
		Editor(const Editor&) = delete;
		Editor(Editor&&) = delete;
		Editor& operator=(const Editor&) = delete;
		Editor& operator=(Editor&&) = delete;

	private:
		EditorRenderer Renderer;
	};

	using EditorLocator = Locator<Editor>;
}

#endif

