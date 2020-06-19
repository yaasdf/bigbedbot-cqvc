#include <sstream>

#include "user_op.h"
#include "data/group.h"
#include "utils/rand.h"
#include "utils/time_util.h"
#include "utils/string_util.h"
#include "cpp-base64/base64.h"

namespace user_op
{

using user::plist;

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
	case commands::开通提示:
		c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
		{
			if (plist.find(qq) != plist.end()) return std::string(CQ_At(qq)) + "，你已经注册过了";

			return "是我要开通菠菜，你会不会开通菠菜";
		};
		break;
	case commands::开通:
		c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
		{
			if (plist.find(qq) != plist.end()) return std::string(CQ_At(qq)) + "，你已经注册过了";
			auto &p = plist[qq];

			p.createAccount(user::INITIAL_BALANCE);

			std::stringstream ss;
			ss << "你可以开始开箱了，送给你" << plist[qq].getCurrency() << "个批";
			return ss.str();
		};
		break;
	case commands::余额:
		c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
		{
			if (plist.find(qq) == plist.end()) return std::string(CQ_At(qq)) + "，你还没有开通菠菜";
			auto &p = plist[qq];
			std::stringstream ss;
			ss << CQ_At(qq) << "，你的余额为" << p.getCurrency() << "个批，"
				<< p.getKeyCount() << "把钥匙";
			auto[enough, stamina, rtime] = p.getStamina(0);
			ss << "\n你还有" << stamina << "点体力";
			if (stamina < user::MAX_STAMINA)
				ss << "，回满还需" << rtime / (60 * 60) << "小时" << rtime / 60 % 60 << "分钟";
			return ss.str();
		};
		break;
	case commands::领批:
		c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
		{
			if (plist.find(qq) == plist.end()) return std::string(CQ_At(qq)) + "，你还没有开通菠菜";
			if (!grp::groups[group].getFlag(grp::Group::MASK_DAILYP)) return "本群被隔离了，么得领批/cy";

			auto &p = plist[qq];

			if (p.getLastDrawTime() > daily_refresh_time)
			{
				std::stringstream ss;
				ss << CQ_At(qq) << "，你今天已经领过了，明天再来8";
				return ss.str();
			}

			std::stringstream ss;
			ss << CQ_At(qq) << "，你今天领到" << FREE_BALANCE_ON_NEW_DAY << "个批";
			if (remain_daily_bonus)
			{
				int bonus = randInt(1, remain_daily_bonus > 66 ? 66 : remain_daily_bonus);
				remain_daily_bonus -= bonus;
				p.modifyCurrency(FREE_BALANCE_ON_NEW_DAY + bonus);

				ss << "，甚至还有先到的" << bonus << "个批\n"
					<< "现在批池还剩" << remain_daily_bonus << "个批";
			}
			else
			{
				p.modifyCurrency(FREE_BALANCE_ON_NEW_DAY);
				ss << "\n每日批池么得了，明天请踩点";
			}
			p.modifyDrawTime(time(nullptr));
			return ss.str();
			/*
			if (plist[qq].last_draw_time > daily_time_point)
			if (remain_daily_bonus > 0)
			{
				int bonus = randInt(0, remain_daily_bonus);
				remain_daily_bonus -= bonus;
			}
			*/
			return "";
		};
		break;
	case commands::生批:
		c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
		{
			if (plist.find(qq) == plist.end()) return std::string(CQ_At(qq)) + "，你还没有开通菠菜";
			auto &p = plist[qq];

			auto[enough, stamina, rtime] = p.modifyStamina(1);

			std::stringstream ss;
			if (!enough) ss << CQ_At(qq) << "，你的体力不足，回满还需"
				<< rtime / (60 * 60) << "小时" << rtime / 60 % 60 << "分钟";

			p.modifyCurrency(1);

			return std::string(CQ_At(qq)) + "消耗1点体力得到1个批";
		};
		break;
	default: break;
	}

	return c;
}

void flushDailyTimep(bool autotriggered)
{
	daily_refresh_time = time(nullptr);
	daily_refresh_tm = getLocalTime(TIMEZONE_HR, TIMEZONE_MIN);
	if (autotriggered) daily_refresh_tm_auto = daily_refresh_tm;

	remain_daily_bonus = DAILY_BONUS + extra_tomorrow;
	extra_tomorrow = 0;

	broadcastMsg("每日批池刷新了；额", grp::Group::MASK_DAILYP);
	//CQ_addLog(ac, CQLOG_DEBUG, "pee", std::to_string(daily_refresh_time).c_str());
}

}