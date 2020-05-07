#include "group.h"
#include "cqp.h"
#include "appmain.h"
#include "cpp-base64/base64.h"

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

    const char* grpbuf = CQ_getGroupMemberList(ac, group_id);
    CQ_addLog(ac, CQLOG_DEBUG, "grp", grpbuf);

    if (grpbuf == NULL)
    {
        members.clear();
        sprintf_s(buf, "group not found");
        CQ_addLog(ac, CQLOG_INFO, "grp", buf);
        return;
    }

    std::string raw = base64_decode(std::string(grpbuf));
    int count = ntohl(*((uint32_t*)(raw.c_str())));
    const char* p = raw.c_str() + sizeof(uint32_t);

    members.clear();

    for (int i = 0; (i < count) && (p < raw.c_str() + raw.length()); ++i)
    {
        int member_size = ntohs(*((uint16_t*)(p)));
        p += sizeof(uint16_t);

        auto m = GroupMemberInfo(p);
        members[m.qqid] = m;
        p += member_size;
    }

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
    sprintf(msg, "added %u groups", groups.size());
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
				else if (subcmd == "翻批")
				{
					g.setFlag(Group::MASK_FLIPCOIN);
					return "本群已开启翻批";
				}
				else if (subcmd == "摇号")
				{
					g.setFlag(Group::MASK_ROULETTE);
					return "本群已开启摇号";
				}
				else if (subcmd == "抽卡")
				{
					g.setFlag(Group::MASK_MONOPOLY);
					return "本群已开启抽卡/开箱";
				}
				else if (subcmd == "禁烟")
				{
					g.setFlag(Group::MASK_SMOKE);
					return "本群已开启禁烟";
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
				else if (subcmd == "翻批")
				{
					g.setFlag(Group::MASK_FLIPCOIN, false);
					return "本群已关闭翻批";
				}
				else if (subcmd == "摇号")
				{
					g.setFlag(Group::MASK_ROULETTE, false);
					return "本群已关闭摇号";
				}
				else if (subcmd == "抽卡")
				{
					g.setFlag(Group::MASK_MONOPOLY, false);
					return "本群已关闭抽卡/开箱";
				}
				else if (subcmd == "禁烟")
				{
					g.setFlag(Group::MASK_SMOKE, false);
					return "本群已关闭禁烟";
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