#pragma once
#include "core/basetypes.h"
#include "core/container/map.h"
#include "core/pointer.h"
#include "core/math/colour.h"

namespace influx
{
    enum class e_texture_semantic : uint8
    {
        none = 0,
        diffuse = 1,
        specular = 2,
        ambient = 3,
        emissive = 4,
        height = 5,
        normals = 6,
        roughness = 7,
        opacity = 8,
        displacement = 9,
        lightmap = 10,
        reflection = 11,

        /** PBR Materials
        * PBR definitions from maya and other modelling packages now use this standard.
        * This was originally introduced around 2012.
        * Support for this is in game engines like Godot, Unreal or Unity3D.
        * Modelling packages which use this are very common now.
        */
        basecolor = 12,
        worldnormal = 13,
        metalness = 14,
        ambientocclusion = 15,

        /** PBR Material Modifiers
        * Some modern renderers have further PBR modifiers that may be overlaid
        * on top of the 'base' PBR materials for additional realism.
        * These use multiple texture maps, so only the base type is directly defined
        */
        unknown = 18, // unknown

        /** Sheen
        * Generally used to simulate textiles that are covered in a layer of microfibers
        * eg velvet
        * https://github.com/KhronosGroup/glTF/tree/master/extensions/2.0/Khronos/KHR_materials_sheen
        */
        sheen = 19,

        /** Clearcoat
        * Simulates a layer of 'polish' or 'lacquer' layered on top of a PBR substrate
        * https://autodesk.github.io/standard-surface/#closures/coating
        * https://github.com/KhronosGroup/glTF/tree/master/extensions/2.0/Khronos/KHR_materials_clearcoat
        */
        clearcoat = 20,

        /** Transmission
        * Simulates transmission through the surface
        * May include further information such as wall thickness
        */
        transmission = 21,
        count
    };

    enum class e_cullmode : uint8
    {
        front,
        back,
        nocull,
        count
    };

	class material final
	{
        enum class e_property_type : uint8
        {
            scalar,
            integer,
            texture,
            count
        };

	public:
        material() = default;

        struct float_property
        {
            float m_value;
        };

        struct texture_property
        {
            e_texture_semantic m_semantic;
            uint32 m_texture_index;
            string m_path;
            
            const string& get_name() const
            {
                return m_path;
            }
        };

        struct int_property
        {
            int m_value;
        };

        void add_int(const string& name, const int_property& new_int)
        {
            m_integers[name] = new_int;
        }

        void add_scalar(const string& name, const float_property& new_scalar)
        {
            m_floats[name] = new_scalar;
        }

        void add_texture(const e_texture_semantic& slot, const texture_property& new_texture)
        {
            m_textures[slot] = new_texture;
        }

        inline texture_property const* get_texture(const e_texture_semantic& slot) const
        {
            if (m_textures.contains(slot))
            {
                return &m_textures.at(slot);
            }

            return nullptr;
        }
        texture_property const* get_texture_normals() const { return get_texture(e_texture_semantic::normals); }
        texture_property const* get_texture_diffuse() const { return get_texture(e_texture_semantic::diffuse); }

        inline const string& get_texture_name(const e_texture_semantic& slot) const
        {
            texture_property const* texture = get_texture(slot);
            if (texture)
            {
                return texture->get_name();
            }

            return "";
        }
        const string& get_texture_normals_name() const { return get_texture_name(e_texture_semantic::normals); }
        const string& get_texture_diffuse_name() const { return get_texture_name(e_texture_semantic::diffuse); }

        math::colour_rgba get_basecolour() const
        {
            return m_basecolour;
        }

        void set_basecolour(const math::colour_rgba& colour)
        {
            m_basecolour = colour;
        }

	private:
        umap<string, float_property> m_floats;
        umap<e_texture_semantic, texture_property> m_textures;
        umap<string, int_property> m_integers;

        math::colour_rgba m_basecolour;

        // settings
        bool m_render_depth;
        bool m_render_stencil;
        e_cullmode m_cullmode;
    };
}