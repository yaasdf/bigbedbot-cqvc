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
	case commands::��ͨ��ʾ:
		c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
		{
			if (plist.find(qq) != plist.end()) return std::string(CQ_At(qq)) + "�����Ѿ�ע�����";

			return "����Ҫ��ͨ���ˣ���᲻�Ὺͨ����";
		};
		break;
	case commands::��ͨ:
		c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
		{
			if (plist.find(qq) != plist.end()) return std::string(CQ_At(qq)) + "�����Ѿ�ע�����";
			auto &p = plist[qq];

			p.createAccount(user::INITIAL_BALANCE);

			std::stringstream ss;
			ss << "����Կ�ʼ�����ˣ��͸���" << plist[qq].getCurrency() << "����";
			return ss.str();
		};
		break;
	case commands::���:
		c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
		{
			if (plist.find(qq) == plist.end()) return std::string(CQ_At(qq)) + "���㻹û�п�ͨ����";
			auto &p = plist[qq];
			std::stringstream ss;
			ss << CQ_At(qq) << "��������Ϊ" << p.getCurrency() << "������"
				<< p.getKeyCount() << "��Կ��";
			auto[enough, stamina, rtime] = p.getStamina(0);
			ss << "\n�㻹��" << stamina << "������";
			if (stamina < user::MAX_STAMINA)
				ss << "����������" << rtime / (60 * 60) << "Сʱ" << rtime / 60 % 60 << "����";
			return ss.str();
		};
		break;
	case commands::����:
		c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
		{
			if (plist.find(qq) == plist.end()) return std::string(CQ_At(qq)) + "���㻹û�п�ͨ����";
			if (!grp::groups[group].getFlag(grp::Group::MASK_DAILYP)) return "��Ⱥ�������ˣ�ô������/cy";

			auto &p = plist[qq];

			if (p.getLastDrawTime() > daily_refresh_time)
			{
				std::stringstream ss;
				ss << CQ_At(qq) << "��������Ѿ�����ˣ���������8";
				return ss.str();
			}

			std::stringstream ss;
			ss << CQ_At(qq) << "��������쵽" << FREE_BALANCE_ON_NEW_DAY << "����";
			if (remain_daily_bonus)
			{
				int bonus = randInt(1, remain_daily_bonus > 66 ? 66 : remain_daily_bonus);
				remain_daily_bonus -= bonus;
				p.modifyCurrency(FREE_BALANCE_ON_NEW_DAY + bonus);

				ss << "�����������ȵ���" << bonus << "����\n"
					<< "�������ػ�ʣ" << remain_daily_bonus << "����";
			}
			else
			{
				p.modifyCurrency(FREE_BALANCE_ON_NEW_DAY);
				ss << "\nÿ������ô���ˣ�������ȵ�";
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
	case commands::����:
		c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
		{
			if (plist.find(qq) == plist.end()) return std::string(CQ_At(qq)) + "���㻹û�п�ͨ����";
			auto &p = plist[qq];

			auto[enough, stamina, rtime] = p.modifyStamina(1);

			std::stringstream ss;
			if (!enough) ss << CQ_At(qq) << "������������㣬��������"
				<< rtime / (60 * 60) << "Сʱ" << rtime / 60 % 60 << "����";

			p.modifyCurrency(1);

			return std::string(CQ_At(qq)) + "����1�������õ�1����";
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

	broadcastMsg("ÿ������ˢ���ˣ���", grp::Group::MASK_DAILYP);
	//CQ_addLog(ac, CQLOG_DEBUG, "pee", std::to_string(daily_refresh_time).c_str());
}

}