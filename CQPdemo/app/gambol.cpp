#include "gambol.h"
#include "cqp.h"
#include "appmain.h"
#include <sstream>
#include <set>
#include <thread>
#include "cqp_ex.h"
#include "utils/string_util.h"
#include "utils/rand.h"
#include "data/user.h"

using namespace std::string_literals;
std::string str_put_fail = "你会不会"s + EMOJI_DOWN + "注？";
std::string str_nobody = "無人"s + EMOJI_DOWN + "注，取消本局";
const char* cstr_put_fail = str_put_fail.c_str();
const char* cstr_nobody = str_nobody.c_str();

namespace gambol
{
using user::plist;

std::map<int64_t, gameData> groupMap;

///////////////////////////////////////////////////////////////////////////////

command msgDispatcher(const char* msg)
{
    command c;
    auto query = msg2args(msg);
    if (query.empty()) return c;

    auto cmd = query[0];

    c.args = query;

    if (cmd.substr(0, 4) == "摇号" && cmd.length() > 4 && query.size() > 1)
    {
        cmd = "摇号";
        c.args.resize(3);
        c.args[0] = cmd;
        c.args[1] = query[0].substr(4);
        c.args[2] = query[1];
    }
    else if (cmd.substr(0, 4) != "摇号" && cmd.substr(0, 2) == "摇" && cmd.length() > 2 && query.size() > 1)
    {
        cmd = "摇号";
        c.args.resize(3);
        c.args[0] = cmd;
        c.args[1] = query[0].substr(2);
        c.args[2] = query[1];
    }
    if (commands_str.find(cmd) == commands_str.end()) return c;

    switch (c.c = commands_str[cmd])
    {
    case commands::flipcoin:
        c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
        {
            if (user::plist.find(qq) == user::plist.end()) return std::string(CQ_At(qq)) + "，你还没有开通菠菜";

            flipcoin::roundStart(group);
            if (args.size() > 2)
            {
                int64_t p = 0;
                try {
                    p = std::stoll(args[2]);
                }
                catch (std::exception&) {}
                if (p < 0) p = 0;

                if (p <= 0) return "";
                else if (args[1] == "正") flipcoin::put(group, qq, { p, 0 });
                else if (args[1] == "反") flipcoin::put(group, qq, { 0, p });
            }
            return "";
        };
        break;
    case commands::正:
        c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
        {
            if (user::plist.find(qq) == user::plist.end()) return std::string(CQ_At(qq)) + "，你还没有开通菠菜";

            if (args.size() > 1)
            {
                int64_t p = 0;
                try {
                    p = std::stoll(args[1]);
                }
                catch (std::exception&) {}
                if (p < 0) p = 0;

                if (p <= 0) return "";
                flipcoin::put(group, qq, { p, 0 });
                return "";
            }
            return cstr_put_fail;
        };
        break;
    case commands::反:
        c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
        {
            if (user::plist.find(qq) == user::plist.end()) return std::string(CQ_At(qq)) + "，你还没有开通菠菜";

            if (args.size() > 1)
            {
                int64_t p = 0;
                try {
                    p = std::stoll(args[1]);
                }
                catch (std::exception&) {}
                if (p < 0) p = 0;

                if (p <= 0) return "";
                flipcoin::put(group, qq, { 0, p });
                return "";
            }
            return cstr_put_fail;
        };
        break;


    case commands::开始摇号:
        c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
        {
            if (user::plist.find(qq) == user::plist.end()) return std::string(CQ_At(qq)) + "，你还没有开通菠菜";

            roulette::roundStart(group);
            if (args.size() > 2)
            {
                int64_t p = 0;
                try {
                    p = std::stoll(args[2]);
                }
                catch (std::exception&) {}
                if (p < 0) p = 0;

                std::string subc = args[1];
                if (p <= 0 || roulette::gridTokens.find(subc) == roulette::gridTokens.end()) return "";
                else roulette::put(group, qq, roulette::gridTokens.at(subc), p);
            }
            return "";
        };
        break;

    case commands::摇号:
        c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
        {
            if (user::plist.find(qq) == user::plist.end()) return std::string(CQ_At(qq)) + "，你还没有开通菠菜";

            if (args.size() > 2)
            {
                int64_t p = 0;
                try {
                    p = std::stoll(args[2]);
                }
                catch (std::exception&) {}
                if (p < 0) p = 0;

                std::string subc = args[1];
                if (p <= 0 || roulette::gridTokens.find(subc) == roulette::gridTokens.end()) return "";
                else roulette::put(group, qq, roulette::gridTokens.at(subc), p);
            }
            return "";
        };
        break;

    default:
        break;
    }
    return c;
}

///////////////////////////////////////////////////////////////////////////////
// flipcoin

namespace flipcoin
{

void roundStart(int64_t group)
{
    if (groupMap[group].flipcoin_running)
    {
        CQ_sendGroupMsg(ac, group, "There is already a game running at this group.");
        return;
    }

    CQ_sendGroupMsg(ac, group, "新一轮的翻批开始了，请群员踊跃参加");
	groupMap[group].flipcoin_running = true;
	groupMap[group].flipcoin = {};

	groupMap[group].flipcoin.startTime = time(nullptr);
    std::thread([=]() {
        using namespace std::chrono_literals;

        // 40s
        std::this_thread::sleep_for(20s);
        roundAnnounce(group);

        // 20s
        std::this_thread::sleep_for(20s);
        roundAnnounce(group);

        // 10s
        std::this_thread::sleep_for(10s);
        roundAnnounce(group);

        // 5s
        std::this_thread::sleep_for(5s);
        //flipcoin::roundAnnounce(group);

        // end
        std::this_thread::sleep_for(5s);
        roundEnd(group);

        }).detach();
}

void roundAnnounce(int64_t group)
{
    if (groupMap[group].flipcoin_running)
    {
        CQ_sendGroupMsg(ac, group, "No flipcoin round is running at this group.");
        return;
    }

    std::stringstream ss;
    const auto& r = groupMap[group].flipcoin;
    ss << "本轮翻批" << EMOJI_DOWN << "注时间还剩" << r.startTime + 60 - time(nullptr) << "秒，当前" << EMOJI_DOWN << "注情况：\n";

    ss << "总计" << r.total << "个批，正面" << r.front << "个，反面" << r.back << "个";
    if (r.total > 0)
    {
        ss << "\n";
        ss << "正面：";
        for (const auto& [qq, stat] : r.pee_per_player)
        {
            if (stat.front)
                ss << getCard(group, qq) << "(" << stat.front << "个批) "
                << "(" << 50.0 * stat.front / r.front << "%)  ";
        }
        ss << "\n";
        ss << "反面：";
        for (const auto& [qq, stat] : r.pee_per_player)
        {
            if (stat.back)
                ss << getCard(group, qq) << "(" << stat.back << "个批) "
                << "(" << 50.0 * stat.back / r.back << "%)  ";
        }
    }
    CQ_sendGroupMsg(ac, group, ss.str().c_str());
}

void roundEnd(int64_t group)
{
    if (groupMap[group].flipcoin_running)
    {
        CQ_sendGroupMsg(ac, group, "No flipcoin round is running at this group.");
        return;
    }

    const auto& r = groupMap[group].flipcoin;
    if (r.front == 0 || r.back == 0)
    {
        CQ_sendGroupMsg(ac, group, cstr_nobody);
        roundCancel(group);
        return;
    }

    bool front = randInt(0, 1);
    double per = randReal();

    int64_t deno = front ? r.front : r.back;
    double totalper = 0;
    int64_t winner;

    if (deno != 0)
    {
        for (const auto& [qq, bet] : r.pee_per_player)
        {
            totalper += 1.0 * (front ? bet.front : bet.back) / deno;
            if (totalper >= per)
            {
                winner = qq;
                break;
            }
        }
    }

    std::stringstream ss;
    ss << "本轮翻批结束，结果为" << (front ? "正面" : "反面") << "，";
    if (deno != 0)
    {
        auto& w = r.pee_per_player.at(winner);
        int64_t bet = (front ? w.front : w.back);
        ss << CQ_At(winner) << "以" << 50.0 * bet / deno
            << "%的概率赢得了大奖" << r.total << "个批！";
    }
    else
        ss << r.total << "个批么得了";
    CQ_sendGroupMsg(ac, group, ss.str().c_str());

	user::plist[winner].modifyCurrency(+r.total);
    groupMap[group].flipcoin_running = false;
}

void roundCancel(int64_t group)
{
    if (groupMap[group].flipcoin_running)
        return;

    for (const auto& [qq, stat] : groupMap[group].flipcoin.pee_per_player)
    {
        if (stat.front) user::plist[qq].modifyCurrency(+stat.front);
		if (stat.back)  user::plist[qq].modifyCurrency(+stat.back);
    }

    groupMap[group].flipcoin_running = false;
    CQ_sendGroupMsg(ac, group, "批不翻了，返还所有批");
}

void put(int64_t group, int64_t qq, bet bet)
{
    if (groupMap[group].flipcoin_running)
    {
        CQ_sendGroupMsg(ac, group, "本群么得开盘啊");
        return;
    }

    if (bet.front + bet.back <= 0)
    {
        CQ_sendGroupMsg(ac, group, "你就是负批怪？");
        return;
    }

    if (user::plist[qq].getCurrency() < bet.front + bet.back)
    {
        std::string s = CQ_At(qq) + "，你的余额不足";
        CQ_sendGroupMsg(ac, group, s.c_str());
        return;
    }

    auto& round = groupMap[group].flipcoin;
    auto& player = round.pee_per_player[qq];
    player.front += bet.front;
    player.back += bet.back;

    round.front += bet.front;
    round.back += bet.back;
    round.total += bet.front + bet.back;

	user::plist[qq].modifyCurrency(-(bet.front + bet.back));

    std::stringstream ss;
    if (bet.front)
    {
        if (player.front == bet.front)
            ss << getCard(group, qq) << "成功" << EMOJI_DOWN << "注" << "正面" << bet.front << "个批";
        else
            ss << getCard(group, qq) << "成功" << EMOJI_DOWN << "注" << "正面" << "到" << player.front << "个批";
    }
    else if (bet.back)
    {
        if (player.back == bet.back)
            ss << getCard(group, qq) << "成功" << EMOJI_DOWN << "注" << "反面" << bet.back << "个批";
        else
            ss << getCard(group, qq) << "成功" << EMOJI_DOWN << "注" << "反面" << "到" << player.back << "个批";
    }

    CQ_sendGroupMsg(ac, group, ss.str().c_str());
}
}


namespace roulette
{

void roundStart(int64_t group)
{
    if (groupMap[group].roulette_running)
    {
        CQ_sendGroupMsg(ac, group, "There is already a game running at this group.");
        return;
    }

    CQ_sendGroupMsg(ac, group, "新一轮的摇号开始了，请群员踊跃参加");
	groupMap[group].roulette = {};

    groupMap[group].roulette.startTime = time(nullptr);
    std::thread([=]() {
        using namespace std::chrono_literals;

        // 40s
        std::this_thread::sleep_for(20s);
        roundAnnounce(group);

        // 20s
        std::this_thread::sleep_for(20s);
        roundAnnounce(group);

        // 10s
        std::this_thread::sleep_for(10s);
        roundAnnounce(group);

        // 5s
        std::this_thread::sleep_for(5s);
        //roulette::roundAnnounce(group);

        // end
        std::this_thread::sleep_for(5s);
        roundEnd(group);

        }).detach();
}

void roundAnnounce(int64_t group)
{
    if (groupMap[group].roulette_running)
    {
        CQ_sendGroupMsg(ac, group, "No roulette round is running at this group.");
        return;
    }

    std::stringstream ss;
    const auto& r = groupMap[group].roulette;
    ss << "本轮摇号时间还剩" << r.startTime + 60 - time(nullptr) << "秒，当前" << EMOJI_DOWN << "注情况：\n";

    ss << "总计" << r.total << "个批";
    if (r.total > 0)
    {
        /*
        for (size_t i = 0; i < GRID_COUNT; ++i)
            if (r.amount[i])
                ss << "，" << gridName[i] << ": " << r.amount[i];
                */

        for (size_t i = 0; i < GRID_COUNT; ++i)
        {
            if (!r.amount[i]) continue;
            ss << "\n";
            ss << gridName[i] << ": ";
            for (const auto& [qq, stat] : r.pee_per_player)
                if (stat.amount[i])
                    ss << getCard(group, qq) << "("<< stat.amount[i] << "个批) ";
        }
    }
    CQ_sendGroupMsg(ac, group, ss.str().c_str());
}

void roundEnd(int64_t group)
{
    if (groupMap[group].roulette_running)
    {
        CQ_sendGroupMsg(ac, group, "No roulette round is running at this group.");
        return;
    }

    const auto& r = groupMap[group].roulette;
    if (r.total == 0)
    {
        CQ_sendGroupMsg(ac, group, cstr_nobody);
        roundCancel(group);
        return;
    }

    int result = randInt(0, 36);
    std::set<unsigned> red_idx{ 1, 3, 5, 7, 9, 12, 14, 16, 18, 19, 21, 23, 25, 27, 30, 32, 34, 36 };
    std::set<unsigned> blk_idx{ 2, 4, 6, 8, 10, 11, 13, 15, 17, 20, 22, 24, 26, 28, 29, 31, 33, 35 };
    bool b_red = red_idx.find(result) != red_idx.end();
    bool b_black = blk_idx.find(result) != blk_idx.end();
    bool b_odd = result % 2 != 0;
    bool b_even = !b_odd && result != 0;
    bool b_1st = result >= 1 && result <= 12;
    bool b_2nd = result >= 13 && result <= 24;
    bool b_3rd = result >= 25 && result <= 36;

    std::map<int64_t, int64_t> reward;
    for (const auto& [qq, bet] : r.pee_per_player)
    {
        int64_t reward_tmp = 0;
        if (result == 0 && bet.amount[0]) reward_tmp += bet.amount[0] * 50;
        if (result != 0 && bet.amount[result]) reward_tmp += bet.amount[result] * 36;
        if (b_black && bet.amount[Cblack]) reward_tmp += bet.amount[Cblack] * 2;
        if (b_red && bet.amount[Cred]) reward_tmp += bet.amount[Cred] * 2;
        if (b_odd && bet.amount[Aodd]) reward_tmp += bet.amount[Aodd] * 2;
        if (b_even && bet.amount[Aeven]) reward_tmp += bet.amount[Aeven] * 2;
        if (b_1st && bet.amount[P1st]) reward_tmp += bet.amount[P1st] * 3;
        if (b_2nd && bet.amount[P2nd]) reward_tmp += bet.amount[P2nd] * 3;
        if (b_3rd && bet.amount[P3rd]) reward_tmp += bet.amount[P3rd] * 3;
        if (reward_tmp)
        {
            CQ_addLog(ac, CQLOG_DEBUG, "duel", ("reward: "s + std::to_string(qq) + " " + std::to_string(reward_tmp)).c_str());
			plist[qq].modifyCurrency(reward_tmp);
            reward[qq] = reward_tmp;
        }
    }

    std::stringstream ss;
    ss << "本轮摇号结束，结果为："
        << "[" << result << "]";
    if (result != 0)
        ss << (b_black ? " [黑]" : " [红]")
            << (b_odd ? " [单]" : " [双]")
            << (b_1st ? " [1st]" : "")
            << (b_2nd ? " [2nd]" : "")
            << (b_3rd ? " [3rd]" : "");
    ss << "，获奖人有：";
    if (!reward.empty())
        for (auto& [qq, amount] : reward)
            ss << "\n" << CQ_At(qq) << " (" << amount << "个批)";
    else
        ss << "\n" << EMOJI_NONE;
    CQ_sendGroupMsg(ac, group, ss.str().c_str());
    groupMap[group].roulette_running = false;
}

void roundCancel(int64_t group)
{
    if (groupMap[group].roulette_running)
        return;

    for (const auto& [qq, stat] : groupMap[group].roulette.pee_per_player)
    {
		int64_t total = 0;
        for (const auto& a : stat.amount)
            total += a;
		plist[qq].modifyCurrency(+total);
    }

    groupMap[group].roulette_running = false;
    CQ_sendGroupMsg(ac, group, "号不摇了，返还所有批");
}

void put(int64_t group, int64_t qq, grid g, int64_t amount)
{
    if (groupMap[group].roulette_running)
    {
        CQ_sendGroupMsg(ac, group, "本群么得开盘啊");
        return;
    }

    if (amount < 0)
    {
        CQ_sendGroupMsg(ac, group, "你就是负批怪？");
        return;
    }

    if (user::plist[qq].getCurrency() < amount)
    {
        std::string s = CQ_At(qq) + "，你的余额不足";
        CQ_sendGroupMsg(ac, group, s.c_str());
        return;
    }

    auto& round = groupMap[group].roulette;
    auto& player = round.pee_per_player[qq];
    player.amount[g] += amount;

    round.amount[g] += amount;
    round.total += amount;

	plist[qq].modifyCurrency(-amount);

    std::stringstream ss;
    if (player.amount[g] == amount)
        ss << getCard(group, qq) << "成功" << EMOJI_DOWN << "注[" << gridName[g] << "]" << amount << "个批";
    else
        ss << getCard(group, qq) << "成功" << EMOJI_DOWN << "注[" << gridName[g] << "]到" << player.amount[g] << "个批";

    CQ_sendGroupMsg(ac, group, ss.str().c_str());
}
}

}

