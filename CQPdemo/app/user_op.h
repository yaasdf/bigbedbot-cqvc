#pragma once
#include <map>
#include <functional>
#include "data/user.h"

namespace user_op
{

enum class commands : size_t {
	开通提示,
	开通,
	余额,
	领批,
	生批,

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
	{"开通", commands::开通提示},
	{"開通", commands::开通提示},  //繁體化
	{"开通菠菜", commands::开通提示},
	{"開通菠菜", commands::开通提示},  //繁體化
	{"给我开通菠菜", commands::开通提示},
	{"給我開通菠菜", commands::开通提示},  //繁體化
	{"注册", commands::开通提示},
	{"註冊", commands::开通提示},  //繁體化
	{"注册菠菜", commands::开通提示},
	{"註冊菠菜", commands::开通提示},  //繁體化
	{"我要注册菠菜", commands::开通提示},
	{"我要註冊菠菜", commands::开通提示},  //繁體化
	{"我要开通菠菜", commands::开通},
	{"我要開通菠菜", commands::开通},  //繁體化
	{"余额", commands::余额},
	{"餘額", commands::余额},  //繁體化
	{"领批", commands::领批},
	{"領批", commands::领批},  //繁體化
	//{"无中生批", commands::生批},
	//{"加批", commands::生批},
	//{"生批", commands::生批},

};

////////////////////////////////////////////////////////////////////////////////
// 领p
const int FREE_BALANCE_ON_NEW_DAY = 10;
const int NEW_DAY_TIME_HOUR = 11;
const int NEW_DAY_TIME_MIN = 0;


inline int extra_tomorrow = 0;

inline time_t daily_refresh_time;   // 可领批的时间，最后领批时间戳需要小于这个时间
inline std::tm daily_refresh_tm;
inline std::tm daily_refresh_tm_auto;
const int DAILY_BONUS = 100;
inline int remain_daily_bonus;

void flushDailyTimep(bool autotriggered = false);

command msgDispatcher(const char* msg);
}