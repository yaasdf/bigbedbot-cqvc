#pragma once

#include <functional>
#include <vector>
#include <string>
#include <map>
#include <cstdint>

namespace smoke
{

enum class commands : size_t {
	禁烟,
	解禁,

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
	{"禁言", commands::禁烟},  //簡體繁體一樣
	{"禁烟", commands::禁烟},
	{"禁菸", commands::禁烟},  //繁體化
	{"禁煙", commands::禁烟},  //烟字有四种写法
	{"解禁", commands::解禁},  //簡體繁體一樣
	//{"无中生批", commands::生批},
	//{"加批", commands::生批},
	//{"生批", commands::生批},

};

////////////////////////////////////////////////////////////////////////////////
// 烟
command msgDispatcher(const char* msg);

enum class RetVal
{
	OK,
	ZERO_DURATION,
	TARGET_IS_ADMIN,
	INVALID_DURATION,
	TARGET_NOT_FOUND,
};

RetVal nosmoking(int64_t group, int64_t target, int duration);
std::string nosmokingWrapper(int64_t qq, int64_t group, int64_t target, int64_t cost);
int64_t getTargetFromStr(const std::string& targetName, int64_t group = 0);
std::string selfUnsmoke(int64_t qq);

inline std::map<int64_t, std::map<int64_t, time_t>> smokeTimeInGroups;

const double UNSMOKE_COST_RATIO = 3;
}