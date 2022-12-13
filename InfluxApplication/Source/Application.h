#pragma once

#include "../ImGui/imgui.h"

#include "Renderer.h"
#include "Common.h"

namespace Influx::Application
{
	enum class EApplicationType
	{
		Minimal,		// Only has an Update loop
		Windowed,		// Only has an Update loop + Window & events
		ImGuiApp		// Only has an Update loop + Window & events + Imgui hooks
	};

	class Application final
	{
	public:
		struct TimeStats final
		{
			using time_type = float;
			using avg_cnt_type = uint32_t; // If we count past type::max_value(), we reset...

			enum class EStat
			{
				UIRender,
				Update,
				Present,
				Frame,
				Max
			};

			constexpr static size_t k_EnumSize = static_cast<size_t>(EStat::Max);
			static constexpr char const* k_StatToNames[k_EnumSize]
			{
				"UIRender",
				"Update",
				"Present",
				"Frame"
			};

			Array<time_type, k_EnumSize> Values{};
			Array<time_type, k_EnumSize> ValueSums{};
			Array<avg_cnt_type, k_EnumSize> AverageCounter{};

			// Adds a value to Values
			template <EStat _S>
			void AddValue(const time_type value)
			{
				constexpr size_t idx = static_cast<size_t>(_S);
				if (AverageCounter[idx] + 1u == 0u) Reset();

				++AverageCounter[idx];
				Values[idx] = value;
				ValueSums[idx] += value;
			}

			// Gets latest value from Values
			template <EStat _S>
			time_type GetValue() const
			{
				constexpr size_t idx = static_cast<size_t>(_S);
				return Values[idx];
			}

			// Gets average value from ValueSums
			template <EStat _S>
			time_type GetAverage() const
			{
				constexpr size_t idx = static_cast<size_t>(_S);
				avg_cnt_type averageCounter = AverageCounter[idx];

				if (averageCounter == 0u) return 0.0;
				return ValueSums[idx] / averageCounter;
			}

			// Reset all values
			void Reset()
			{
				Values = {};
				ValueSums = {};
				AverageCounter = {};
			}
		};

	public:
		struct Settings final
		{
			EApplicationType Type = EApplicationType::Minimal;
			String Name = "InfluxApp";
			Math::Vectoru2 WindowDimensions = { 640u, 480u };
		};

		Application(int argc, char** argv, const Settings& creationSettings);
		
		void Run();
		void Quit();

		virtual void OnUpdate() {};
		virtual void OnUIRender();
		virtual void OnResize() {};

		Application(const Application&) = delete;
		Application(Application&&) = delete;
		Application& operator=(const Application&) = delete;
		Application& operator=(Application&&) = delete;
		virtual ~Application();

		bool GetHasStarted() const;
		bool GetShouldQuit() const;
		bool GetHasWindow() const;
		bool GetHasUIRenderer() const;

		const Settings& GetSettings() const;
		const Settings& GetCreationSettings() const;

		const TimeStats& GetTimeStats() const;

	private:
		bool m_isInitialized;
		bool m_hasStarted;
		bool m_shouldQuit;

		const Settings m_creationSettings;
		Settings m_currentSettings;
		
		void Initialize();
		void Update();

		void UIRender();
		void UIRender_ApplicationUI();
		
		void CreateWindow();

		uint64_t m_frame;
		float m_time;
		float m_deltaTime;

		TimeStats m_timeStats;

		Time::TimePoint m_beforeFrame = Influx::Time::Now();
		Time::TimePoint m_beforeUpdate = Influx::Time::Now();
		Time::TimePoint m_beforeUIRender = Influx::Time::Now();
		Time::TimePoint m_beforePresent = Influx::Time::Now();

		class ImGuiRendererDx12* mp_renderer{};

#if PLATFORM_WINDOWS
		HWND m_windowHandle;
		static LRESULT CALLBACK WindowsProcedure(::HWND hWnd, ::UINT uMsg, ::WPARAM wParam, ::LPARAM lParam);
		static Application* sp_currentApplicationInstance;
#endif
	};
}


