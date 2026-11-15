#pragma once
#include <vector>
inline std::vector<std::wstring>& commands(){static std::vector<std::wstring> c;return c;}
inline int mciSendStringW(const wchar_t*s,wchar_t*,unsigned,void*){commands().push_back(s);return 0;}
