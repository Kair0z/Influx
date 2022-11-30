#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include "Core/Math/Vector.h"

namespace InfluxCoreTests
{
	TEST_CLASS(InfluxCoreTests)
	{
	public:
		TEST_METHOD(Vector)
		{
			using namespace Influx;

			// Sizing
			{
				Math::Vector<bool, -1>		vb_min	{ 1, 0, 2, 0, 5 }; // Size >> 255
				Math::Vector<bool, 358u>	vb_358	{ 1, 0, 2, 0, 5 }; // Size >> 358 - 256 == 102
				Math::Vector<bool, 4u>		vb_4	{ 1, 0, 2, 0, 5 }; // Size >> 4

				Assert::AreEqual(vb_min.Size(), static_cast<Math::VectorSizeType>(-1));
				Assert::AreEqual(vb_358.Size(), static_cast<Math::VectorSizeType>(358));
				Assert::AreEqual(vb_4.Size(),	static_cast<Math::VectorSizeType>(4));
			}
			
			// Unsigned
			{
				Math::Vector<uint8_t, 0u>		vu_8_0{ 1 };
				Math::Vector<uint8_t, 1u>		vu_8_1{ 2 };
				Math::Vector<uint16_t, 2u>		vu_16_2{ -1, -2 };
				Math::Vector<uint32_t, 3u>		vu_32_3{ 1, 2, 3 };
				Math::Vector<uint64_t, 4u>		vu_64_4{ 1, 2, 3, 4 };
			}

			// Signed
			{
				Math::Vector<int8_t, 0u>		vs_8_0{ 1 };
				Math::Vector<int8_t, 1u>		vs_8_1{ 2 };
				Math::Vector<int16_t, 2u>		vs_16_2{ -1, 2 };
				Math::Vector<int32_t, 3u>		vs_32_3{ 1, 2, 3 };
				Math::Vector<int64_t, 4u>		vs_64_4{ 1 };
			}
			
			// Float & Double
			{
				Math::Vector<float, 2u>		vf_2{};
				Math::Vector<double, 3u>		vd_3{};
				Math::Vector<long double, 4u>		vld_4{};
			}

			Math::Vector<float, 2u>		vf_2{};
			Math::Vector<float, 3u>		vf_3{};
			Math::Vector<float, 4u>		vf_4{};

			Math::Vector<bool,	2u>		vb_2{};
			Math::Vector<bool,	3u>		vb_3{};
			Math::Vector<bool,	4u>		vb_4{};

			// Copy Constructor
			{
				Math::Vector<float, 3u>		c_vf_3{ vf_3 };
				Math::Vector<float, 3u>		c_vf_2{ vf_2 }; // copy-count-down-cast
				Math::Vector<float, 3u>		c_vf_4{ vf_4 }; // copy-count-up-cast

				Math::Vector<float, 3u>		c_vb_3{ vb_3 }; // copy-type-cast
				Math::Vector<float, 3u>		c_vb_2{ vb_2 };	// copy-type-count-down-cast
				Math::Vector<float, 3u>		c_vb_4{ vb_4 }; // copy-type-count-up-cast

				Math::Vector<float, 3u>		m_vf_3{ Math::Vector<float, 3u>{} };
				Math::Vector<float, 3u>		m_vf_2{ Math::Vector<float, 2u>{} }; // move-count-down-cast
				Math::Vector<float, 3u>		m_vf_4{ Math::Vector<float, 4u>{} }; // move-count-up-cast

				Math::Vector<float, 3u>		m_vb_3{ Math::Vector<bool, 3u>{} }; // move-type-cast
				Math::Vector<float, 3u>		m_vb_2{ Math::Vector<bool, 2u>{} };	// move-type-count-down-cast
				Math::Vector<float, 3u>		m_vb_4{ Math::Vector<bool, 4u>{} }; // move-type-count-up-cast
			}
			
			// Assignement
			{
				Math::Vector<float, 2u>		c_vf_2 = vf_2;
				Math::Vector<float, 2u>		m_vf_2 = Math::Vector<float, 2u>{5.0f, 10.0f};
			}

				
		}
	};
}
