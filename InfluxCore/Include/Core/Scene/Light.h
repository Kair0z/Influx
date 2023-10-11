#pragma once

#ifndef __CORE_SCENE_LIGHT_H_
#define __CORE_SCENE_LIGHT_H_

namespace influx::scene
{
	enum class e_light_type
	{
		directional,
		point,
		spot,
		maximum
	};

	namespace detail
	{
		class i_light
		{
		public:

		private:

		};

		template <e_light_type _E>
		class light final : public i_light
		{

		};
	}

	using light = detail::i_light;
	using DirectionalLight = detail::light<e_light_type::directional>;
	using PointLight = detail::light<e_light_type::point>;
	using SpotLight = detail::light<e_light_type::spot>;
}

#endif