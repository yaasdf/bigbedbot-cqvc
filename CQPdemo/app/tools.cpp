#include <sstream>

#include "tools.h"
#include "utils/rand.h"
#include "utils/string_util.h"

#include "cqp.h"

namespace tools
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
	case commands::roll:
		c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
		{
			int max = 100;
			if (args.size() > 1)
			{
				max = atoi(args[1].c_str());
				if (max < 0) max = 100;
			}
			return std::to_string(randInt(0, max));
		};
		break;

	default: break;
	}
	return c;
}

}