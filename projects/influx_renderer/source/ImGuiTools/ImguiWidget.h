#pragma once

namespace influx::GUI
{
	class IWidget
	{
	public:
		virtual void Update() {};
		virtual void Render() const = 0;
	};
}


