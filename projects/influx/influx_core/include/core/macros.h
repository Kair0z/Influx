#pragma once

#ifndef __CORE_MACROS_H_
#define __CORE_MACROS_H_

#define influx_property_read(Type, Name) \
		private: \
			Type m_##Name; \
		public: \
			const Type& get_##Name() const \
			{ \
				return m_##Name; \
			} \
		private: // default to private...

#define influx_property_readwrite(Type, Name) \
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


#define INFLUX_NO_COPY(clss) clss(const clss&) = delete; clss& operator=(const clss&) = delete;
#define INFLUX_NO_MOVE(clss) clss(clss&&) = delete; clss& operator=(clss&&) = delete;
#endif