#include <cmath>
#include <thread>
#include <sstream>
#include "monopoly.h"

#include "utils/rand.h"
#include "utils/string_util.h"

#include "data/user.h"
#include "data/group.h"
#include "private/qqid.h"

#include "user_op.h"
#include "smoke.h"

#include "cqp.h"
#include "cqp_ex.h"

extern time_t banTime_me;

namespace mnp
{
using user::plist;

const std::vector<event_type> EVENT_POOL{
    { 0.02,[](int64_t group, int64_t qq) { plist[qq].modifyKeyCount(+1); return "铁制钥匙1把"; }},
    { 0.02,[](int64_t group, int64_t qq) { plist[qq].modifyKeyCount(+1); return "金钥匙1把"; }},
    { 0.02,[](int64_t group, int64_t qq) { plist[qq].modifyKeyCount(+1); return "铜制钥匙1把"; }},
    { 0.02,[](int64_t group, int64_t qq) { plist[qq].modifyKeyCount(+1); return "开锁用铁丝，能撬开1个箱子"; }},
    { 0.02,[](int64_t group, int64_t qq) { plist[qq].modifyKeyCount(+1); return "扳手，能砸开1个箱子"; }},
    { 0.01,[](int64_t group, int64_t qq) { plist[qq].modifyKeyCount(+2); return "黑桃钥匙，能开2个箱子"; }},
    { 0.01,[](int64_t group, int64_t qq) { plist[qq].modifyKeyCount(+2); return "红心钥匙，能开2个箱子"; }},
    { 0.01,[](int64_t group, int64_t qq) { plist[qq].modifyKeyCount(+2); return "梅花钥匙，能开2个箱子"; }},
    { 0.01,[](int64_t group, int64_t qq) { plist[qq].modifyKeyCount(+2); return "方块钥匙，能开2个箱子"; }},
    { 0.01,[](int64_t group, int64_t qq) { plist[qq].modifyKeyCount(+2); return "钻石钥匙，能开2个箱子"; }},
    { 0.01,[](int64_t group, int64_t qq) { plist[qq].modifyKeyCount(+10); return "钥匙套装，获得10把钥匙"; }},
	{ 0.05,[](int64_t group, int64_t qq) { plist[qq].modifyCurrency(+1); return "扔在路边没人要的地下水，获得1个批"; }},
    { 0.02,[](int64_t group, int64_t qq) { plist[qq].modifyCurrency(+5); return "限量版康帅博牛肉面，获得5个批"; }},
    { 0.01,[](int64_t group, int64_t qq) { plist[qq].modifyCurrency(+20); return "爪子刀玩具模型，获得20个批"; }},
    { 0.005,[](int64_t group, int64_t qq) { plist[qq].modifyCurrency(+100); return "大床纪念相册，获得100个批"; }},
	{ 0.06,[](int64_t group, int64_t qq) { plist[qq].modifyStamina(-1); return "再来一次券，获得1点体力"; }},
    { 0.02,[](int64_t group, int64_t qq) { plist[qq].modifyStamina(-2); return "再来一次双人体验套餐，获得2点体力"; }},
    { 0.01,[](int64_t group, int64_t qq) { plist[qq].modifyStamina(-4); return "原素瓶，获得4点体力"; }},
    { 0.01,[](int64_t group, int64_t qq) { plist[qq].modifyKeyCount(+4); return "原素灰瓶，你用魔法做出了4把钥匙"; }},
    { 0.002,[](int64_t group, int64_t qq) { plist[qq].modifyCurrency(+500); return "大床兔子BEX通关纪念雕像，获得500个批"; }},
    { 0.02,[](int64_t group, int64_t qq) { plist[qq].modifyKeyCount(+5); return "开锁工具，能开5个箱子"; }},

    { 0.01,[](int64_t group, int64_t qq) { plist[qq].modifyStamina(1); return "残业，你加了半小时班"; } },
    { 0.01,[](int64_t group, int64_t qq) { plist[qq].modifyStamina(-2); return "葡萄糖液，获得2点体力"; } },
    { 0.01,[](int64_t group, int64_t qq) { plist[qq].modifyStamina(-2); return "海市蜃楼，获得2点体力"; } },
    { 0.01,[](int64_t group, int64_t qq) { plist[qq].modifyStamina(1); return "大家好，你被美食图片饿走1点体力"; } },
    { 0.01,[](int64_t group, int64_t qq) { smoke::nosmoking(group, qq, 2); return "禁烟1+1=12套餐，烟起2分钟"; } },
    { 0.005,[](int64_t group, int64_t qq) { plist[qq].modifyStamina(-user::MAX_STAMINA); return "回复石，你感觉神清气爽，体力回满了"; }},

    { 0.01,[](int64_t group, int64_t qq)
    {
        plist[qq].modifyStamina(-3, true);
        using namespace std::string_literals;
        return "小蓝杯，获得3点体力"s + (plist[qq].getStamina(true).staminaAfterUpdate > user::MAX_STAMINA ? "，甚至突破了体力上限"s : ""s);
    }},

    { 0.008,[](int64_t group, int64_t qq)
    {
        auto [enough, stamina, t] = plist[qq].modifyStamina(0);
        plist[qq].modifyStamina(stamina);
        return "加班邮件，你的体力被吸光了";
    }},

    { 0.04,[](int64_t group, int64_t qq) { smoke::nosmoking(group, qq, 1); return "禁烟儿童套餐，烟起1分钟"; }},
    { 0.005,[](int64_t group, int64_t qq) { smoke::nosmoking(group, qq, 5); return "禁烟板烧套餐，烟起5分钟"; }},

    { 0.01,[](int64_t group, int64_t qq)
    {
        plist[qq].modifyCurrency(-50);
        return "仙人掌，你的手被扎成了马蜂窝，花费医药费50批";
    }},

    { 0.01,[](int64_t group, int64_t qq)
    {
		plist[qq].modifyCurrency(-10);
        user_op::extra_tomorrow += 10;
        return "慈善参与奖，花费10批加入明日批池"; 
    }},

    { 0.01,[](int64_t group, int64_t qq)
    {
        double p = randReal();
        if (p < 0.5) p = 1.0 + (p / 0.5);
		plist[qq].multiplyCurrency(p);
        using namespace std::string_literals;
        return "一把菠菜，你的批变成了"s + std::to_string(p) + "倍"s;
    }},

    { 0.01,[](int64_t group, int64_t qq) -> std::string
    {
        if (grp::groups[group].haveMember(qqid_dk))
        {
            smoke::nosmoking(group, qqid_dk, 1);
            return "干死也军车，烟他！";
        }
        else return EVENT_POOL[0].func()(group, qq);
    }},

    { 0.002,[](int64_t group, int64_t qq) -> std::string
    {
        for (auto& [qq, pd] : plist)
        {
            if (grp::groups[group].haveMember(qq))
            {
                auto [enough, stamina, t] = plist[qq].modifyStamina(0);
                plist[qq].modifyStamina(stamina);
                plist[qq].modifyStamina(-2);
            }
        }
        return "二向箔，本群所有人的体力都变成了2点";
    }},

    { 0.002,[](int64_t group, int64_t qq) -> std::string
    {
        for (auto& [qq, pd] : plist)
        {
            if (grp::groups[group].haveMember(qq))
            {
                if (plist[qq].meteor_shield)
                {
                    plist[qq].meteor_shield = false;
                    continue;
                }
				plist[qq].multiplyCurrency(0.9);
            }
        }
        return "陨石，本群所有人的钱包都被炸飞10%";
    }},

    { 0.005,[](int64_t group, int64_t qq) -> std::string
    {
        std::thread([]() {using namespace std::chrono_literals; std::this_thread::sleep_for(2s); user_op::flushDailyTimep(); }).detach();
        return "F5";
    }},

    { 0.01,[](int64_t group, int64_t qq) { return "断钥匙一把，你尝试用来开箱但是失败了"; } },
    { 0.01,[](int64_t group, int64_t qq) { return "机械鼠标球一个，但是没有什么用"; } },
    { 0.01,[](int64_t group, int64_t qq) { return "巨大回车键一个，但是没有什么用"; } },
    { 0.01,[](int64_t group, int64_t qq) { return EMOJI_HAMMER + "，你抽个" + EMOJI_HAMMER; } },
    { 0.01,[](int64_t group, int64_t qq) { return "一团稀有气体，但是没有什么用"; } },

    { 0.002,[](int64_t group, int64_t qq)
    {
		plist[qq].modifyCurrency(-500);
        return "JJ怪，你的家被炸烂了，损失500批";
    } },

    { 0.01,[](int64_t group, int64_t qq)
    {
		plist[qq].modifyCurrency(+10);
		user_op::extra_tomorrow -= 10;
        if (-user_op::extra_tomorrow > user_op::DAILY_BONUS) user_op::extra_tomorrow = -user_op::DAILY_BONUS;
        return "二级钳工认证，从明日批池偷到10批";
    } },

    { 0.01,[](int64_t group, int64_t qq) -> std::string
    {
        if (plist[qq].getKeyCount() < 1) return EVENT_POOL[0].func()(group, qq);
		plist[qq].modifyKeyCount(-1);
        return "空箱子，你什么也没有开到，消耗1把钥匙";
    } },

    { 0.002,[](int64_t group, int64_t qq) -> std::string
    {
        for (auto& [qq, pd] : plist)
        {
            if (grp::groups[group].haveMember(qq))
            {
                plist[qq].modifyStamina(-user::MAX_STAMINA);
            }
        }
        return "一箱黄金胡萝卜，本群所有人的体力都回满了";
    } },

    { 0.002,[](int64_t group, int64_t qq) -> std::string
    {
        for (auto& [qq, pd] : plist)
        {
            if (grp::groups[group].haveMember(qq))
            {
                int bonus = randInt(50, 1000);
				plist[qq].modifyCurrency(+bonus);
            }
        }
        return "大风吹来本群的一堆批，自己查余额看捡到多少哦";
    } },

    { 0.01,[](int64_t group, int64_t qq) -> std::string
    {
        if (grp::groups[group].haveMember(qqid_dk))
        {
            int d = randInt(1, 5);
            smoke::nosmoking(group, qq, d);
            using namespace std::string_literals;
            return "疯批跌坑，你被跌坑禁烟"s + std::to_string(d) + "分钟"s;
        }
        else return EVENT_POOL[0].func()(group, qq);
    } },

    { 0.01,[](int64_t group, int64_t qq) { return EMOJI_HORN + "，傻逼"; } },

    /*
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
        size_t size = 0;
        while (c > 0)
        {
            int tmp = c % 10;
            c /= 10;
            ++size;
            if (tmp != 0) numbers.push_back(tmp);
        }
        if (numbers.empty())
        {
            // 0
            return "融合药水，你的批全部变成了一样的数字";
        }

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
        */

    { 0.005,[](int64_t group, int64_t qq) -> std::string
    {
        std::vector<int64_t> grouplist;
        for (auto& [qq, pd] : plist)
        {
            if (grp::groups[group].haveMember(qq))
                grouplist.push_back(qq);
        }
        size_t idx = randInt(0, grouplist.size() - 1);
        int64_t target = grouplist[idx];
        smoke::nosmoking(group, target, 1);
        using namespace std::string_literals;
        return "禁烟洲际导弹，"s + CQ_At(target) + "不幸被烟1分钟";
    } },


    { 0.002,[](int64_t group, int64_t qq) -> std::string
    {
        plist[qq].meteor_shield = true;
        return "陨石防护罩充能胶囊，免疫下一次陨石伤害";
    } },

    { 0.002,[](int64_t group, int64_t qq) -> std::string
    {
        int64_t cost = plist[qq].getCurrency() * 0.1;
		plist[qq].modifyCurrency(-cost);
        using namespace std::string_literals;
        return "菠菜罚款单，你高强度菠菜被菜市场管理小组发现了，缴纳罚款"s + std::to_string(cost) + "批";
    } },

        /*
    { 0.003,[](int64_t group, int64_t qq) -> std::string
    {
        plist[qq].air_pump_count = 5;
        return "高级气泵，5次抽卡内必出空气";
    } },

    { 0.005,[](int64_t group, int64_t qq) -> std::string
    {
        plist[qq].air_ignore_count = 10;
        return "真空抽气机，10次抽卡内不出空气";
    } },
    */

    { 0.01,[](int64_t group, int64_t qq) -> std::string
    {
        time_t t = time(nullptr);
        for (auto& c : smoke::smokeTimeInGroups)
            for (auto& g : c.second)
                if (t < g.second)
                    CQ_setGroupBan(ac, g.first, c.first, 0);
        return "大暴雨，所有人的烟都被浇灭了";
    } },

    { 0.002,[](int64_t group, int64_t qq) -> std::string
    {
		plist[qq].modifyKeyCount(+50);
        return "王国之链，获得等身大接触式万能钥匙型武器一把，能敲开50个箱子"; 
    } },

    { 0.002,[](int64_t group, int64_t qq) -> std::string
    {
        time_t t = time(nullptr);
        plist[qq].adrenaline_expire_time = t + 15;
        return "肾上腺素，接下来15秒内抽卡不消耗体力";
    } },


    { 0.005,[](int64_t group, int64_t qq) -> std::string
    {
        time_t t = time(nullptr);
        banTime_me = t + 60;
        return "电子烟，bot被烟起1分钟";
    } },

    { 0.001,[](int64_t group, int64_t qq) -> std::string
    {
        time_t t = time(nullptr);
        banTime_me = t + 60 * 5;
        return "流感病毒，bot差点被你毒死，只好休息5分钟";
    } },

    { 0.005,[](int64_t group, int64_t qq) -> std::string
    {
        plist[qq].chaos = true;
        return "世界线震动预警，下一次抽卡很可能会抽到奇怪的东西哦";
    } },
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

            std::stringstream ss;
            time_t t = time(nullptr);
            if (t > plist[qq].adrenaline_expire_time)
            {
                auto [enough, stamina, rtime] = plist[qq].modifyStamina(1);

                if (!enough)
                {
                    ss << CQ_At(qq) << "，你的体力不足，回满还需"
                        << rtime / (60 * 60) << "小时" << rtime / 60 % 60 << "分钟";
                    return ss.str();
                }
                // fall through if stamina is enough
            }
            else
            {
                ss << "肾上腺素生效！";
            }

            event_type reward = EVENT_DEFAULT;
            bool chaos = plist[qq].chaos;
            plist[qq].chaos = false;

            // 气泵最先生效
            if (plist[qq].air_pump_count)
            {
                --plist[qq].air_pump_count;
                reward = EVENT_DEFAULT;
                ss << CQ_At(qq) << "，恭喜你抽到了" << reward.func()(group, qq);
                return ss.str();
            }

            // 抽气机覆盖气泵效果
            if (plist[qq].air_ignore_count)
            {
                --plist[qq].air_ignore_count;
                do {
                    reward = chaos ? draw_event_chaos() : draw_event(randReal());
                } while (reward.prob() == 1.0);
            }
            else // 通常抽卡
            {
                reward = chaos ? draw_event_chaos() : draw_event(randReal());
            }
            ss << CQ_At(qq) << "，恭喜你抽到了" << reward.func()(group, qq);
            return ss.str();
        };
        break;

    default:
        break;
    }
    return c;
}

double event_max = 0.0;
void calc_event_max()
{
    event_max = 0.0;
    for (const auto& e : EVENT_POOL)
    {
        event_max += e.prob();
    }
}

const event_type& draw_event(double p)
{
    p = p * event_max;
    if (p < 0.0 || p > event_max) return EVENT_DEFAULT;
    double totalp = 0;
    for (const auto& c : EVENT_POOL)
    {
        if (p <= totalp + c.prob()) return c;
        totalp += c.prob();
    }
    return EVENT_DEFAULT;        // not match any case
}


const event_type& draw_event_chaos()
{
    int cnt = EVENT_POOL.size() - 1 + 1;  // include default
    int evt = randInt(0, cnt);

    if (evt == cnt)
        return EVENT_DEFAULT;
    else
        return EVENT_POOL[evt];
}

}