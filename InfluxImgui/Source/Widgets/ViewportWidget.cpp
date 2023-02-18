#include "imgui_pch.h"
#include "Widgets/ViewportWidget.h"

#include "ImGui/imgui_impl_dx12.h"
#include "InfluxGraphics/D3D12/ResourceViews/D3D12ShaderResourceView.h"

namespace Influx::GUI
{
	void ViewportWidget::Update()
	{

	}

	void ViewportWidget::Render() const
	{
		if (!IsTargetShaderResourceViewSet())
		{
			return;
		}

		Influx::Graphics::D3D12ShaderResourceView* d3d12Srv = (Influx::Graphics::D3D12ShaderResourceView*)GetTargetShaderResourceView();
		if (d3d12Srv == nullptr)
		{
			return;
		}

		const Settings& curSettings = GetCurrentSettings();
		ImGui::Image((ImTextureID)d3d12Srv->GetDxGPUHandle().ptr, curSettings.ImageSize, curSettings.Uv0, curSettings.Uv1, curSettings.ImageTint, curSettings.BorderColour);
	}

	void ViewportWidget::SetTargetShaderResourceView(SrvPtr srv)
	{
		if (srv == nullptr)
		{
			return;
		}

		m_pTargetSrv = srv;
	}

	bool ViewportWidget::IsTargetShaderResourceViewSet() const
	{
		return GetTargetShaderResourceView() != nullptr;
	}

	const ViewportWidget::SrvPtr ViewportWidget::GetTargetShaderResourceView() const
	{
		return m_pTargetSrv;
	}

	const ViewportWidget::Settings& ViewportWidget::GetCurrentSettings() const
	{
		return m_currentSettings;
	}
}

