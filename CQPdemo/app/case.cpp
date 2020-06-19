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
	case commands::����:
		c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
		{
			if (plist.find(qq) == plist.end()) return std::string(CQ_At(qq)) + "���㻹û�п�ͨ����";
			auto &p = plist[qq];
			//CQ_setGroupBan(ac, group, qq, 60);
			//return "��׼��";

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
					ss << CQ_At(qq) << "������������㣬��������"
						<< rtime / (60 * 60) << "Сʱ" << rtime / 60 % 60 << "����";
					return ss.str();
				}
				p.modifyCurrency(-FEE_PER_CASE);
				p.modifyStamina(-1, true);
			}
			else
				return std::string(CQ_At(qq)) + "���������";

			const case_detail& reward = draw_case(randReal());
			if (reward > 300) ss << "��Ӵ��" << CQ_At(qq) << "���ˣ�������";
			else ss << CQ_At(qq) << "����ϲ�㿪����";
			ss << reward.full() << "�����" << reward.worth() << "����";

			p.modifyCurrency(reward.worth());
			p.modifyBoxCount(+1);
			//ss << "�㻹��" << stamina << "��������";

			return ss.str();
		};
		break;
	case commands::����10:
		c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
		{
			if (plist.find(qq) == plist.end()) return std::string(CQ_At(qq)) + "���㻹û�п�ͨ����";
			auto &p = plist[qq];
			//CQ_setGroupBan(ac, group, qq, 60);
			//return "��׼��";

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
					ss << CQ_At(qq) << "������������㣬��������"
						<< rtime / (60 * 60) << "Сʱ" << rtime / 60 % 60 << "����";
					return ss.str();
				}
				p.modifyCurrency(-FEE_PER_CASE * 10);
				p.modifyStamina(-10, true);
			}
			else
				return std::string(CQ_At(qq)) + "���������";

			std::vector<int> case_counts(CASE_POOL.size() + 1, 0);
			int r = 0;
			/*
			for (size_t i = 0; i < 10; ++i)
			{
				const case_detail& reward = draw_case(randReal());
				case_counts[reward.type_idx()]++;
				r += reward.worth();
			}
			if (r > 300) ss << "��Ӵ��" << CQ_At(qq) << "���ˣ�������";
			else ss << CQ_At(qq) << "����ϲ�㿪����";
			for (size_t i = 0; i < case_counts.size(); ++i)
			{
				if (case_counts[i])
				{
					ss << case_counts[i] << "��" <<
						((i == CASE_POOL.size()) ? CASE_DEFAULT.name() : CASE_POOL[i].name()) << "��";
				}
			}
			ss << "һ��" << r << "����";
			*/

			ss << CQ_At(qq) << "����ϲ�㿪���ˣ�\n";
			for (size_t i = 0; i < 10; ++i)
			{
				const case_detail& reward = draw_case(randReal());
				case_counts[reward.type_idx()]++;
				r += reward.worth();
				ss << "- " << reward.full() << " (" << reward.worth() << "��)\n";
			}
			ss << "������";
			for (size_t i = 0; i < case_counts.size(); ++i)
			{
				if (case_counts[i])
				{
					ss << case_counts[i] << "��" <<
						((i == CASE_POOL.size()) ? CASE_DEFAULT.name() : CASE_POOL[i].name()) << "��";
				}
			}
			ss << "һ��" << r << "����";

			p.modifyCurrency(+r);
			p.modifyBoxCount(+10);

			return ss.str();
		};
		break;
	case commands::������:
		c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
		{
			if (plist.find(qq) == plist.end()) return std::string(CQ_At(qq)) + "���㻹û�п�ͨ����";
			auto &p = plist[qq];
			//CQ_setGroupBan(ac, group, qq, 60);
			//return "��׼��";

			if (p.getCurrency() < COST_OPEN_RED)
				return std::string(CQ_At(qq)) + "���������";

			auto[enough, stamina, rtime] = p.testStamina(COST_OPEN_RED_STAMINA);

			std::stringstream ss;
			if (!enough) ss << CQ_At(qq) << "������������㣬��������"
				<< rtime / (60 * 60) << "Сʱ" << rtime / 60 % 60 << "����";
			else
			{
				p.modifyCurrency(-COST_OPEN_RED);
				p.modifyStamina(-COST_OPEN_RED_STAMINA, true);
				ss << getCard(group, qq) << "����" << COST_OPEN_RED << "������" << COST_OPEN_RED_STAMINA << "�������������ܣ�\n";

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
						ss << CQ_At(qq) << "����" << count << "���������ڿ�����" << reward.full() << " (" << reward.worth() << "��)��"
							<< "���ξ�����" << pee - p.getCurrency() << "����";
						p.modifyCurrency(pee - p.getCurrency());
						p.modifyBoxCount(count);
						return ss.str();
					}
					else
						if (reward.type_idx() == 1)
						{
							ss << "��Ӵ��" << CQ_At(qq) << "���ˣ�����" << count << "�����Ӿ�Ȼ������" << reward.full() << " (" << reward.worth() << "��)��"
								<< "���ξ�����" << pee - p.getCurrency() << "����";
							p.modifyCurrency(pee - p.getCurrency());
							p.modifyBoxCount(count);
							return ss.str();
						}
				} while (pee >= FEE_PER_CASE);

				ss << CQ_At(qq) << "�Ʋ��ˣ�����" << count << "������Ҳû�ܿ������䣬"
					<< "���ξ�����" << pee - p.getCurrency() << "����";
				p.modifyCurrency(pee - p.getCurrency());
				p.modifyBoxCount(count);
				return ss.str();

				//ss << "�㻹��" << stamina << "��������";
			}

			return ss.str();
		};
		break;
	case commands::������:
		c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
		{
			if (plist.find(qq) == plist.end()) return std::string(CQ_At(qq)) + "���㻹û�п�ͨ����";
			auto &p = plist[qq];
			//CQ_setGroupBan(ac, group, qq, 60);
			//return "��׼��";

			if (p.getCurrency() < COST_OPEN_YELLOW)
				return std::string(CQ_At(qq)) + "���������";

			auto[enough, stamina, rtime] = p.testStamina(user::MAX_STAMINA);

			std::stringstream ss;
			if (!enough) ss << CQ_At(qq) << "������������㣬��������"
				<< rtime / (60 * 60) << "Сʱ" << rtime / 60 % 60 << "����";
			else
			{
				p.modifyCurrency(-COST_OPEN_YELLOW);
				p.modifyStamina(-COST_OPEN_RED_STAMINA, true);
				ss << getCard(group, qq) << "����" << COST_OPEN_YELLOW << "������ȫ�������������ܣ�\n";

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
						ss << CQ_At(qq) << "����" << count << "���������ڿ�����" << reward.full() << " (" << reward.worth() << "��)��"
							<< "���ξ�����" << pee - p.getCurrency() << "����";
						p.modifyCurrency(pee - p.getCurrency());
						p.modifyBoxCount(count);
						return ss.str();
					}
				} while (pee >= FEE_PER_CASE);

				ss << CQ_At(qq) << "�Ʋ��ˣ�����" << count << "������Ҳû�ܿ������䣬"
					<< "���ξ�����" << pee - p.getCurrency() << "����";
				p.modifyCurrency(pee - p.getCurrency());
				p.modifyBoxCount(count);
				return ss.str();

				//ss << "�㻹��" << stamina << "��������";
			}

			return ss.str();
		};
		break;

	case commands::����endless:
		c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
		{
			return "���̨��Ⱥ��͵�ˣ�û������";
		};
		break;
		/*
	case commands::����endless:
		c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
		{
			if (plist.find(qq) == plist.end()) return std::string(CQ_At(qq)) + "���㻹û�п�ͨ����";
			//CQ_setGroupBan(ac, group, qq, 60);
			//return "��׼��";

			if (plist[qq].currency < COST_OPEN_ENDLESS)
				return std::string(CQ_At(qq)) + "���������";

			auto [enough, stamina, rtime] = updateStamina(qq, COST_OPEN_ENDLESS_STAMINA);

			std::stringstream ss;
			if (!enough) ss << CQ_At(qq) << "������������㣬��������"
				<< rtime / (60 * 60) << "Сʱ" << rtime / 60 % 60 << "����";
			else
			{
				plist[qq].currency -= COST_OPEN_ENDLESS;
				long long total_cases = plist[qq].currency / FEE_PER_CASE;
				plist[qq].currency %= FEE_PER_CASE;
				long long extra_cost = 0.1 * total_cases * FEE_PER_CASE;
				total_cases -= long long(std::floor(total_cases * 0.1));
				ss << getCard(group, qq) << "����" << COST_OPEN_ENDLESS + extra_cost << "������" << COST_OPEN_ENDLESS_STAMINA << "�������������ܣ�\n";
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

				ss << CQ_At(qq) << "�����������" << count << "�����ӣ�������" << case_counts[1] << "�����䣬" << case_counts[2] << "�����䣬" << case_counts[0] << "�����䣬\n"
					<< "��ֵǮ����" << max.full() << " (" << max.worth() << "��)��"
					<< "���ξ�����" << res - cost - COST_OPEN_ENDLESS - extra_cost << "����";
				if (plist[qq].currency < 0) plist[qq].currency = 0;
				modifyCurrency(qq, plist[qq].currency);
				modifyBoxCount(qq, plist[qq].opened_box_count);
				return ss.str();

				//ss << "�㻹��" << stamina << "��������";
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