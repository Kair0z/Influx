#pragma once

#include "core/math/math.h"

namespace influx::artscii
{
    // [======C . . . .]
    class progress_bar final
    {
    public:
        static constexpr uint32 k_max_characters = 512u;
        struct settings final
        {
            char m_cursor_char = 'C';
            char m_done_chars[2] = { '=', ' ' };
            char m_todo_chars[2] = { '.', ' ' };
            bool m_todo_alternate = true;
            bool m_done_alternate = false;
            bool m_ceil = true;
            uint32 m_bar_length = 12u;

            settings& set_cursor(char ch)
            {
                m_cursor_char = ch;
                return *this;
            }
            settings& set_done(char ch, bool alt = false)
            {
                m_done_chars[alt ? 1u:0u] = ch;
                return *this;
            }
            settings& set_todo(char ch, bool alt = false)
            {
                m_todo_chars[alt ? 1u:0u] = ch;
                return *this;
            }
            settings& set_length(uint32 length)
            {
                m_bar_length = length;
                return *this;
            }
        };

        settings m_settings{};
        float m_t = 0.0f;
        char m_cstring[k_max_characters]{};

    public:
        progress_bar(float init_pc = 0.0f, const settings& settings = {})
        {
            m_t = init_pc;
            m_settings = settings;
        }

        void reset()
        {
            m_t = 0.0f;
        }

        uint32& bar_length()
        {
            return m_settings.m_bar_length;
        }
        char& done_char(bool alternate = false)
        {
            return m_settings.m_done_chars[alternate ? 1u : 0u];
        }
        char& todo_char(bool alternate = false)
        {
            return m_settings.m_todo_chars[alternate ? 1u : 0u];
        }
        char& cursor_char()
        {
            return m_settings.m_cursor_char;
        }
        float& pc()
        {
            return m_t;
        }
        settings& get_settings()
        {
            return m_settings;
        }

        progress_bar& increment(uint32 block)
        {
            m_t += 1.0f / (float)m_settings.m_bar_length;
            return *this;
        }
        progress_bar& increment(float pc)
        {
            m_t += pc;
            return *this;
        }
        progress_bar& operator+=(uint32 block)
        {
            return increment(block);
        }
        progress_bar& operator+=(float pc)
        {
            return increment(pc);
        }
        
        inline const char* get_cstr()
        {
            const float t = math::clamp(m_t, 0.0f, 1.0f);

            uint32 progress_index = m_settings.m_ceil ?
                math::ceil<uint32, float>(t * m_settings.m_bar_length) :
                math::round<uint32, float>(t * m_settings.m_bar_length);
            
            uint32 i = 0u;
            m_cstring[i++] = '[';
            for (i; i < m_settings.m_bar_length - 1; ++i)
            {
                char character{};
                if (i < progress_index)
                {
                    const bool alternate = m_settings.m_done_alternate;
                    character = m_settings.m_done_chars[ (alternate ? i % 2 : 0) ];
                }
                else if (i == progress_index)
                {
                    character = m_settings.m_cursor_char;
                }
                else
                {
                    const bool alternate = m_settings.m_todo_alternate;
                    character = m_settings.m_todo_chars[(alternate ? i % 2 : 0)];
                }
                m_cstring[i] = character;
            }

            m_cstring[i++] = ']';
            m_cstring[m_settings.m_bar_length] = '\0';
            
            return m_cstring;
        }
    };
}