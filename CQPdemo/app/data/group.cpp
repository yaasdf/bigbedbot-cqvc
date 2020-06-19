#include "group.h"
#include "cqp.h"
#include "appmain.h"
#include "cpp-base64/base64.h"
#include "utils/string_util.h"

namespace grp
{

void Group::setFlag(int64_t mask, bool set)
{
	if (set)
		flags |= mask;
	else
		flags ^= mask;

	const char query[] = "UPDATE grp SET flags=? WHERE id=?";
	auto ret = db.exec(query, { flags, group_id });
	if (ret != SQLITE_OK)
	{
		char buf[64];
		sprintf_s(buf, "update flag error: id=%lld, flags=%lld", group_id, flags);
		CQ_addLog(ac, CQLOG_ERROR, "grp", buf);
	}
}

bool Group::getFlag(int64_t mask)
{
	return flags & mask;
}

void Group::updateMembers()
{
    char buf[64];
    sprintf_s(buf, "updating members for group %lld", group_id);
    CQ_addLog(ac, CQLOG_INFO, "grp", buf);

	auto members_tmp = getGroupMemberList(group_id);
	if (members_tmp.empty())
	{
		char buf[64];
		sprintf_s(buf, "updating members for group %lld error", group_id);
		CQ_addLog(ac, CQLOG_ERROR, "grp", buf);
		return;
	}
	members = members_tmp;

    sprintf_s(buf, "updated %u members", members.size());
    CQ_addLog(ac, CQLOG_INFO, "grp", buf);
}

bool Group::haveMember(int64_t qq)
{
    return members.find(qq) != members.end();
}

int64_t Group::getMember(const char* name)
{
    for (auto& [m, v] : members)
    {
        char qqid[16];
        _i64toa_s(m, qqid, sizeof(qqid), 10);

        // qqid
        if (!strcmp(qqid, name))
            return m;

        // card
        if (!strcmp(v.card.c_str(), name))
            return m;

        // nickname
        if (!strcmp(v.nick.c_str(), name))
            return m;
    }

    return 0;
}

void Group::sendMsg(const char* msg)
{
    CQ_sendGroupMsg(ac, group_id, msg);
}

void CreateTable()
{
    if (db.exec(
        "CREATE TABLE IF NOT EXISTS grp( \
            id    INTEGER PRIMARY KEY, \
            flags INTEGER         NOT NULL DEFAULT 0 \
         )") != SQLITE_OK)
        CQ_addLog(ac, CQLOG_ERROR, "grp", db.errmsg());
}


void LoadListFromDb()
{
    auto list = db.query("SELECT * FROM grp", 2);
    for (auto& row : list)
    {
		Group g;
		g.group_id = std::any_cast<int64_t>(row[0]);
		
		g.flags = std::any_cast<int64_t>(row[1]);

		groups[g.group_id] = g;
    }
    char msg[128];
    sprintf_s(msg, "added %u groups", groups.size());
    CQ_addLog(ac, CQLOG_DEBUG, "grp", msg);
}

int newGroup(int64_t id)
{
	if (groups.find(id) == groups.end())
	{
		groups[id];

		const char query[] = "INSERT INTO grp(id,flags) VALUES (?,?)";
		auto ret = db.exec(query, { id, 0 });
		if (ret != SQLITE_OK)
		{
			char buf[128];
			sprintf_s(buf, "insert group error: id=%lld", id);
			CQ_addLog(ac, CQLOG_ERROR, "grp", buf);
		}
	}
	return 0;
}

command msgDispatcher(const char* msg)
{
	command c;
	auto query = msg2args(msg);
	if (query.empty()) return c;

	auto cmd = query[0];
	if (cmd.substr(0, 4) == "开启")
	{
		c.args = query;
		c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw)->std::string
		{
			if (isGroupManager(group, qq))
			{
				auto subcmd = args[0].substr(4);
				auto& g = groups[group];
				if (subcmd == "吃什么")
				{
					g.setFlag(Group::MASK_EAT);
					return "本群已开启吃什么";
				}
				else if (subcmd == "翻批" || subcmd == "摇号")
				{
					g.setFlag(Group::MASK_GAMBOL);
					return "本群已开启翻批/摇号";
				}
				else if (subcmd == "抽卡")
				{
					g.setFlag(Group::MASK_MONOPOLY);
					return "本群已开启抽卡";
				}
				else if (subcmd == "禁烟")
				{
					g.setFlag(Group::MASK_SMOKE);
					return "本群已开启禁烟";
				}
				else if (subcmd == "开箱")
				{
					g.setFlag(Group::MASK_CASE);
					return "本群已开启开箱";
				}
				else if (subcmd == "活动开箱")
				{
					g.setFlag(Group::MASK_EVENT_CASE);
					return "本群已开启活动开箱";
				}
				else if (subcmd == "每日批池")
				{
					g.setFlag(Group::MASK_DAILYP);
					return "本群已开启每日批池";
				}
				else if (subcmd == "启动信息")
				{
					g.setFlag(Group::MASK_BOOT_ANNOUNCE);
					return "本群已开启启动信息";
				}
			}
			return "你开个锤子？";
		};
	}
	else if (cmd.substr(0, 4) == "关闭")
	{
		c.args = query;
		c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw)->std::string
		{
			if (isGroupManager(group, qq))
			{
				auto subcmd = args[0].substr(4);
				auto& g = groups[group];
				if (subcmd == "吃什么")
				{
					g.setFlag(Group::MASK_EAT, false);
					return "本群已关闭吃什么";
				}
				else if (subcmd == "翻批" || subcmd == "摇号")
				{
					g.setFlag(Group::MASK_GAMBOL, false);
					return "本群已关闭翻批/摇号";
				}
				else if (subcmd == "抽卡")
				{
					g.setFlag(Group::MASK_MONOPOLY, false);
					return "本群已关闭抽卡";
				}
				else if (subcmd == "禁烟")
				{
					g.setFlag(Group::MASK_SMOKE, false);
					return "本群已关闭禁烟";
				}
				else if (subcmd == "开箱")
				{
					g.setFlag(Group::MASK_CASE, false);
					return "本群已关闭开箱";
				}
				else if (subcmd == "活动开箱")
				{
					g.setFlag(Group::MASK_EVENT_CASE, false);
					return "本群已关闭活动开箱";
				}
				else if (subcmd == "每日批池")
				{
					g.setFlag(Group::MASK_DAILYP);
					return "本群已关闭每日批池";
				}
				else if (subcmd == "启动信息")
				{
					g.setFlag(Group::MASK_BOOT_ANNOUNCE);
					return "本群已关闭启动信息";
				}
			}
			return "你关个锤子？";
		};
	}

	return c;
}

}

void broadcastMsg(const char* msg, int64_t flag)
{
    for (auto& [id, g] : grp::groups)
    {
        //CQ_sendGroupMsg(ac, group, msg);
        if (!g.members.empty() && ((g.flags & flag) || flag == -1))
            g.sendMsg(msg);
    }
}