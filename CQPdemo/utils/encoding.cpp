#include "encoding.h"

#define byte win_byte_override 
#include <Windows.h>
#undef byte
std::string gbk2utf8(std::string gbk)
{
	int len = MultiByteToWideChar(CP_ACP, 0, gbk.c_str(), -1, NULL, 0);
	wchar_t wstr[512];
	memset(wstr, 0, sizeof(wstr));

	MultiByteToWideChar(CP_ACP, 0, gbk.c_str(), -1, wstr, len);
	len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);

	char str[1024];
	memset(str, 0, sizeof(str));
	WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);

	return std::string(str);
}

std::string utf82gbk(std::string utf8)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, NULL, 0);
	wchar_t wstr[512];
	memset(wstr, 0, sizeof(wstr));

	MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, wstr, len);
	len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);

	char str[1024];
	memset(str, 0, sizeof(str));
	WideCharToMultiByte(CP_ACP, 0, wstr, -1, str, len, NULL, NULL);

	return std::string(str);
}