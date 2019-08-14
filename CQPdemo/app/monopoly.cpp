#include "monopoly.h"
#include "cqp.h"
#include "appmain.h"
#include "private/qqid.h"

namespace mnp
{
const std::vector<event_type> EVENT_POOL{
    { 0.1,[](int64_t group, int64_t qq) { modifyKeyCount(qq, ++plist[qq].keys); return "钥匙1把"; }},
    { 0.1,[](int64_t group, int64_t qq) { nosmoking(group, qq, 1); return "禁烟儿童套餐，烟起1分钟"; }},
    { 0.05,[](int64_t group, int64_t qq) { plist[qq].keys += 2; modifyKeyCount(qq, plist[qq].keys); return "钥匙链，获得2把钥匙"; }},
    { 0.01,[](int64_t group, int64_t qq) { plist[qq].keys += 10; modifyKeyCount(qq, plist[qq].keys); return "钥匙套装，获得10把钥匙"; }},
    { 0.04,[](int64_t group, int64_t qq) { plist[qq].currency += 1; modifyCurrency(qq, plist[qq].currency); return "地下水，获得1个批"; }},
    { 0.03,[](int64_t group, int64_t qq) { plist[qq].currency += 1; modifyCurrency(qq, plist[qq].currency); return "祖传地下水，但还是只获得1个批"; }},
    { 0.03,[](int64_t group, int64_t qq) { plist[qq].currency += 1; modifyCurrency(qq, plist[qq].currency); return "崭新出厂地下水，但还是只获得1个批"; }},
    { 0.07,[](int64_t group, int64_t qq) { plist[qq].currency += 2; modifyCurrency(qq, plist[qq].currency); return "限量版康帅博牛肉面，获得2个批"; }},
    { 0.05,[](int64_t group, int64_t qq) { plist[qq].currency += 5; modifyCurrency(qq, plist[qq].currency); return "爪子刀玩具模型，获得5个批"; }},
    { 0.03,[](int64_t group, int64_t qq) { plist[qq].currency += 20; modifyCurrency(qq, plist[qq].currency); return "群主贴纸，获得20个批"; }},
    { 0.01,[](int64_t group, int64_t qq) { plist[qq].currency += 100; modifyCurrency(qq, plist[qq].currency); return "大床纪念相册，获得100个批"; }},
    { 0.06,[](int64_t group, int64_t qq) { updateStamina(qq, -1); return "再来一次券，获得1点体力"; }},
    { 0.03,[](int64_t group, int64_t qq) { updateStamina(qq, -2); return "再来一套券，获得2点体力"; }},
    { 0.01,[](int64_t group, int64_t qq) { updateStamina(qq, -4); return "原素瓶，获得4点体力"; }},
    { 0.005,[](int64_t group, int64_t qq) { updateStamina(qq, -pee::MAX_STAMINA); return "回复石，你感觉神清气爽，体力回满了"; }},
    { 0.01,[](int64_t group, int64_t qq) { nosmoking(group, qq, 5); return "禁烟板烧套餐，烟起5分钟"; }},

    { 0.01,[](int64_t group, int64_t qq)
    {
        auto [enough, stamina, t] = updateStamina(qq, 0);
        updateStamina(qq, stamina);
        return "加班邮件，你的体力被吸光了";
    }},

    { 0.01,[](int64_t group, int64_t qq)
    {
        updateStamina(qq, -3, true);
        using namespace std::string_literals;
        return "小蓝杯，获得3点体力"s + (pee::staminaExtra[qq] > 0 ? "，甚至突破了体力上限"s : ""s);
    }},

    { 0.01,[](int64_t group, int64_t qq)
    {
        plist[qq].currency -= 50;
        if (plist[qq].currency < 0) plist[qq].currency = 0;
        modifyCurrency(qq, plist[qq].currency);
        return "仙人掌，你的手被扎成了马蜂窝，花费医药费50批";
    }},

    { 0.01,[](int64_t group, int64_t qq)
    {
        plist[qq].currency -= 10; 
        if (plist[qq].currency < 0) plist[qq].currency = 0;
        modifyCurrency(qq, plist[qq].currency);
        pee::extra_tomorrow += 10;
        return "慈善参与奖，花费10批加入明日批池"; 
    }},

    { 0.01,[](int64_t group, int64_t qq)
    {
        double p = randReal(0.5, 1.5);
        plist[qq].currency *= p;
        modifyCurrency(qq, plist[qq].currency);
        using namespace std::string_literals;
        return "一把菠菜，你的批变成了"s + std::to_string(p) + "倍"s;
    }},

    { 0.01,[](int64_t group, int64_t qq) -> std::string
    {
        const char* cqinfo = CQ_getGroupMemberInfoV2(ac, group, qqid_dk, FALSE);
        if (cqinfo && strlen(cqinfo) > 0)
        {
            nosmoking(group, qqid_dk, 1);
            return "干死也军车，烟他！";
        }
        else return EVENT_POOL[0].func()(group, qq);
    }},

    { 0.002,[](int64_t group, int64_t qq) -> std::string
    {
        for (auto& [qq, pd] : plist)
        {
            const char* cqinfo = CQ_getGroupMemberInfoV2(ac, group, qq, FALSE);
            if (cqinfo && strlen(cqinfo) > 0)
            {
                auto [enough, stamina, t] = updateStamina(qq, 0);
                updateStamina(qq, stamina);
                updateStamina(qq, -2);
            }
        }
        return "二向箔，本群所有人的体力都变成了2点";
    }},

    { 0.002,[](int64_t group, int64_t qq) -> std::string
    {
        for (auto& [qq, pd] : plist)
        {
            const char* cqinfo = CQ_getGroupMemberInfoV2(ac, group, qq, FALSE);
            if (cqinfo && strlen(cqinfo) > 0)
            {
                plist[qq].currency *= 0.9;
                modifyCurrency(qq, plist[qq].currency);
            }
        }
        return "陨石，本群所有人的钱包都被炸飞10%";
    }},

    { 0.002,[](int64_t group, int64_t qq)
    {
        plist[qq].currency -= 500;
        if (plist[qq].currency < 0) plist[qq].currency = 0;
        modifyCurrency(qq, plist[qq].currency);
        return "JJ怪，你的家被炸烂了，损失500批";
    }},

    { 0.01,[](int64_t group, int64_t qq)
    {
        plist[qq].currency += 10;
        modifyCurrency(qq, plist[qq].currency);
        pee::extra_tomorrow -= 10;
        if (-pee::extra_tomorrow > pee::DAILY_BONUS) pee::extra_tomorrow = -pee::DAILY_BONUS;
        return "二级钳工认证，从明日批池偷到10批";
    } },

    { 0.005,[](int64_t group, int64_t qq) -> std::string
    {
        std::thread([]() {using namespace std::chrono_literals; std::this_thread::sleep_for(2s); pee::flushDailyTimep(); }).detach();
        return "F5";
    }},

    { 0.002,[](int64_t group, int64_t qq)
    {
        std::vector<unsigned> numbers;
        auto c = plist[qq].currency;
        while (c > 0)
        {
            int tmp = c % 10;
            c /= 10;
            numbers.push_back(tmp);
        }
        size_t size = numbers.size();
        int64_t p = 0;
        for (size_t i = 0; i < size; ++i)
        {
            unsigned idx = randInt(0, numbers.size() - 1);
            p = p * 10 + numbers[idx];
            numbers.erase(numbers.begin() + idx);
        }
        plist[qq].currency = p;
        modifyCurrency(qq, plist[qq].currency);
        return "混沌药水，你的批被打乱了";
    } },

    { 0.002,[](int64_t group, int64_t qq)
    {
        std::vector<unsigned> numbers;
        auto c = plist[qq].currency;
        while (c > 0)
        {
            int tmp = c % 10;
            c /= 10;
            numbers.push_back(tmp);
        }
        size_t size = numbers.size();
        int64_t p = 0;
        unsigned idx = randInt(0, numbers.size() - 1);
        for (size_t i = 0; i < size; ++i)
        {
            p = p * 10 + numbers[idx];
        }
        plist[qq].currency = p;
        modifyCurrency(qq, plist[qq].currency);
        return "融合药水，你的批全部变成了一样的数字";
    } },

    { 0.002,[](int64_t group, int64_t qq)
    {
        std::vector<unsigned> numbers;
        auto c = plist[qq].currency;
        while (c > 0)
        {
            int tmp = c % 10;
            c /= 10;
            numbers.push_back(tmp);
        }
        size_t size = numbers.size();
        int64_t p = 0;
        for (size_t i = 0; i < size; ++i)
        {
            p = p * 10 + numbers[i];
        }
        plist[qq].currency = p;
        modifyCurrency(qq, plist[qq].currency);
        return "镜像药水，你的批被反过来了";
    } },


    { 0.002,[](int64_t group, int64_t qq) -> std::string
    {
        for (auto& [qq, pd] : plist)
        {
            const char* cqinfo = CQ_getGroupMemberInfoV2(ac, group, qq, FALSE);
            if (cqinfo && strlen(cqinfo) > 0)
            {
                updateStamina(qq, -pee::MAX_STAMINA);
            }
        }
        return "一箱黄金胡萝卜，本群所有人的体力都回满了";
    } },

    { 0.01,[](int64_t group, int64_t qq) -> std::string
    {
        if (plist[qq].keys < 1) return EVENT_POOL[0].func()(group, qq);
        plist[qq].keys--; 
        modifyKeyCount(qq, plist[qq].keys);
        return "空箱子，你什么也没有开到，消耗1把钥匙"; 
    } },

    { 0.002,[](int64_t group, int64_t qq) -> std::string
    {
        for (auto& [qq, pd] : plist)
        {
            const char* cqinfo = CQ_getGroupMemberInfoV2(ac, group, qq, FALSE);
            if (cqinfo && strlen(cqinfo) > 0)
            {
                int bonus = randInt(50, 1000);
                plist[qq].currency += bonus;
                modifyCurrency(qq, plist[qq].currency);
            }
        }
        return "大风吹来本群的一堆批，自己查余额看捡到多少哦";
    } },

    { 0.01,[](int64_t group, int64_t qq) -> std::string
    {
        const char* cqinfo = CQ_getGroupMemberInfoV2(ac, group, qqid_dk, FALSE);
        if (cqinfo && strlen(cqinfo) > 0)
        {
            int d = randInt(1, 5);
            nosmoking(group, qq, d);
            using namespace std::string_literals;
            return "疯批跌坑，你被跌坑禁烟"s + std::to_string(d) + "分钟"s;
        }
        else return EVENT_POOL[0].func()(group, qq);
    } },

    { 0.01,[](int64_t group, int64_t qq) { return "断钥匙一把，你尝试用来开箱但是失败了"; } },
    { 0.01,[](int64_t group, int64_t qq) { return "机械鼠标球一个，但是没有什么用"; }},
    { 0.01,[](int64_t group, int64_t qq) { return "巨大回车键一个，但是没有什么用"; }},
    { 0.01,[](int64_t group, int64_t qq) { return "一团稀有气体，但是没有什么用"; } },
    { 0.01,[](int64_t group, int64_t qq) { return EMOJI_HAMMER + "，你抽个" + EMOJI_HAMMER; } },
};
const event_type EVENT_DEFAULT{ 1.0, [](int64_t group, int64_t qq) { return "空气，什么都么得"; } };


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
    case commands::抽卡:
        c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw)->std::string
        {
            if (plist.find(qq) == plist.end()) return std::string(CQ_At(qq)) + "，你还没有开通菠菜";

            auto [enough, stamina, rtime] = updateStamina(qq, 1);

            std::stringstream ss;
            if (!enough) ss << CQ_At(qq) << "，你的体力不足，回满还需"
                << rtime / (60 * 60) << "小时" << rtime / 60 % 60 << "分钟";
            else
            {
                auto reward = draw_event(randReal());
                ss << CQ_At(qq) << "，恭喜你抽到了" << reward.func()(group, qq);
            }
            return ss.str();
        };
        break;

    default:
        break;
    }
    return c;
}

const event_type& draw_event(double p)
{
    if (p < 0.0 || p > 1.0) return EVENT_DEFAULT;
    double totalp = 0;
    for (const auto& c : EVENT_POOL)
    {
        if (p <= totalp + c.prob()) return c;
        totalp += c.prob();
    }
    return EVENT_DEFAULT;        // not match any case
}

}