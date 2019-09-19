#include "event_case.h"
using namespace event_case;

#include <iostream>
#include <regex>
#include <sstream>
#include "cqp.h"
#include "appmain.h"

#include "pee.h"

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
    case commands::测试:
        c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
        {
            if (plist.find(qq) == plist.end()) return std::string(CQ_At(qq)) + "，你还没有开通菠菜";

            std::stringstream ss;

            auto [enough, stamina, rtime] = testStamina(qq, 3);
            if (!enough)
            {
                ss << CQ_At(qq) << "，你的体力不足，回满还需"
                    << rtime / (60 * 60) << "小时" << rtime / 60 % 60 << "分钟";
                return ss.str();
            }

            int type = -1;
            try
            {
                if (args.size() > 1)
                    type = std::stoi(args[1]);
            }
            catch (...) {}

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

            updateStamina(qq, 3);
            const case_detail& reward = pool_event.draw(type);
            if (reward.worth > 300) ss << "歪哟，" << CQ_At(qq) << "发了，开出了";
            else ss << CQ_At(qq) << "，恭喜你开出了" << pool_event.caseFullName(reward);

            plist[qq].currency += reward.worth;
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