#include "event_case.h"
using namespace event_case;

#include <iostream>
#include <sstream>
#include "cqp.h"
#include "appmain.h"

#include "pee.h"
#include "group.h"

case_pool::case_pool(const types_t& type_b, const levels_t& level_b, const std::vector<case_detail>& cases_b):
    types(type_b), levels(level_b)
{
    cases.resize(types.size());
    for (auto& c : cases)
        c.resize(levels.size());

    for (const auto& c : cases_b)
    {
        if (c.level < (int)types.size() && c.type < (int)types.size())
        {
            cases[c.type][c.level].second.push_back(c);
        }
    }

    // normalize level probability
    for (auto& type : cases)
    {
        unsigned level = 0;
        double total_p = 0.0;

        for (auto& [prop, case_c] : type)
        {
            if (!case_c.empty())
            {
                prop = levels[level].second;
                total_p += prop;
            }
            ++level;
        }

        for (auto& [prop, c] : type)
        {
            prop /= total_p;
        }
    }
}


case_detail case_pool::draw(int type)
{
    double p = randReal();
    if (type >= 0 && type < getTypeCount())
    {
        size_t idx = 0;

        if (p >= 0.0 && p <= 1.0)
        {
            double totalp = 0;
            for (const auto& [prob, list] : cases[type])
            {
                if (p <= totalp + prob) break;
                ++idx;
                totalp += prob;
            }
            // idx = CASE_POOL.size() if not match any case
        }

        size_t detail_idx = randInt(0, cases[type][idx].second.size() - 1);
        return cases[type][idx].second[detail_idx];
    }
    else
    {
        return {-1, -1, "非法箱子", -1};
    }
}

std::string case_pool::casePartName(const case_detail& c) const
{
    std::stringstream ss;
    //if (c.type >= 0 && c.type < getTypeCount()) ss << "[" << getType(c.type) << "] ";
    if (c.level >= 0 && c.level < getLevelCount()) ss << "<" << getLevel(c.level) << "> ";
    ss << c.name;
    ss << " (" << c.worth << "批)";

    std::string ret = ss.str();
    return ret;
}

std::string case_pool::caseFullName(const case_detail& c) const
{
    std::stringstream ss;
    if (c.type >= 0 && c.type < getTypeCount()) ss << "[" << getType(c.type) << "] ";
    if (c.level >= 0 && c.level < getLevelCount()) ss << "<" << getLevel(c.level) << "> ";
    ss << c.name;
    ss << " (" << c.worth << "批)";

    std::string ret = ss.str();
    return ret;
}

using pee::plist;
using pee::testStamina;
using pee::updateStamina;
using pee::modifyCurrency;
using pee::modifyKeyCount;
using pee::modifyDrawTime;

command event_case::msgDispatcher(const char* msg)
{
    command c;
    auto query = msg2args(msg);
    if (query.empty()) return c;

    auto cmd = query[0];
    if (commands_str.find(cmd) == commands_str.end()) return c;

    c.args = query;
    switch (c.c = commands_str[cmd])
    {
    case commands::开箱:
        c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
        {
            if (plist.find(qq) == plist.end()) return std::string(CQ_At(qq)) + "，你还没有开通菠菜";

            std::stringstream ss;

            if (type < 0 || type >= pool_event.getTypeCount())
            {
                ss << CQ_At(qq) << "，活动尚未开始，请下次再来";
                return ss.str();
            }

            auto [enough, stamina, rtime] = testStamina(qq, 1);
            if (!enough)
            {
                ss << CQ_At(qq) << "，你的体力不足，回满还需"
                    << rtime / (60 * 60) << "小时" << rtime / 60 % 60 << "分钟";
                return ss.str();
            }

            int cost = 0;
            if (type >= 0 && type < (int)pool_event.getTypeCount() && plist[qq].currency < pool_event.getTypeCost(type))
            {
                cost = pool_event.getTypeCost(type);
                if (plist[qq].currency < cost)
                {
                    ss << CQ_At(qq) << "，你的余额不足，需要" << cost << "个批";
                    return ss.str();
                }
            }

            updateStamina(qq, 1);
            plist[qq].currency -= pool_event.getTypeCost(type);
            const case_detail& reward = pool_event.draw(type);
            if (reward.worth > 300) ss << "歪哟，" << CQ_At(qq) << "发了，开出了";
            else ss << CQ_At(qq) << "，恭喜你开出了";
            ss << pool_event.casePartName(reward);
            plist[qq].currency += reward.worth;

            // drop
            if (randReal() < 0.1)
            {
                /*
                ss << "，并获得1个箱子掉落";
                plist[qq].event_drop_count++;
                */
                ss << "，并获得1个箱子掉落，开出了";
                int type = randInt(0, pool_drop.getTypeCount() - 1);
                auto dcase = pool_drop.draw(type);
                plist[qq].currency += dcase.worth;
                ss << pool_drop.caseFullName(dcase);
            }

            if (plist[qq].currency < 0) plist[qq].currency = 0;
            modifyCurrency(qq, plist[qq].currency);
            //modifyBoxCount(qq, ++plist[qq].opened_box_count);
            //ss << "你还有" << stamina << "点体力，";

            return ss.str();
        };
        break;
    default: break;
    }
    return c;
}

void event_case::startEvent()
{
    if (type == -1)
    {
        type = randInt(0, pool_event.getTypeCount() - 1);
        event_case_tm = getLocalTime(TIMEZONE_HR, TIMEZONE_MIN);
        std::stringstream ss;
        ss << "限时活动已开始，这次是<" << pool_event.getType(type) << ">，每次收费" << pool_event.getTypeCost(type) << "批，请群员踊跃参加";
        broadcastMsg(ss.str().c_str());
    }
    else
    {
        CQ_addLog(ac, CQLOG_WARNING, "event", "attempt to start event during event");
    }
}

void event_case::stopEvent()
{
    if (type != -1)
    {
        type = -1;
        auto event_case_time = time(nullptr);
        event_case_end_tm = getLocalTime(TIMEZONE_HR, TIMEZONE_MIN);

        /*
        std::map<int64_t, std::stringstream> msgbuf;
        for (auto& [qq, data] : plist)
        {
            if (data.event_drop_count > 0)
            {
                auto& ss = msgbuf[qq];
                ss << CQ_At(qq) << "获得" << data.event_drop_count << "个箱子掉落，开出了：";
                while (data.event_drop_count--)
                {
                    int type = randInt(0, pool_drop.getTypeCount() - 1);
                    auto dcase = pool_drop.draw(type);
                    plist[qq].currency += dcase.worth;
                    ss << "\n - " << pool_drop.caseFullName(dcase);
                }
            }
        }
        */

        for (const auto& [group, cfg] : grp::groups)
        {
            std::stringstream ss;
            ss << "限时活动已结束！";
            /*
            for (auto& [qq, data] : plist)
            {
                const char* cqinfo = CQ_getGroupMemberInfoV2(ac, group, qq, FALSE);
                if (cqinfo && strlen(cqinfo) > 0 && msgbuf.find(qq) != msgbuf.end())
                {
                    ss << "\n" << msgbuf[qq].str();
                }
            }
            */
            CQ_sendGroupMsg(ac, group, ss.str().c_str());
        }
    }
    else
    {
        CQ_addLog(ac, CQLOG_WARNING, "event", "attempt to end event during normal time");
    }
}