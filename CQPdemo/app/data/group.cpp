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
	if (cmd.substr(0, 4) == "����")
	{
		c.args = query;
		c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw)->std::string
		{
			if (isGroupManager(group, qq))
			{
				auto subcmd = args[0].substr(4);
				auto& g = groups[group];
				if (subcmd == "��ʲô")
				{
					g.setFlag(Group::MASK_EAT);
					return "��Ⱥ�ѿ�����ʲô";
				}
				else if (subcmd == "����" || subcmd == "ҡ��")
				{
					g.setFlag(Group::MASK_GAMBOL);
					return "��Ⱥ�ѿ�������/ҡ��";
				}
				else if (subcmd == "�鿨")
				{
					g.setFlag(Group::MASK_MONOPOLY);
					return "��Ⱥ�ѿ����鿨";
				}
				else if (subcmd == "����")
				{
					g.setFlag(Group::MASK_SMOKE);
					return "��Ⱥ�ѿ�������";
				}
				else if (subcmd == "����")
				{
					g.setFlag(Group::MASK_CASE);
					return "��Ⱥ�ѿ�������";
				}
				else if (subcmd == "�����")
				{
					g.setFlag(Group::MASK_EVENT_CASE);
					return "��Ⱥ�ѿ��������";
				}
				else if (subcmd == "ÿ������")
				{
					g.setFlag(Group::MASK_DAILYP);
					return "��Ⱥ�ѿ���ÿ������";
				}
				else if (subcmd == "������Ϣ")
				{
					g.setFlag(Group::MASK_BOOT_ANNOUNCE);
					return "��Ⱥ�ѿ���������Ϣ";
				}
			}
			return "�㿪�����ӣ�";
		};
	}
	else if (cmd.substr(0, 4) == "�ر�")
	{
		c.args = query;
		c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw)->std::string
		{
			if (isGroupManager(group, qq))
			{
				auto subcmd = args[0].substr(4);
				auto& g = groups[group];
				if (subcmd == "��ʲô")
				{
					g.setFlag(Group::MASK_EAT, false);
					return "��Ⱥ�ѹرճ�ʲô";
				}
				else if (subcmd == "����" || subcmd == "ҡ��")
				{
					g.setFlag(Group::MASK_GAMBOL, false);
					return "��Ⱥ�ѹرշ���/ҡ��";
				}
				else if (subcmd == "�鿨")
				{
					g.setFlag(Group::MASK_MONOPOLY, false);
					return "��Ⱥ�ѹرճ鿨";
				}
				else if (subcmd == "����")
				{
					g.setFlag(Group::MASK_SMOKE, false);
					return "��Ⱥ�ѹرս���";
				}
				else if (subcmd == "����")
				{
					g.setFlag(Group::MASK_CASE, false);
					return "��Ⱥ�ѹرտ���";
				}
				else if (subcmd == "�����")
				{
					g.setFlag(Group::MASK_EVENT_CASE, false);
					return "��Ⱥ�ѹرջ����";
				}
				else if (subcmd == "ÿ������")
				{
					g.setFlag(Group::MASK_DAILYP);
					return "��Ⱥ�ѹر�ÿ������";
				}
				else if (subcmd == "������Ϣ")
				{
					g.setFlag(Group::MASK_BOOT_ANNOUNCE);
					return "��Ⱥ�ѹر�������Ϣ";
				}
			}
			return "��ظ����ӣ�";
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