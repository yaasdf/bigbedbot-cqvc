#include "group.h"
#include "cqp.h"
#include "appmain.h"
#include "cpp-base64/base64.h"

namespace grp
{

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

}

void broadcastMsg(const char* msg)
{
    for (auto& [id, g] : grp::groups)
    {
        //CQ_sendGroupMsg(ac, group, msg);
        if (!g.members.empty())
            g.sendMsg(msg);
    }
}