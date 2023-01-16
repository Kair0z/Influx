#pragma once

namespace Influx::GUI
{
	namespace Internal
	{
		class IWidget
		{
		public:
			virtual void Render() const = 0;
		};
	}
}


