#pragma once
#include "core/string.h"
#include "core/file.h"
#include "core/container/list.h"

// https://www.youtube.com/watch?v=DiXyTylhrGw

#include "rttr/registration.h"

namespace influx::script
{
	struct xvariable
	{
		string m_type;
		string m_id;
	};

	struct xclass
	{
		string m_name;
		path m_file;
		list<xvariable> m_vars;
	};

	struct xstruct
	{
		string m_name;
		path m_file;
		list<xvariable> m_vars;
	};

	class script
	{
	public:
		virtual void on_start() {}
		virtual void on_update() {}
		virtual const char* print() {}

	private:
		path m_file;
		vector<xclass> m_classes;
		vector<xstruct> m_structs;
	};
}

RTTR_REGISTRATION
{
	using namespace rttr;
	registration::method("f", &);
}