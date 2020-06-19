#include <sstream>

#include "smoke.h"
#include "utils/rand.h"
#include "utils/string_util.h"

#include "data/group.h"
#include "data/user.h"
#include "private/qqid.h"

#include "cpp-base64/base64.h"
#include "cqp.h"
#include "appmain.h"

namespace smoke
{

using user::plist;

command msgDispatcher(const char* msg)
{
	command c;
	auto query = msg2args(msg);
	if (query.empty()) return c;

	auto cmd = query[0];
	bool found = false;
	decltype(commands::禁烟) cmdt;
	for (auto& [str, com] : commands_str)
	{
		if (cmd.substr(0, str.length()) == str)
		{
			cmdt = com;
			found = true;
			break;
		}
	}

	if (!found) return c;

	c.args = query;
	switch (c.c = cmdt)
	{
	case commands::禁烟:
		c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
		{
			if (!isGroupManager(group, QQME)) return "";

			if (plist.find(qq) == plist.end()) return std::string(CQ_At(qq)) + "，你还没有开通菠菜";

			
			int64_t target = 0;
			int64_t duration = -1;

			// smoke last member if req without target
			if (args[0].substr(4).empty())
				target = grp::groups[group].last_talk_member;
			else
				target = getTargetFromStr(args[0].substr(4));	// 禁烟： bd fb / ?? ??
			if (target == 0) return "";

			// get time
			try {
				if (args.size() >= 2)
					duration = std::stoll(args[1]);
			}
			catch (std::exception&) {
				//ignore
			}
			if (duration < 0) return "你会不会烟人？";

			return nosmokingWrapper(qq, group, target, duration);

		};
		break;
	case commands::解禁:
		c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
		{
			if (!isGroupManager(group, QQME)) return "";

			if (args[0].substr(4).empty()) return "";

			int64_t target = getTargetFromStr(args[0].substr(4));	// 解禁： bd fb / ?? ??
			if (target == 0) return "查无此人";

			if (RetVal::OK == nosmoking(group, target, 0))
				return "解禁了";
			else
				return "解禁失败";
		};
		break;
	}
	return c;
}

RetVal nosmoking(int64_t group, int64_t target, int duration)
{
	if (duration < 0) return RetVal::INVALID_DURATION;

	if (grp::groups.find(group) != grp::groups.end())
	{
		if (grp::groups[group].haveMember(target))
		{
			if (grp::groups[group].members[target].permission >= 2)
				return RetVal::TARGET_IS_ADMIN;

			CQ_setGroupBan(ac, group, target, int64_t(duration) * 60);

			if (duration == 0)
			{
				smokeTimeInGroups[target].erase(group);
				return RetVal::ZERO_DURATION;
			}
			
			smokeTimeInGroups[target][group] = time(nullptr) + int64_t(duration) * 60;
			return RetVal::OK;
		}
	}
	else
	{
		const char* cqinfo = CQ_getGroupMemberInfoV2(ac, group, target, FALSE);
		if (cqinfo && strlen(cqinfo) > 0)
		{
			std::string decoded = base64_decode(std::string(cqinfo));
			if (!decoded.empty())
			{
				if (getPermissionFromGroupInfoV2(decoded.c_str()) >= 2)
					return RetVal::TARGET_IS_ADMIN;

				CQ_setGroupBan(ac, group, target, int64_t(duration) * 60);

				if (duration == 0)
				{
					smokeTimeInGroups[target].erase(group);
					return RetVal::ZERO_DURATION;
				}

				smokeTimeInGroups[target][group] = time(nullptr) + int64_t(duration) * 60;
				return RetVal::OK;
			}
		}
	}
	return RetVal::TARGET_NOT_FOUND;
}

std::string nosmokingWrapper(int64_t qq, int64_t group, int64_t target, int64_t duration)
{
	if (duration > 30 * 24 * 60) duration = 30 * 24 * 60;
	if (duration == 0)
	{
		nosmoking(group, target, duration);
		return "解禁了";
	}

	int cost = (int64_t)std::floor(std::pow(duration, 1.777777));
	if (cost > plist[qq].getCurrency()) return std::string(CQ_At(qq)) + "，你的余额不足，需要" + std::to_string(cost) + "个批";

	double reflect = randReal();

	// 10% fail
	if (reflect < 0.1) 
		return "烟突然灭了，烟起失败";

	// 20% reflect
	else if (reflect < 0.3)
	{
		switch (nosmoking(group, qq, duration))
		{
			using r = RetVal;

		case r::OK:
			return "你自己烟起吧";
		case r::TARGET_IS_ADMIN:
			return "烟突然灭了，烟起失败";
		case r::TARGET_NOT_FOUND:
			return "瞎几把烟谁呢";
		default:
			return "你会不会烟人？";
		}
	}
	else
	{
		switch (nosmoking(group, target, duration))
		{
			using r = RetVal;

		case r::OK:
		{
			plist[qq].modifyCurrency(-cost);
			std::stringstream ss;
			if (qq == target)
				ss << "？我从来没听过这样的要求，消费" << cost << "个批";
			else
				ss << "烟起哦，消费" << cost << "个批";
			return ss.str();
		}

		case r::TARGET_IS_ADMIN:
			return "禁烟管理请联系群主哦";

		case r::TARGET_NOT_FOUND:
			return "瞎几把烟谁呢";

		default:
			return "你会不会烟人？";
		}
	}
}

int64_t getTargetFromStr(const std::string& targetName, int64_t group)
{
	// @
	if (int64_t tmp; tmp = stripAt(targetName))
		return tmp;

	// qqid_str (private)
	else if (qqid_str.find(targetName) != qqid_str.end())
		return qqid_str.at(targetName);

	// nick, card
	else if (group != 0)
	{
		if (grp::groups.find(group) != grp::groups.end())
		{
			if (int64_t qq = grp::groups[group].getMember(targetName.c_str()); qq != 0)
				return qq;
		}
	}

	return 0;
}

std::string selfUnsmoke(int64_t qq)
{
	if (smokeTimeInGroups.find(qq) == smokeTimeInGroups.end() || smokeTimeInGroups[qq].empty())
		return "你么得被烟啊";

	if (plist.find(qq) == plist.end()) return "你还没有开通菠菜";
	auto &p = plist[qq];

	time_t t = time(nullptr);
	int64_t total_remain = 0;
	for (auto& g : smokeTimeInGroups[qq])
	{
		if (t <= g.second)
		{
			int64_t remain = (g.second - t) / 60; // min
			int64_t extra = (g.second - t) % 60;  // sec
			total_remain += remain + !!extra;
		}
	}
	std::stringstream ss;
	ss << "你在" << smokeTimeInGroups[qq].size() << "个群被烟" << total_remain << "分钟，";
	int64_t total_cost = (int64_t)std::ceil(total_remain * UNSMOKE_COST_RATIO);
	if (p.getCurrency() < total_cost)
	{
		ss << "你批不够，需要" << total_cost << "个批，哈";
	}
	else
	{
		p.modifyCurrency(-total_cost);
		ss << "本次接近消费" << total_cost << "个批";

		// broadcast to groups
		for (auto& g : smokeTimeInGroups[qq])
		{
			if (t > g.second) continue;
			std::string qqname = getCard(g.first, qq);
			CQ_setGroupBan(ac, g.first, qq, 0);
			std::stringstream sg;
			int64_t remain = (g.second - t) / 60; // min
			int64_t extra = (g.second - t) % 60;  // sec
			sg << qqname << "花费巨资" << (int64_t)std::round((remain + !!extra) * UNSMOKE_COST_RATIO) << "个批申请接近成功";
			CQ_sendGroupMsg(ac, g.first, sg.str().c_str());
		}
		smokeTimeInGroups[qq].clear();
	}
	return ss.str();
}

}