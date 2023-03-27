#pragma once

#ifndef __CORE_MACROS_H_
#define __CORE_MACROS_H_

#define FLX_CORE_GET(Type, Name) \
		private: \
			Type Name; \
		public: \
			const Type& Get##Name() const \
			{ \
				return Name; \
			} \
		private: // default to private...

#define FLX_CORE_GET_SET(Type, Name) \
		private: \
			Type Name; \
		public: \
			const Type& Get##Name() const \
			{ \
				return Name; \
			} \
			\
			void Set##Name(const Type& newValue) \
			{ \
				Name = newValue; \
			} \
		private: // default to private...

#endif