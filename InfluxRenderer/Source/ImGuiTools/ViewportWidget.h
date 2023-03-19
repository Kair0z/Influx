#pragma once

#include "Core/Math/Vector.h"
#include "ImguiWidget.h"

#include "ImGui/imgui.h"

namespace Influx::Graphics
{
	class RHIShaderResourceView;
}

namespace Influx::GUI
{
	class ViewportWidget final : public IWidget
	{
	public:
		struct Settings final
		{
			ImVec2 ImageSize{};
			ImVec2 Uv0 { 0, 0 };
			ImVec2 Uv1 { 1, 1 };

			ImVec4 ImageTint = { 1, 1, 1, 1 };
			ImVec4 BorderColour = { 1, 1, 1, 1 };

			Settings() = default;
			Settings(const Math::Vectorf2& imageSize, const Math::Vectorf2& uv0, const Math::Vectorf2& uv1, const Math::Vectorf4& tint, const Math::Vectorf4& borderColour)
				: ImageSize{ imageSize.x, imageSize.y }, Uv0{ uv0.x, uv0.y }, Uv1{ uv1.x, uv1.y }, ImageTint{ tint.r, tint.g, tint.b, tint.a }, BorderColour{ borderColour.r, borderColour.g, borderColour.b, borderColour.a } {}
		};

	private:
		using SrvPtr = Influx::Graphics::RHIShaderResourceView*;
		SrvPtr m_pTargetSrv;
		
		Settings m_currentSettings;

	private:
		virtual void Update() override;
		virtual void Render() const override;

	public:
		void SetTargetShaderResourceView(SrvPtr srv);
		bool IsTargetShaderResourceViewSet() const;
		const SrvPtr GetTargetShaderResourceView() const;

		const Settings& GetCurrentSettings() const;
	};
}


