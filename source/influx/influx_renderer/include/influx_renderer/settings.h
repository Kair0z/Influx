
namespace influx::renderer
{
	struct render_settings final
	{
		enum class cullmode { back, front, none };
		cullmode	m_cullmode = cullmode::back;
		bool		m_wireframe = false;
	};
}