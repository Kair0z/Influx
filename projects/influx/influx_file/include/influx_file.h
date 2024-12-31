#pragma once

#if _DLL
#define INFLUX_FILE_API __declspec(dllexport)
#else
#define INFLUX_FILE_API __declspec(dllimport)
#endif

// influx::core
#include "core/basetypes.h"
#include "core/file.h"
#include <fstream>

namespace influx::files
{
	class componentfile final
	{
	public:
		void INFLUX_FILE_API save(const file& file);
		void INFLUX_FILE_API load(const file& file);

		string m_name;
	};

	class entityfile final
	{
	public:
		void INFLUX_FILE_API save(const file& file);
		void INFLUX_FILE_API load(const file& file);
		void INFLUX_FILE_API clear();

		string m_name;
		vector<componentfile> m_components{};
	};

	class projectfile final
	{
	public:
		void INFLUX_FILE_API save(const file& file);
		void INFLUX_FILE_API load(const file& file);
		void INFLUX_FILE_API clear();

		string m_name;
		vector<entityfile> m_entities{};
	};
}