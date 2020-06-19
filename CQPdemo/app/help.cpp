#include "help.h"
#include <sstream>
#include <algorithm>

#include "utils/string_util.h"

namespace help
{

command msgDispatcher(const char* msg)
{
	command c;
	auto query = msg2args(msg);
	if (query.empty()) return c;

	auto cmd = query[0];
	if (commands_str.find(cmd) == commands_str.end()) return c;

	c.args = query;
	switch (c.c = commands_str[cmd])
	{
	case commands::help:
		c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
		{
			return help();
		};
		break;

	default:
		break;
	}
	return c;
}

std::string boot_info()
{
    std::stringstream ss;
    ss << "bot活了！";
    ss << help();
	return ss.str();
}

std::string help()
{
    std::stringstream ss;
    ss << "最后更新日期：" << __DATE__ << " " __TIME__ << std::endl <<
        "帮助文档：https://github.com/yaasdf/bigbedbot-cqvc/blob/master/CQPdemo/README.md";
    return ss.str();
}
}
