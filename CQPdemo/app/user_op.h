#pragma once
#include <map>
#include <functional>
#include "data/user.h"

namespace user_op
{

enum class commands : size_t {
	��ͨ��ʾ,
	��ͨ,
	���,
	����,
	����,

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
	{"��ͨ", commands::��ͨ��ʾ},
	{"�_ͨ", commands::��ͨ��ʾ},  //���w��
	{"��ͨ����", commands::��ͨ��ʾ},
	{"�_ͨ����", commands::��ͨ��ʾ},  //���w��
	{"���ҿ�ͨ����", commands::��ͨ��ʾ},
	{"�o���_ͨ����", commands::��ͨ��ʾ},  //���w��
	{"ע��", commands::��ͨ��ʾ},
	{"�]��", commands::��ͨ��ʾ},  //���w��
	{"ע�Ფ��", commands::��ͨ��ʾ},
	{"�]�Բ���", commands::��ͨ��ʾ},  //���w��
	{"��Ҫע�Ფ��", commands::��ͨ��ʾ},
	{"��Ҫ�]�Բ���", commands::��ͨ��ʾ},  //���w��
	{"��Ҫ��ͨ����", commands::��ͨ},
	{"��Ҫ�_ͨ����", commands::��ͨ},  //���w��
	{"���", commands::���},
	{"�N�~", commands::���},  //���w��
	{"����", commands::����},
	{"�I��", commands::����},  //���w��
	//{"��������", commands::����},
	//{"����", commands::����},
	//{"����", commands::����},

};

////////////////////////////////////////////////////////////////////////////////
// ��p
const int FREE_BALANCE_ON_NEW_DAY = 10;
const int NEW_DAY_TIME_HOUR = 11;
const int NEW_DAY_TIME_MIN = 0;


inline int extra_tomorrow = 0;

inline time_t daily_refresh_time;   // ��������ʱ�䣬�������ʱ�����ҪС�����ʱ��
inline std::tm daily_refresh_tm;
inline std::tm daily_refresh_tm_auto;
const int DAILY_BONUS = 100;
inline int remain_daily_bonus;

void flushDailyTimep(bool autotriggered = false);

command msgDispatcher(const char* msg);
}