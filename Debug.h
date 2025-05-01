#include <windows.h>
#include <sstream>

#define DEBUG_PRINT(fmt, ...) \
    do { \
        std::wstringstream ss; \
        ss << L"[DEBUG] "; \
        ss << fmt; \
        ss << L"\n"; \
        OutputDebugStringW(ss.str().c_str()); \
    } while (0)
//define DEBUG_PRINT(msg) OutputDebugString((std::wstring(L"[DEBUG] ") + msg + L"\n").c_str())

//C++20
//#define DEBUG_PRINT(fmt, ...) \
    OutputDebugStringW( \
        (std::wstring(L"[DEBUG] ") + std::format(fmt, __VA_ARGS__) + L"\n").c_str() \
    )