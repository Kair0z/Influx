#pragma once

#ifndef _CORE_PROCEDURE_H_
#define _CORE_PROCEDURE_H_

#include "Core/Time.h"

#include <thread>
#include <mutex>
#include <thread>
#include <condition_variable>

namespace Influx
{
	class Procedure
	{
	public:
		/* At construction */
		virtual void OnStart() {};

		virtual void OnTick() {};

		/* At destruction */
		virtual void OnQuit() {};

		const std::thread& GetInternalThreadObject() const { return m_stdThread; }
		uint64_t GetTickCount() const { return m_tickCount; };
		bool IsQuit() const { return mb_isQuit; };


		Procedure();
		Procedure(const Procedure&) = default;
		Procedure(Procedure&&) = default;
		Procedure& operator=(const Procedure&) = default;
		Procedure& operator=(Procedure&&) = default;
		virtual ~Procedure() { if (!mb_isQuit) OnQuit(); }

	protected:
		std::thread m_stdThread;
		std::atomic_bool mb_isQuit = false;

	private:
		uint64_t m_tickCount{};
		Time::TimePoint m_lastTick;

		double m_startMsDuration{};
		double m_tickMsDuration{};
	};

	Procedure::Procedure()
	{
		{
			Time::TimePoint startStart = Time::Now();
			OnStart();
			m_startMsDuration = Time::MsBetween<double>(Time::Now(), startStart);
		}

		m_lastTick = Time::Now();
		while (!mb_isQuit)
		{
			Time::TimePoint tickStart = Time::Now();
			OnTick();
			m_tickMsDuration = Time::MsBetween<double>(Time::Now(), tickStart);

			++m_tickCount;
		}

		mb_isQuit = true;
		OnQuit();
	}
}

#endif