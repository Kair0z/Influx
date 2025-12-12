#pragma once

// influx::shader
namespace influx::shader
{
	struct shader_signature;
}

namespace influx::engine
{
	class content_manager;

	class render_streamer final
	{
	public:
		void stream(const content_manager& content);

		// shaders:
		bool has_shader_loaded(const shader::shader_signature& signature) const;

		// meshes:
		bool has_mesh_loaded(const string& name) const;

		// textures:
		bool has_texture_loaded(const string& name) const;
		bool has_cubemap_loaded(const string& name) const;
		void* get_loaded_texture_id(const string& name) const;

	private:
		void stream_shaders(const content_manager& content);
		void stream_images(const content_manager& content);
		void stream_cubemaps(const content_manager& content);
		void stream_meshes(const content_manager& content);
	};
}