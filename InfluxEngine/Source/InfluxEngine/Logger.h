#pragma once

#ifndef _H_LOGGER_
#define _H_LOGGER_

#include "spdlog/spdlog.h"

namespace Influx
{
	class Logger final
	{
	public:
		template<typename... Args>
		static inline void Info(const String& info, Args &&... args) noexcept
		{
			spdlog::info(info, std::forward<Args>(args)...);
		}

		template<typename... Args>
		static inline void Error(const String& error, Args &&... args) noexcept
		{
			spdlog::error(error, std::forward<Args>(args)...);
		}

		template<typename... Args>
		static inline void Warn(const String& warn, Args &&... args) noexcept
		{
			spdlog::warn(warn, std::forward<Args>(args)...);
		}

		template<typename... Args>
		static inline void Debug(const String& debug, Args &&... args) noexcept
		{
			#if DEBUG
			spdlog::debug(debug, std::forward<Args>(args)...);
			#endif
		}
	};
}

#endif
