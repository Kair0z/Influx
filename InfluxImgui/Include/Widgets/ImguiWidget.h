#pragma once

namespace Influx::GUI
{
	class IWidget
	{
	public:
		virtual void Update() {};
		virtual void Render() const = 0;
	};
}


