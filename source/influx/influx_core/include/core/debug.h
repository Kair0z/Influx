#pragma once

#include <cassert>
#include <iostream>
namespace influx
{
#define influx_assert(x) assert(x)
#define influx_assert_msg(expr, msg) \
    do { \
        if (!(expr)) { \
            std::wcerr << L"Assertion failed: " << (msg) << L"\n"; \
            assert(expr); \
        } \
    } while (0)

#define influx_assert_not_null(x) assert(x != nullptr)
#define influx_todo(x) influx_assert(false, x)

#define influx_delete(x) \
	if (x != nullptr) { delete x; x = nullptr; }
}