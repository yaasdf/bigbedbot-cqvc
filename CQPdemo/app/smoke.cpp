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
	decltype(commands::����) cmdt;
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
	case commands::����:
		c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
		{
			if (!isGroupManager(group, QQME)) return "";

			if (plist.find(qq) == plist.end()) return std::string(CQ_At(qq)) + "���㻹û�п�ͨ����";

			
			int64_t target = 0;
			int64_t duration = -1;

			// smoke last member if req without target
			if (args[0].substr(4).empty())
				target = grp::groups[group].last_talk_member;
			else
				target = getTargetFromStr(args[0].substr(4));	// ���̣� bd fb / ?? ??
			if (target == 0) return "";

			// get time
			try {
				if (args.size() >= 2)
					duration = std::stoll(args[1]);
			}
			catch (std::exception&) {
				//ignore
			}
			if (duration < 0) return "��᲻�����ˣ�";

			return nosmokingWrapper(qq, group, target, duration);

		};
		break;
	case commands::���:
		c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
		{
			if (!isGroupManager(group, QQME)) return "";

			if (args[0].substr(4).empty()) return "";

			int64_t target = getTargetFromStr(args[0].substr(4));	// ����� bd fb / ?? ??
			if (target == 0) return "���޴���";

			if (RetVal::OK == nosmoking(group, target, 0))
				return "�����";
			else
				return "���ʧ��";
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
		return "�����";
	}

	int cost = (int64_t)std::floor(std::pow(duration, 1.777777));
	if (cost > plist[qq].getCurrency()) return std::string(CQ_At(qq)) + "��������㣬��Ҫ" + std::to_string(cost) + "����";

	double reflect = randReal();

	// 10% fail
	if (reflect < 0.1) 
		return "��ͻȻ���ˣ�����ʧ��";

	// 20% reflect
	else if (reflect < 0.3)
	{
		switch (nosmoking(group, qq, duration))
		{
			using r = RetVal;

		case r::OK:
			return "���Լ������";
		case r::TARGET_IS_ADMIN:
			return "��ͻȻ���ˣ�����ʧ��";
		case r::TARGET_NOT_FOUND:
			return "Ϲ������˭��";
		default:
			return "��᲻�����ˣ�";
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
				ss << "���Ҵ���û����������Ҫ������" << cost << "����";
			else
				ss << "����Ŷ������" << cost << "����";
			return ss.str();
		}

		case r::TARGET_IS_ADMIN:
			return "���̹�������ϵȺ��Ŷ";

		case r::TARGET_NOT_FOUND:
			return "Ϲ������˭��";

		default:
			return "��᲻�����ˣ�";
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
		return "��ô�ñ��̰�";

	if (plist.find(qq) == plist.end()) return "�㻹û�п�ͨ����";
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
	ss << "����" << smokeTimeInGroups[qq].size() << "��Ⱥ����" << total_remain << "���ӣ�";
	int64_t total_cost = (int64_t)std::ceil(total_remain * UNSMOKE_COST_RATIO);
	if (p.getCurrency() < total_cost)
	{
		ss << "������������Ҫ" << total_cost << "��������";
	}
	else
	{
		p.modifyCurrency(-total_cost);
		ss << "���νӽ�����" << total_cost << "����";

		// broadcast to groups
		for (auto& g : smokeTimeInGroups[qq])
		{
			if (t > g.second) continue;
			std::string qqname = getCard(g.first, qq);
			CQ_setGroupBan(ac, g.first, qq, 0);
			std::stringstream sg;
			int64_t remain = (g.second - t) / 60; // min
			int64_t extra = (g.second - t) % 60;  // sec
			sg << qqname << "���Ѿ���" << (int64_t)std::round((remain + !!extra) * UNSMOKE_COST_RATIO) << "��������ӽ��ɹ�";
			CQ_sendGroupMsg(ac, g.first, sg.str().c_str());
		}
		smokeTimeInGroups[qq].clear();
	}
	return ss.str();
}

}