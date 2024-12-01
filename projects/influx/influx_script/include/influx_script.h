#pragma once

#include "core/string.h"
#include "core/file.h"
#include "core/container/list.h"

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
		file m_file;
		list<xvariable> m_vars;
	};

	struct xstruct
	{
		string m_name;
		file m_file;
		list<xvariable> m_vars;
	};

	class script
	{
	public:

	private:
		file m_file;
		vector<xclass> m_classes;
		vector<xstruct> m_structs;
	};
}