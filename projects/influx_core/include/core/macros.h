#pragma once

#ifndef __CORE_MACROS_H_
#define __CORE_MACROS_H_

#define FLX_CORE_GET(Type, Name) \
		private: \
			Type m_##Name; \
		public: \
			const Type& get_##Name() const \
			{ \
				return m_##Name; \
			} \
		private: // default to private...

#define FLX_CORE_GET_SET(Type, Name) \
		private: \
			Type m_##Name; \
		public: \
			const Type& get_##Name() const \
			{ \
				return m_##Name; \
			} \
			\
			void set_##Name(const Type& new_value) \
			{ \
				m_##Name = new_value; \
			} \
		private: // default to private...

#endif