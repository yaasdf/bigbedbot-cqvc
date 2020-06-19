#include "case.h"
#include "data/user.h"
#include "cqp_ex.h"
#include "utils/string_util.h"
#include "utils/rand.h"
#include <sstream>

namespace opencase
{
using user::plist;

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
	case commands::开箱:
		c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
		{
			if (plist.find(qq) == plist.end()) return std::string(CQ_At(qq)) + "，你还没有开通菠菜";
			auto &p = plist[qq];
			//CQ_setGroupBan(ac, group, qq, 60);
			//return "不准开";

			std::stringstream ss;
			if (p.getKeyCount() >= 1)
			{
				p.modifyBoxCount(-1);
			}
			else if (p.getCurrency() >= FEE_PER_CASE)
			{
				auto[enough, stamina, rtime] = p.testStamina(1);
				if (!enough)
				{
					ss << CQ_At(qq) << "，你的体力不足，回满还需"
						<< rtime / (60 * 60) << "小时" << rtime / 60 % 60 << "分钟";
					return ss.str();
				}
				p.modifyCurrency(-FEE_PER_CASE);
				p.modifyStamina(-1, true);
			}
			else
				return std::string(CQ_At(qq)) + "，你的余额不足";

			const case_detail& reward = draw_case(randReal());
			if (reward > 300) ss << "歪哟，" << CQ_At(qq) << "发了，开出了";
			else ss << CQ_At(qq) << "，恭喜你开出了";
			ss << reward.full() << "，获得" << reward.worth() << "个批";

			p.modifyCurrency(reward.worth());
			p.modifyBoxCount(+1);
			//ss << "你还有" << stamina << "点体力，";

			return ss.str();
		};
		break;
	case commands::开箱10:
		c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
		{
			if (plist.find(qq) == plist.end()) return std::string(CQ_At(qq)) + "，你还没有开通菠菜";
			auto &p = plist[qq];
			//CQ_setGroupBan(ac, group, qq, 60);
			//return "不准开";

			std::stringstream ss;
			if (p.getKeyCount() >= 10)
			{
				p.modifyKeyCount(-10);
			}
			else if (p.getCurrency() >= FEE_PER_CASE * 10)
			{
				auto[enough, stamina, rtime] = p.testStamina(10);
				if (!enough)
				{
					ss << CQ_At(qq) << "，你的体力不足，回满还需"
						<< rtime / (60 * 60) << "小时" << rtime / 60 % 60 << "分钟";
					return ss.str();
				}
				p.modifyCurrency(-FEE_PER_CASE * 10);
				p.modifyStamina(-10, true);
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

			p.modifyCurrency(+r);
			p.modifyBoxCount(+10);

			return ss.str();
		};
		break;
	case commands::开红箱:
		c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
		{
			if (plist.find(qq) == plist.end()) return std::string(CQ_At(qq)) + "，你还没有开通菠菜";
			auto &p = plist[qq];
			//CQ_setGroupBan(ac, group, qq, 60);
			//return "不准开";

			if (p.getCurrency() < COST_OPEN_RED)
				return std::string(CQ_At(qq)) + "，你的余额不足";

			auto[enough, stamina, rtime] = p.testStamina(COST_OPEN_RED_STAMINA);

			std::stringstream ss;
			if (!enough) ss << CQ_At(qq) << "，你的体力不足，回满还需"
				<< rtime / (60 * 60) << "小时" << rtime / 60 % 60 << "分钟";
			else
			{
				p.modifyCurrency(-COST_OPEN_RED);
				p.modifyStamina(-COST_OPEN_RED_STAMINA, true);
				ss << getCard(group, qq) << "消耗" << COST_OPEN_RED << "个批和" << COST_OPEN_RED_STAMINA << "点体力发动技能！\n";

				std::vector<int> case_counts(CASE_POOL.size() + 1, 0);
				int count = 0;
				int cost = 0;
				int res = 0;
				int64_t pee = p.getCurrency();
				case_detail reward;
				do {
					++count;
					cost += FEE_PER_CASE;
					reward = draw_case(randReal());
					case_counts[reward.type_idx()]++;
					res += reward.worth();
					pee += reward.worth() - FEE_PER_CASE;

					if (reward.type_idx() == 2)
					{
						ss << CQ_At(qq) << "开了" << count << "个箱子终于开出了" << reward.full() << " (" << reward.worth() << "批)，"
							<< "本次净收益" << pee - p.getCurrency() << "个批";
						p.modifyCurrency(pee - p.getCurrency());
						p.modifyBoxCount(count);
						return ss.str();
					}
					else
						if (reward.type_idx() == 1)
						{
							ss << "歪哟，" << CQ_At(qq) << "发了，开了" << count << "个箱子居然开出了" << reward.full() << " (" << reward.worth() << "批)，"
								<< "本次净收益" << pee - p.getCurrency() << "个批";
							p.modifyCurrency(pee - p.getCurrency());
							p.modifyBoxCount(count);
							return ss.str();
						}
				} while (pee >= FEE_PER_CASE);

				ss << CQ_At(qq) << "破产了，开了" << count << "个箱子也没能开出红箱，"
					<< "本次净收益" << pee - p.getCurrency() << "个批";
				p.modifyCurrency(pee - p.getCurrency());
				p.modifyBoxCount(count);
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
			auto &p = plist[qq];
			//CQ_setGroupBan(ac, group, qq, 60);
			//return "不准开";

			if (p.getCurrency() < COST_OPEN_YELLOW)
				return std::string(CQ_At(qq)) + "，你的余额不足";

			auto[enough, stamina, rtime] = p.testStamina(user::MAX_STAMINA);

			std::stringstream ss;
			if (!enough) ss << CQ_At(qq) << "，你的体力不足，回满还需"
				<< rtime / (60 * 60) << "小时" << rtime / 60 % 60 << "分钟";
			else
			{
				p.modifyCurrency(-COST_OPEN_YELLOW);
				p.modifyStamina(-COST_OPEN_RED_STAMINA, true);
				ss << getCard(group, qq) << "消耗" << COST_OPEN_YELLOW << "个批和全部体力发动技能！\n";

				std::vector<int> case_counts(CASE_POOL.size() + 1, 0);
				int count = 0;
				int cost = 0;
				int res = 0;
				int64_t pee = p.getCurrency();
				case_detail reward;
				do {
					++count;
					cost += FEE_PER_CASE;
					reward = draw_case(randReal());
					case_counts[reward.type_idx()]++;
					res += reward.worth();
					pee += reward.worth() - FEE_PER_CASE;

					if (reward.type_idx() == 1)
					{
						ss << CQ_At(qq) << "开了" << count << "个箱子终于开出了" << reward.full() << " (" << reward.worth() << "批)，"
							<< "本次净收益" << pee - p.getCurrency() << "个批";
						p.modifyCurrency(pee - p.getCurrency());
						p.modifyBoxCount(count);
						return ss.str();
					}
				} while (pee >= FEE_PER_CASE);

				ss << CQ_At(qq) << "破产了，开了" << count << "个箱子也没能开出黄箱，"
					<< "本次净收益" << pee - p.getCurrency() << "个批";
				p.modifyCurrency(pee - p.getCurrency());
				p.modifyBoxCount(count);
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