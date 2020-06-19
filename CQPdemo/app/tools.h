#pragma once
#include <functional>
#include <vector>
#include <string>
#include <map>

#include "utils/string_util.h"

namespace tools
{

enum class commands : size_t {
	roll,

};
typedef std::function<std::string(::int64_t, ::int64_t, std::vector<std::string>&, const char*)> callback;
//typedef std::string(*callback)(::int64_t, ::int64_t, std::vector<std::string>);
struct command
{
	commands c = (commands)0;
	std::vector<std::string> args;
	callback func = nullptr;
};

inline std::map<std::string, commands> commands_str
{
	{"!roll", commands::roll},
	{"/roll", commands::roll},
};

////////////////////////////////////////////////////////////////////////////////
command msgDispatcher(const char* msg);
}