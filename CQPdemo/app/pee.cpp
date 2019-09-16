#include <sstream>
#include <iomanip>
#include "cqp.h"
#include "pee.h"
#include "appmain.h"
#include "cpp-base64/base64.h"

namespace pee {

void modifyCurrency(int64_t qq, int64_t c)
{
    db.exec("UPDATE pee SET currency=? WHERE qqid=?", { c, qq });
}
void modifyBoxCount(int64_t qq, int64_t c)
{
    db.exec("UPDATE pee SET cases=? WHERE qqid=?", { c, qq });
}
void modifyDrawTime(int64_t qq, time_t c)
{
    db.exec("UPDATE pee SET dailytime=? WHERE qqid=?", { c, qq });
}
void modifyKeyCount(int64_t qq, int64_t c)
{
    db.exec("UPDATE pee SET keys=? WHERE qqid=?", { c, qq });
}
int64_t nosmoking(int64_t group, int64_t target, int duration)
{
    if (duration < 0) return -1;
    const char* cqinfo = CQ_getGroupMemberInfoV2(ac, group, target, FALSE);
    if (cqinfo && strlen(cqinfo) > 0)
    {
        std::string decoded = base64_decode(std::string(cqinfo));
        if (!decoded.empty())
        {
            if (getPermissionFromGroupInfoV2(decoded.c_str()) >= 2) return -2;
            CQ_setGroupBan(ac, group, target, int64_t(duration) * 60);
            if (duration > 0) smokeGroups[target][group] = time(nullptr) + int64_t(duration) * 60;
            else if (duration == 0) smokeGroups[target].erase(group);
            return duration;
        }
    }
    return -1;
}

std::string nosmokingWrapper(int64_t qq, int64_t group, int64_t target, int64_t cost)
{
    int duration = (int)cost;
    if (duration > 30 * 24 * 60) duration = 30 * 24 * 60;
    cost *= cost; // 打土豪 分批

    if (cost < 0) return "你会不会烟人？";

    if (cost > plist[qq].currency) return std::string(CQ_At(qq)) + "，你的余额不足，需要" + std::to_string(cost) + "个批";

    if (cost == 0)
    {
        nosmoking(group, target, duration);
        return "解禁了";
    }

    double reflect = randReal();
    if (reflect < 0.1) return "烟突然灭了，烟起失败";
    else if (reflect < 0.3)
    {
        auto ret = nosmoking(group, qq, duration);
        if (ret > 0)
        {
            return "你自己烟起吧";
        }
        else if (ret == -2)
        {
            return "烟突然灭了，烟起失败";
        }
        else if (ret == 0 || ret == -1)
        {
            return "你会不会烟人？";
        }
    }
    else
    {
        auto ret = nosmoking(group, target, duration);
        if (ret > 0)
        {
            plist[qq].currency -= cost;
            modifyCurrency(qq, plist[qq].currency);
            std::stringstream ss;
            if (qq == target)
                ss << "？我从来没听过这样的要求，消费" << cost << "个批";
            else
                ss << "烟起哦，消费" << cost << "个批";
            return ss.str();
        }
        else if (ret == 0 || ret == -1)
        {
            return "你会不会烟人？";
        }
        else if (ret == -2)
        {
            return "禁烟管理请联系群主哦";
        }
    }
    return "你会不会烟人？";
}

std::tuple<bool, int, time_t> updateStamina(int64_t qq, int cost, bool extra)
{
    time_t t = time(nullptr);
    time_t last = staminaRecoveryTime[qq];
    int stamina = MAX_STAMINA;
    if (last > t) stamina -= (last - t) / STAMINA_TIME + !!((last - t) % STAMINA_TIME);

    bool enough = false;

    if (cost > 0)
    {
        if (staminaExtra[qq] >= cost)
        {
            enough = true;
            staminaExtra[qq] -= cost;
        }
        else if (stamina + staminaExtra[qq] >= cost)
        {
            enough = true;
            cost -= staminaExtra[qq];
            staminaExtra[qq] = 0;
            stamina -= cost;
        }
        else
        {
            enough = false;
        }
    }
    else if (cost < 0)
    {
        enough = true;
        if (stamina - cost <= MAX_STAMINA)
        {
            // do nothing
        }
        else if (extra) // part of cost(recovery) goes to extra
        {
            staminaExtra[qq] += stamina - cost - MAX_STAMINA;
        }
    }

    if (enough)
    {
        if (last > t)
            staminaRecoveryTime[qq] += STAMINA_TIME * cost;
        else
            staminaRecoveryTime[qq] = t + STAMINA_TIME * cost;
    }

    if (enough && stamina >= MAX_STAMINA) staminaRecoveryTime[qq] = t;
    return { enough, stamina, staminaRecoveryTime[qq] - t };
}

std::tuple<bool, int, time_t> testStamina(int64_t qq, int cost)
{
    time_t t = time(nullptr);
    time_t last = staminaRecoveryTime[qq];
    int stamina = MAX_STAMINA;
    if (last > t) stamina -= (last - t) / STAMINA_TIME + !!((last - t) % STAMINA_TIME);

    bool enough = false;

    if (cost > 0)
    {
        if (staminaExtra[qq] >= cost)
        {
            enough = true;
        }
        else if (stamina + staminaExtra[qq] >= cost)
        {
            enough = true;
        }
        else
        {
            enough = false;
        }
    }

    if (enough)
    {
        if (last > t)
            last += STAMINA_TIME * cost;
        else
            last = t + STAMINA_TIME * cost;
    }

    if (enough && stamina >= MAX_STAMINA) last = t;
    return { enough, stamina, last - t };
}

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
    case commands::开通提示:
        c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
        {
            if (plist.find(qq) != plist.end()) return std::string(CQ_At(qq)) + "，你已经注册过了";

            return "是我要开通菠菜，你会不会开通菠菜";
        };
        break;
    case commands::开通:
        c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
        {
            if (plist.find(qq) != plist.end()) return std::string(CQ_At(qq)) + "，你已经注册过了";

            plist[qq].currency = INITIAL_BALANCE;
            db.exec("INSERT INTO pee(qqid, currency, cases, dailytime, keys) VALUES(? , ? , ? , ? , ?)",
                { qq, plist[qq].currency, 0, 0, 0 });

            std::stringstream ss;
            ss << "你可以开始开箱了，送给你" << plist[qq].currency << "个批";
            return ss.str();
        };
        break;
    case commands::余额:
        c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
        {
            if (plist.find(qq) == plist.end()) return std::string(CQ_At(qq)) + "，你还没有开通菠菜";
            std::stringstream ss;
            ss << CQ_At(qq) << "，你的余额为" << plist[qq].currency << "个批，"
                << plist[qq].keys << "把钥匙";
            auto[enough, stamina, rtime] = updateStamina(qq, 0);
            ss << "\n你还有" << stamina + staminaExtra[qq] << "点体力";
            if (stamina < MAX_STAMINA)
                ss << "，回满还需" << rtime / (60 * 60) << "小时" << rtime / 60 % 60 << "分钟";
            return ss.str();
        };
        break;
    case commands::禁烟:
        c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
        {
            const char* cqinfo = CQ_getGroupMemberInfoV2(ac, group, CQ_getLoginQQ(ac), FALSE);
            if (cqinfo && strlen(cqinfo) > 0)
            {
                std::string decoded = base64_decode(std::string(cqinfo));
                if (!decoded.empty())
                {
                    if (getPermissionFromGroupInfoV2(decoded.c_str()) < 2)
                        return "";
                }
            }

            if (plist.find(qq) == plist.end()) return std::string(CQ_At(qq)) + "，你还没有开通菠菜";

            int64_t target = 0;
            if (true || args.size() == 1)
            {
                if (prevUser.find(group) == prevUser.end()) return "";
                target = prevUser[group];
            }

            int64_t cost = -1;
            try {
                if (args.size() >= 2)
                    cost = std::stoll(args[1]);
            }
            catch (std::exception&) {
                //ignore
            }

            return nosmokingWrapper(qq, group, target, cost);

        };
        break;
    case commands::解禁:
        c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
        {
            const char* cqinfo = CQ_getGroupMemberInfoV2(ac, group, CQ_getLoginQQ(ac), FALSE);
            if (cqinfo && strlen(cqinfo) > 0)
            {
                std::string decoded = base64_decode(std::string(cqinfo));
                if (!decoded.empty())
                {
                    if (getPermissionFromGroupInfoV2(decoded.c_str()) < 2)
                        return "";
                }
            }

            if (args.size() < 1) return "";
            try {
                //unsmoke(std::stoll(args[1]));
                CQ_setGroupBan(ac, group, std::stoll(args[1]), 0);
                return "";
            }
            catch (std::exception&) {}
            return "";
        };
        break;
    case commands::领批:
        c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
        {
            if (plist.find(qq) == plist.end()) return std::string(CQ_At(qq)) + "，你还没有开通菠菜";

            time_t lastDailyTime = plist[qq].last_draw_time;
            if (lastDailyTime > daily_refresh_time)
            {
                std::stringstream ss;
                ss << CQ_At(qq) << "，你今天已经领过了，明天再来8";
                return ss.str();
            }

            std::stringstream ss;
            ss << CQ_At(qq) << "，你今天领到" << FREE_BALANCE_ON_NEW_DAY << "个批";
            if (remain_daily_bonus)
            {
                int bonus = randInt(1, remain_daily_bonus > 66 ? 66 : remain_daily_bonus);
                remain_daily_bonus -= bonus;
                plist[qq].currency += FREE_BALANCE_ON_NEW_DAY + bonus;

                ss << "，甚至还有先到的" << bonus << "个批\n"
                    << "现在批池还剩" << remain_daily_bonus << "个批";
            }
            else
            {
                plist[qq].currency += FREE_BALANCE_ON_NEW_DAY;
                ss << "\n每日批池么得了，明天请踩点";
            }
            plist[qq].last_draw_time = time(nullptr);
            modifyCurrency(qq, plist[qq].currency);
            modifyDrawTime(qq, plist[qq].last_draw_time);
            return ss.str();
            /*
            if (plist[qq].last_draw_time > daily_time_point)
            if (remain_daily_bonus > 0)
            {
                int bonus = randInt(0, remain_daily_bonus);
                remain_daily_bonus -= bonus;
            }
            */
            return "";
        };
        break;
    case commands::生批:
        c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
        {
            if (plist.find(qq) == plist.end()) return std::string(CQ_At(qq)) + "，你还没有开通菠菜";

            auto[enough, stamina, rtime] = updateStamina(qq, 1);

            std::stringstream ss;
            if (!enough) ss << CQ_At(qq) << "，你的体力不足，回满还需"
                << rtime / (60 * 60) << "小时" << rtime / 60 % 60 << "分钟";

            plist[qq].currency += 1;
            modifyCurrency(qq, plist[qq].currency);

            return std::string(CQ_At(qq)) + "消耗1点体力得到1个批";
        };
        break;
    case commands::开箱:
        c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
        {
            if (plist.find(qq) == plist.end()) return std::string(CQ_At(qq)) + "，你还没有开通菠菜";
            //CQ_setGroupBan(ac, group, qq, 60);
            //return "不准开";

            std::stringstream ss;
            if (plist[qq].keys >= 1)
            {
                plist[qq].keys--;
                modifyKeyCount(qq, plist[qq].keys);
            }
            else if (plist[qq].currency >= FEE_PER_CASE)
            {
                auto [enough, stamina, rtime] = updateStamina(qq, 1);
                if (!enough)
                {
                    ss << CQ_At(qq) << "，你的体力不足，回满还需"
                        << rtime / (60 * 60) << "小时" << rtime / 60 % 60 << "分钟";
                    return ss.str();
                }
                plist[qq].currency -= FEE_PER_CASE;
            }
            else
                return std::string(CQ_At(qq)) + "，你的余额不足";

            const case_detail& reward = draw_case(randReal());
            if (reward > 300) ss << "歪哟，" << CQ_At(qq) << "发了，开出了";
            else ss << CQ_At(qq) << "，恭喜你开出了";
            ss << reward.full() << "，获得" << reward.worth() << "个批";

            plist[qq].currency += reward.worth();
            if (plist[qq].currency < 0) plist[qq].currency = 0;
            modifyCurrency(qq, plist[qq].currency);
            modifyBoxCount(qq, ++plist[qq].opened_box_count);
            //ss << "你还有" << stamina << "点体力，";

            return ss.str();
        };
        break;
    case commands::开箱10:
        c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
        {
            if (plist.find(qq) == plist.end()) return std::string(CQ_At(qq)) + "，你还没有开通菠菜";
            //CQ_setGroupBan(ac, group, qq, 60);
            //return "不准开";

            std::stringstream ss;
            if (plist[qq].keys >= 10)
            {
                plist[qq].keys -= 10;
                modifyKeyCount(qq, plist[qq].keys);
            }
            else if (plist[qq].currency >= FEE_PER_CASE * 10)
            {
                auto [enough, stamina, rtime] = updateStamina(qq, 10);
                if (!enough)
                {
                    ss << CQ_At(qq) << "，你的体力不足，回满还需"
                        << rtime / (60 * 60) << "小时" << rtime / 60 % 60 << "分钟";
                    return ss.str();
                }
                plist[qq].currency -= FEE_PER_CASE * 10;
            }
            else
                return std::string(CQ_At(qq)) + "，你的余额不足";

            std::vector<int> case_counts(CASE_POOL.size() + 1, 0);
            int r = 0;
            /*
            for (size_t i = 0; i < 10; ++i)
            {
                const case_detail& reward = draw_case(randReal());
                case_counts[reward.type_idx()]++;
                r += reward.worth();
            }
            if (r > 300) ss << "歪哟，" << CQ_At(qq) << "发了，开出了";
            else ss << CQ_At(qq) << "，恭喜你开出了";
            for (size_t i = 0; i < case_counts.size(); ++i)
            {
                if (case_counts[i])
                {
                    ss << case_counts[i] << "个" <<
                        ((i == CASE_POOL.size()) ? CASE_DEFAULT.name() : CASE_POOL[i].name()) << "，";
                }
            }
            ss << "一共" << r << "个批";
            */

            ss << CQ_At(qq) << "，恭喜你开出了：\n";
            for (size_t i = 0; i < 10; ++i)
            {
                const case_detail& reward = draw_case(randReal());
                case_counts[reward.type_idx()]++;
                r += reward.worth();
                ss << "- " << reward.full() << " (" << reward.worth() << "批)\n";
            }
            ss << "上面有";
            for (size_t i = 0; i < case_counts.size(); ++i)
            {
                if (case_counts[i])
                {
                    ss << case_counts[i] << "个" <<
                        ((i == CASE_POOL.size()) ? CASE_DEFAULT.name() : CASE_POOL[i].name()) << "，";
                }
            }
            ss << "一共" << r << "个批";

            plist[qq].currency += r;
            if (plist[qq].currency < 0) plist[qq].currency = 0;
            plist[qq].opened_box_count += 10;
            modifyCurrency(qq, plist[qq].currency);
            modifyBoxCount(qq, plist[qq].opened_box_count);

            return ss.str();
        };
        break;
    case commands::开红箱:
        c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
        {
            if (plist.find(qq) == plist.end()) return std::string(CQ_At(qq)) + "，你还没有开通菠菜";
            //CQ_setGroupBan(ac, group, qq, 60);
            //return "不准开";

            if (plist[qq].currency < COST_OPEN_RED)
                return std::string(CQ_At(qq)) + "，你的余额不足";

            auto[enough, stamina, rtime] = updateStamina(qq, COST_OPEN_RED_STAMINA);

            std::stringstream ss;
            if (!enough) ss << CQ_At(qq) << "，你的体力不足，回满还需"
                << rtime / (60 * 60) << "小时" << rtime / 60 % 60 << "分钟";
            else
            {
                plist[qq].currency -= COST_OPEN_RED;
                ss << getCard(group, qq) << "消耗" << COST_OPEN_RED << "个批和" << COST_OPEN_RED_STAMINA <<"点体力发动技能！\n";

                std::vector<int> case_counts(CASE_POOL.size() + 1, 0);
                int count = 0;
                int cost = 0;
                int res = 0;
                case_detail reward;
                do {
                    ++count;
                    cost += FEE_PER_CASE;
                    reward = draw_case(randReal());
                    case_counts[reward.type_idx()]++;
                    res += reward.worth();
                    plist[qq].currency += reward.worth() - FEE_PER_CASE;
                    plist[qq].opened_box_count++;

                    if (reward.type_idx() == 2)
                    {
                        ss << CQ_At(qq) << "开了" << count << "个箱子终于开出了" << reward.full() << " (" << reward.worth() << "批)，"
                            << "本次净收益" << res - cost - COST_OPEN_RED << "个批";
                        if (plist[qq].currency < 0) plist[qq].currency = 0;
                        modifyCurrency(qq, plist[qq].currency);
                        modifyBoxCount(qq, plist[qq].opened_box_count);
                        return ss.str();
                    }
                    else
                        if (reward.type_idx() == 1)
                        {
                            ss << "歪哟，" << CQ_At(qq) << "发了，开了" << count << "个箱子居然开出了" << reward.full() << " (" << reward.worth() << "批)，"
                                << "本次净收益" << res - cost - COST_OPEN_RED << "个批";
                            if (plist[qq].currency < 0) plist[qq].currency = 0;
                            modifyCurrency(qq, plist[qq].currency);
                            modifyBoxCount(qq, plist[qq].opened_box_count);
                            return ss.str();
                        }
                } while (plist[qq].currency >= FEE_PER_CASE);

                ss << CQ_At(qq) << "破产了，开了" << count << "个箱子也没能开出红箱，"
                    << "本次净收益" << res - cost - COST_OPEN_RED << "个批";
                if (plist[qq].currency < 0) plist[qq].currency = 0;
                modifyCurrency(qq, plist[qq].currency);
                modifyBoxCount(qq, ++plist[qq].opened_box_count);
                return ss.str();

                //ss << "你还有" << stamina << "点体力，";
            }

            return ss.str();
        };
        break;
    case commands::开黄箱:
        c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
        {
            if (plist.find(qq) == plist.end()) return std::string(CQ_At(qq)) + "，你还没有开通菠菜";
            //CQ_setGroupBan(ac, group, qq, 60);
            //return "不准开";

            if (plist[qq].currency < COST_OPEN_YELLOW)
                return std::string(CQ_At(qq)) + "，你的余额不足";

            auto[enough, stamina, rtime] = updateStamina(qq, MAX_STAMINA);

            std::stringstream ss;
            if (!enough) ss << CQ_At(qq) << "，你的体力不足，回满还需"
                << rtime / (60 * 60) << "小时" << rtime / 60 % 60 << "分钟";
            else
            {
                plist[qq].currency -= COST_OPEN_YELLOW;
                ss << getCard(group, qq) << "消耗" << COST_OPEN_YELLOW << "个批和全部体力发动技能！\n";

                std::vector<int> case_counts(CASE_POOL.size() + 1, 0);
                int count = 0;
                int cost = 0;
                int res = 0;
                case_detail reward;
                do {
                    ++count;
                    cost += FEE_PER_CASE;
                    reward = draw_case(randReal());
                    case_counts[reward.type_idx()]++;
                    res += reward.worth();
                    plist[qq].currency += reward.worth() - FEE_PER_CASE;
                    plist[qq].opened_box_count++;

                    if (reward.type_idx() == 1)
                    {
                        ss << CQ_At(qq) << "开了" << count << "个箱子终于开出了" << reward.full() << " (" << reward.worth() << "批)，"
                            << "本次净收益" << res - cost - COST_OPEN_YELLOW << "个批";
                        if (plist[qq].currency < 0) plist[qq].currency = 0;
                        modifyCurrency(qq, plist[qq].currency);
                        modifyBoxCount(qq, plist[qq].opened_box_count);
                        return ss.str();
                    }
                } while (plist[qq].currency >= FEE_PER_CASE);

                ss << CQ_At(qq) << "破产了，开了" << count << "个箱子也没能开出黄箱，"
                    << "本次净收益" << res - cost - COST_OPEN_YELLOW << "个批";
                if (plist[qq].currency < 0) plist[qq].currency = 0;
                modifyCurrency(qq, plist[qq].currency);
                modifyBoxCount(qq, plist[qq].opened_box_count);
                return ss.str();

                //ss << "你还有" << stamina << "点体力，";
            }

            return ss.str();
        };
        break;

    case commands::开箱endless:
        c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
        {
            return "梭哈台被群主偷了，没得梭了";
        };
        break;
        /*
    case commands::开箱endless:
        c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
        {
            if (plist.find(qq) == plist.end()) return std::string(CQ_At(qq)) + "，你还没有开通菠菜";
            //CQ_setGroupBan(ac, group, qq, 60);
            //return "不准开";

            if (plist[qq].currency < COST_OPEN_ENDLESS)
                return std::string(CQ_At(qq)) + "，你的余额不足";

            auto [enough, stamina, rtime] = updateStamina(qq, COST_OPEN_ENDLESS_STAMINA);

            std::stringstream ss;
            if (!enough) ss << CQ_At(qq) << "，你的体力不足，回满还需"
                << rtime / (60 * 60) << "小时" << rtime / 60 % 60 << "分钟";
            else
            {
                plist[qq].currency -= COST_OPEN_ENDLESS;
                long long total_cases = plist[qq].currency / FEE_PER_CASE;
                plist[qq].currency %= FEE_PER_CASE;
                long long extra_cost = 0.1 * total_cases * FEE_PER_CASE;
                total_cases -= long long(std::floor(total_cases * 0.1));
                ss << getCard(group, qq) << "消耗" << COST_OPEN_ENDLESS + extra_cost << "个批和" << COST_OPEN_ENDLESS_STAMINA << "点体力发动技能！\n";
                std::vector<int> case_counts(CASE_POOL.size() + 1, 0);
                int count = 0;
                int cost = 0;
                int res = 0;
                case_detail max;
                case_detail reward;
                do {
                    ++count;
                    cost += FEE_PER_CASE;
                    reward = draw_case(randReal());
                    case_counts[reward.type_idx()]++;
                    res += reward.worth();
                    plist[qq].currency += reward.worth();
                    plist[qq].opened_box_count++;

                    if (max.worth() < reward.worth()) max = reward;
                } while (--total_cases > 0);

                ss << CQ_At(qq) << "本次梭哈开了" << count << "个箱子，开出了" << case_counts[1] << "个黄箱，" << case_counts[2] << "个红箱，" << case_counts[0] << "个黑箱，\n"
                    << "最值钱的是" << max.full() << " (" << max.worth() << "批)，"
                    << "本次净收益" << res - cost - COST_OPEN_ENDLESS - extra_cost << "个批";
                if (plist[qq].currency < 0) plist[qq].currency = 0;
                modifyCurrency(qq, plist[qq].currency);
                modifyBoxCount(qq, plist[qq].opened_box_count);
                return ss.str();

                //ss << "你还有" << stamina << "点体力，";
            }

            return ss.str();
        };
        break;
        */
    default: break;
    }

    return c;
}

command smokeIndicator(const char* msg)
{
    if (msg == nullptr) return command();
    std::stringstream ss(msg);
    std::vector<std::string> query;
    while (ss)
    {
        std::string s;
        ss >> s;
        if (!s.empty())
            query.push_back(s);
    }
    if (query.empty()) return command();

    auto cmd = query[0];
    if (cmd.length() <= 4 || cmd.substr(0, 4) != "禁烟") return command();

    command c;
    c.args = query;
    c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw)->std::string
    {
        if (args.size() == 1)
        {
            return "";
        }
        auto cmd = args[0];
        // TODO find qqid from target nick name
        std::string targetName = cmd.substr(4);
        int64_t target = 0;
        if (qqid_str.find(targetName) != qqid_str.end())
            target = qqid_str[targetName];

        int64_t cost = -1;
        try {
            cost = std::stoll(args[1]);
        }
        catch (std::exception&) {
            //ignore
        }

        return nosmokingWrapper(qq, group, target, cost);
    };

    return c;
}

void peeCreateTable()
{
    if (db.exec(
        "CREATE TABLE IF NOT EXISTS pee( \
            qqid    INTEGER PRIMARY KEY NOT NULL, \
            currency INTEGER            NOT NULL, \
            cases   INTEGER             NOT NULL, \
            dailytime INTEGER           NOT NULL,  \
            keys    INTEGER             NOT NULL  \
         )") != SQLITE_OK)
        CQ_addLog(ac, CQLOG_ERROR, "pee", db.errmsg());
}

void peeLoadFromDb()
{
    auto list = db.query("SELECT * FROM pee", 5);
    for (auto& row : list)
    {
        int64_t qq = std::any_cast<int64_t>(row[0]);
        int64_t p1, p2, p4;
        time_t  p3;
        p1 = std::any_cast<int64_t>(row[1]);
        p2 = std::any_cast<int64_t>(row[2]);
        p3 = std::any_cast<time_t> (row[3]);
        p4 = std::any_cast<int64_t>(row[4]);
        plist[qq] = { p1, p2, p3, p4 };
        plist[qq].freeze_assets_expire_time = INT64_MAX;
    }
    char msg[128];
    sprintf(msg, "added %u users", plist.size());
    CQ_addLog(ac, CQLOG_DEBUG, "pee", msg);
}

const double UNSMOKE_COST_RATIO = 3;
std::string unsmoke(int64_t qq)
{
    if (smokeGroups.find(qq) == smokeGroups.end() || smokeGroups[qq].empty())
        return "你么得被烟啊";

    if (plist.find(qq) == plist.end()) return "你还没有开通菠菜";

    time_t t = time(nullptr);
    int total_remain = 0;
    for (auto& g : smokeGroups[qq])
    {
        if (t <= g.second)
        {
            int remain = (g.second - t) / 60; // min
            int extra = (g.second - t) % 60;  // sec
            total_remain += remain + !!extra;
        }
    }
    std::stringstream ss;
    ss << "你在" << smokeGroups[qq].size() << "个群被烟" << total_remain << "分钟，";
    int total_cost = total_remain * UNSMOKE_COST_RATIO;
    if (plist[qq].currency < total_cost)
    {
        ss << "你批不够，需要" << total_cost << "个批，哈";
    }
    else
    {
        plist[qq].currency -= total_cost;
        ss << "本次接近消费" << total_cost << "个批";
        modifyCurrency(qq, plist[qq].currency);
        for (auto& g : smokeGroups[qq])
        {
            if (t > g.second) continue;
            std::string qqname = getCard(g.first, qq);
            CQ_setGroupBan(ac, g.first, qq, 0);
            std::stringstream sg;
            int remain = (g.second - t) / 60; // min
            int extra = (g.second - t) % 60;  // sec
            sg << qqname << "花费巨资" << (long long)std::round((remain + !!extra) * UNSMOKE_COST_RATIO) << "个批申请接近成功";
            CQ_sendGroupMsg(ac, g.first, sg.str().c_str());
        }
        smokeGroups[qq].clear();
    }
    return ss.str();
}

void flushDailyTimep(bool autotriggered)
{
    daily_refresh_time = time(nullptr);
    daily_refresh_tm = *localtime(&daily_refresh_time);
    if (autotriggered) daily_refresh_tm_auto = daily_refresh_tm;

    remain_daily_bonus = DAILY_BONUS + extra_tomorrow;
    extra_tomorrow = 0;

    CQ_sendGroupMsg(ac, 479733965, "每日批池刷新了；额");
    CQ_sendGroupMsg(ac, 391406854, "每日批池刷新了；额");
    CQ_addLog(ac, CQLOG_DEBUG, "pee", std::to_string(daily_refresh_time).c_str());
}

const case_detail& draw_case(double p)
{
    size_t idx = 0;

    if (p >= 0.0 && p <= 1.0) 
    {
        double totalp = 0;
        for (const auto& c : CASE_POOL)
        {
            if (p <= totalp + c.prob()) break;
            ++idx;
            totalp += c.prob();
        }
        // idx = CASE_POOL.size() if not match any case
    }

    size_t detail_idx = randInt(0, CASE_DETAILS[idx].size() - 1);
    return CASE_DETAILS[idx][detail_idx];
}

}
