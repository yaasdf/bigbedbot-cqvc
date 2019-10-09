#include "utils.h"
#include <sstream>

//////////////////////////////////
std::vector<std::string> msg2args(const char* msg)
{
    std::vector<std::string> query;
    if (msg == nullptr) return query;
    std::stringstream ss(msg);
    while (ss)
    {
        std::string s;
        ss >> s;
        if (!s.empty())
            query.push_back(s);
    }
    return query;
}

#define byte win_byte_override 
#include <Windows.h>
#undef byte
std::string gbk2utf8(std::string gbk)
{
    int len = MultiByteToWideChar(CP_ACP, 0, gbk.c_str(), -1, NULL, 0);
    wchar_t wstr[128];
    memset(wstr, 0, sizeof(wstr));

    MultiByteToWideChar(CP_ACP, 0, gbk.c_str(), -1, wstr, len);
    len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);

    char str[256];
    memset(str, 0, sizeof(str));
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);

    return std::string(str);
}

std::string utf82gbk(std::string utf8)
{
    int len = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, NULL, 0);
    wchar_t wstr[128];
    memset(wstr, 0, sizeof(wstr));

    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, wstr, len);
    len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);

    char str[256];
    memset(str, 0, sizeof(str));
    WideCharToMultiByte(CP_ACP, 0, wstr, -1, str, len, NULL, NULL);

    return std::string(str);
}

std::string strip(std::string& s)
{
    int end = (int)s.length() - 1;
    int start = 0;
    while (start < (int)s.length() - 1 && (s[start] == ' ')) ++start;
    while (end > start && (s[end] == ' ' || s[end] == '\n' || s[end] == '\r')) --end;
    return s.substr(start, end + 1 - start);
}

int64_t stripAt(std::string& s)
{
    if (s.find("[CQ:at,qq=") == 0 && s.rfind("]") != -1)
    {
        try {
            return std::stoll(s.substr(10, s.length() - 11));
        }
        catch (std::exception&) {
            //ignore
        }
    }
    else
    {
        return 0;
    }
}